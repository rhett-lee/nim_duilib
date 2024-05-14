#include "GlobalManager.h"
#include "duilib/Utils/StringUtil.h"

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

//渲染引擎的选择(目前仅支持在编译期间选择)
#include "duilib/Render/RenderConfig.h"

//备注：
//1、性能方面：LLVM编译的版本，性能明显好于Visual Studio 2022编译的版本。
//（1）LLVM编译的Debug版本，可以流畅运行，感觉不到卡顿现象；
//（2）Visual Studio 2022编译Debug版本，运行明显卡顿，速度很慢。有GIF动画时，感觉跑不动。
//2、兼容性方面：
//（1）Visual Studio 2022编译的版本，Debug和Release都没有问题；
//（2）DebugLLVM 版本有问题，对于部分智能指针的赋值接口，程序会崩溃；ReleaseLLVM暂时没发现问题。
//     比如void SkPaint::setShader(sk_sp<SkShader> shader);
//     需要改为：void SkPaint::setShader(const sk_sp<SkShader>& shader); 才能避免崩溃
//     这种类型的接口比较多，如果想要使用，就需要修改源码，然后重新编译Skia。

#if (duilib_kRenderType == duilib_kRenderType_Skia)
//Skia引擎
#include "duilib/RenderSkia/RenderFactory_Skia.h"
	#pragma comment (lib, "opengl32.lib")

#ifdef DUILIB_RENDER_SKIA_BY_LLVM
    //使用LLVM编译Skia
	#ifdef _DEBUG
		//Debug版本
		#ifdef  _WIN64
			#pragma comment (lib, "../../../skia/out/LLVM.x64.Debug/skia.lib")
		#else
			#pragma comment (lib, "../../../skia/out/LLVM.x86.Debug/skia.lib")
		#endif //_WIN64	
	#else
		//Release版本
		#ifdef  _WIN64
			#pragma comment (lib, "../../../skia/out/LLVM.x64.Release/skia.lib")
		#else
			#pragma comment (lib, "../../../skia/out/LLVM.x86.Release/skia.lib")
		#endif //_WIN64	
	#endif //_DEBUG
#else
    //使用Visual Studio 2022编译Skia
    #ifdef _DEBUG
		//Debug版本
		#ifdef  _WIN64
			#pragma comment (lib, "../../../skia/out/vs2022.x64.Debug/skia.lib")
		#else
			#pragma comment (lib, "../../../skia/out/vs2022.x86.Debug/skia.lib")
		#endif //_WIN64	
	#else
		//Release版本
		#ifdef  _WIN64
			#pragma comment (lib, "../../../skia/out/vs2022.x64.Release/skia.lib")
		#else
			#pragma comment (lib, "../../../skia/out/vs2022.x86.Release/skia.lib")
		#endif //_WIN64	
	#endif //_DEBUG
#endif //DUILIB_RENDER_SKIA_BY_LLVM

#else if(duilib_kRenderType == duilib_kRenderType_GdiPlus)
//Gdiplus引擎
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"
#include "duilib/RenderGdiPlus/RenderFactory_Gdiplus.h"

#endif

//ToolTip/日期时间等标准控件，需要初始化commctrl
#include <commctrl.h>

#include <filesystem>

namespace ui 
{

#if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
//Gdiplus引擎
static ULONG_PTR g_gdiplusToken = 0;
static Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
#endif

GlobalManager::GlobalManager():
	m_dwUiThreadId(0),
	m_pfnCreateControlCallback(nullptr)
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
	m_dwUiThreadId = GetCurrentThreadId();

	//保存回调函数
	m_pfnCreateControlCallback = callback;

	//初始化DPI感知模式，//初始化DPI值
	DpiManager& dpiManager = Dpi();
	dpiManager.InitDpiAwareness(dpiInitParam);

#if (duilib_kRenderType == duilib_kRenderType_Skia)
	//Skia渲染引擎实现
	m_renderFactory = std::make_unique<RenderFactory_Skia>();	
#else if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
	//Gdiplus渲染引擎实现
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
	m_renderFactory = std::make_unique<RenderFactory_GdiPlus>();
#endif

#ifdef UILIB_IMPL_WINSDK
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
	m_dwUiThreadId = 0;
	m_resourcePath.clear();
	m_languagePath.clear();
	m_builderMap.clear();	

#if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
	if (g_gdiplusToken != 0) {
		Gdiplus::GdiplusShutdown(g_gdiplusToken);
		g_gdiplusToken = 0;
	}	
#endif
}

const std::wstring& GlobalManager::GetResourcePath() const
{
	return m_resourcePath;
}

void GlobalManager::SetResourcePath(const std::wstring& strPath)
{
	m_resourcePath = StringHelper::NormalizeDirPath(strPath);
}

void GlobalManager::SetLanguagePath(const std::wstring& strPath)
{
	m_languagePath = StringHelper::NormalizeDirPath(strPath);
}

const std::wstring& GlobalManager::GetLanguagePath() const
{
	return m_languagePath;
}

const std::wstring& GlobalManager::GetLanguageFileName() const
{
	return m_languageFileName;
}

bool GlobalManager::ReloadResource(const ResourceParam& resParam, bool bInvalidate)
{
	AssertUIThread();
	//校验输入参数
	std::wstring strResourcePath = resParam.resourcePath;
	if (resParam.GetResType() == ResourceType::kLocalFiles) {
		//本地文件的形式，所有资源都已本地文件的形式存在
		//const LocalFilesResParam& param = static_cast<const LocalFilesResParam&>(resParam);
		ASSERT(!strResourcePath.empty());
		if (strResourcePath.empty()) {
			return false;
		}
	}
	else if (resParam.GetResType() == ResourceType::kZipFile) {
		//资源文件打包为zip压缩包，然后以本地文件的形式存在
		const ZipFileResParam& param = static_cast<const ZipFileResParam&>(resParam);
		bool bZipOpenOk = Zip().OpenZipFile(param.zipFilePath.c_str(), param.zipPassword);
		if (!bZipOpenOk) {
			ASSERT(!"OpenZipFile failed!");
			return false;
		}
	}
	else if (resParam.GetResType() == ResourceType::kResZipFile) {
		//资源文件打包为zip压缩包，然后放在exe/dll的资源文件中
		const ResZipFileResParam& param = static_cast<const ResZipFileResParam&>(resParam);
		bool bZipOpenOk = Zip().OpenResZip(param.hResModule, param.resourceName, param.resourceType, param.zipPassword);
		if (!bZipOpenOk) {
			ASSERT(!"OpenResZip failed!");
			return false;
		}
	}
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
	SetResourcePath(StringHelper::JoinFilePath(strResourcePath, resParam.themePath));

	//解析全局资源信息(默认是"global.xml"文件)
	ASSERT(!resParam.globalXmlFileName.empty());
	if (!resParam.globalXmlFileName.empty()) {
		WindowBuilder dialog_builder;
		Window paint_manager;
		dialog_builder.Create(resParam.globalXmlFileName, CreateControlCallback(), &paint_manager);
	}

	//加载多语言文件(可选)
	if (!resParam.languagePath.empty() && !resParam.languageFileName.empty()) {
		std::wstring languagePath = StringHelper::JoinFilePath(strResourcePath, resParam.languagePath);
		ReloadLanguage(languagePath, resParam.languageFileName, false);
	}
	else if (!resParam.languagePath.empty()) {
		SetLanguagePath(StringHelper::JoinFilePath(strResourcePath, resParam.languagePath));
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

bool GlobalManager::ReloadLanguage(const std::wstring& languagePath,
	                               const std::wstring& languageFileName,
	                               bool bInvalidate)
{
	AssertUIThread();
	ASSERT(!languageFileName.empty());
	if (languageFileName.empty()) {
		return false;
	}

	std::wstring newLanguagePath = GetLanguagePath();
	if (!languagePath.empty()) {
		newLanguagePath = StringHelper::NormalizeDirPath(languagePath);
	}

	//加载多语言文件，如果使用了资源压缩包则从内存中加载语言文件
	bool bReadOk = false;
	if ( (newLanguagePath.empty() || !StringHelper::IsAbsolutePath(newLanguagePath)) && 
		 m_zipManager.IsUseZip() ) {
		std::vector<unsigned char> fileData;
		std::wstring filePath = StringHelper::JoinFilePath(newLanguagePath, languageFileName);
		if (m_zipManager.GetZipData(filePath, fileData)) {
			bReadOk = m_langManager.LoadStringTable(fileData);
		}
		else {
			ASSERT(!"GetZipData failed!");
		}
	}
	else {
		std::wstring filePath = StringHelper::JoinFilePath(newLanguagePath, languageFileName);
		bReadOk = m_langManager.LoadStringTable(filePath);
	}

	if (bReadOk) {
		//保存语言文件路径
		if (!newLanguagePath.empty() && (newLanguagePath != GetLanguagePath())) {
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

bool GlobalManager::GetLanguageList(std::vector<std::pair<std::wstring, std::wstring>>& languageList,
									const std::wstring& languageNameID) const
{
	std::wstring languagePath = GetLanguagePath();
	ASSERT(!languagePath.empty());
	if (languagePath.empty()) {
		return false;
	}

	languageList.clear();
	const std::filesystem::path path{ languagePath };
	if (path.is_absolute()) {
		//绝对路径，语言文件在本地磁盘中
		for (auto const& dir_entry : std::filesystem::directory_iterator{ path }) {
			if (dir_entry.is_regular_file()) {
				languageList.push_back({ dir_entry.path().filename().c_str(), L"" });
			}
		}
		if (!languageNameID.empty()) {
			for (auto& lang : languageList) {
				const std::wstring& fileName = lang.first;
				std::wstring& displayName = lang.second;

				std::wstring filePath = StringHelper::JoinFilePath(languagePath, fileName);
				ui::LangManager langManager;
				if (langManager.LoadStringTable(filePath)) {
					displayName = langManager.GetStringViaID(languageNameID);
				}
			}
		}
	}
	else if(m_zipManager.IsUseZip()){
		//相对路径，语言文件应该都在压缩包内
		std::vector<std::wstring> fileList;
		m_zipManager.GetZipFileList(languagePath, fileList);
		for (auto const& file : fileList) {
			languageList.push_back({ file, L"" });
		}

		if (!languageNameID.empty()) {
			for (auto& lang : languageList) {
				const std::wstring& fileName = lang.first;
				std::wstring& displayName = lang.second;

				std::wstring filePath = StringHelper::JoinFilePath(languagePath, fileName);
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

std::wstring GlobalManager::GetResFullPath(const std::wstring& windowResPath, const std::wstring& resPath)
{
	if (resPath.empty() || !StringHelper::IsRelativePath(resPath)) {
		return resPath;
	}

	std::wstring imageFullPath = StringHelper::JoinFilePath(GlobalManager::GetResourcePath(), windowResPath);
	imageFullPath = StringHelper::JoinFilePath(imageFullPath, resPath);
	imageFullPath = StringHelper::NormalizeFilePath(imageFullPath);
	if (!m_zipManager.IsZipResExist(imageFullPath) && !StringHelper::IsExistsPath(imageFullPath)) {
		imageFullPath = StringHelper::JoinFilePath(GlobalManager::GetResourcePath(), resPath);
		imageFullPath = StringHelper::NormalizeFilePath(imageFullPath);
	}
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

void GlobalManager::AddClass(const std::wstring& strClassName, const std::wstring& strControlAttrList)
{
	AssertUIThread();
	ASSERT(!strClassName.empty() && !strControlAttrList.empty());
	if (!strClassName.empty() && !strControlAttrList.empty()) {
		m_globalClass[strClassName] = strControlAttrList;
	}	
}

std::wstring GlobalManager::GetClassAttributes(const std::wstring& strClassName) const
{
	AssertUIThread();
	auto it = m_globalClass.find(strClassName);
	if (it != m_globalClass.end()) {
		return it->second;
	}
	return std::wstring();
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

#ifdef UILIB_IMPL_WINSDK
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

LangManager& GlobalManager::Lang()
{
	return m_langManager;
}

Box* GlobalManager::CreateBox(const std::wstring& strXmlPath, CreateControlCallback callback)
{
	WindowBuilder builder;
	Box* box = builder.Create(strXmlPath, callback);
	ASSERT(box != nullptr);
	return box;
}

Box* GlobalManager::CreateBoxWithCache(const std::wstring& strXmlPath, CreateControlCallback callback)
{
	Box* box = nullptr;
	auto it = m_builderMap.find(strXmlPath);
	if (it == m_builderMap.end()) {
		WindowBuilder* builder = new WindowBuilder();
		box = builder->Create(strXmlPath, callback);
		if (box != nullptr) {
			m_builderMap[strXmlPath].reset(builder);
		}
		else {
			delete builder;
			builder = nullptr;
		}
	}
	else {
		box = it->second->Create(callback);
	}
	ASSERT(box != nullptr);
	return box;
}

void GlobalManager::FillBox(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback)
{
	ASSERT(pUserDefinedBox != nullptr);
	if (pUserDefinedBox != nullptr) {
		WindowBuilder winBuilder;
		Box* box = winBuilder.Create(strXmlPath, callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
		ASSERT_UNUSED_VARIABLE(box != nullptr);
	}	
}

void GlobalManager::FillBoxWithCache(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback)
{
	ASSERT(pUserDefinedBox != nullptr);
	if (pUserDefinedBox == nullptr) {
		return;
	}
	Box* box = nullptr;
	auto it = m_builderMap.find(strXmlPath);
	if (it == m_builderMap.end()) {
		WindowBuilder* winBuilder = new WindowBuilder();
		box = winBuilder->Create(strXmlPath, callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
		if (box != nullptr) {
			m_builderMap[strXmlPath].reset(winBuilder);
		}
		else {
			delete winBuilder;
			winBuilder = nullptr;
		}
	}
	else {
		box = it->second->Create(callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
	}
	ASSERT(pUserDefinedBox == box);
	ASSERT_UNUSED_VARIABLE(box != nullptr);
}

Control* GlobalManager::CreateControl(const std::wstring& strControlName)
{
	if (m_pfnCreateControlCallback) {
		return m_pfnCreateControlCallback(strControlName);
	}
	return nullptr;
}

void GlobalManager::AssertUIThread() const
{
	ASSERT(m_dwUiThreadId == ::GetCurrentThreadId());
}

} // namespace ui
