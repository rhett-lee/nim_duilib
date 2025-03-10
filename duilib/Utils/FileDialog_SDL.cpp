#include "FileDialog.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

#ifdef DUILIB_BUILD_FOR_SDL
#include "duilib/Core/MessageLoop_SDL.h"
#include <SDL3/SDL.h>

namespace ui
{
struct DialogFileCallbackData
{
public:
    /** 引用计数
    */
    int32_t m_nRefs = 0;

    /** 退出标志
    */
    bool m_bTerminate = false;

    /** 返回的文件路径
    */
    std::vector<DStringA> m_filePaths;

    /** 是否支持多选
    */
    bool m_bAllowMany = false;

public:
    /** 增加引用计数
    */
    int32_t AddRef(void)
    {
        return ++m_nRefs;
    }

    /** 减少引用计数
    */
    int32_t Release(void)
    {
        int32_t nRefs = --m_nRefs;
        if (nRefs == 0) {
            delete this;
        }
        return nRefs;
    }
};

static void SDLCALL DialogFileCallback(void* userdata, const char* const* filelist, int /*filter*/)
{
    DialogFileCallbackData* pUserData = (DialogFileCallbackData*)userdata;
    if (pUserData == nullptr) {
        return;
    }
    //读取结果
    if (filelist != nullptr) {
        if (pUserData->m_bAllowMany) {
            //多选
            int32_t nIndex = 0;
            while (filelist[nIndex] != nullptr) {
                pUserData->m_filePaths.push_back(filelist[nIndex]);
                ++nIndex;
            }
        }
        else if (filelist[0] != nullptr){
            //单选
            pUserData->m_filePaths.push_back(filelist[0]);
        }
    }
    //标记为退出
    pUserData->m_bTerminate = true;
    MessageLoop_SDL::PostNoneEvent();

    //减少引用计数，标记可以释放数据
    pUserData->Release();
}

bool FileDialog::BrowseForFolder(Window* pWindow, FilePath& folderPath, const FilePath& defaultLocation)
{
    folderPath.Clear();
    SDL_Window* sdlWindow = nullptr;
    if (pWindow != nullptr) {
        sdlWindow = (SDL_Window*)pWindow->NativeWnd()->GetWindowHandle();
    }
    //用于同步的数据
    DialogFileCallbackData* pUserData = new DialogFileCallbackData;
    pUserData->AddRef();
    pUserData->m_bAllowMany = false;

    //初始选择的文件夹
    DStringA defaultFolder = defaultLocation.ToStringA();
    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultFolder.empty()) {
        default_location = defaultFolder.c_str();
    }
    bool allow_many = false;
    SDL_ShowOpenFolderDialog(callback, userdata, sdlWindow, default_location, allow_many);

    //运行消息循环，等待退出
    MessageLoop_SDL messageLoop;
    messageLoop.RunUserLoop(pUserData->m_bTerminate);

    //读取数据
    if (!pUserData->m_filePaths.empty()) {
        folderPath = FilePath(StringConvert::UTF8ToWString(pUserData->m_filePaths.front()));
    }

    pUserData->Release();
    return !folderPath.IsEmpty();
}

bool FileDialog::BrowseForFolders(Window* pWindow, std::vector<FilePath>& folderPaths, const FilePath& defaultLocation)
{
    folderPaths.clear();
    SDL_Window* sdlWindow = nullptr;
    if (pWindow != nullptr) {
        sdlWindow = (SDL_Window*)pWindow->NativeWnd()->GetWindowHandle();
    }
    //用于同步的数据
    DialogFileCallbackData* pUserData = new DialogFileCallbackData;
    pUserData->AddRef();
    pUserData->m_bAllowMany = true;

    //初始选择的文件夹
    DStringA defaultFolder = defaultLocation.ToStringA();
    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultFolder.empty()) {
        default_location = defaultFolder.c_str();
    }
    bool allow_many = false;
    SDL_ShowOpenFolderDialog(callback, userdata, sdlWindow, default_location, allow_many);

    //运行消息循环，等待退出
    MessageLoop_SDL messageLoop;
    messageLoop.RunUserLoop(pUserData->m_bTerminate);

    //读取数据    
    const size_t nCount = pUserData->m_filePaths.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        folderPaths.push_back(FilePath(StringConvert::UTF8ToWString(pUserData->m_filePaths.at(nIndex))));
    }

    pUserData->Release();
    return !folderPaths.empty();
}

bool FileDialog::BrowseForFile(Window* pWindow, 
                               FilePath& filePath,                               
                               bool bOpenFileDialog,
                               const std::vector<FileType>& fileTypes,
                               int32_t /*nFileTypeIndex*/,
                               const DString& /*defaultExt*/,
                               const DString& fileName,
                               const FilePath& /*defaultLocation*/)
{
    filePath.Clear();
    SDL_Window* sdlWindow = nullptr;
    if (pWindow != nullptr) {
        sdlWindow = (SDL_Window*)pWindow->NativeWnd()->GetWindowHandle();
    }
    //用于同步的数据
    DialogFileCallbackData* pUserData = new DialogFileCallbackData;
    pUserData->AddRef();
    pUserData->m_bAllowMany = false;

    //初始选择的文件夹
    DStringA defaultFileName = StringConvert::TToUTF8(fileName);
    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultFileName.empty()) {
        default_location = defaultFileName.c_str();
    }
    bool allow_many = false;

    struct FileTypeA
    {
        DStringA szNameA;
        DStringA szExtA;
    };
    std::vector<FileTypeA> fileTypesA;
    if (!fileTypes.empty()) {
        FileTypeA fileTypeA;
        for (const FileType& fileType : fileTypes) {
            fileTypeA.szNameA = StringConvert::TToUTF8(fileType.szName);
            fileTypeA.szExtA = StringConvert::TToUTF8(fileType.szExt);
            //将"*.txt"格式的转换为"txt"格式
            StringUtil::ReplaceAll("*.", "", fileTypeA.szExtA);
            if (!fileTypeA.szNameA.empty() && !fileTypeA.szExtA.empty()) {
                fileTypesA.push_back(fileTypeA);
            }
        }
    }

    std::vector<SDL_DialogFileFilter> dlgFileFilters;
    if (!fileTypesA.empty()) {
        SDL_DialogFileFilter dlgFileFilter;
        for (const FileTypeA& fileTypeA : fileTypesA) {
            dlgFileFilter.name = fileTypeA.szNameA.c_str();
            dlgFileFilter.pattern = fileTypeA.szExtA.c_str();
            dlgFileFilters.push_back(dlgFileFilter);
        }
    }

    const SDL_DialogFileFilter* filters = nullptr;
    int nfilters = 0;
    if (!dlgFileFilters.empty()) {
        filters = dlgFileFilters.data();
        nfilters = (int)dlgFileFilters.size();
    }

    if (bOpenFileDialog) {
        SDL_ShowOpenFileDialog(callback, userdata, sdlWindow, filters, nfilters, default_location, allow_many);
    }
    else {
        SDL_ShowSaveFileDialog(callback, userdata, sdlWindow, filters, nfilters, default_location);
    }

    //运行消息循环，等待退出
    MessageLoop_SDL messageLoop;
    messageLoop.RunUserLoop(pUserData->m_bTerminate);

    //读取数据
    if (!pUserData->m_filePaths.empty()) {
        filePath = FilePath(StringConvert::UTF8ToWString(pUserData->m_filePaths.front()));
    }

    pUserData->Release();
    return !filePath.IsEmpty();
}

bool FileDialog::BrowseForFiles(Window* pWindow, 
                                std::vector<FilePath>& filePaths,                                
                                const std::vector<FileType>& fileTypes,
                                int32_t /*nFileTypeIndex*/,
                                const DString& /*defaultExt*/,
                                const FilePath& /*defaultLocation*/)
{
    filePaths.clear();
    SDL_Window* sdlWindow = nullptr;
    if (pWindow != nullptr) {
        sdlWindow = (SDL_Window*)pWindow->NativeWnd()->GetWindowHandle();
    }
    //用于同步的数据
    DialogFileCallbackData* pUserData = new DialogFileCallbackData;
    pUserData->AddRef();
    pUserData->m_bAllowMany = true;

    //初始选择的文件夹
    DStringA defaultFileName;
    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultFileName.empty()) {
        default_location = defaultFileName.c_str();
    }
    bool allow_many = true;

    struct FileTypeA
    {
        DStringA szNameA;
        DStringA szExtA;
    };
    std::vector<FileTypeA> fileTypesA;
    if (!fileTypes.empty()) {
        FileTypeA fileTypeA;
        for (const FileType& fileType : fileTypes) {
            fileTypeA.szNameA = StringConvert::TToUTF8(fileType.szName);
            fileTypeA.szExtA = StringConvert::TToUTF8(fileType.szExt);
            //将"*.txt"格式的转换为"txt"格式
            StringUtil::ReplaceAll("*.", "", fileTypeA.szExtA);
            if (!fileTypeA.szNameA.empty() && !fileTypeA.szExtA.empty()) {
                fileTypesA.push_back(fileTypeA);
            }
        }
    }

    std::vector<SDL_DialogFileFilter> dlgFileFilters;
    if (!fileTypesA.empty()) {
        SDL_DialogFileFilter dlgFileFilter;
        for (const FileTypeA& fileTypeA : fileTypesA) {
            dlgFileFilter.name = fileTypeA.szNameA.c_str();
            dlgFileFilter.pattern = fileTypeA.szExtA.c_str();
            dlgFileFilters.push_back(dlgFileFilter);
        }
    }

    const SDL_DialogFileFilter* filters = nullptr;
    int nfilters = 0;
    if (!dlgFileFilters.empty()) {
        filters = dlgFileFilters.data();
        nfilters = (int)dlgFileFilters.size();
    }

    SDL_ShowOpenFileDialog(callback, userdata, sdlWindow, filters, nfilters, default_location, allow_many);

    //运行消息循环，等待退出
    MessageLoop_SDL messageLoop;
    messageLoop.RunUserLoop(pUserData->m_bTerminate);

    //读取数据    
    const size_t nCount = pUserData->m_filePaths.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        filePaths.push_back(FilePath(StringConvert::UTF8ToWString(pUserData->m_filePaths.at(nIndex))));
    }

    //TODO: 多选的情况下，SDL 3.0目前返回的值不对，有Bug，待修复。
    pUserData->Release();
    return !filePaths.empty();
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_SDL
