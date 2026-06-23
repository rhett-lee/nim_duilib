#include "ColorMap.h"
#include "duilib/Utils/StringUtil.h"

#ifdef _DEBUG
#include "duilib/Core/GlobalManager.h"
#endif

namespace ui 
{
void ColorMap::AddColor(const DString& strName, const DString& strValue)
{
    ASSERT(!strName.empty() && !strValue.empty());
    if (strName.empty() || strValue.empty()) {
        return;
    }
    UiColor color;
    if (strValue.at(0) == _T('#')) {
        color = StandardColorMap::HexToColor(strValue);
    }
    else {
        const StandardColorMap& standardColorMap = StandardColorMap::Instance();
        color = standardColorMap.GetColor(strValue);
    }
    ASSERT(!color.IsEmpty());
    if (!color.IsEmpty()) {
        AddColor(strName, color);
    }
}

void ColorMap::AddColor(const DString& strName, UiColor argb)
{
    ASSERT(!strName.empty() && (argb.GetARGB() != 0));
    if (strName.empty() || argb.IsEmpty()) {
        return;
    }
#ifdef _DEBUG
    if (!GlobalManager::Instance().Theme().IsSwitchingTheme()) {
        //在切换主题的时候，允许覆盖，其他情况下覆盖时断言
        auto iter = m_colorMap.find(strName);
        if (iter != m_colorMap.end()) {
            ASSERT(iter->second == argb);
        }
    }
#endif
    m_colorMap[strName] = argb;
}

UiColor ColorMap::GetColor(const DString& strName) const
{
    if (!strName.empty()) {
        auto it = m_colorMap.find(strName);
        if (it != m_colorMap.end()) {
            return it->second;
        }
    }    
    return UiColor();
}

void ColorMap::RemoveColor(const DString& strName)
{
    auto it = m_colorMap.find(strName);
    if (it != m_colorMap.end()) {
        m_colorMap.erase(it);
    }
}

void ColorMap::RemoveAllColors()
{
    m_colorMap.clear();
}

StandardColorMap::StandardColorMap()
{
    //初始化标准颜色表, 字符串不区分大小写
    std::vector<std::pair<DString, int32_t>> uiColors;
    UiColors::GetUiColorsString(uiColors);
    for (auto iter : uiColors) {
        AddColor(StringUtil::MakeLowerString(iter.first), UiColor(iter.second));
    }
}

const StandardColorMap& StandardColorMap::Instance()
{
    static StandardColorMap self;
    return self;
}

UiColor StandardColorMap::HexToColor(const DString& hexColor)
{
    ASSERT((hexColor.size() == 9) || (hexColor.size() == 7));
    if ((hexColor.size() != 9) && (hexColor.size() != 7)) {
        return UiColor();
    }
    ASSERT(hexColor.at(0) == _T('#'));
    if (hexColor.at(0) != _T('#')) {
        return UiColor();
    }
    for (size_t i = 1; i < hexColor.size(); ++i) {
        DString::value_type ch = hexColor.at(i);
        bool isValid = (((ch >= _T('0')) && (ch <= _T('9'))) ||
                       ((ch >= _T('a')) && (ch <= _T('f'))) ||
                       ((ch >= _T('A')) && (ch <= _T('F'))));
        ASSERT(isValid);
        if (!isValid) {
            return UiColor();
        }
    }
    DString colorValue = hexColor.substr(1);
    if (colorValue.size() == 6) {
        //如果是#FFFFFF格式，自动补上Alpha值
        colorValue = _T("FF") + colorValue;
    }
    UiColor::ARGB argb = StringUtil::StringToUInt32(colorValue.c_str(), nullptr, 16);
    return UiColor(argb);
}

DString StandardColorMap::ColorToHex(const UiColor& color)
{
    if (color.IsEmpty()) {
        return DString();
    }
    else {
        return StringUtil::Printf(_T("#%02X%02X%02X%02X"), color.GetA(), color.GetR(), color.GetG(), color.GetB());
    }
}

UiColor StandardColorMap::GetColor(const DString& strName) const
{
    DString lowerName = StringUtil::MakeLowerString(strName);
    return ColorMap::GetColor(lowerName);
}

} // namespace ui

