// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_INVERT_BUBBLE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_INVERT_BUBBLE_VIEW_H_
#pragma once

class Profile;

namespace views {
class View;
}

namespace browser {

// Show a bubble telling the user that they're using Windows high-constrast mode
// with a light-on-dark scheme, so they may be interested in a high-contrast
// Chrome extension and a dark theme. Only shows the first time we encounter
// this condition for a particular profile.
void MaybeShowInvertBubbleView(Profile* profile, views::View* anchor_view);

}  // namespace browser

#endif  // CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_INVERT_BUBBLE_VIEW_H_
