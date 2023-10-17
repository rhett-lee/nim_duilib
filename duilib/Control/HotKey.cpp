#include "HotKey.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"

namespace ui
{

#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08

class HotKeyRichEdit : public RichEdit
{
public:

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
        bool bShiftDown = ::GetAsyncKeyState(VK_SHIFT) < 0;
        bool bControlDown = ::GetAsyncKeyState(VK_CONTROL) < 0;
        bool bAltDown = ::GetAsyncKeyState(VK_MENU) < 0;
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
        if (msg.lParam & KF_EXTENDED) {
            wModifiers |= HOTKEYF_EXT;
        }

        if ((msg.wParam == VK_DELETE) || (msg.wParam == VK_BACK)) {
            //清空文本
            SetTextNoEvent(m_defaultText.c_str());
        }
        else if (msg.wParam == VK_MENU) {
            SetHotKey(0, wModifiers);
        }
        else if (msg.wParam == VK_SHIFT) {
            SetHotKey(0, wModifiers);
        }
        else if (msg.wParam == VK_CONTROL) {
            SetHotKey(0, wModifiers);
        }
        else {
            std::wstring keyName = GetKeyName(msg.lParam);
            if (!keyName.empty()) {
                SetHotKey(static_cast<uint8_t>(msg.wParam), wModifiers);
            }
        }
        std::wstring keyName = GetHotKeyName();
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
        if ((wVirtualKeyCode == VK_MENU) ||
            (wVirtualKeyCode == VK_SHIFT) ||
            (wVirtualKeyCode == VK_CONTROL)) {
            wVirtualKeyCode = 0;
        }
        if ((wVirtualKeyCode == 0) || (wModifiers == 0)) {
            //无有效的热键，清空文本
            SetTextNoEvent(m_defaultText.c_str());
        }
        return true;
    }

    /** 按键：F10 或者 Alt
    */
    virtual bool OnSysKeyDown(const EventArgs& msg) override
    {
        return OnKeyDown(msg);
    }

    /** 按键：F10 或者 Alt
    */
    virtual bool OnSysKeyUp(const EventArgs& msg) override
    {
        return OnKeyUp(msg);
    }

    /** 获取键的显示名称
    */
    static std::wstring GetKeyName(uint8_t wParam, bool fExtended)
    {
        UINT nScanCode = ::MapVirtualKeyEx(wParam, 0, ::GetKeyboardLayout(0));
        switch (wParam)
        {
            // Keys which are "extended" (except for Return which is Numeric Enter as extended)
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_NEXT:  // Page down
        case VK_PRIOR: // Page up
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            nScanCode |= 0x100; // Add extended bit
            break;
        default:
            break;
        }
        if (fExtended) {
            nScanCode |= 0x01000000L;
        }

        TCHAR szStr[MAX_PATH] = { 0 };
        ::GetKeyNameText(nScanCode << 16, szStr, MAX_PATH);
        return std::wstring(szStr);
    }

    /** 获取键的显示名称
    */
    std::wstring GetKeyName(LPARAM lParam) const
    {
        TCHAR szStr[MAX_PATH] = { 0 };
        ::GetKeyNameText((LONG)lParam, szStr, MAX_PATH);
        std::wstring keyName = szStr;
        return keyName;
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
    std::wstring GetHotKeyName() const
    {
        std::wstring sKeyName;
        uint8_t wCode = 0;
        uint8_t wModifiers = 0;
        const wchar_t szPlus[] = L" + ";
        GetHotKey(wCode, wModifiers);
        if (wModifiers == 0) {
            //必须有组合键，才是有效的热键
            return sKeyName;
        }
        if (wCode != 0 || wModifiers != 0) {
            if (wModifiers & HOTKEYF_CONTROL) {
                sKeyName += GetKeyName(VK_CONTROL, wModifiers & HOTKEYF_EXT);
                sKeyName += szPlus;
            }
            if (wModifiers & HOTKEYF_SHIFT) {
                sKeyName += GetKeyName(VK_SHIFT, wModifiers & HOTKEYF_EXT);
                sKeyName += szPlus;
            }
            if (wModifiers & HOTKEYF_ALT) {
                sKeyName += GetKeyName(VK_MENU, wModifiers & HOTKEYF_EXT);
                sKeyName += szPlus;
            }
            if ((wCode != VK_SHIFT) && (wCode != VK_CONTROL) && (wCode != VK_MENU)) {
                sKeyName += GetKeyName(wCode, wModifiers & HOTKEYF_EXT);
            }
        }
        return sKeyName;
    }

    /** 设置默认的文本
    */
    void SetDefaultText(const std::wstring& defaultText)
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

HotKey::HotKey():
    m_bInited(false)
{
    m_pRichEdit = new HotKeyRichEdit();
}

HotKey::~HotKey()
{
    if (!m_bInited) {
        if (m_pRichEdit != nullptr) {
            delete m_pRichEdit;
            m_pRichEdit = nullptr;
        }
    }
}

std::wstring HotKey::GetType() const { return DUI_CTR_HOTKEY; }

void HotKey::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"default_text") {
        m_defaultText = strValue;
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void HotKey::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;
    __super::DoInit();
    HotKeyRichEdit* pRichEdit = m_pRichEdit;
    ASSERT(pRichEdit != nullptr);
    pRichEdit->SetDefaultText(m_defaultText.c_str());
    pRichEdit->SetText(m_defaultText.c_str());
    pRichEdit->SetAttribute(L"text_align", L"vcenter,hcenter");
    pRichEdit->SetAttribute(L"want_tab", L"false");
    AddItem(pRichEdit);
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

    std::wstring hotKeyName = m_pRichEdit->GetHotKeyName();
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

std::wstring HotKey::GetHotKeyName() const
{
    return m_pRichEdit->GetHotKeyName();
}

std::wstring HotKey::GetKeyName(uint8_t wVirtualKeyCode, bool fExtended)
{
    return HotKeyRichEdit::GetKeyName(wVirtualKeyCode, fExtended);
}

}//namespace ui

