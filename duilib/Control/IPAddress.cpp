#include "IPAddress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Utils/Clipboard.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

#include <algorithm>
#include <list>

namespace ui
{

namespace
{
    /** 段的数量（IPv4地址分为四段）
    */
    constexpr int32_t kSegmentCount = 4;

    /** 每段的最大值
    */
    constexpr int32_t kSegmentMaxValue = 255;

    /** 每段编辑框的宽度
    */
    constexpr int32_t kSegmentWidth = 28;

    /** 分隔符（'.'）的宽度
    */
    constexpr int32_t kSeparatorWidth = 6;

    /** 当前聚焦段的背景色
    */
    const DString kHighlightColor(_T("bg_list_item_hovered"));

    /** 将数值限制在合法范围内
    */
    int32_t ClampSegmentValue(int32_t nValue)
    {
        if (nValue < 0) {
            return 0;
        }
        if (nValue > kSegmentMaxValue) {
            return kSegmentMaxValue;
        }
        return nValue;
    }

    /** 判断字符串是否全部由数字字符组成（空字符串返回 false）
    */
    bool IsAllDigits(const DString& text)
    {
        if (text.empty()) {
            return false;
        }
        const DString::value_type kCharZero = static_cast<DString::value_type>('0');
        const DString::value_type kCharNine = static_cast<DString::value_type>('9');
        for (DString::value_type ch : text) {
            if ((ch < kCharZero) || (ch > kCharNine)) {
                return false;
            }
        }
        return true;
    }

    /** 解析IP地址字符串，要求必须是四段纯数字，数值会限制到 0~255
     * @param [out] values 解析出的四段数值
     * @return 返回 true 表示解析成功
     */
    bool TryParseIPAddress(const DString& ipAddress, int32_t values[kSegmentCount])
    {
        std::list<DString> ipList = StringUtil::Split(ipAddress, _T("."));
        if (ipList.size() != static_cast<size_t>(kSegmentCount)) {
            return false;
        }
        int32_t index = 0;
        for (const DString& part : ipList) {
            if (!IsAllDigits(part)) {
                return false;
            }
            values[index++] = ClampSegmentValue(StringUtil::StringToInt32(part));
        }
        return true;
    }

    /** 判断控件是否为本控件中的某个编辑框
    */
    bool IsSegmentControl(const std::vector<RichEdit*>& editList, Control* pControl)
    {
        return std::find(editList.begin(), editList.end(), pControl) != editList.end();
    }
}

IPAddress::IPAddress(Window* pWindow):
    HBox(pWindow),
    m_pLastFocus(nullptr),
    m_nFocusIndex(-1),
    m_bAllowEmpty(false),
    m_nSegmentWidth(kSegmentWidth)
{
}

DString IPAddress::GetType() const { return DUI_CTR_IPADDRESS; }

void IPAddress::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("ip")) {
        SetIPAddress(strValue);
    }
    else if (strName == _T("allow_empty")) {
        //是否允许空值（四段都为空时，GetIPAddress 返回空字符串）
        m_bAllowEmpty = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("segment_width")) {
        //每段编辑框的宽度（逻辑像素）
        int32_t nWidth = StringUtil::StringToInt32(strValue);
        if (nWidth < 1) {
            nWidth = 1;
        }
        m_nSegmentWidth = nWidth;
        for (RichEdit* pRichEdit : m_editList) {
            if (pRichEdit != nullptr) {
                pRichEdit->SetFixedWidth(UiFixedInt(m_nSegmentWidth), true, true);
            }
        }
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
    m_editList.reserve(static_cast<size_t>(kSegmentCount));
    for (int32_t index = 0; index < kSegmentCount; ++index) {
        RichEdit* pRichEdit = new RichEdit(GetWindow());
        pRichEdit->SetAttribute(_T("text_align"), _T("vcenter,hcenter"));
        pRichEdit->SetAttribute(_T("limit_text"), _T("3"));
        pRichEdit->SetAttribute(_T("want_return_msg"), _T("false"));
        pRichEdit->SetAttribute(_T("want_tab"), _T("false"));
        pRichEdit->SetAttribute(_T("text_padding"), _T("0,0,0,0"));
        pRichEdit->SetAttribute(_T("border_round"), _T("3,3"));
        //空格时显示占位符"0"，提示该段需要输入数字
        pRichEdit->SetPromptMode(true);
        pRichEdit->SetPromptText(_T("0"));
        pRichEdit->SetAttribute(_T("prompt_color"), _T("text_richedit_prompt"));
        pRichEdit->SetNumberOnly(true);
        pRichEdit->SetMinNumber(0);
        pRichEdit->SetMaxNumber(kSegmentMaxValue);
        pRichEdit->SetFixedWidth(UiFixedInt(m_nSegmentWidth), true, true);
        AddItem(pRichEdit);
        m_editList.push_back(pRichEdit);
        if (index != (kSegmentCount - 1)) {
            Label* pLabel = new Label(GetWindow());
            pLabel->SetText(_T("."));
            pLabel->SetAttribute(_T("text_align"), _T("vcenter,hcenter"));
            pLabel->SetFixedWidth(UiFixedInt(kSeparatorWidth), true, true);
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
                    FocusSegment(0, true);
                }
                else {
                    FocusSegment(index + 1, true);
                }
            }
            return true;
            });
        pRichEdit->AttachKillFocus([this, pRichEdit](const EventArgs& args) {
            OnKillFocusEvent(pRichEdit, (Control*)args.wParam);
            return true;
            });
        pRichEdit->AttachSetFocus([this, index](const EventArgs& /*args*/) {
            OnSegmentSetFocus(index);
            return true;
            });
        pRichEdit->AttachTextChanged([this, index](const EventArgs& /*args*/) {
            OnSegmentTextChanged(index);
            return true;
            });
        pRichEdit->AttachEvent(kEventKeyDown, [this, index](const EventArgs& args) {
            return OnSegmentKeyDown(index, args);
            }, 0);
    }
    if (!m_ipAddress.empty()) {
        //初始化前设置的IP地址：静默应用，不触发文本变化事件
        int32_t values[kSegmentCount] = { 0 };
        if (TryParseIPAddress(DString(m_ipAddress.c_str()), values)) {
            ApplyIPAddress(values);
        }
    }
    //记录初始值，避免初始化后立即触发一次文本变化事件
    m_lastIPAddress = GetSegmentText();
}

void IPAddress::SetFocus()
{
    if (IsNoFocus()) {
        return;
    }
    if (IsVisible() && IsEnabled() && !m_editList.empty()) {
        size_t nIndex = 0;
        auto iter = std::find(m_editList.begin(), m_editList.end(), m_pLastFocus);
        if ((m_pLastFocus != nullptr) && (iter != m_editList.end())) {
            nIndex = static_cast<size_t>(iter - m_editList.begin());
        }
        FocusSegment(nIndex, true);
    }
    else {
        BaseClass::SetFocus();
    }
}

void IPAddress::SetIPAddress(const DString& ipAddress)
{
    if (ipAddress.empty()) {
        //设置为空字符串：等价于清空IP地址
        ClearIPAddress();
        return;
    }
    int32_t values[kSegmentCount] = { 0 };
    if (!TryParseIPAddress(ipAddress, values)) {
        //格式非法（非四段纯数字）：忽略本次设置，不修改当前值
        return;
    }
    m_ipAddress = ipAddress;
    if (m_editList.size() != static_cast<size_t>(kSegmentCount)) {
        //尚未初始化完成，等 OnInit 时再应用
        return;
    }
    ApplyIPAddress(values);
    //程序化赋值属于值发生变化，对外发送文本变化事件
    NotifyValueChanged();
}

void IPAddress::ApplyIPAddress(const int32_t values[4])
{
    for (int32_t index = 0; index < kSegmentCount; ++index) {
        RichEdit* pRichEdit = m_editList[static_cast<size_t>(index)];
        if (pRichEdit != nullptr) {
            //程序化赋值不触发文本变化事件
            pRichEdit->SetTextNoEvent(StringUtil::Printf(_T("%d"), values[index]));
        }
    }
    UpdateSegmentBkColor();
}

DString IPAddress::GetSegmentText() const
{
    if (m_editList.size() != static_cast<size_t>(kSegmentCount)) {
        //尚未初始化完成时，返回初始化前设置的IP地址
        return m_ipAddress.empty() ? DString() : DString(m_ipAddress.c_str());
    }
    DString segmentText;
    for (int32_t index = 0; index < kSegmentCount; ++index) {
        if (index != 0) {
            segmentText += _T('.');
        }
        RichEdit* pRichEdit = m_editList[static_cast<size_t>(index)];
        if (pRichEdit != nullptr) {
            segmentText += pRichEdit->GetText();
        }
    }
    return segmentText;
}

DString IPAddress::GetIPAddress() const
{
    if (m_editList.size() != static_cast<size_t>(kSegmentCount)) {
        return m_ipAddress.empty() ? DString() : DString(m_ipAddress.c_str());
    }
    bool bAllEmpty = true;
    int32_t values[kSegmentCount] = { 0 };
    for (int32_t index = 0; index < kSegmentCount; ++index) {
        values[index] = GetSegmentNumber(static_cast<size_t>(index));
        RichEdit* pRichEdit = m_editList[static_cast<size_t>(index)];
        if ((pRichEdit != nullptr) && !pRichEdit->GetText().empty()) {
            bAllEmpty = false;
        }
    }
    if (m_bAllowEmpty && bAllEmpty) {
        return DString();
    }
    return StringUtil::Printf(_T("%d.%d.%d.%d"), values[0], values[1], values[2], values[3]);
}

bool IPAddress::IsComplete() const
{
    if (m_editList.size() != static_cast<size_t>(kSegmentCount)) {
        //尚未初始化完成时，基于初始化前设置的IP地址判断
        int32_t values[kSegmentCount] = { 0 };
        return !m_ipAddress.empty() && TryParseIPAddress(DString(m_ipAddress.c_str()), values);
    }
    for (RichEdit* pRichEdit : m_editList) {
        if ((pRichEdit == nullptr) || pRichEdit->GetText().empty()) {
            return false;
        }
    }
    return true;
}

bool IPAddress::IsValid() const
{
    if (m_editList.size() != static_cast<size_t>(kSegmentCount)) {
        //尚未初始化完成时，基于初始化前设置的IP地址判断
        int32_t values[kSegmentCount] = { 0 };
        return !m_ipAddress.empty() && TryParseIPAddress(DString(m_ipAddress.c_str()), values);
    }
    for (RichEdit* pRichEdit : m_editList) {
        if (pRichEdit == nullptr) {
            return false;
        }
        DString text = pRichEdit->GetText();
        if (!IsAllDigits(text)) {
            return false;
        }
        const int32_t nValue = StringUtil::StringToInt32(text);
        if ((nValue < 0) || (nValue > kSegmentMaxValue)) {
            return false;
        }
    }
    return true;
}

void IPAddress::ClearIPAddress()
{
    m_ipAddress.clear();
    for (RichEdit* pRichEdit : m_editList) {
        if (pRichEdit != nullptr) {
            pRichEdit->SetTextNoEvent(_T(""));
        }
    }
    NotifyValueChanged();
    UpdateSegmentBkColor();
}

bool IPAddress::IsFocused() const
{
    if (BaseClass::IsFocused()) {
        return true;
    }
    for (RichEdit* pRichEdit : m_editList) {
        if ((pRichEdit != nullptr) && pRichEdit->IsFocused()) {
            return true;
        }
    }
    return false;
}

void IPAddress::SendEventMsg(const EventArgs& msg)
{
    if ((msg.GetSender() == this) && (msg.eventType == kEventKillFocus)) {
        Control* pNewFocus = (Control*)msg.wParam;
        if (IsSegmentControl(m_editList, pNewFocus)) {
            //焦点从本控件自身移动到本控件的编辑框时，基类也会发出KillFocus事件，
            //此时焦点仍在控件内部，不应对外发出KillFocus事件
            return;
        }
    }
    BaseClass::SendEventMsg(msg);
}

void IPAddress::OnKillFocusEvent(RichEdit* pRichEdit, Control* pNewFocus)
{
    if ((pNewFocus == this) || IsSegmentControl(m_editList, pNewFocus)) {
        //焦点在本控件内部移动，不算失去焦点
        return;
    }
    m_pLastFocus = pRichEdit;
    m_nFocusIndex = -1;
    UpdateSegmentBkColor();
    //焦点转移到其他的控件，发出KillFocus事件
    SendEvent(kEventKillFocus);
}

void IPAddress::OnSegmentTextChanged(size_t index)
{
    if (index >= m_editList.size()) {
        return;
    }
    RichEdit* pRichEdit = m_editList[index];
    if (pRichEdit == nullptr) {
        return;
    }
    //输入满3位时，自动跳到下一段（最后一段不再跳转）
    if (((index + 1) < m_editList.size()) && (pRichEdit->GetText().size() >= 3)) {
        FocusSegment(index + 1, true);
    }
    UpdateSegmentBkColor();
    NotifyValueChanged();
}

bool IPAddress::OnSegmentKeyDown(size_t index, const EventArgs& msg)
{
    //返回值含义：返回 true 表示该按键没有被本控件消费，继续交给编辑框默认处理；
    //返回 false 表示已消费该按键，编辑框不再处理
    if (index >= m_editList.size()) {
        return true;
    }
    RichEdit* pRichEdit = m_editList[index];
    if (pRichEdit == nullptr) {
        return true;
    }
    const bool bCtrl = (msg.modifierKey & ModifierKey::kControl) != 0;
    switch (msg.vkCode) {
    case kVK_OEM_PERIOD:
    case kVK_DECIMAL:
        {
            //输入'.'时，切换到下一段（不插入字符）
            if ((index + 1) < m_editList.size()) {
                FocusSegment(index + 1, true);
            }
            return false;
        }
    case kVK_BACK:
        {
            //仅当段内没有内容，或者光标在段首时，才切换到上一段的末尾；
            //其余情况由编辑框处理，删除光标前的一个字符
            int32_t nStartChar = 0;
            int32_t nEndChar = 0;
            pRichEdit->GetSel(nStartChar, nEndChar);
            if ((index > 0) && (pRichEdit->GetText().empty() || ((nStartChar == 0) && (nEndChar == 0)))) {
                FocusSegment(index - 1, false);
                return false;
            }
            return true;
        }
    case kVK_LEFT:
        {
            //仅当光标在段首时，才切换到上一段的末尾；
            //其余情况由编辑框处理，光标左移一个字符
            int32_t nStartChar = 0;
            int32_t nEndChar = 0;
            pRichEdit->GetSel(nStartChar, nEndChar);
            if ((index > 0) && (nStartChar == 0) && (nEndChar == 0)) {
                FocusSegment(index - 1, false);
                return false;
            }
            return true;
        }
    case kVK_RIGHT:
        {
            //仅当光标在段尾时，才切换到下一段的开头；
            //其余情况由编辑框处理，光标右移一个字符
            int32_t nStartChar = 0;
            int32_t nEndChar = 0;
            pRichEdit->GetSel(nStartChar, nEndChar);
            const int32_t nTextLength = static_cast<int32_t>(pRichEdit->GetText().size());
            if (((index + 1) < m_editList.size()) && (nStartChar == nTextLength) && (nEndChar == nTextLength)) {
                //光标置于下一段的开头
                FocusSegment(index + 1, false, true);
                return false;
            }
            return true;
        }
    case kVK_UP:
        {
            //带修饰键时（如 Ctrl+↑）不处理，避免与框架的快捷键冲突
            if ((msg.modifierKey & (ModifierKey::kShift | ModifierKey::kControl | ModifierKey::kAlt)) != 0) {
                return true;
            }
            SetSegmentNumber(index, GetSegmentNumber(index) + 1);
            return false;
        }
    case kVK_DOWN:
        {
            //带修饰键时（如 Ctrl+↓）不处理，避免与框架的快捷键冲突
            if ((msg.modifierKey & (ModifierKey::kShift | ModifierKey::kControl | ModifierKey::kAlt)) != 0) {
                return true;
            }
            SetSegmentNumber(index, GetSegmentNumber(index) - 1);
            return false;
        }
    case kVK_C:
        {
            //Ctrl+C：复制完整的IP地址（四段未填写完整时，交给编辑框复制当前选中的文本）
            if (!bCtrl || !IsComplete()) {
                return true;
            }
            const DString ipAddress = GetIPAddress();
#ifdef DUILIB_UNICODE
            DStringW clipboardText(ipAddress.c_str());
#else
            DStringW clipboardText = StringConvert::UTF8ToWString(ipAddress);
#endif
            Clipboard::SetClipboardText(clipboardText);
            return false;
        }
    case kVK_V:
        {
            //Ctrl+V：剪贴板内容为完整的IP地址时，整体粘贴
            if (!bCtrl) {
                return true;
            }
            DStringW clipboardText;
            if (!Clipboard::GetClipboardText(clipboardText) || clipboardText.empty()) {
                return true;
            }
#ifdef DUILIB_UNICODE
            DString text(clipboardText.c_str());
#else
            DString text = StringConvert::WStringToUTF8(clipboardText);
#endif
            int32_t values[kSegmentCount] = { 0 };
            if (!TryParseIPAddress(text, values)) {
                return true;
            }
            //整体粘贴（值变化时由SetIPAddress对外发送文本变化事件）
            SetIPAddress(text);
            return false;
        }
    default:
        break;
    }
    //其余按键（数字、Delete、Home/End 等）交给编辑框默认处理
    return true;
}

void IPAddress::OnSegmentSetFocus(size_t index)
{
    if (index >= m_editList.size()) {
        return;
    }
    const bool bWasFocused = (m_nFocusIndex >= 0);
    m_nFocusIndex = static_cast<int32_t>(index);
    m_pLastFocus = m_editList[index];
    UpdateSegmentBkColor();
    if (!bWasFocused) {
        //焦点从控件外部进入时，对外发送SetFocus事件
        SendEvent(kEventSetFocus);
    }
}

void IPAddress::FocusSegment(size_t index, bool bSelectAll, bool bCaretAtStart)
{
    if (index >= m_editList.size()) {
        return;
    }
    RichEdit* pRichEdit = m_editList[index];
    if (pRichEdit == nullptr) {
        return;
    }
    pRichEdit->SetFocus();
    if (bSelectAll) {
        pRichEdit->SetSelAll();
    }
    else if (bCaretAtStart) {
        pRichEdit->SetSel(0, 0);
    }
    else {
        const int32_t nTextLength = static_cast<int32_t>(pRichEdit->GetText().size());
        pRichEdit->SetSel(nTextLength, nTextLength);
    }
}

void IPAddress::UpdateSegmentBkColor()
{
    for (size_t index = 0; index < m_editList.size(); ++index) {
        RichEdit* pRichEdit = m_editList[index];
        if (pRichEdit == nullptr) {
            continue;
        }
        DString bkColor;
        if ((static_cast<int32_t>(index) == m_nFocusIndex) && pRichEdit->IsFocused()) {
            bkColor = kHighlightColor;
        }
        pRichEdit->SetBkColor(bkColor);
    }
}

void IPAddress::NotifyValueChanged()
{
    //以四段原始文本比较，确保"某段由空变为0"等值变化也能触发事件
    const DString segmentText = GetSegmentText();
    if (!(m_lastIPAddress == segmentText)) {
        m_lastIPAddress = segmentText;
        SendEvent(kEventTextChanged);
    }
}

int32_t IPAddress::GetSegmentNumber(size_t index) const
{
    if (index >= m_editList.size()) {
        return 0;
    }
    RichEdit* pRichEdit = m_editList[index];
    if (pRichEdit == nullptr) {
        return 0;
    }
    const DString text = pRichEdit->GetText();
    if (!IsAllDigits(text)) {
        return 0;
    }
    return ClampSegmentValue(StringUtil::StringToInt32(text));
}

void IPAddress::SetSegmentNumber(size_t index, int32_t nValue)
{
    if (index >= m_editList.size()) {
        return;
    }
    RichEdit* pRichEdit = m_editList[index];
    if (pRichEdit == nullptr) {
        return;
    }
    const DString text = StringUtil::Printf(_T("%d"), ClampSegmentValue(nValue));
    if (pRichEdit->GetText() == text) {
        return;
    }
    pRichEdit->SetTextNoEvent(text);
    pRichEdit->SetSelAll();
    UpdateSegmentBkColor();
    NotifyValueChanged();
}

}//namespace ui
