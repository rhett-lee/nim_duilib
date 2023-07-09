#include "BoxShadow.h"
#include "duilib/Core/GlobalManager.h"
#include <tchar.h>

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
	std::wstring sItem;
	std::wstring sValue;
	LPTSTR pstr = NULL;

	LPCTSTR pStrKey = strBoxShadow.c_str();
	while (*pStrKey != _T('\0')) {
		sItem.clear();
		sValue.clear();
		while (*pStrKey > _T('\0') && *pStrKey <= _T(' ')) pStrKey = ::CharNext(pStrKey);
		while (*pStrKey != _T('\0') && *pStrKey != _T('=') && *pStrKey > _T(' ')) {
			LPTSTR pstrTemp = ::CharNext(pStrKey);
			while (pStrKey < pstrTemp) {
				sItem += *pStrKey++;
			}
		}
		while (*pStrKey > _T('\0') && *pStrKey <= _T(' ')) pStrKey = ::CharNext(pStrKey);
		if (*pStrKey++ != _T('=')) break;
		while (*pStrKey > _T('\0') && *pStrKey <= _T(' ')) pStrKey = ::CharNext(pStrKey);
		if (*pStrKey++ != _T('\'')) break;
		while (*pStrKey != _T('\0') && *pStrKey != _T('\'')) {
			LPTSTR pstrTemp = ::CharNext(pStrKey);
			while (pStrKey < pstrTemp) {
				sValue += *pStrKey++;
			}
		}
		if (*pStrKey++ != _T('\'')) break;
		if (!sValue.empty()) {
			if (sItem == _T("color")) {
				m_strColor = sValue;
			}
			else if (sItem == _T("offset")) {
				m_cpOffset.x = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				m_cpOffset.y = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				GlobalManager::Instance().Dpi().ScalePoint(m_cpOffset);
			}
			else if ((sItem == _T("blur_radius")) || sItem == _T("blurradius")) {
				m_nBlurRadius = _tcstol(sValue.c_str(), &pstr, 10); ASSERT(pstr);
				ASSERT(m_nBlurRadius >= 0);
				if (m_nBlurRadius < 0) {
					m_nBlurRadius = 2;
				}
				GlobalManager::Instance().Dpi().ScaleInt(m_nBlurRadius);
			}
			else if ((sItem == _T("spread_radius")) || (sItem == _T("spreadradius"))) {
				m_nSpreadRadius = _tcstol(sValue.c_str(), &pstr, 10); ASSERT(pstr);
				ASSERT(m_nSpreadRadius >= 0);
				if (m_nSpreadRadius < 0) {
					m_nSpreadRadius = 2;
				}
				GlobalManager::Instance().Dpi().ScaleInt(m_nSpreadRadius);
			}
			else {
				ASSERT(!"BoxShadow::SetBoxShadowString found unknown item!");
			}
		}
		if (*pStrKey++ != _T(' ')) {
			break;
		}
	}
}

bool BoxShadow::HasShadow() const
{
	return !m_strColor.empty();
}

} // namespace ui
