#ifndef UI_CORE_GLOBALMANAGER_H_
#define UI_CORE_GLOBALMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/FontManager.h"
#include "duilib/Core/ImageManager.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace ui 
{
	class IRenderFactory;
;
/**
* @brief 全局属性管理工具类
* 用于管理一些全局属性的工具类，包含全局样式（global.xml）和语言设置等
*/
class UILIB_API GlobalManager
{
public:
	/**
	 * @brief 初始化全局设置函数
	 * @param[in] strResourcePath 资源路径位置
	 * @param[in] callback 创建自定义控件时的全局回调函数
	 * @param[in] bAdaptDpi 是否启用 DPI 适配
	 * @param[in] theme 主题目录名，默认为 themes\\default
	 * @param[in] language 使用语言，默认为 lang\\zh_CN
	 * @return 无
	 */
	static void Startup(const std::wstring& strResourcePath, const CreateControlCallback& callback, bool bAdaptDpi, const std::wstring& theme = L"themes\\default", const std::wstring& language = L"lang\\zh_CN");

	/**
	 * @brief 释放资源
	 * @return 无
	 */
	static void Shutdown();

	/**
	 * @brief 获取当前资源所在目录
	 * @return 返回当前资源所在目录
	 */
	static std::wstring GetResourcePath();

	/**
	* @brief 获取当前语言文件路径
	* @return 返回当前语言文件路径
	*/
	static std::wstring GetLanguagePath();

	/**
	 * @brief 设置皮肤资源所在目录
	 * @param[in] strPath 要设置的路径
	 * @return 无
	 */
	static void SetResourcePath(const std::wstring& strPath);

	/**
	* @brief 设置当前语言文件路径
	* @return 设置当前语言文件路径
	*/
	static void SetLanguagePath(const std::wstring& strPath);

	/**
	* 待补充
	*/
	static void AddPreMessage(Window* pWindow);

	/**
	* 待补充
	*/
	static void RemovePreMessage(Window* pWindow);

	/**
	 * @brief 重新加载皮肤资源
	 * @param[in] resourcePath 资源路径
	 * @return 无
	 */
	static void ReloadSkin(const std::wstring& resourcePath);

	/**
	 * @brief 重新加载语言资源
	 * @param[in] languagePath 资源路径
	 * @return 无
	 */
	static void ReloadLanguage(const std::wstring& languagePath, bool invalidateAll = false);

	/**
	 * @brief 获取绘制接口类对象
	 * @return 返回接口类对象指针
	 */
	static IRenderFactory* GetRenderFactory();

	/**
	 * @brief 添加一个全局 class 属性
	 * @param[in] strClassName 全局 class 名称
	 * @param[in] strControlAttrList 属性列表，需要做 XML 转义
	 * @return 无
	 */
	static void AddClass(const std::wstring& strClassName, const std::wstring& strControlAttrList);

	/**
	 * @brief 获取一个全局 class 属性的值
	 * @param[in] strClassName 全局 class 名称
	 * @return 返回字符串形式的 class 属性值
	 */
	static std::wstring GetClassAttributes(const std::wstring& strClassName);

	/**
	 * @brief 从全局属性中删除所有 class 属性
	 * @return 返回绘制区域对象
	 */
	static void RemoveAllClasss();

	/** 获取颜色管理器
	*/
	static ColorManager& GetColorManager();

	/** 获取字体管理器
	*/
	static FontManager& GetFontManager();

	/** 获取图片管理器
	*/
	static ImageManager& GetImageManager();

	/**
	 * @brief 根据 XML 创建一个 Box
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 指定布局模块的对象指针
	 */
	static Box* CreateBox(const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/**
	 * @brief 根据 XML 在缓存中查找指定 Box，如果没有则创建
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 指定布局模块的对象指针
	 */
	static Box* CreateBoxWithCache(const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/**
	 * @brief 使用 XML 填充指定 Box
	 * @param[in] pUserDefinedBox 要填充的 box 指针
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 无
	 */
	static void FillBox(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/**
	 * @brief 使用构建过的缓存填充指定 Box，如果没有则重新构建
	 * @param[in] pUserDefinedBox 要填充的 box 指针
	 * @param[in] strXmlPath XML 文件路径
	 * @param[in] callback 自定义控件的回调处理函数
	 * @return 无
	 */
	static void FillBoxWithCache(Box* pUserDefinedBox, const std::wstring& strXmlPath, CreateControlCallback callback = CreateControlCallback());

	/**
	 * @brief 自定义控件创建后的全局回调函数
	 * @param[in] strControlName 自定义控件名称
	 * @return 返回一个自定义控件的对象指针
	 */
	static Control* CreateControl(const std::wstring& strControlName);

	static void AssertUIThread();

	/**
	 * @brief 判断当前是否使用了 zip 压缩包
	 * @return 返回 true 表示使用了 zip 压缩包作为资源，false 为普通目录模式
	 */
	static bool IsUseZip();

	/**
	 * @brief 打开一个内存压缩包资源
	 * @param[in] resource_name 资源名称，由 MAKEINTRESOURCE 宏生成
	 * @param[in] resource_type 资源类型，自定义导入的资源类型名称
	 * @param[in] password 压缩包密码
	 * @return 返回 true 打开成功，返回 false 为打开失败
	 */
	static bool OpenResZip(LPCTSTR  resource_name, LPCTSTR  resource_type, const std::string& password);

	/**
	 * @brief 打开一个本地文件压缩包资源
	 * @param[in] path 压缩包文件路径
	 * @param[in] password 压缩包密码
	 * @return 返回 true 打开成功，返回 false 为打开失败
	 */
	static bool OpenResZip(const std::wstring& path, const std::string& password);

	/**
	 * @brief 获取压缩包中的内容到内存
	 * @param[in] path 要获取的文件的路径
	 * @param[out] file_data 要获取的文件的路径
	 * @return 返回 true 打开成功，返回 false 为打开失败
	 */
	static bool GetZipData(const std::wstring& path, std::vector<unsigned char>& file_data);

	/**
	 * @brief 获取文件在压缩包中的位置
	 * @param[in] path 要获取的文件路径
	 * @return 返回在压缩包中的文件位置
	 */
	static std::wstring GetZipFilePath(const std::wstring& path);

	/**
	 * @brief 根据资源加载方式，返回对应的资源路径
	 * @param[in] path 要获取的资源路径
	 * @return 可用的资源路径
	 */
	static std::wstring GetResFullPath(const std::wstring& window_res_path, const std::wstring& res_path);

	/**
	 * @brief 判断资源是否存在zip当中
	 * @param[in] path 要判断的资源路径
	 * @return 是否存在
	 */
	static bool IsZipResExist(const std::wstring& path);

private:
	/**
	 * @brief 加载全局资源
	 * @return 无
	 */
	static void LoadGlobalResource();

	/** 从缓存中删除所有图片
	 */
	static void RemoveAllImages();

private:
	static std::unique_ptr<IRenderFactory> m_renderFactory;

	static std::wstring m_pStrResourcePath; //全局的资源路径，换肤的时候修改这个变量
	static std::wstring m_pStrLanguagePath; //全局语言文件路径
	static std::vector<Window*> m_aPreMessages;
	static std::map<std::wstring, std::unique_ptr<WindowBuilder>> m_builderMap;
	static CreateControlCallback m_createControlCallback;

	static std::map<std::wstring, std::wstring> m_mGlobalClass;

	static DWORD m_dwUiThreadId;

	/** 颜色管理器
	*/
	static ColorManager m_colorManager;

	/** 字体管理器
	*/
	static FontManager m_fontManager;

	/** 图片管理器
	*/
	static ImageManager m_imageManager;
};

} // namespace ui

#endif // UI_CORE_GLOBALMANAGER_H_
