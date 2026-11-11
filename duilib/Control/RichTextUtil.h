#ifndef UI_CONTROL_RICHTEXT_UTIL_H_
#define UI_CONTROL_RICHTEXT_UTIL_H_

#include "duilib/duilib_defs.h"

#include <string>
#include <string_view>
#include <vector>

namespace ui
{
/** RichText 文本的Trim策略
*/
enum class RichTextTrimPolicy: int8_t
{
    kNone    = 0, //不处理
    kAll     = 1, //去掉多余的空格，但保留跨文本节点的词间空格
    kKeepOne = 2, //去掉多余的空格，只保留一个空格
};

/** RichText 的 text_align 属性解析结果
*/
struct RichTextTextAlign
{
    /** 水平对齐方式：-1表示未设置，0左对齐，1居中，2右对齐
    */
    int8_t m_nHAlign = -1;

    /** 垂直对齐方式：-1表示未设置，0顶部，1居中，2底部
    */
    int8_t m_nVAlign = -1;

    /** 是否两端对齐（justify）
    */
    bool m_bJustify = false;
};

/** 转义XML文本内容中的特殊字符（& < >）
*/
DUILIB_API DString EscapeRichTextXmlText(const DString& text);

/** 转义XML属性值中的特殊字符（& < > "）
*/
DUILIB_API DString EscapeRichTextXmlAttribute(const DString& text);

/** 判断以逗号分隔的属性值中，是否包含指定的取值（忽略每个取值两侧的空格）
*/
DUILIB_API bool ContainsRichTextAlignValue(const DString& text, const DString::value_type* value);

/** 解析 text_align 属性
* @param [in] text 属性值，可用逗号分隔多个取值，如 "left,vcenter"、"justify"
*            水平方向支持 left/hcenter/center/right，垂直方向支持 top/vcenter/bottom，
*            另外支持 justify（两端对齐）；同时出现时按 left > hcenter > right 的优先级取值
*/
DUILIB_API RichTextTextAlign ParseRichTextTextAlign(const DString& text);

/** 判断标签名称是否为RichText支持的标签（忽略大小写）
*/
DUILIB_API bool IsSupportedRichTextTagName(const DString& tagName);

/** 检查文本中出现的所有标签是否都是RichText支持的标签
* @param [in] text 替换花括号后的格式化文本
* @return 返回true表示所有标签都受支持
*/
DUILIB_API bool IsSupportedRichTextTags(const DString& text);

/** 判断标签是否为块级标签（块级标签需要独占一段）
*/
DUILIB_API bool IsRichTextBlockTagName(const DString& tagName);

/** 文本是否包含可见内容（非纯空白）
*/
DUILIB_API bool HasRichTextVisibleText(const DString& text);

/** 解析 <p>/<h1>~<h6> 的 align 属性
* @return 返回-1表示未设置，0左对齐，1居中，2右对齐，3两端对齐
*/
DUILIB_API int8_t ParseRichTextHAlignValue(const DString& alignValue);

/** 归一化链接地址：去掉协议前缀（http://、https://、www.）与结尾的斜杠，并转为小写
* 说明：仅用于比较链接的显示文本与URL是否相同，不用于实际访问
*/
DUILIB_API DString NormalizeRichTextLinkUrl(const DString& url);

/** 链接的显示文本与URL是否实质不同
* 显示文本与URL一致时（如 www.baidu.com 显示为 www.baidu.com），展示URL提示没有意义
* @param [in] linkText 链接的显示文本（UTF16编码）
* @param [in] linkUrl 链接的URL
* @return 返回true表示两者不同，需要展示提示
*/
DUILIB_API bool IsRichTextLinkUrlDifferentFromText(const std::wstring_view& linkText, const DString& linkUrl);

/** 按Trim策略处理单个文本片段的空白
* @param [in] text 待处理的文本
* @param [in] policy Trim策略
* @return 返回处理后的文本
*/
DUILIB_API DString TrimRichText(const DString& text, RichTextTrimPolicy policy);

/** 跨文本节点的空格处理：保留词间的一个空格，去掉整段文本开头和结尾的空格
* @param [in,out] textViews 各文本片段的文本内容（按文本顺序排列），处理时只调整视图的范围
*/
DUILIB_API void TrimRichTextSlices(std::vector<std::wstring_view>& textViews);

/** 归一化选中的文本：去掉回车符，换行符统一转换为"\r\n"（Windows剪贴板格式）
*/
DUILIB_API std::wstring NormalizeRichTextSelectedText(const std::wstring_view& text);

}//namespace ui

#endif //UI_CONTROL_RICHTEXT_UTIL_H_
