#include "FileDialog.h"
#include "duilib/Core/Window.h"
#include <shlobj.h>

namespace ui
{
bool FileDialog::BrowseForFolder(Window* pWindow, std::wstring& folderPath)
{
    folderPath.clear();
    IFileDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM;
        pfd->SetOptions(fos);
        hr = pfd->Show((pWindow != nullptr) ? pWindow->GetHWND() : nullptr);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pfd->GetResult(&pItem);
            if (SUCCEEDED(hr) && (pItem != nullptr)) {
                LPWSTR pName = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pName);
                if (SUCCEEDED(hr) && (pName != nullptr)) {
                    folderPath = pName;
                    ::CoTaskMemFree(pName);
                    pName = nullptr;
                }
                pItem->Release();
            }
        }
        pfd->Release();
    }
    return !folderPath.empty();
}

bool FileDialog::BrowseForFolders(Window* pWindow, std::vector<std::wstring>& folderPaths)
{
    folderPaths.clear();
    IFileOpenDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT;
        pfd->SetOptions(fos);
        hr = pfd->Show((pWindow != nullptr) ? pWindow->GetHWND() : nullptr);
        if (SUCCEEDED(hr)) {
            IShellItemArray* pSelResultArray = nullptr;
            hr = pfd->GetResults(&pSelResultArray);
            if (SUCCEEDED(hr) && (pSelResultArray != nullptr)) {
                DWORD dwNumItems = 0;
                hr = pSelResultArray->GetCount(&dwNumItems);
                if (!SUCCEEDED(hr)) {
                    dwNumItems = 0;
                }
                for (DWORD i = 0; i < dwNumItems; i++) {
                    IShellItem* pItem = nullptr;
                    hr = pSelResultArray->GetItemAt(i, &pItem);
                    if (SUCCEEDED(hr) && (pItem != nullptr)) {
                        LPWSTR pName = nullptr;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pName);
                        if (SUCCEEDED(hr) && (pName != nullptr)) {
                            std::wstring folderPath = pName;
                            ::CoTaskMemFree(pName);
                            pName = nullptr;
                            if (!folderPath.empty()) {
                                folderPaths.push_back(folderPath);
                            }
                        }
                        pItem->Release();
                    }
                }                
                pSelResultArray->Release();
            }
        }
        pfd->Release();
    }
    return !folderPaths.empty();
}

bool FileDialog::BrowseForFile(Window* pWindow, 
                               std::wstring& filePath,                               
                               bool bOpenFileDialog,
                               const std::vector<FileType>& fileTypes,
                               int32_t nFileTypeIndex,
                               const std::wstring& defaultExt)
{
    filePath.clear();
    IFileDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = S_OK;
    if (bOpenFileDialog) {
        hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    }
    else {
        hr = ::CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    }
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_FORCEFILESYSTEM;
        pfd->SetOptions(fos);
        if (!fileTypes.empty()) {
            std::vector<COMDLG_FILTERSPEC> filterSpecs;
            for (const FileType& fileType : fileTypes) {
                filterSpecs.push_back({ (LPCWSTR)fileType.szName.c_str(), (LPCWSTR)fileType.szExt.c_str() });
            }
            hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
        }
        if ((nFileTypeIndex >= 0) && (nFileTypeIndex < fileTypes.size())) {
            hr = pfd->SetFileTypeIndex((UINT)nFileTypeIndex + 1); //SetFileTypeIndex的下标是从1开始的
            ASSERT(SUCCEEDED(hr));
        }
        if (!defaultExt.empty()) {
            hr = pfd->SetDefaultExtension(defaultExt.c_str());
            ASSERT(SUCCEEDED(hr));
        }

        if (pfd->Show((pWindow != nullptr) ? pWindow->GetHWND() : nullptr) == S_OK) {
            IShellItem* pItem = nullptr;
            hr = pfd->GetResult(&pItem);
            if (SUCCEEDED(hr) && (pItem != nullptr)) {
                LPWSTR resultptr = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);
                if (SUCCEEDED(hr) && (resultptr != nullptr)) {
                    filePath = resultptr;
                    ::CoTaskMemFree(resultptr);
                    resultptr = nullptr;
                }
                pItem->Release();
            }
        }
        pfd->Release();
    }
    return !filePath.empty();
}

bool FileDialog::BrowseForFiles(Window* pWindow, 
                                std::vector<std::wstring>& filePaths,                                
                                const std::vector<FileType>& fileTypes,
                                int32_t nFileTypeIndex,
                                const std::wstring& defaultExt)
{
    filePaths.clear();
    IFileOpenDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT;
        pfd->SetOptions(fos);
        if (!fileTypes.empty()) {
            std::vector<COMDLG_FILTERSPEC> filterSpecs;
            for (const FileType& fileType : fileTypes) {
                filterSpecs.push_back({ (LPCWSTR)fileType.szName.c_str(), (LPCWSTR)fileType.szExt.c_str() });
            }
            hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
        }
        if ((nFileTypeIndex >= 0) && (nFileTypeIndex < fileTypes.size())) {
            hr = pfd->SetFileTypeIndex((UINT)nFileTypeIndex + 1); //SetFileTypeIndex的下标是从1开始的
            ASSERT(SUCCEEDED(hr));
        }
        if (!defaultExt.empty()) {
            hr = pfd->SetDefaultExtension(defaultExt.c_str());
            ASSERT(SUCCEEDED(hr));
        }
        hr = pfd->Show((pWindow != nullptr) ? pWindow->GetHWND() : nullptr);
        if (SUCCEEDED(hr)) {
            IShellItemArray* pSelResultArray = nullptr;
            hr = pfd->GetResults(&pSelResultArray);
            if (SUCCEEDED(hr) && (pSelResultArray != nullptr)) {
                DWORD dwNumItems = 0;
                hr = pSelResultArray->GetCount(&dwNumItems);
                if (!SUCCEEDED(hr)) {
                    dwNumItems = 0;
                }
                for (DWORD i = 0; i < dwNumItems; i++) {
                    IShellItem* pItem = nullptr;
                    hr = pSelResultArray->GetItemAt(i, &pItem);
                    if (SUCCEEDED(hr) && (pItem != nullptr)) {
                        LPWSTR pName = nullptr;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pName);
                        if (SUCCEEDED(hr) && (pName != nullptr)) {
                            std::wstring folderPath = pName;
                            ::CoTaskMemFree(pName);
                            pName = nullptr;
                            if (!folderPath.empty()) {
                                filePaths.push_back(folderPath);
                            }
                        }
                        pItem->Release();
                    }
                }
                pSelResultArray->Release();
            }
        }
        pfd->Release();
    }
    return !filePaths.empty();
}

}//namespace ui
