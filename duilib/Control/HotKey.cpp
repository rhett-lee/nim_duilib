#include "HotKey.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Utils/StringUtil.h"
#include <map>
#include <list>

namespace ui
{

#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08

/** 将按键消息中的修饰键标志转换为内部的HOTKEYF_*标志
*/
static uint8_t GetHotKeyModifiers(const EventArgs& msg)
{
    uint8_t wModifiers = 0;
    if (msg.modifierKey & ModifierKey::kShift) {
        wModifiers |= HOTKEYF_SHIFT;
    }
    if (msg.modifierKey & ModifierKey::kControl) {
        wModifiers |= HOTKEYF_CONTROL;
    }
    if (msg.modifierKey & ModifierKey::kAlt) {
        wModifiers |= HOTKEYF_ALT;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //扩展键标志：仅Windows原生消息的lParam中包含该信息（SDL下无法获取，忽略）
    if ((msg.lParam >> 16) & kKF_EXTENDED) {
        wModifiers |= HOTKEYF_EXT;
    }
#endif
    return wModifiers;
}

class HotKeyRichEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    HotKeyRichEdit(Window* pWindow, HotKey* pOwner):
        RichEdit(pWindow),
        m_pOwner(pOwner)
    {
    }

    /** 输入字符
    */
    virtual bool OnChar(const EventArgs& /*msg*/) override
    {
        //禁止输入字符
        return true;
    }

    /** 按键事件
    */
    virtual bool OnKeyDown(const EventArgs& msg) override
    {
        HotKey* pOwner = m_pOwner;
        if (pOwner == nullptr) {
            return true;
        }
        //本次按键对应的修饰键标志
        const uint8_t wModifiers = GetHotKeyModifiers(msg);

        //Esc：取消本次输入，恢复为已提交热键的显示
        if (msg.vkCode == kVK_ESCAPE) {
            pOwner->RestoreDisplayText();
            return true;
        }
        //Delete/Backspace：清空热键
        if ((msg.vkCode == kVK_DELETE) || (msg.vkCode == kVK_BACK)) {
            pOwner->ClearHotKey();
            return true;
        }
        //Windows键不作为热键记录
        if ((msg.vkCode == kVK_LWIN) || (msg.vkCode == kVK_RWIN)) {
            return true;
        }
        //Alt与系统组合键不作为热键记录（避免被系统优先处理）
        if ((wModifiers & HOTKEYF_ALT) &&
            ((msg.vkCode == kVK_F4) || (msg.vkCode == kVK_TAB) ||
             (msg.vkCode == kVK_SPACE))) {
            return true;
        }
        //只按下修饰键时：仅预览显示，不修改热键值
        if ((msg.vkCode == kVK_SHIFT) || (msg.vkCode == kVK_CONTROL) || (msg.vkCode == kVK_MENU)) {
            pOwner->PreviewHotKey(wModifiers);
            return true;
        }
        //普通按键：提交热键（允许不带修饰键的单独按键）
        if (!Keyboard::GetKeyName(msg.vkCode, (wModifiers & HOTKEYF_EXT) != 0).empty()) {
            pOwner->ApplyHotKey(static_cast<uint8_t>(msg.vkCode), wModifiers);
        }
        return true;
    }

    /** 按键事件
    */
    virtual bool OnKeyUp(const EventArgs& msg) override
    {
        //松开修饰键时，结束预览并恢复为已提交热键的显示
        if ((msg.vkCode == kVK_SHIFT) || (msg.vkCode == kVK_CONTROL) ||
            (msg.vkCode == kVK_MENU) || (msg.vkCode == kVK_LWIN) || (msg.vkCode == kVK_RWIN)) {
            if (m_pOwner != nullptr) {
                m_pOwner->RestoreDisplayText();
            }
        }
        return true;
    }

private:
    /** 所属的热键控件
    */
    HotKey* m_pOwner;
};

HotKey::HotKey(Window* pWindow):
    HBox(pWindow),
    m_pRichEdit(nullptr),
    m_wVirtualKeyCode(0),
    m_wModifiers(0),
    m_nLastHotKeyValue(0),
    m_bPreviewing(false)
{
    ASSERT(pWindow != nullptr);
    m_pRichEdit = new HotKeyRichEdit(pWindow, this);
    m_pRichEdit->SetAttribute(_T("text_align"), _T("vcenter,hcenter"));
    m_pRichEdit->SetAttribute(_T("text_padding"), _T("0,0,0,0"));
    m_pRichEdit->SetAttribute(_T("want_tab"), _T("false"));
    //未设置热键时，显示灰色的占位提示文字
    m_pRichEdit->SetAttribute(_T("prompt_mode"), _T("true"));
    m_pRichEdit->SetAttribute(_T("prompt_color"), _T("text_richedit_prompt"));
    m_pRichEdit->SetAttribute(_T("width"), _T("100%"));
    m_pRichEdit->SetAttribute(_T("height"), _T("100%"));
}

HotKey::~HotKey()
{
    if (!IsInited()) {
        if (m_pRichEdit != nullptr) {
            delete m_pRichEdit;
            m_pRichEdit = nullptr;
        }
    }
}

DString HotKey::GetType() const { return DUI_CTR_HOTKEY; }

void HotKey::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("default_text")) {
        if (m_pRichEdit != nullptr) {
            //仅作为占位提示文字显示，不修改热键值
            m_pRichEdit->SetPromptText(strValue);
        }
    }
    else if (strName == _T("default_text_id")) {
        if (m_pRichEdit != nullptr) {
            //仅作为占位提示文字显示，不修改热键值
            m_pRichEdit->SetPromptTextId(strValue);
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void HotKey::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    HotKeyRichEdit* pRichEdit = m_pRichEdit;
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit == nullptr) {
        return;
    }
    AddItem(pRichEdit);

    //以RichEdit控件的焦点作为整个控件的焦点
    pRichEdit->AttachSetFocus([this](const EventArgs&) {
        //焦点变化后，刷新焦点边框的显示
        Invalidate();
        SendEvent(kEventSetFocus);
        return true;
        });
    pRichEdit->AttachKillFocus([this](const EventArgs&) {
        //失去焦点时，结束修饰键的预览状态
        RestoreDisplayText();
        Invalidate();
        SendEvent(kEventKillFocus);
        return true;
        });

    //同步显示的文本（初始化前可能已经设置过热键）
    UpdateDisplayText();
}

void HotKey::SetFocus()
{
    if (IsVisible() && IsEnabled() && (m_pRichEdit != nullptr)) {
        m_pRichEdit->SetFocus();
    }
}

bool HotKey::IsFocused() const
{
    if (BaseClass::IsFocused()) {
        return true;
    }
    return (m_pRichEdit != nullptr) && m_pRichEdit->IsFocused();
}

void HotKey::SetHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    uint8_t wNewModifiers = 0;
    if (wModifiers & kHotKey_Shift) {
        wNewModifiers |= (HOTKEYF_SHIFT);
    }
    if (wModifiers & kHotKey_Contrl) {
        wNewModifiers |= (HOTKEYF_CONTROL);
    }
    if (wModifiers & kHotKey_Alt) {
        wNewModifiers |= (HOTKEYF_ALT);
    }
    if (wModifiers & kHotKey_Ext) {
        wNewModifiers |= (HOTKEYF_EXT);
    }
    ApplyHotKey(wVirtualKeyCode, wNewModifiers);
}

void HotKey::GetHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    wVirtualKeyCode = m_wVirtualKeyCode;
    wModifiers = m_wModifiers;
}

void HotKey::SetHotKey(uint16_t dwHotKey)
{
    uint8_t wVirtualKeyCode = LOBYTE(dwHotKey);
    uint8_t wModifiers = HIBYTE(dwHotKey);
    SetHotKey(wVirtualKeyCode, wModifiers);
}

uint16_t HotKey::GetHotKey() const
{
    return MAKEWORD(m_wVirtualKeyCode, m_wModifiers);
}

void HotKey::ClearHotKey()
{
    ApplyHotKey(0, 0);
}

DString HotKey::GetHotKeyName() const
{
    return FormatHotKeyName(m_wVirtualKeyCode, m_wModifiers);
}

bool HotKey::SetHotKeyName(const DString& hotKeyName)
{
    DString hotKeyText = hotKeyName;
    StringUtil::Trim(hotKeyText);
    if (hotKeyText.empty()) {
        //空名称：等价于清空热键
        ClearHotKey();
        return true;
    }

    std::list<DString> hotKeyList = StringUtil::Split(hotKeyText, _T("+"));
    for (DString& hotKey : hotKeyList) {
        StringUtil::Trim(hotKey);
        hotKey = StringUtil::MakeLowerString(hotKey);
    }
    if (hotKeyList.empty()) {
        return false;
    }
    DString keyCtrl = GetKeyName(kVK_CONTROL, false);
    DString keyShift = GetKeyName(kVK_SHIFT, false);
    DString keyAlt = GetKeyName(kVK_MENU, false);
    keyCtrl = StringUtil::MakeLowerString(keyCtrl);
    keyShift = StringUtil::MakeLowerString(keyShift);
    keyAlt = StringUtil::MakeLowerString(keyAlt);

    uint8_t wModifiers = 0;
    auto iter = hotKeyList.begin();
    while (iter != hotKeyList.end()) {
        const DString& hotKey = *iter;
        if (hotKey == keyCtrl) {
            wModifiers |= kHotKey_Contrl;
            iter = hotKeyList.erase(iter);
            continue;
        }
        else if (hotKey == keyShift) {
            wModifiers |= kHotKey_Shift;
            iter = hotKeyList.erase(iter);
            continue;
        }
        else if (hotKey == keyAlt) {
            wModifiers |= kHotKey_Alt;
            iter = hotKeyList.erase(iter);
            continue;
        }
        else {
            ++iter;
        }
    }
    uint8_t wVirtualKeyCode = 0;
    if (!hotKeyList.empty()) {
        std::map<DString, uint8_t> vkCodeMap;
        DString temp;
        for (uint32_t vkCode = 0; vkCode <= 256; ++vkCode) {
            temp = StringUtil::MakeLowerString(GetKeyName((uint8_t)vkCode, false));
            if (!temp.empty()) {
                vkCodeMap[temp] = (uint8_t)vkCode;
            }
        }
        for (const DString& hotKey : hotKeyList) {
            auto pos = vkCodeMap.find(hotKey);
            if (pos != vkCodeMap.end()) {
                //只支持一个键，其他的忽略掉
                wVirtualKeyCode = pos->second;
                break;
            }
        }
    }
    if ((wVirtualKeyCode == 0) && !hotKeyList.empty()) {
        std::map<DString, uint8_t> vkCodeExtMap;
        DString temp;
        for (uint32_t vkCode = 0; vkCode <= 256; ++vkCode) {
            temp = StringUtil::MakeLowerString(GetKeyName((uint8_t)vkCode, true));
            if (!temp.empty()) {
                vkCodeExtMap[temp] = (uint8_t)vkCode;
            }
        }
        for (const DString& hotKey : hotKeyList) {
            auto pos = vkCodeExtMap.find(hotKey);
            if (pos != vkCodeExtMap.end()) {
                //只支持一个键，其他的忽略掉
                wVirtualKeyCode = pos->second;
                wModifiers |= kHotKey_Ext;
                break;
            }
        }
    }

    if (wVirtualKeyCode == 0) {
        //没有主键时（比如仅"Ctrl"），不是有效的热键
        return false;
    }
    SetHotKey(wVirtualKeyCode, wModifiers);
    return true;
}

DString HotKey::GetKeyName(uint8_t wVirtualKeyCode, bool fExtended)
{
    return Keyboard::GetKeyName((VirtualKeyCode)wVirtualKeyCode, fExtended);
}

void HotKey::ApplyHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    if (wVirtualKeyCode == 0) {
        //虚拟键码为空时，视为清空热键
        wModifiers = 0;
    }
    m_bPreviewing = false;
    m_wVirtualKeyCode = wVirtualKeyCode;
    m_wModifiers = wModifiers;
    UpdateDisplayText();
    NotifyHotKeyChanged();
}

void HotKey::UpdateDisplayText()
{
    m_bPreviewing = false;
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetTextNoEvent(GetHotKeyName());
    }
}

void HotKey::PreviewHotKey(uint8_t wModifiers)
{
    if ((m_pRichEdit == nullptr) || (wModifiers == 0)) {
        return;
    }
    m_bPreviewing = true;
    m_pRichEdit->SetTextNoEvent(FormatHotKeyName(0, wModifiers));
}

void HotKey::RestoreDisplayText()
{
    if (m_bPreviewing) {
        UpdateDisplayText();
    }
}

void HotKey::NotifyHotKeyChanged()
{
    const uint16_t nHotKeyValue = MAKEWORD(m_wVirtualKeyCode, m_wModifiers);
    if (nHotKeyValue != m_nLastHotKeyValue) {
        m_nLastHotKeyValue = nHotKeyValue;
        SendEvent(kEventTextChanged);
    }
}

DString HotKey::FormatHotKeyName(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    DString sKeyName;
    auto appendKey = [&sKeyName](const DString& sKey) {
        if (!sKey.empty()) {
            if (!sKeyName.empty()) {
                sKeyName += _T(" + ");
            }
            sKeyName += sKey;
        }
        };
    if (wModifiers & HOTKEYF_CONTROL) {
        appendKey(Keyboard::GetKeyName(kVK_CONTROL, false));
    }
    if (wModifiers & HOTKEYF_SHIFT) {
        appendKey(Keyboard::GetKeyName(kVK_SHIFT, false));
    }
    if (wModifiers & HOTKEYF_ALT) {
        appendKey(Keyboard::GetKeyName(kVK_MENU, false));
    }
    if ((wVirtualKeyCode != 0) &&
        (wVirtualKeyCode != kVK_SHIFT) && (wVirtualKeyCode != kVK_CONTROL) && (wVirtualKeyCode != kVK_MENU)) {
        appendKey(Keyboard::GetKeyName((VirtualKeyCode)wVirtualKeyCode, (wModifiers & HOTKEYF_EXT) != 0));
    }
    return sKeyName;
}

}//namespace ui
