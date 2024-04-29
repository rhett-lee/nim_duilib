#ifndef UI_CORE_GLOBALMANAGER_H_
#define UI_CORE_GLOBALMANAGER_H_

#pragma once

#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/FontManager.h"
#include "duilib/Core/ImageManager.h"
#include "duilib/Core/IconManager.h"
#include "duilib/Core/ZipManager.h"
#include "duilib/Core/LangManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/TimerManager.h"

#include <string>
#include <vector>
#include <map>

namespace ui 
{
	class IRenderFactory;
;
/**
* 全局属性管理工具类
* 用于管理一些全局属性的工具类，包含全局样式（global.xml）和语言设置等
*/
class UILIB_API GlobalManager
{
private:
	GlobalManager();
	~GlobalManager();
	GlobalManager(const GlobalManager&) = delete;
	GlobalManager& operator = (const GlobalManager&) = delete;

public:
	/** 获取单例对象
	*/
	static GlobalManager& Instance();

public:
	/** 初始化全局设置函数
	 * @param[in] strResourcePath 资源路径位置
	 * @param[in] callback 创建自定义控件时的全局回调函数
	 * @param[in] bAdaptDpi 是否启用 DPI 适配
	 * @param[in] theme 主题目录名，默认为 themes\\default
	 * @param[in] language 使用语言，默认为 lang\\zh_CN
	 */
	void Startup(const std::wstring& strResourcePath, const CreateControlCallback& callback, bool bAdaptDpi, const std::wstring& theme = L"themes\\default", const std::wstring& language = L"lang\\zh_CN");

	/** 释放全局资源
	 */
	void Shutdown();

public:
	/** 设置皮肤资源所在目录
	 */
	void SetResourcePath(const std::wstring& strPath);

	/** 获取当前资源所在目录
	 */
	const std::wstring& GetResourcePath();

	/** 设置当前语言文件路径
	*/
	void SetLanguagePath(const std::wstring& strPath);

	/** 获取当前语言文件路径
	*/
	const std::wstring& GetLanguagePath();

	/** 根据资源加载方式，返回对应的资源路径
	 * @param[in] path 要获取的资源路径
	 * @param [in] windowResPath 窗口对应的资源相对目录，比如："basic\"
	 * @param [in] resPath 资源文件路径，比如："../public/button/btn_wnd_gray_min_hovered.png"
	 * @return 可用的完整的资源路径，比如：
	          （1）如果是使用ZIP压缩包，返回："resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
			  （2）如果未使用ZIP压缩包，返回："D:\<程序所在目录>\resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
	 */
	std::wstring GetResFullPath(const std::wstring& windowResPath, const std::wstring& resPath);

	/** 重新加载皮肤资源
	 * @param[in] resourcePath 资源路径
	 */
	void ReloadSkin(const std::wstring& resourcePath);

	/** 重新加载语言资源
	 * @param[in] languagePath 语言资源路径
	 */
	void ReloadLanguage(const std::wstring& languagePath, bool invalidateAll = false);

	/** 添加一个窗口接口（主要用于换肤、切换语言之后的重绘、资源同步等操作）
	*/
	void AddWindow(Window* pWindow);

	/** 移除一个窗口
	*/
	void RemoveWindow(Window* pWindow);

	/** 获取绘制接口类对象
	 */
	IRenderFactory* GetRenderFactory();

	/** 添加一个全局 Class 属性
	 * @param[in] strClassName 全局 Class 名称
	 * @param[in] strControlAttrList 属性列表，需要做 XML 转义
	 */
	void AddClass(const std::wstring& strClassName, const std::wstring& strControlAttrList);

	/** 获取一个全局 class 属性的值
	 * @param[in] strClassName 全局 class 名称
	 * @return 返回字符串形式的 class 属性值
	 */
	std::wstring GetClassAttributes(const std::wstring& strClassName);

	/** 从全局属性中删除所有 class 属性
	 * @return 返回绘制区域对象
	 */
	void RemoveAllClasss();

public:
	/** 获取颜色管理器
	*/
	ColorManager& Color();

	/** 获取字体管理器
	*/
	FontManager& Font();

	/** 获取图片管理器
	*/
	ImageManager& Image();

#ifdef UILIB_IMPL_WINSDK
	/** 获取ICON资源管理器
	*/
	IconManager& Icon();

	/** RichEdit依赖的DLL, 加载并返回句柄
	*/
	HMODULE GetRichEditModule();

#endif

	/** 获取Zip管理器
	*/
	ZipManager& Zip();

	/** 获取DPI管理器
	*/
	DpiManager& Dpi();

	/** 获取定时器管理器
	*/
	TimerManager& Timer();

	/** 多语言支持管理器
	*/
	LangManager& Lang();

public:
	/** 根据 XML 创建一个 Box
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 指定布局模块的对象指针
	 */
	Box* CreateBox(const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/** 根据 XML 在缓存中查找指定 Box，如果没有则创建
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 指定布局模块的对象指针
	 */
	Box* CreateBoxWithCache(const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/** 使用 XML 填充指定 Box
	 * @param[in] pUserDefinedBox 要填充的 box 指针
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 */
	void FillBox(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/** 使用构建过的缓存填充指定 Box，如果没有则重新构建
	 * @param[in] pUserDefinedBox 要填充的 box 指针
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 */
	void FillBoxWithCache(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/** 自定义控件创建后的全局回调函数
	 * @param[in] strControlName 自定义控件名称
	 * @return 返回一个自定义控件的对象指针
	 */
	Control* CreateControl(const std::wstring& strControlName);

	/** 如果该函数不是在UI线程中调用的，进行断言报错
	*/
	void AssertUIThread();

private:
	/**
	 * @brief 加载全局资源
	 * @return 无
	 */
	void LoadGlobalResource();

	/** 从缓存中删除所有图片
	 */
	void RemoveAllImages();

private:

	/** 渲染引擎管理接口
	*/
	std::unique_ptr<IRenderFactory> m_renderFactory;

	/** 全局的资源路径，换肤的时候修改这个变量
	*/
	std::wstring m_resourcePath; 

	/** 全局语言文件路径
	*/
	std::wstring m_languagePath; 
	
	/** 窗口构建管理接口，KEY是XML文件路径，VALUE是窗口构建管理接口（已经解析后的XML，可避免重复解析）
	*/
	std::map<std::wstring, std::unique_ptr<WindowBuilder>> m_builderMap;

	/** 控件创建函数，用于用户自定义控件的创建
	*/
	CreateControlCallback m_pfnCreateControlCallback;

	/** 每个Class的名称(KEY)和属性列表(VALUE)（比如global.xml中定义的Class）
	*/
	std::map<std::wstring, std::wstring> m_globalClass;

	/** 主线程ID
	*/
	uint32_t m_dwUiThreadId;

	/** 所有的窗口列表
	*/
	std::vector<Window*> m_windowList;

	/** 颜色管理器
	*/
	ColorManager m_colorManager;

	/** 字体管理器
	*/
	FontManager m_fontManager;

	/** 图片管理器
	*/
	ImageManager m_imageManager;

	/** ZIP压缩包管理器
	*/
	ZipManager m_zipManager;

	/** DPI管理器
	*/
	DpiManager m_dpiManager;

	/** 定时器管理器
	*/
	TimerManager m_timerManager;

	/** 多语言管理器
	*/
	LangManager m_langManager;

#ifdef UILIB_IMPL_WINSDK
	/** ICON资源管理器
	*/
	IconManager m_iconManager;

	/** RichEdit依赖的DLL, 加载并返回句柄
	*/
	HMODULE m_hRichEditModule;
#endif
};

} // namespace ui

#endif // UI_CORE_GLOBALMANAGER_H_
