// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SHELL_BROWSER_UI_X_X_WINDOW_UTILS_H_
#define SHELL_BROWSER_UI_X_X_WINDOW_UTILS_H_

#include <string>

#include "ui/gfx/x/x11.h"

namespace electron {

::Atom GetAtom(const char* name);

// Sends a message to the x11 window manager, enabling or disabling the |state|
// for _NET_WM_STATE.
void SetWMSpecState(::Window xwindow, bool enabled, ::Atom state);

// Sets the _NET_WM_WINDOW_TYPE of window.
void SetWindowType(::Window xwindow, const std::string& type);

// Returns true if the bus name "com.canonical.AppMenu.Registrar" is available.
bool ShouldUseGlobalMenuBar();

// Bring the given window to the front and give it the focus.
void MoveWindowToForeground(::Window xwindow);

}  // namespace electron

#endif  // SHELL_BROWSER_UI_X_X_WINDOW_UTILS_H_
