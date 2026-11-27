#ifndef UI_UTILS_CLIPBOARD_H_
#define UI_UTILS_CLIPBOARD_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 剪切板操作（支持 UTF-16 与 UTF-8 两种文本格式）
*   对系统剪切板中的纯文本进行读取与写入。
*/
class DUILIB_API Clipboard
{
public:
    /** 获取粘贴板字符串（UTF-16）
    * @param [out] text 返回读取到的文本；读取失败时被清空
    * @return 读取成功返回true；剪切板无法打开或无文本时返回false
    */
    static bool GetClipboardText(DStringW& text);

    /** 获取粘贴板字符串（UTF-8）
    * @param [out] text 返回读取到的文本；读取失败时被清空
    * @return 读取成功返回true；剪切板无法打开或无文本时返回false
    */
    static bool GetClipboardText(DStringA& text);

    /** 设置剪贴板数据（UTF-16）
    * @param [in] text 要写入的文本
    * @return 写入成功返回true；剪切板无法打开或写入失败时返回false
    */
    static bool SetClipboardText(const DStringW& text);

    /** 设置剪贴板数据（UTF-8）
    * @param [in] text 要写入的文本（UTF-8编码）
    * @return 写入成功返回true；转换为UTF-16失败或写入失败时返回false
    */
    static bool SetClipboardText(const DStringA& text);
};

} //namespace ui

#endif // UI_UTILS_CLIPBOARD_H_
