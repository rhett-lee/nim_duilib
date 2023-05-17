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
	std::wstring path = GlobalManager::GetResFullPath(L"", L"font\\" + strFontFile);
	bool res = false;
	if (GlobalManager::IsUseZip()) {
		std::vector<unsigned char> file_data;
		if (GlobalManager::GetZipData(path, file_data)) {
			DWORD dwFonts = 0;
			HANDLE handle = AddFontMemResourceEx(file_data.data(), static_cast<DWORD>(file_data.size()), NULL, &dwFonts);
			res = handle != nullptr;
			if (res) {
				m_fontHandles.push_back(handle);
			}
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