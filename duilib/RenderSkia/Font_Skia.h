#ifndef UI_RENDER_SKIA_FONT_GDI_H_
#define UI_RENDER_SKIA_FONT_GDI_H_

#pragma once

#include "duilib/Render/IRender.h"

class SkFont;

namespace ui 
{

/** Skia字体接口的实现
*/
class UILIB_API Font_Skia: public IFont
{
public:
    explicit Font_Skia(IRenderFactory* pRenderFactory);
    Font_Skia(const Font_Skia&) = delete;
    Font_Skia& operator=(const Font_Skia&) = delete;
    virtual ~Font_Skia();

    /** 初始化字体(内部未对字体大小做DPI自适应)
    */
    virtual bool InitFont(const UiFont& fontInfo) override;

    /**@brief 获取字体名
     */
    virtual const wchar_t* FontName() const override { return m_uiFont.m_fontName.c_str(); }

    /**@brief 获取字体大小
     */
    virtual int FontSize() const override { return m_uiFont.m_fontSize; }

    /**@brief 是否为粗体
     */
    virtual bool IsBold() const override { return m_uiFont.m_bBold; }

    /**@brief 字体下划线状态
     */
    virtual bool IsUnderline() const override { return m_uiFont.m_bUnderline; }

    /**@brief 字体的斜体状态
     */
    virtual bool IsItalic() const override { return m_uiFont.m_bItalic; }

    /**@brief 字体的删除线状态
     */
    virtual bool IsStrikeOut() const override { return m_uiFont.m_bStrikeOut; }

public:
    /** 获取字体句柄
    */
    const SkFont* GetFontHandle();

private:
    //字体信息
    UiFont m_uiFont;

    //字体句柄
    SkFont* m_skFont;

    //渲染接口
    IRenderFactory* m_pRenderFactory;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_FONT_GDI_H_
