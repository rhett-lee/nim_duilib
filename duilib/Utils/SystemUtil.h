#ifndef UI_UTILS_SYSTEM_UTIL_H_
#define UI_UTILS_SYSTEM_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
class Window;

/** 系统相关的辅助操作（打开URL、显示消息框）
*/
class DUILIB_API SystemUtil
{
public:
    /** 打开URL（使用系统的默认方式打开）
    * @param [in] url 需要打开的URL，为空时直接返回false
    * @return 打开成功返回true，否则返回false
    */
    static bool OpenUrl(const DString& url);

    /** 显示一个简单的MessageBox
    * @param [in] pWindow 父窗口，可为nullptr（此时使用系统默认父窗口）
    * @param [in] content 需要显示的内容
    * @param [in] title 标题
    * @return 显示成功返回true，否则返回false（Windows实现总是返回true）
    */
    static bool ShowMessageBox(const Window* pWindow, const DString& content, const DString& title);
};

} //namespace ui

#endif // UI_UTILS_SYSTEM_UTIL_H_
