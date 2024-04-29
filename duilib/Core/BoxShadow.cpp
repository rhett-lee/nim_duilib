#include "BoxShadow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui {

BoxShadow::BoxShadow():
	m_cpOffset(0, 0),
	m_nBlurRadius(2),
	m_nSpreadRadius(2)
{
	GlobalManager::Instance().Dpi().ScaleInt(m_nBlurRadius);
	GlobalManager::Instance().Dpi().ScaleInt(m_nSpreadRadius);
}

void BoxShadow::SetBoxShadowString(const std::wstring& strBoxShadow)
{
	//格式如：如 "color='black' offset='1,1' blur_radius='2' spread_radius='2'"
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
	AttributeUtil::ParseAttributeList(strBoxShadow, L'\'', attributeList);
	for (const auto& attribute : attributeList) {
		const std::wstring& name = attribute.first;
		const std::wstring& value = attribute.second;
		if (name.empty() || value.empty()) {
			continue;
		}
		if (name == L"color") {
			m_strColor = value;
		}
		else if (name == L"offset") {
			AttributeUtil::ParsePointValue(value.c_str(), m_cpOffset);
			GlobalManager::Instance().Dpi().ScalePoint(m_cpOffset);			
		}
		else if ((name == L"blur_radius") || name == L"blurradius") {
			m_nBlurRadius = wcstol(value.c_str(), nullptr, 10);
			ASSERT(m_nBlurRadius >= 0);
			if (m_nBlurRadius < 0) {
				m_nBlurRadius = 2;
			}
			GlobalManager::Instance().Dpi().ScaleInt(m_nBlurRadius);
		}
		else if ((name == L"spread_radius") || (name == L"spreadradius")) {
			m_nSpreadRadius = wcstol(value.c_str(), nullptr, 10);
			ASSERT(m_nSpreadRadius >= 0);
			if (m_nSpreadRadius < 0) {
				m_nSpreadRadius = 2;
			}
			GlobalManager::Instance().Dpi().ScaleInt(m_nSpreadRadius);
		}
		else {
			ASSERT(!"BoxShadow::SetBoxShadowString found unknown item name!");
		}
	}
}

bool BoxShadow::HasShadow() const
{
	return !m_strColor.empty();
}

} // namespace ui
