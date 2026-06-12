#ifndef UI_RENDER_SKIA_SKIA_TEXT_DATA_H_
#define UI_RENDER_SKIA_SKIA_TEXT_DATA_H_

#include "duilib/Render/IRender.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "SkiaHeaderEnd.h"

#include <functional>

class SkFont;

namespace ui
{

/** 枚举字符串的回调函数
* @return 返回 true 继续枚举，返回 false 停止枚举
*/
typedef std::function<bool (SkUnichar unicodeChar, size_t charByteLength)> EnumTextCallback;

/** 文本数据封装类
* @note 封装文本的三个核心参数：(const void* text, size_t byteLength, SkTextEncoding textEncoding)
*       提供便捷的文本遍历和转换功能
*/
class SkiaTextData
{
public:
    /** 默认构造函数
    */
    SkiaTextData();

    /** 构造函数
    * @param [in] text 文本数据
    * @param [in] byteLength 文本长度（字节数）
    * @param [in] textEncoding 文本编码格式
    */
    SkiaTextData(const void* text, size_t byteLength, SkTextEncoding textEncoding);

    /** 设置文本数据
    * @param [in] text 文本数据
    * @param [in] byteLength 文本长度（字节数）
    * @param [in] textEncoding 文本编码格式
    */
    void SetText(const void* text, size_t byteLength, SkTextEncoding textEncoding);

    /** 获取文本数据指针
    * @return 文本数据指针
    */
    const void* GetText() const { return m_text; }

    /** 获取文本长度（字节数）
    * @return 文本长度（字节数）
    */
    size_t GetByteLength() const { return m_byteLength; }

    /** 获取文本编码格式
    * @return 文本编码格式
    */
    SkTextEncoding GetTextEncoding() const { return m_textEncoding; }

    /** 判断文本是否为空
    * @return true：文本为空；false：文本不为空
    */
    bool IsEmpty() const;

    /** 枚举文本中的每个字符
    * @param [in] callback 回调函数，参数依次为：(SkUnichar unicodeChar, size_t charByteLength)
    *                      返回 true 继续枚举，返回 false 停止枚举
    * @return 返回枚举是否成功（即使被callback中断也返回true，只有数据错误才返回false）
    */
    bool EnumChars(EnumTextCallback callback) const;

    /** 转换为UTF32编码的字符串
    * @return UTF32编码的字符串
    */
    UTF32String ToUTF32String() const;

    /** 获取当前字符串起始地址对应的单个字符的字节数
    * @param [in] text 文本起始位置
    * @return 单个字符的字节数（仅适用于UTF8/UTF16编码，UTF32固定返回4）
    */
    static size_t GetCharByteLength(const void* text, SkTextEncoding textEncoding);

private:
    /** 文本数据指针
    */
    const void* m_text;

    /** 文本长度（字节数）
    */
    size_t m_byteLength;

    /** 文本编码格式
    */
    SkTextEncoding m_textEncoding;
};

} // namespace ui

#endif // UI_RENDER_SKIA_SKIA_TEXT_DATA_H_
