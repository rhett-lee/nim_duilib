#ifndef UI_UTILS_SYSTEM_UTIL_H_
#define UI_UTILS_SYSTEM_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
class Window;

/** 系统相关的辅助操作
*/
class UILIB_API SystemUtil
{
public:
    /** 打开URL
    * @param [in] url 需要打开的URL
    */
    static bool OpenUrl(const DString& url);

    /** 显示一个简单的MessageBox
    * @param [in] pWindow 父窗口
    * @param [in] content 需要显示的内容
    * @param [in] title 标题
    */
    static bool ShowMessageBox(const Window* pWindow, const DString& content, const DString& title);
};

} //namespace ui

#endif // UI_UTILS_SYSTEM_UTIL_H_
