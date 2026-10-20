#include "FileDialog.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

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

/** 文件类型筛选器(转换为SDL格式后使用)
*/
struct DlgFileFilter
{
    /** 筛选器显示名称(UTF8编码)，保持生命周期，供m_filters引用
    */
    std::vector<DStringA> m_names;

    /** 筛选器扩展名(UTF8编码)，保持生命周期，供m_filters引用
    */
    std::vector<DStringA> m_exts;

    /** SDL格式的文件类型筛选器
    */
    std::vector<SDL_DialogFileFilter> m_filters;
};

/** 将duilib的文件类型筛选器，转换为SDL格式的文件类型筛选器
*/
static void ConvertFileTypes(const std::vector<FileDialog::FileType>& fileTypes, DlgFileFilter& dlgFileFilter)
{
    for (const FileDialog::FileType& fileType : fileTypes) {
        DString name = fileType.szName;
        if (!fileType.szNameId.empty()) {
            name = GlobalManager::GetTextById(fileType.szNameId);
        }
        DStringA nameA = StringConvert::TToUTF8(name);
        DStringA extA = StringConvert::TToUTF8(fileType.szExt);
        //将"*.txt"格式的转换为"txt"格式
        StringUtil::ReplaceAll("*.", "", extA);
        if (!nameA.empty() && !extA.empty()) {
            dlgFileFilter.m_names.push_back(nameA);
            dlgFileFilter.m_exts.push_back(extA);
        }
    }
    if (!dlgFileFilter.m_names.empty()) {
        const size_t nCount = dlgFileFilter.m_names.size();
        dlgFileFilter.m_filters.resize(nCount);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            dlgFileFilter.m_filters[nIndex].name = dlgFileFilter.m_names[nIndex].c_str();
            dlgFileFilter.m_filters[nIndex].pattern = dlgFileFilter.m_exts[nIndex].c_str();
        }
    }
}

/** 获取默认的文件扩展名
* @param [in] defaultExt 默认的文件类型, 举例："*.txt"、"doc;docx"
* @return 返回扩展名(不含点号), 举例："txt"；如果无法得到有效的扩展名，则返回空字符串
*/
static DStringA GetDefaultExtension(const DString& defaultExt)
{
    DStringA extA = StringConvert::TToUTF8(defaultExt);
    //将"*.txt"格式的转换为"txt"格式
    StringUtil::ReplaceAll("*.", "", extA);
    //有多个扩展名时(如"doc;docx")，使用第一个扩展名
    const DStringA::size_type nPos = extA.find(';');
    if (nPos != DStringA::npos) {
        extA = extA.substr(0, nPos);
    }
    //去掉扩展名前缀的点号：".txt"转换为"txt"；
    //而"*.*"转换后为"."，去掉点号后为空，属于无效的扩展名
    while (!extA.empty() && (extA.front() == '.')) {
        extA.erase(0, 1);
    }
    StringUtil::Trim(extA);
    //扩展名中不允许包含通配符
    if (extA.find('*') != DStringA::npos) {
        extA.clear();
    }
    return extA;
}

/** 获取文件对话框的"默认位置"(即SDL的default_location参数值)
*  SDL的文件对话框只有一个默认位置参数：在Windows平台，SDL会把该参数按最后一个路径分隔符，
*  拆分为"默认文件夹"(IFileDialog::SetFolder)和"默认文件名"(IFileDialog::SetFileName)两部分，
*  因此这里将"默认目录"和"默认文件名"组合成一个路径字符串传入；
*  注意：如果路径以路径分隔符结尾，则只会被识别为目录(不会设置默认文件名)。
* @param [in] defaultLocation 默认的文件夹路径(可选参数, 可以为空)
* @param [in] defaultFileName 默认的文件名(可选参数, 可以为空, UTF8编码)
* @return 返回UTF8编码的路径字符串
*/
static DStringA GetDefaultLocation(const FilePath& defaultLocation, const DStringA& defaultFileName)
{
    DStringA locationA = defaultLocation.ToStringA();
    if (!locationA.empty()) {
        //确保以路径分隔符结尾(如果结尾不是路径分隔符，SDL会把它识别为默认的文件名)
        const char lastChar = locationA.back();
        if ((lastChar != '/') && (lastChar != '\\')) {
            locationA += StringConvert::TToUTF8(FilePath::GetPathSeparatorStr());
        }
    }
    if (!defaultFileName.empty()) {
        locationA += defaultFileName;
    }
    return locationA;
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

    //初始选择的文件夹(确保以路径分隔符结尾，否则SDL会把它识别为默认的文件名)
    DStringA defaultFolder = GetDefaultLocation(defaultLocation, DStringA());
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

    //初始选择的文件夹(确保以路径分隔符结尾，否则SDL会把它识别为默认的文件名)
    DStringA defaultFolder = GetDefaultLocation(defaultLocation, DStringA());
    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultFolder.empty()) {
        default_location = defaultFolder.c_str();
    }
    bool allow_many = true;
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
                               int32_t nFileTypeIndex,
                               const DString& defaultExt,
                               const DString& fileName,
                               const FilePath& defaultLocation)
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

    //获取默认的文件扩展名：
    //SDL的文件对话框不支持设置默认的文件类型(没有预选筛选器索引的接口)，
    //也不支持设置默认的扩展名(没有调用IFileDialog::SetDefaultExtension)，
    //因此：如果defaultExt为空，且nFileTypeIndex有效时，使用该文件类型的扩展名作为默认扩展名；
    //然后将默认扩展名追加到默认文件名与返回的文件名之后，达到与Windows平台一致的效果。
    DString defaultExtValue = defaultExt;
    if (defaultExtValue.empty() && (nFileTypeIndex >= 0) && (nFileTypeIndex < (int32_t)fileTypes.size())) {
        defaultExtValue = fileTypes[(size_t)nFileTypeIndex].szExt;
    }
    const DStringA defaultExtA = GetDefaultExtension(defaultExtValue);

    //默认的文件名：如果文件名没有扩展名，则加上默认的扩展名
    DStringA defaultFileNameA = StringConvert::TToUTF8(fileName);
    if (!defaultFileNameA.empty() && !defaultExtA.empty()) {
        FilePath namePath(fileName);
        if (namePath.GetFileExtension().empty()) {
            defaultFileNameA += ".";
            defaultFileNameA += defaultExtA;
        }
    }

    //初始选择的文件夹与文件名(组合为一个路径，作为SDL的default_location参数)
    const DStringA defaultLocationA = GetDefaultLocation(defaultLocation, defaultFileNameA);

    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultLocationA.empty()) {
        default_location = defaultLocationA.c_str();
    }
    bool allow_many = false;

    //文件类型筛选器
    DlgFileFilter dlgFileFilter;
    ConvertFileTypes(fileTypes, dlgFileFilter);
    const SDL_DialogFileFilter* filters = nullptr;
    int nfilters = 0;
    if (!dlgFileFilter.m_filters.empty()) {
        filters = dlgFileFilter.m_filters.data();
        nfilters = (int)dlgFileFilter.m_filters.size();
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
    //保存文件时：如果返回的文件路径没有扩展名，则追加默认的扩展名
    //(SDL的文件对话框不会自动添加默认扩展名，这里做一次校正)
    if (!bOpenFileDialog && !filePath.IsEmpty() && !defaultExtA.empty() && filePath.GetFileExtension().empty()) {
        filePath = FilePath(filePath.ToString() + _T(".") + StringConvert::UTF8ToT(defaultExtA));
    }

    pUserData->Release();
    return !filePath.IsEmpty();
}

bool FileDialog::BrowseForFiles(Window* pWindow, 
                                std::vector<FilePath>& filePaths,                                
                                const std::vector<FileType>& fileTypes,
                                int32_t nFileTypeIndex,
                                const DString& defaultExt,
                                const FilePath& defaultLocation)
{
    //注：参数nFileTypeIndex(预选的文件类型)与defaultExt(默认的文件扩展名)在"打开文件(多选)"对话框中
    //没有对应的SDL接口(SDL不支持预选筛选器索引，也没有设置默认扩展名的接口)，因此这两个参数未使用；
    //保留参数是为了与Windows平台(FileDialog_Windows.cpp)的接口保持一致。
    (void)nFileTypeIndex;
    (void)defaultExt;

    filePaths.clear();
    SDL_Window* sdlWindow = nullptr;
    if (pWindow != nullptr) {
        sdlWindow = (SDL_Window*)pWindow->NativeWnd()->GetWindowHandle();
    }
    //用于同步的数据
    DialogFileCallbackData* pUserData = new DialogFileCallbackData;
    pUserData->AddRef();
    pUserData->m_bAllowMany = true;

    //初始选择的文件夹(确保以路径分隔符结尾，否则SDL会把它识别为默认的文件名)
    const DStringA defaultLocationA = GetDefaultLocation(defaultLocation, DStringA());

    SDL_DialogFileCallback callback = DialogFileCallback;
    void* userdata = pUserData;
    pUserData->AddRef(); //增加一个引用计数，保护数据
    const char* default_location = nullptr;
    if (!defaultLocationA.empty()) {
        default_location = defaultLocationA.c_str();
    }
    bool allow_many = true;

    //文件类型筛选器
    DlgFileFilter dlgFileFilter;
    ConvertFileTypes(fileTypes, dlgFileFilter);
    const SDL_DialogFileFilter* filters = nullptr;
    int nfilters = 0;
    if (!dlgFileFilter.m_filters.empty()) {
        filters = dlgFileFilter.m_filters.data();
        nfilters = (int)dlgFileFilter.m_filters.size();
    }

    SDL_ShowOpenFileDialog(callback, userdata, sdlWindow, filters, nfilters, default_location, allow_many);

    //运行消息循环，等待退出
    MessageLoop_SDL messageLoop;
    messageLoop.RunUserLoop(pUserData->m_bTerminate);

    //读取数据
    //注：多选时，SDL返回的是以NULL结尾的完整路径数组(由IFileOpenDialog::GetResults获取)，
    //    SDL 3.0的早期版本该返回值存在缺陷，当前使用的SDL版本(3.5.0)已修复。
    const size_t nCount = pUserData->m_filePaths.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        filePaths.push_back(FilePath(StringConvert::UTF8ToWString(pUserData->m_filePaths.at(nIndex))));
    }

    pUserData->Release();
    return !filePaths.empty();
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_SDL
