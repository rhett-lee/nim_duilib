#include "BoxShadow.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui {

BoxShadow::BoxShadow(Control* pControl):
    m_cpOffset(0, 0),
    m_nBlurRadius(2),
    m_nSpreadRadius(2),
    m_pControl(pControl)
{
    ASSERT(m_pControl != nullptr);
    if (m_pControl != nullptr) {
        m_pControl->Dpi().ScaleInt(m_nBlurRadius);
        m_pControl->Dpi().ScaleInt(m_nSpreadRadius);
    }
}

void BoxShadow::SetBoxShadowString(const DString& strBoxShadow)
{
    //格式如：如 "color='black' offset='1,1' blur_radius='2' spread_radius='2'"
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(strBoxShadow, _T('\''), attributeList);
    for (const auto& attribute : attributeList) {
        const DString& name = attribute.first;
        const DString& value = attribute.second;
        if (name.empty() || value.empty()) {
            continue;
        }
        if (name == _T("color")) {
            m_strColor = value;
        }
        else if (name == _T("offset")) {
            AttributeUtil::ParsePointValue(value.c_str(), m_cpOffset);
            ASSERT(m_pControl != nullptr);
            if (m_pControl != nullptr) {
                m_pControl->Dpi().ScalePoint(m_cpOffset);
            }
        }
        else if ((name == _T("blur_radius")) || name == _T("blurradius")) {
            m_nBlurRadius = StringUtil::StringToInt32(value);
            ASSERT(m_nBlurRadius >= 0);
            if (m_nBlurRadius < 0) {
                m_nBlurRadius = 2;
            }
            ASSERT(m_pControl != nullptr);
            if (m_pControl != nullptr) {
                m_pControl->Dpi().ScaleInt(m_nBlurRadius);
            }
        }
        else if ((name == _T("spread_radius")) || (name == _T("spreadradius"))) {
            m_nSpreadRadius = StringUtil::StringToInt32(value);
            ASSERT(m_nSpreadRadius >= 0);
            if (m_nSpreadRadius < 0) {
                m_nSpreadRadius = 2;
            }
            ASSERT(m_pControl != nullptr);
            if (m_pControl != nullptr) {
                m_pControl->Dpi().ScaleInt(m_nSpreadRadius);
            }
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
