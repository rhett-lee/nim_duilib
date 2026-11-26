// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_UTIL_WIN_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_UTIL_WIN_H_
#pragma once

#include "duilib/duilib_defs.h"

#ifdef DUILIB_BUILD_FOR_CEF

#include <string>

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/internal/cef_types_wrappers.h"
#pragma warning (pop)

namespace client {

// 获取当前时间（单位：微秒）
uint64_t GetTimeNow();

/** 设置窗口的用户数据指针
* @param [in] hWnd 窗口句柄
* @param [in] ptr 要设置的用户数据指针
*/
void SetUserDataPtr(HWND hWnd, void* ptr);

/** 获取窗口的用户数据指针
* @param [in] hWnd 窗口句柄
* @return 返回窗口的用户数据指针
*/
template <typename T>
T GetUserDataPtr(HWND hWnd) {
  return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

/** 设置窗口的窗口过程函数指针
* @param [in] hWnd 窗口句柄
* @param [in] wndProc 新的窗口过程函数
* @return 返回旧的窗口过程函数
*/
WNDPROC SetWndProcPtr(HWND hWnd, WNDPROC wndProc);

/** 获取资源字符串
* @param [in] id 字符串资源的 ID
* @return 返回资源字符串
*/
DString GetResourceString(UINT id);

/** 根据鼠标消息的 wParam 生成 CEF 鼠标事件的修饰键标志
* @param [in] wparam 鼠标消息的 wParam 参数（MK_XXX 标志的组合）
* @return 返回 EVENTFLAG_XXX 组合的修饰键标志
*/
int GetCefMouseModifiers(WPARAM wparam);

/** 根据键盘消息的 wParam 与 lParam 生成 CEF 键盘事件的修饰键标志
* @param [in] wparam 键盘消息的 wParam 参数（虚拟键码）
* @param [in] lparam 键盘消息的 lParam 参数
* @return 返回 EVENTFLAG_XXX 组合的修饰键标志
*/
int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);

/** 判断指定按键当前是否处于按下状态
* @param [in] wparam 虚拟键码
* @return 返回 true 表示按键处于按下状态
*/
bool IsKeyDown(WPARAM wparam);

/** 获取设备的缩放比例（例如 200% 的显示缩放将返回 2.0）
* @return 返回设备缩放比例
*/
float GetDeviceScaleFactor();

}  // namespace client

#endif //DUILIB_BUILD_FOR_CEF

#endif  // UI_CEF_CONTROL_UTIL_WIN_H_
