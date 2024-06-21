#include "Font_Skia.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"

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

} // namespace ui

