#ifndef UI_CONTROL_RICHTEXT_H_
#define UI_CONTROL_RICHTEXT_H_

#pragma once

#include "duilib/Core/Control.h"

namespace ui 
{
class RichTextSlice
{
public:
    RichTextSlice();
    
    /** 节点名称
    */
    std::wstring m_nodeName;

    /** 文字内容
    */
    std::wstring m_text;

    /** 超链接的URL: "href"
    */
    std::wstring m_linkUrl;

    /** 文字颜色: "color"
    */
    std::wstring m_textColor;

    /** 背景颜色: "color"
    */
    std::wstring m_bgColor;

    /** 字体信息
    */
    UiFont m_fontInfo;

    /** 对象绘制区域
    */
    //UiRect m_textRect;

public:
    /** 子节点
    */
    std::vector<RichTextSlice> m_childs;
};

class UILIB_API RichText : public Control
{
public:
    RichText();
    RichText(const RichText& r) = delete;
    RichText& operator=(const RichText& r) = delete;
    virtual ~RichText();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void PaintText(IRender* pRender) override;
    virtual void SetPos(UiRect rc) override;

    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiSize EstimateText(UiSize szAvailable);

public:
    /** 获取文字内边距
     */
    UiPadding GetTextPadding() const;

    /** 设置文字内边距信息, 函数内部会进行DPI自适应操作
     * @param [in] padding 矩形的四个边值分别代表对应的四个内边距值
     * @param [in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale = true);

    /** 设置格式的文本
    * @param [in] richText 带有格式的文本内容
    */
    bool SetRichText(const std::wstring& richText);

    /** 追加一个文本片段
    */
    void AppendTextSlice(const RichTextSlice&& textSlice);

    /** 追加一个文本片段
    */
    void AppendTextSlice(const RichTextSlice& textSlice);

public:
    /** 输出带格式化文本
    */
    std::wstring ToString() const;

private:
    /** 输出带格式化文本
    */
    std::wstring ToString(const RichTextSlice& textSlice, const std::wstring& indent) const;

private:
    /** 文本绘制的内边距(分别对应四个边的内边距大小)
    */
    UiPadding m_rcTextPadding;

    /** 绘制的文本内容
    */
    std::vector<RichTextSlice> m_textSlice;
};

} // namespace ui

#endif // UI_CONTROL_RICHTEXT_H_
