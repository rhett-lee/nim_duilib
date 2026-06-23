#include "ColorManager.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{
ColorManager::ColorManager():
    m_bColorThemeDarkMode(false)
{
}

UiColor ColorManager::ConvertToUiColor(const DString& strColor)
{
    ASSERT(!strColor.empty());
    if (strColor.empty()) {
        return UiColor();
    }
    if (strColor.at(0) == _T('#')) {
        //十六位数字表示的颜色值
        return StandardColorMap::HexToColor(strColor);
    }
    else {
        //按已定义颜色获取
        UiColor color = GetColor(strColor);
        if (!color.IsEmpty()) {
            return color;
        }

        //按标准颜色值获取
        color = StandardColorMap::Instance().GetColor(strColor);
        if (!color.IsEmpty()) {
            return color;
        }
        return color;
    }
}

void ColorManager::AddColor(const DString& strName, const DString& strValue)
{
    m_colorMap.AddColor(strName, strValue);
}

void ColorManager::AddColor(const DString& strName, UiColor argb)
{
    m_colorMap.AddColor(strName, argb);
}

UiColor ColorManager::GetColor(const DString& strName2) const
{
    //别名优先，由于需要保留历史兼容性问题(比如原代码中使用了"red"这种颜色值，需要被覆盖掉)
    DString strName = GlobalManager::Instance().GetAliasValue(strName2);
    if (strName.empty()) {
        strName = strName2;
    }
    UiColor color = m_colorMap.GetColor(strName);
    if (color.IsEmpty() && (strName != strName2)){
        color = m_colorMap.GetColor(strName2);
    }
    return color;
}

void ColorManager::RemoveAllColors()
{
    m_colorMap.RemoveAllColors();
    m_defaultDisabledTextColor.clear();
    m_defaultTextColor.clear();
}

void ColorManager::Clear()
{
    RemoveAllColors();
}

const DString& ColorManager::GetDefaultDisabledTextColor()
{
    return m_defaultDisabledTextColor;
}

void ColorManager::SetDefaultDisabledTextColor(const DString& strColor)
{
    m_defaultDisabledTextColor = strColor;
}

const DString& ColorManager::GetDefaultTextColor()
{
    return m_defaultTextColor;
}

void ColorManager::SetDefaultTextColor(const DString& strColor)
{
    m_defaultTextColor = strColor;
}

void ColorManager::SetColorThemeDarkMode(bool bColorThemeDarkMode)
{
    m_bColorThemeDarkMode = bColorThemeDarkMode;
}

bool ColorManager::IsColorThemeDarkMode() const
{
    return m_bColorThemeDarkMode;
}

} // namespace ui

