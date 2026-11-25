#include "HotKey.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Utils/StringUtil.h"
#include <map>
#include <list>

namespace ui
{

/** 当系统接口无法获取键名时，使用固定的英文键名解析虚拟键码（仅支持字母、数字和F1-F24）
*/
static uint8_t FindFixedVirtualKey(const DString& keyName)
{
    if (keyName.size() == 1) {
        const wchar_t chKey = keyName[0];
        if ((chKey >= _T('a')) && (chKey <= _T('z'))) {
            return static_cast<uint8_t>(kVK_A + (chKey - _T('a')));
        }
        if ((chKey >= _T('0')) && (chKey <= _T('9'))) {
            return static_cast<uint8_t>(kVK_0 + (chKey - _T('0')));
        }
    }
    else if ((keyName.size() >= 2) && (keyName[0] == _T('f'))) {
        const int32_t nIndex = StringUtil::StringToInt32(keyName.c_str() + 1);
        if ((nIndex >= 1) && (nIndex <= 24)) {
            return static_cast<uint8_t>(kVK_F1 + nIndex - 1);
        }
    }
    return 0;
}

/** 当系统接口无法获取键名时，使用固定的英文键名（用于显示热键）
*/
static DString FindFixedKeyName(uint8_t wVirtualKeyCode)
{
    if ((wVirtualKeyCode >= kVK_A) && (wVirtualKeyCode <= kVK_Z)) {
        return DString(1, (wchar_t)(_T('A') + (wVirtualKeyCode - kVK_A)));
    }
    if ((wVirtualKeyCode >= kVK_0) && (wVirtualKeyCode <= kVK_9)) {
        return DString(1, (wchar_t)(_T('0') + (wVirtualKeyCode - kVK_0)));
    }
    if ((wVirtualKeyCode >= kVK_NUMPAD0) && (wVirtualKeyCode <= kVK_NUMPAD9)) {
        return _T("Num") + DString(1, (wchar_t)(_T('0') + (wVirtualKeyCode - kVK_NUMPAD0)));
    }
    if ((wVirtualKeyCode >= kVK_F1) && (wVirtualKeyCode <= kVK_F24)) {
        return _T("F") + StringUtil::Int32ToString((int32_t)wVirtualKeyCode - (int32_t)kVK_F1 + 1);
    }
    switch (wVirtualKeyCode) {
    case kVK_BACK:      return _T("Backspace");
    case kVK_TAB:       return _T("Tab");
    case kVK_RETURN:    return _T("Enter");
    case kVK_SHIFT:     return _T("Shift");
    case kVK_CONTROL:   return _T("Ctrl");
    case kVK_MENU:      return _T("Alt");
    case kVK_PAUSE:     return _T("Pause");
    case kVK_CAPITAL:   return _T("CapsLock");
    case kVK_ESCAPE:    return _T("Esc");
    case kVK_SPACE:     return _T("Space");
    case kVK_PRIOR:     return _T("PageUp");
    case kVK_NEXT:      return _T("PageDown");
    case kVK_END:       return _T("End");
    case kVK_HOME:      return _T("Home");
    case kVK_LEFT:      return _T("Left");
    case kVK_UP:        return _T("Up");
    case kVK_RIGHT:     return _T("Right");
    case kVK_DOWN:      return _T("Down");
    case kVK_INSERT:    return _T("Insert");
    case kVK_DELETE:    return _T("Delete");
    case kVK_SNAPSHOT:  return _T("PrintScreen");
    case kVK_NUMLOCK:   return _T("NumLock");
    case kVK_SCROLL:    return _T("ScrollLock");
    case kVK_MULTIPLY:  return _T("Num*");
    case kVK_ADD:       return _T("Num+");
    case kVK_SUBTRACT:  return _T("Num-");
    case kVK_DECIMAL:   return _T("Num.");
    case kVK_DIVIDE:    return _T("Num/");
    default:            break;
    }
    return _T("");
}

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

        //Esc：取消本次输入，恢复为已提交热键的显示，并通知外部（如属性页）退出编辑
        if (msg.vkCode == kVK_ESCAPE) {
            pOwner->RestoreDisplayText();
            pOwner->SendEvent(kEventEsc);
            return true;
        }
        //Enter：应用当前的热键，并通知外部（如属性页）退出编辑状态
        if (msg.vkCode == kVK_RETURN) {
            pOwner->SendEvent(kEventReturn);
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
        //普通按键：提交热键（必须符合热键规则，不符合规则时保持原热键不变）
        DString sKeyName = Keyboard::GetKeyName(msg.vkCode, (wModifiers & HOTKEYF_EXT) != 0);
        if (sKeyName.empty()) {
            //系统接口无法获取键名时，使用固定的英文键名
            sKeyName = FindFixedKeyName((uint8_t)msg.vkCode);
        }
        if (!sKeyName.empty()) {
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
    m_pRichEdit->SetAttribute(_T("text_align"), _T("vcenter,left"));
    m_pRichEdit->SetAttribute(_T("text_padding"), _T("8,0,8,0"));
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

bool HotKey::SetHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
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
    return ApplyHotKey(wVirtualKeyCode, wNewModifiers);
}

void HotKey::GetHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    wVirtualKeyCode = m_wVirtualKeyCode;
    wModifiers = m_wModifiers;
}

bool HotKey::SetHotKey(uint16_t dwHotKey)
{
    uint8_t wVirtualKeyCode = LOBYTE(dwHotKey);
    uint8_t wModifiers = HIBYTE(dwHotKey);
    return SetHotKey(wVirtualKeyCode, wModifiers);
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
        //修饰键的匹配：同时兼容系统本地化的名称和英文名称，避免不同输入法/键盘布局下解析失败
        if ((hotKey == keyCtrl) || (hotKey == _T("ctrl")) || (hotKey == _T("control"))) {
            wModifiers |= kHotKey_Contrl;
            iter = hotKeyList.erase(iter);
            continue;
        }
        else if ((hotKey == keyShift) || (hotKey == _T("shift"))) {
            wModifiers |= kHotKey_Shift;
            iter = hotKeyList.erase(iter);
            continue;
        }
        else if ((hotKey == keyAlt) || (hotKey == _T("alt")) || (hotKey == _T("menu"))) {
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
                //同一个名称可能对应多个虚拟键码（比如部分保留值的名称与字母键重复），保留第一个（即标准键码）
                vkCodeMap.insert(std::make_pair(temp, (uint8_t)vkCode));
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
                vkCodeExtMap.insert(std::make_pair(temp, (uint8_t)vkCode));
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
    if ((wVirtualKeyCode == 0) && !hotKeyList.empty()) {
        //系统接口无法获取键名时，使用固定的英文键名作为后备（如字母、数字、F1-F24）
        for (const DString& hotKey : hotKeyList) {
            uint8_t vkCode = FindFixedVirtualKey(hotKey);
            if (vkCode != 0) {
                wVirtualKeyCode = vkCode;
                break;
            }
        }
    }

    if (wVirtualKeyCode == 0) {
        //没有主键时（比如仅"Ctrl"），不是有效的热键
        return false;
    }
    return SetHotKey(wVirtualKeyCode, wModifiers);
}

DString HotKey::GetKeyName(uint8_t wVirtualKeyCode, bool fExtended)
{
    return Keyboard::GetKeyName((VirtualKeyCode)wVirtualKeyCode, fExtended);
}

bool HotKey::IsValidHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    //主键不能为空，也不能是修饰键本身：热键必须包含一个非修饰键
    if ((wVirtualKeyCode == 0) ||
        (wVirtualKeyCode == kVK_SHIFT) ||
        (wVirtualKeyCode == kVK_CONTROL) ||
        (wVirtualKeyCode == kVK_MENU)) {
        return false;
    }
    const uint8_t wKeyModifiers = wModifiers & (kHotKey_Contrl | kHotKey_Shift | kHotKey_Alt);
    if (wKeyModifiers == 0) {
        //不带修饰键时，只允许功能键（F1-F24）单独作为热键；
        //单个字母、数字等字符键必须与修饰键组合使用
        return (wVirtualKeyCode >= kVK_F1) && (wVirtualKeyCode <= kVK_F24);
    }
    if ((wKeyModifiers & kHotKey_Alt) && !(wKeyModifiers & (kHotKey_Contrl | kHotKey_Shift))) {
        //系统保留的Alt组合键不允许
        if ((wVirtualKeyCode == kVK_TAB) || (wVirtualKeyCode == kVK_F4) ||
            (wVirtualKeyCode == kVK_ESCAPE) || (wVirtualKeyCode == kVK_SPACE)) {
            return false;
        }
    }
    if ((wKeyModifiers == kHotKey_Contrl) && (wVirtualKeyCode == kVK_ESCAPE)) {
        //Ctrl+Esc为系统保留组合键（打开开始菜单）
        return false;
    }
    return true;
}

bool HotKey::ApplyHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    if (wVirtualKeyCode == 0) {
        //虚拟键码为空时，视为清空热键
        wModifiers = 0;
    }
    else if (!IsValidHotKey(wVirtualKeyCode, wModifiers)) {
        //不符合热键规则：不修改当前热键
        return false;
    }
    m_bPreviewing = false;
    m_wVirtualKeyCode = wVirtualKeyCode;
    m_wModifiers = wModifiers;
    UpdateDisplayText();
    NotifyHotKeyChanged();
    return true;
}

void HotKey::UpdateDisplayText()
{
    m_bPreviewing = false;
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetTextNoEvent(GetHotKeyName());
        //光标置于文本起始位置：文本较长时，优先显示前面的修饰键（避免只显示末尾的按键）
        m_pRichEdit->SetSel(0, 0);
        m_pRichEdit->ScrollCaret();
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
    //获取按键名称：修饰键统一使用固定名称（Ctrl/Shift/Alt），其他按键使用系统接口的名称
    //备注：系统接口的名称与平台/语言相关（比如Alt键在部分环境下显示为"Menu"），不符合使用习惯
    auto getKeyName = [](VirtualKeyCode nVirtKey, bool fExtended) -> DString {
        if ((nVirtKey == kVK_CONTROL) || (nVirtKey == kVK_SHIFT) || (nVirtKey == kVK_MENU)) {
            return FindFixedKeyName((uint8_t)nVirtKey);
        }
        DString sKey = Keyboard::GetKeyName(nVirtKey, fExtended);
        if (sKey.empty()) {
            sKey = FindFixedKeyName((uint8_t)nVirtKey);
        }
        return sKey;
        };
    if (wModifiers & HOTKEYF_CONTROL) {
        appendKey(getKeyName(kVK_CONTROL, false));
    }
    if (wModifiers & HOTKEYF_SHIFT) {
        appendKey(getKeyName(kVK_SHIFT, false));
    }
    if (wModifiers & HOTKEYF_ALT) {
        appendKey(getKeyName(kVK_MENU, false));
    }
    if ((wVirtualKeyCode != 0) &&
        (wVirtualKeyCode != kVK_SHIFT) && (wVirtualKeyCode != kVK_CONTROL) && (wVirtualKeyCode != kVK_MENU)) {
        appendKey(getKeyName((VirtualKeyCode)wVirtualKeyCode, (wModifiers & HOTKEYF_EXT) != 0));
    }
    return sKeyName;
}

}//namespace ui
