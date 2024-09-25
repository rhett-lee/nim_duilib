#include "IPAddress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"

namespace ui
{

IPAddress::IPAddress(Window* pWindow):
    HBox(pWindow),
    m_pLastFocus(nullptr)
{
}

DString IPAddress::GetType() const { return DUI_CTR_IPADDRESS; }

void IPAddress::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("ip")) {
        SetIPAddress(strValue);
    }    
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void IPAddress::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    m_editList.clear();
    for (size_t index = 0; index < 4; ++index) {
        RichEdit* pRichEdit = new RichEdit(GetWindow());
        pRichEdit->SetText(_T(""));
        pRichEdit->SetAttribute(_T("text_align"), _T("vcenter,hcenter"));
        pRichEdit->SetAttribute(_T("limit_text"), _T("3"));
        pRichEdit->SetAttribute(_T("want_return_msg"), _T("false"));
        pRichEdit->SetAttribute(_T("want_tab"), _T("false"));
        pRichEdit->SetAttribute(_T("number_only"), _T("true"));
        pRichEdit->SetMinNumber(0);
        pRichEdit->SetMaxNumber(255);
        AddItem(pRichEdit);
        m_editList.push_back(pRichEdit);
        if (index != 3) {
            Label* pLabel = new Label(GetWindow());
            pLabel->SetText(_T("."));
            pLabel->SetAttribute(_T("text_align"), _T("bottom,hcenter"));
            pLabel->SetAttribute(_T("width"), _T("4"));
            pLabel->SetTabStop(false);
            pLabel->SetNoFocus();
            pLabel->SetMouseEnabled(false);
            AddItem(pLabel);
        }
    }

    for (size_t index = 0; index < m_editList.size(); ++index) {
        RichEdit* pRichEdit = m_editList[index];
        if (pRichEdit == nullptr) {
            continue;
        }
        pRichEdit->AttachReturn([this, index](const EventArgs& /*args*/) {
            //回车后，切换到下一个编辑框，并且循环
            if (index < m_editList.size()) {
                if (index == (m_editList.size() - 1)) {
                    m_editList[0]->SetFocus();
                }
                else {
                    m_editList[index + 1]->SetFocus();
                }
            }
            return true;
            });
        pRichEdit->AttachKillFocus([this, pRichEdit](const EventArgs& args) {
            OnKillFocusEvent(pRichEdit, (Control*)args.wParam);
            return true;
            });
    }
    if (!m_ipAddress.empty()) {
        SetIPAddress(m_ipAddress.c_str());
    }
}

void IPAddress::SetFocus()
{
    if (IsVisible() && IsEnabled() && !m_editList.empty()) {
        RichEdit* pRichEdit = m_editList.front();
        if (m_pLastFocus != nullptr) {
            if (std::find(m_editList.begin(), m_editList.end(), m_pLastFocus) != m_editList.end()) {
                pRichEdit = m_pLastFocus;
            }
        }
        if (pRichEdit != nullptr) {
            pRichEdit->SetFocus();
        }
    }
    else {
        BaseClass::SetFocus();
    }
}

void IPAddress::SetIPAddress(const DString& ipAddress)
{
    m_ipAddress = ipAddress;
    if (!ipAddress.empty() && m_editList.size() == 4) {
        std::list<DString> ipList = StringUtil::Split(ipAddress, _T("."));
        if (ipList.size() == 4) {
            auto iter = ipList.begin();
            int32_t ip1 = std::min(StringUtil::StringToInt32((*iter++)), 255);
            int32_t ip2 = std::min(StringUtil::StringToInt32((*iter++)), 255);
            int32_t ip3 = std::min(StringUtil::StringToInt32((*iter++)), 255);
            int32_t ip4 = std::min(StringUtil::StringToInt32((*iter++)), 255);
            ip1 = std::max(ip1, 0);
            ip2 = std::max(ip2, 0);
            ip3 = std::max(ip3, 0);
            ip4 = std::max(ip4, 0);
            m_editList[0]->SetText(StringUtil::Printf(_T("%d"), ip1));
            m_editList[1]->SetText(StringUtil::Printf(_T("%d"), ip2));
            m_editList[2]->SetText(StringUtil::Printf(_T("%d"), ip3));
            m_editList[3]->SetText(StringUtil::Printf(_T("%d"), ip4));
        }
    }
}

DString IPAddress::GetIPAddress() const
{
    DString ipAddress;
    if (m_editList.size() == 4) {
        int32_t ip1 = std::min(StringUtil::StringToInt32(m_editList[0]->GetText()), 255);
        int32_t ip2 = std::min(StringUtil::StringToInt32(m_editList[1]->GetText()), 255);
        int32_t ip3 = std::min(StringUtil::StringToInt32(m_editList[2]->GetText()), 255);
        int32_t ip4 = std::min(StringUtil::StringToInt32(m_editList[3]->GetText()), 255);
        ip1 = std::max(ip1, 0);
        ip2 = std::max(ip2, 0);
        ip3 = std::max(ip3, 0);
        ip4 = std::max(ip4, 0);
        ipAddress = StringUtil::Printf(_T("%d.%d.%d.%d"), ip1, ip2, ip3, ip4);
    }
    else {
        ipAddress = m_ipAddress.c_str();
    }
    return ipAddress;
}

void IPAddress::SendEventMsg(const EventArgs& msg)
{
    if ((msg.GetSender() == this) && (msg.eventType == kEventKillFocus)) {
        Control* pNewFocus = (Control*)msg.wParam;
        if (std::find(m_editList.begin(), m_editList.end(), pNewFocus) != m_editList.end()) {
            //焦点切换到编辑框，不发出KillFocus事件
            return;
        }
    }
    BaseClass::SendEventMsg(msg);
}

void IPAddress::OnKillFocusEvent(RichEdit* pRichEdit, Control* pNewFocus)
{
    m_pLastFocus = pRichEdit;
    if (pNewFocus == this) {
        return;
    }
    if (std::find(m_editList.begin(), m_editList.end(), pNewFocus) != m_editList.end()) {
        return;
    }
    //焦点转移到其他的控件，发出KillFocus事件
    SendEvent(kEventKillFocus);
}

}//namespace ui

