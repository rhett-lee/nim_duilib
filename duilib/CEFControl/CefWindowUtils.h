#ifndef UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_
#define UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_

#include "CefControl.h"
#include <vector>

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

/** 抓取CEF窗口的截图为位图数据
*/
bool CaptureCefWindowBitmap(CefWindowHandle cefWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height);

/** 设置光标
*/
void SetCefWindowCursor(CefWindowHandle cefWindow, CefCursorHandle cursor);

/** 解除CEF子窗口与父窗口的父子关系
*/
void RemoveCefWindowFromParent(CefWindowHandle cefWindow);

} //namespace ui

#endif //UI_CEF_CONTROL_CEF_WINDOW_UTILS_H_
