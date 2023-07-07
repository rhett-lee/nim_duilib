#include "Font_Skia.h"
#include "duilib/Utils/StringUtil.h"
#include "include/core/SkFont.h"

namespace ui 
{

Font_Skia::Font_Skia() :
	m_skFont(nullptr)
{
}

Font_Skia::~Font_Skia()
{
    if (m_skFont != nullptr) {
		delete m_skFont;
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
	ASSERT(m_skFont == nullptr);
	if (m_skFont != nullptr) {
		delete m_skFont;
		m_skFont = nullptr;
	}
	return true;
}

const SkFont* Font_Skia::GetFontHandle()
{
	if (m_skFont != nullptr) {
		return m_skFont;
	}
	ASSERT(!m_uiFont.m_fontName.empty());
	if (m_uiFont.m_fontName.empty()) {
		return nullptr;
	}

	SkFontStyle fontStyle = SkFontStyle::Normal();
	if (m_uiFont.m_bBold && m_uiFont.m_bItalic) {
		fontStyle = SkFontStyle::BoldItalic();
	}
	else if (m_uiFont.m_bBold) {
		fontStyle = SkFontStyle::Bold();
	}
	else if (m_uiFont.m_bItalic) {
		fontStyle = SkFontStyle::Italic();
	}
	std::string fontName; //UTF8±àÂëµÄ×ÖÌåÃû³Æ
	StringHelper::UnicodeToMBCS(m_uiFont.m_fontName, fontName, CP_UTF8);
	sk_sp<SkTypeface> spTypeface = SkTypeface::MakeFromName(fontName.c_str(), fontStyle);
	ASSERT(spTypeface != nullptr);
	if (spTypeface == nullptr) {
		return nullptr;
	}
	m_skFont = new SkFont();
	m_skFont->setTypeface(spTypeface);
	m_skFont->setSize(SkIntToScalar(std::abs(m_uiFont.m_fontSize)));
	m_skFont->setEdging(SkFont::Edging::kSubpixelAntiAlias);
	return m_skFont;
}

} // namespace ui

