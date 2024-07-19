#include "GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

//渲染引擎
#include "duilib/RenderSkia/RenderFactory_Skia.h"
#include "duilib/Render/RenderConfig.h"

#ifdef DUILIB_BUILD_FOR_WIN
    //ToolTip/日期时间等标准控件，需要初始化commctrl
    #include <commctrl.h>
#endif

#include <filesystem>

namespace ui 
{

GlobalManager::GlobalManager():
    m_pfnCreateControlCallback(nullptr),
    m_platformData(nullptr)
{
}

GlobalManager::~GlobalManager()
{
    Shutdown();
}

GlobalManager& GlobalManager::Instance()
{
    static GlobalManager self;
    return self;
}

bool GlobalManager::Startup(const ResourceParam& resParam,
                            DpiInitParam dpiInitParam,
                            const CreateControlCallback& callback)
{
    ASSERT(m_renderFactory == nullptr);
    if (m_renderFactory != nullptr) {
        return false;
    }
    //记录当前线程ID
    m_dwUiThreadId = std::this_thread::get_id();

    //记录平台相关数据
    m_platformData = resParam.platformData;

    //保存回调函数
    m_pfnCreateControlCallback = callback;

    //初始化DPI感知模式，//初始化DPI值
    DpiManager& dpiManager = Dpi();
    dpiManager.InitDpiAwareness(dpiInitParam);

    //初始化定时器
    m_timerManager.Initialize(m_platformData);

    //Skia渲染引擎实现
    m_renderFactory = std::make_unique<RenderFactory_Skia>();    

#ifdef DUILIB_BUILD_FOR_WIN
    //Init Windows Common Controls (for the ToolTip control)
    ::InitCommonControls();
#endif

    ASSERT(m_renderFactory != nullptr);
    if (m_renderFactory == nullptr) {
        return false;
    }

    //加载资源
    if (!ReloadResource(resParam, false)) {
        m_pfnCreateControlCallback = nullptr;
        m_renderFactory.reset();
        return false;
    }
    return true;
}

void GlobalManager::Shutdown()
{
    m_threadManager.Clear();
    m_timerManager.Clear();
    m_colorManager.Clear();    
    m_fontManager.RemoveAllFonts();
    m_fontManager.RemoveAllFontFiles();
    m_imageManager.RemoveAllImages();
    m_zipManager.CloseResZip();    
    m_langManager.ClearStringTable();
    
    m_renderFactory.reset();
    m_renderFactory = nullptr;
    m_pfnCreateControlCallback = nullptr;
    m_globalClass.clear();
    m_windowList.clear();
    m_dwUiThreadId = std::thread::id();
    m_resourcePath.Clear();
    m_languagePath.Clear();
    m_fontFilePath.Clear();
    m_builderMap.clear();
    m_platformData = nullptr;

    //执行退出时清理资源的函数
    for (std::function<void()> atExitFunction : m_atExitFunctions) {
        if (atExitFunction != nullptr) {
            atExitFunction();
        }
    }
    m_atExitFunctions.clear();
}

const FilePath& GlobalManager::GetResourcePath() const
{
    return m_resourcePath;
}

void GlobalManager::SetResourcePath(const FilePath& strPath)
{
    m_resourcePath = strPath;
    m_resourcePath.NormalizeDirectoryPath();
}

void* GlobalManager::GetPlatformData() const
{
    return m_platformData;
}

void GlobalManager::SetFontFilePath(const FilePath& strPath)
{
     m_fontFilePath = strPath;
     m_fontFilePath.NormalizeDirectoryPath();
}

const FilePath& GlobalManager::GetFontFilePath() const
{
    return m_fontFilePath;
}

void GlobalManager::SetLanguagePath(const FilePath& strPath)
{
    m_languagePath = strPath;
    m_languagePath.NormalizeDirectoryPath();
}

const FilePath& GlobalManager::GetLanguagePath() const
{
    return m_languagePath;
}

const DString& GlobalManager::GetLanguageFileName() const
{
    return m_languageFileName;
}

bool GlobalManager::ReloadResource(const ResourceParam& resParam, bool bInvalidate)
{
    AssertUIThread();
    //校验输入参数
    FilePath strResourcePath = resParam.resourcePath;
    if (resParam.GetResType() == ResourceType::kLocalFiles) {
        //本地文件的形式，所有资源都已本地文件的形式存在
        //const LocalFilesResParam& param = static_cast<const LocalFilesResParam&>(resParam);
        ASSERT(!strResourcePath.IsEmpty());
        if (strResourcePath.IsEmpty()) {
            return false;
        }
    }
    else if (resParam.GetResType() == ResourceType::kZipFile) {
        //资源文件打包为zip压缩包，然后以本地文件的形式存在
        const ZipFileResParam& param = static_cast<const ZipFileResParam&>(resParam);
        bool bZipOpenOk = Zip().OpenZipFile(param.zipFilePath, param.zipPassword);
        if (!bZipOpenOk) {
            ASSERT(!"OpenZipFile failed!");
            return false;
        }
    }
#ifdef DUILIB_BUILD_FOR_WIN
    else if (resParam.GetResType() == ResourceType::kResZipFile) {
        //资源文件打包为zip压缩包，然后放在exe/dll的资源文件中
        const ResZipFileResParam& param = static_cast<const ResZipFileResParam&>(resParam);
        bool bZipOpenOk = Zip().OpenResZip(param.hResModule, param.resourceName, param.resourceType, param.zipPassword);
        if (!bZipOpenOk) {
            ASSERT(!"OpenResZip failed!");
            return false;
        }
    }
#endif
    else {
        ASSERT(false);
        return false;
    }

    //清空原有资源数据（字体、颜色、Class定义、图片资源等）
    m_fontManager.RemoveAllFonts();
    m_fontManager.RemoveAllFontFiles();
    m_colorManager.RemoveAllColors();
    RemoveAllImages();
    RemoveAllClasss();

    //保存资源路径
    SetResourcePath(FilePathUtil::JoinFilePath(strResourcePath, resParam.themePath));

    //保存字体文件所在路径
    SetFontFilePath(FilePathUtil::JoinFilePath(strResourcePath, resParam.fontFilePath));

    //解析全局资源信息(默认是"global.xml"文件)
    ASSERT(!resParam.globalXmlFileName.empty());
    if (!resParam.globalXmlFileName.empty()) {
        WindowBuilder dialog_builder;
        Window paint_manager;
        dialog_builder.CreateFromXmlFile(FilePath(resParam.globalXmlFileName), CreateControlCallback(), &paint_manager);
    }

    //加载多语言文件(可选)
    if (!resParam.languagePath.IsEmpty() && !resParam.languageFileName.empty()) {
        FilePath languagePath = FilePathUtil::JoinFilePath(strResourcePath, resParam.languagePath);
        ReloadLanguage(languagePath, resParam.languageFileName, false);
    }
    else if (!resParam.languagePath.IsEmpty()) {
        SetLanguagePath(FilePathUtil::JoinFilePath(strResourcePath, resParam.languagePath));
    }

    //更新窗口中的所有子控件状态
    if (bInvalidate) {
        std::vector<WindowWeakFlag> windowList = m_windowList;
        for (const WindowWeakFlag& windowFlag : windowList) {
            Box* pBox = nullptr;
            if ((windowFlag.m_pWindow != nullptr) && !windowFlag.m_weakFlag.expired()) {
                pBox = windowFlag.m_pWindow->GetRoot();
            }
            if ((pBox != nullptr) && !windowFlag.m_weakFlag.expired()) {
                pBox->Invalidate();
            }
        }
    }
    return true;
}

bool GlobalManager::ReloadLanguage(const FilePath& languagePath,
                                   const DString& languageFileName,
                                   bool bInvalidate)
{
    AssertUIThread();
    ASSERT(!languageFileName.empty());
    if (languageFileName.empty()) {
        return false;
    }

    FilePath newLanguagePath = GetLanguagePath();
    if (!languagePath.IsEmpty()) {
        newLanguagePath = languagePath;
        newLanguagePath.NormalizeDirectoryPath();
    }

    //加载多语言文件，如果使用了资源压缩包则从内存中加载语言文件
    bool bReadOk = false;
    if ( (newLanguagePath.IsEmpty() || !newLanguagePath.IsAbsolutePath()) &&
         m_zipManager.IsUseZip() ) {
        std::vector<unsigned char> fileData;
        FilePath filePath = FilePathUtil::JoinFilePath(newLanguagePath, FilePath(languageFileName));
        if (m_zipManager.GetZipData(filePath, fileData)) {
            bReadOk = m_langManager.LoadStringTable(fileData);
        }
        else {
            ASSERT(!"GetZipData failed!");
        }
    }
    else {
        FilePath filePath = FilePathUtil::JoinFilePath(newLanguagePath, FilePath(languageFileName));
        bReadOk = m_langManager.LoadStringTable(filePath);
    }

    if (bReadOk) {
        //保存语言文件路径
        if (!newLanguagePath.IsEmpty() && (newLanguagePath != GetLanguagePath())) {
            SetLanguagePath(newLanguagePath);
        }
        //保存语言文件名
        m_languageFileName = languageFileName;
    }

    ASSERT(bReadOk && "ReloadLanguage");
    if (bReadOk && bInvalidate) {
        //刷新界面显示
        std::vector<WindowWeakFlag> windowList = m_windowList;
        for (const WindowWeakFlag& windowFlag : windowList) {
            Box* pBox = nullptr;
            if ((windowFlag.m_pWindow != nullptr) && !windowFlag.m_weakFlag.expired()) {
                pBox = windowFlag.m_pWindow->GetRoot();
                if (windowFlag.m_pWindow->GetText().empty() && 
                    !windowFlag.m_pWindow->GetTextId().empty()) {
                    //更新窗口标题栏文本
                    windowFlag.m_pWindow->SetTextId(windowFlag.m_pWindow->GetTextId());
                }
            }
            if ((pBox != nullptr) && !windowFlag.m_weakFlag.expired()) {
                pBox->Invalidate();
            }
        }
    }
    return bReadOk;
}

bool GlobalManager::GetLanguageList(std::vector<std::pair<DString, DString>>& languageList,
                                    const DString& languageNameID) const
{
    FilePath languagePath = GetLanguagePath();
    ASSERT(!languagePath.IsEmpty());
    if (languagePath.IsEmpty()) {
        return false;
    }

    languageList.clear();
    const std::filesystem::path path{ languagePath.ToStringW()};
    if (path.is_absolute()) {
        //绝对路径，语言文件在本地磁盘中
        for (auto const& dir_entry : std::filesystem::directory_iterator{ path }) {
            if (dir_entry.is_regular_file()) {
                languageList.push_back({ StringUtil::UTF16ToT(dir_entry.path().filename().c_str()), _T("") });
            }
        }
        if (!languageNameID.empty()) {
            for (auto& lang : languageList) {
                const DString& fileName = lang.first;
                DString& displayName = lang.second;

                FilePath filePath = FilePathUtil::JoinFilePath(languagePath, FilePath(fileName));
                ui::LangManager langManager;
                if (langManager.LoadStringTable(filePath)) {
                    displayName = langManager.GetStringViaID(languageNameID);
                }
            }
        }
    }
    else if(m_zipManager.IsUseZip()){
        //相对路径，语言文件应该都在压缩包内
        std::vector<DString> fileList;
        m_zipManager.GetZipFileList(languagePath, fileList);
        for (auto const& file : fileList) {
            languageList.push_back({ file, _T("") });
        }

        if (!languageNameID.empty()) {
            for (auto& lang : languageList) {
                const DString& fileName = lang.first;
                DString& displayName = lang.second;

                FilePath filePath = FilePathUtil::JoinFilePath(languagePath, FilePath(fileName));
                std::vector<unsigned char> fileData;
                if (m_zipManager.GetZipData(filePath, fileData)) {
                    ui::LangManager langManager;
                    if (langManager.LoadStringTable(fileData)) {
                        displayName = langManager.GetStringViaID(languageNameID);
                    }
                }
            }
        }
    }
    else {
        ASSERT(false);
        return false;
    }
    return true;
}

FilePath GlobalManager::GetExistsResFullPath(const FilePath& windowResPath, const FilePath& windowXmlPath, const FilePath& resPath)
{
    if (resPath.IsEmpty() || !resPath.IsRelativePath()) {
        return resPath;
    }

    //首先在窗口的资源目录中查找（命中率高）
    const FilePath windowResFullPath = FilePathUtil::JoinFilePath(GlobalManager::GetResourcePath(), windowResPath);
    FilePath imageFullPath = FilePathUtil::JoinFilePath(windowResFullPath, resPath);
    imageFullPath.NormalizeFilePath();
    if (!m_zipManager.IsZipResExist(imageFullPath) && !imageFullPath.IsExistsFile()) {
        //如果文件不存在，返回空
        imageFullPath.Clear();
    }

    if (imageFullPath.IsEmpty()) {
        //其次在公共目录中查找（命中率高）
        imageFullPath = FilePathUtil::JoinFilePath(GlobalManager::GetResourcePath(), resPath);
        imageFullPath.NormalizeFilePath();
        if (!m_zipManager.IsZipResExist(imageFullPath) && !imageFullPath.IsExistsFile()) {
            //如果文件不存在，返回空
            imageFullPath.Clear();
        }
    }

    if (imageFullPath.IsEmpty() && !windowXmlPath.IsEmpty()) {
        //最后在XML文件所在目录中查找
        const FilePath windowXmlFullPath = FilePathUtil::JoinFilePath(windowResFullPath, windowXmlPath);
        imageFullPath = FilePathUtil::JoinFilePath(windowXmlFullPath, resPath);
        imageFullPath.NormalizeFilePath();
        if (!m_zipManager.IsZipResExist(imageFullPath) && !imageFullPath.IsExistsFile()) {
            //如果文件不存在，返回空
            imageFullPath.Clear();
        }
    }
    ASSERT(!imageFullPath.IsEmpty());
    return imageFullPath;
}

void GlobalManager::AddWindow(Window* pWindow)
{
    AssertUIThread();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        WindowWeakFlag flag;
        flag.m_pWindow = pWindow;
        flag.m_weakFlag = pWindow->GetWeakFlag();
        m_windowList.push_back(flag);
    }    
}

void GlobalManager::RemoveWindow(Window* pWindow)
{
    AssertUIThread();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
            if (iter->m_pWindow == pWindow) {
                m_windowList.erase(iter);
                break;
            }
        }
    }
}

void GlobalManager::RemoveAllImages()
{
    AssertUIThread();
    std::vector<WindowWeakFlag> windowList = m_windowList;
    for (const WindowWeakFlag& windowFlag : windowList) {
        Box* pBox = nullptr;
        if ((windowFlag.m_pWindow != nullptr) && !windowFlag.m_weakFlag.expired()) {
            pBox = windowFlag.m_pWindow->GetRoot();
        }
        if ((pBox != nullptr) && !windowFlag.m_weakFlag.expired()) {
            pBox->ClearImageCache();
        }
    }
    m_imageManager.RemoveAllImages();
}

IRenderFactory* GlobalManager::GetRenderFactory()
{
    return m_renderFactory.get();
}

void GlobalManager::AddClass(const DString& strClassName, const DString& strControlAttrList)
{
    AssertUIThread();
    ASSERT(!strClassName.empty() && !strControlAttrList.empty());
    if (!strClassName.empty() && !strControlAttrList.empty()) {
        m_globalClass[strClassName] = strControlAttrList;
    }    
}

DString GlobalManager::GetClassAttributes(const DString& strClassName) const
{
    AssertUIThread();
    auto it = m_globalClass.find(strClassName);
    if (it != m_globalClass.end()) {
        return it->second;
    }
    return DString();
}

void GlobalManager::RemoveAllClasss()
{
    AssertUIThread();
    m_globalClass.clear();
}

ColorManager& GlobalManager::Color()
{
    return m_colorManager;
}

FontManager& GlobalManager::Font()
{
    return m_fontManager;
}

ImageManager& GlobalManager::Image()
{
    return m_imageManager;
}

#ifdef DUILIB_BUILD_FOR_WIN
IconManager& GlobalManager::Icon()
{
    return m_iconManager;
}
#endif

ZipManager& GlobalManager::Zip()
{
    return m_zipManager;
}

DpiManager& GlobalManager::Dpi()
{
    return m_dpiManager;
}

TimerManager& GlobalManager::Timer()
{
    return m_timerManager;
}

ThreadManager& GlobalManager::Thread()
{
    return m_threadManager;
}

LangManager& GlobalManager::Lang()
{
    return m_langManager;
}

CursorManager& GlobalManager::Cursor()
{
    return m_cursorManager;
}

Box* GlobalManager::CreateBox(const FilePath& strXmlPath, CreateControlCallback callback)
{
    WindowBuilder builder;
    Control* pControl = builder.CreateFromXmlFile(strXmlPath, callback);
    ASSERT(pControl != nullptr);
    return builder.ToBox(pControl);
}

Box* GlobalManager::CreateBoxWithCache(const FilePath& strXmlPath, CreateControlCallback callback)
{
    Box* box = nullptr;
    auto it = m_builderMap.find(strXmlPath);
    if (it == m_builderMap.end()) {
        WindowBuilder* builder = new WindowBuilder();
        Control* pControl = builder->CreateFromXmlFile(strXmlPath, callback);
        box = builder->ToBox(pControl);
        if (box != nullptr) {
            m_builderMap[strXmlPath].reset(builder);
        }
        else {
            delete builder;
            builder = nullptr;
        }
    }
    else {
        Control* pControl = it->second->CreateFromCachedXml(callback);
        box = it->second->ToBox(pControl);
    }
    ASSERT(box != nullptr);
    return box;
}

void GlobalManager::FillBox(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback)
{
    ASSERT(pUserDefinedBox != nullptr);
    if (pUserDefinedBox != nullptr) {
        WindowBuilder winBuilder;
        Control* pControl = winBuilder.CreateFromXmlFile(strXmlPath, callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
        Box* box = winBuilder.ToBox(pControl);
        ASSERT_UNUSED_VARIABLE(box != nullptr);
    }    
}

void GlobalManager::FillBoxWithCache(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback)
{
    ASSERT(pUserDefinedBox != nullptr);
    if (pUserDefinedBox == nullptr) {
        return;
    }
    ASSERT(pUserDefinedBox->GetWindow() != nullptr); //DPI感知功能要求，必须先关联窗口
    Box* box = nullptr;
    auto it = m_builderMap.find(strXmlPath);
    if (it == m_builderMap.end()) {
        WindowBuilder* winBuilder = new WindowBuilder();
        Control* pControl = winBuilder->CreateFromXmlFile(strXmlPath, callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
        box = winBuilder->ToBox(pControl);
        if (box != nullptr) {
            m_builderMap[strXmlPath].reset(winBuilder);
        }
        else {
            delete winBuilder;
            winBuilder = nullptr;
        }
    }
    else {
        Control* pControl = it->second->CreateFromCachedXml(callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
        box = it->second->ToBox(pControl);
    }
    ASSERT(pUserDefinedBox == box);
    ASSERT_UNUSED_VARIABLE(box != nullptr);
}

Control* GlobalManager::CreateControl(const DString& strControlName)
{
    if (m_pfnCreateControlCallback) {
        return m_pfnCreateControlCallback(strControlName);
    }
    return nullptr;
}

void GlobalManager::AssertUIThread() const
{
    ASSERT(m_dwUiThreadId == std::this_thread::get_id());
}

void GlobalManager::AddAtExitFunction(std::function<void()> atExitFunction)
{
    ASSERT(atExitFunction != nullptr);
    if (atExitFunction != nullptr) {
        m_atExitFunctions.push_back(atExitFunction);
    }
}

} // namespace ui
