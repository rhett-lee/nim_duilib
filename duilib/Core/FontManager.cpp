#include "FontManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/VersionHelpers.h"
#include "duilib/Render/IRender.h"

namespace ui 
{

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
	RemoveAllFonts();
	RemoveAllFontFiles();
}

bool FontManager::AddFont(const std::wstring& strFontId,
						  const UiFont& fontInfo,
						  bool bDefault)
{
	ASSERT(!strFontId.empty());
	if (strFontId.empty()) {
		return false;
	}

	auto iter = m_fontMap.find(strFontId);
	ASSERT(iter == m_fontMap.end());
	if (iter != m_fontMap.end()) {
		return false;
	}

	static bool bOsOverXp = IsWindowsVistaOrGreater();
	std::wstring fontName = fontInfo.m_fontName;
	if (fontName.empty() || (fontName == L"system")) {
		//字体使用英文名称，保持兼容性
		fontName = bOsOverXp ? L"Microsoft YaHei" : L"SimSun";
	}

	IFont* pFont = nullptr;
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	if (pRenderFactory != nullptr) {
		pFont = pRenderFactory->CreateIFont();
	}
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return false;
	}
	UiFont fontInfoNew = fontInfo;
	fontInfoNew.m_fontName = fontName;
	bool isInitOk = pFont->InitFont(fontInfoNew);
	ASSERT(isInitOk);
	if (!isInitOk) {
		delete pFont;
		pFont = nullptr;
		return false;
	}
	m_fontMap.insert(std::make_pair(strFontId, pFont));
	if (bDefault) {
		m_defaultFontId = strFontId;
	}
	return true;
}

IFont* FontManager::GetIFont(const std::wstring& strFontId)
{
	auto iter = m_fontMap.find(strFontId);
	if (iter == m_fontMap.end()) {
		//如果找不到，则用默认字体
		if (!m_defaultFontId.empty()) {
			iter = m_fontMap.find(m_defaultFontId);
		}		
	}
	IFont* pFont = nullptr;
	if (iter != m_fontMap.end()) {
		pFont = iter->second;
	}
	return pFont;
}

void FontManager::RemoveAllFonts()
{
	for (auto fontInfo : m_fontMap) {
		IFont* pFont = fontInfo.second;
		if (pFont != nullptr) {
			delete pFont;
		}
	}
	m_fontMap.clear();
	m_defaultFontId.clear();
}

void FontManager::AddFontFile(const std::wstring& strFontFile, const std::wstring& strFontName)
{
	FontFileInfo fontFileInfo;
	std::wstring path = GlobalManager::Instance().GetResFullPath(L"", L"font\\" + strFontFile);
	bool res = false;
	if (GlobalManager::Instance().Zip().IsUseZip()) {
		std::vector<unsigned char> file_data;
		if (GlobalManager::Instance().Zip().GetZipData(path, file_data)) {
			DWORD dwFonts = 0;
			fontFileInfo.m_hFontFile = ::AddFontMemResourceEx(file_data.data(), static_cast<DWORD>(file_data.size()), NULL, &dwFonts);
			res = fontFileInfo.m_hFontFile != nullptr;
		}
	}
	else {
		res = ::AddFontResourceEx(path.c_str(), FR_PRIVATE, 0) != 0;
	}
	ASSERT(res);
	if (res) {
		fontFileInfo.m_fontFilePath = path;
		fontFileInfo.m_fontName = strFontName;
		m_fontFileInfo.push_back(fontFileInfo);
	}
}

void FontManager::RemoveAllFontFiles()
{
	for (const auto& it : m_fontFileInfo) {
		if (it.m_hFontFile != nullptr) {
			::RemoveFontMemResourceEx(it.m_hFontFile);
		}
		else if(!it.m_fontFilePath.empty()){
			::RemoveFontResourceEx(it.m_fontFilePath.c_str(), FR_PRIVATE, 0);
		}
	}
	m_fontFileInfo.clear();
}

}