#include "GlobalManager.h"
#include "duilib/Render/Font_GDI.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/MultiLangSupport.h"
#include "duilib/Utils/VersionHelpers.h"

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageDecoder.h"

#include "duilib/third_party/unzip/UnZip.h"

//渲染引擎的选择(目前仅支持在编译期间选择)
#include "duilib/Render/RenderConfig.h"
#if (duilib_kRenderType == duilib_kRenderType_Skia)
//Skia引擎
#include "duilib/RenderSkia/RenderFactory_Skia.h"
	#pragma comment (lib, "opengl32.lib")
#ifdef _DEBUG
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
	#pragma comment (lib, "../../../../develop/skia/skia/out/DebugLLVM/skia.lib")
#else
	#pragma comment (lib, "../../../../develop/skia/skia/out/ReleaseLLVM/skia.lib")
#endif

#else if(duilib_kRenderType == duilib_kRenderType_GdiPlus)
//Gdiplus引擎
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"
#include "duilib/RenderGdiPlus/RenderFactory_Gdiplus.h"

#endif

#include <commctrl.h>
#include <tchar.h>
#include <unordered_set>
#include <shlwapi.h>

namespace ui 
{

namespace {

	//查找对应DPI下的图片，可以每个DPI设置一个图片，以提高不同DPI下的图片质量
    std::wstring GetDpiImageFullPath(const std::wstring& strImageFullPath, bool bIsUseZip)
    {
        UINT dpi = DpiManager::GetInstance()->GetScale();
        if (dpi == 100) {
			//当前DPI无需缩放
            return strImageFullPath;
        }

        std::wstring strPathDir;
        std::wstring strPathFileName;
        std::list<std::wstring> strPathList = StringHelper::Split(strImageFullPath, L"\\");
        for (auto it = strPathList.begin(); it != strPathList.end(); ++it) {
            auto itTemp = it;
            if (++itTemp == strPathList.end()) {
                strPathFileName = *it;
            }
            else {
                strPathDir += *it + L"\\";
            }
        }

        size_t iPointPos = strPathFileName.rfind('.');
        ASSERT(iPointPos != std::wstring::npos);
        if (iPointPos == std::wstring::npos)
        {
            return std::wstring();
        }
        std::wstring strFileExtension = strPathFileName.substr(iPointPos, strPathFileName.length() - iPointPos);
        std::wstring strFile = strPathFileName.substr(0, iPointPos);
		//返回指定DPI下的图片，举例DPI为120的图片："image.png" 对应于 "image@120.png"
        strPathFileName = StringHelper::Printf(L"%s%s%d%s", strFile.c_str(), L"@", dpi, strFileExtension.c_str());
        std::wstring strNewFilePath = strPathDir + strPathFileName;
        if (bIsUseZip) {
            bool hasData = ui::GlobalManager::IsZipResExist(strNewFilePath);
            return hasData ? strNewFilePath : strImageFullPath;
        }
		else {
			const DWORD file_attr = ::GetFileAttributesW(strNewFilePath.c_str());
			return file_attr != INVALID_FILE_ATTRIBUTES ? strNewFilePath : strImageFullPath;
		}        
    }
}

std::wstring GlobalManager::m_pStrResourcePath;
std::wstring GlobalManager::m_pStrLanguagePath;
std::vector<Window*> GlobalManager::m_aPreMessages;
std::map<std::wstring, std::unique_ptr<WindowBuilder>> GlobalManager::m_builderMap;
CreateControlCallback GlobalManager::m_createControlCallback;

GlobalManager::MapStringToImagePtr GlobalManager::m_mImageHash;
bool GlobalManager::m_bDpiScaleAllImages = true;
std::map<std::wstring, UiColor> GlobalManager::m_mapTextColor;
std::map<std::wstring, std::wstring> GlobalManager::m_mGlobalClass;

std::map<std::wstring, IFont*> GlobalManager::m_mCustomFonts;
std::wstring GlobalManager::m_sDefaultFontId;

std::wstring GlobalManager::m_strDefaultDisabledColor = L"textdefaultdisablecolor";
std::wstring GlobalManager::m_strDefaultFontColor = L"textdefaultcolor";
UiColor GlobalManager::m_dwDefaultLinkFontColor = UiColor(0xFF0000FF);
UiColor GlobalManager::m_dwDefaultLinkHoverFontColor = UiColor(0xFFD3215F);
UiColor GlobalManager::m_dwDefaultSelectedBkColor = UiColor(0xFFBAE4FF);

std::unique_ptr<IRenderFactory> GlobalManager::m_renderFactory;
DWORD GlobalManager::m_dwUiThreadId = 0;

#if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
//Gdiplus引擎
static ULONG_PTR g_gdiplusToken;
static Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
#endif

static HZIP g_hzip = NULL;
const std::wstring kLanguageFileName = L"gdstrings.ini";

void GlobalManager::Startup(const std::wstring& strResourcePath, const CreateControlCallback& callback, bool bAdaptDpi, const std::wstring& theme, const std::wstring& language)
{
	ASSERT(m_renderFactory == nullptr);
	m_dwUiThreadId = GetCurrentThreadId();

#if (duilib_kRenderType == duilib_kRenderType_Skia)
	m_renderFactory = std::make_unique<RenderFactory_Skia>();	
#else if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
	m_renderFactory = std::make_unique<RenderFactory_GdiPlus>();
#endif
	ASSERT(m_renderFactory != nullptr);

	GlobalManager::SetResourcePath(strResourcePath + theme);
	m_createControlCallback = callback;

    // 适配DPI
	DpiManager::GetInstance()->SetAdaptDPI(bAdaptDpi);
	if (bAdaptDpi) {
		DpiManager::GetInstance()->SetScale(DpiManager::GetMainMonitorDPI());
	}

    // 解析全局资源信息
	LoadGlobalResource();

	SetLanguagePath(strResourcePath + language);

    // 加载多语言文件，如果使用了资源压缩包则从内存中加载语言文件
	if (g_hzip) {
		std::vector<unsigned char> file_data;
		if (GetZipData(GetLanguagePath() + L"\\" + kLanguageFileName, file_data)) {
			MultiLangSupport::GetInstance()->LoadStringTable(file_data);
		}
	}
	else {
		MultiLangSupport::GetInstance()->LoadStringTable(GetLanguagePath() + L"\\" + kLanguageFileName);
	}
#if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif
	// Boot Windows Common Controls (for the ToolTip control)
	::InitCommonControls();
}

void GlobalManager::Shutdown()
{
	if (g_hzip)
	{
		CloseZip(g_hzip);
		g_hzip = NULL;
	}
	m_renderFactory.reset();
	RemoveAllFonts();
#if (duilib_kRenderType == duilib_kRenderType_GdiPlus)
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
#endif
}

std::wstring GlobalManager::GetResourcePath()
{
	return m_pStrResourcePath;
}

std::wstring GlobalManager::GetLanguagePath()
{
	return m_pStrLanguagePath;
}

void GlobalManager::SetResourcePath(const std::wstring& strPath)
{
	m_pStrResourcePath = strPath;
	if (!m_pStrResourcePath.empty()) {
		//确保路径最后字符是分割字符
		TCHAR cEnd = m_pStrResourcePath.back();
		if (cEnd != _T('\\') && cEnd != _T('/')) {
			m_pStrResourcePath += _T('\\');
		}
	}
}

void GlobalManager::SetLanguagePath(const std::wstring& strPath)
{
	m_pStrLanguagePath = strPath;
}

void GlobalManager::LoadGlobalResource()
{
	WindowBuilder dialog_builder;
	Window paint_manager;
	dialog_builder.Create(L"global.xml", CreateControlCallback(), &paint_manager);
}

void GlobalManager::AddPreMessage(Window* pWindow)
{
	m_aPreMessages.push_back(pWindow);
}

void GlobalManager::RemovePreMessage(Window* pWindow)
{
	auto it = std::find(m_aPreMessages.begin(), m_aPreMessages.end(), pWindow);
	if (it != m_aPreMessages.end()) {
		m_aPreMessages.erase(it);
	}
}

void GlobalManager::ReloadSkin(const std::wstring& resourcePath)
{
	RemoveAllFonts();
	RemoveAllTextColors();
	RemoveAllClasss();
	RemoveAllImages();

	SetResourcePath(resourcePath);
	LoadGlobalResource();

	for (auto it = m_aPreMessages.begin(); it != m_aPreMessages.end(); it++) {
		(*it)->GetRoot()->Invalidate();
	}
}

void GlobalManager::ReloadLanguage(const std::wstring& languagePath, bool invalidateAll) 
{
	if (GetLanguagePath() != languagePath) {
		SetLanguagePath(languagePath);

		MultiLangSupport::GetInstance()->LoadStringTable(languagePath + L"\\" + kLanguageFileName);

		if (invalidateAll) {
			for (auto it = m_aPreMessages.begin(); it != m_aPreMessages.end(); it++) {
				(*it)->GetRoot()->Invalidate();
			}
		}
	}

}

IRenderFactory* GlobalManager::GetRenderFactory()
{
	return m_renderFactory.get();
}

void GlobalManager::AddClass(const std::wstring& strClassName, const std::wstring& strControlAttrList)
{
	ASSERT(!strClassName.empty());
	ASSERT(!strControlAttrList.empty());
	m_mGlobalClass[strClassName] = strControlAttrList;
}

std::wstring GlobalManager::GetClassAttributes(const std::wstring& strClassName)
{
	auto it = m_mGlobalClass.find(strClassName);
	if (it != m_mGlobalClass.end()) {
		return it->second;
	}

	return L"";
}

void GlobalManager::RemoveAllClasss()
{
	m_mGlobalClass.clear();
}

void GlobalManager::AddTextColor(const std::wstring& strName, const std::wstring& strValue)
{
	std::wstring strColor = strValue.substr(1);
	LPTSTR pstr = NULL;
	UiColor::ARGB dwBackColor = _tcstoul(strColor.c_str(), &pstr, 16);
	AddTextColor(strName, UiColor(dwBackColor));
}

void GlobalManager::AddTextColor(const std::wstring& strName, UiColor argb)
{
	m_mapTextColor[strName] = argb;
}

UiColor GlobalManager::GetTextColor(const std::wstring& strName)
{
	auto it = m_mapTextColor.find(strName);
	if (it != m_mapTextColor.end()) {
		return it->second;
	}
	return UiColor();
}

void GlobalManager::RemoveAllTextColors()
{
	m_mapTextColor.clear();
}

std::shared_ptr<ImageInfo> GlobalManager::GetCachedImage(const ImageLoadAttribute& loadAtrribute)
{
	std::wstring cacheKey = loadAtrribute.GetCacheKey();
	std::shared_ptr<ImageInfo> sharedImage;
	auto it = m_mImageHash.find(cacheKey);
	if (it != m_mImageHash.end()) {
		sharedImage = it->second.lock();
	}
	return sharedImage;
}

std::shared_ptr<ImageInfo> GlobalManager::GetImage(const ImageLoadAttribute& loadAtrribute)
{
    std::wstring imageCacheKey = loadAtrribute.GetCacheKey();
    auto it = m_mImageHash.find(imageCacheKey);
	if (it != m_mImageHash.end()) {
		std::shared_ptr<ImageInfo> sharedImage = it->second.lock();
		if (sharedImage) {
			//从缓存中，找到有效图片资源，直接返回
			return sharedImage;
		}
	}
	
	//重新加载资源
	std::wstring imageFullPath = GetDpiImageFullPath(loadAtrribute.GetImageFullPath(), IsUseZip());
	std::vector<unsigned char> file_data;
    if (IsUseZip()) {
		GetZipData(imageFullPath, file_data);
    }
	else {
		FILE* f = nullptr;
		errno_t ret = _wfopen_s(&f, imageFullPath.c_str(), L"rb");
		if ((ret == 0) && (f != nullptr)) {
			fseek(f, 0, SEEK_END);
			int fileSize = ftell(f);
			fseek(f, 0, SEEK_SET);
			if (fileSize > 0) {
				file_data.resize((size_t)fileSize);
				size_t readLen = fread(file_data.data(), 1, file_data.size(), f);
				ASSERT_UNUSED_VARIABLE(readLen == file_data.size());
				if (readLen != file_data.size()) {
					file_data.clear();
				}
			}
			fclose(f);
		}
	}
	//标记DPI自适应图片属性，如果路径不同，说明已经选择了对应DPI下的文件
	bool isDpiScaledImageFile = imageFullPath != loadAtrribute.GetImageFullPath();
	std::unique_ptr<ImageInfo> imageInfo;
	if (!file_data.empty()) {
		ImageDecoder imageDecoder;
		ImageLoadAttribute imageLoadAtrribute(loadAtrribute);
		if (isDpiScaledImageFile) {
			imageLoadAtrribute.SetNeedDpiScale(false);
		}
		imageInfo = imageDecoder.LoadImageData(file_data, imageLoadAtrribute);
	}
	std::shared_ptr<ImageInfo> sharedImage;
	if (imageInfo) {
		sharedImage.reset(imageInfo.release(), &OnImageInfoDestroy);
		sharedImage->SetCacheKey(imageCacheKey);
		if (isDpiScaledImageFile) {
			//使用了DPI自适应的图片，做标记
			sharedImage->SetBitmapSizeDpiScaled(true);
		}
		m_mImageHash[imageCacheKey] = sharedImage;
	}    
    return sharedImage;
}

void GlobalManager::OnImageInfoDestroy(ImageInfo* pImageInfo)
{
	ASSERT(pImageInfo != nullptr);
	if (pImageInfo != nullptr) {
		if (!pImageInfo->GetCacheKey().empty()) {
			auto it = m_mImageHash.find(pImageInfo->GetCacheKey());
			if (it != m_mImageHash.end()) {
				m_mImageHash.erase(it);
			}
		}
	}
	delete pImageInfo;
}

void GlobalManager::RemoveAllImages()
{
	for (auto it = m_aPreMessages.begin(); it != m_aPreMessages.end(); it++) {
		(*it)->GetRoot()->ClearImageCache();
	}

	m_mImageHash.clear();
}

void GlobalManager::SetDpiScaleAllImages(bool bEnable)
{
	m_bDpiScaleAllImages = bEnable;
}

bool GlobalManager::IsDpiScaleAllImages()
{
	return m_bDpiScaleAllImages;
}

bool GlobalManager::AddFont(const std::wstring& strFontId, 
						    const std::wstring& strFontName, 
							int nSize, 
							bool bBold,
	                        bool bUnderline, 
							bool bStrikeout, 
						    bool bItalic, 
							bool bDefault, 
						    int nWeight)
{
	ASSERT(!strFontId.empty());
	if (strFontId.empty()) {
		return false;
	}

	auto iter = m_mCustomFonts.find(strFontId);
	ASSERT(iter == m_mCustomFonts.end());
	if (iter != m_mCustomFonts.end()) {
		return false;
	}

	static bool bOsOverXp = IsWindowsVistaOrGreater();
	std::wstring fontName = strFontName;
	if (fontName == L"system") {
		//字体使用英文名称，保持兼容性
		fontName = bOsOverXp ? L"Microsoft YaHei" : L"SimSun";
	}

	LOGFONT lf = { 0 };
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	wcscpy_s(lf.lfFaceName, fontName.c_str());
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = -DpiManager::GetInstance()->ScaleInt(nSize);
	if (bUnderline) {
		lf.lfUnderline = TRUE;
	}
	if (bStrikeout) {
		lf.lfStrikeOut = TRUE;
	}
	if (bItalic) {
		lf.lfItalic = TRUE;
	}
	if (nWeight > 0) {
		lf.lfWeight = nWeight;
	}
	if (bBold) {
		if (lf.lfWeight < FW_BOLD) {
			lf.lfWeight = FW_BOLD;
		}		
	}

	Font_GDI* pGdiFont = new Font_GDI(lf);
	if (pGdiFont->GetFontHandle() == nullptr) {
		delete pGdiFont;
		return false;
	}
	m_mCustomFonts.insert(std::make_pair(strFontId, pGdiFont));
	if (bDefault) {
		m_sDefaultFontId = strFontId;
	}
	return true;
}

HFONT GlobalManager::GetFont(const std::wstring& strFontId)
{
	auto iter = m_mCustomFonts.find(strFontId);
	if (iter == m_mCustomFonts.end()) {
		//如果找不到，则用默认字体
		iter = m_mCustomFonts.find(m_sDefaultFontId);
	}
	if (iter != m_mCustomFonts.end()) {
		Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(iter->second);
		if (pGdiFont != nullptr) {
			return pGdiFont->GetFontHandle();
		}
	}
	return nullptr;
}

IFont* GlobalManager::GetIFont(const std::wstring& strFontId)
{
	auto iter = m_mCustomFonts.find(strFontId);
	if (iter == m_mCustomFonts.end()) {
		//如果找不到，则用默认字体
		iter = m_mCustomFonts.find(m_sDefaultFontId);
	}
	IFont* pFont = nullptr;
	if (iter != m_mCustomFonts.end()) {
		pFont = iter->second;
	}
	return pFont;
}

void GlobalManager::RemoveAllFonts()
{
	for (auto iter : m_mCustomFonts) {
		delete iter.second;
	}
	m_mCustomFonts.clear();
}

std::wstring GlobalManager::GetDefaultDisabledTextColor()
{
	return m_strDefaultDisabledColor;
}

void GlobalManager::SetDefaultDisabledTextColor(const std::wstring& strColor)
{
	m_strDefaultDisabledColor = strColor;
}

std::wstring GlobalManager::GetDefaultTextColor()
{
	return m_strDefaultFontColor;
}

void GlobalManager::SetDefaultTextColor(const std::wstring& strColor)
{
	m_strDefaultFontColor = strColor;
}

UiColor GlobalManager::GetDefaultLinkFontColor()
{
	return m_dwDefaultLinkFontColor;
}

void GlobalManager::SetDefaultLinkFontColor(UiColor strColor)
{
	m_dwDefaultLinkFontColor = strColor;
}

UiColor GlobalManager::GetDefaultLinkHoverFontColor()
{
	return m_dwDefaultLinkHoverFontColor;
}

void GlobalManager::SetDefaultLinkHoverFontColor(UiColor dwColor)
{
	m_dwDefaultLinkHoverFontColor = dwColor;
}

UiColor GlobalManager::GetDefaultSelectedBkColor()
{
	return m_dwDefaultSelectedBkColor;
}

void GlobalManager::SetDefaultSelectedBkColor(UiColor dwColor)
{
	m_dwDefaultSelectedBkColor = dwColor;
}

Box* GlobalManager::CreateBox(const std::wstring& strXmlPath, CreateControlCallback callback)
{
	WindowBuilder builder;
	Box* box = builder.Create(strXmlPath, callback);
	ASSERT(box);

	return box;
}

Box* GlobalManager::CreateBoxWithCache(const std::wstring& strXmlPath, CreateControlCallback callback)
{
	Box* box = nullptr;
	auto it = m_builderMap.find(strXmlPath);
	if (it == m_builderMap.end()) {
		WindowBuilder* builder = new WindowBuilder();
		box = builder->Create(strXmlPath, callback);
		if (box) {
			m_builderMap[strXmlPath].reset(builder);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else {
		box = it->second->Create(callback);
	}

	return box;
}

void GlobalManager::FillBox(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback)
{
	ASSERT(pUserDefinedBox != nullptr);
	if (pUserDefinedBox == nullptr) {
		return;
	}
	WindowBuilder winBuilder;
	Box* box = winBuilder.Create(strXmlPath, callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
	(void)box;
	ASSERT(box);

	return;
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
		if (box) {
			m_builderMap[strXmlPath].reset(winBuilder);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else {
		box = it->second->Create(callback, pUserDefinedBox->GetWindow(), nullptr, pUserDefinedBox);
	}
}

Control* GlobalManager::CreateControl(const std::wstring& strControlName)
{
	if (m_createControlCallback) {
		return m_createControlCallback(strControlName);
	}

	return nullptr;
}

bool GlobalManager::IsUseZip()
{
	return g_hzip != NULL;
}

bool GlobalManager::OpenResZip(LPCTSTR  resource_name, LPCTSTR  resource_type, const std::string& password)
{
	HRSRC rsc = FindResource(NULL, resource_name, resource_type);
	HGLOBAL resource = LoadResource(NULL, rsc);
	DWORD   size = SizeofResource(NULL, rsc);
	if (resource && size > 0)
	{

	}

	if (g_hzip)
	{
		CloseZip(g_hzip);
		g_hzip = NULL;
	}
	g_hzip = OpenZip(resource, size, password.c_str());
	return g_hzip != NULL;
}

bool GlobalManager::OpenResZip(const std::wstring& path, const std::string& password)
{
	if (g_hzip)
	{
		CloseZip(g_hzip);
		g_hzip = NULL;
	}
	g_hzip = OpenZip(path.c_str(), password.c_str());
	return g_hzip != NULL;
}

bool GlobalManager::GetZipData(const std::wstring& path, std::vector<unsigned char>& file_data)
{
	ASSERT(g_hzip != nullptr);	
	std::wstring file_path = GetZipFilePath(path);
	ASSERT(!file_path.empty());
	if ((g_hzip == nullptr) || file_path.empty())
	{
		return false;
	}

	AssertUIThread();

	ZIPENTRY ze = {0};
	int i = 0;
	if (FindZipItem(g_hzip, file_path.c_str(), true, &i, &ze) == ZR_OK) {
		if ((ze.index >= 0) && (ze.unc_size > 0)){
			file_data.resize(ze.unc_size);
			ZRESULT res = UnzipItem(g_hzip, ze.index, file_data.data(), ze.unc_size);
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

std::wstring GlobalManager::GetZipFilePath(const std::wstring& path)
{
	if (!::PathIsRelative(path.c_str()))
		return L"";

	std::wstring file_path = path;
	StringHelper::ReplaceAll(L"\\", L"/", file_path);
	StringHelper::ReplaceAll(L"//", L"/", file_path);
	for (unsigned int i = 0; i < file_path.size();)
	{
		bool start_node = false;
		if (i == 0 || file_path.at(i - 1) == L'/')
		{
			start_node = true;
		}
		WCHAR wch = file_path.at(i);
		if (start_node && wch == L'/')//"//"
		{
			file_path.erase(i, 1);
			continue;
		}
		if (start_node && wch == L'.')
		{
			if (i + 1 < file_path.size() && file_path.at(i + 1) == L'/')// "./"
			{
				file_path.erase(i, 2);
				continue;
			}
			else if (i + 2 < file_path.size() && file_path.at(i + 1) == L'.' && file_path.at(i + 2) == L'/')// "../"
			{
				file_path.erase(i, 2);
				int i_erase = i - 2;
				if (i_erase < 0)
				{
					ASSERT(0);
				}
				while (i_erase > 0 && file_path.at(i_erase) != L'/')
					i_erase--;
				file_path.erase(i_erase, i - i_erase);
				i = i_erase;
				continue;
			}
		}
		i++;
	}
	return file_path;
}

std::wstring GlobalManager::GetResFullPath(const std::wstring& window_res_path, const std::wstring& res_path)
{
	if (res_path.empty() || !::PathIsRelative(res_path.c_str())){
		return res_path;
	}

	std::wstring imageFullPath = GlobalManager::GetResourcePath() + window_res_path + res_path;
	imageFullPath = StringHelper::ReparsePath(imageFullPath);

	if (!GlobalManager::IsZipResExist(imageFullPath) && !::PathFileExists(imageFullPath.c_str())) {
		imageFullPath = GlobalManager::GetResourcePath() + res_path;
		imageFullPath = StringHelper::ReparsePath(imageFullPath);
	}
	return imageFullPath;
}

bool GlobalManager::IsZipResExist(const std::wstring& path)
{
	AssertUIThread();

	if (g_hzip && !path.empty()) {
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
		bool find = FindZipItem(g_hzip, file_path.c_str(), true, &i, &ze) == ZR_OK;
		if (find) {
			zip_path_cache.insert(path);
		}
		return find;
	}
	return false;
}

void GlobalManager::AssertUIThread()
{
#ifdef _DEBUG
	ASSERT(m_dwUiThreadId == GetCurrentThreadId());
#endif
}

} // namespace ui
