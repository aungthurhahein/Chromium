// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/ime/tray_ime.h"

#include <vector>

#include "ash/shell.h"
#include "ash/system/tray/system_tray.h"
#include "ash/system/tray/system_tray_delegate.h"
#include "ash/system/tray/tray_constants.h"
#include "ash/system/tray/tray_details_view.h"
#include "ash/system/tray/tray_item_more.h"
#include "ash/system/tray/tray_item_view.h"
#include "ash/system/tray/tray_views.h"
#include "base/logging.h"
#include "base/timer.h"
#include "base/utf_string_conversions.h"
#include "grit/ash_strings.h"
#include "grit/ui_resources_standard.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/font.h"
#include "ui/gfx/image/image.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/widget/widget.h"

namespace ash {
namespace internal {

namespace tray {

class IMEDefaultView : public TrayItemMore {
 public:
  explicit IMEDefaultView(SystemTrayItem* owner)
      : TrayItemMore(owner) {
    ui::ResourceBundle& bundle = ui::ResourceBundle::GetSharedInstance();

    SetImage(bundle.GetImageNamed(
        IDR_AURA_UBER_TRAY_IME).ToImageSkia());

    IMEInfo info;
    Shell::GetInstance()->tray_delegate()->GetCurrentIME(&info);
    UpdateLabel(info);
  }

  virtual ~IMEDefaultView() {}

  void UpdateLabel(const IMEInfo& info) {
    SetLabel(info.name);
    SetAccessibleName(info.name);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(IMEDefaultView);
};

class IMEDetailedView : public TrayDetailsView,
                        public ViewClickListener {
 public:
  IMEDetailedView(SystemTrayItem* owner, user::LoginStatus login)
      : login_(login) {
    SystemTrayDelegate* delegate = Shell::GetInstance()->tray_delegate();
    IMEInfoList list;
    delegate->GetAvailableIMEList(&list);
    IMEPropertyInfoList property_list;
    delegate->GetCurrentIMEProperties(&property_list);
    Update(list, property_list);
  }

  virtual ~IMEDetailedView() {}

  void Update(const IMEInfoList& list,
              const IMEPropertyInfoList& property_list) {
    Reset();

    AppendIMEList(list);
    if (!property_list.empty())
      AppendIMEProperties(property_list);
    if (login_ != user::LOGGED_IN_NONE && login_ != user::LOGGED_IN_LOCKED)
      AppendSettings();
    AppendHeaderEntry();

    Layout();
    SchedulePaint();
  }

 private:
  void AppendHeaderEntry() {
    CreateSpecialRow(IDS_ASH_STATUS_TRAY_IME, this);
  }

  void AppendIMEList(const IMEInfoList& list) {
    ime_map_.clear();
    CreateScrollableList();
    for (size_t i = 0; i < list.size(); i++) {
      HoverHighlightView* container = new HoverHighlightView(this);
      container->set_fixed_height(kTrayPopupItemHeight);
      container->AddLabel(list[i].name,
          list[i].selected ? gfx::Font::BOLD : gfx::Font::NORMAL);
      scroll_content()->AddChildView(container);
      ime_map_[container] = list[i].id;
    }
  }

  void AppendIMEProperties(const IMEPropertyInfoList& property_list) {
    property_map_.clear();
    for (size_t i = 0; i < property_list.size(); i++) {
      HoverHighlightView* container = new HoverHighlightView(this);
      container->set_fixed_height(kTrayPopupItemHeight);
      container->AddLabel(
          property_list[i].name,
          property_list[i].selected ? gfx::Font::BOLD : gfx::Font::NORMAL);
      if (i == 0)
        container->set_border(views::Border::CreateSolidSidedBorder(1, 0, 0, 0,
        kBorderLightColor));
      scroll_content()->AddChildView(container);
      property_map_[container] = property_list[i].key;
    }
  }

  void AppendSettings() {
    HoverHighlightView* container = new HoverHighlightView(this);
    container->set_fixed_height(kTrayPopupItemHeight);
    container->AddLabel(ui::ResourceBundle::GetSharedInstance().
        GetLocalizedString(IDS_ASH_STATUS_TRAY_IME_SETTINGS),
        gfx::Font::NORMAL);
    AddChildView(container);
    settings_ = container;
  }

  // Overridden from ViewClickListener.
  virtual void ClickedOn(views::View* sender) OVERRIDE {
    SystemTrayDelegate* delegate = Shell::GetInstance()->tray_delegate();
    if (sender == footer()->content()) {
      Shell::GetInstance()->system_tray()->ShowDefaultView(BUBBLE_USE_EXISTING);
    } else if (sender == settings_) {
      delegate->ShowIMESettings();
    } else {
      std::map<views::View*, std::string>::const_iterator ime_find;
      ime_find = ime_map_.find(sender);
      if (ime_find != ime_map_.end()) {
        std::string ime_id = ime_find->second;
        delegate->SwitchIME(ime_id);
        GetWidget()->Close();
      } else {
        std::map<views::View*, std::string>::const_iterator prop_find;
        prop_find = property_map_.find(sender);
        if (prop_find != property_map_.end()) {
          const std::string key = prop_find->second;
          delegate->ActivateIMEProperty(key);
          GetWidget()->Close();
        }
      }
    }
  }

  user::LoginStatus login_;

  std::map<views::View*, std::string> ime_map_;
  std::map<views::View*, std::string> property_map_;
  views::View* settings_;

  DISALLOW_COPY_AND_ASSIGN(IMEDetailedView);
};

class IMENotificationView : public TrayNotificationView {
 public:
  explicit IMENotificationView(TrayIME* tray)
      : TrayNotificationView(IDR_AURA_UBER_TRAY_IME),
        tray_(tray) {
    InitView(new views::Label(
        ui::ResourceBundle::GetSharedInstance().GetLocalizedString(
            IDS_ASH_STATUS_TRAY_IME_TURNED_ON_BUBBLE)));
  }

  void StartAutoCloseTimer(int seconds) {
    autoclose_.Stop();
    autoclose_delay_ = seconds;
    if (autoclose_delay_) {
      autoclose_.Start(FROM_HERE,
                       base::TimeDelta::FromSeconds(autoclose_delay_),
                       this, &IMENotificationView::Close);
    }
  }

  void StopAutoCloseTimer() {
    autoclose_.Stop();
  }

  void RestartAutoCloseTimer() {
    if (autoclose_delay_)
      StartAutoCloseTimer(autoclose_delay_);
  }

  // Overridden from TrayNotificationView:
  virtual void OnClose() OVERRIDE {
    Close();
  }

 private:
  void Close() {
    tray_->HideNotificationView();
  }

  TrayIME* tray_;

  int autoclose_delay_;
  base::OneShotTimer<IMENotificationView> autoclose_;

  DISALLOW_COPY_AND_ASSIGN(IMENotificationView);
};


}  // namespace tray

TrayIME::TrayIME()
    : tray_label_(NULL),
      default_(NULL),
      detailed_(NULL),
      notification_(NULL) {
}

TrayIME::~TrayIME() {
}

void TrayIME::UpdateTrayLabel(const IMEInfo& current, size_t count) {
  if (tray_label_) {
    tray_label_->label()->SetText(current.short_name);
    tray_label_->SetVisible(count > 1);
  }
}

views::View* TrayIME::CreateTrayView(user::LoginStatus status) {
  CHECK(tray_label_ == NULL);
  tray_label_ = new TrayItemView;
  tray_label_->CreateLabel();
  SetupLabelForTray(tray_label_->label());
  tray_label_->label()->set_border(
      views::Border::CreateEmptyBorder(0, 2, 0, 2));
  return tray_label_;
}

views::View* TrayIME::CreateDefaultView(user::LoginStatus status) {
  SystemTrayDelegate* delegate = Shell::GetInstance()->tray_delegate();
  IMEInfoList list;
  IMEPropertyInfoList property_list;
  delegate->GetAvailableIMEList(&list);
  delegate->GetCurrentIMEProperties(&property_list);
  if (list.size() <= 1 && property_list.size() <= 1)
    return NULL;
  CHECK(default_ == NULL);
  default_ = new tray::IMEDefaultView(this);
  return default_;
}

views::View* TrayIME::CreateDetailedView(user::LoginStatus status) {
  CHECK(detailed_ == NULL);
  detailed_ = new tray::IMEDetailedView(this, status);
  return detailed_;
}

views::View* TrayIME::CreateNotificationView(user::LoginStatus status) {
  DCHECK(notification_ == NULL);
  notification_ = new tray::IMENotificationView(this);
  notification_->StartAutoCloseTimer(kTrayPopupAutoCloseDelayForTextInSeconds);
  return notification_;
}

void TrayIME::DestroyTrayView() {
  tray_label_ = NULL;
}

void TrayIME::DestroyDefaultView() {
  default_ = NULL;
}

void TrayIME::DestroyDetailedView() {
  detailed_ = NULL;
}

void TrayIME::DestroyNotificationView() {
  notification_ = NULL;
}

void TrayIME::UpdateAfterLoginStatusChange(user::LoginStatus status) {
}

void TrayIME::OnIMERefresh(bool show_message) {
  SystemTrayDelegate* delegate = Shell::GetInstance()->tray_delegate();
  IMEInfoList list;
  IMEInfo current;
  IMEPropertyInfoList property_list;
  delegate->GetCurrentIME(&current);
  delegate->GetAvailableIMEList(&list);
  delegate->GetCurrentIMEProperties(&property_list);

  UpdateTrayLabel(current, list.size());

  if (default_)
    default_->UpdateLabel(current);
  if (detailed_)
    detailed_->Update(list, property_list);

  if (show_message) {
    if (!notification_)
      ShowNotificationView();
    else
      notification_->RestartAutoCloseTimer();
  }
}

}  // namespace internal
}  // namespace ash
