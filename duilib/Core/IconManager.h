#ifndef UI_CORE_ICONMANAGER_H_
#define UI_CORE_ICONMANAGER_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <map>
#include <string>
#include <vector>

#ifdef UILIB_IMPL_WINSDK

namespace ui 
{
/** HICON句柄管理器
 */
class UILIB_API IconManager
{
public:
	IconManager();
	~IconManager();
	IconManager(const IconManager&) = delete;
	IconManager& operator = (const IconManager&) = delete;

public:
	/** 获取ICON的资源字符串（可用作为图片文件路径使用）
	*/
	std::wstring GetIconString(HICON hIcon) const;

	/** 获取ICON的资源字符串（可用作为图片文件路径使用）
	*/
	std::wstring GetIconString(uint32_t id) const;

	/** 判断是否为ICON的资源字符串
	*/
	bool IsIconString(const std::wstring& str) const;

	/** 获取资源字符串对应的ICON的图标句柄，如果没有该资源，则返回nullptr
	*/
	HICON GetIcon(const std::wstring& str) const;

	/** 获取ICON资源字符串对应图标的大小
	*/
	UiSize GetIconSize(const std::wstring& str) const;

	/** 将ICON资源字符串对应的HICON句柄，解码为位图数据（32位，ARGB位图）
	*/
	bool LoadIconData(const std::wstring& str,
		              std::vector<uint8_t>& bitmapData,
		              uint32_t& imageWidth,
					  uint32_t& imageHeight) const;

public:
	/** 添加一个HICON句柄
	* @param [in] hIcon 需要加入的ICON句柄, 加入后句柄资源生命周期由该类管理
	* @return 返回该句柄对应的ID，如果失败则返回0
	*/
	uint32_t AddIcon(HICON hIcon);

	/** 获取句柄对应的ID
	* @param [in] hIcon ICON句柄
	* @return 返回该句柄对应的ID，如果找不到则返回0
	*/
	uint32_t GetIconID(HICON hIcon) const;

	/** 根据ID获取句柄
	* @param [in] id ICON句柄ID
	*/
	HICON GetIcon(uint32_t id) const;

	/** 删除一个ICON句柄
	* @param [in] hIcon 需要删除的ICON句柄，从容器删除后，该句柄的生命周期由调用方管理
	*/
	void RemoveIcon(HICON hIcon);

	/** 根据ID删除一个ICON句柄
	* @param [in] id 需要删除的ICON句柄ID，从容器删除后，该句柄的生命周期由调用方管理
	*/
	void RemoveIcon(uint32_t id);

	/** 获取ICON的大小（宽度和高度）
	* @param [in] hIcon ICON句柄
	*/
	UiSize GetIconSize(HICON hIcon) const;

private:
	/** ICON句柄资源映射表
	*/
	std::map<uint32_t, HICON> m_iconMap;

	/** 下一个ID
	*/
	uint32_t m_nNextID;

	/** ICON资源字符串前缀
	*/
	std::wstring m_prefix;
};

} //namespace ui 

#endif //UILIB_IMPL_WINSDK

#endif //UI_CORE_ICONMANAGER_H_
