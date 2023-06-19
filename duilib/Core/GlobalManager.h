#ifndef UI_CORE_WINDOWHELPER_H_
#define UI_CORE_WINDOWHELPER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Render/UiColor.h"
#include "duilib/Render/IFont.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace ui 
{
	class IRenderFactory;
	class ImageInfo;
	class ImageLoadAttribute;

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

	/**
	 * @brief 添加一个全局颜色值提供程序使用
	 * @param[in] strName 颜色名称（如 white）
	 * @param[in] strValue 颜色具体数值（如 #FFFFFFFF）
	 * @return 无
	 */
	static void AddTextColor(const std::wstring& strName, const std::wstring& strValue);

	/**
	 * @brief 添加一个全局颜色值提供程序使用
	 * @param[in] strName 颜色名称（如 white）
	  *@param[in] argb 颜色具体数值, 以ARGB格式表示
	 * @return 无
	 */
	static void AddTextColor(const std::wstring& strName, UiColor argb);

	/**
	 * @brief 根据名称获取一个颜色的具体数值
	 * @param[in] strName 要获取的颜色名称
	 * @return 返回 DWORD 格式的颜色描述值
	 */
	static UiColor GetTextColor(const std::wstring& strName);

	/**
	 * @brief 删除所有全局颜色属性
	 * @return 无
	 */
	static void RemoveAllTextColors();

	/**
	 * @brief 检查指定图片是否已经被缓存
	 * @param[in] loadAtrribute 图片的加载属性，包含图片路径等信息
	 * @return 如果已经被缓存，则返回 ImageInfo 的智能指针对象
	 */
	static std::shared_ptr<ImageInfo> GetCachedImage(const ImageLoadAttribute& loadAtrribute);

	/**
	 * @brief 加载图片 ImageInfo 对象
	 * @param[in] loadAtrribute 图片的加载属性，包含图片路径等信息
	 * @return 返回图片 ImageInfo 对象的智能指针
	 */
	static std::shared_ptr<ImageInfo> GetImage(const ImageLoadAttribute& loadAtrribute);

	/**
	 * @brief 图片被销毁的回调
	 * @param[in] pImageInfo 图片对应的 ImageInfo 对象
	 * @return 无
	 */
	static void OnImageInfoDestroy(ImageInfo* pImageInfo);

	/**
	 * @brief 从缓存中删除所有图片
	 * @return 无
	 */
	static void RemoveAllImages();

	/** 设置是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
	   （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
	*/
	static void SetDpiScaleAllImages(bool bEnable);

	/**判断是否默认对所有图片在加载时根据DPI进行缩放
	*/
	static bool IsDpiScaleAllImages();

	/**
	 * @brief 添加一个字体
	 * @param[in] strFontId 指定字体的ID标记
	 * @param[in] strFontName 字体名称
	 * @param[in] nSize 字体大小
	 * @param[in] bBold 是否粗体
	 * @param[in] bUnderline 是否有下划线
	 * @param[in] bStrikeout 是否带有删除线
	 * @param[in] bItalic 是否倾斜
	 * @param[in] bDefault 是否默认
	 * @param[in] nWeight 字体粗细，自重，默认为 FW_NORMAL(400)
	 */
	static bool AddFont(const std::wstring& strFontId, 
						const std::wstring& strFontName, 
					    int nSize, 
					    bool bBold, 
					    bool bUnderline, 
					    bool bStrikeout, 
					    bool bItalic, 
					    bool bDefault, 
					    int nWeight = 0);

	/**
	 * @brief 根据字体ID返回一个字体对象
	 * @param[in] strFontId 字体ID
	 * @return 返回字体的 HFONT 句柄
	 */
	static HFONT GetFont(const std::wstring& strFontId);

	/** 获取字体接口
	*/
	static IFont* GetIFont(const std::wstring& strFontId);

	/**
	 * @brief 删除所有字体
	 * @return 无
	 */
	static void RemoveAllFonts();

	/**
	 * @brief 获取默认禁用状态下字体颜色
	 * @return 默认禁用状态颜色的字符串表示，对应 global.xml 中指定颜色值
	 */
	static std::wstring GetDefaultDisabledTextColor();

	/**
	 * @brief 设置默认禁用状态下的字体颜色
	 * @param[in] dwColor 字体颜色字符串值（如 white）
	 * @return 无
	 */
	static void SetDefaultDisabledTextColor(const std::wstring& strColor);

	/**
	 * @brief 获取默认字体颜色
	 * @return 默认字体颜色字符串表示，对应 global.xml 中指定颜色值
	 */
	static std::wstring GetDefaultTextColor();

	/**
	 * @brief 设置默认字体颜色
	 * @param[in] 字体颜色的字符串值，对应 global.xml 中指定颜色值
	 * @return 无
	 */
	static void SetDefaultTextColor(const std::wstring& strColor);

	/**
	 * @brief 获取默认链接字体颜色
	 * @return 链接字体颜色 DWORD 格式的色值
	 */
	static UiColor GetDefaultLinkFontColor();

	/**
	 * @brief 设置默认链接字体颜色
	 * @param[in] dwColor 默认链接字体颜色
	 * @return 无
	 */
	static void SetDefaultLinkFontColor(UiColor dwColor);

	/**
	 * @brief 获取焦点链接的默认字体颜色
	 * @return 返回焦点链接的默认字体颜色
	 */
	static UiColor GetDefaultLinkHoverFontColor();

	/**
	 * @brief 设置焦点链接的默认字体颜色
	 * @param[in] dwColor 焦点链接的默认字体颜色
	 * @return 无
	 */
	static void SetDefaultLinkHoverFontColor(UiColor dwColor);

	/**
	 * @brief 获取默认选择状态字体颜色
	 * @return 返回默认选择状态的字体颜色
	 */
	static UiColor GetDefaultSelectedBkColor();

	/**
	 * @brief 设置默认选择状态的字体颜色
	 * @param[in] dwColor 字体颜色值
	 * @return 无
	 */
	static void SetDefaultSelectedBkColor(UiColor dwColor);

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

private:
	static std::unique_ptr<IRenderFactory> m_renderFactory;
	typedef std::map<std::wstring, std::weak_ptr<ImageInfo>> MapStringToImagePtr;

	static std::wstring m_pStrResourcePath; //全局的资源路径，换肤的时候修改这个变量
	static std::wstring m_pStrLanguagePath; //全局语言文件路径
	static std::vector<Window*> m_aPreMessages;
	static std::map<std::wstring, std::unique_ptr<WindowBuilder>> m_builderMap;
	static CreateControlCallback m_createControlCallback;

	static MapStringToImagePtr m_mImageHash;

	/** 设置是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
	   （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
	*/
	static bool m_bDpiScaleAllImages;

	static std::map<std::wstring, UiColor> m_mapTextColor;
	static std::map<std::wstring, std::wstring> m_mGlobalClass;

	static std::wstring m_strDefaultDisabledColor;
	static std::wstring m_strDefaultFontColor;
	static UiColor m_dwDefaultLinkFontColor;
	static UiColor m_dwDefaultLinkHoverFontColor;
	static UiColor m_dwDefaultSelectedBkColor;

	static DWORD m_dwUiThreadId;

	/** 可选字体列表
	*/
	static std::map<std::wstring, IFont*> m_mCustomFonts;

	/** 默认字体ID
	*/
	static std::wstring m_sDefaultFontId;
};

} // namespace ui

#endif // UI_CORE_WINDOWHELPER_H_
