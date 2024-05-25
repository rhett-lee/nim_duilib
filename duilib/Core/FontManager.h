#ifndef UI_CORE_FONTMANAGER_H_
#define UI_CORE_FONTMANAGER_H_

#pragma once

#include "duilib/Core/UiFont.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ui 
{
/** @class FontManager
  * @brief 自定义字体管理类
  * @copyright (c) 2021, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2021/2/26
  */
class IFont;
class DpiManager;
class UILIB_API FontManager
{
public:
	FontManager();
	~FontManager();
	FontManager(const FontManager&) = delete;
	FontManager& operator = (const FontManager&) = delete;

public:
	/** 字体中英文名称转换(Skia只支持字体英文名称，不支持中文名称)
	*/
	static std::wstring GetFontEnglishName(const std::wstring& fontName);

	/** 字体的中英文名称转换(选择字体时，只能使用中文名称)
	*/
	static std::wstring GetFontSystemName(const std::wstring& fontName);

public:
	/** 添加一个字体信息，字体大小未经DPI处理
	 * @param [in] fontId 指定字体的ID标记
	 * @param [in] fontInfo 字体属性信息
	 * @param [in] bDefault 是否设置为默认字体
	 */
	bool AddFont(const std::wstring& fontId, const UiFont& fontInfo, bool bDefault);

	/** 获取字体接口, 如果通过fontId找不到字体接口，那么会继续查找m_defaultFontId字体接口
	* @param [in] fontId 字体ID
	* @param [in] dpi DPI缩放管理器，用于对字体大小进行缩放
	* @return 成功返回字体接口，外部调用不需要释放资源；如果失败则返回nullptr
	*/
	IFont* GetIFont(const std::wstring& fontId, const DpiManager& dpi);

	/** 删除所有字体, 不包含已经加载的字体文件
	 */
	void RemoveAllFonts();

public:
	/** @brief 添加一个字体文件, 添加后可以按照正常字体使用
	  * @param[in] strFontFile 字体文件名, 相对路径，字体文件的保存路径是目录："<资源路径>\font\"
	  * @param[in] strFontName 字体名
	  * @return 无返回值
	  */
	void AddFontFile(const std::wstring& strFontFile, const std::wstring& strFontName);

	/** @brief 清理所有添加的字体文件
	  * @return 无返回值
	  */
	void RemoveAllFontFiles();

private:
	/** 获取DPI缩放后实际的字体ID
	*/
	std::wstring GetDpiFontId(const std::wstring& fontId, const DpiManager& dpi) const;

private:
	/** 自定义字体数据：Key时FontID，Value是字体描述信息
	*/
	std::unordered_map<std::wstring, UiFont> m_fontIdMap;

	/** 自定义字体信息：Key是FontId
	*/
	std::unordered_map<std::wstring, IFont*> m_fontMap;

	/** 默认字体ID
	*/
	std::wstring m_defaultFontId;

	/** 字体文件的加载信息
	*/
	struct FontFileInfo
	{
		//字体路径
		std::wstring m_fontFilePath;

		//字体名称
		std::wstring m_fontName;

		//加载字体文件后的句柄
		HANDLE m_hFontFile = nullptr;
	};
	std::vector<FontFileInfo> m_fontFileInfo;
};

}
#endif //UI_CORE_FONTMANAGER_H_
