#include "IPAddress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"

namespace ui
{

IPAddress::IPAddress():
    m_pLastFocus(nullptr)
{
}

std::wstring IPAddress::GetType() const { return DUI_CTR_IPADDRESS; }

void IPAddress::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"ip") {
        SetIPAddress(strValue);
    }    
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void IPAddress::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    m_editList.clear();
    for (size_t index = 0; index < 4; ++index) {
        RichEdit* pRichEdit = new RichEdit;
        pRichEdit->SetText(L"");
        pRichEdit->SetAttribute(L"text_align", L"vcenter,hcenter");
        pRichEdit->SetAttribute(L"limit_text", L"3");
        pRichEdit->SetAttribute(L"want_return_msg", L"true");
        pRichEdit->SetAttribute(L"want_tab", L"false");
        pRichEdit->SetAttribute(L"number_only", L"true");        
        AddItem(pRichEdit);
        m_editList.push_back(pRichEdit);
        if (index != 3) {
            Label* pLabel = new Label;
            pLabel->SetText(L".");
            pLabel->SetAttribute(L"text_align", L"bottom,hcenter");
            pLabel->SetAttribute(L"width", L"4");
            pLabel->SetTabStop(false);
            pLabel->SetNoFocus();
            pLabel->SetMouseEnabled(false);
            AddItem(pLabel);
        }
    }

    for (size_t index = 0; index < m_editList.size(); ++index) {
        RichEdit* pRichEdit = m_editList[index];
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
        pRichEdit->AttachTextChange([this, pRichEdit](const ui::EventArgs& /*args*/) {
            if (pRichEdit != nullptr) {
                std::wstring text = pRichEdit->GetText();
                int32_t nValue = _wtoi(text.c_str());
                if (nValue < 0) {
                    nValue = 0;
                    pRichEdit->SetTextNoEvent(L"0");
                }
                else if (nValue > 255) {
                    nValue = 255;
                    pRichEdit->SetTextNoEvent(L"255");
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
        __super::SetFocus();
    }
}

void IPAddress::SetIPAddress(const std::wstring& ipAddress)
{
    m_ipAddress = ipAddress;
    if (!ipAddress.empty() && m_editList.size() == 4) {
        std::list<std::wstring> ipList = StringHelper::Split(ipAddress, L".");
        if (ipList.size() == 4) {
            auto iter = ipList.begin();
            int32_t ip1 = std::min(_wtoi((*iter++).c_str()), 255);
            int32_t ip2 = std::min(_wtoi((*iter++).c_str()), 255);
            int32_t ip3 = std::min(_wtoi((*iter++).c_str()), 255);
            int32_t ip4 = std::min(_wtoi((*iter++).c_str()), 255);
            ip1 = std::max(ip1, 0);
            ip2 = std::max(ip2, 0);
            ip3 = std::max(ip3, 0);
            ip4 = std::max(ip4, 0);
            m_editList[0]->SetText(StringHelper::Printf(L"%d", ip1));
            m_editList[1]->SetText(StringHelper::Printf(L"%d", ip2));
            m_editList[2]->SetText(StringHelper::Printf(L"%d", ip3));
            m_editList[3]->SetText(StringHelper::Printf(L"%d", ip4));
        }
    }
}

std::wstring IPAddress::GetIPAddress() const
{
    std::wstring ipAddress;
    if (m_editList.size() == 4) {
        int32_t ip1 = std::min(_wtoi(m_editList[0]->GetText().c_str()), 255);
        int32_t ip2 = std::min(_wtoi(m_editList[1]->GetText().c_str()), 255);
        int32_t ip3 = std::min(_wtoi(m_editList[2]->GetText().c_str()), 255);
        int32_t ip4 = std::min(_wtoi(m_editList[3]->GetText().c_str()), 255);
        ip1 = std::max(ip1, 0);
        ip2 = std::max(ip2, 0);
        ip3 = std::max(ip3, 0);
        ip4 = std::max(ip4, 0);
        ipAddress = StringHelper::Printf(L"%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    }
    else {
        ipAddress = m_ipAddress.c_str();
    }
    return ipAddress;
}

void IPAddress::SendEvent(EventType eventType,
                          WPARAM wParam,
                          LPARAM lParam,
                          TCHAR tChar,
                          const UiPoint& mousePos)
{
    __super::SendEvent(eventType, wParam, lParam, tChar, mousePos);
}

void IPAddress::SendEvent(const EventArgs& msg)
{
    if ((msg.pSender == this) && (msg.Type == kEventKillFocus)) {
        Control* pNewFocus = (Control*)msg.wParam;
        if (std::find(m_editList.begin(), m_editList.end(), pNewFocus) != m_editList.end()) {
            //焦点切换到编辑框，不发出KillFocus事件
            return;
        }
    }
    __super::SendEvent(msg);
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

