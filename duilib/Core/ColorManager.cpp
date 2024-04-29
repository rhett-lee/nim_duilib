#include "ColorManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include <unordered_map>

namespace ui 
{
void ColorMap::AddColor(const std::wstring& strName, const std::wstring& strValue)
{
	ASSERT(!strName.empty() && !strValue.empty());
	if (strName.empty() || strValue.empty()) {
		return;
	}
	UiColor color = ColorManager::ConvertToUiColor(strValue);
	AddColor(strName, color);
}

void ColorMap::AddColor(const std::wstring& strName, UiColor argb)
{
	ASSERT(!strName.empty() && (argb.GetARGB() != 0));
	if (strName.empty() || (argb.GetARGB() == 0)) {
		return;
	}
	m_colorMap[strName] = argb;
}

UiColor ColorMap::GetColor(const std::wstring& strName) const
{
	auto it = m_colorMap.find(strName);
	if (it != m_colorMap.end()) {
		return it->second;
	}
	return UiColor();
}

void ColorMap::RemoveAllColors()
{
	m_colorMap.clear();
}

ColorManager::ColorManager()
{
	//初始化标准颜色表, 字符串不区分大小写
	std::vector<std::pair<std::wstring, int32_t>> uiColors;
	UiColors::GetUiColorsString(uiColors);
	for (auto iter : uiColors) {		
		m_standardColorMap.AddColor(StringHelper::MakeLowerString(iter.first), UiColor(iter.second));
	}
}

UiColor ColorManager::ConvertToUiColor(const std::wstring& strColor)
{
	ASSERT(!strColor.empty());
	UiColor color;
	if (strColor.empty()) {
		return color;
	}
	if (strColor.at(0) != L'#') {
		//按标准颜色值获取
		color = GlobalManager::Instance().Color().GetStandardColor(strColor);
		if (!color.IsEmpty()) {
			return color;
		}
	}

	//具体颜色值，格式如：#FFFFFFFF 或者 #FFFFFF
	ASSERT((strColor.size() == 9) || (strColor.size() == 7));
	if ((strColor.size() != 9) && (strColor.size() != 7)) {
		return color;
	}
	ASSERT(strColor.at(0) == L'#');
	if (strColor.at(0) != L'#') {
		return color;
	}
	for (size_t i = 1; i < strColor.size(); ++i) {
		wchar_t ch = strColor.at(i);
		bool isValid = (((ch >= L'0') && (ch <= L'9')) ||
			((ch >= L'a') && (ch <= L'f')) ||
			((ch >= L'A') && (ch <= L'F')));
		ASSERT(isValid);
		if (!isValid) {
			return color;
		}
	}
	std::wstring colorValue = strColor.substr(1);
	if (colorValue.size() == 6) {
		//如果是#FFFFFF格式，自动补上Alpha值
		colorValue = L"FF" + colorValue;
	}
	UiColor::ARGB argb = wcstoul(colorValue.c_str(), nullptr, 16);
	return UiColor(argb);
}

void ColorManager::AddColor(const std::wstring& strName, const std::wstring& strValue)
{
	m_colorMap.AddColor(strName, strValue);
}

void ColorManager::AddColor(const std::wstring& strName, UiColor argb)
{
	m_colorMap.AddColor(strName, argb);
}

UiColor ColorManager::GetColor(const std::wstring& strName) const
{
	return m_colorMap.GetColor(strName);
}

UiColor ColorManager::GetStandardColor(const std::wstring& strName) const
{
	//名称不区分大小写
	return m_standardColorMap.GetColor(StringHelper::MakeLowerString(strName));
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
	m_standardColorMap.RemoveAllColors();
}

const std::wstring& ColorManager::GetDefaultDisabledTextColor()
{
	return m_defaultDisabledTextColor;
}

void ColorManager::SetDefaultDisabledTextColor(const std::wstring& strColor)
{
	m_defaultDisabledTextColor = strColor;
}

const std::wstring& ColorManager::GetDefaultTextColor()
{
	return m_defaultTextColor;
}

void ColorManager::SetDefaultTextColor(const std::wstring& strColor)
{
	m_defaultTextColor = strColor;
}

} // namespace ui

