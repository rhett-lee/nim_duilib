#include "FontManager.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{

FontManager* FontManager::GetInstance()
{
	static FontManager fontManager;
	return &fontManager;
}

void FontManager::AddFontResource(const std::wstring& strFontFile, const std::wstring& strFontName)
{
	std::wstring path = GlobalManager::GetResPath(L"font\\" + strFontFile, L"");
	bool res = false;
	if (GlobalManager::IsUseZip()) {
		HGLOBAL hGlobal = GlobalManager::GetZipData(path);
		if (hGlobal) {
			void* data = GlobalLock(hGlobal);
			SIZE_T len = GlobalSize(hGlobal);
			DWORD dwFonts = 0;
			HANDLE handle = nullptr;
			if (data != nullptr) {
				handle = AddFontMemResourceEx(data, static_cast<DWORD>(len), NULL, &dwFonts);
			}
			res = handle != nullptr;
			if (res) {
				m_fontHandles.push_back(handle);
			}

			GlobalUnlock(hGlobal);
			GlobalFree(hGlobal);
		}
	}
	else {
		res = AddFontResourceEx(path.c_str(), FR_PRIVATE, 0) != 0;
	}
	ASSERT(res);
	if (res) {
		m_fontPath.push_back(path);
		m_fontNames.push_back(strFontName);
	}
}

const std::vector<std::wstring>& FontManager::GetFontNames()
{
	return m_fontNames;
}

void FontManager::Clear()
{
	for (const auto &it : m_fontHandles) {
		RemoveFontMemResourceEx(it);
	}
	for (const auto &it : m_fontPath) {
		RemoveFontResourceEx(it.c_str(), FR_PRIVATE, 0);
	}
	m_fontHandles.clear();
	m_fontPath.clear();
	m_fontNames.clear();
}

FontManager::~FontManager()
{
	Clear();
}

}