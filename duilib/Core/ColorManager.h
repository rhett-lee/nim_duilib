#ifndef UI_CORE_COLOR_MANAGER_H_
#define UI_CORE_COLOR_MANAGER_H_

#pragma once

#include "duilib/Core/UiColor.h"
#include <unordered_map>
#include <string>

namespace ui 
{
/** 颜色值的管理容器
*/
class UILIB_API ColorMap
{
public:
    /** 添加一个颜色值
    * @param[in] strName 颜色名称（如 white）
    * @param[in] strValue 颜色具体数值（如 #FFFFFFFF）
    */
    void AddColor(const std::wstring& strName, const std::wstring& strValue);

    /** 添加一个颜色值
    * @param[in] strName 颜色名称（如 white）
    * @param[in] argb 颜色具体数值, 以ARGB格式表示
    */
    void AddColor(const std::wstring& strName, UiColor argb);

    /** 根据名称获取一个颜色的具体数值
    * @param[in] strName 要获取的颜色名称
    * @return 返回 ARGB 格式的颜色描述值
    */
    UiColor GetColor(const std::wstring& strName) const;

    /** 删除所有颜色属性
    */
    void RemoveAllColors();

private:
    /** 颜色名称与颜色值的映射关系
    */
    std::unordered_map<std::wstring, UiColor> m_colorMap;
};

/** 颜色值的管理类
*/
class UILIB_API ColorManager
{
public:
    ColorManager();

    /** 将颜色数值字符串转换为ARGB颜色类
     * @param[in] strColor 支持两种颜色值：
                  (1) 颜色具体数值（如 #FFFFFFFF）
                  (2) 内置的标准颜色值，比如"blue"，参见ui::UiColors::UiColorConsts函数中的定义
     */
    static UiColor ConvertToUiColor(const std::wstring& strColor);

public:
    /** 添加一个全局颜色值
     * @param[in] strName 颜色名称（如 white）
     * @param[in] strValue 颜色具体数值（如 #FFFFFFFF）
     */
    void AddColor(const std::wstring& strName, const std::wstring& strValue);

    /** 添加一个全局颜色值
     * @param[in] strName 颜色名称（如 white）
     * @param[in] argb 颜色具体数值, 以ARGB格式表示
     */
    void AddColor(const std::wstring& strName, UiColor argb);

    /** 根据名称获取一个颜色的具体数值
     * @param[in] strName 要获取的颜色名称
     * @return 返回 ARGB 格式的颜色描述值
     */
    UiColor GetColor(const std::wstring& strName) const;

    /** 根据名称获取一个标准颜色的具体数值
     * @param[in] strName 要获取的颜色名称，比如"blue"，详细列表参见：ui::UiColors::UiColorConsts函数中的定义
     * @return 返回 ARGB 格式的颜色描述值
     */
    UiColor GetStandardColor(const std::wstring& strName) const;

    /** 删除所有颜色属性
     */
    void RemoveAllColors();

    /** 删除所有颜色属性等缓存
     */
    void Clear();

public:
	/** 获取默认禁用状态下字体颜色
	 * @return 默认禁用状态颜色的字符串表示，对应 global.xml 中指定颜色值
	 */
	const std::wstring& GetDefaultDisabledTextColor();

	/** 设置默认禁用状态下的字体颜色
	 */
	void SetDefaultDisabledTextColor(const std::wstring& strColor);

	/** 获取默认字体颜色
	 */
    const std::wstring& GetDefaultTextColor();

	/** 设置默认字体颜色
	 */
	void SetDefaultTextColor(const std::wstring& strColor);

private:
    /** 颜色名称与颜色值的映射关系
    */
    ColorMap m_colorMap;

    /** 标准颜色值映射表, 参见：UiColors::GetUiColorsString函数中的列表
    */
    ColorMap m_standardColorMap;

    /** 默认禁用状态下的字体颜色
    */
    std::wstring m_defaultDisabledTextColor;

    /** 默认正常状态的字体颜色
    */
    std::wstring m_defaultTextColor;
};

} // namespace ui

#endif // UI_CORE_COLOR_MANAGER_H_
