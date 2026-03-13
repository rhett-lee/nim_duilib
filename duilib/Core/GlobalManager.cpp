#include "GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

//渲染引擎
#include "duilib/RenderSkia/RenderFactory_Skia.h"

//图片解码接口
#include "duilib/Image/ImageDecoder_ICO.h"
#include "duilib/Image/ImageDecoder_Icon.h"
#include "duilib/Image/ImageDecoder_GIF.h"
#include "duilib/Image/ImageDecoder_PNG.h"
#include "duilib/Image/ImageDecoder_PAG.h"
#include "duilib/Image/ImageDecoder_SVG.h"
#include "duilib/Image/ImageDecoder_WEBP.h"
#include "duilib/Image/ImageDecoder_JPEG.h"
#include "duilib/Image/ImageDecoder_LOTTIE.h"
#include "duilib/Image/ImageDecoder_Common.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    //ToolTip/日期时间等标准控件，需要初始化commctrl
    #include <commctrl.h>
    #include <Objbase.h>
#endif

#include <filesystem>

namespace ui 
{
/** 库内部的工作线程
*/
class UiWorkerThread : public ui::FrameworkThread
{
public:
    struct Param
    {
        DString name;
        int32_t nIdentifier;
    };
public:
    UiWorkerThread(const DString& threadName, int32_t nThreadIdentifier):
        FrameworkThread(threadName, nThreadIdentifier)
    { }
    virtual ~UiWorkerThread() override {}

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override
    {
#if defined (DUILIB_BUILD_FOR_WIN)
        HRESULT hr = ::CoInitialize(nullptr);
        ASSERT_UNUSED_VARIABLE((hr == S_OK) || (hr == S_FALSE));
#endif
    }

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override
    {
#if defined (DUILIB_BUILD_FOR_WIN)
        ::CoUninitialize();
#endif
    }
};

GlobalManager::GlobalManager():
    m_platformData(nullptr),
    m_bAnimationEnabled(true),
    m_bStartup(false)
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

FilePath GlobalManager::GetResourceRootPath(bool bMacOsAppBundle)
{
    ui::FilePath resourcePath;
#ifdef DUILIB_BUILD_FOR_MACOS
    //MacOS平台，优先使用bundle的资源目录
    if (bMacOsAppBundle) {
        resourcePath = ui::FilePathUtil::GetBundleResourcesPath();
        if (!resourcePath.IsEmpty()) {
            resourcePath.NormalizeDirectoryPath();
            resourcePath += _T("duilib/");
            if (!resourcePath.IsExistsDirectory()) {
                resourcePath.Clear();
            }
        }
    }
#else
    UNUSED_VARIABLE(bMacOsAppBundle);
#endif
    if (resourcePath.IsEmpty()) {
        resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
        resourcePath += DUILIB_RESOURCE_DIR;
    }
    resourcePath.NormalizeDirectoryPath();
    return resourcePath;
}

FilePath GlobalManager::GetResourceZipPath()
{
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += DUILIB_RESOURCE_ZIP;
    resourcePath.NormalizeFilePath();
    return resourcePath;
}

bool GlobalManager::Startup(const ResourceParam& resParam,
                            DpiInitParam dpiInitParam,
                            const CreateControlCallback& callback)
{
    PerformanceUtil::Instance().BeginStat(_T("Startup"));//程序启动时间统计
    ASSERT(!m_bStartup);
    if (m_bStartup) {
        return false;//禁止重复初始化
    }
    ASSERT(m_renderFactory == nullptr);
    if (m_renderFactory != nullptr) {
        return false;
    }
    //初始化COM/OLE
#if defined (DUILIB_BUILD_FOR_WIN)
    HRESULT hr = ::CoInitialize(nullptr);
    ASSERT_UNUSED_VARIABLE((hr == S_OK) || (hr == S_FALSE));

    hr = ::OleInitialize(nullptr);
    ASSERT_UNUSED_VARIABLE((hr == S_OK) || (hr == S_FALSE));

    //Init Windows Common Controls (for the ToolTip control)
    ::InitCommonControls();
#endif

    //记录当前线程ID
    m_dwUiThreadId = std::this_thread::get_id();

    //记录平台相关数据
    m_platformData = resParam.platformData;

    //初始化DPI感知模式，//初始化DPI值
    DpiManager& dpiManager = Dpi();
    dpiManager.InitDpiAwareness(dpiInitParam);

    //初始化图片格式解码器
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_SVG>());
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_PNG>());
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_GIF>());

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_JPEG>());
#endif

    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_WEBP>());
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_ICO>());
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_Icon>());
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_LOTTIE>());

#ifdef DUILIB_IMAGE_SUPPORT_LIB_PAG
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_PAG>());
#endif

    //通用解码器，放在最后
    m_imageDecoderFactory.AddImageDecoder(std::make_shared<ImageDecoder_Common>());
    

    //初始化定时器
    m_timerManager.Initialize(m_platformData);

    //Skia渲染引擎实现
    m_renderFactory = std::make_unique<RenderFactory_Skia>();    

    ASSERT(m_renderFactory != nullptr);
    if (m_renderFactory == nullptr) {
        return false;
    }

    //保存回调函数
    if (callback != nullptr) {
        m_pfnCreateControlCallbackList.push_back(callback);
    }

    //初始化线程池
    StartInnerThread(ThreadIdentifier::kThreadWorker);
    StartInnerThread(ThreadIdentifier::kThreadImage1);
    StartInnerThread(ThreadIdentifier::kThreadImage2);

    //加载资源
    if (!LoadGlobalResource(resParam)) {
        m_renderFactory.reset();
        return false;
    }
    return true;
}

void GlobalManager::Shutdown()
{
    //终止线程池
    for (std::shared_ptr<FrameworkThread> pThread: m_threadList) {
        if (pThread != nullptr) {
            pThread->Stop();
        }
    }
    m_threadList.clear();

    m_threadManager.Clear();
    m_timerManager.Clear();
    if (m_pColorManager != nullptr) {
        m_pColorManager->Clear();
    }    
    m_fontManager.RemoveAllFonts();
    m_fontManager.RemoveAllFontFiles();
    m_imageManager.RemoveAllImages();
    m_zipManager.CloseResZip();    
    m_langManager.ClearStringTable();
    m_windowManager.Clear();
    m_themeManager.Clear();
    
    m_renderFactory.reset();
    m_renderFactory = nullptr;
    m_pfnCreateControlCallbackList.clear();
    m_globalClass.clear();    
    m_dwUiThreadId = std::thread::id();
    m_languagePath.Clear();
    m_fontFilePath.Clear();
    m_builderMap.clear();
    m_platformData = nullptr;

    m_aliasMap.clear();
    m_defineMap.clear();

    //执行退出时清理资源的函数
    for (std::function<void()> atExitFunction : m_atExitFunctions) {
        if (atExitFunction != nullptr) {
            atExitFunction();
        }
    }
    m_atExitFunctions.clear();

#if defined (DUILIB_BUILD_FOR_WIN)
    ::CoUninitialize();
    ::OleUninitialize();
#endif
    m_bStartup = false;
}

bool GlobalManager::StopInnerThread(int32_t nThreadIdentifier)
{
    AssertUIThread();
    ASSERT((nThreadIdentifier == ui::kThreadWorker)  ||
           (nThreadIdentifier == ui::kThreadNetwork) ||
           (nThreadIdentifier == ui::kThreadImage1)  ||
           (nThreadIdentifier == ui::kThreadImage2));
    if ((nThreadIdentifier != ui::kThreadWorker)  &&
        (nThreadIdentifier != ui::kThreadNetwork) &&
        (nThreadIdentifier != ui::kThreadImage1)  &&
        (nThreadIdentifier != ui::kThreadImage2)) {
        return false;
    }
    bool bRet = false;
    for (auto iter = m_threadList.begin(); iter != m_threadList.end(); ++iter) {
        auto pThread = *iter;
        if ((pThread != nullptr) && (nThreadIdentifier == pThread->GetThreadIdentifier())) {
            m_threadList.erase(iter);
            pThread->Stop();
            bRet = true;
            break;
        }
    }
    return bRet;
}

bool GlobalManager::StartInnerThread(int32_t nThreadIdentifier)
{
    AssertUIThread();
    ASSERT((nThreadIdentifier == ui::kThreadWorker)  ||
           (nThreadIdentifier == ui::kThreadNetwork) ||
           (nThreadIdentifier == ui::kThreadImage1)  ||
           (nThreadIdentifier == ui::kThreadImage2));
    if ((nThreadIdentifier != ui::kThreadWorker)  &&
        (nThreadIdentifier != ui::kThreadNetwork) &&
        (nThreadIdentifier != ui::kThreadImage1)  &&
        (nThreadIdentifier != ui::kThreadImage2)) {
        return false;
    }
    bool bRet = false;
    for (auto iter = m_threadList.begin(); iter != m_threadList.end(); ++iter) {
        auto pThread = *iter;
        if ((pThread != nullptr) && (nThreadIdentifier == pThread->GetThreadIdentifier())) {
            bRet = true;
            break;
        }
    }
    if (!bRet) {
        //初始化线程池
        std::vector<UiWorkerThread::Param> threadParams = { {_T("Worker"), ThreadIdentifier::kThreadWorker},
                                                            {_T("Network"), ThreadIdentifier::kThreadNetwork},
                                                            {_T("Image1"), ThreadIdentifier::kThreadImage1},
                                                            {_T("Image2"), ThreadIdentifier::kThreadImage2} };
        for (const UiWorkerThread::Param& param : threadParams) {
            if (param.nIdentifier != nThreadIdentifier) {
                continue;
            }
            auto pThread = std::make_shared<UiWorkerThread>(param.name, param.nIdentifier);
            m_threadList.push_back(pThread);
            pThread->Start();
            bRet = true;
            break;
        }
    }
    return bRet;
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

bool GlobalManager::LoadGlobalResource(const ResourceParam& resParam)
{
    AssertUIThread();
    //校验输入参数
    ASSERT(!resParam.themeRootPath.IsEmpty());
    if (resParam.themeRootPath.IsEmpty()) {
        return false;
    }
    ASSERT(!resParam.defaultThemePath.IsEmpty());
    if (resParam.defaultThemePath.IsEmpty()) {
        return false;
    }
    ASSERT(!resParam.globalXmlFileName.empty());
    if (resParam.globalXmlFileName.empty()) {
        return false;
    }

    const FilePath resourcePath = resParam.resourcePath;        //资源根目录: resources（使用zip时是相对路径，使用文件系统是是本地绝对路径）
    FilePath themeRootFullPath = resourcePath;
    themeRootFullPath /= resParam.themeRootPath;                        //主题根目录：resources/themes

    FilePath globalXmlFileNameFullPath = themeRootFullPath;             //主题根目录：resources/themes
    globalXmlFileNameFullPath /= resParam.defaultThemePath;             //默认主题目录：resources/themes/default
    globalXmlFileNameFullPath /= FilePath(resParam.globalXmlFileName);  //配置文件：resources/themes/default/global.xml
    globalXmlFileNameFullPath.NormalizeFilePath();                      //默认取值示例：resources/themes/default/global.xml

    if (resParam.GetResType() == ResourceType::kLocalFiles) {
        //本地文件的形式，所有资源都已本地文件的形式存在
        ASSERT(!resourcePath.IsEmpty());
        if (resourcePath.IsEmpty()) {
            ASSERT(!"Resource path is empty!");
            return false;
        }
        if (!globalXmlFileNameFullPath.IsExistsFile()) {
            ASSERT(!"global xml file not exists!");
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
        if (!Zip().IsZipResExist(globalXmlFileNameFullPath)) {
            ASSERT(!"global xml file not exists!");
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
        if (!Zip().IsZipResExist(globalXmlFileNameFullPath)) {
            ASSERT(!"global xml file not exists!");
            return false;
        }
    }
#endif
    else {
        ASSERT(false);
        return false;
    }

    //清空原有资源数据（字体、颜色、Class定义、图片资源、别名、变量定义等）
    m_fontManager.RemoveAllFonts();
    m_fontManager.RemoveAllFontFiles();
    if (m_pColorManager != nullptr) {
        m_pColorManager->RemoveAllColors();
    }
    RemoveAllImages();
    RemoveAllClasss();
    ClearAlias();
    ClearDefine();

    //保存字体文件所在路径
    SetFontFilePath(FilePathUtil::JoinFilePath(resourcePath, resParam.fontFilePath));

    //加载多语言文件(可选)
    if (!resParam.languagePath.IsEmpty() && !resParam.languageFileName.empty()) {
        FilePath languagePath = FilePathUtil::JoinFilePath(resourcePath, resParam.languagePath);
        ReloadLanguage(languagePath, resParam.languageFileName, false);
    }
    else if (!resParam.languagePath.IsEmpty()) {
        SetLanguagePath(FilePathUtil::JoinFilePath(resourcePath, resParam.languagePath));
    }

    //加载默认主题资源
    if (!Theme().InitTheme(themeRootFullPath, resParam.defaultThemePath, resParam.globalXmlFileName)) {
        ASSERT(!"InitTheme failed!");
        return false;
    }
    if (!resParam.iconThemePath.IsEmpty()) {
        //加载图标主题资源
        if (!Theme().SwitchIconTheme(resParam.iconThemePath)) {
            ASSERT(!"Load icon theme failed!");
        }
    }
    if (!resParam.colorThemePath.IsEmpty()) {
        //加载颜色主题资源
        if (!Theme().SwitchColorTheme(resParam.colorThemePath)) {
            ASSERT(!"Load color theme failed!");
        }
    }
    return true;
}

void GlobalManager::ClearThemeCache()
{
    //图片资源缓存
    RemoveAllImages();

    //字体缓存
    m_fontManager.ClearFontCache();
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
        std::vector<WindowPtr> windowList = Windows().GetAllWindowList();
        for (const WindowPtr& pWindow : windowList) {
            if (pWindow == nullptr) {
                continue;
            }
            Box* pBox = pWindow->GetRoot();
            //支持多语言时，TextId优先
            if (!pWindow->GetTextId().empty()) {
                //更新窗口标题栏文本
                pWindow->SetTextId(pWindow->GetTextId());
            }
            if (pBox != nullptr) {
                pBox->OnLanguageChanged();
                pBox->SetPos(pBox->GetPos());
            }
            pWindow->InvalidateAll();
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
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows: 路径字符串用的是DStringW::value_type，UTF16
    const std::filesystem::path path{ languagePath.ToStringW()};
#else
    //Windows: 路径字符串用的是char，UTF8
    const std::filesystem::path path{ languagePath.ToStringA() };
#endif
    if (path.is_absolute()) {
        //绝对路径，语言文件在本地磁盘中
        for (auto const& dir_entry : std::filesystem::directory_iterator{ path }) {
            if (dir_entry.is_regular_file()) {
                languageList.push_back({ FilePath(dir_entry.path().filename()).ToString(), _T("")});
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
        m_zipManager.GetZipFileList(languagePath, &fileList, nullptr);
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
    bool bLocalPath = false;
    bool bResPath = false;
    return GetExistsResFullPath(windowResPath, windowXmlPath, resPath, nullptr, bLocalPath, bResPath);
}

FilePath GlobalManager::GetExistsResFullPath(const FilePath& windowResPath,
                                             const FilePath& windowXmlPath,
                                             const FilePath& resPath,
                                             const Control* pControl,
                                             bool& bLocalPath,
                                             bool& bResPath)
{
    FilePath imageFullPath = Theme().FindExistsResFullPath(windowResPath, windowXmlPath, resPath, bLocalPath, bResPath);
    if (imageFullPath.IsEmpty()) {
        //图片资源加载失败，通过回调函数给出修正一次的机会
        std::vector<ResNotFoundCallbackData> resNotFoundCallbacks = m_resNotFoundCallbacks;
        for (const ResNotFoundCallbackData& callbackData : resNotFoundCallbacks) {
            FilePath newWindowResPath = windowResPath;
            FilePath newWindowXmlPath = windowXmlPath;
            if (callbackData.m_callback(pControl, resPath, newWindowResPath, newWindowXmlPath)) {
                if ((newWindowResPath != windowResPath) || (newWindowXmlPath != windowXmlPath)) {
                    imageFullPath = Theme().FindExistsResFullPath(newWindowResPath, newWindowXmlPath, resPath, bLocalPath, bResPath);
                    if (!imageFullPath.IsEmpty()) {
                        //查找资源成功，终止尝试
                        break;
                    }
                }
            }
        }
    }
    ASSERT(!imageFullPath.IsEmpty() && !resPath.IsEmpty() && "Image File Not Found!");
    return imageFullPath;
}

void GlobalManager::AddResNotFoundCallback(ResNotFoundCallback callback, size_t callbackId)
{
    if (callback != nullptr) {
        ResNotFoundCallbackData data;
        data.m_callback = callback;
        data.m_callbackId = callbackId;
        m_resNotFoundCallbacks.push_back(data);
    }
}

void GlobalManager::RemoveResNotFoundCallback(size_t callbackId)
{
    auto iter = m_resNotFoundCallbacks.begin();
    while (iter != m_resNotFoundCallbacks.end()) {
        if (iter->m_callbackId == callbackId) {
            iter = m_resNotFoundCallbacks.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void GlobalManager::RemoveAllImages()
{
    AssertUIThread();
    std::vector<WindowPtr> windowList = Windows().GetAllWindowList();
    for (const WindowPtr& pWindow : windowList) {
        if (pWindow != nullptr) {
            Box* pBox = pWindow->GetRoot();
            if (pBox != nullptr) {
                pBox->ClearImageCache();
            }
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
#ifdef _DEBUG
        if (!Theme().IsSwitchingTheme()) {
            //在切换主题的时候，允许覆盖，其他情况下覆盖时断言
            auto it = m_globalClass.find(strClassName);
            if (it != m_globalClass.end()) {
                //如果出现覆盖的情况，给予断言
                ASSERT(m_globalClass[strClassName] == strControlAttrList);
            }
        }
#endif
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
    if (m_pColorManager == nullptr) {
        m_pColorManager = std::make_unique<ColorManager>();
    }
    return *m_pColorManager.get();
}

FontManager& GlobalManager::Font()
{
    return m_fontManager;
}

ImageManager& GlobalManager::Image()
{
    return m_imageManager;
}

ImageDecoderFactory& GlobalManager::ImageDecoders()
{
    return m_imageDecoderFactory;
}

IconManager& GlobalManager::Icon()
{
    return m_iconManager;
}

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

WindowManager& GlobalManager::Windows()
{
    return m_windowManager;
}

ThemeManager& GlobalManager::Theme()
{
    return m_themeManager;
}

Box* GlobalManager::CreateBox(Window* pWindow, const FilePath& strXmlPath, CreateControlCallback callback)
{
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return nullptr;
    }
    Box* pBox = nullptr;
    WindowBuilder builder;
    if (builder.ParseXmlFile(strXmlPath, pWindow->GetResourcePath())) {
        Control* pControl = builder.CreateControls(pWindow, callback);
        ASSERT(pControl != nullptr);
        if (pControl != nullptr) {
            pBox = builder.ToBox(pControl);
            ASSERT(pBox != nullptr);
            if (pBox == nullptr) {
                delete pControl;
                pControl = nullptr;
            }
        }
    }
    return pBox;
}

Box* GlobalManager::CreateBoxWithCache(Window* pWindow, const FilePath& strXmlPath, CreateControlCallback callback)
{
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return nullptr;
    }
    Box* pBox = nullptr;
    auto it = m_builderMap.find(strXmlPath);
    if (it == m_builderMap.end()) {
        WindowBuilder* builder = new WindowBuilder();
        if (builder->ParseXmlFile(strXmlPath, pWindow->GetResourcePath())) {
            Control* pControl = builder->CreateControls(pWindow, callback);
            ASSERT(pControl != nullptr);
            if (pControl != nullptr) {
                pBox = builder->ToBox(pControl);
                ASSERT(pBox != nullptr);
                if (pBox == nullptr) {
                    delete pControl;
                    pControl = nullptr;
                }
            }            
        }        
        if (pBox != nullptr) {
            m_builderMap[strXmlPath].reset(builder);
        }
        else {
            delete builder;
            builder = nullptr;
        }
    }
    else {
        Control* pControl = it->second->CreateControls(pWindow, callback);
        ASSERT(pControl != nullptr);
        if (pControl != nullptr) {
            pBox = it->second->ToBox(pControl);
            ASSERT(pBox != nullptr);
            if (pBox == nullptr) {
                delete pControl;
                pControl = nullptr;
            }
        }
    }
    ASSERT(pBox != nullptr);
    return pBox;
}

bool GlobalManager::FillBox(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback)
{
    bool bRet = false;
    ASSERT(pUserDefinedBox != nullptr);
    if (pUserDefinedBox != nullptr) {
        Window* pWindow = pUserDefinedBox->GetWindow();
        ASSERT(pWindow != nullptr);
        if (pWindow == nullptr) {
            return false;
        }
        WindowBuilder winBuilder;
        if (winBuilder.ParseXmlFile(strXmlPath, pWindow->GetResourcePath())) {
            Control* pControl = winBuilder.CreateControls(pWindow, callback, nullptr, pUserDefinedBox);
            Box* box = winBuilder.ToBox(pControl);
            bRet = box != nullptr;
        }
    }
    return bRet;
}

bool GlobalManager::FillBoxWithCache(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback)
{
    ASSERT(pUserDefinedBox != nullptr);
    if (pUserDefinedBox == nullptr) {
        return false;
    }
    Window* pWindow = pUserDefinedBox->GetWindow();
    ASSERT(pWindow != nullptr); //DPI感知功能要求，必须先关联窗口
    if (pWindow == nullptr) {
        return false;
    }    
    Box* pBox = nullptr;
    auto it = m_builderMap.find(strXmlPath);
    if (it == m_builderMap.end()) {
        WindowBuilder* winBuilder = new WindowBuilder();
        if (winBuilder->ParseXmlFile(strXmlPath, pWindow->GetResourcePath())) {
            Control* pControl = winBuilder->CreateControls(pWindow, callback, nullptr, pUserDefinedBox);
            ASSERT(pControl != nullptr);
            if (pControl != nullptr) {
                pBox = winBuilder->ToBox(pControl);
                if (pBox == nullptr) {
                    delete pControl;
                    pControl = nullptr;
                }
            }
        }        
        if (pBox != nullptr) {
            m_builderMap[strXmlPath].reset(winBuilder);
        }
        else {
            delete winBuilder;
            winBuilder = nullptr;
        }
    }
    else {
        Control* pControl = it->second->CreateControls(pWindow, callback, nullptr, pUserDefinedBox);
        ASSERT(pControl != nullptr);
        if (pControl != nullptr) {
            pBox = it->second->ToBox(pControl);
            ASSERT(pBox != nullptr);
            if (pBox == nullptr) {
                delete pControl;
                pControl = nullptr;
            }
        }        
    }
    ASSERT(pUserDefinedBox == pBox);
    return (pBox != nullptr);
}

Control* GlobalManager::CreateControl(const DString& strControlName)
{
    Control* pControl = nullptr;
    for (CreateControlCallback pfnCreateControlCallback : m_pfnCreateControlCallbackList) {
        if (pfnCreateControlCallback != nullptr) {
            pControl = pfnCreateControlCallback(strControlName);
            if (pControl != nullptr) {
                break;
            }
        }
    }
    return pControl;
}

void GlobalManager::AddCreateControlCallback(const CreateControlCallback& pfnCreateControlCallback)
{
    if (pfnCreateControlCallback != nullptr) {
        m_pfnCreateControlCallbackList.push_back(pfnCreateControlCallback);
    }
}

Box* GlobalManager::CreateBoxForXmlPreview(Window* pWindow, const FilePath& xmlFilePath, XmlPreviewAttributes& xmlPreviewAttributes)
{
    const std::vector<unsigned char> xmlFileData;
    return CreateBoxForXmlPreview(pWindow, xmlFileData, xmlPreviewAttributes, xmlFilePath);
}

Box* GlobalManager::CreateBoxForXmlPreview(Window* pWindow,
                                           const std::vector<unsigned char>& xmlFileData,
                                           XmlPreviewAttributes& xmlPreviewAttributes,
                                           const FilePath& xmlFilePath)
{
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return nullptr;
    }
    bool bParseXmlResult = false;
    Box* pBox = nullptr;
    WindowBuilder builder;
    if (!xmlFileData.empty()) {
        if (builder.ParseXmlData(xmlFileData, xmlFilePath)) {
            bParseXmlResult = true;
        }
    }
    else if (!xmlFilePath.IsEmpty()) {
        if (builder.ParseXmlFile(xmlFilePath, pWindow->GetResourcePath())) {
            bParseXmlResult = true;
        }
    }
    if (bParseXmlResult) {
        Control* pControl = builder.CreateControls(pWindow, nullptr);
        ASSERT(pControl != nullptr);
        if (pControl != nullptr) {
            pBox = builder.ToBox(pControl);
            ASSERT(pBox != nullptr);
            if (pBox == nullptr) {
                delete pControl;
                pControl = nullptr;
            }
        }
        if (pBox != nullptr) {
            xmlPreviewAttributes.m_windowAttributes.clear();
            builder.ParseWindowAttributes(xmlPreviewAttributes.m_windowAttributes);
            xmlPreviewAttributes.m_windowClassList = builder.GetWindowClassList();
            xmlPreviewAttributes.m_windowTextColorList = builder.GetWindowTextColorList();
            xmlPreviewAttributes.m_globalFontIdList = builder.GetGlobalFontIdList();
        }
    }
    return pBox;
}

bool GlobalManager::IsInUIThread() const
{
    return (m_dwUiThreadId == std::this_thread::get_id()) ? true : false;
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

void GlobalManager::SetAnimationEnabled(bool bEnable)
{
    m_bAnimationEnabled = bEnable;
}

bool GlobalManager::IsAnimationEnabled() const
{
    return m_bAnimationEnabled;
}

void GlobalManager::AddAlias(const DString& name, const DString& value)
{
    if (!name.empty() && !value.empty()) {
        m_aliasMap[name] = value;
    }
}

void GlobalManager::RemoveAlias(const DString& name)
{
    if (!m_aliasMap.empty() && !name.empty()) {
        m_aliasMap.erase(name);
    }
}

DString GlobalManager::GetAliasValue(const DString& name) const
{
    if (!m_aliasMap.empty()) {
        auto iter = m_aliasMap.find(name);
        if (iter != m_aliasMap.end()) {
            return iter->second;
        }
    }
    return DString();
}

void GlobalManager::ClearAlias()
{
    m_aliasMap.clear();
}

void GlobalManager::AddDefine(const DString& name, const DString& value)
{
    if (!name.empty() && !value.empty()) {
        m_defineMap[name] = value;
    }
}

void GlobalManager::RemoveDefine(const DString& name)
{
    if (!m_defineMap.empty() && !name.empty()) {
        m_defineMap.erase(name);
    }
}

DString GlobalManager::GetDefineValue(const DString& name) const
{
    if (!m_defineMap.empty()) {
        auto iter = m_defineMap.find(name);
        if (iter != m_defineMap.end()) {
            return iter->second;
        }
    }
    return DString();
}

void GlobalManager::ClearDefine()
{
    m_defineMap.clear();
}

void GlobalManager::ExpandDefinePlaceholders(DString& varValue) const
{
    if (m_defineMap.empty() || varValue.empty()) {
        return;
    }

    bool hasPlaceholder = true;
    size_t currentPos = 0; // 记录当前查找位置，避免重复扫描
    while (hasPlaceholder) {
        hasPlaceholder = false;
        size_t start = varValue.find(_T("${"), currentPos); // 从当前位置开始查找

        // 1. 无占位符 → 退出循环
        if (start == DString::npos) {
            break;
        }

        // 2. 有占位符，但无闭合} → 标记为无占位符，退出（避免死循环）
        size_t end = varValue.find(_T('}'), start + 2);
        if (end == DString::npos) {
            break;
        }

        // 3. 提取变量名并查找映射表
        DString varName(varValue, start + 2, end - start - 2);
        auto it = m_defineMap.find(varName);
        if (it != m_defineMap.end()) {
            // 4. 执行替换（支持空值替换）
            const DString& replaceValue = it->second;
            varValue.replace(start, end - start + 1, replaceValue);

            // 5. 重置查找位置：替换后从当前start位置继续查找（处理新插入的占位符）
            currentPos = start;
            hasPlaceholder = true; // 标记有替换，继续循环
        }
        else {
            // 6. 未找到变量 → 跳过当前占位符，从end+1开始查找下一个
            currentPos = end + 1;
            hasPlaceholder = true; // 仍有占位符未处理，继续循环
        }
    }
}

} // namespace ui
