#ifndef UI_CORE_COLOR_MAP_H_
#define UI_CORE_COLOR_MAP_H_

#include "duilib/Core/UiColor.h"
#include <unordered_map>

namespace ui 
{
/** 颜色值的管理容器
*/
class DUILIB_API ColorMap
{
public:
    /** 添加一个颜色值
    * @param [in] strName 颜色名称（如 bkcolor）
    * @param [in] strValue 颜色具体数值（如 "#FFFFFFFF" 或者 "#FFFFFF" 或者 "red"）
    */
    void AddColor(const DString& strName, const DString& strValue);

    /** 添加一个颜色值
    * @param [in] strName 颜色名称（如 bkcolor）
    * @param [in] argb 颜色具体数值, 以ARGB格式表示
    */
    void AddColor(const DString& strName, UiColor argb);

    /** 根据名称获取一个颜色的具体数值
    * @param [in] strName 要获取的颜色名称
    * @return 返回 ARGB 格式的颜色描述值
    */
    UiColor GetColor(const DString& strName) const;

    /** 删除指定的颜色属性
    * @param [in] strName 要删除的颜色名称
    */
    void RemoveColor(const DString& strName);

    /** 删除所有颜色属性
    */
    void RemoveAllColors();

private:
    /** 颜色名称与颜色值的映射关系
    */
    std::unordered_map<DString, UiColor> m_colorMap;
};

/** 标准颜色值的管理容器（ui::UiColors这个命名空间里定义的标准颜色值）
*/
class DUILIB_API StandardColorMap : private ColorMap
{
public:
    StandardColorMap();

    /** 单例对象
    */
    static const StandardColorMap& Instance();

    /** 将颜色数值字符串转换为ARGB颜色类
     * @param [in] hexColor 支持两种格式的颜色字符串：
     *  (1) 9个字符的颜色字符串，格式为#AARRGGBB
     *  (2) 7个字符的颜色字符串，格式为#RRGGBB
     */
    static UiColor HexToColor(const DString& hexColor);

    /** 获取颜色值对应的字符串, 返回该颜色对应的字符串
    * @param [in] color 颜色值
    * @return 返回颜色值对应的字符串，比如"#FF123456"
    */
    static DString ColorToHex(const UiColor& color);

public:
    /** 根据名称获取一个颜色的具体数值
    * @param[in] strName 要获取的颜色名称
    * @return 返回 ARGB 格式的颜色描述值
    */
    UiColor GetColor(const DString& strName) const;
};

} // namespace ui

#endif // UI_CORE_COLOR_MAP_H_
