#ifndef UI_UTILS_CLIPBOARD_H_
#define UI_UTILS_CLIPBOARD_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 剪切板操作
*/
class UILIB_API Clipboard
{
public:
    /** 获取粘贴板字符串（UTF16）
    */
    static bool GetClipboardText(DStringW& text);

    /** 获取粘贴板字符串（UTF8）
    */
    static bool GetClipboardText(DStringA& text);

    /** 设置剪贴板数据（UTF16）
    */
    static bool SetClipboardText(const DStringW& text);

    /** 设置剪贴板数据（UTF8）
    */
    static bool SetClipboardText(const DStringA& text);
};

} //namespace ui

#endif // UI_UTILS_CLIPBOARD_H_
