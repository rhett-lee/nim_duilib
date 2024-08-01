#include "RichEdit.h"
#include "RichEditHost.h"
#include "RichEditCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/WindowDropTarget.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/BitmapHelper_Windows.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Menu.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/Button.h"

#ifdef DUILIB_BUILD_FOR_WIN

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>
#endif

namespace ui {

/** 拖放操作接口的实现（仅是拖入操作）
*/
class RichEditDropTarget : public ControlDropTarget
{
public:
    RichEditDropTarget(RichEdit* pRichEdit, ITextServices* pTextServices):
        m_pRichEdit(pRichEdit),
        m_pTextServices(pTextServices)
    {
    }

    //IDropTarget::DragEnter
    virtual int32_t DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override
    {
        HRESULT hr = S_FALSE;
        if (m_pTextServices == nullptr) {
            return hr;
        }
        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            DWORD dwEffect = DROPEFFECT_NONE;
            if (pdwEffect != nullptr) {
                dwEffect = *pdwEffect;
            }
            hr = pDropTarget->DragEnter((IDataObject*)pDataObj, grfKeyState, POINTL(pt.x, pt.y), &dwEffect);
            if (pdwEffect != nullptr) {
                *pdwEffect = dwEffect;
            }
            pDropTarget->Release();
        }
        return hr;
    }

    //IDropTarget::DragOver
    virtual int32_t DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override
    {
        HRESULT hr = S_FALSE;
        if (m_pTextServices == nullptr) {
            return hr;
        }
        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            //设置当前RichEdit控件的光标到鼠标所在位置，方便查看拖放目标位置
            if (m_pRichEdit != nullptr) {
                UiPoint clientPt = pt;
                m_pRichEdit->ScreenToClient(clientPt);
                if (m_pRichEdit->GetPos().ContainsPt(clientPt)) {                    
                    int32_t pos = m_pRichEdit->CharFromPos(clientPt);
                    if (pos >= 0) {
                        UiPoint charPt = m_pRichEdit->PosFromChar(pos);
                        m_pRichEdit->SetCaretPos(charPt.x, charPt.y);
                        m_pRichEdit->ShowCaret(true);
                    }
                }
            }

            //转接给文字服务
            DWORD dwEffect = DROPEFFECT_NONE;
            if (pdwEffect != nullptr) {
                dwEffect = *pdwEffect;
            }
            hr = pDropTarget->DragOver(grfKeyState, POINTL(pt.x, pt.y), &dwEffect);
            if (pdwEffect != nullptr) {
                *pdwEffect = dwEffect;
            }
            pDropTarget->Release();
        }
        return hr;
    }

    //IDropTarget::DragLeave
    virtual int32_t DragLeave(void) override
    {
        HRESULT hr = S_FALSE;
        if (m_pTextServices == nullptr) {
            return hr;
        }
        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            hr = pDropTarget->DragLeave();
            pDropTarget->Release();
        }
        return hr;
    }

    //IDropTarget::Drop
    virtual int32_t Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect) override
    {
        HRESULT hr = S_FALSE;
        if (m_pTextServices == nullptr) {
            return hr;
        }
        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            DWORD dwEffect = DROPEFFECT_NONE;
            if (pdwEffect != nullptr) {
                dwEffect = *pdwEffect;
            }
            hr = pDropTarget->Drop((IDataObject*)pDataObj, grfKeyState, POINTL(pt.x, pt.y), &dwEffect);
            if (pdwEffect != nullptr) {
                *pdwEffect = dwEffect;
            }
            pDropTarget->Release();
        }
        return hr;
    }

private:
    /** RichEdit接口
    */
    RichEdit* m_pRichEdit;

    /** 文字服务接口
    */
    ITextServices* m_pTextServices;
};

RichEdit::RichEdit(Window* pWindow) :
    ScrollBox(pWindow, new Layout),
    m_pRichHost(nullptr), 
    m_bVScrollBarFixing(false), 
    m_bWantTab(true),
    m_bNeedReturnMsg(false),
    m_bReturnMsgWantCtrl(false),
    m_bAllowPrompt(false),
    m_bSelAllEver(false),         
    m_bNoSelOnKillFocus(true), 
    m_bSelAllOnFocus(false),    
    m_bNoCaretReadonly(false),
    m_bIsCaretVisiable(false),
    m_bIsComposition(false),
    m_iCaretPosX(0),
    m_iCaretPosY(0),
    m_iCaretWidth(0),
    m_iCaretHeight(0),
    m_sFontId(),
    m_sTextColor(),
    m_sDisabledTextColor(),
    m_sPromptColor(),
    m_sPromptText(),
    m_drawCaretFlag(),
    m_timeFlagMap(),
    m_pFocusedImage(nullptr),
    m_bUseControlCursor(false),
    m_bEnableWheelZoom(false),
    m_bEnableDefaultContextMenu(false),
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pControlDropTarget(nullptr),
#endif
    m_bDisableTextChangeEvent(false),
    m_maxNumber(INT_MAX),
    m_minNumber(INT_MIN),
    m_pSpinBox(nullptr),
    m_pClearButton(nullptr),
    m_pShowPasswordButton(nullptr)
{
    //这个标记必须为false，否则绘制有问题
    SetUseCache(false);

    //创建RichEditHost接口
    m_pRichHost = new RichEditHost(this);
    ASSERT(m_pRichHost->GetTextServices() != nullptr);
    m_richCtrl.SetTextServices(m_pRichHost->GetTextServices());
}

RichEdit::~RichEdit()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pControlDropTarget != nullptr) {
        UnregisterDragDrop();
        delete m_pControlDropTarget;
        m_pControlDropTarget = nullptr;
    }
#endif
    if( m_pRichHost != nullptr) {
        m_richCtrl.SetTextServices(nullptr);
        m_pRichHost->ShutdownTextServices();
        m_pRichHost->Release();
        m_pRichHost = nullptr;
    }
    if (m_pFocusedImage != nullptr) {
        delete m_pFocusedImage;
        m_pFocusedImage = nullptr;
    }
    m_pLimitChars.reset();
}

void RichEdit::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("vscrollbar")) {
        //纵向滚动条
        if (strValue == _T("true")) {
            EnableScrollBar(true, GetHScrollBar() != NULL);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetVScrollBar(true);
            }
        }
        else {
            EnableScrollBar(false, GetHScrollBar() != NULL);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetVScrollBar(false);
            }
        }
    }
    else if (strName == _T("hscrollbar")) {
        //横向滚动条
        if (strValue == _T("true")) {
            EnableScrollBar(GetVScrollBar() != NULL, true);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetHScrollBar(true);
            }
        }
        else {
            EnableScrollBar(GetVScrollBar() != NULL, false);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetHScrollBar(false);
            }
        }
    }
    else if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetMultiLine(strValue != _T("true"));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        SetMultiLine(strValue == _T("true"));
    }
    else if (strName == _T("readonly")) {
        SetReadOnly(strValue == _T("true"));
    }
    else if (strName == _T("password")) {
        SetPassword(strValue == _T("true"));
    }
    else if (strName == _T("show_password")) {
        SetShowPassword(strValue == _T("true"));
    }
    else if (strName == _T("password_char")) {
        if (!strValue.empty()) {
            SetPasswordChar(strValue.front());
        }
    }
    else if (strName == _T("flash_password_char")) {
        SetFlashPasswordChar(strValue == _T("true"));
    }
    else if ((strName == _T("number_only")) || (strName == _T("number"))) {
        SetNumberOnly(strValue == _T("true"));
    }
    else if (strName == _T("max_number")) {
        SetMaxNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("min_number")) {
        SetMinNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("text_align")) {
        if (strValue.find(_T("left")) != DString::npos) {
            SetHAlignType(kHorAlignLeft);
        }
        if (strValue.find(_T("right")) != DString::npos) {
            SetHAlignType(kHorAlignRight);
        }
        if (strValue.find(_T("hcenter")) != DString::npos) {
            SetHAlignType(kHorAlignCenter);
        }

        if (strValue.find(_T("top")) != DString::npos) {
            SetVAlignType(kVerAlignTop);
        }
        if (strValue.find(_T("bottom")) != DString::npos) {
            SetVAlignType(kVerAlignBottom);
        }
        if (strValue.find(_T("vcenter")) != DString::npos) {
            SetVAlignType(kVerAlignCenter);
        }
    }
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        UiPadding rcTextPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding, true);
    }
    else if ((strName == _T("text_color")) || (strName == _T("normal_text_color")) || (strName == _T("normaltextcolor"))) {
        SetTextColor(strValue);
    }
    else if ((strName == _T("disabled_text_color")) || (strName == _T("disabledtextcolor"))) {
        SetDisabledTextColor(strValue);
    }
    else if ((strName == _T("caret_color")) || (strName == _T("caretcolor"))) {
        //设置光标的颜色
        SetCaretColor(strValue);
    }
    else if ((strName == _T("prompt_mode")) || (strName == _T("promptmode"))) {
        //提示模式
        m_bAllowPrompt = (strValue == _T("true")) ? true : false;
    }
    else if ((strName == _T("prompt_color")) || (strName == _T("promptcolor"))) {
        //提示文字的颜色
        m_sPromptColor = strValue;
    }
    else if ((strName == _T("prompt_text")) || (strName == _T("prompttext"))) {
        //提示文字
        SetPromptText(strValue);
    }
    else if ((strName == _T("prompt_text_id")) || (strName == _T("prompt_textid")) || (strName == _T("prompttextid"))) {
        //提示文字ID
        SetPromptTextId(strValue);
    }
    else if ((strName == _T("focused_image")) || (strName == _T("focusedimage"))) {
        SetFocusedImage(strValue);
    }
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        SetTextId(strValue);
    }
    else if ((strName == _T("want_tab")) || (strName == _T("wanttab"))) {
        SetWantTab(strValue == _T("true"));
    }
    else if ((strName == _T("want_return_msg")) || (strName == _T("wantreturnmsg"))) {
        SetNeedReturnMsg(strValue == _T("true"));
    }
    else if ((strName == _T("return_msg_want_ctrl")) || (strName == _T("returnmsgwantctrl"))) {
        SetReturnMsgWantCtrl(strValue == _T("true"));
    }
    else if ((strName == _T("limit_text")) || (strName == _T("max_char")) || (strName == _T("maxchar"))) {
        //限制最多字符数
        SetLimitText(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("limit_chars")) {
        //限制允许输入哪些字符
        SetLimitChars(strValue);
    }
    else if (strName == _T("word_wrap")) {
        //是否自动换行
        SetWordWrap(strValue == _T("true"));
    }
    else if (strName == _T("no_caret_readonly")) {
        //只读模式，不显示光标
        SetNoCaretReadonly();
    }
    else if (strName == _T("default_context_menu")) {
        //是否使用默认的右键菜单
        SetEnableDefaultContextMenu(strValue == _T("true"));
    }
    else if (strName == _T("spin_class")) {
        SetSpinClass(strValue);
    }
    else if (strName == _T("clear_btn_class")) {
        SetClearBtnClass(strValue);
    }
    else if (strName == _T("show_passowrd_btn_class")) {
        SetShowPasswordBtnClass(strValue);
    }
    else if (strName == _T("wheel_zoom")) {
        //设置是否允许Ctrl + 滚轮来调整缩放比例
        SetEnableWheelZoom(strValue == _T("true"));
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    else if (strName == _T("zoom")) {
        //缩放比例：
        //设置缩放比例：设 wParam：缩放比例的分子，lParam：缩放比例的分母，
        // "wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。
        // 举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50% 
        UiSize zoomValue;
        AttributeUtil::ParseSizeValue(strValue.c_str(), zoomValue);
        if ((zoomValue.cx >= 0) && (zoomValue.cx <= 64) &&
            (zoomValue.cy >= 0) && (zoomValue.cy <= 64)) {
            m_richCtrl.SetZoom(zoomValue.cx, zoomValue.cy);
        }
    }    
    else if ((strName == _T("auto_vscroll")) || (strName == _T("autovscroll"))) {
        //当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
        if (m_pRichHost != nullptr) {
            m_pRichHost->SetAutoVScroll(strValue == _T("true"));
        }
    }
    else if ((strName == _T("auto_hscroll")) || (strName == _T("autohscroll"))) {
        //当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
        //当用户按 Enter 时，控件会将所有文本滚动回零位置。
        if (m_pRichHost != nullptr) {
            m_pRichHost->SetAutoHScroll(strValue == _T("true"));
        }
    }
    else if ((strName == _T("rich_text")) || (strName == _T("rich"))) {
        //是否为富文本属性
        SetRichText(strValue == _T("true"));
    }
    else if (strName == _T("auto_detect_url")) {
        //是否自动检测URL，如果是URL则显示为超链接
        SetAutoURLDetect(strValue == _T("true"));
    }
    else if (strName == _T("allow_beep")) {
        //是否允许发出Beep声音
        SetAllowBeep(strValue == _T("true"));
    }
    else if (strName == _T("save_selection")) {
        //如果 为 TRUE，则当控件处于非活动状态时，应保存所选内容的边界。
        //如果 为 FALSE，则当控件再次处于活动状态时，可以选择边界重置为 start = 0，length = 0。
        SetSaveSelection(strValue == _T("true"));
    }
    else if (strName == _T("hide_selection")) {
        //是否隐藏选择内容
        SetHideSelection(strValue == _T("true"));
    }
    else if (strName == _T("enable_drag_drop")) {
        //是否允许拖放操作
        SetEnableDragDrop(strValue == _T("true"));
    }
#endif
    else {
        Box::SetAttribute(strName, strValue);
    }
}

void RichEdit::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);

    //更新字体大小
    CHARFORMAT2W cf;
    memset(&cf, 0, sizeof(CHARFORMAT2W));
    cf.cbSize = sizeof(CHARFORMAT2W);
    m_richCtrl.GetDefaultCharFormat(cf);
    cf.yHeight = Dpi().GetScaleInt((int32_t)cf.yHeight, nOldDpiScale);
    cf.dwMask |= CFM_SIZE;
    m_richCtrl.SetDefaultCharFormat(cf);

    __super::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void RichEdit::SetWindow(Window* pWindow)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsEnableDragDrop() && (pWindow != GetWindow())) {
        UnregisterDragDrop();
    }
#endif
    __super::SetWindow(pWindow);

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsEnableDragDrop()) {
        RegisterDragDrop();
    }
#endif
}

bool RichEdit::IsWantTab() const
{
    return m_bWantTab;
}

void RichEdit::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool RichEdit::CanPlaceCaptionBar() const
{
    return true;
}

bool RichEdit::IsNeedReturnMsg()
{
    return m_bNeedReturnMsg;
}

void RichEdit::SetNeedReturnMsg(bool bNeedReturnMsg)
{
    m_bNeedReturnMsg = bNeedReturnMsg;
}

bool RichEdit::IsReturnMsgWantCtrl()
{
    return m_bReturnMsgWantCtrl;
}

void RichEdit::SetReturnMsgWantCtrl(bool bReturnMsgWantCtrl)
{
    m_bReturnMsgWantCtrl = bReturnMsgWantCtrl;
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
bool RichEdit::IsRichText()
{
    TEXTMODE newTextMode = m_richCtrl.GetTextMode();
    return (newTextMode & TM_RICHTEXT) ? true : false;
}

void RichEdit::SetRichText(bool bRichText)
{
    if (IsRichText() == bRichText) {
        return;
    }
    uint32_t textMode = m_richCtrl.GetTextMode();
    if (bRichText) {
        textMode &= ~TM_PLAINTEXT;
        textMode |= TM_RICHTEXT;
    }
    else {
        textMode &= ~TM_RICHTEXT;
        textMode |= TM_PLAINTEXT;
    }

    //切换文本模式的时候，RichEdit的文本内容必须为空
    DString text = GetText();
    if (!text.empty()) {        
        SetTextNoEvent(_T(""));
        m_richCtrl.EmptyUndoBuffer();
    }
    m_richCtrl.SetTextMode((TEXTMODE)textMode);

    if (!text.empty()) {
        SetTextNoEvent(text);
        SetSel(0, 0);        
    }
#ifdef _DEBUG
    TEXTMODE newTextMode2 = m_richCtrl.GetTextMode();
    ASSERT((uint32_t)textMode & (uint32_t)newTextMode2);
#endif
}
#endif

bool RichEdit::IsReadOnly()
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsReadOnly();
    }
    return false;
}

void RichEdit::SetReadOnly(bool bReadOnly)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetReadOnly(bReadOnly);
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (bReadOnly) {
        //只读模式关闭拖放功能
        SetEnableDragDrop(false);
    }
#endif
}

bool RichEdit::IsPassword()
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsPassword();
    }
    return false;
}

void RichEdit::SetPassword(bool bPassword)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetPassword(bPassword);
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (bPassword) {
        //密码模式关闭拖放功能
        SetEnableDragDrop(false);
    }
#endif
}

void RichEdit::SetShowPassword(bool bShow)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetShowPassword(bShow);
    }
}

bool RichEdit::IsShowPassword() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsShowPassword();
    }
    return false;
}

void RichEdit::SetPasswordChar(wchar_t ch)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetPasswordChar(ch);
    }
}

void RichEdit::SetFlashPasswordChar(bool bFlash)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetFlashPasswordChar(bFlash);
    }
}

bool RichEdit::GetFlashPasswordChar() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsFlashPasswordChar();
    }
    return false;
}

bool RichEdit::IsNumberOnly() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsNumberOnly();
    }
    return false;
}

void RichEdit::SetNumberOnly(bool bNumberOnly)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetNumberOnly(bNumberOnly);
    }
}

void RichEdit::SetMaxNumber(int32_t maxNumber)
{
    m_maxNumber = maxNumber;
}

int32_t RichEdit::GetMaxNumber() const
{
    return m_maxNumber;
}

void RichEdit::SetMinNumber(int32_t minNumber)
{
    m_minNumber = minNumber;
}

int32_t RichEdit::GetMinNumber() const
{
    return m_minNumber;
}

bool RichEdit::GetWordWrap()
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsWordWrap();
    }
    return false;
}

void RichEdit::SetWordWrap(bool bWordWrap)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetWordWrap(bWordWrap);
    }
}

bool RichEdit::GetMultiLine() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsMultiLine();
    }
    return false;
}

void RichEdit::SetMultiLine(bool bMultiLine)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetMultiLine(bMultiLine);
    }
}

DString RichEdit::GetFontId() const
{
    return m_sFontId.c_str();
}

void RichEdit::SetFontId(const DString& strFontId)
{
    if (m_sFontId != strFontId) {
        m_sFontId = strFontId;
        SetFontIdInternal(strFontId);
    }
}

void RichEdit::SetTextColor(const DString& dwTextColor)
{
    m_sTextColor = dwTextColor;
    if (IsEnabled()) {
        UiColor dwTextColor2 = GetUiColor(dwTextColor);
        SetTextColorInternal(dwTextColor2);
    }
}

DString RichEdit::GetTextColor() const
{
    if (!m_sTextColor.empty()) {
        return m_sTextColor.c_str();
    }
    else {
        return GlobalManager::Instance().Color().GetDefaultTextColor();
    }
}

void RichEdit::SetDisabledTextColor(const DString& dwTextColor)
{
    m_sDisabledTextColor = dwTextColor;
    if (!IsEnabled()) {
        UiColor dwTextColor2 = GetUiColor(dwTextColor);
        SetTextColorInternal(dwTextColor2);
    }
}

DString RichEdit::GetDisabledTextColor() const
{
    if (!m_sDisabledTextColor.empty()) {
        return m_sDisabledTextColor.c_str();
    }
    else {
        return GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
    }
}

int32_t RichEdit::GetLimitText() const
{
    return m_richCtrl.GetLimitText();
}

void RichEdit::SetLimitText(int32_t iChars)
{
    if (iChars < 0) {
        iChars = 0;
    }
    m_richCtrl.SetLimitText(iChars);
}

DString RichEdit::GetLimitChars() const
{
    if (m_pLimitChars != nullptr) {
        return StringUtil::UTF16ToT(m_pLimitChars.get());
    }
    else {
        return DString();
    }
}

void RichEdit::SetLimitChars(const DString& limitChars)
{
    m_pLimitChars.reset();
    DStringW limitCharsW = StringUtil::TToUTF16(limitChars);
    if (!limitCharsW.empty()) {
        size_t nLen = limitCharsW.size() + 1;
        m_pLimitChars.reset(new wchar_t[nLen]);
        memset(m_pLimitChars.get(), 0, nLen * sizeof(wchar_t));
        StringUtil::StringCopy(m_pLimitChars.get(), nLen, limitCharsW.c_str());
    }
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
bool RichEdit::GetAllowBeep() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsAllowBeep();
    }
    return false;
}

void RichEdit::SetAllowBeep(bool bAllowBeep)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetAllowBeep(bAllowBeep);
    }
}
#endif

int32_t RichEdit::GetTextLength() const
{
    return m_richCtrl.GetTextLengthEx(GTL_DEFAULT, 1200);
}

DString RichEdit::GetText() const
{
    UINT uCodePage = 1200;
    int32_t nTextLen = m_richCtrl.GetTextLengthEx(GTL_DEFAULT, uCodePage);
    if (nTextLen < 1) {
        return DString();
    }
    nTextLen += 1;
    wchar_t* pText = new wchar_t[nTextLen];
    memset(pText, 0, sizeof(wchar_t) * nTextLen);
    m_richCtrl.GetTextEx(pText, nTextLen, GTL_DEFAULT, uCodePage);
    std::wstring sText(pText);
    delete[] pText;
    pText = nullptr;
#ifdef DUILIB_UNICODE
    return sText;
#else
    return StringUtil::UTF16ToT(sText);
#endif
}

std::string RichEdit::GetUTF8Text() const
{
    std::string strOut = StringUtil::TToUTF8(GetText());
    return strOut;
}

void RichEdit::SetText(const DString& strText)
{
    m_bDisableTextChangeEvent = false;
    SetSel(0, -1);
    ReplaceSel(strText, FALSE);

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_linkInfo.clear();
#endif
}

void RichEdit::SetTextNoEvent(const DString& strText)
{
    m_bDisableTextChangeEvent = true;
    SetSel(0, -1);
    ReplaceSel(strText, FALSE);
    m_bDisableTextChangeEvent = false;

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_linkInfo.clear();
#endif
}

void RichEdit::SetTextId(const DString& strTextId)
{
    DString strText = GlobalManager::Instance().Lang().GetStringViaID(strTextId);
    SetText(strText);
}

void RichEdit::SetUTF8Text( const std::string& strText )
{
    DString strOut = StringUtil::UTF8ToT(strText);
    SetText(strOut);
}

bool RichEdit::GetModify() const
{ 
    return m_richCtrl.GetModify();
}

void RichEdit::SetModify(bool bModified)
{ 
    m_richCtrl.SetModify(bModified);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
void RichEdit::GetSel(CHARRANGE& cr) const
{ 
    m_richCtrl.GetSel(cr);
}
#endif

void RichEdit::GetSel(long& nStartChar, long& nEndChar) const
{
    m_richCtrl.GetSel(nStartChar, nEndChar);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
int RichEdit::SetSel(CHARRANGE& cr)
{ 
    return m_richCtrl.SetSel(cr);
}
#endif

int RichEdit::SetSel(long nStartChar, long nEndChar)
{
    return m_richCtrl.SetSel(nStartChar, nEndChar);
}

void RichEdit::ReplaceSel(const DString& lpszNewText, bool bCanUndo)
{
#ifdef DUILIB_UNICODE
    m_richCtrl.ReplaceSel(lpszNewText.c_str(), bCanUndo);
#else
    std::wstring newText = StringUtil::TToUTF16(lpszNewText);
    m_richCtrl.ReplaceSel(newText.c_str(), bCanUndo);
#endif
}

DString RichEdit::GetSelText() const
{
    DString text;
#ifdef DUILIB_UNICODE    
    m_richCtrl.GetSelText(text);
#else
    DStringW textW;
    m_richCtrl.GetSelText(textW);
    text = StringUtil::UTF16ToUTF8(textW);
#endif
    return text;
}

int RichEdit::SetSelAll()
{
    return m_richCtrl.SetSelAll();
}

int RichEdit::SetSelNone()
{
    return m_richCtrl.SetSelNone();
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
bool RichEdit::GetZoom(int& nNum, int& nDen) const
{
    return m_richCtrl.GetZoom(nNum, nDen);
}

bool RichEdit::SetZoom(int nNum, int nDen)
{
    return m_richCtrl.SetZoom(nNum, nDen);
}

bool RichEdit::SetZoomOff()
{
    return m_richCtrl.SetZoomOff();
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
WORD RichEdit::GetSelectionType() const
{
    return m_richCtrl.GetSelectionType();
}

LONG RichEdit::FindRichText(DWORD dwFlags, FINDTEXTW& ft) const
{
    return m_richCtrl.FindTextW(dwFlags, ft);
}

LONG RichEdit::FindRichText(DWORD dwFlags, FINDTEXTEXW& ft) const
{
    return m_richCtrl.FindTextW(dwFlags, ft);
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
bool RichEdit::GetAutoURLDetect() const
{
    return m_richCtrl.GetAutoURLDetect();
}

bool RichEdit::SetAutoURLDetect(bool bAutoDetect)
{
    if (bAutoDetect) {
        ASSERT(m_richCtrl.GetEventMask() & ENM_LINK);
    }
    return m_richCtrl.SetAutoURLDetect(bAutoDetect ? TRUE : FALSE);
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
DWORD RichEdit::GetEventMask() const
{
    return m_richCtrl.GetEventMask();
}

DWORD RichEdit::SetEventMask(DWORD dwEventMask)
{
    return m_richCtrl.SetEventMask(dwEventMask);
}
#endif

DString RichEdit::GetTextRange(long nStartChar, long nEndChar) const
{
    TEXTRANGEW tr = { 0 };
    tr.chrg.cpMin = nStartChar;
    tr.chrg.cpMax = nEndChar;
    LPWSTR lpText = NULL;
    const int32_t nLen = nEndChar - nStartChar + 1;
    if (nLen < 1) {
        return DString();
    }
    lpText = new WCHAR[nLen];
    if (lpText == nullptr) {
        return DString();
    }
    ::ZeroMemory(lpText, nLen * sizeof(WCHAR));
    tr.lpstrText = lpText;
    m_richCtrl.GetTextRange(&tr);
    DStringW sText = (LPCWSTR)lpText;
    delete[] lpText;
#ifdef DUILIB_UNICODE
    return sText;
#else
    return StringUtil::UTF16ToT(sText);
#endif
}

void RichEdit::HideSelection(bool bHide, bool bChangeStyle)
{
    m_richCtrl.HideSelection(bHide, bChangeStyle);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

void RichEdit::ScrollCaret()
{
    m_richCtrl.ScrollCaret();
}

int RichEdit::InsertText(long nInsertAfterChar, const DString& text, bool bCanUndo)
{
#ifdef DUILIB_UNICODE
    return m_richCtrl.InsertText(nInsertAfterChar, text.c_str(), bCanUndo);
#else
    return m_richCtrl.InsertText(nInsertAfterChar, StringUtil::TToUTF16(text).c_str(), bCanUndo);
#endif
}

int RichEdit::AppendText(const DString& strText, bool bCanUndo)
{
#ifdef DUILIB_UNICODE
    return m_richCtrl.AppendText(strText.c_str(), bCanUndo);
#else
    return m_richCtrl.AppendText(StringUtil::TToUTF16(strText).c_str(), bCanUndo);
#endif
}

#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
DWORD RichEdit::GetDefaultCharFormat(CHARFORMAT2W& cf) const
{
    return m_richCtrl.GetDefaultCharFormat(cf);
}

bool RichEdit::SetDefaultCharFormat(CHARFORMAT2W& cf)
{
    if (m_richCtrl.SetDefaultCharFormat(cf)) {
        if (cf.dwMask & CFM_COLOR) {
            //同步文本颜色
            UiColor textColor;
            textColor.SetFromCOLORREF(cf.crTextColor);
            m_sTextColor = ui::StringUtil::Printf(_T("#%02X%02X%02X%02X"), textColor.GetA(), textColor.GetR(), textColor.GetG(), textColor.GetB());
        }
        return true;
    }
    return false;
}

DWORD RichEdit::GetSelectionCharFormat(CHARFORMAT2W& cf) const
{
    return m_richCtrl.GetSelectionCharFormat(cf);
}

bool RichEdit::SetSelectionCharFormat(CHARFORMAT2W& cf)
{
    return m_richCtrl.SetSelectionCharFormat(cf);
}

bool RichEdit::SetWordCharFormat(CHARFORMAT2W&cf)
{
    return m_richCtrl.SetWordCharFormat(cf);
}

DWORD RichEdit::GetParaFormat(PARAFORMAT2& pf) const
{
    return m_richCtrl.GetParaFormat(pf);
}

bool RichEdit::SetParaFormat(PARAFORMAT2 &pf)
{
    if (m_richCtrl.SetParaFormat(pf)) {
        return true;
    }
    return false;
}
#endif

bool RichEdit::CanRedo() const
{
    return m_richCtrl.CanRedo();
}

bool RichEdit::Redo()
{
    return m_richCtrl.Redo();
}

bool RichEdit::CanUndo() const
{
    return m_richCtrl.CanUndo();
}

bool RichEdit::Undo()
{
    return m_richCtrl.Undo();
}

void RichEdit::Clear()
{
    m_richCtrl.Clear();
}

void RichEdit::Copy()
{
    m_richCtrl.Copy();
}

void RichEdit::Cut()
{
    m_richCtrl.Cut();
}

void RichEdit::Paste()
{
    if (IsPasteLimited()) {
        return;
    }
    m_richCtrl.Paste();
}

bool RichEdit::CanPaste() const
{
    if (IsPasteLimited()) {
        return false;
    }
    return m_richCtrl.CanPaste(0);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
BOOL RichEdit::CanPaste(UINT nFormat/* = 0*/)
{
    if (nFormat == 0) {
        if (IsPasteLimited()) {
            return false;
        }
    }
    return m_richCtrl.CanPaste(nFormat);
}

void RichEdit::PasteSpecial(UINT uClipFormat, DWORD dwAspect/* = 0*/, HMETAFILE hMF/* = 0*/)
{
    if (IsPasteLimited()) {
        return;
    }
    return m_richCtrl.PasteSpecial(uClipFormat, dwAspect, hMF);
}
#endif

int RichEdit::GetLineCount() const
{
    return m_richCtrl.GetLineCount();
}

DString RichEdit::GetLine(int nIndex, int nMaxLength) const
{
    LPWSTR lpText = nullptr;
    if (nMaxLength < 1) {
        return DString();
    }
    lpText = new WCHAR[nMaxLength + 1];
    if (lpText == nullptr) {
        return DString();
    }
    ::ZeroMemory(lpText, (nMaxLength + 1) * sizeof(WCHAR));
    *(LPWORD)lpText = (WORD)nMaxLength;
    m_richCtrl.GetLine(nIndex, lpText);
    DStringW sText = lpText;
    delete[] lpText;
#ifdef DUILIB_UNICODE
    return sText;
#else
    return StringUtil::UTF16ToUTF8(sText);
#endif
}

int RichEdit::LineIndex(int nLine) const
{
    return m_richCtrl.LineIndex(nLine);
}

int RichEdit::LineLength(int nLine) const
{
    return m_richCtrl.LineLength(nLine);
}

bool RichEdit::LineScroll(int nLines)
{
    return m_richCtrl.LineScroll(nLines);
}

long RichEdit::LineFromChar(long nIndex) const
{
    return m_richCtrl.LineFromChar(nIndex);
}

UiPoint RichEdit::PosFromChar(long lChar) const
{ 
    POINT pt = m_richCtrl.PosFromChar(lChar);
    return UiPoint(pt.x, pt.y);
}

int RichEdit::CharFromPos(UiPoint pt) const
{
    POINT ptValue = { pt.x, pt.y };
    return m_richCtrl.CharFromPos(ptValue);
}

void RichEdit::EmptyUndoBuffer()
{
    m_richCtrl.EmptyUndoBuffer();
}

UINT RichEdit::SetUndoLimit(UINT nLimit)
{
    return m_richCtrl.SetUndoLimit(nLimit);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
long RichEdit::StreamIn(UINT nFormat, EDITSTREAM &es)
{
    return m_richCtrl.StreamIn(nFormat, es);
}

long RichEdit::StreamOut(UINT nFormat, EDITSTREAM &es)
{
    return m_richCtrl.StreamOut(nFormat, es);
}
#endif

void RichEdit::OnTxNotify(DWORD iNotify, void *pv)
{
    switch(iNotify)
    { 
    case EN_LINK:   
        {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
            NMHDR* hdr = (NMHDR*) pv;
            ENLINK* link = (ENLINK*)hdr;

            if((link != nullptr) && (link->msg == WM_LBUTTONUP)) {
                CHARRANGE oldSel = {0, 0};
                GetSel(oldSel);
                SetSel(link->chrg);
                DString url = GetSelText();
                const DString prefix = _T("HYPERLINK ");
                size_t pos = url.find(prefix);
                if (pos == 0) {
                    url = url.substr(prefix.size());
                    if (!url.empty() && url.front() == _T('\"')) {
                        url.erase(url.begin());
                        pos = url.find(_T('\"'));
                        if (pos != DString::npos) {
                            url.resize(pos);
                        }
                    }
                }
                SetSel(oldSel);
                if (!url.empty()) {
                    this->SendEvent(kEventLinkClick, (WPARAM)url.c_str());
                }
            }
#endif
        }
        break;
    case EN_CHANGE:
        //文本内容变化，发送事件
        OnTextChanged();            
        break;
    case EN_SELCHANGE:
        //选择变化
        SendEvent(kEventSelChange);
        break;
    case EN_DROPFILES:   
    case EN_MSGFILTER:   
    case EN_OLEOPFAILED:    
    case EN_PROTECTED:
    case EN_SAVECLIPBOARD:     
    case EN_STOPNOUNDO:   
    case EN_OBJECTPOSITIONS:   
    case EN_DRAGDROPDONE:   
        {
            if (pv) {   // Fill out NMHDR portion of pv   
                LONG nId =  ::GetWindowLong(GetWindowHandle(), GWL_ID);   
                NMHDR  *phdr = (NMHDR *)pv;   
                phdr->hwndFrom = GetWindowHandle();   
                phdr->idFrom = nId;   
                phdr->code = iNotify;  

                ::SendMessage(GetWindowHandle(), WM_NOTIFY, (WPARAM)nId, (LPARAM)pv);
            }    
        }
        break;
    default:
        break;
    }
}

HWND RichEdit::GetWindowHandle() const
{
    auto window = GetWindow();
    return window ? window->NativeWnd()->GetHWND() : nullptr;
}

HDC RichEdit::GetDrawDC() const
{
    auto window = GetWindow();
    return window ? window->NativeWnd()->GetPaintDC() : nullptr;
}

UiSize RichEdit::GetNaturalSize(LONG width, LONG height)
{
    if (width < 0) {
        width = 0;
    }
    if (height < 0) {
        height = 0;
    }    
    UiSize sz(0,0);
    LONG lWidth = width;
    LONG lHeight = height;
    SIZEL szExtent = { -1, -1 };

    ITextServices* pTextServices = nullptr;
    if (m_pRichHost) {
        pTextServices = m_pRichHost->GetTextServices();
    }
    if (pTextServices != nullptr) {
        pTextServices->TxGetNaturalSize(DVASPECT_CONTENT,
                                        GetDrawDC(),
                                        NULL,
                                        NULL,
                                        TXTNS_FITTOCONTENT,
                                        &szExtent,
                                        &lWidth,
                                        &lHeight);
    }
    sz.cx = (int)lWidth;
    sz.cy = (int)lHeight;
    return sz;
}

void RichEdit::SetImmStatus(BOOL bOpen)
{
    HWND hwnd = GetWindowHandle();
    if (hwnd == nullptr) {
        return;
    }
    // 失去焦点时关闭输入法
    HIMC hImc = ::ImmGetContext(hwnd);
    // 失去焦点是会把关联的输入法去掉，导致无法无法输入中文
    //::ImmAssociateContext(hwnd, bOpen ? hImc : NULL);
    if (hImc != NULL) {
        if (::ImmGetOpenStatus(hImc)) {
            if (!bOpen) {
                ::ImmSetOpenStatus(hImc, FALSE);
            }
        }
        else {
            if (bOpen) {
                ::ImmSetOpenStatus(hImc, TRUE);
            }
        }
        ::ImmReleaseContext(hwnd, hImc);
    }
}

void RichEdit::SetTimer(UINT idTimer, UINT uTimeout)
{
    auto timeFlag = m_timeFlagMap.find(idTimer);
    if (timeFlag != m_timeFlagMap.end()) {
        timeFlag->second.Cancel();
    }

    auto callback = [this, idTimer]() {
        m_richCtrl.TxSendMessage(WM_TIMER, idTimer, 0);
    };
    GlobalManager::Instance().Timer().AddTimer(m_timeFlagMap[idTimer].GetWeakFlag(), callback, uTimeout);
}

void RichEdit::KillTimer(UINT idTimer)
{
    auto timeFlag = m_timeFlagMap.find(idTimer);
    if (timeFlag != m_timeFlagMap.end()) {
        timeFlag->second.Cancel();
        m_timeFlagMap.erase(timeFlag);
    }
}

bool RichEdit::ScreenToClient(UiPoint& pt)
{
    bool bRet = false;
    if (m_pRichHost != nullptr) {
        POINT point = {pt.x, pt.y};
        bRet = m_pRichHost->TxScreenToClient(&point);
        pt.x = point.x;
        pt.y = point.y;
    }
    if (!bRet) {
        bRet = __super::ScreenToClient(pt);
    }
    return bRet;
}

bool RichEdit::ClientToScreen(UiPoint& pt)
{
    bool bRet = false;
    if (m_pRichHost != nullptr) {
        POINT point = { pt.x, pt.y };
        bRet = m_pRichHost->TxClientToScreen(&point);
        pt.x = point.x;
        pt.y = point.y;
    }
    if (!bRet) {
        bRet = __super::ClientToScreen(pt);
    }
    return bRet;
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void RichEdit::SetScrollPos(UiSize64 szPos)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bRichText = IsRichText();
#else
    bool bRichText = false;
#endif
    int64_t cx = 0;
    int64_t cy = 0;
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid() ) {
        int64_t iLastScrollPos = pVScrollBar->GetScrollPos();
        pVScrollBar->SetScrollPos(szPos.cy);
        cy = pVScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if ((pHScrollBar != nullptr) && pHScrollBar->IsValid() ) {
        int64_t iLastScrollPos = pHScrollBar->GetScrollPos();
        pHScrollBar->SetScrollPos(szPos.cx);
        cx = pHScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( cy != 0 ) {
        int64_t iPos = 0;
        if (!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
            iPos = pVScrollBar->GetScrollPos();
        }
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, (pVScrollBar != nullptr) ? pVScrollBar->GetScrollPos() : 0);
        m_richCtrl.TxSendMessage(WM_VSCROLL, wParam, 0L);
        if(!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid() ) {
            if (cy > 0 && pVScrollBar->GetScrollPos() <= iPos) {
                pVScrollBar->SetScrollPos(iPos);
            }
        }
    }
    if( cx != 0 ) {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, (pHScrollBar != nullptr) ? pHScrollBar->GetScrollPos() : 0);
        m_richCtrl.TxSendMessage(WM_HSCROLL, wParam, 0L);
    }
}

void RichEdit::LineUp()
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L);
}

void RichEdit::LineDown()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bRichText = IsRichText();
#else
    bool bRichText = false;
#endif

    int64_t iPos = 0;
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
        iPos = pVScrollBar->GetScrollPos();
    }
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_LINEDOWN, 0L);
    if(!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid() ) {
        if (pVScrollBar->GetScrollPos() <= iPos) {
            pVScrollBar->SetScrollPos(pVScrollBar->GetScrollRange());
        }
    }
}

void RichEdit::PageUp()
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_PAGEUP, 0L);
}

void RichEdit::PageDown()
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L);
}

void RichEdit::HomeUp()
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_TOP, 0L);
}

void RichEdit::EndDown()
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L);
}

void RichEdit::LineLeft()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L);
}

void RichEdit::LineRight()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_LINERIGHT, 0L);
}

void RichEdit::PageLeft()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_PAGELEFT, 0L);
}

void RichEdit::PageRight()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_PAGERIGHT, 0L);
}

void RichEdit::HomeLeft()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_LEFT, 0L);
}

void RichEdit::EndRight()
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_RIGHT, 0L);
}

DString RichEdit::GetType() const { return DUI_CTR_RICHEDIT; }

void RichEdit::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();

    //设置字体和字体颜色
    DString fontId = GetFontId();
    if (fontId.empty()) {
        fontId = GlobalManager::Instance().Font().GetDefaultFontId();
        SetFontIdInternal(fontId);
    }

    if (IsEnabled()) {
        UiColor dwTextColor = GetUiColor(GetTextColor());
        SetTextColorInternal(dwTextColor);
    }
    else {
        UiColor dwTextColor = GetUiColor(GetDisabledTextColor());
        SetTextColorInternal(dwTextColor);
    }

    ASSERT(m_pRichHost != nullptr);
    ScrollBar* pHScrollBar = GetHScrollBar();
    if (pHScrollBar != nullptr) {
        pHScrollBar->SetScrollRange(0);
    }
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollRange(0);
    }
}

void RichEdit::SetEnabled(bool bEnable /*= true*/)
{
    __super::SetEnabled(bEnable);
    if (IsEnabled()) {
        SetState(kControlStateNormal);
        UiColor dwTextColor = GetUiColor(GetTextColor());
        SetTextColorInternal(dwTextColor);
    }
    else {
        SetState(kControlStateDisabled);
        UiColor dwTextColor = GetUiColor(GetDisabledTextColor());
        SetTextColorInternal(dwTextColor);

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        //不可用的状态关闭拖放功能
        SetEnableDragDrop(false);
#endif
    }
}

UiEstSize RichEdit::EstimateSize(UiSize /*szAvailable*/)
{
    UiFixedSize fixexSize = GetFixedSize();
    UiSize size(fixexSize.cx.GetInt32(), fixexSize.cy.GetInt32());
    if (fixexSize.cx.IsAuto() || fixexSize.cy.IsAuto()) {
        UiSize szNaturalSize = GetNaturalSize(size.cx, size.cy);
        //返回大小需要包含内边距
        UiPadding rcPadding = GetControlPadding();
        UiPadding rcTextPadding = GetTextPadding();
        if (fixexSize.cy.IsAuto()) {
            size.cy = szNaturalSize.cy + (rcPadding.top + rcPadding.bottom) + (rcTextPadding.top + rcTextPadding.bottom);
            fixexSize.cy.SetInt32(size.cy);
        }
        if (fixexSize.cx.IsAuto()) {
            size.cx = szNaturalSize.cx + (rcPadding.left + rcPadding.right) + (rcTextPadding.left + rcTextPadding.right);
            fixexSize.cx.SetInt32(size.cx);
        }
    }
    return MakeEstSize(fixexSize);
}

UiSize RichEdit::EstimateText(UiSize szAvailable)
{
    UiPadding rcPadding = GetControlPadding();
    UiPadding rcTextPadding = GetTextPadding();
    szAvailable.cx -= (rcPadding.left + rcPadding.right);
    szAvailable.cy -= (rcPadding.top + rcPadding.bottom);
    szAvailable.cx -= (rcTextPadding.left + rcTextPadding.right);
    szAvailable.cy -= (rcTextPadding.top + rcTextPadding.bottom);

    szAvailable.Validate();
    LONG iWidth = szAvailable.cx;
    if (iWidth < 0) {
        iWidth = 0;
    }
    LONG iHeight = 0;
    UiSize szNaturalSize = GetNaturalSize(iWidth, iHeight);
    iWidth = szNaturalSize.cx;
    iHeight = szNaturalSize.cy;

    iWidth = std::max((int32_t)iWidth, 0);
    iHeight = std::max((int32_t)iHeight, 0);
    iWidth += (rcPadding.left + rcPadding.right);
    iHeight += (rcPadding.top + rcPadding.bottom);

    iWidth += (rcTextPadding.left + rcTextPadding.right);
    iHeight += (rcTextPadding.top + rcTextPadding.bottom);

    szAvailable.cx = std::max((int32_t)iWidth, 0);
    szAvailable.cy = std::max((int32_t)iHeight, 0);
    return szAvailable;
}

void RichEdit::SetPos(UiRect rc)
{
    Control::SetPos(rc);
    rc = GetRectWithoutPadding();
    bool bVScrollBarVisible = false;
    ScrollBar* pVScrollBar = GetVScrollBar();
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
        bVScrollBarVisible = true;
        rc.right -= pVScrollBar->GetFixedWidth().GetInt32();
    }
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pHScrollBar != nullptr) && pHScrollBar->IsValid()) {
        rc.bottom -= pHScrollBar->GetFixedHeight().GetInt32();
    }

    if (m_pRichHost != nullptr) {
        //调整编辑框的位置, 剪去文本内边距
        UiRect textRect = rc;
        UiPadding rcTextPadding = GetTextPadding();
        textRect.Deflate(rcTextPadding);
        m_pRichHost->SetClientRect(textRect);
        if (bVScrollBarVisible && (pVScrollBar != nullptr) && (!pVScrollBar->IsValid() || m_bVScrollBarFixing)) {
            LONG lWidth = rc.Width() + pVScrollBar->GetFixedWidth().GetInt32();
            LONG lHeight = 0;
            UiSize szNaturalSize = GetNaturalSize(lWidth, lHeight);
            lWidth = szNaturalSize.cx;
            lHeight = szNaturalSize.cy;
            if (lHeight > rc.Height()) {
                pVScrollBar->SetScrollPos(0);
                m_bVScrollBarFixing = true;
            }
            else {
                if (m_bVScrollBarFixing) {
                    pVScrollBar->SetScrollRange(0);
                    m_bVScrollBarFixing = false;
                }
            }
        }
    }

    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
        UiRect rcScrollBarPos(rc.right, rc.top, rc.right + pVScrollBar->GetFixedWidth().GetInt32(), rc.bottom);
        pVScrollBar->SetPos(rcScrollBarPos);
    }
    if (pHScrollBar != NULL && pHScrollBar->IsValid()) {
        UiRect rcScrollBarPos(rc.left, rc.bottom, rc.right, rc.bottom + pHScrollBar->GetFixedHeight().GetInt32());
        pHScrollBar->SetPos(rcScrollBarPos);
    }

    //排列子控件
    ArrangeChild(m_items);
}

void RichEdit::ArrangeChild(const std::vector<Control*>& items) const
{
    //使用默认布局的排布方式
    GetLayout()->ArrangeChild(items, GetPos());
}

uint32_t RichEdit::GetControlFlags() const
{
    return IsEnabled() && IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void RichEdit::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            __super::HandleEvent(msg);
        }
        return;
    }
    if (msg.eventType == kEventMouseMove) {
        OnMouseMessage(WM_MOUSEMOVE, msg);
        return;
    }
    if (msg.eventType == kEventMouseWheel) {
        if (IsKeyDown(msg, ModifierKey::kControl) && IsEnableWheelZoom()) {
            //Ctrl + 滚轮：缩放功能
            OnMouseMessage(WM_MOUSEWHEEL, msg);
            int32_t nNum = 0;
            int32_t nDen = 0;
            m_richCtrl.GetZoom(nNum, nDen);
            SendEvent(kEventZoom, (WPARAM)nNum, (LPARAM)nDen);
        }
        else {
            ScrollBox::HandleEvent(msg);
        }
        return;
    }

    if (msg.eventType == kEventMouseButtonDown) {

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        if (m_linkInfo.size() > 0)    {
            DString url;
            if (HittestCustomLink(UiPoint(msg.ptMouse), url)) {
                SendEvent(kEventLinkClick, (WPARAM)url.c_str());
                return;
            }
        }
#endif

        OnMouseMessage(WM_LBUTTONDOWN, msg);
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        if (IsEnabled() && !m_bSelAllEver) {
            m_bSelAllEver = true;
            if (m_bSelAllOnFocus) {
                SetSelAll();
                if (GetMultiLine()) {
                    HomeUp();
                }
                else {
                    HomeLeft();
                }
            }
        }

        OnMouseMessage(WM_LBUTTONUP, msg);
        return;
    }
    if (msg.eventType == kEventMouseDoubleClick) {
        if (IsReadOnly()) {
            SetSelAll();
            return;
        }

        OnMouseMessage(WM_LBUTTONDBLCLK, msg);
        return;
    }
    if (msg.eventType == kEventMouseRButtonDown) {
        OnMouseMessage(WM_RBUTTONDOWN, msg);
        return;
    }
    if (msg.eventType == kEventMouseRButtonUp) {
        OnMouseMessage(WM_RBUTTONUP, msg);
        return;
    }
    else if (msg.eventType == kEventKeyDown) {
        OnKeyDown(msg);
        return;
    }
    ScrollBox::HandleEvent(msg);
}

bool RichEdit::OnSetCursor(const EventArgs& msg)
{
    if (m_bUseControlCursor) {
        //使用Control设置的光标
        return __super::OnSetCursor(msg);
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    DString strLink;
    if (HittestCustomLink(UiPoint(msg.ptMouse), strLink)) {
        SetCursor(CursorType::kCursorHand);
        return true;
    }
#endif

    if (m_pRichHost && m_pRichHost->SetCursor(nullptr, &msg.ptMouse)) {
        return true;
    }
    else {
        SetCursor(IsReadOnly() ? CursorType::kCursorArrow : CursorType::kCursorIBeam);
        return true;
    }
}

bool RichEdit::OnSetFocus(const EventArgs& /*msg*/)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->OnTxInPlaceActivate(NULL);
        m_richCtrl.TxSendMessage(WM_SETFOCUS, 0, 0);
        ShowCaret(true);
    }
    SetImmStatus(TRUE);

#ifdef DUILIB_BUILD_FOR_SDL
    if (IsVisible() && !IsReadOnly() && IsEnabled()) {
        SDL_StartTextInput((SDL_Window*)GetWindow()->NativeWnd()->GetWindowHandle());
    }
#endif

    if ((m_pClearButton != nullptr) && !IsReadOnly()){
        m_pClearButton->SetFadeVisible(true);
    }
    if ((m_pShowPasswordButton != nullptr) && IsPassword() && !IsShowPassword()) {
        m_pShowPasswordButton->SetFadeVisible(true);
    }
    Invalidate();
    return true;
}

bool RichEdit::OnKillFocus(const EventArgs& /*msg*/)
{
    if (m_pRichHost) {
        m_pRichHost->OnTxInPlaceActivate(NULL);
        m_richCtrl.TxSendMessage(WM_KILLFOCUS, 0, 0);
        ShowCaret(false);
    }

    m_bSelAllEver = false;
    if (m_bNoSelOnKillFocus && IsReadOnly() && IsEnabled()) {
        SetSelNone();
    }
    if (m_bSelAllOnFocus && IsEnabled()) {
        SetSelNone();
    }

    SetImmStatus(FALSE);

#ifdef DUILIB_BUILD_FOR_SDL
    if (IsVisible() && !IsReadOnly() && IsEnabled()) {
        SDL_StopTextInput((SDL_Window*)GetWindow()->NativeWnd()->GetWindowHandle());
    }
#endif

    if (m_pClearButton != nullptr) {
        m_pClearButton->SetFadeVisible(false);
    }
    if (m_pShowPasswordButton != nullptr) {
        m_pShowPasswordButton->SetFadeVisible(false);
    }
    Invalidate();
    return true;
}

bool RichEdit::OnChar(const EventArgs& msg)
{
    //TAB
    if (::GetKeyState(VK_TAB) < 0 && !m_bWantTab) {
        SendEvent(kEventTab);
        return true;
    }
    //Number
    if (IsNumberOnly()) {
        if (msg.vkCode < '0' || msg.vkCode > '9') {
            if (msg.vkCode == _T('-')) {
                if (GetTextLength() > 0) {
                    //不是第一个字符，禁止输入负号
                    return true;
                }
                else if (GetMinNumber() >= 0) {
                    //最小数字是0或者正数，禁止输入符号
                    return true;
                }
            }
            else {
                return true;
            }
        }
    }

    //限制允许输入的字符
    if (m_pLimitChars != nullptr) {
        if (!IsInLimitChars((wchar_t)msg.vkCode)) {
            //字符不在列表里面，禁止输入
            return true;
        }
    }
#ifdef DUILIB_UNICODE
    WPARAM wParam = msg.wParam;
    WPARAM lParam = msg.lParam;
#ifdef DUILIB_BUILD_FOR_SDL
    wParam = msg.vkCode;
    lParam = 0;
#endif
    m_richCtrl.TxSendMessage(WM_CHAR, wParam, lParam);
#else
    //只支持1字节和2字节的文字输入，不支持4字节的文字输入
    if ((::GetTickCount() - m_dwLastCharTime) > 5000) {
        m_pendingChars.clear();
    }
    bool bHandled = false;
    if (m_pendingChars.empty()) {
        if (IsDBCSLeadByte((BYTE)msg.wParam)) {
            m_pendingChars.push_back((BYTE)msg.wParam);
            bHandled = true;
        }
    }
    else {
        if (m_pendingChars.size() == 1) {
            BYTE chMBCS[8] = {m_pendingChars.front(), (BYTE)msg.wParam, 0, };
            wchar_t chWideChar[4] = {0, };
            ::MultiByteToWideChar(CP_ACP, 0, (const char*)chMBCS, 2, chWideChar, 4);
            if (chWideChar[0] != 0) {
                WPARAM wParam = chWideChar[0];
                m_richCtrl.TxSendMessage(WM_CHAR, wParam, msg.lParam);                
                bHandled = true;
            }
            m_pendingChars.clear();
        }
        else {
            ASSERT(false);
            m_pendingChars.clear();
        }
    }
    m_dwLastCharTime = ::GetTickCount();
    if (!bHandled) {
        m_richCtrl.TxSendMessage(WM_CHAR, msg.wParam, msg.lParam);
    }    
#endif    
    return true;
}

bool RichEdit::IsInLimitChars(wchar_t charValue) const
{
    //返回false时：禁止输入
    if (m_pLimitChars == nullptr) {
        return true;
    }
    const wchar_t* ch = m_pLimitChars.get();
    if ((ch == nullptr) || (*ch == L'\0')) {
        return true;
    }
    bool bInLimitChars = false;
    while (*ch != L'\0') {
        if (*ch == charValue) {
            bInLimitChars = true;
            break;
        }
        ++ch;
    }
    return bInLimitChars;
}

bool RichEdit::IsPasteLimited() const
{
    if (m_pLimitChars != nullptr) {
        //有设置限制字符
        DStringW strClipText;
        GetClipboardText(strClipText);
        if (!strClipText.empty()) {
            size_t count = strClipText.size();
            for (size_t index = 0; index < count; ++index) {
                if (strClipText[index] == L'\0') {
                    break;
                }
                if (!IsInLimitChars(strClipText[index])) {
                    //有字符不在列表里面，禁止粘贴
                    return true;
                }
            }
        }
    }
    else if (IsNumberOnly()) {
        //数字模式
        DStringW strClipText;
        GetClipboardText(strClipText);
        if (!strClipText.empty()) {
            size_t count = strClipText.size();
            for (size_t index = 0; index < count; ++index) {
                if (strClipText[index] == L'\0') {
                    break;
                }
                if (strClipText[index] == L'-') {
                    if ((index == 0) && (strClipText.size() > 1)) {
                        //允许第一个字符是负号
                        continue;
                    }
                }
                if ((strClipText[index] > L'9') || (strClipText[index] < L'0')) {
                    //有不是数字的字符，禁止粘贴
                    return true;
                }
            }
        }
    }
    return false;
}

bool RichEdit::OnKeyDown(const EventArgs& msg)
{
    if (msg.vkCode == kVK_RETURN && ::GetKeyState(VK_SHIFT) >= 0)    {
        if (m_bNeedReturnMsg && ((m_bReturnMsgWantCtrl && ::GetKeyState(VK_CONTROL) < 0) ||
            (!m_bReturnMsgWantCtrl && ::GetKeyState(VK_CONTROL) >= 0))) {
            SendEvent(kEventReturn);
            return true;
        }
    }
    else if ((msg.vkCode == 'V') && (::GetKeyState(VK_CONTROL) < 0)) {
        if (IsPasteLimited()) {
            return true;
        }
    }

    WPARAM wParam = msg.wParam;
    LPARAM lParam = msg.lParam;
#ifdef DUILIB_BUILD_FOR_SDL
    wParam = msg.vkCode;
    lParam = 0;
#endif

    m_richCtrl.TxSendMessage(WM_KEYDOWN, wParam, lParam);
    return true;
}

bool RichEdit::OnImeStartComposition(const EventArgs& /*msg*/)
{
    HWND hWnd = GetWindowHandle();
    if (hWnd == nullptr) {
        return true;
    }

    HIMC hImc = ::ImmGetContext(hWnd);
    if (hImc == nullptr) {
        return true;
    }

    COMPOSITIONFORM    cfs = { 0, };
    UiPoint ptScrollOffset = GetScrollOffsetInScrollBox();
    POINT pt;
    pt.x = m_iCaretPosX - ptScrollOffset.x;
    pt.y = m_iCaretPosY - ptScrollOffset.y;

    //pt.y += (m_iCaretHeight + lf.lfHeight) / 4;
    cfs.dwStyle = CFS_POINT;
    if (pt.x < 1) {
        pt.x = 1;
    }
    if (pt.y < 1) {
        pt.y = 1;
    }
    cfs.ptCurrentPos = pt;
    ::ImmSetCompositionWindow(hImc, &cfs);
    ::ImmReleaseContext(hWnd, hImc);
    m_bIsComposition = true;
    return true;
}

bool RichEdit::OnImeEndComposition(const EventArgs& /*msg*/)
{
    m_bIsComposition = false;
    return true;
}

void RichEdit::OnMouseMessage(UINT uMsg, const EventArgs& msg)
{
    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    WPARAM wParam = msg.wParam;
#ifdef DUILIB_BUILD_FOR_SDL
    wParam = 0;
    if (IsKeyDown(msg, ModifierKey::kControl)) {
        wParam |= MK_CONTROL;
    }
    if (IsKeyDown(msg, ModifierKey::kShift)) {
        wParam |= MK_SHIFT;
    }
#endif
    m_richCtrl.TxSendMessage(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
}

void RichEdit::Paint(IRender* pRender, const UiRect& rcPaint)
{
    if (pRender == nullptr) {
        return;
    }
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);

    bool bNeedPaint = true;
    if (pRender->IsClipEmpty()) {
        bNeedPaint = false;
    }    
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        bNeedPaint = false;
    }

    if (bNeedPaint) {
        Control::Paint(pRender, rcPaint);
    }

    ITextServices* pTextServices = nullptr;
    if (m_pRichHost != nullptr) {
        pTextServices = m_pRichHost->GetTextServices();
    }
    if ((pTextServices == nullptr) || (m_pRichHost == nullptr)) {
        return;
    }

    UiRect rc;
    m_pRichHost->GetControlRect(&rc);

    bool bNormalPrint = true;
    if (bNeedPaint && bNormalPrint) {
        // Remember wparam is actually the hdc and lparam is the update
        // rect because this message has been preprocessed by the window.
#if defined (DUILIB_BUILD_FOR_SDL)
        HDC hdc = nullptr;
#elif defined (DUILIB_BUILD_FOR_WIN) 
        HDC hdc = pRender->GetRenderDC(GetWindow()->NativeWnd()->GetHWND());
#else
        HDC hdc = nullptr;
#endif
        if(hdc != nullptr){
            RECT paintRect = { rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom };
            pTextServices->TxDraw(DVASPECT_CONTENT,         // Draw Aspect
                                  /*-1*/0,                  // Lindex
                                  NULL,                     // Info for drawing optimazation
                                  NULL,                     // target device information
                                  hdc,                      // Draw device HDC
                                  NULL,                     // Target device HDC
                                  (RECTL*)&rc,              // Bounding client rectangle
                                  NULL,                     // Clipping rectangle for metafiles
                                  &paintRect,               // Update rectangle
                                  NULL,                     // Call back function
                                  NULL,                     // Call back parameter
                                  0);                       // What view of the object

            pRender->ReleaseRenderDC(hdc);
            //绘制完成后，做标记，避免重复绘制
            bNeedPaint = false;
        }
    }
    if (bNeedPaint) {
        PaintRichEdit(pRender, rcPaint);
    }

    ScrollBar* pVScrollBar = GetVScrollBar();
    if (m_bVScrollBarFixing && (pVScrollBar != nullptr)) {
        LONG lWidth = rc.Width() + pVScrollBar->GetFixedWidth().GetInt32();
        UiSize szNaturalSize = GetNaturalSize(lWidth, 0);
        if(szNaturalSize.cy <= rc.Height() ) {
            Arrange();
        }
    }
}

RichEdit::TxDrawData::TxDrawData():
    m_hDrawDC(nullptr),
    m_hOldBitmap(nullptr),
    m_hBitmap(nullptr),
    m_pBitmapBits(nullptr)
{

}

RichEdit::TxDrawData::~TxDrawData()
{
    Clear();
}

void RichEdit::TxDrawData::Clear()
{
    if ((m_hDrawDC != nullptr) && (m_hOldBitmap != nullptr)) {
        ::SelectObject(m_hDrawDC, m_hOldBitmap);
    }
    m_hOldBitmap = nullptr;
    if (m_hDrawDC != nullptr) {
        ::DeleteDC(m_hDrawDC);
        m_hDrawDC = nullptr;
    }
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
    m_pBitmapBits = nullptr;
    m_szBitmap.Clear();
}

bool RichEdit::TxDrawData::CheckCreateBitmap(HDC hWindowDC, int32_t nWidth, int32_t nHeight)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return false;
    }
    if ((m_szBitmap.cx != nWidth) || (m_szBitmap.cy != nHeight)) {
        if (m_hBitmap != nullptr) {
            ::DeleteObject(m_hBitmap);
            m_hBitmap = nullptr;
        }
        m_pBitmapBits = nullptr;
    }
    if (m_hBitmap == nullptr) {
        m_pBitmapBits = nullptr;
        m_hBitmap = BitmapHelper::CreateGDIBitmap(nWidth, nHeight, true, &m_pBitmapBits);
        if ((m_hBitmap != nullptr) && (m_pBitmapBits != nullptr)) {
            m_szBitmap.cx = nWidth;
            m_szBitmap.cy = nHeight;

            if (m_hDrawDC == nullptr) {
                m_hDrawDC = ::CreateCompatibleDC(hWindowDC);
                ASSERT(m_hDrawDC != nullptr);
                if (m_hDrawDC != nullptr) {
                    m_hOldBitmap = ::SelectObject(m_hDrawDC, m_hBitmap);
                }
            }
            else {
                ::SelectObject(m_hDrawDC, m_hBitmap);
            }
        }
        else {
            if (m_hBitmap != nullptr) {
                ::DeleteObject(m_hBitmap);
                m_hBitmap = nullptr;
            }
            m_pBitmapBits = nullptr;
        }        
    }
    return (m_hBitmap != nullptr) && (m_pBitmapBits != nullptr) && (m_hDrawDC != nullptr);
}

void RichEdit::PaintRichEdit(IRender* pRender, const UiRect& rcPaint)
{
    if (pRender == nullptr) {
        return;
    }

    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    ITextServices* pTextServices = nullptr;
    if (m_pRichHost != nullptr) {
        pTextServices = m_pRichHost->GetTextServices();
    }
    if ((pTextServices == nullptr) || (m_pRichHost == nullptr)) {
        return;
    }

    const UiRect rc = m_pRichHost->GetControlRect();
    if (rc.IsEmpty()) {
        return;
    }

    std::vector<UiRect> clipRects;
    RenderClipType clipType = pRender->GetClipInfo(clipRects);
    if (clipType == RenderClipType::kEmpty) {
        //如果裁剪区域为空，不需要绘制
        return;
    }

    //获取与本控件的绘制区域交集
    const UiRect& rcDirty = GetPaintRect();
    ASSERT(!rcDirty.IsEmpty());
    if (rcDirty.IsEmpty()) {
        return;
    }

    UiRect rcUpdate = rc;
    rcUpdate.Intersect(rcDirty);
    if (rcUpdate.IsEmpty()) {
        return;
    }

    if (!clipRects.empty()) {
        bool bHasIntersect = false;
        UiRect rcCheck = rcUpdate;
        for (const UiRect& clipRect : clipRects) {
            if (rcCheck.Intersect(clipRect)) {
                bHasIntersect = true;
                break;
            }
        }
        if (!bHasIntersect) {
            //脏区域矩形与裁剪区域矩形无交集，无需绘制
            return;
        }
    }

    //创建绘制所需DC和位图
    bool bRet = m_txDrawData.CheckCreateBitmap(GetDrawDC(), rc.Width(), rc.Height());
    ASSERT(bRet);
    if (!bRet) {
        return;
    }

    HDC hDrawDC = m_txDrawData.m_hDrawDC;
    LPVOID pBitmapBits = m_txDrawData.m_pBitmapBits;
    ASSERT((pBitmapBits != nullptr) && (hDrawDC != nullptr));
    if ((pBitmapBits == nullptr) || (hDrawDC == nullptr)) {
        return;
    }

    //复制渲染引擎源位图数据
    bRet = pRender->ReadPixels(rc, pBitmapBits, rc.Width() * rc.Height() * sizeof(uint32_t));
    ASSERT(bRet);
    if (!bRet) {
        return;
    }

    //更新区域（相对于位图左上角坐标）
    rcUpdate.Offset(-rc.left, -rc.top);

    //清除Alpha通道（标记为全部透明）
    const int32_t nTop = std::max(rcUpdate.top, 0);
    const int32_t nBottom = std::min(rcUpdate.bottom, rc.Height());
    const int32_t nLeft = std::max(rcUpdate.left, 0);
    const int32_t nRight = std::min(rcUpdate.right, rc.Width());
    const int32_t nWidth = rc.Width();
    for (int32_t i = nTop; i < nBottom; i++) {
        for (int32_t j = nLeft; j < nRight; j++) {
            uint8_t* a = (uint8_t*)pBitmapBits + (i * nWidth + j) * sizeof(uint32_t) + 3;
            *a = 0;
        }
    }

    //位图的矩形区域
    RECTL rcBitmap = { 0, };
    rcBitmap.left = 0;
    rcBitmap.top = 0;
    rcBitmap.right = rcBitmap.left + rc.Width();
    rcBitmap.bottom = rcBitmap.top + rc.Height();

    //设置裁剪信息，避免绘制非更新区域
    bool bSetClipRect = false;
    if (!clipRects.empty()) {
        size_t nCount = clipRects.size() + 1;
        size_t nSize = sizeof(RGNDATAHEADER) + nCount * sizeof(RECT);
        RGNDATA* rgnData = (RGNDATA*)::malloc(nSize);
        ASSERT(rgnData != nullptr);
        if (rgnData != nullptr) {
            memset(rgnData, 0, nSize);
            rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
            rgnData->rdh.iType = RDH_RECTANGLES;
            rgnData->rdh.nCount = (DWORD)nCount;
            rgnData->rdh.rcBound.left = 0;
            rgnData->rdh.rcBound.top = 0;
            rgnData->rdh.rcBound.right = rc.Width();
            rgnData->rdh.rcBound.bottom = rc.Height();

            nCount = 0;
            LPRECT pRc = (LPRECT)rgnData->Buffer;
            for (UiRect clipRect : clipRects) {
                clipRect.Offset(-rc.left, -rc.top);
                RECT rcClip = { clipRect.left, clipRect.top, clipRect.right, clipRect.bottom };
                pRc[nCount++] = rcClip;
            }

            RECT rcClip = { rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom };
            pRc[nCount++] = rcClip;

            HRGN hRgn = ::ExtCreateRegion(NULL, (DWORD)nSize, rgnData);
            ::free(rgnData);
            if (hRgn != nullptr) {
                bSetClipRect = true;
                ::SelectClipRgn(hDrawDC, hRgn);
                ::DeleteObject(hRgn);
            }
        }
    }
    if (!bSetClipRect) {
        ::IntersectClipRect(hDrawDC, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom);
    }
    
    RECT rectUpdate = { rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom };
    pTextServices->TxDraw(DVASPECT_CONTENT,      // Draw Aspect
                            /*-1*/0,               // Lindex
                            NULL,                  // Info for drawing optimazation
                            NULL,                  // target device information
                            hDrawDC,               // Draw device HDC
                            NULL,                  // Target device HDC
                            &rcBitmap,             // Bounding client rectangle
                            NULL,                  // Clipping rectangle for metafiles
                            &rectUpdate,           // Update rectangle
                            NULL,                  // Call back function
                            NULL,                  // Call back parameter
                            0);                    // What view of the object

    //恢复Alpha(绘制过程中，会导致绘制区域部分的Alpha通道出现异常)
    for (int32_t i = nTop; i < nBottom; i++) {
        for (int32_t j = nLeft; j < nRight; j++) {
            uint8_t* a = (uint8_t*)pBitmapBits + (i * nWidth + j) * sizeof(uint32_t)+ 3;
            if (*a == 0) {
                *a = 255;
            }
        }
    }

    //将绘制完成的数据，回写到渲染引擎位图
    rcUpdate.Offset(rc.left, rc.top);
    bRet = pRender->WritePixels(pBitmapBits, rc.Width() * rc.Height() * sizeof(uint32_t), rc, rcUpdate);
    ASSERT(bRet);
}

void RichEdit::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    PaintCaret(pRender, rcPaint);

    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if (m_items.size() > 0) {
        UiRect rc = GetRectWithoutPadding();
        if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
            rc.right -= pVScrollBar->GetFixedWidth().GetInt32();
        }
        if ((pHScrollBar != nullptr) && pHScrollBar->IsValid()) {
            rc.bottom -= pHScrollBar->GetFixedHeight().GetInt32();
        }

        if (!UiRect::Intersect(rcTemp, rcPaint, rc)) {
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
        else {
            AutoClip childClip(pRender, rcTemp);
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
                else {
                    if (!UiRect::Intersect(rcTemp, rc, controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
    }

    if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible()) {
        UiRect verBarPos = pVScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, verBarPos)) {
            pVScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }

    if ((pHScrollBar != nullptr) && pHScrollBar->IsVisible()) {
        UiRect horBarPos = pHScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, horBarPos)) {
            pHScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }
}

BOOL RichEdit::CreateCaret(INT xWidth, INT yHeight)
{
    m_iCaretWidth = xWidth;
    m_iCaretHeight = yHeight;
    return true;
}

BOOL RichEdit::ShowCaret(BOOL fShow)
{
    if (fShow) {
        m_bIsCaretVisiable = true;
        m_drawCaretFlag.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::ChangeCaretVisiable, this);
        GlobalManager::Instance().Timer().AddTimer(m_drawCaretFlag.GetWeakFlag(), closure, 500);
    }
    else {
        m_bIsCaretVisiable = false;
        m_drawCaretFlag.Cancel();
    }

    Invalidate();
    return true;
}

void RichEdit::SetCaretColor(const DString& dwColor)
{
    m_sCaretColor = dwColor;
}

DString RichEdit::GetCaretColor()
{
    return m_sCaretColor.c_str();
}

RECT RichEdit::GetCaretRect()
{
    RECT rc = { m_iCaretPosX, m_iCaretPosY, m_iCaretPosX + m_iCaretWidth, m_iCaretPosY + m_iCaretHeight };
    return rc;
}

BOOL RichEdit::SetCaretPos(INT x, INT y)
{
    m_iCaretPosX = x;
    m_iCaretPosY = y;
    ShowCaret(!m_richCtrl.HasSelText());
    return true;
}

void RichEdit::ChangeCaretVisiable()
{
    m_bIsCaretVisiable = !m_bIsCaretVisiable;
    Invalidate();
}

void RichEdit::PaintCaret(IRender* pRender, const UiRect& /*rcPaint*/)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (IsReadOnly() && m_bNoCaretReadonly) {
        return;
    }

    if (m_bIsCaretVisiable && !m_bIsComposition) {
        UiRect rect(m_iCaretPosX, m_iCaretPosY, m_iCaretPosX, m_iCaretPosY + m_iCaretHeight);
        UiColor dwClrColor(0xff000000);
        if (!m_sCaretColor.empty()) {
            dwClrColor = this->GetUiColor(m_sCaretColor.c_str());
        }
        pRender->DrawLine(UiPoint(rect.left, rect.top), UiPoint(rect.right, rect.bottom), dwClrColor, m_iCaretWidth);
    }
}

void RichEdit::SetPromptMode(bool bPrompt)
{
    if(bPrompt == m_bAllowPrompt)
        return;
    m_bAllowPrompt = bPrompt;
    Invalidate();
}

DString RichEdit::GetPromptText() const
{
    DString strText = m_sPromptText.c_str();
    if (strText.empty() && !m_sPromptTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringViaID(m_sPromptTextId.c_str());
    }

    return strText;
}

std::string RichEdit::GetUTF8PromptText() const
{
    std::string strOut = StringUtil::TToUTF8(GetPromptText());
    return strOut;
}

void RichEdit::SetPromptText(const DString& strText)
{
    if (m_sPromptText != strText) {
        m_sPromptText = strText;
        Invalidate();
    }
}

void RichEdit::SetUTF8PromptText(const std::string& strText)
{
    DString strOut = StringUtil::UTF8ToT(strText);
    SetPromptText(strOut);
}

void RichEdit::SetPromptTextId(const DString& strTextId)
{
    if (m_sPromptTextId == strTextId) {
        m_sPromptTextId = strTextId;
        Invalidate();
    }
}

void RichEdit::SetUTF8PromptTextId(const std::string& strTextId)
{
    DString strOut = StringUtil::UTF8ToT(strTextId);
    SetPromptTextId(strOut);
}

void RichEdit::PaintPromptText(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (GetTextLength() != 0) {
        return;
    }

    if (m_pRichHost == nullptr) {
        return;
    }

    DString strPrompt = GetPromptText();
    if (strPrompt.empty() || m_sPromptColor.empty()) {
        return;
    }

    UiRect rc;
    m_pRichHost->GetControlRect(&rc);
    UiColor dwClrColor = GetUiColor(m_sPromptColor.c_str());
    UINT dwStyle = TEXT_NOCLIP;
    pRender->DrawString(rc, strPrompt, dwClrColor, GetIFontById(m_sFontId.c_str()), dwStyle);
}

DString RichEdit::GetFocusedImage()
{
    if (m_pFocusedImage != nullptr) {
        return m_pFocusedImage->GetImageString();
    }
    return DString();
}

void RichEdit::SetFocusedImage( const DString& strImage )
{
    if (m_pFocusedImage == nullptr) {
        m_pFocusedImage = new Image;
    }
    m_pFocusedImage->SetImageString(strImage, Dpi());
    Invalidate();
}

void RichEdit::PaintStateImages(IRender* pRender)
{
    if (IsReadOnly()) {
        return;
    }

    if (IsFocused()) {
        if (m_pFocusedImage != nullptr) {
            PaintImage(pRender, m_pFocusedImage);
        }        
        PaintPromptText(pRender);
    }
    else {
        __super::PaintStateImages(pRender);
        PaintPromptText(pRender);
    }
}

void RichEdit::SetNoSelOnKillFocus(bool bNoSel)
{
    m_bNoSelOnKillFocus = bNoSel;
}

void RichEdit::SetSelAllOnFocus(bool bSelAll)
{
    m_bSelAllOnFocus = bSelAll;
}

void RichEdit::SetNoCaretReadonly()
{
    m_bNoCaretReadonly = true;
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
void RichEdit::SetSaveSelection(bool fSaveSelection)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetSaveSelection(fSaveSelection);
    }
}

void RichEdit::SetHideSelection(bool fHideSelection)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetHideSelection(fHideSelection);
    }
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

void RichEdit::AddColorText(const DString &str, const DString &color)
{
    if( !IsRichText() || str.empty() || color.empty() ) {
        ASSERT(FALSE);
        return;
    }
    UiColor dwColor = GetUiColor(color);

    CHARFORMAT2W cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(CHARFORMAT2W);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0;
    cf.crTextColor = dwColor.ToCOLORREF();

    ReplaceSel(str, FALSE);
    int len = GetTextLength();
    SetSel(len - (int)str.size(), len);
    SetSelectionCharFormat(cf);

    SetSelNone();
    GetDefaultCharFormat(cf);
    SetSelectionCharFormat(cf);
}

void RichEdit::AddLinkColorText(const DString &str, const DString &color, const DString &linkInfo)
{
    if( !IsRichText() || str.empty() || color.empty() ) {
        ASSERT(FALSE);
        return;
    }
    UiColor dwColor = GetUiColor(color);

    CHARFORMAT2W cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(CHARFORMAT2W);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = dwColor.ToCOLORREF();

    ReplaceSel(str, FALSE);
    int len = GetTextLength();
    SetSel(len - (int)str.size(), len);
    SetSelectionCharFormat(cf);
    LinkInfo info;
    info.info = linkInfo;
    m_richCtrl.TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&(info.cr));
    m_linkInfo.push_back(info);

    SetSelNone();
    GetDefaultCharFormat(cf);
    SetSelectionCharFormat(cf);
}
void RichEdit::AddLinkColorTextEx(const DString& str, const DString& color, const DString& linkInfo, const DString& strFontId)
{
    if (!IsRichText() || str.empty() || color.empty()) {
        ASSERT(FALSE);
        return;
    }
    
    std::string link = StringUtil::TToMBCS(linkInfo);
    std::string text = StringUtil::TToMBCS(str);
    std::string font_face;

    CHARFORMAT2W cf;
    GetCharFormat(strFontId, cf);
    font_face = StringUtil::UnicodeToMBCS(cf.szFaceName);
    UiColor dwTextColor = GlobalManager::Instance().Color().GetColor(color);
    static std::string font_format = "{\\fonttbl{\\f0\\fnil\\fcharset%d %s;}}";
    static std::string color_format = "{\\colortbl ;\\red%d\\green%d\\blue%d;}";
    static std::string link_format = "{\\rtf1%s%s\\f0\\fs%d{\\field{\\*\\fldinst{HYPERLINK \"%s\"}}{\\fldrslt{\\cf1 %s}}}}";
    char sfont[255] = { 0 };
    sprintf_s(sfont, font_format.c_str(), cf.bCharSet, font_face.c_str());
    char scolor[255] = { 0 };
    sprintf_s(scolor, color_format.c_str(), dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB());
    char slinke[1024] = { 0 };
    sprintf_s(slinke, link_format.c_str(), sfont, scolor, ((int)(cf.yHeight *1.5))/2*2, link.c_str(), text.c_str());

    SETTEXTEX st;
    st.codepage = ((UINT32)~((UINT32)0));
    st.flags = ST_SELECTION | ST_KEEPUNDO;
    m_richCtrl.TxSendMessage(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)(LPCTSTR)slinke);
    return;
}
void RichEdit::AddLinkInfo(const CHARRANGE cr, const DString &linkInfo)
{
    LinkInfo info;
    info.info = linkInfo;
    info.cr = cr;
    m_linkInfo.push_back(info);
}

void RichEdit::AddLinkInfoEx(const CHARRANGE cr, const DString& linkInfo)
{
    CHARFORMAT2W cf2;
    ZeroMemory(&cf2, sizeof(CHARFORMAT2W));
    cf2.cbSize = sizeof(CHARFORMAT2W);
    cf2.dwMask = CFM_LINK;
    cf2.dwEffects |= CFE_LINK;

    SetSel(cr.cpMin, cr.cpMax);
    SetSelectionCharFormat(cf2);

    AddLinkInfo(cr, linkInfo);
}

//根据point来hittest自定义link的数据，返回true表示在link上，info是link的自定义属性
bool RichEdit::HittestCustomLink(UiPoint pt, DString& info)
{
    bool bLink = false;
    info.clear();
    if (!m_linkInfo.empty()) {
        pt.Offset(GetScrollOffsetInScrollBox());
        int nCharIndex = CharFromPos(pt);
        for (auto it = m_linkInfo.begin(); it != m_linkInfo.end(); it++) {
            if ((*it).cr.cpMin <= nCharIndex && (*it).cr.cpMax > nCharIndex) {
                info = (*it).info.c_str();
                bLink = true;
                break;
            }
        }
    }
    return bLink;
}
#endif

void RichEdit::ClearImageCache()
{
    __super::ClearImageCache();
    if (m_pFocusedImage != nullptr) {
        m_pFocusedImage->ClearImageCache();
    }    
}

void RichEdit::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        Dpi().ScalePadding(padding);
    }
    if (!GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        RelayoutOrRedraw();
    }
}

UiPadding RichEdit::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void RichEdit::SetUseControlCursor(bool bUseControlCursor)
{
    m_bUseControlCursor = bUseControlCursor;
}

void RichEdit::GetClipboardText(DStringW& out )
{
    out.clear();
    BOOL ret = ::OpenClipboard(NULL);
    if(ret) {
        if(::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
            if(h != INVALID_HANDLE_VALUE) {
                wchar_t* buf = (wchar_t*)::GlobalLock(h);
                if(buf != NULL)    {
                    DStringW str(buf, GlobalSize(h)/sizeof(wchar_t));
                    out = str;
                    ::GlobalUnlock(h);
                }
            }
        }
        else if(::IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE h = ::GetClipboardData(CF_TEXT);
            if(h != INVALID_HANDLE_VALUE) {
                char* buf = (char*)::GlobalLock(h);
                if(buf != NULL)    {
                    std::string str(buf, GlobalSize(h));
                    out = StringUtil::MBCSToUnicode(str);
                    ::GlobalUnlock(h);
                }
            }
        }
        ::CloseClipboard();
    }
}

void RichEdit::AttachSelChange(const EventCallback& callback)
{ 
    AttachEvent(kEventSelChange, callback); 
    uint32_t oldEventMask = m_richCtrl.GetEventMask();
    if (!(oldEventMask & ENM_SELCHANGE)) {
        m_richCtrl.SetEventMask(oldEventMask | ENM_SELCHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_SELCHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_CHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_LINK);
    }    
}

void RichEdit::SetEnableWheelZoom(bool bEnable)
{
    m_bEnableWheelZoom = bEnable;
}

bool RichEdit::IsEnableWheelZoom(void) const
{
    return m_bEnableWheelZoom;
}

void RichEdit::SetEnableDefaultContextMenu(bool bEnable)
{
    if (m_bEnableDefaultContextMenu != bEnable) {
        m_bEnableDefaultContextMenu = bEnable;
        if (bEnable) {
            AttachContextMenu([this](const ui::EventArgs& args) {
                if (args.eventType == ui::kEventContextMenu) {
                    ui::UiPoint pt = args.ptMouse;
                    if ((pt.x != -1) && (pt.y != -1)) {
                        //鼠标右键点击产生的上下文菜单                        
                        ShowPopupMenu(pt);
                    }
                    else {
                        //按Shift + F10，由系统产生上下文菜单
                        pt = { 100, 100 };
                        ShowPopupMenu(pt);
                    }
                }
                return true;
                });
        }
        else {
            DetachEvent(kEventContextMenu);
        }
    }
}

bool RichEdit::IsEnableDefaultContextMenu() const
{
    return m_bEnableDefaultContextMenu;
}

void RichEdit::ShowPopupMenu(const ui::UiPoint& point)
{
    RichEdit* pRichEdit = this;
    if ((pRichEdit == nullptr) || !pRichEdit->IsEnabled() || pRichEdit->IsPassword()) {
        return;
    }

    //如果没有选中文本，则将光标切换到当前点击的位置
    long nStartChar = 0; 
    long nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    if (nStartChar == nEndChar) {
        int32_t pos = pRichEdit->m_richCtrl.CharFromPos(POINT(point.x, point.y));
        if (pos > 0) {
            pRichEdit->SetSel(pos, pos);
            pRichEdit->GetSel(nStartChar, nEndChar);
        }
    }
    
    DString skinFolder = _T("public/menu/");
    Menu* menu = new Menu(GetWindow());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(skinFolder);
    DString xml(_T("rich_edit_menu.xml"));

    //菜单弹出位置的坐标应为屏幕坐标
    UiPoint pt = point;
    ClientToScreen(pt);
    menu->ShowMenu(xml, pt);

    ui::MenuItem* menu_item = nullptr;
    //更新命令状态，并添加菜单命令响应
    bool hasSelText = nEndChar > nStartChar ? true : false;
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_copy")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Copy();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_cut")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Cut();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_paste")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanPaste()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Paste();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_del")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Clear();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_sel_all")));
    if (menu_item != nullptr) {
        if ((nStartChar == 0) && (nEndChar == pRichEdit->GetTextLength())) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->SetSelAll();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_undo")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanUndo()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Undo();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_redo")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanRedo()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Redo();
            return true;
            });
    }
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

void RichEdit::SetEnableDragDrop(bool bEnable)
{
    if (m_pRichHost == nullptr) {
        return;
    }
    if (bEnable) {
        //只读模式、密码模式、不可用模式，关闭拖放功能
        if (IsReadOnly() || IsPassword() || !IsEnabled()) {
            bEnable = false;
        }
    }
    if (bEnable) {
        m_pControlDropTarget = new RichEditDropTarget(this, m_pRichHost->GetTextServices());
        m_pControlDropTarget->SetControl(this);
        RegisterDragDrop();
    }
    else {
        UnregisterDragDrop();
        if (m_pControlDropTarget != nullptr) {
            delete m_pControlDropTarget;
            m_pControlDropTarget = nullptr;
        }
    }
}

bool RichEdit::IsEnableDragDrop() const
{
    return m_pControlDropTarget != nullptr;
}

#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
void RichEdit::RegisterDragDrop()
{
    ASSERT(m_pControlDropTarget != nullptr);
    if (m_pControlDropTarget != nullptr) {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            pWindow->RegisterDragDrop(m_pControlDropTarget);
        }
    }
}

void RichEdit::UnregisterDragDrop()
{
    if (m_pControlDropTarget != nullptr) {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            pWindow->UnregisterDragDrop(m_pControlDropTarget);
        }
    }
}
#endif

void RichEdit::OnTextChanged()
{
    if (IsNumberOnly() && ((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX))) {
        //数字模式，检查文本对应的数字是否在范围内
        DString text = GetText();
        if (!text.empty()) {
            int64_t n = StringUtil::StringToInt64(text);
            if (n < GetMinNumber()) {
                //超过最小数字，进行修正
                int32_t newValue = GetMinNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                if (!m_bDisableTextChangeEvent) {
                    SendEvent(kEventTextChange);
                }
                return;
            }
            else if (n > GetMaxNumber()) {
                //超过最大数字，进行修正
                int32_t newValue = GetMaxNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                if (!m_bDisableTextChangeEvent) {
                    SendEvent(kEventTextChange);
                }
                return;
            }
        }
    }
    if (!m_bDisableTextChangeEvent) {
        SendEvent(kEventTextChange);
    }
}

bool RichEdit::SetSpinClass(const DString& spinClass)
{
    DString spinBoxClass;
    DString spinBtnUpClass;
    DString spinBtnDownClass;
    std::list<DString> classNames = StringUtil::Split(spinClass, _T(","));
    if (classNames.size() == 3) {
        auto iter = classNames.begin();
        spinBoxClass = *iter++;
        spinBtnUpClass = *iter++;
        spinBtnDownClass = *iter++;
    }
    if (!spinClass.empty()) {
        ASSERT(!spinBoxClass.empty() && !spinBtnUpClass.empty() && !spinBtnDownClass.empty());
    }
    if (!spinBoxClass.empty() && !spinBtnUpClass.empty() && !spinBtnDownClass.empty()) {        
        Button* pUpButton = nullptr;
        Button* pDownButton = nullptr;
        if (m_pSpinBox == nullptr) {
            m_pSpinBox = new VBox(GetWindow());
            AddItem(m_pSpinBox);

            pUpButton = new Button(GetWindow());
            m_pSpinBox->AddItem(pUpButton);

            pDownButton = new Button(GetWindow());
            m_pSpinBox->AddItem(pDownButton);
        }
        else {
            pUpButton = dynamic_cast<Button*>(m_pSpinBox->GetItemAt(0));
            pDownButton = dynamic_cast<Button*>(m_pSpinBox->GetItemAt(1));            
        }

        ASSERT((pUpButton != nullptr) && (pDownButton != nullptr));
        if ((pUpButton == nullptr) || (pDownButton == nullptr)) {
            RemoveItem(m_pSpinBox);
            m_pSpinBox = nullptr;
            return false;
        }
        m_pSpinBox->SetClass(spinBoxClass);
        pUpButton->SetClass(spinBtnUpClass);
        pDownButton->SetClass(spinBtnDownClass);
        
        //挂载事件处理
        pUpButton->DetachEvent(kEventClick);
        pUpButton->AttachClick([this](const EventArgs& /*args*/){
            AdjustTextNumber(1);
            return true;
            });

        pUpButton->DetachEvent(kEventMouseButtonDown);
        pUpButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            StartAutoAdjustTextNumberTimer(1);
            return true;
            });

        pUpButton->DetachEvent(kEventMouseButtonUp);
        pUpButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pUpButton->DetachEvent(kEventMouseLeave);
        pUpButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pDownButton->DetachEvent(kEventClick);
        pDownButton->AttachClick([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            AdjustTextNumber(-1);
            return true;
            });

        pDownButton->DetachEvent(kEventMouseButtonDown);
        pDownButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            StartAutoAdjustTextNumberTimer(-1);
            return true;
            });

        pDownButton->DetachEvent(kEventMouseButtonUp);
        pDownButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pDownButton->DetachEvent(kEventMouseLeave);
        pDownButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });
        return true;
    }
    else {
        if (m_pSpinBox != nullptr) {
            RemoveItem(m_pSpinBox);
            m_pSpinBox = nullptr;
        }    
    }
    return false;
}

bool RichEdit::SetEnableSpin(bool bEnable, const DString& spinClass, int32_t nMin, int32_t nMax)
{
    bool bRet = false;
    if (bEnable) {
        ASSERT(!spinClass.empty());
        if (spinClass.empty()) {
            return false;
        }
        bRet = SetSpinClass(spinClass);
        if (bRet) {
            SetNumberOnly(true);
            if ((nMin != 0) || (nMax != 0)) {
                SetMaxNumber(nMax);
                SetMinNumber(nMin);
            }            
        }
    }
    else {
        bool hasSpin = m_pSpinBox != nullptr;
        SetSpinClass(_T(""));
        bRet = true;
        if (hasSpin) {
            SetNumberOnly(false);
            SetMaxNumber(INT_MAX);
            SetMinNumber(INT_MIN);
        }        
    }
    return bRet;
}

int64_t RichEdit::GetTextNumber() const
{
    DString text = GetText();
    if (text.empty()) {
        return 0;
    }
    int64_t n = StringUtil::StringToInt64(text);
    return n;
}

void RichEdit::SetTextNumber(int64_t nValue)
{
    SetText(StringUtil::Printf(_T("%I64d"), nValue));
}

void RichEdit::AdjustTextNumber(int32_t nDelta)
{
    ASSERT(IsNumberOnly());
    if (IsNumberOnly()) {
        const int64_t nOldValue = GetTextNumber();
        int64_t nNewValue = nOldValue + nDelta;
        if (((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX))) {
            if (nNewValue > GetMaxNumber()) {
                //超过最大数字，进行修正
                nNewValue = GetMaxNumber();
            }
            else if (nNewValue < GetMinNumber()) {
                //小于最小数字，进行修正
                nNewValue = GetMinNumber();
            }
        }
        if (nNewValue != nOldValue) {
            SetTextNumber(nNewValue);
        }
    }
}

void RichEdit::StartAutoAdjustTextNumberTimer(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::StartAutoAdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 1000, 1);
    }
}

void RichEdit::StartAutoAdjustTextNumber(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::AdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 120);
    }
}

void RichEdit::StopAutoAdjustTextNumber()
{
    m_flagAdjustTextNumber.Cancel();
}

void RichEdit::SetClearBtnClass(const DString& btnClass)
{
    if (!btnClass.empty()) {
        ASSERT(m_pClearButton == nullptr);
        if (m_pClearButton != nullptr) {
            return;
        }
        Button* pClearButton = new Button(GetWindow());
        pClearButton->SetClass(btnClass);
        pClearButton->SetNoFocus();
        pClearButton->SetVisible(false);
        AddItem(pClearButton);
        m_pClearButton = pClearButton;

        //响应按钮点击事件
        pClearButton->AttachClick([this](const EventArgs& /*args*/) {
            SetText(_T(""));
            return true;
            });
    }
}

void RichEdit::SetShowPasswordBtnClass(const DString& btnClass)
{
    if (!btnClass.empty()) {
        ASSERT(m_pShowPasswordButton == nullptr);
        if (m_pShowPasswordButton != nullptr) {
            return;
        }
        Button* pButton = new Button(GetWindow());
        pButton->SetClass(btnClass);
        pButton->SetNoFocus();
        pButton->SetVisible(false);
        AddItem(pButton);
        m_pShowPasswordButton = pButton;

        //响应按钮点击事件
        pButton->AttachClick([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
        pButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            SetShowPassword(true);
            return true;
            });
        pButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
        pButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
    }
    else {
        if (m_pShowPasswordButton != nullptr) {
            RemoveItem(m_pShowPasswordButton);
            m_pShowPasswordButton = nullptr;
        }
    }
}

void RichEdit::GetCharFormat(const DString& fontId, CHARFORMAT2W& cf) const
{
    ZeroMemory(&cf, sizeof(CHARFORMAT2W));
    cf.cbSize = sizeof(CHARFORMAT2W);
    m_richCtrl.GetDefaultCharFormat(cf);
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(fontId, Dpi());
    if (pFont != nullptr) {
        wcscpy_s(cf.szFaceName, StringUtil::TToUTF16(pFont->FontName()).c_str());
        cf.dwMask |= CFM_FACE;

        cf.yHeight = ConvertToFontHeight(pFont->FontSize());
        cf.dwMask |= CFM_SIZE;

        LOGFONTW lf = { 0, };
        ::GetObjectW(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW), &lf);

        cf.bCharSet = lf.lfCharSet;
        cf.dwMask |= CFM_CHARSET;

        cf.bPitchAndFamily = lf.lfPitchAndFamily;

        if (pFont->IsUnderline()) {
            cf.dwEffects |= CFE_UNDERLINE;
        }
        else {
            cf.dwEffects &= ~CFE_UNDERLINE;
        }
        cf.dwMask |= CFM_UNDERLINE;

        if (pFont->IsStrikeOut()) {
            cf.dwEffects |= CFE_STRIKEOUT;
        }
        else {
            cf.dwEffects &= ~CFE_STRIKEOUT;
        }
        cf.dwMask |= CFM_STRIKEOUT;

        if (pFont->IsItalic()) {
            cf.dwEffects |= CFE_ITALIC;
        }
        else {
            cf.dwEffects &= ~CFE_ITALIC;
        }
        cf.dwMask |= CFM_ITALIC;

        if (pFont->IsBold()) {
            cf.dwEffects |= CFE_BOLD;
        }
        else {
            cf.dwEffects &= ~CFE_BOLD;
        }
        cf.dwMask |= CFM_BOLD;
    }
}

void RichEdit::SetFontIdInternal(const DString& fontId)
{
    CHARFORMAT2W cf;
    GetCharFormat(fontId, cf);
    BOOL bRet = m_richCtrl.SetDefaultCharFormat(cf);
    ASSERT_UNUSED_VARIABLE(bRet);
}

void RichEdit::SetTextColorInternal(const UiColor& textColor)
{
    if (!textColor.IsEmpty()) {
        CHARFORMAT2W cf;
        ZeroMemory(&cf, sizeof(CHARFORMAT2W));
        cf.cbSize = sizeof(CHARFORMAT2W);
        m_richCtrl.GetDefaultCharFormat(cf);
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = textColor.ToCOLORREF();
        cf.dwEffects &= ~CFE_AUTOCOLOR;
        BOOL bRet = m_richCtrl.SetDefaultCharFormat(cf);
        ASSERT_UNUSED_VARIABLE(bRet);
    }
}

int32_t RichEdit::ConvertToFontHeight(int32_t fontSize) const
{
    bool bGetDC = false;
    HDC hDC = GetDrawDC();
    if (hDC == nullptr) {
        hDC = ::GetDC(nullptr);
        bGetDC = true;
    }
    LONG yPixPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
    if (bGetDC && (hDC != nullptr)) {
        ::ReleaseDC(nullptr, hDC);
        hDC = nullptr;
    }
    if (yPixPerInch == 0) {
        yPixPerInch = 96;
    }
    constexpr const int32_t LY_PER_INCH = 1440;
    int32_t lfHeight = fontSize * LY_PER_INCH / yPixPerInch;
    return lfHeight;
}

void RichEdit::SetHAlignType(HorAlignType alignType)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetHAlignType(alignType);
    }
    PARAFORMAT pf;
    ZeroMemory(&pf, sizeof(PARAFORMAT));
    pf.cbSize = sizeof(PARAFORMAT);
    m_richCtrl.GetParaFormat(pf);
    pf.dwMask |= PFM_ALIGNMENT;
    if (alignType == HorAlignType::kHorAlignCenter) {
        pf.wAlignment = PFA_CENTER;
    }        
    else if (alignType == HorAlignType::kHorAlignRight) {
        pf.wAlignment = PFA_RIGHT;
    }
    else {
        pf.wAlignment = PFA_LEFT;
    }
    BOOL bRet = m_richCtrl.SetParaFormat(pf);
    ASSERT_UNUSED_VARIABLE(bRet);
}

void RichEdit::SetVAlignType(VerAlignType alignType)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetVAlignType(alignType);
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
