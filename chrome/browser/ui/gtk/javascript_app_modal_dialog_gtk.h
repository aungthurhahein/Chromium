// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_GTK_JAVASCRIPT_APP_MODAL_DIALOG_GTK_H_
#define CHROME_BROWSER_UI_GTK_JAVASCRIPT_APP_MODAL_DIALOG_GTK_H_
#pragma once

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/ui/app_modal_dialogs/native_app_modal_dialog.h"
#include "ui/base/gtk/gtk_signal.h"
#include "ui/gfx/native_widget_types.h"

typedef struct _GtkWidget GtkWidget;

class JavaScriptAppModalDialog;

class JavaScriptAppModalDialogGtk : public NativeAppModalDialog {
 public:
  JavaScriptAppModalDialogGtk(JavaScriptAppModalDialog* dialog,
                              gfx::NativeWindow parent_window);
  virtual ~JavaScriptAppModalDialogGtk();

  // Overridden from NativeAppModalDialog:
  virtual int GetAppModalDialogButtons() const OVERRIDE;
  virtual void ShowAppModalDialog() OVERRIDE;
  virtual void ActivateAppModalDialog() OVERRIDE;
  virtual void CloseAppModalDialog() OVERRIDE;
  virtual void AcceptAppModalDialog() OVERRIDE;
  virtual void CancelAppModalDialog() OVERRIDE;

 private:
  CHROMEGTK_CALLBACK_1(JavaScriptAppModalDialogGtk, void, OnResponse, int);

  scoped_ptr<JavaScriptAppModalDialog> dialog_;
  GtkWidget* gtk_dialog_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptAppModalDialogGtk);
};

#endif  // CHROME_BROWSER_UI_GTK_JAVASCRIPT_APP_MODAL_DIALOG_GTK_H_
