#include "ZipManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PathUtil.h"
#include "duilib/third_party/unzip/UnZip.h"

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


bool ZipManager::IsUseZip() const
{
    return m_hzip != nullptr;
}

#ifdef DUILIB_PLATFORM_WIN

bool ZipManager::OpenResZip(HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const std::string& password)
{
    HRSRC rsc = ::FindResource(hModule, resourceName, resourceType);
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
#endif

bool ZipManager::OpenZipFile(const DString& path, const std::string& password)
{
    CloseResZip();
    m_hzip = OpenZip(path.c_str(), password.c_str());
    return m_hzip != nullptr;
}

bool ZipManager::GetZipData(const DString& path, std::vector<unsigned char>& file_data) const
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(m_hzip != nullptr);
    if (m_hzip == nullptr) {
        return false;
    }
    DString file_path = GetZipFilePath(path);
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

DString ZipManager::GetZipFilePath(const DString& path) const
{
    if (!PathUtil::IsRelativePath(path)) {
        return _T("");
    }
    DString file_path = path;
    StringUtil::ReplaceAll(_T("\\"), _T("/"), file_path);
    StringUtil::ReplaceAll(_T("//"), _T("/"), file_path);
    for (size_t i = 0; i < file_path.size();)
    {
        bool start_node = false;
        if (i == 0 || file_path.at(i - 1) == _T('/')) {
            start_node = true;
        }
        WCHAR wch = file_path.at(i);
        if (start_node && wch == _T('/')) {//"//"
            file_path.erase(i, 1);
            continue;
        }
        if (start_node && wch == _T('.')) {
            if (i + 1 < file_path.size() && file_path.at(i + 1) == _T('/')) {// "./"
                file_path.erase(i, 2);
                continue;
            }
            else if (i + 2 < file_path.size() && file_path.at(i + 1) == _T('.') && file_path.at(i + 2) == _T('/')) {// "../"
                file_path.erase(i, 2);
                int i_erase = (int)i - 2;
                if (i_erase < 0){
                    ASSERT(0);
                }
                while (i_erase > 0 && file_path.at(i_erase) != _T('/')) {
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

bool ZipManager::IsZipResExist(const DString& path) const
{
    GlobalManager::Instance().AssertUIThread();
    if ((m_hzip != nullptr) && !path.empty()) {
        DString file_path = GetZipFilePath(path);
        if (file_path.empty()) {
            return false;
        }
        auto it = m_zipPathCache.find(path);
        if (it != m_zipPathCache.end()) {
            return true;
        }

        ZIPENTRY ze = { 0 };
        int i = 0;
        bool find = FindZipItem((HZIP)m_hzip, file_path.c_str(), true, &i, &ze) == ZR_OK;
        if (find) {
            m_zipPathCache.insert(path);
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
    m_zipPathCache.clear();
}

bool ZipManager::GetZipFileList(const DString& path, std::vector<DString>& fileList) const
{
    GlobalManager::Instance().AssertUIThread();
    if ((m_hzip != nullptr) && !path.empty()) {
        DString innerPath = GetZipFilePath(path);
        if (innerPath.empty()) {
            return false;
        }
        ZIPENTRY ze = {0, };
        if (GetZipItem((HZIP)m_hzip, -1, &ze) != ZR_OK) {
            ze.index = 0;
        }
        DString fileName;
        int32_t numitems = ze.index;
        for (int32_t i = 0; i < numitems; i++) {
            if (GetZipItem((HZIP)m_hzip, i, &ze) != ZR_OK) {
                break;
            }
            fileName = ze.name;
            size_t nPos = fileName.find(innerPath);
            if ((nPos == 0) && (fileName.size() > innerPath.size())) {
                fileList.push_back(fileName.substr(innerPath.size()));
            }
        }
        return true;
    }
    return false;
}

}

