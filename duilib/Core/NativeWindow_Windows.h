#ifndef UI_CORE_NATIVE_WINDOW_WINDOWS_H_
#define UI_CORE_NATIVE_WINDOW_WINDOWS_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui {

#define UI_CLASS_STYLE_FRAME        (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
#define UI_WINDOW_STYLE_FRAME       (WS_VISIBLE | WS_OVERLAPPEDWINDOW)

/** 创建窗口所需的参数
*/
class WindowCreateParam
{
public:
    /** 资源所在模块句柄，如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    HMODULE m_hResModule = nullptr;

    /** 窗口类名（可选参数）
    */
    DString m_className = _T("nim_duilib_window");

    /** 窗口类的风格（可选参数）, 参考：https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-class-styles
    */
    uint32_t m_dwClassStyle = UI_CLASS_STYLE_FRAME;

    /** 该窗口类中的窗口图标的资源ID（可选参数）
    */
    uint32_t m_nClassLogoResId = 0;

public:
    /** 窗口风格（可选参数）
    */
    uint32_t m_dwStyle = UI_WINDOW_STYLE_FRAME;

    /** 窗口扩展风格（可选参数）
    */
    uint32_t m_dwExStyle = 0;

    /** 窗口的标题（可选参数）
    */
    DString m_windowTitle;
};
 
} // namespace ui

#endif // UI_CORE_NATIVE_WINDOW_WINDOWS_H_

