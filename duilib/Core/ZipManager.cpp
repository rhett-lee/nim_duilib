#include "ZipManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"

#include "duilib/third_party/unzip/UnZip.h"

#include <unordered_set>

namespace ui 
{

ZipManager::ZipManager():
    m_hzip(nullptr)
{
}

ZipManager::~ZipManager()
{
	CloseResZip();
}


bool ZipManager::IsUseZip()
{
	return m_hzip != nullptr;
}

bool ZipManager::OpenResZip(LPCTSTR resourceName, LPCTSTR resourceType, const std::string& password)
{
	HRSRC rsc = ::FindResource(NULL, resourceName, resourceType);
	ASSERT(rsc != nullptr);
	if (rsc == nullptr) {
		return false;
	}
	HGLOBAL resource = ::LoadResource(NULL, rsc);
	DWORD size = ::SizeofResource(NULL, rsc);
	ASSERT((resource != nullptr) && (size > 0));
	if ((resource == nullptr) || (size == 0)) {
		return false;
	}
	CloseResZip();
	m_hzip = OpenZip(resource, size, password.c_str());
	return m_hzip != nullptr;
}

bool ZipManager::OpenResZip(const std::wstring& path, const std::string& password)
{
	CloseResZip();
	m_hzip = OpenZip(path.c_str(), password.c_str());
	return m_hzip != nullptr;
}

bool ZipManager::GetZipData(const std::wstring& path, std::vector<unsigned char>& file_data)
{
	GlobalManager::Instance().AssertUIThread();
	ASSERT(m_hzip != nullptr);
	if (m_hzip == nullptr) {
		return false;
	}
	std::wstring file_path = GetZipFilePath(path);
	ASSERT(!file_path.empty());
	if (file_path.empty()) {
		return false;
	}

	ZIPENTRY ze = { 0 };
	int i = 0;
	if (FindZipItem((HZIP)m_hzip, file_path.c_str(), true, &i, &ze) == ZR_OK) {
		if ((ze.index >= 0) && (ze.unc_size > 0)) {
			file_data.resize(ze.unc_size);
			ZRESULT res = UnzipItem((HZIP)m_hzip, ze.index, file_data.data(), ze.unc_size);
			if (res == ZR_OK) {
				return true;
			}
			else {
				file_data.clear();
			}
		}
	}
	return false;
}

std::wstring ZipManager::GetZipFilePath(const std::wstring& path)
{
	if (!StringHelper::IsRelativePath(path)) {
		return L"";
	}
	std::wstring file_path = path;
	StringHelper::ReplaceAll(L"\\", L"/", file_path);
	StringHelper::ReplaceAll(L"//", L"/", file_path);
	for (size_t i = 0; i < file_path.size();)
	{
		bool start_node = false;
		if (i == 0 || file_path.at(i - 1) == L'/') {
			start_node = true;
		}
		WCHAR wch = file_path.at(i);
		if (start_node && wch == L'/') {//"//"
			file_path.erase(i, 1);
			continue;
		}
		if (start_node && wch == L'.') {
			if (i + 1 < file_path.size() && file_path.at(i + 1) == L'/') {// "./"
				file_path.erase(i, 2);
				continue;
			}
			else if (i + 2 < file_path.size() && file_path.at(i + 1) == L'.' && file_path.at(i + 2) == L'/') {// "../"
				file_path.erase(i, 2);
				int i_erase = (int)i - 2;
				if (i_erase < 0){
					ASSERT(0);
				}
				while (i_erase > 0 && file_path.at(i_erase) != L'/') {
					i_erase--;
				}
				file_path.erase(i_erase, i - i_erase);
				i = i_erase;
				continue;
			}
		}
		i++;
	}
	return file_path;
}

bool ZipManager::IsZipResExist(const std::wstring& path)
{
	GlobalManager::Instance().AssertUIThread();
	if ((m_hzip != nullptr) && !path.empty()) {
		std::wstring file_path = GetZipFilePath(path);
		if (file_path.empty()) {
			return false;
		}
		static std::unordered_set<std::wstring> zip_path_cache;
		auto it = zip_path_cache.find(path);
		if (it != zip_path_cache.end()) {
			return true;
		}

		ZIPENTRY ze = { 0 };
		int i = 0;
		bool find = FindZipItem((HZIP)m_hzip, file_path.c_str(), true, &i, &ze) == ZR_OK;
		if (find) {
			zip_path_cache.insert(path);
		}
		return find;
	}
	return false;
}

void ZipManager::CloseResZip()
{
	if (m_hzip != nullptr) {
		CloseZip((HZIP)m_hzip);
		m_hzip = nullptr;
	}
}

}

