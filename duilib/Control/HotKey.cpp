#include "HotKey.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"
#include "duilib/Core/Keyboard.h"

namespace ui
{

#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08

class HotKeyRichEdit : public RichEdit
{
public:
    explicit HotKeyRichEdit(Window* pWindow):
        RichEdit(pWindow),
        m_wVirtualKeyCode(0),
        m_wModifiers(0)
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
        SetHotKey(0, 0);
        bool bShiftDown = Keyboard::IsKeyDown(kVK_SHIFT);
        bool bControlDown = Keyboard::IsKeyDown(kVK_CONTROL);
        bool bAltDown = Keyboard::IsKeyDown(kVK_MENU);
        uint8_t wModifiers = 0;
        if (bShiftDown) {
            wModifiers |= HOTKEYF_SHIFT;
        }
        if (bControlDown) {
            wModifiers |= HOTKEYF_CONTROL;
        }
        if (bAltDown) {
            wModifiers |= HOTKEYF_ALT;
        }
        if ((msg.lParam >> 16) & kKF_EXTENDED) {
            wModifiers |= HOTKEYF_EXT;
        }

        if ((msg.wParam == kVK_DELETE) || (msg.wParam == kVK_BACK)) {
            //清空文本
            SetTextNoEvent(m_defaultText.c_str());
        }
        else if (msg.wParam == kVK_MENU) {
            SetHotKey(0, wModifiers);
        }
        else if (msg.wParam == kVK_SHIFT) {
            SetHotKey(0, wModifiers);
        }
        else if (msg.wParam == kVK_CONTROL) {
            SetHotKey(0, wModifiers);
        }
        else {
            DString keyName = Keyboard::GetKeyName(msg.vkCode, false);
            if (!keyName.empty()) {
                SetHotKey(static_cast<uint8_t>(msg.wParam), wModifiers);
            }
        }
        DString keyName = GetHotKeyName();
        if (!keyName.empty()) {
            SetTextNoEvent(keyName);
        }
        return true;
    }

    /** 按键事件
    */
    virtual bool OnKeyUp(const EventArgs& /*msg*/) override
    {
        uint8_t wVirtualKeyCode = 0;
        uint8_t wModifiers = 0;
        GetHotKey(wVirtualKeyCode, wModifiers);
        if ((wVirtualKeyCode == kVK_MENU) ||
            (wVirtualKeyCode == kVK_SHIFT) ||
            (wVirtualKeyCode == kVK_CONTROL)) {
            wVirtualKeyCode = 0;
        }
        if ((wVirtualKeyCode == 0) || (wModifiers == 0)) {
            //无有效的热键，清空文本
            SetTextNoEvent(m_defaultText.c_str());
        }
        return true;
    }

    /** 设置热键
    */
    void SetHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
    {
        m_wVirtualKeyCode = wVirtualKeyCode;
        m_wModifiers = wModifiers;
    }

    /** 获取热键
    */
    void GetHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
    {
        wVirtualKeyCode = m_wVirtualKeyCode;
        wModifiers = m_wModifiers;
    }

    /** 获取热键显示名称
    */
    DString GetHotKeyName() const
    {
        DString sKeyName;
        uint8_t wCode = 0;
        uint8_t wModifiers = 0;
        const DString::value_type szPlus[] = _T("+");
        GetHotKey(wCode, wModifiers);
        if (wModifiers == 0) {
            //必须有组合键，才是有效的热键
            return sKeyName;
        }
        if (wCode != 0 || wModifiers != 0) {
            if (wModifiers & HOTKEYF_CONTROL) {
                DString sKey = Keyboard::GetKeyName(kVK_CONTROL, wModifiers & HOTKEYF_EXT);
                if (!sKey.empty()) {
                    if (!sKeyName.empty()) {
                        sKeyName += szPlus;
                    }
                    sKeyName += sKey;
                }
            }
            if (wModifiers & HOTKEYF_SHIFT) {
                DString sKey = Keyboard::GetKeyName(kVK_SHIFT, wModifiers & HOTKEYF_EXT);
                if (!sKey.empty()) {
                    if (!sKeyName.empty()) {
                        sKeyName += szPlus;
                    }
                    sKeyName += sKey;
                }
            }
            if (wModifiers & HOTKEYF_ALT) {
                DString sKey = Keyboard::GetKeyName(kVK_MENU, wModifiers & HOTKEYF_EXT);
                if (!sKey.empty()) {
                    if (!sKeyName.empty()) {
                        sKeyName += szPlus;
                    }
                    sKeyName += sKey;
                }
            }
            if ((wCode != kVK_SHIFT) && (wCode != kVK_CONTROL) && (wCode != kVK_MENU)) {
                DString sKey = Keyboard::GetKeyName((VirtualKeyCode)wCode, wModifiers & HOTKEYF_EXT);
                if (!sKey.empty()) {
                    if (!sKeyName.empty()) {
                        sKeyName += szPlus;
                    }
                    sKeyName += sKey;
                }
            }
        }
        return sKeyName;
    }

    /** 设置默认的文本
    */
    void SetDefaultText(const DString& defaultText)
    {
        m_defaultText = defaultText;
    }

private:

    /** 虚拟键盘码，比如：VK_DOWN等
        可参考：https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
    */
    uint8_t m_wVirtualKeyCode;

    /** 热键组合键：HOTKEYF_SHIFT 或者 HOTKEYF_CONTROL 或者 HOTKEYF_ALT
    */
    uint8_t m_wModifiers;

    /** 默认的文本
    */
    UiString m_defaultText;
};

HotKey::HotKey(Window* pWindow):
    HBox(pWindow)
{
    ASSERT(pWindow != nullptr);
    m_pRichEdit = new HotKeyRichEdit(pWindow);
    m_pRichEdit->SetAttribute(_T("text_align"), _T("vcenter,hcenter"));
    m_pRichEdit->SetAttribute(_T("want_tab"), _T("false"));
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

void HotKey::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("default_text")) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetDefaultText(strValue);
            m_pRichEdit->SetText(strValue);
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
        SendEvent(kEventSetFocus);
        return true;
        });
    pRichEdit->AttachKillFocus([this](const EventArgs&) {
        SendEvent(kEventKillFocus);
        return true;
        });
}

void HotKey::SetFocus()
{
    if (IsVisible() && IsEnabled() && (m_pRichEdit != nullptr)) {
        m_pRichEdit->SetFocus();
    }
    else {
        BaseClass::SetFocus();
    }
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
    m_pRichEdit->SetHotKey(wVirtualKeyCode, wNewModifiers);

    DString hotKeyName = m_pRichEdit->GetHotKeyName();
    if (!hotKeyName.empty()) {
        m_pRichEdit->SetText(hotKeyName);
    }
}

void HotKey::GetHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    m_pRichEdit->GetHotKey(wVirtualKeyCode, wModifiers);
}

void HotKey::SetHotKey(uint16_t dwHotKey)
{
    uint8_t wVirtualKeyCode = LOBYTE(dwHotKey);
    uint8_t wModifiers = HIBYTE(dwHotKey);
    SetHotKey(wVirtualKeyCode, wModifiers);
}

uint16_t HotKey::GetHotKey() const
{
    uint8_t wVirtualKeyCode = 0;
    uint8_t wModifiers = 0;
    GetHotKey(wVirtualKeyCode, wModifiers);
    return (MAKEWORD(wVirtualKeyCode, wModifiers));
}

DString HotKey::GetHotKeyName() const
{
    return m_pRichEdit->GetHotKeyName();
}

bool HotKey::SetHotKeyName(const DString& hotKeyName)
{
    std::list<DString> hotKeyList = StringUtil::Split(hotKeyName, _T("+"));
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

    bool bRet = false;
    if ((wVirtualKeyCode != 0) || (wModifiers != 0)) {
        SetHotKey(wVirtualKeyCode, wModifiers);
        bRet = true;
    }
    return bRet;
}

DString HotKey::GetKeyName(uint8_t wVirtualKeyCode, bool fExtended)
{
    return Keyboard::GetKeyName((VirtualKeyCode)wVirtualKeyCode, fExtended);
}

}//namespace ui

