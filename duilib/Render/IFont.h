#ifndef UI_RENDER_IFONT_H_
#define UI_RENDER_IFONT_H_

#pragma once

#include "duilib/duilib_defs.h"

namespace ui 
{

/** 字体接口的封装
*/
class UILIB_API IFont
{
public:
    virtual ~IFont() = default;

    /**@brief 获取字体名
     */
    virtual const wchar_t* FontName() const = 0;

    /**@brief 获取字体大小(字体高度)
     */
    virtual int FontSize() const = 0;

    /**@brief 获取字体宽度
     */
    virtual int FontWidth() const = 0;

    /**@brief 获取字体Weight值
     */
    virtual int FontWeight() const = 0;

    /**@brief 是否为粗体
     */
    virtual bool IsBold() const = 0;

    /**@brief 字体下划线状态
     */
    virtual bool IsUnderline() const = 0;

    /**@brief 字体的斜体状态
     */
    virtual bool IsItalic() const = 0;

    /**@brief 字体的删除线状态
     */
    virtual bool IsStrikeOut() const = 0;
};

/** 字体接口的基本数据结构
*/
class UILIB_API UiFont
{
public:
    UiFont():
        m_fontSize(0),
        m_bBold(false),
        m_bUnderline(false),
        m_bItalic(false),
        m_bStrikeOut(false)
    {}

    /** 字体名称
    */
    std::wstring m_fontName;

    /** 字体大小（单位：像素）
    */
    int32_t m_fontSize;

    /** 是否为粗体
     */
    bool m_bBold;

    /** 字体下划线状态
     */
    bool m_bUnderline;

    /** 字体的斜体状态
     */
    bool m_bItalic;

    /** 字体的删除线状态
     */
    bool m_bStrikeOut;
};

} // namespace ui

#endif // UI_RENDER_IFONT_H_
