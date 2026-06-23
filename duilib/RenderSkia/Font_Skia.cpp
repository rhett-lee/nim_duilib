#include "Font_Skia.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkFont.h"
#include "SkiaHeaderEnd.h"

namespace ui 
{

Font_Skia::Font_Skia(std::shared_ptr<IFontMgr>& spFontMgr):
    m_skFont(nullptr),
    m_spFontMgr(spFontMgr)
{
    ASSERT(m_spFontMgr != nullptr);
}

Font_Skia::~Font_Skia()
{
    ClearSkFont();
    m_spFontMgr.reset();
}

void Font_Skia::ClearSkFont()
{
    if (m_skFont != nullptr) {
        FontMgr_Skia* pSkiaFontMgr = dynamic_cast<FontMgr_Skia*>(m_spFontMgr.get());
        ASSERT(pSkiaFontMgr != nullptr);
        if (pSkiaFontMgr != nullptr) {
            pSkiaFontMgr->DeleteSkFont(m_skFont);
        }
        m_skFont = nullptr;
    }
}

bool Font_Skia::InitFont(const UiFont& fontInfo)
{
    ASSERT(!fontInfo.m_fontName.empty());
    if (fontInfo.m_fontName.empty()) {
        return false;
    }
    // 仅更新字体信息并清理旧的 SkFont 实例
    // 实际的 SkFont 创建会延迟到首次调用 GetFontHandle() 时（懒加载模式）
    m_uiFont = fontInfo;
    ClearSkFont();
    return true;
}

const SkFont* Font_Skia::GetFontHandle()
{
    if (m_skFont != nullptr) {
        return m_skFont;
    }
    FontMgr_Skia* pSkiaFontMgr = dynamic_cast<FontMgr_Skia*>(m_spFontMgr.get());
    ASSERT(pSkiaFontMgr != nullptr);
    if (pSkiaFontMgr != nullptr) {
        m_skFont = pSkiaFontMgr->CreateSkFont(m_uiFont);
    }
    return m_skFont;
}

bool Font_Skia::IsUnicodeCharSupported(uint32_t unicodeChar, uint16_t* glyphId)
{
    if (unicodeChar != 0) {
        const SkFont* pSkFont = GetFontHandle();
        if (pSkFont != nullptr) {
            ASSERT(sizeof(SkGlyphID) == sizeof(uint16_t));
            ASSERT(sizeof(SkUnichar) == sizeof(uint32_t));
            SkGlyphID glyph = pSkFont->unicharToGlyph((SkUnichar)unicodeChar);
            if (glyph != 0) {
                if (glyphId) {
                    *glyphId = glyph;
                }
                return true;
            }
        }
    }
    return false;
}

IFontMgr* Font_Skia::GetFontMgr() const
{
    return m_spFontMgr.get();
}

} // namespace ui

