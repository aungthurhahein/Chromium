// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file defines helper functions shared by the various implementations
// of OmniboxView.

#include "chrome/browser/ui/omnibox/omnibox_view.h"

#include "base/string_util.h"
#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "ui/base/clipboard/clipboard.h"

string16 OmniboxView::StripJavascriptSchemas(const string16& text) {
  const string16 kJsPrefix(ASCIIToUTF16(chrome::kJavaScriptScheme) +
                           ASCIIToUTF16(":"));
  string16 out(text);
  while (StartsWith(out, kJsPrefix, false))
    TrimWhitespace(out.substr(kJsPrefix.length()), TRIM_LEADING, &out);
  return out;
}

// static
string16 OmniboxView::GetClipboardText() {
  // Try text format.
  ui::Clipboard* clipboard = g_browser_process->clipboard();
  if (clipboard->IsFormatAvailable(ui::Clipboard::GetPlainTextWFormatType(),
                                   ui::Clipboard::BUFFER_STANDARD)) {
    string16 text;
    clipboard->ReadText(ui::Clipboard::BUFFER_STANDARD, &text);
    // Note: Unlike in the find popup and textfield view, here we completely
    // remove whitespace strings containing newlines.  We assume users are
    // most likely pasting in URLs that may have been split into multiple
    // lines in terminals, email programs, etc., and so linebreaks indicate
    // completely bogus whitespace that would just cause the input to be
    // invalid.
    return StripJavascriptSchemas(CollapseWhitespace(text, true));
  }

  // Try bookmark format.
  //
  // It is tempting to try bookmark format first, but the URL we get out of a
  // bookmark has been cannonicalized via GURL.  This means if a user copies
  // and pastes from the URL bar to itself, the text will get fixed up and
  // cannonicalized, which is not what the user expects.  By pasting in this
  // order, we are sure to paste what the user copied.
  if (clipboard->IsFormatAvailable(ui::Clipboard::GetUrlWFormatType(),
                                   ui::Clipboard::BUFFER_STANDARD)) {
    std::string url_str;
    clipboard->ReadBookmark(NULL, &url_str);
    // pass resulting url string through GURL to normalize
    GURL url(url_str);
    if (url.is_valid())
      return StripJavascriptSchemas(UTF8ToUTF16(url.spec()));
  }

  return string16();
}
