#ifndef UI_RENDER_SKIA_FONT_H_
#define UI_RENDER_SKIA_FONT_H_

#include "duilib/Render/IRender.h"

class SkFont;
class SkFontMgr;

namespace ui 
{
class FontMgr_Skia;

/** Skia字体接口的实现
*/
class Font_Skia: public IFont
{
public:
    explicit Font_Skia(std::shared_ptr<IFontMgr>& spFontMgr);
    Font_Skia(const Font_Skia&) = delete;
    Font_Skia& operator=(const Font_Skia&) = delete;
    virtual ~Font_Skia() override;

    /** 设置字体信息(内部未对字体大小做DPI自适应)
    * @note 该方法只保存 UiFont 并清理旧的 SkFont，实际 SkFont 的创建延迟到首次调用 GetFontHandle()
    */
    virtual bool InitFont(const UiFont& fontInfo) override;

    /**@brief 获取字体名
     */
    virtual DString FontName() const override { return m_uiFont.m_fontName.c_str(); }

    /**@brief 获取字体大小
     */
    virtual int32_t FontSize() const override { return m_uiFont.m_fontSize; }

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

    /** 当前字体是否支持指定的Unicode字符
    * @param [in] unicodeChar UTF32字符
    * @return 支持该字符返回true，否则返回false
    */
    virtual bool IsUnicodeCharSupported(uint32_t unicodeChar) const override;

public:
    /** 获取字体句柄（懒加载：首次调用时创建Skia字体对象）
    */
    const SkFont* GetFontHandle() const;

    /** 获取字体管理器接口
    */
    virtual IFontMgr* GetFontMgr() const override;

    /** 获取字体签名（由字体名、字号与字体样式计算，在InitFont时计算一次）
    */
    virtual uint64_t GetFontKey() const override;

private:
    /** 删除Skia字体
    */
    void ClearSkFont();

private:
    //字体信息
    UiFont m_uiFont;

    //字体句柄（懒加载，const接口中也可创建，故声明为mutable）
    mutable SkFont* m_skFont;

    //字体管理器
    std::shared_ptr<IFontMgr> m_spFontMgr;

    //字体签名（用于字体相关的缓存，避免重复计算）
    uint64_t m_fontKey;

    //Skia字体管理器（缓存，避免重复的dynamic_cast）
    FontMgr_Skia* m_pSkiaFontMgr;
};

} // namespace ui

#endif // UI_RENDER_SKIA_FONT_H_
