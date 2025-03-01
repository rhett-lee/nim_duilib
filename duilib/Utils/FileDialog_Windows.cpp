#include "FileDialog.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include "duilib/duilib_config_windows.h"
#include <shlobj.h>

namespace ui
{
bool FileDialog::BrowseForFolder(Window* pWindow, FilePath& folderPath, const FilePath& defaultLocation)
{
    folderPath.Clear();
    IFileDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    ASSERT(SUCCEEDED(hr));
    if (SUCCEEDED(hr) && (pfd != nullptr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM;
        pfd->SetOptions(fos);

        //设置默认文件夹
        if (!defaultLocation.IsEmpty()) {
            DStringW defaultPath = defaultLocation.ToStringW();
            IShellItem* psi = nullptr;
            hr = SHCreateItemFromParsingName(defaultPath.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hr) && (psi != nullptr)) {
                pfd->SetDefaultFolder(psi);
                psi->Release();
                psi = nullptr;
            }
        }

        hr = pfd->Show((pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pfd->GetResult(&pItem);
            if (SUCCEEDED(hr) && (pItem != nullptr)) {
                LPWSTR pName = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pName);
                if (SUCCEEDED(hr) && (pName != nullptr)) {
                    folderPath = FilePath(pName);
                    ::CoTaskMemFree(pName);
                    pName = nullptr;
                }
                pItem->Release();
            }
        }
        pfd->Release();
    }
    return !folderPath.IsEmpty();
}

bool FileDialog::BrowseForFolders(Window* pWindow, std::vector<FilePath>& folderPaths, const FilePath& defaultLocation)
{
    folderPaths.clear();
    IFileOpenDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    ASSERT(SUCCEEDED(hr));
    if (SUCCEEDED(hr) && (pfd != nullptr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT;
        pfd->SetOptions(fos);

        //设置默认文件夹
        if (!defaultLocation.IsEmpty()) {
            DStringW defaultPath = defaultLocation.ToStringW();
            IShellItem* psi = nullptr;
            hr = SHCreateItemFromParsingName(defaultPath.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hr) && (psi != nullptr)) {
                pfd->SetDefaultFolder(psi);
                psi->Release();
                psi = nullptr;
            }
        }

        hr = pfd->Show((pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr);
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
                            FilePath folderPath(pName);
                            ::CoTaskMemFree(pName);
                            pName = nullptr;
                            if (!folderPath.IsEmpty()) {
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
                               FilePath& filePath,                               
                               bool bOpenFileDialog,
                               const std::vector<FileType>& fileTypes,
                               int32_t nFileTypeIndex,
                               const DString& defaultExt,
                               const DString& fileName,
                               const FilePath& defaultLocation)
{
    filePath.Clear();
    IFileDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = S_OK;
    if (bOpenFileDialog) {
        hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    }
    else {
        hr = ::CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    }
    //过滤类型固定为Unicode字符串
    struct FileTypeW
    {
        DStringW szName;
        DStringW szExt;
    };
    std::vector<FileTypeW> fileTypesW;
    for (const FileType& fileType : fileTypes) {
        fileTypesW.push_back({ StringConvert::TToWString(fileType.szName), StringConvert::TToWString(fileType.szExt) });
    }

    if (SUCCEEDED(hr) && (pfd != nullptr)) {
        FILEOPENDIALOGOPTIONS fos = 0;
        pfd->GetOptions(&fos);
        fos |= FOS_FORCEFILESYSTEM;
        pfd->SetOptions(fos);
        if (!fileTypesW.empty()) {
            std::vector<COMDLG_FILTERSPEC> filterSpecs;
            for (const FileTypeW& fileType : fileTypesW) {
                filterSpecs.push_back({ (LPCWSTR)fileType.szName.c_str(), (LPCWSTR)fileType.szExt.c_str() });
            }
            hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
        }
        if ((nFileTypeIndex >= 0) && (nFileTypeIndex < (int32_t)fileTypesW.size())) {
            hr = pfd->SetFileTypeIndex((UINT)nFileTypeIndex + 1); //SetFileTypeIndex的下标是从1开始的
            ASSERT(SUCCEEDED(hr));
        }
        if (!defaultExt.empty()) {
            hr = pfd->SetDefaultExtension(StringConvert::TToWString(defaultExt).c_str());
            ASSERT(SUCCEEDED(hr));
        }

        DStringW fileNameW = StringConvert::TToWString(fileName);
        pfd->SetFileName(fileNameW.c_str());

        //设置默认文件夹
        if (!defaultLocation.IsEmpty()) {
            DStringW defaultPath = defaultLocation.ToStringW();
            IShellItem* psi = nullptr;
            hr = SHCreateItemFromParsingName(defaultPath.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hr) && (psi != nullptr)) {
                pfd->SetDefaultFolder(psi);
                psi->Release();
                psi = nullptr;
            }
        }

        if (pfd->Show((pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr) == S_OK) {
            IShellItem* pItem = nullptr;
            hr = pfd->GetResult(&pItem);
            if (SUCCEEDED(hr) && (pItem != nullptr)) {
                LPWSTR resultptr = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);
                if (SUCCEEDED(hr) && (resultptr != nullptr)) {
                    filePath = FilePath(resultptr);
                    ::CoTaskMemFree(resultptr);
                    resultptr = nullptr;
                }
                pItem->Release();
            }
        }
        pfd->Release();
    }
    return !filePath.IsEmpty();
}

bool FileDialog::BrowseForFiles(Window* pWindow, 
                                std::vector<FilePath>& filePaths,                                
                                const std::vector<FileType>& fileTypes,
                                int32_t nFileTypeIndex,
                                const DString& defaultExt,
                                const FilePath& defaultLocation)
{
    filePaths.clear();
    IFileOpenDialog* pfd = nullptr;//仅Win7以及上支持
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && (pfd != nullptr)) {
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
        if ((nFileTypeIndex >= 0) && (nFileTypeIndex < (int32_t)fileTypes.size())) {
            hr = pfd->SetFileTypeIndex((UINT)nFileTypeIndex + 1); //SetFileTypeIndex的下标是从1开始的
            ASSERT(SUCCEEDED(hr));
        }
        if (!defaultExt.empty()) {
            hr = pfd->SetDefaultExtension(StringConvert::TToWString(defaultExt).c_str());
            ASSERT(SUCCEEDED(hr));
        }

        //设置默认文件夹
        if (!defaultLocation.IsEmpty()) {
            DStringW defaultPath = defaultLocation.ToStringW();
            IShellItem* psi = nullptr;
            hr = SHCreateItemFromParsingName(defaultPath.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hr) && (psi != nullptr)) {
                pfd->SetDefaultFolder(psi);
                psi->Release();
                psi = nullptr;
            }
        }

        hr = pfd->Show((pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr);
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
                            DStringW folderPath = pName;
                            ::CoTaskMemFree(pName);
                            pName = nullptr;
                            if (!folderPath.empty()) {
                                filePaths.push_back(FilePath(folderPath));
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

#endif //DUILIB_BUILD_FOR_WIN
