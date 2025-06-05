#ifndef UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_
#define UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_

#include "CefControl.h"

namespace ui
{
/** 设置CEF关联的窗口位置(位置同pCefControl的位置)
*/
void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl);

/** 设置CEF关联的窗口可见性(可见性同pCefControl相同)
*/
void SetCefWindowVisible(CefWindowHandle cefWindow, CefControl* pCefControl);

/** 设置CEF关联的窗口的父窗口(父窗口为pCefControl的关联窗口)
*/
void SetCefWindowParent(CefWindowHandle cefWindow, CefControl* pCefControl);

} //namespace ui

#endif //UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_
