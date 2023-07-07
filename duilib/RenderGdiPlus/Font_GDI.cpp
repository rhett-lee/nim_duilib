#include "Font_GDI.h"

namespace ui 
{

Font_GDI::Font_GDI() :
    m_logFont{ 0, }, 
    m_hFont(nullptr) 
{
}

Font_GDI::~Font_GDI()
{
    if (m_hFont != nullptr) {
        ::DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
}

bool Font_GDI::InitFont(const UiFont& fontInfo)
{
	ASSERT(!fontInfo.m_fontName.empty());
	if (fontInfo.m_fontName.empty()) {
		return false;
	}
	LOGFONT lf = { 0 };
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	wcscpy_s(lf.lfFaceName, fontInfo.m_fontName.c_str());
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = -std::abs(fontInfo.m_fontSize);
	if (fontInfo.m_bUnderline) {
		lf.lfUnderline = TRUE;
	}
	if (fontInfo.m_bStrikeOut) {
		lf.lfStrikeOut = TRUE;
	}
	if (fontInfo.m_bItalic) {
		lf.lfItalic = TRUE;
	}
	if (fontInfo.m_bBold) {
		lf.lfWeight = FW_BOLD;
	}
	m_logFont = lf;
	ASSERT(m_hFont == nullptr);
	if (m_hFont != nullptr) {
		::DeleteObject(m_hFont);
		m_hFont = nullptr;
	}	
	return true;
}

HFONT Font_GDI::GetFontHandle()
{
	if (m_hFont != nullptr) {
		return m_hFont;
	}
	m_hFont = ::CreateFontIndirect(&m_logFont);
	ASSERT(m_hFont != nullptr);
	return m_hFont; 
}

} // namespace ui

