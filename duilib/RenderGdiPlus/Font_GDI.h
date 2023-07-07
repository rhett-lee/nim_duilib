#ifndef UI_RENDER_GDIPLUS_FONT_GDI_H_
#define UI_RENDER_GDIPLUS_FONT_GDI_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

/** GDI字体接口的实现
*/
class UILIB_API Font_GDI: public IFont
{
public:
    Font_GDI();
    Font_GDI(const Font_GDI&) = delete;
    Font_GDI& operator=(const Font_GDI&) = delete;
    virtual ~Font_GDI();

    /** 初始化字体(内部未对字体大小做DPI自适应)
    */
    virtual bool InitFont(const UiFont& fontInfo) override;

    /**@brief 获取字体名
     */
    virtual const wchar_t* FontName() const override { return m_logFont.lfFaceName; }

    /**@brief 获取字体大小
     */
    virtual int FontSize() const override { return -m_logFont.lfHeight; }

    /**@brief 是否为粗体
     */
    virtual bool IsBold() const override { return m_logFont.lfWeight >= FW_BOLD; }

    /**@brief 字体下划线状态
     */
    virtual bool IsUnderline() const override { return m_logFont.lfUnderline; }

    /**@brief 字体的斜体状态
     */
    virtual bool IsItalic() const override { return m_logFont.lfItalic; }

    /**@brief 字体的删除线状态
     */
    virtual bool IsStrikeOut() const override { return m_logFont.lfStrikeOut; }

public:
    /** 获取字体句柄
    */
    HFONT GetFontHandle();

private:
    //字体信息
    LOGFONT m_logFont;

    //字体句柄
    HFONT m_hFont;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_FONT_GDI_H_
