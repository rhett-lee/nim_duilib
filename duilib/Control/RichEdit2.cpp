#include "RichEdit2.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/Clipboard.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Menu.h"
#include "duilib/Control/Button.h"
#include "duilib/Box/VBox.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "RichEditDropTarget_Windows.h"
    #include "RichEditDragSource_Windows.h"
#elif defined (DUILIB_BUILD_FOR_WIN)
    #include "RichEditDragSource_Windows.h"
#endif

#ifdef DUILIB_BUILD_FOR_SDL
    #include "RichEditDropTarget_SDL.h"
    #include <SDL3/SDL.h>
#endif //DUILIB_BUILD_FOR_SDL

//缩放百分比的最大值
#define MAX_ZOOM_PERCENT 800

namespace ui {

RichEdit2::RichEdit2(Window* pWindow) :
    ScrollBox(pWindow, new Layout),
    m_bWantTab(false),
    m_bWantReturn(false),
    m_bWantCtrlReturn(false),
    m_bAllowPrompt(false),
    m_bSelAllEver(false),         
    m_bNoSelOnKillFocus(true), 
    m_bSelAllOnFocus(false),    
    m_bNoCaretReadonly(false),
    m_bIsCaretVisible(false),
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_bIsComposition(false),    
    #ifdef DUILIB_UNICODE
        m_chHighSurrogate(L'\0'),
    #else
        m_dwLastCharTime(0),
    #endif
#endif
    m_iCaretPosX(0),
    m_iCaretPosY(0),
    m_iCaretWidth(0),
    m_iCaretHeight(0),
    m_nRowHeight(0),
    m_sFontId(),
    m_sTextColor(),
    m_sDisabledTextColor(),
    m_sPromptColor(),
    m_sPromptText(),
    m_drawCaretFlag(),
    m_pFocusedImage(nullptr),
    m_bUseControlCursor(false),
    m_nZoomPercent(100),
    m_bEnableWheelZoom(false),
    m_bEnableDefaultContextMenu(false),
    m_bDisableTextChangeEvent(false),
    m_maxNumber(INT_MAX),
    m_minNumber(INT_MIN),
    m_pSpinBox(nullptr),
    m_pClearButton(nullptr),
    m_pShowPasswordButton(nullptr),
    m_bReadOnly(false),
    m_bPasswordMode(false),
    m_bShowPassword(false),
    m_chPasswordChar(L'*'),
    m_bFlashPasswordChar(false),
    m_bInputPasswordChar(false),
    m_bNumberOnly(false),
    m_bWordWrap(false),
    m_nLimitText(0),
    m_bModified(false),
    m_nSelStartIndex(0),
    m_nSelEndCharIndex(0),
    m_nShiftStartIndex(-1),
    m_nCtrlStartIndex(-1),
    m_bSelForward(true),
    m_nSelXPos(-1),
    m_bHideSelection(true),
    m_bActive(false),
    m_bTextInputMode(false),
    m_bMouseDownInView(false),
    m_bMouseDown(false),
    m_bRMouseDown(false),
    m_bInMouseMove(false),
    m_pMouseSender(nullptr),
    m_pTextData(nullptr),
    m_sSelectionBkColor(_T("CornflowerBlue")),
    m_sInactiveSelectionBkColor(_T("DarkGray")),
    m_sCurrentRowBkColor(_T("")),
    m_sInactiveCurrentRowBkColor(_T("")),
    m_nFocusBottomBorderSize(0),
    m_fRowSpacingMul(1.0f),
    m_fRowSpacingAdd(0.0f),
    m_pControlDropTarget(nullptr),
    m_bEnableDragOut(true),
    m_bDraggingOut(false),
    m_bDraggingOutMouseDown(false),
    m_nDropTextPos(-1),
    m_bReplaceNewline(false)
{
    m_pTextData = new RichEditData(this);
}

RichEdit2::~RichEdit2()
{
    if (m_pFocusedImage != nullptr) {
        delete m_pFocusedImage;
        m_pFocusedImage = nullptr;
    }
    m_pLimitChars.reset();
    if (m_pTextData != nullptr) {
        delete m_pTextData;
        m_pTextData = nullptr;
    }
    if (m_pControlDropTarget != nullptr) {
        delete m_pControlDropTarget;
        m_pControlDropTarget = nullptr;
    }
    DString internalFontId = GetInternalFontId();
    if (GlobalManager::Instance().Font().HasFontId(internalFontId)) {
        GlobalManager::Instance().Font().RemoveFontId(internalFontId);
    }
}

DString RichEdit2::GetType() const { return DUI_CTR_RICHEDIT2; }

void RichEdit2::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetMultiLine(strValue != _T("true"));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        SetMultiLine(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("readonly")) {
        SetReadOnly(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("password")) {
        SetPasswordMode(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("show_password")) {
        SetShowPassword(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("password_char")) {
        if (!strValue.empty()) {
            SetPasswordChar(strValue.front());
        }
    }
    else if (strName == _T("flash_password_char")) {
        SetFlashPasswordChar(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("number_only")) || (strName == _T("number"))) {
        SetNumberOnly(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("max_number")) {
        SetMaxNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("min_number")) {
        SetMinNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("number_format")) {
        SetNumberFormat64(strValue);
    }
    else if (strName == _T("text_align")) {
        //水平方向对齐
        if (strValue.find(_T("left")) != DString::npos) {
            SetTextHAlignType(HorAlignType::kAlignLeft);
        }
        else if (strValue.find(_T("hcenter")) != DString::npos) {
            SetTextHAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            SetTextHAlignType(HorAlignType::kAlignRight);
        }

        //垂直方向对齐
        if (strValue.find(_T("top")) != DString::npos) {
            SetTextVAlignType(VerAlignType::kAlignTop);
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            SetTextVAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            SetTextVAlignType(VerAlignType::kAlignBottom);
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
        SetPromptMode(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("prompt_color")) || (strName == _T("promptcolor"))) {
        //提示文字的颜色
        SetPromptTextColor(strValue);
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
        if (IsReplaceNewline()) {
            //将反斜杠+n这两个字符替换成换行符
            StringUtil::ReplaceAll(_T("\\n"), _T("\n"), strValue);
        }
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        DString strText = GlobalManager::Instance().Lang().GetStringByID(strValue);
        if (IsReplaceNewline()) {
            //将反斜杠+n这两个字符替换成换行符
            StringUtil::ReplaceAll(_T("\\n"), _T("\n"), strText);
        }
        SetText(strText);
    }
    else if ((strName == _T("want_tab")) || (strName == _T("wanttab"))) {
        SetWantTab(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("want_return")) || (strName == _T("want_return_msg")) || (strName == _T("wantreturnmsg"))) {
        SetWantReturn(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("want_ctrl_return")) || (strName == _T("return_msg_want_ctrl")) || (strName == _T("returnmsgwantctrl"))) {
        SetWantCtrlReturn(StringUtil::IsValueTrue(strValue));
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
        SetWordWrap(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("no_caret_readonly")) {
        //只读模式，不显示光标
        SetNoCaretReadonly();
    }
    else if (strName == _T("default_context_menu")) {
        //是否使用默认的右键菜单
        SetEnableDefaultContextMenu(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("spin_class")) {
        SetSpinClass(strValue);
    }
    else if (strName == _T("clear_btn_class")) {
        SetClearBtnClass(strValue);
    }
    else if (strName == _T("show_password_btn_class")) {
        SetShowPasswordBtnClass(strValue);
    }
    else if (strName == _T("wheel_zoom")) {
        //设置是否允许Ctrl + 滚轮来调整缩放比例
        SetEnableWheelZoom(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("hide_selection")) {
        //当控件处于非激活状态时，是否隐藏选择内容
        SetHideSelection(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("focused_bottom_border_size")) || (strName == _T("focus_bottom_border_size"))) {
        //焦点状态时，底部边框的大小
        SetFocusedBottomBorderSize(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("focused_bottom_border_color")) || (strName == _T("focus_bottom_border_color"))) {
        //焦点状态时，底部边框的颜色
        SetFocusedBottomBorderColor(strValue);
    }
    else if (strName == _T("zoom")) {
        //缩放比例：格式有两种，一种如"2,1" 放大到200%； 表示另外一种如："200%"，代表放大到200%。
        // "2,1"这种格式设置缩放比例（兼容微软的RichEdit控件格式）：设 wParam：缩放比例的分子，lParam：缩放比例的分母，
        //                         "wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。
        //                         举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50%
        float fZoomRatio = 1.0f;
        if (strValue.find(L',') != DString::npos) {
            UiSize zoomValue;
            AttributeUtil::ParseSizeValue(strValue.c_str(), zoomValue);
            if ((zoomValue.cx > 0) && (zoomValue.cx <= 64) &&
                (zoomValue.cy > 0) && (zoomValue.cy <= 64)) {
                fZoomRatio = (float)zoomValue.cx / (float)zoomValue.cy;
            }
        }
        else if (strValue.find(L'%') != DString::npos) {
            DString zoomValue = strValue.substr(0, strValue.find(L'%'));
            int32_t nZoomValue = StringUtil::StringToInt32(zoomValue.c_str());
            ASSERT(nZoomValue > 0);
            if (nZoomValue > 0) {
                fZoomRatio = (float)nZoomValue / 100.0f;
            }
        }
        else {
            ASSERT(0);
        }
        uint32_t nZoomPercent = (uint32_t)(fZoomRatio * 100.0f);
        SetZoomPercent(nZoomPercent);
    }

    //这几个属性，不支持
    else if ((strName == _T("auto_vscroll")) || (strName == _T("autovscroll"))) {
        //当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
    }
    else if ((strName == _T("auto_hscroll")) || (strName == _T("autohscroll"))) {
        //当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
        //当用户按 Enter 时，控件会将所有文本滚动回零位置。
    }
    else if ((strName == _T("rich_text")) || (strName == _T("rich"))) {
        //是否为富文本属性
    }
    else if (strName == _T("auto_detect_url")) {
        //是否自动检测URL，如果是URL则显示为超链接
    }
    else if (strName == _T("allow_beep")) {
        //是否允许发出Beep声音
    }
    else if (strName == _T("save_selection")) {
        //如果 为 TRUE，则当控件处于非活动状态时，应保存所选内容的边界。
        //如果 为 FALSE，则当控件再次处于活动状态时，可以选择边界重置为 start = 0，length = 0。
    }
    else if ((strName == _T("select_all_on_focused")) || (strName == _T("select_all_on_focus"))) {
        //获取焦点的时候，是否全选
        SetSelAllOnFocus(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("selection_bkcolor")) {
        //选择文本的背景色（焦点状态），如果设置为空，则不显示
        SetSelectionBkColor(strValue);
    }
    else if (strName == _T("inactive_selection_bkcolor")) {
        //选择文本的背景色（非焦点状态），如果设置为空，则不显示
        SetInactiveSelectionBkColor(strValue);
    }
    else if (strName == _T("current_row_bkcolor")) {
        //当前行的背景色（焦点状态），如果设置为空，则在焦点状态不显示当前行的背景色
        SetCurrentRowBkColor(strValue);
    }
    else if (strName == _T("inactive_current_row_bkcolor")) {
        //当前行的背景色（非焦点状态），如果设置为空，则在非焦点状态不显示当前行的背景色
        SetInactiveCurrentRowBkColor(strValue);
    }
    else if (strName == _T("row_spacing_mul")) {
        SetRowSpacingMul(StringUtil::StringToFloat(strValue.c_str(), nullptr));
    }
    else if (strName == _T("row_spacing_add")) {
        SetRowSpacingAdd(StringUtil::StringToFloat(strValue.c_str(), nullptr));
    }
    else if (strName == _T("enable_drag_out")) {
        //是否允许拖出功能（作为拖放源）
        SetEnableDragOut(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("replace_newline")) {
        // 设置是否替换换行符(将字符串"\\n"替换为换行符"\n"
        SetReplaceNewline(StringUtil::IsValueTrue(strValue));
    }
    else {
        ScrollBox::SetAttribute(strName, strValue);
    }
}

void RichEdit2::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //设置字体和字体颜色
    DString fontId = GetCurrentFontId();
    if (fontId.empty()) {
        fontId = GlobalManager::Instance().Font().GetDefaultFontId();
        SetFontId(fontId);
    }

    ScrollBar* pHScrollBar = GetHScrollBar();
    if (pHScrollBar != nullptr) {
        pHScrollBar->SetScrollRange(0);
    }
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollRange(0);
    }
}

void RichEdit2::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);

    //更新字体大小
    SetFontIdInternal(GetCurrentFontId());

    //清除绘制缓存，并重绘
    ClearCacheAndRedraw();

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

bool RichEdit2::IsWantTab() const
{
    return m_bWantTab;
}

void RichEdit2::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool RichEdit2::CanPlaceCaptionBar() const
{
    return true;
}

bool RichEdit2::IsWantReturn() const
{
    return m_bWantReturn;
}

void RichEdit2::SetWantReturn(bool bWantReturn)
{
    m_bWantReturn = bWantReturn;
}

bool RichEdit2::IsWantCtrlReturn() const
{
    return m_bWantCtrlReturn;
}

void RichEdit2::SetWantCtrlReturn(bool bWantCtrlReturn)
{
    m_bWantCtrlReturn = bWantCtrlReturn;
}

bool RichEdit2::IsReadOnly() const
{
    return m_bReadOnly;
}

void RichEdit2::SetReadOnly(bool bReadOnly)
{
    if (m_bReadOnly != bReadOnly) {
        m_bReadOnly = bReadOnly;
        Redraw();
    }    
}

bool RichEdit2::RemoveInvalidPasswordChar(DStringA& text)
{
    bool bRet = false;
    if (!text.empty()) {
        DStringA oldText = text;
        StringUtil::ReplaceAll("\r", "", text);
        StringUtil::ReplaceAll("\n", "", text);
        StringUtil::ReplaceAll("\t", "", text);
        if (oldText != text) {
            bRet = true;
        }
    }
    return bRet;
}

bool RichEdit2::RemoveInvalidPasswordChar(DStringW& text)
{
    bool bRet = false;
    if (!text.empty()) {
        DStringW oldText = text;
        StringUtil::ReplaceAll(L"\r", L"", text);
        StringUtil::ReplaceAll(L"\n", L"", text);
        StringUtil::ReplaceAll(L"\t", L"", text);
        if (oldText != text) {
            bRet = true;
        }
    }
    return bRet;
}

void RichEdit2::ReplacePasswordChar(DStringW& text) const
{
    const size_t nTextLen = text.size();
    if (!IsShowPassword() && (nTextLen > 0)) {
        DStringW oldText = text;
        text.clear();
        text.resize(nTextLen, m_chPasswordChar);
        if (IsFlashPasswordChar() && m_bInputPasswordChar) {
            //最后一个字符：闪现
            text[nTextLen - 1] = oldText[nTextLen - 1];
        }
    }
}

int32_t RichEdit2::GetTextLimitLength() const
{
    return GetLimitText();
}

bool RichEdit2::IsTextReadOnlyMode() const
{
    return IsReadOnly();
}

bool RichEdit2::IsTextDisabledMode() const
{
    return !IsEnabled();
}

void RichEdit2::StopFlashPasswordChar()
{
    if (m_bInputPasswordChar) {
        m_bInputPasswordChar = false;
        Invalidate();
    }
}

bool RichEdit2::IsPasswordMode() const
{
    return m_bPasswordMode;
}

void RichEdit2::SetPasswordMode(bool bPasswordMode)
{
    if (m_bPasswordMode != bPasswordMode) {
        m_bPasswordMode = bPasswordMode;
        bool bUpdated = false;
        if (bPasswordMode) {
            //密码模式下，去掉文本中的回车，换行，TAB键
            DString text = GetText();
            if (RemoveInvalidPasswordChar(text)) {
                SetText(text);
                bUpdated = true;
            }
        }
        if (!bUpdated) {
            m_pTextData->SetCacheDirty(true);
            Redraw();
        }
    }
}

void RichEdit2::SetShowPassword(bool bShow)
{
    if (m_bShowPassword != bShow) {
        m_bShowPassword = bShow;
        m_pTextData->SetCacheDirty(true);
        Redraw();
    }
}

bool RichEdit2::IsShowPassword() const
{
    return m_bShowPassword;
}

void RichEdit2::SetPasswordChar(DStringW::value_type ch)
{
    if (m_chPasswordChar != ch) {
        m_chPasswordChar = ch;
        if (IsPasswordMode()) {
            m_pTextData->SetCacheDirty(true);
            Redraw();
        }
    }
}

DStringW::value_type RichEdit2::GetPasswordChar() const
{
    return m_chPasswordChar;
}

void RichEdit2::SetFlashPasswordChar(bool bFlash)
{
    m_bFlashPasswordChar = bFlash;
}

bool RichEdit2::IsFlashPasswordChar() const
{
    return m_bFlashPasswordChar;
}

bool RichEdit2::IsNumberOnly() const
{
    return m_bNumberOnly;
}

void RichEdit2::SetNumberOnly(bool bNumberOnly)
{
    //只影响输入，不影响其他方式（比如SetText等方式改变文本）
    m_bNumberOnly = bNumberOnly;
}

void RichEdit2::SetMaxNumber(int32_t maxNumber)
{
    m_maxNumber = maxNumber;
}

int32_t RichEdit2::GetMaxNumber() const
{
    return m_maxNumber;
}

void RichEdit2::SetMinNumber(int32_t minNumber)
{
    m_minNumber = minNumber;
}

int32_t RichEdit2::GetMinNumber() const
{
    return m_minNumber;
}

void RichEdit2::SetNumberFormat64(const DString& numberFormat)
{
    DString format = numberFormat;
#if defined (DUILIB_BUILD_FOR_WIN)
    StringUtil::ReplaceAll(_T("lld"), _T("I64d"), format);
#else
    StringUtil::ReplaceAll(_T("I64d"), _T("lld"), format);
#endif
    m_numberFormat = format;
}

DString RichEdit2::GetNumberFormat64() const
{
    return m_numberFormat.c_str();
}

bool RichEdit2::IsWordWrap() const
{
    return m_bWordWrap;
}

void RichEdit2::SetWordWrap(bool bWordWrap)
{
    if (m_bWordWrap != bWordWrap) {
        m_bWordWrap = bWordWrap;
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

bool RichEdit2::IsMultiLine() const
{
    return !m_pTextData->IsSingleLineMode();
}

void RichEdit2::SetMultiLine(bool bMultiLine)
{
    bool bSingleLineMode = !bMultiLine;
    if (m_pTextData->IsSingleLineMode() != bSingleLineMode) {
        m_pTextData->SetSingleLineMode(bSingleLineMode);
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

void RichEdit2::SetFontId(const DString& strFontId)
{
    if (m_sFontId != strFontId) {
        m_sFontId = strFontId;
        //如果有外部设置的字体信息，则移除（使用SetFontInfo设置的字体信息）
        DString internalFontId = GetInternalFontId();
        if (GlobalManager::Instance().Font().HasFontId(internalFontId)) {
            GlobalManager::Instance().Font().RemoveFontId(internalFontId);
        }
        OnFontChanged(strFontId);
    }
}

DString RichEdit2::GetFontId() const
{
    return m_sFontId.c_str();
}

DString RichEdit2::GetInternalFontId() const
{
    return StringUtil::Printf(_T("RichEdit_SDL(0x%p)"), this);
}

DString RichEdit2::GetCurrentFontId() const
{
    DString fontId = GetInternalFontId();
    if (!GlobalManager::Instance().Font().HasFontId(fontId)) {
        fontId = GetFontId();
    }
    return fontId;
}

void RichEdit2::SetFontIdInternal(const DString& fontId)
{
    //创建光标
    IFont* pFont = GetIFontInternal(fontId);
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return;
    }
    IRender* pRender = nullptr;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pRender = pWindow->GetRender();
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //按字体高度设置光标的高度
    MeasureStringParam measureParam;
    measureParam.pFont = pFont;
    UiRect fontRect = pRender->MeasureString(_T("T"), measureParam);
    m_nRowHeight = fontRect.Height();
    ASSERT(m_nRowHeight > 0);
    int32_t nCaretHeight = fontRect.Height();
    int32_t nCaretWidth = Dpi().GetScaleInt(1);
    CreateCaret(nCaretWidth, nCaretHeight);

    //设置滚动条滚动一行的基本单位
    SetVerScrollUnitPixels(m_nRowHeight, false);

    m_nSelXPos = -1;
}

IFont* RichEdit2::GetIFontInternal(const DString& fontId) const
{
    ASSERT(!fontId.empty());
    const DpiManager& dpi = Dpi();
    uint32_t nZoomPercent = GetZoomPercent();
    ASSERT(nZoomPercent != 0);
    dpi.ScaleInt(nZoomPercent);
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(fontId, nZoomPercent);
    ASSERT(pFont != nullptr);
    return pFont;
}

UiFont RichEdit2::GetFontInfo() const
{
    UiFont uiFont;
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(GetCurrentFontId(), Dpi());
    if (pFont != nullptr) {
        uiFont.m_fontName = pFont->FontName();
        uiFont.m_fontSize = pFont->FontSize();
        uiFont.m_bBold = pFont->IsBold();
        uiFont.m_bUnderline = pFont->IsUnderline();
        uiFont.m_bItalic = pFont->IsItalic();
        uiFont.m_bStrikeOut = pFont->IsStrikeOut();
    }
    return uiFont;
}

bool RichEdit2::SetFontInfo(const UiFont& fontInfo)
{
    ASSERT(fontInfo.m_fontSize > 0);
    if (fontInfo.m_fontSize <= 0) {
        return false;
    }
    ASSERT(!fontInfo.m_fontName.empty());
    if (fontInfo.m_fontName.empty()) {
        return false;
    }
    //删除旧的字体信息
    DString internalFontId = GetInternalFontId();
    if (GlobalManager::Instance().Font().HasFontId(internalFontId)) {
        GlobalManager::Instance().Font().RemoveFontId(internalFontId);
    }
    //添加字体信息(去除DPI缩放)
    UiFont orgFontInfo = fontInfo;
    const DpiManager& dpi = Dpi();
    if (dpi.GetDisplayScaleFactor() > 0) {
        dpi.UnscaleInt(orgFontInfo.m_fontSize);
        if (dpi.GetScaleInt(orgFontInfo.m_fontSize) != fontInfo.m_fontSize) {
            //计算原来的字体大小
            int32_t nFontSizeMax = fontInfo.m_fontSize;
            if (dpi.GetDisplayScaleFactor() < 100) {
                nFontSizeMax = dpi.GetUnscaleInt(fontInfo.m_fontSize) + 1;
            }
            for (int32_t nFontSize = 1; nFontSize <= nFontSizeMax; ++nFontSize) {
                int32_t nScaledFontSize = dpi.GetScaleInt(nFontSize);
                if (nScaledFontSize == fontInfo.m_fontSize) {
                    orgFontInfo.m_fontSize = nFontSize;
                    break;
                }
            }
        }
    }
    GlobalManager::Instance().Font().AddFont(internalFontId, orgFontInfo, false);

    OnFontChanged(internalFontId);
    return true;
}

void RichEdit2::OnFontChanged(const DString& fontId)
{
    SetFontIdInternal(fontId);
    //清除绘制缓存，并重绘
    ClearCacheAndRedraw();
}

void RichEdit2::OnZoomPercentChanged(uint32_t nOldZoomPercent, uint32_t nNewZoomPercent)
{
    //删除旧的字体缓存，以释放内存
    if (nOldZoomPercent != 100) {
        uint32_t nZoomPercent = Dpi().GetScaleInt(nOldZoomPercent);
        DString internalFontId = GetInternalFontId();
        if (GlobalManager::Instance().Font().HasFontId(internalFontId)) {
            GlobalManager::Instance().Font().RemoveIFont(internalFontId, nZoomPercent);
        }
        DString fontId = GetFontId();
        if (GlobalManager::Instance().Font().HasFontId(fontId)) {
            GlobalManager::Instance().Font().RemoveIFont(fontId, nZoomPercent);
        }
    }

    SetFontIdInternal(GetCurrentFontId());

    //清除绘制缓存，并重绘
    ClearCacheAndRedraw();

    //触发kEventZoom事件
    SendEvent(kEventZoom, (WPARAM)nNewZoomPercent, 0);
}

void RichEdit2::SetTextColor(const DString& dwTextColor)
{
    if (m_sTextColor != dwTextColor) {
        m_sTextColor = dwTextColor;
        if (IsEnabled()) {
            Redraw();
        }
    }    
}

DString RichEdit2::GetTextColor() const
{
    if (!m_sTextColor.empty()) {
        return m_sTextColor.c_str();
    }
    else {
        if (GetWindow() != nullptr) {
            return GetWindow()->GetDefaultTextColor();
        }
        else {
            return GlobalManager::Instance().Color().GetDefaultTextColor();
        }
    }
}

void RichEdit2::SetDisabledTextColor(const DString& dwTextColor)
{
    if (m_sDisabledTextColor != dwTextColor) {
        m_sDisabledTextColor = dwTextColor;
        if (!IsEnabled()) {
            Redraw();
        }
    }    
}

DString RichEdit2::GetDisabledTextColor() const
{
    if (!m_sDisabledTextColor.empty()) {
        return m_sDisabledTextColor.c_str();
    }
    else if (!m_sTextColor.empty()) {
        return m_sTextColor.c_str();
    }
    else {
        if (GetWindow() != nullptr) {
            return GetWindow()->GetDefaultDisabledTextColor();
        }
        else {
            return GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
        }        
    }
}

void RichEdit2::SetSelectionBkColor(const DString& selectionBkColor)
{
    if (m_sSelectionBkColor != selectionBkColor) {
        m_sSelectionBkColor = selectionBkColor;
        Invalidate();
    }
}

DString RichEdit2::GetSelectionBkColor() const
{
    return m_sSelectionBkColor.c_str();
}

void RichEdit2::SetInactiveSelectionBkColor(const DString& selectionBkColor)
{
    if (m_sInactiveSelectionBkColor != selectionBkColor) {
        m_sInactiveSelectionBkColor = selectionBkColor;
        Invalidate();
    }
}

DString RichEdit2::GetInactiveSelectionBkColor() const
{
    return m_sInactiveSelectionBkColor.c_str();
}

void RichEdit2::SetCurrentRowBkColor(const DString& currentRowBkColor)
{
    m_sCurrentRowBkColor = currentRowBkColor;
}

DString RichEdit2::GetCurrentRowBkColor() const
{
    return m_sCurrentRowBkColor.c_str();
}

void RichEdit2::SetInactiveCurrentRowBkColor(const DString& currentRowBkColor)
{
    m_sInactiveCurrentRowBkColor = currentRowBkColor;
}

DString RichEdit2::GetInactiveCurrentRowBkColor() const
{
    return m_sInactiveCurrentRowBkColor.c_str();
}

float RichEdit2::GetRowSpacingMul() const
{
    return m_fRowSpacingMul;
}

void RichEdit2::SetRowSpacingMul(float fRowSpacingMul)
{
    if (m_fRowSpacingMul != fRowSpacingMul) {
        m_fRowSpacingMul = fRowSpacingMul;
        if (m_fRowSpacingMul <= 0.01f) {
            m_fRowSpacingMul = 1.0f;
        }
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

float RichEdit2::GetRowSpacingAdd() const
{
    return m_fRowSpacingAdd;
}

void RichEdit2::SetRowSpacingAdd(float fRowSpacingAdd)
{
    if (m_fRowSpacingAdd != fRowSpacingAdd) {
        m_fRowSpacingAdd = fRowSpacingAdd;
        if (m_fRowSpacingAdd <= 0.0001f) {
            m_fRowSpacingAdd = 0.0f;
        }
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

int32_t RichEdit2::GetLimitText() const
{
    return m_nLimitText;
}

void RichEdit2::SetLimitText(int32_t iChars)
{
    if (iChars < 0) {
        iChars = 0;
    }
    m_nLimitText = iChars;
    if (m_nLimitText > 0) {
        //有限制
        int32_t nTextLen = GetTextLength();
        if (nTextLen > m_nLimitText) {
            //截断当前的文本
            DStringW text = m_pTextData->GetText();
            m_pTextData->TruncateLimitText(text, m_nLimitText);
            SetText(text);
        }
    }    
}

DString RichEdit2::GetLimitChars() const
{
    if (m_pLimitChars != nullptr) {
        return StringConvert::WStringToT(m_pLimitChars.get());
    }
    else {
        return DString();
    }
}

void RichEdit2::SetLimitChars(const DString& limitChars)
{
    m_pLimitChars.reset();
    DStringW limitCharsW = StringConvert::TToWString(limitChars);
    if (!limitCharsW.empty()) {
        size_t nLen = limitCharsW.size() + 1;
        m_pLimitChars.reset(new DStringW::value_type[nLen]);
        memset(m_pLimitChars.get(), 0, nLen * sizeof(DStringW::value_type));
        StringUtil::StringCopy(m_pLimitChars.get(), nLen, limitCharsW.c_str());
    }
}

int32_t RichEdit2::GetTextLength() const
{
    return (int32_t)m_pTextData->GetTextLength();
}

int32_t RichEdit2::InsertText(int32_t nInsertAfterChar, const DString& text, bool bCanUndo)
{
    int32_t nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
    ReplaceSel(text, bCanUndo);
    return nRet;
}

int32_t RichEdit2::AppendText(const DString& text, bool bCanUndo, bool bScrollBottom)
{
    int32_t nRet = SetSel(-1, -1);
    ReplaceSel(text, bCanUndo);
    if (bScrollBottom) {
        int64_t nScrollRangeY = GetScrollRange().cy;
        if (nScrollRangeY > 0) {
            SetScrollPosY(nScrollRangeY);
        }
    }
    return nRet;
}

bool RichEdit2::IsEmpty() const
{
    return m_pTextData->IsEmpty();
}

DString RichEdit2::GetText() const
{
    
#ifdef DUILIB_UNICODE
    return m_pTextData->GetText();
#else
    return StringConvert::WStringToUTF8(m_pTextData->GetText());
#endif
}

DStringA RichEdit2::GetTextA() const
{
    return StringConvert::WStringToUTF8(m_pTextData->GetText());
}

DStringW RichEdit2::GetTextW() const
{
    return m_pTextData->GetText();
}

void RichEdit2::SetText(const DStringW& strText)
{
    //目前内存占用情况：2MB的UTF16格式文本，Debug版本：占用约23MB的内存，Release版本：占用约12MB的内存。
    bool bChanged = false;
    if (IsPasswordMode()) {
        //密码模式
        DStringW passwordText = strText;
        RemoveInvalidPasswordChar(passwordText);
        bChanged = m_pTextData->SetText(passwordText);
    }
    else {
        bChanged = m_pTextData->SetText(strText);
    }
    if (bChanged && IsInited()) {
        //重新计算字符区域
        Redraw();

        //文本变化时，选择点放到文本末端
        int32_t nTextLen = (int32_t)m_pTextData->GetTextLength();
        InternalSetSel(nTextLen, nTextLen);

        UpdateScrollRange();
        OnTextChanged();
    }
}

void RichEdit2::SetText(const DStringA& strText)
{
    DStringW text = StringConvert::UTF8ToWString(strText);
    SetText(text);
}

void RichEdit2::SetTextNoEvent(const DString& strText)
{
    bool bOldValue = m_bDisableTextChangeEvent;
    m_bDisableTextChangeEvent = true;
    SetText(strText);
    m_bDisableTextChangeEvent = bOldValue;
}

void RichEdit2::SetTextId(const DString& strTextId)
{
    DString strText = GlobalManager::Instance().Lang().GetStringByID(strTextId);
    SetText(strText);
}

bool RichEdit2::GetModify() const
{ 
    return m_bModified;
}

void RichEdit2::SetModify(bool bModified)
{ 
    m_bModified = bModified;
}

void RichEdit2::GetSel(int32_t& nStartChar, int32_t& nEndChar) const
{
    const int32_t nTextLen = GetTextLength();
    nStartChar = m_nSelStartIndex;
    nEndChar = m_nSelEndCharIndex;

    ASSERT((nStartChar <= nTextLen) && (nStartChar >= 0));
    ASSERT((nEndChar <= nTextLen) && (nEndChar >= 0));
    if ((nStartChar < 0) || (nStartChar > nTextLen) || (nEndChar < 0) || (nEndChar > nTextLen)) {
        nStartChar = nTextLen;
        nEndChar = nTextLen;
    }
}

int32_t RichEdit2::SetSel(int32_t nStartChar, int32_t nEndChar)
{
    m_nSelXPos = -1;
    int32_t nSelStartChar = InternalSetSel(nStartChar, nEndChar);
    //确保选择末尾的字符可见
    EnsureCharVisible(nEndChar);
    return nSelStartChar;
}

int32_t RichEdit2::InternalSetSel(int32_t nStartChar, int32_t nEndChar)
{
    if (nStartChar < 0) {
        nStartChar = -1;
    }
    if (nEndChar < 0) {
        nEndChar = -1;
    }

    const int32_t nTextLen = GetTextLength();
    if (nTextLen <= 0) {
        //无选择文本
        nStartChar = 0;
        nEndChar = 0;
    }
    else if ((nStartChar == -1) && (nEndChar == 0)) {
        //无选择文本，获取原光标位置
        nStartChar = m_nSelEndCharIndex;
        nEndChar = m_nSelEndCharIndex;
        if (nStartChar >= nTextLen) {
            nStartChar = nTextLen;
            nEndChar = nTextLen;
        }
    }
    else {
        if (nStartChar < 0) {
            nStartChar = nTextLen;
        }
        if (nEndChar < 0) {
            nEndChar = nTextLen;
        }
        if (nStartChar > nEndChar) {
            std::swap(nStartChar, nEndChar);
        }
        ASSERT(nEndChar >= nStartChar);
        if (nStartChar >= nTextLen) {
            nStartChar = nTextLen;
        }
        if (nEndChar >= nTextLen) {
            nEndChar = nTextLen;
        }
        if (nStartChar == nEndChar) {
            //无选择文本
        }
        else if (nStartChar >= nTextLen) {
            //无选择文本
            nStartChar = nTextLen;
            nEndChar = nTextLen;
        }
        else {
            //有选择文本，选择文本范围，隐藏光标
            ASSERT(nEndChar > nStartChar);
        }
    }

    ASSERT((nStartChar <= nTextLen) && (nStartChar >= 0));
    ASSERT((nEndChar <= nTextLen) && (nEndChar >= 0));

    bool bChanged = (m_nSelStartIndex != nStartChar) || (m_nSelEndCharIndex != nEndChar);
    m_nSelStartIndex = nStartChar;
    m_nSelEndCharIndex = nEndChar;

    //根据选择的文本，调整光标的位置，并控制光标的显示或者隐藏
    SetCaretPos(nEndChar);

    if (bChanged) {
        //重绘(但不能调用Redraw()函数，避免清除绘制缓存)
        Invalidate();

        //触发文本选择变化事件
        SendEvent(kEventSelChanged);
    }
    return nStartChar;
}

void RichEdit2::EnsureCharVisible(int32_t nCharIndex)
{
    UiRect rcDrawRect = GetTextDrawRect(GetRect());
    if (rcDrawRect.IsEmpty()) {
        return;
    }
    UiPoint pt = PosFromChar(nCharIndex);    
    if (!rcDrawRect.ContainsPt(pt)) {
        if (pt.y < rcDrawRect.top) {
            //向上滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cy -= (rcDrawRect.top - pt.y);
            SetScrollPos(scrollPos);
        }
        else if (pt.y >= rcDrawRect.bottom) {
            //向下滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cy += (pt.y - rcDrawRect.bottom);
            scrollPos.cy += m_nRowHeight;
            SetScrollPos(scrollPos);
        }

        if (pt.x < rcDrawRect.left) {
            //向左滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cx -= (rcDrawRect.left - pt.x);
            SetScrollPos(scrollPos);
        }
        else if (pt.x >= rcDrawRect.right) {
            //向右滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cx += (pt.x - rcDrawRect.right);
            scrollPos.cx += m_pTextData->GetCharWidthValue(nCharIndex);
            SetScrollPos(scrollPos);
        }
    }
}

bool RichEdit2::FindRichText(const FindTextParam& findParam, TextCharRange& chrgText) const
{
    DStringW findText = StringConvert::TToWString(findParam.findText);
    if (findText.empty()) {
        return false;
    }
    int32_t nFoundStartChar = -1;
    int32_t nFoundEndChar = -1;
    bool bRet = m_pTextData->FindRichText(findParam.bMatchCase, findParam.bMatchWholeWord, findParam.bFindDown,
                                          findParam.chrg.cpMin, findParam.chrg.cpMax, findText,
                                          nFoundStartChar, nFoundEndChar);
    chrgText.cpMin = nFoundStartChar;
    chrgText.cpMax = nFoundEndChar;
    return bRet;
}

bool RichEdit2::IsRichText() const
{
    return false;
}

DString RichEdit2::GetSelectionTextColor() const
{
    ASSERT(0);
    return DString();
}

void RichEdit2::SetSelectionTextColor(const DString& /*textColor*/)
{
    ASSERT(0);
}

bool RichEdit2::ReplaceSel(const DString& newText, bool bCanUndo)
{
    if (IsReadOnly() || !IsEnabled()) {
        //只读或者Disable状态，禁止编辑
        return false;
    }
    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    GetSel(nStartChar, nEndChar);
    bool bRet = false;
    DStringW text = StringConvert::TToWString(newText);
    if (IsPasswordMode()) {        
        RemoveInvalidPasswordChar(text);
        bRet = m_pTextData->ReplaceText(nStartChar, nEndChar, text, false);
    }
    else {
        bRet = m_pTextData->ReplaceText(nStartChar, nEndChar, text, bCanUndo);
    }
    if (bRet) {
        nEndChar = nStartChar + (int32_t)text.size();
        SetSel(nStartChar, nEndChar);
        UpdateScrollRange();
        OnTextChanged();
    }    
    return bRet;
}

DString RichEdit2::GetSelText() const
{
    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    GetSel(nStartChar, nEndChar);
    DStringW text = m_pTextData->GetTextRange(nStartChar, nEndChar);
#ifdef DUILIB_UNICODE
    return text;
#else
    return StringConvert::WStringToUTF8(text);
#endif
}

bool RichEdit2::HasSelText() const
{
    ASSERT((int32_t)m_pTextData->GetText().size() == GetTextLength());

    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    return (nSelEndChar - nSelStartChar) > 0 ? true : false;
}

int32_t RichEdit2::SetSelAll()
{
    return SetSel(0, -1);
}

void RichEdit2::SetSelNone()
{
    SetSel(-1, 0);
}

DString RichEdit2::GetTextRange(int32_t nStartChar, int32_t nEndChar) const
{
    return StringConvert::WStringToT(m_pTextData->GetTextRange(nStartChar, nEndChar));
}

void RichEdit2::HideSelection(bool bHideSelection)
{
    SetHideSelection(bHideSelection);
}

void RichEdit2::SetHideSelection(bool bHideSelection)
{
    if (m_bHideSelection != bHideSelection) {
        m_bHideSelection = bHideSelection;
        Invalidate();
    }
}

bool RichEdit2::IsHideSelection() const
{
    return m_bHideSelection;
}

void RichEdit2::SetFocusedBottomBorderSize(int32_t nBottomBorderSize)
{
    ASSERT(nBottomBorderSize > 0);
    if (nBottomBorderSize < 0) {
        nBottomBorderSize = 0;
    }
    m_nFocusBottomBorderSize = ui::TruncateToUInt8(nBottomBorderSize);
}

int32_t RichEdit2::GetFocusedBottomBorderSize() const
{
    return (int32_t)(uint32_t)m_nFocusBottomBorderSize;
}

void RichEdit2::SetFocusedBottomBorderColor(const DString& bottomBorderColor)
{
    m_sFocusBottomBorderColor = bottomBorderColor;
}

DString RichEdit2::GetFocusedBottomBorderColor() const
{
    return m_sFocusBottomBorderColor.c_str();
}

bool RichEdit2::CanRedo() const
{
    if (IsReadOnly() || !IsEnabled()) {
        return false;
    }
    return m_pTextData->CanRedo();
}

bool RichEdit2::Redo()
{
    if (!CanRedo()) {
        return false;
    }
    m_nSelXPos = -1;
    int32_t nEndCharIndex = 0;
    bool bRet = m_pTextData->Redo(nEndCharIndex);
    if (bRet) {
        SetSel(nEndCharIndex, nEndCharIndex);
        OnTextChanged();
    }
    return bRet;
}

bool RichEdit2::CanUndo() const
{
    if (IsReadOnly() || !IsEnabled()) {
        return false;
    }
    return m_pTextData->CanUndo();
}

bool RichEdit2::Undo()
{
    if (!CanUndo()) {
        return false;
    }
    m_nSelXPos = -1;
    int32_t nEndCharIndex = 0;
    bool bRet = m_pTextData->Undo(nEndCharIndex);
    if (bRet) {
        SetSel(nEndCharIndex, nEndCharIndex);
        OnTextChanged();
    }
    return bRet;
}

void RichEdit2::Clear()
{
    if (IsReadOnly() || !IsEnabled() || !HasSelText()) {
        return;
    }
    
    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    GetSel(nStartChar, nEndChar);
    if (nEndChar > nStartChar) {
        bool bCanUndo = !IsPasswordMode();
        bool bRet = m_pTextData->ReplaceText(nStartChar, nEndChar, L"", bCanUndo);
        if (bRet) {
            SetSel(nStartChar, nStartChar);
            m_nSelXPos = -1;
            Invalidate();
            UpdateScrollRange();
            OnTextChanged();
        }
    }
}

void RichEdit2::Copy()
{
    if (IsPasswordMode()) {
        //密码模式下，不支持复制
        return;
    }
    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    GetSel(nStartChar, nEndChar);
    DStringW text = m_pTextData->GetTextRange(nStartChar, nEndChar);
    Clipboard::SetClipboardText(text);
}

void RichEdit2::Cut()
{
    if (IsReadOnly() || !IsEnabled() || IsPasswordMode()) {
        //只读模式，密码模式，Disable状态下，不支持剪切
        return;
    }

    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    GetSel(nStartChar, nEndChar);
    DStringW text = m_pTextData->GetTextRange(nStartChar, nEndChar);
    if (!text.empty()) {
        Clipboard::SetClipboardText(text);
        bool bCanUndo = !IsPasswordMode();
        bool bRet = m_pTextData->ReplaceText(nStartChar, nEndChar, L"", bCanUndo);
        if (bRet) {
            SetSel(nStartChar, nStartChar);
            m_nSelXPos = -1;
            Invalidate();
            UpdateScrollRange();
            OnTextChanged();
        }
    }
}

void RichEdit2::Paste()
{
    if (!CanPaste()) {
        return;
    }
    DStringW text;
    Clipboard::GetClipboardText(text);
    if (IsPasswordMode()) {
        RemoveInvalidPasswordChar(text);
    }
    if (!text.empty()) {
        int32_t nStartChar = -1;
        int32_t nEndChar = -1;
        GetSel(nStartChar, nEndChar);
        bool bCanUndo = !IsPasswordMode();
        bool bRet = m_pTextData->ReplaceText(nStartChar, nEndChar, text, bCanUndo);
        if (bRet) {
            int32_t nNewSel = nStartChar + (int32_t)text.size();
            SetSel(nNewSel, nNewSel);
            m_nSelXPos = -1;
            Invalidate();
            UpdateScrollRange();
            OnTextChanged();
        }        
    }
}

bool RichEdit2::CanPaste() const
{
    if (IsReadOnly() || !IsEnabled()) {
        return false;
    }
    if (IsPasteLimited()) {
        return false;
    }
    return true;
}

int32_t RichEdit2::GetLineCount() const
{
    return m_pTextData->GetRowCount();
}

DString RichEdit2::GetLine(int32_t nIndex, int32_t nMaxLength) const
{
    if (nMaxLength < 1) {
        return DString();
    }
    DStringW rowText = m_pTextData->GetRowText(nIndex);
    if ((int32_t)rowText.size() > nMaxLength) {
        rowText.resize((size_t)nMaxLength);
    }
    return StringConvert::WStringToT(rowText);
}

int32_t RichEdit2::LineIndex(int32_t nLine) const
{
    return m_pTextData->RowIndex(nLine);
}

int32_t RichEdit2::LineLength(int32_t nLine) const
{
    return m_pTextData->RowLength(nLine);
}

bool RichEdit2::LineScroll(int32_t nLines)
{
    int32_t nCharIndex = m_pTextData->RowIndex(nLines);
    if (nCharIndex < 0) {
        return false;
    }
    UiPoint pt = PosFromChar(nCharIndex);
    UiRect rcDrawRect = GetTextDrawRect(GetRect());
    if (!rcDrawRect.ContainsPt(pt)) {
        if (pt.y < rcDrawRect.top) {
            //向上滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cy -= (rcDrawRect.top - pt.y);
            SetScrollPos(scrollPos);
        }
        else if (pt.y >= rcDrawRect.bottom) {
            //向下滚动
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cy += (pt.y - rcDrawRect.bottom);
            scrollPos.cy += m_nRowHeight;
            SetScrollPos(scrollPos);
        }
    }
    return true;
}

int32_t RichEdit2::LineFromChar(int32_t nIndex) const
{
    return m_pTextData->RowFromChar(nIndex);
}

void RichEdit2::EmptyUndoBuffer()
{
    m_pTextData->EmptyUndoBuffer();
}

void RichEdit2::SetUndoLimit(uint32_t nLimit)
{
    m_pTextData->SetUndoLimit(nLimit);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
HWND RichEdit2::GetWindowHWND() const
{
    auto window = GetWindow();
    return window ? window->NativeWnd()->GetHWND() : nullptr;
}

#endif

void RichEdit2::OnScrollOffsetChanged(const UiSize& /*oldScrollOffset*/, const UiSize& newScrollOffset)
{
    //滚动条位置变化后，需要重新绘制，但不需要重新计算，以免影响绘制速度
    Invalidate();

    m_pTextData->SetScrollOffset(newScrollOffset);
}

void RichEdit2::SetWindow(Window* pWindow)
{
    BaseClass::SetWindow(pWindow);
    if (pWindow != nullptr) {
        IRender* pRender = pWindow->GetRender();
        ASSERT(pRender != nullptr);
        m_pTextData->SetRender(pRender);
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        m_pTextData->SetRenderFactory(pRenderFactory);
    }
    else {
        m_pTextData->SetRender(nullptr);
        m_pTextData->SetRenderFactory(nullptr);
    }
}

void RichEdit2::LineUp(int32_t deltaValue)
{
    BaseClass::LineUp(deltaValue);
}

void RichEdit2::LineDown(int32_t deltaValue)
{
    BaseClass::LineDown(deltaValue);
}

void RichEdit2::PageUp()
{
    UiSize64 sz = GetScrollPos();
    int32_t iOffset = GetPageScrollDeltaValue(false);
    sz.cy -= iOffset;
    SetScrollPos(sz);
}

void RichEdit2::PageDown()
{
    UiSize64 sz = GetScrollPos();
    int32_t iOffset = GetPageScrollDeltaValue(true);
    sz.cy += iOffset;
    SetScrollPos(sz);
}

void RichEdit2::HomeUp()
{
    BaseClass::HomeUp();
}

void RichEdit2::EndDown(bool arrange)
{
    BaseClass::EndDown(arrange);
}

void RichEdit2::LineLeft(int32_t deltaValue)
{
    BaseClass::LineLeft(deltaValue);
}

void RichEdit2::LineRight(int32_t deltaValue)
{
    BaseClass::LineRight(deltaValue);
}

void RichEdit2::PageLeft()
{
    BaseClass::PageLeft();
}

void RichEdit2::PageRight()
{
    BaseClass::PageRight();
}

void RichEdit2::HomeLeft()
{
    BaseClass::HomeLeft();
}

void RichEdit2::EndRight()
{
    BaseClass::EndRight();
}

void RichEdit2::OnSetEnabled(bool bChanged)
{
    BaseClass::OnSetEnabled(bChanged);
    if (IsEnabled()) {
        SetState(kControlStateNormal);
    }
    else {
        SetState(kControlStateDisabled);
    }
    if (bChanged) {
        Redraw();
    }
}

bool RichEdit2::IsScrollBoxLayoutByActualAreaSize() const
{
    //当处于ScrollBox中时，是否预先计算实际区域大小，然后再按实际区域大小对子控件进行布局
    //返回false，避免每次在ScrollBox::SetPosInternally函数中都调用两次CalcRequiredSize，以减少对性能的影响
    return false;
}

uint32_t RichEdit2::GetControlFlags() const
{
    return IsEnabled() && IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

bool RichEdit2::IsInLimitChars(DStringW::value_type charValue) const
{
    //返回false时：禁止输入
    if (m_pLimitChars == nullptr) {
        return true;
    }
    const DStringW::value_type* ch = m_pLimitChars.get();
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

bool RichEdit2::IsPasteLimited() const
{
    if (m_pLimitChars != nullptr) {
        //有设置限制字符
        DStringW strClipText;
        Clipboard::GetClipboardText(strClipText);
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
        Clipboard::GetClipboardText(strClipText);
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

void RichEdit2::Paint(IRender* pRender, const UiRect& rcPaint)
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("PaintWindow, RichEdit2::Paint");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    if (pRender == nullptr) {
        return;
    }
    
    bool bNeedPaint = true;
    if (pRender->IsClipEmpty()) {
        bNeedPaint = false;
    }    
    UiRect rcTemp; //本控件范围内的脏区域，本次需要绘制的区域
    if (!UiRect::Intersect(rcTemp, rcPaint, GetBoxShadowExpandedRect(GetRect()))) {//如果包含box-shadow的区域内为脏区域，就需要进行绘制
        bNeedPaint = false;
    }

    if (!bNeedPaint) {
        return;
    }

    Control::Paint(pRender, rcPaint);

    UiRect rcDrawText = GetTextDrawRect(GetRect());
    if (rcDrawText.IsEmpty()) {
        //绘制区域为空，不绘制
        return;
    }

    //检查并按需重新计算文本区域
    m_pTextData->CheckCalcTextRects();

    //绘制当前编辑行的背景色
    if (!IsReadOnly() && IsEnabled()) {
        if (IsEmpty()) {
            //文本为空，仅在有焦点时绘制
            if (m_bActive) {
                PaintCurrentRowBkColor(pRender, rcPaint);
            }
        }
        else {
            //文本不为空，始终绘制
            PaintCurrentRowBkColor(pRender, rcPaint);
        }
    }

    //绘制选择背景色
    PaintSelectionColor(pRender, rcPaint);

    //绘制文字
    if (!IsPasswordMode()) {
        //非密码模式，使用绘制缓存来绘制
        std::vector<RichTextData> richTextDataList;
        GetRichTextForDraw(richTextDataList);

        std::shared_ptr<DrawRichTextCache> spDrawRichTextCache = m_pTextData->GetDrawRichTextCache();
        if (spDrawRichTextCache != nullptr) {
            //校验缓存是否失效
            if (!pRender->IsValidDrawRichTextCache(rcDrawText, richTextDataList, spDrawRichTextCache)) {
                spDrawRichTextCache.reset();
                m_pTextData->ClearDrawRichTextCache();
            }
        }

        //绘制文字
        UiSize szScrollOffset = GetScrollOffset();
        if (spDrawRichTextCache != nullptr) {
            //通过缓存绘制
            rcDrawText.Offset(0, m_pTextData->GetTextRectOfssetY());
            pRender->DrawRichTextCacheData(spDrawRichTextCache, rcDrawText, szScrollOffset, m_pTextData->GetTextRowXOffset(), GetAlpha());
        }
        else if (!richTextDataList.empty()) {
            spDrawRichTextCache.reset();

            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            ASSERT(pRenderFactory != nullptr);
            pRender->CreateDrawRichTextCache(rcDrawText, szScrollOffset, pRenderFactory, richTextDataList, spDrawRichTextCache);
            ASSERT(spDrawRichTextCache != nullptr);
            if (spDrawRichTextCache != nullptr) {
                ASSERT(pRender->IsValidDrawRichTextCache(rcDrawText, richTextDataList, spDrawRichTextCache));
                //通过缓存绘制
                rcDrawText.Offset(0, m_pTextData->GetTextRectOfssetY());
                pRender->DrawRichTextCacheData(spDrawRichTextCache, rcDrawText, szScrollOffset, m_pTextData->GetTextRowXOffset(), GetAlpha());
                m_pTextData->SetDrawRichTextCache(spDrawRichTextCache);
            }
        }
    }
    else {
        //密码模式，不使用绘制缓存
        DStringW passwordText = m_pTextData->GetText();
        //处理显示字符
        ReplacePasswordChar(passwordText);

        DString fontId = GetCurrentFontId();
        ASSERT(!fontId.empty());
        if (!passwordText.empty() && !fontId.empty()) {
            UiRect rcDrawRect = GetRichTextDrawRect();
            UiColor dwClrColor = GetUiColor(GetTextColor());
            if (!IsEnabled()) {
                dwClrColor = GetUiColor(GetDisabledTextColor());
            }
            ASSERT(!dwClrColor.IsEmpty());

            DrawStringParam drawParam;
            drawParam.pFont = GetIFontInternal(fontId);
            drawParam.uFormat = GetTextStyle();
            drawParam.textRect = rcDrawRect;
            drawParam.dwTextColor = dwClrColor;

#ifdef DUILIB_UNICODE
            pRender->DrawString(passwordText, drawParam);
#else
            pRender->DrawString(StringConvert::WStringToUTF8(passwordText), drawParam);
#endif
            
        }
    }

    //绘制光标
    PaintCaret(pRender, rcPaint);
}

void RichEdit2::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

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

void RichEdit2::PaintBorder(IRender* pRender)
{
    BaseClass::PaintBorder(pRender);
    if (!IsFocused() || IsReadOnly() || !IsEnabled()) {
        return;
    }
    //绘制下边线
    DString borderColor = GetFocusedBottomBorderColor();
    int32_t borderSize = GetFocusedBottomBorderSize();
    if ((borderSize > 0) && !borderColor.empty()) {
        UiColor dwBorderColor = GetUiColor(borderColor);
        UiRect rcBorder = GetRect();
        float fRoundWidth = 0;
        float fRoundHeight = 0;
        GetBorderRound(fRoundWidth, fRoundHeight);

        float fBottomBorderWidth = Dpi().GetScaleFloat(borderSize);
        rcBorder.right -= int32_t(fRoundWidth + 0.5f);
        rcBorder.left -= int32_t(fRoundWidth + 0.5f);
        UiPointF pt1((float)rcBorder.left, (float)rcBorder.bottom - fBottomBorderWidth / 2);
        UiPointF pt2((float)rcBorder.right, (float)rcBorder.bottom - fBottomBorderWidth / 2);
        DrawBorderLine(pRender, pt1, pt2, fBottomBorderWidth, dwBorderColor, GetBorderDashStyle());
    }
}

void RichEdit2::CreateCaret(int32_t xWidth, int32_t yHeight)
{
    m_iCaretWidth = xWidth;
    m_iCaretHeight = yHeight;
    if (m_iCaretWidth < 0) {
        m_iCaretWidth = 0;
    }
    if (m_iCaretHeight < 0) {
        m_iCaretHeight = 0;
    }
}

void RichEdit2::GetCaretSize(int32_t& xWidth, int32_t& yHeight) const
{
    xWidth = m_iCaretWidth;
    yHeight = m_iCaretHeight;
}

void RichEdit2::ShowCaret(bool fShow)
{
    if (fShow) {
        m_bIsCaretVisible = true;
        m_drawCaretFlag.Cancel();
        std::function<void()> closure = UiBind(&RichEdit2::ChangeCaretVisible, this);
        GlobalManager::Instance().Timer().AddTimer(m_drawCaretFlag.GetWeakFlag(), closure, 500);
    }
    else {
        m_bIsCaretVisible = false;
        m_drawCaretFlag.Cancel();
    }

    if (m_bTextInputMode) {
        //设置输入框的位置
        int32_t xPos = 0;
        int32_t yPos = 0;
        GetCaretPos(xPos, yPos);

        int32_t xWidth = 0;
        int32_t yHeight = 0;
        GetCaretSize(xWidth, yHeight);

        UiRect rc = GetRect();
        UiSize szScrollOffset = GetScrollOffset();
        rc.Offset(-szScrollOffset.cx, -szScrollOffset.cy);

        UiPoint scrollOffset = GetScrollOffsetInScrollBox();
        rc.Offset(-scrollOffset.x, -scrollOffset.y);

        UiRect inputRect;
        inputRect.left = xPos - scrollOffset.x;
        inputRect.top = yPos - scrollOffset.y;
        inputRect.right = inputRect.left + (rc.right - inputRect.left);
        if (inputRect.right <= inputRect.left) {
            inputRect.right = inputRect.left + 2;
        }
        inputRect.bottom = inputRect.top + m_nRowHeight; //高度设置与行高相同
        ASSERT(m_nRowHeight > 0);

        //设置输入区域
        int32_t nCursorOffset = xWidth + Dpi().GetScaleInt(1); //输入法的候选框与光标当前位置的距离（水平方向）, 避免遮盖光标
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            pWindow->NativeWnd()->SetTextInputArea(&inputRect, nCursorOffset);
        }
    }

    Invalidate();
}

void RichEdit2::SetCaretColor(const DString& dwColor)
{
    m_sCaretColor = dwColor;
}

DString RichEdit2::GetCaretColor() const
{
    return m_sCaretColor.c_str();
}

UiRect RichEdit2::GetCaretRect() const
{
    int32_t xPos = 0;
    int32_t yPos = 0;
    GetCaretPos(xPos, yPos);

    int32_t xWidth = 0;
    int32_t yHeight = 0;
    GetCaretSize(xWidth, yHeight);

    UiRect rc = { xPos, yPos, xPos + xWidth, yPos + yHeight };
    return rc;
}

void RichEdit2::SetCaretPos(int32_t xPos, int32_t yPos)
{
    SetCaretPos(UiPoint(xPos, yPos));
}

void RichEdit2::SetCaretPos(const UiPoint& pt)
{
    int32_t nCharPosIndex = CharFromPos(pt);
    SetCaretPos(nCharPosIndex);
}

void RichEdit2::SetCaretPos(int32_t nCharPosIndex)
{
    UiPoint cursorPos = m_pTextData->CaretPosFromChar(nCharPosIndex);
    SetCaretPosInternal(cursorPos.x, cursorPos.y);
}

void RichEdit2::SetCaretPosInternal(int32_t xPos, int32_t yPos)
{
    //光标的坐标是以当前控件的左上角为原点的坐标
    UiSize szScrollOffset = GetScrollOffset();
    xPos += szScrollOffset.cx;
    yPos += szScrollOffset.cy;

    m_iCaretPosX = xPos;
    m_iCaretPosY = yPos;
    ShowCaret(m_bActive && !HasSelText());   
}

void RichEdit2::GetCaretPos(int32_t& xPos, int32_t& yPos) const
{
    xPos = m_iCaretPosX;
    yPos = m_iCaretPosY;

    UiSize szScrollOffset = GetScrollOffset();
    xPos -= szScrollOffset.cx;
    yPos -= szScrollOffset.cy;
}

void RichEdit2::ChangeCaretVisible()
{
    m_bIsCaretVisible = !m_bIsCaretVisible;
    Invalidate();
}

void RichEdit2::PaintCaret(IRender* pRender, const UiRect& /*rcPaint*/)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (IsReadOnly() && m_bNoCaretReadonly) {
        return;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_bIsCaretVisible && !m_bIsComposition) {
#else
    if (m_bIsCaretVisible) {
#endif
        int32_t xPos = 0;
        int32_t yPos = 0;
        GetCaretPos(xPos, yPos);

        int32_t xWidth = 0;
        int32_t yHeight = 0;
        GetCaretSize(xWidth, yHeight);

        UiRect rcDrawText = GetTextDrawRect(GetRect());
        UiRect rcCaret(xPos, yPos, xPos + xWidth, yPos + yHeight);
        if(rcCaret.Intersect(rcDrawText)) {
            //光标在文字显示区域范围内时，绘制光标
            UiColor dwClrColor;
            if (!m_sCaretColor.empty()) {
                dwClrColor = this->GetUiColor(m_sCaretColor.c_str());
            }
            if (dwClrColor.IsEmpty()) {
                //默认光标颜色
                if ((GetWindow() != nullptr) && GetWindow()->IsColorThemeDarkMode()) {
                    //深色主题
                    dwClrColor = UiColor(UiColors::White);
                }
                else {
                    //浅色主题
                    dwClrColor = UiColor(UiColors::Black);
                }
            }
            pRender->DrawLine(UiPointF(xPos + 1, yPos), UiPointF(xPos + 1, yPos + yHeight), dwClrColor, (float)xWidth);
        }
    }
}

void RichEdit2::PaintCurrentRowBkColor(IRender* pRender, const UiRect& /*rcPaint*/)
{
    if (pRender == nullptr) {
        return;
    }
    if (IsHideSelection() && !m_bActive) {
        //当控件处于非激活状态时，隐藏选择内容
        return;
    }

    UiColor currentRowBkColor;
    DString strCurrentRowBkColor;
    if (!m_bActive) {
        //非焦点状态
        strCurrentRowBkColor = GetInactiveCurrentRowBkColor();
    }
    else {
        //焦点状态
        strCurrentRowBkColor = GetCurrentRowBkColor();
    }    
    if (!strCurrentRowBkColor.empty()) {
        currentRowBkColor = GetUiColor(strCurrentRowBkColor);
    }
    if (currentRowBkColor.IsEmpty()) {
        return;
    }

    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    GetSel(nStartChar, nEndChar);
    UiRect rowRect = m_pTextData->GetCharRowRect(nEndChar);
    if (!rowRect.IsEmpty()) {
        pRender->FillRect(UiRectF::MakeFromRect(rowRect), currentRowBkColor);
    }
}

void RichEdit2::PaintSelectionColor(IRender* pRender, const UiRect& /*rcPaint*/)
{
    if (pRender == nullptr) {
        //无需绘制选择背景
        return;
    }
    if (IsHideSelection() && !m_bActive) {
        //当控件处于非激活状态时，隐藏选择内容
        return;
    }

    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    if (nSelStartChar >= nSelEndChar) {
        //无选择文本
        return;
    }

    //每行中选择的矩形范围
    std::map<int32_t, UiRectF> rowTextRectFs;
    m_pTextData->GetCharRangeRects(nSelStartChar, nSelEndChar, rowTextRectFs);
    if(rowTextRectFs.empty()) {
        //无需绘制（无选择文本）
        return;
    }

    UiColor selectionColor;
    DString selectionBkColor;
    if (m_bActive) {
        //焦点状态
        selectionBkColor = GetSelectionBkColor();
    }
    else {
        //非焦点状态
        selectionBkColor = GetInactiveSelectionBkColor();
    }    
    if (!selectionBkColor.empty()) {
        selectionColor = GetUiColor(selectionBkColor);
    }
    if (selectionColor.IsEmpty()) {
        selectionColor = UiColor(UiColors::SkyBlue);
    }

    UiRect rcTemp;
    const UiRect rcDrawText = GetTextDrawRect(GetRect());

    //按行绘制每行的背景
    UiRect rowRect;
    for (auto iter = rowTextRectFs.begin(); iter != rowTextRectFs.end(); ++iter) {
        const UiRectF& rectF = iter->second;
        if (rectF.IsEmpty()) {
            //空行，画一条线
            rowRect.left = (int32_t)rectF.left;
            rowRect.right = (int32_t)ui::CEILF(rectF.right);
            rowRect.top = (int32_t)rectF.top;
            rowRect.bottom = (int32_t)ui::CEILF(rectF.bottom);
            if (rowRect.left == rowRect.right) {
                rowRect.right = rowRect.left + Dpi().GetScaleInt(2);
            }
        }
        else {
            rowRect.left = (int32_t)rectF.left;
            rowRect.right = (int32_t)ui::CEILF(rectF.right);
            rowRect.top = (int32_t)rectF.top;
            rowRect.bottom = (int32_t)ui::CEILF(rectF.bottom);
        }
        if (UiRect::Intersect(rcTemp, rcDrawText, rowRect)) {
            pRender->FillRect(UiRectF::MakeFromRect(rowRect), selectionColor);
        }
    }
}

void RichEdit2::SetPromptMode(bool bPrompt)
{
    if (bPrompt != m_bAllowPrompt) {
        m_bAllowPrompt = bPrompt;
        Invalidate();
    }    
}

bool RichEdit2::AllowPromptMode() const
{
    return m_bAllowPrompt;
}

DString RichEdit2::GetPromptText() const
{
    DString strText = m_sPromptText.c_str();
    if (strText.empty() && !m_sPromptTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringByID(m_sPromptTextId.c_str());
    }
    return strText;
}

void RichEdit2::SetPromptText(const DString& strText)
{
    if (m_sPromptText != strText) {
        m_sPromptText = strText;
        Invalidate();
    }
}

void RichEdit2::SetPromptTextId(const DString& strTextId)
{
    if (m_sPromptTextId != strTextId) {
        m_sPromptTextId = strTextId;
        Invalidate();
    }
}

void RichEdit2::SetPromptTextColor(const DString& promptColor)
{
    if (m_sPromptColor != promptColor) {
        m_sPromptColor = promptColor;
        Invalidate();
    }
}

DString RichEdit2::GetPromptTextColor() const
{
    if (!m_sPromptColor.empty()) {
        return m_sPromptColor.c_str();
    }
    else if (!m_sDisabledTextColor.empty()) {
        return m_sDisabledTextColor.c_str();
    }
    else if (!m_sTextColor.empty()) {
        return m_sTextColor.c_str();
    }
    return DString();
}

void RichEdit2::PaintPromptText(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (!AllowPromptMode()) {
        return;
    }
    DString promptText = GetPromptText();
    if (promptText.empty()) {
        return;
    }
    DString promptTextColor = GetPromptTextColor();
    if (promptTextColor.empty()) {
        return;
    }
    DString fontId = GetCurrentFontId();
    if (fontId.empty()) {
        return;
    }
    if (GetTextLength() != 0) {
        return;
    }

    DrawStringParam drawParam;
    drawParam.pFont = GetIFontInternal(fontId);
    drawParam.uFormat = GetTextStyle();
    drawParam.textRect = GetRichTextDrawRect();
    drawParam.dwTextColor = GetUiColor(promptTextColor);
    pRender->DrawString(promptText, drawParam);
}

DString RichEdit2::GetFocusedImage()
{
    if (m_pFocusedImage != nullptr) {
        return m_pFocusedImage->GetImageString();
    }
    return DString();
}

void RichEdit2::SetFocusedImage( const DString& strImage )
{
    if (m_pFocusedImage == nullptr) {
        m_pFocusedImage = new Image;
    }
    m_pFocusedImage->SetImageString(strImage, Dpi());
    Invalidate();
}

void RichEdit2::PaintStateImages(IRender* pRender)
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
        BaseClass::PaintStateImages(pRender);
        PaintPromptText(pRender);
    }
}

void RichEdit2::SetNoSelOnKillFocus(bool bNoSel)
{
    m_bNoSelOnKillFocus = bNoSel;
}

void RichEdit2::SetSelAllOnFocus(bool bSelAll)
{
    m_bSelAllOnFocus = bSelAll;
}

void RichEdit2::SetNoCaretReadonly()
{
    m_bNoCaretReadonly = true;
}

void RichEdit2::ClearImageCache()
{
    BaseClass::ClearImageCache();
    if (m_pFocusedImage != nullptr) {
        m_pFocusedImage->ClearImageCache();
    }    
}

void RichEdit2::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
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

UiPadding RichEdit2::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void RichEdit2::SetUseControlCursor(bool bUseControlCursor)
{
    m_bUseControlCursor = bUseControlCursor;
}

void RichEdit2::AttachSelChanged(const EventCallback& callback, EventCallbackID callbackID)
{ 
    AttachEvent(kEventSelChanged, callback, callbackID);
}

void RichEdit2::SetZoomPercent(uint32_t nZoomPercent)
{
    ASSERT(nZoomPercent != 0);
    if (nZoomPercent == 0) {
        return;
    }
    if (nZoomPercent > MAX_ZOOM_PERCENT) {
        //限制：最大放大到8倍数
        nZoomPercent = MAX_ZOOM_PERCENT;
    }
    if (m_nZoomPercent != nZoomPercent) {
        uint32_t nOldZoomPercent = m_nZoomPercent;        
        m_nZoomPercent = TruncateToInt16(nZoomPercent);
        uint32_t nNewZoomPercent = m_nZoomPercent;
        OnZoomPercentChanged(nOldZoomPercent, nNewZoomPercent);
    }
}

uint32_t RichEdit2::GetZoomPercent() const
{
    return m_nZoomPercent;
}

void RichEdit2::SetEnableWheelZoom(bool bEnable)
{
    m_bEnableWheelZoom = bEnable;
}

bool RichEdit2::IsEnableWheelZoom(void) const
{
    return m_bEnableWheelZoom;
}

void RichEdit2::SetEnableDefaultContextMenu(bool bEnable)
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

bool RichEdit2::IsEnableDefaultContextMenu() const
{
    return m_bEnableDefaultContextMenu;
}

void RichEdit2::ShowPopupMenu(const ui::UiPoint& point)
{
    RichEdit2* pRichEdit = this;
    if ((pRichEdit == nullptr) || !pRichEdit->IsEnabled() || pRichEdit->IsPasswordMode()) {
        return;
    }

    //如果没有选中文本，则将光标切换到当前点击的位置
    int32_t nStartChar = 0; 
    int32_t nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    if (nStartChar == nEndChar) {
        int32_t pos = m_pTextData->CharFromPos(point);
        if (pos >= 0) {
            pRichEdit->SetSel(pos, pos);
            pRichEdit->GetSel(nStartChar, nEndChar);
        }
    }
    
    DString skinFolder = DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/");
    Menu* menu = new Menu(GetWindow());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(skinFolder);
    DString xml(_T("rich_edit_menu.xml"));

    //菜单显示过程中，不隐藏当前选择的文本
    bool bOldHideSelection = IsHideSelection();
    SetHideSelection(false);

    //菜单关闭事件
    std::weak_ptr<WeakFlag> richEditFlag = GetWeakFlag();
    menu->AttachWindowCloseMsg([this, richEditFlag, bOldHideSelection](const ui::EventArgs&) {
        if (!richEditFlag.expired()) {
            //恢复HideSelection属性
            SetHideSelection(bOldHideSelection);
        }
        return true;
        });

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

void RichEdit2::OnTextChanged()
{
    //设置已修改标志
    SetModify(true);
    if (!m_bDisableTextChangeEvent) {
        SendEvent(kEventTextChanged);
    }
}

bool RichEdit2::SetSpinClass(const DString& spinClass)
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

bool RichEdit2::SetEnableSpin(bool bEnable, const DString& spinClass, int32_t nMin, int32_t nMax)
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

int64_t RichEdit2::GetTextNumber() const
{
    DString text = GetText();
    if (text.empty()) {
        return 0;
    }
    int64_t n = StringUtil::StringToInt64(text);
    return n;
}

void RichEdit2::SetTextNumber(int64_t nValue)
{
    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    if (!m_numberFormat.empty()) {
        SetText(StringUtil::Printf(m_numberFormat.c_str(), nValue));
    }
    else {
        SetText(StringUtil::Int64ToString(nValue));
    }
    if ((nSelStartChar == nSelEndChar) && (nSelStartChar >= 0) && (nSelStartChar <= GetTextLength())) {
        SetSel(nSelStartChar, nSelStartChar);
    }
}

void RichEdit2::AdjustTextNumber(int32_t nDelta)
{
    ASSERT(IsNumberOnly());
    if (IsNumberOnly()) {
        const int64_t nOldValue = GetTextNumber();
        int64_t nNewValue = nOldValue + nDelta;
        if (((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX))) {
            if (nNewValue > GetMaxNumber()) {
                //超过最大数字，进行修正
                nNewValue = GetMaxNumber();
                if ((nDelta == 1) && (nOldValue == GetMaxNumber())) {
                    //循环
                    nNewValue = GetMinNumber();
                }
            }
            else if (nNewValue < GetMinNumber()) {
                //小于最小数字，进行修正
                nNewValue = GetMinNumber();
                if ((nDelta == -1) && (nOldValue == GetMinNumber())) {
                    //循环
                    nNewValue = GetMaxNumber();
                }
            }
        }
        if (nNewValue != nOldValue) {
            SetTextNumber(nNewValue);
        }
    }
}

void RichEdit2::StartAutoAdjustTextNumberTimer(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit2::StartAutoAdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 1000, 1);
    }
}

void RichEdit2::StartAutoAdjustTextNumber(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit2::AdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 120);
    }
}

void RichEdit2::StopAutoAdjustTextNumber()
{
    m_flagAdjustTextNumber.Cancel();
}

void RichEdit2::SetReplaceNewline(bool bReplaceNewline)
{
    m_bReplaceNewline = bReplaceNewline;
}

bool RichEdit2::IsReplaceNewline() const
{
    return m_bReplaceNewline;
}

void RichEdit2::SetClearBtnClass(const DString& btnClass)
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

void RichEdit2::SetShowPasswordBtnClass(const DString& btnClass)
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

void RichEdit2::SetTextHAlignType(HorAlignType alignType)
{
    if (m_pTextData->GetHAlignType() != alignType) {
        m_pTextData->SetTextHAlignType(alignType);
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

HorAlignType RichEdit2::GetHAlignType() const
{
    return m_pTextData->GetHAlignType();
}

void RichEdit2::SetTextVAlignType(VerAlignType alignType)
{
    if (m_pTextData->GetVAlignType() != alignType) {
        m_pTextData->SetTextVAlignType(alignType);
        //清除绘制缓存，并重绘
        ClearCacheAndRedraw();
    }
}

VerAlignType RichEdit2::GetVAlignType() const
{
    return m_pTextData->GetVAlignType();
}

UiPoint RichEdit2::PosFromChar(int32_t lChar) const
{
    return m_pTextData->PosFromChar(lChar);
}

int32_t RichEdit2::CharFromPos(UiPoint pt)
{
    pt.Offset(GetScrollOffsetInScrollBox());
    return m_pTextData->CharFromPos(pt);
}

uint16_t RichEdit2::GetTextStyle() const
{
    uint32_t uTextStyle = 0;
    HorAlignType hAlignType = GetHAlignType();
    if (hAlignType == HorAlignType::kAlignCenter) {
        uTextStyle |= TEXT_HCENTER;
    }
    else if (hAlignType == HorAlignType::kAlignRight) {
        uTextStyle |= TEXT_RIGHT;
    }
    else {
        uTextStyle |= TEXT_LEFT;
    }

    VerAlignType vAlignType = GetVAlignType();
    if (vAlignType == VerAlignType::kAlignCenter) {
        uTextStyle |= TEXT_VCENTER;
    }
    else if (vAlignType == VerAlignType::kAlignBottom) {
        uTextStyle |= TEXT_BOTTOM;
    }
    else {
        uTextStyle |= TEXT_TOP;
    }

    //密码模式，不进行自动换行
    if (IsWordWrap() && !IsPasswordMode()) {
        uTextStyle |= TEXT_WORD_WRAP;
    }

    //单行/多行属性(不设置属性则为多行模式)
    if (!IsMultiLine()) {
        uTextStyle |= TEXT_SINGLELINE;        
    }
    return ui::TruncateToUInt16(uTextStyle);
}

bool RichEdit2::GetRichTextForDraw(std::vector<RichTextData>& richTextDataList) const
{
    std::vector<std::wstring_view> textView;
    m_pTextData->GetTextView(textView);
    GetRichTextForDraw(textView, richTextDataList);
    return !richTextDataList.empty();
}

bool RichEdit2::GetRichTextForDraw(const std::vector<std::wstring_view>& textView,
                                  std::vector<RichTextData>& richTextDataList,
                                  size_t nStartLine,
                                  const std::vector<size_t>& modifiedLines) const
{
    if (nStartLine != (size_t)-1) {
        if (!modifiedLines.empty()) {
            ASSERT(modifiedLines[0] == nStartLine);
            if (modifiedLines[0] != nStartLine) {
                return false;
            }
        }
    }
    richTextDataList.clear();
    if (textView.empty()) {
        return false;
    }
    DString sFontId = GetCurrentFontId();
    ASSERT(!sFontId.empty());
    IFont* pFont = GetIFontInternal(sFontId);
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return false;
    }
    RichTextData richTextData;    
    //默认文本属性
    richTextData.m_textStyle = GetTextStyle();
    //默认文本颜色
    if (!IsEnabled()) {
        richTextData.m_textColor = GetUiColor(GetDisabledTextColor());
    }
    else {
        richTextData.m_textColor = GetUiColor(GetTextColor());
    }
    if (richTextData.m_textColor.IsEmpty()) {
        richTextData.m_textColor = UiColor(UiColors::Black);
    }
    //文本的字体信息
    richTextData.m_pFontInfo.reset(new UiFontEx);
    richTextData.m_pFontInfo->m_fontName = pFont->FontName();
    richTextData.m_pFontInfo->m_fontSize = pFont->FontSize();
    richTextData.m_pFontInfo->m_bBold = pFont->IsBold();
    richTextData.m_pFontInfo->m_bUnderline = pFont->IsUnderline();
    richTextData.m_pFontInfo->m_bItalic = pFont->IsItalic();
    richTextData.m_pFontInfo->m_bStrikeOut = pFont->IsStrikeOut();

    //行间距倍数
    richTextData.m_fRowSpacingMul = GetRowSpacingMul();
    richTextData.m_fRowSpacingAdd = GetRowSpacingAdd();

    if (nStartLine != (size_t)-1) {
        //增量绘制，只绘制变化的部分
        size_t nLineIndex = 0;
        const size_t nCount = modifiedLines.size();
        richTextDataList.reserve(nCount);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            nLineIndex = modifiedLines[nIndex];
            ASSERT(nLineIndex < textView.size());
            if (nLineIndex < textView.size()) {
                ASSERT(!textView[nLineIndex].empty());
                if (!textView[nLineIndex].empty()) {
                    richTextData.m_textView = textView[nLineIndex];
                    richTextDataList.emplace_back(richTextData);
                }
            }
            else {
                return false;
            }
        }
    }
    else {
        //全部绘制
        const size_t nCount = textView.size();
        richTextDataList.reserve(nCount);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            ASSERT(!textView[nIndex].empty());
            if (!textView[nIndex].empty()) {
                richTextData.m_textView = textView[nIndex];
                richTextDataList.emplace_back(richTextData);
            }
        }
    }    
    return !richTextDataList.empty();
}

UiRect RichEdit2::GetRichTextDrawRect() const
{
    return GetTextDrawRect(GetRect());
}

uint8_t RichEdit2::GetDrawAlpha() const
{
    return GetAlpha();
}

void RichEdit2::OnTextRectsChanged()
{
    //更新光标的位置
    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    if (nSelStartChar == nSelEndChar) {
        SetCaretPos(nSelStartChar);
        EnsureCharVisible(nSelStartChar);
    }

    //更新滚动条的范围
    UpdateScrollRange();
}

int32_t RichEdit2::GetTextRowHeight() const
{
    return m_nRowHeight;
}

int32_t RichEdit2::GetTextCaretWidth() const
{
    return m_iCaretWidth;
}

bool RichEdit2::IsTextPasswordMode() const
{
    return IsPasswordMode();
}

UiSize RichEdit2::EstimateText(UiSize szAvailable)
{
    UiSize fixedSize;
    IRender* pRender = nullptr;
    if (GetWindow() != nullptr) {
        pRender = GetWindow()->GetRender();
    }
    if (pRender == nullptr) {
        return fixedSize;
    }

    int32_t nWidth = szAvailable.cx;
    if (GetFixedWidth().IsStretch()) {
        //如果是拉伸类型，使用外部宽度
        nWidth = CalcStretchValue(GetFixedWidth(), szAvailable.cx);
    }
    else if (GetFixedWidth().IsInt32()) {
        nWidth = GetFixedWidth().GetInt32();
    }
    else if (GetFixedWidth().IsAuto()) {
        //宽度为自动时，不限制宽度
        nWidth = GetMaxWidth();
    }

    //最大高度，不限制
    int32_t nHeight = INT_MAX;

    UiRect rc;
    rc.left = 0;
    rc.right = rc.left + nWidth;
    rc.top = 0;
    rc.bottom = rc.top + nHeight;

    const UiPadding rcTextPadding = GetTextPadding();
    const UiPadding rcPadding = GetControlPadding();
    if (nWidth != INT_MAX) {
        rc.left += (rcPadding.left + rcTextPadding.left);
        rc.right -= (rcPadding.right + rcTextPadding.right);
    }
    if (nHeight != INT_MAX) {
        rc.top += (rcPadding.top + rcTextPadding.top);
        rc.bottom -= (rcPadding.bottom + rcTextPadding.bottom);
    }
    if (rc.IsEmpty()) {
        return fixedSize;
    }

    //计算绘制所占的区域大小
    UiRect rect = m_pTextData->EstimateTextDisplayBounds(rc);

    fixedSize.cx = rect.Width();
    if (fixedSize.cx > 0) {
        fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
        fixedSize.cx += (rcPadding.left + rcPadding.right);
    }

    fixedSize.cy = rect.Height();
    if ((fixedSize.cy == 0) && GetFixedHeight().IsAuto() && (GetTextLength() == 0)) {
        //文本为空，并且高度为"auto"时，设置高度为行高，与Windows版本的保持一致
        fixedSize.cy = m_nRowHeight > 0 ? m_nRowHeight : 0;
    }
    if (fixedSize.cy > 0) {
        fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
        fixedSize.cy += (rcPadding.top + rcPadding.bottom);
    }
    return fixedSize;
}

UiRect RichEdit2::GetTextDrawRect(const UiRect& rc) const
{
    UiRect rcAvailable = rc;
    rcAvailable.Deflate(GetTextPadding());
    rcAvailable.Deflate(GetControlPadding());

    //按照有滚动条的状态进行估算，避免需要二次估算才能估算出最终结果
    if (!GetScrollBarFloat() && (GetVScrollBar() != nullptr)) {
        if (IsVScrollBarAtLeft()) {
            rcAvailable.left += GetVScrollBar()->GetFixedWidth().GetInt32();
        }
        else {
            rcAvailable.right -= GetVScrollBar()->GetFixedWidth().GetInt32();
        }
    }
    if (!GetScrollBarFloat() && (GetHScrollBar() != nullptr)) {
        rcAvailable.bottom -= GetHScrollBar()->GetFixedHeight().GetInt32();
    }
    rcAvailable.Validate();
    return rcAvailable;
}

UiSize64 RichEdit2::CalcRequiredSize(const UiRect& rc, bool bEstimateOnly)
{
    //计算子控件的大小
    UiSize64 requiredSize = BaseClass::CalcRequiredSize(rc, bEstimateOnly);
    if (requiredSize.cx > rc.Width()) {
        requiredSize.cx = 0;
    }
    else if (requiredSize.cy > rc.Height()) {
        requiredSize.cy = 0;
    }

    //评估文本区域：当前控件区域减去内边距，减去滚动条所占的宽度或者高度
    UiRect rcAvailable = GetTextDrawRect(rc);
    UiSize szAvailable(rcAvailable.Width(), rcAvailable.Height());

    //估算图片区域大小
    UiSize imageSize = EstimateImage(szAvailable, EstimateImageType::kBoth);
    if (imageSize.cx > rc.Width()) {
        imageSize.cx = 0;
    }
    else if (imageSize.cy > rc.Height()) {
        imageSize.cy = 0;
    }

    //估算文本区域大小, 函数计算时，已经包含了内边距
    UiSize textSize = EstimateText(UiSize(rc.Width(), rc.Height()));
    //文本区域，需要包含滚动条的宽度和高度(仅当滚动条设置为非浮动时)
    if (!GetScrollBarFloat() && (GetVScrollBar() != nullptr) && GetVScrollBar()->IsValid()) {
        if (IsVScrollBarAtLeft()) {
            textSize.cx += GetVScrollBar()->GetFixedWidth().GetInt32();
        }
        else {
            textSize.cx += GetVScrollBar()->GetFixedWidth().GetInt32();
        }
    }
    if (!GetScrollBarFloat() && (GetHScrollBar() != nullptr) && GetHScrollBar()->IsValid()) {
        textSize.cy += GetHScrollBar()->GetFixedHeight().GetInt32();
    }

    UiSize szControlSize;
    szControlSize.cx = std::max(imageSize.cx, textSize.cx);
    szControlSize.cy = std::max(imageSize.cy, textSize.cy);

    //以文本的大小为准，子控件或者背景图的大小，不影响控件是否出现滚动条
    UiSize64 szSize = requiredSize;
    if (szSize.cx < szControlSize.cx) {
        szSize.cx = szControlSize.cx;
    }
    if (szSize.cy < szControlSize.cy) {
        szSize.cy = szControlSize.cy;
    }
    return szSize;
}

void RichEdit2::Redraw()
{
    m_nSelXPos = -1;
    m_pTextData->ClearDrawRichTextCache();
    Invalidate();
}

void RichEdit2::ClearCacheAndRedraw()
{
    m_pTextData->SetCacheDirty(true);
    Redraw();
    UpdateScrollRange();
}

////////////////////////////////////////////////////////////
bool RichEdit2::OnSetCursor(const EventArgs& msg)
{
    if (m_bUseControlCursor) {
        //使用Control设置的光标
        return BaseClass::OnSetCursor(msg);
    }
    if (!IsEnabled()) {
        //未启用状态下，使用默认光标
        return BaseClass::OnSetCursor(msg);
    }

    SetCursor(IsReadOnly() ? CursorType::kCursorArrow : CursorType::kCursorIBeam);
    return true;
}

bool RichEdit2::OnSetFocus(const EventArgs& /*msg*/)
{
    m_bActive = true;
    //获得焦点时，打开输入法
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        bool bEnableIME = IsVisible() && !IsReadOnly() && IsEnabled();
        pWindow->NativeWnd()->SetImeOpenStatus(bEnableIME);
        if (bEnableIME) {
            UiRect inputRect = GetRect();
            UiPoint scrollOffset = GetScrollOffsetInScrollBox();
            inputRect.Offset(-scrollOffset.x, -scrollOffset.y);
            pWindow->NativeWnd()->SetTextInputArea(&inputRect, 0);
        }
    }

    if ((pWindow != nullptr) && IsVisible() && !IsReadOnly() && IsEnabled()) {
        m_bTextInputMode = true;
    }
    else {
        m_bTextInputMode = false;
    }

    //设置是否显示光标
    ShowCaret(!HasSelText());

    if ((m_pClearButton != nullptr) && !IsReadOnly()) {
        m_pClearButton->SetFadeVisible(true);
    }
    if ((m_pShowPasswordButton != nullptr) && IsPasswordMode() && !IsShowPassword()) {
        m_pShowPasswordButton->SetFadeVisible(true);
    }

    //不调用基类的方法
    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHovered);
    }

    Invalidate();
    return true;
}

bool RichEdit2::OnKillFocus(const EventArgs& msg)
{
    m_bActive = false;
    ShowCaret(false);
    m_bSelAllEver = false;
    if (m_bNoSelOnKillFocus && IsReadOnly() && IsEnabled()) {
        SetSelNone();
    }
    m_bTextInputMode = false;

    if (m_pClearButton != nullptr) {
        m_pClearButton->SetFadeVisible(false);
    }
    if (m_pShowPasswordButton != nullptr) {
        m_pShowPasswordButton->SetFadeVisible(false);
    }
    return BaseClass::OnKillFocus(msg);
}

void RichEdit2::CheckSelAllOnFocus()
{
    if (IsEnabled() && !m_bSelAllEver) {
        m_bSelAllEver = true;
        if (m_bSelAllOnFocus) {
            SetSelAll();
            if (IsMultiLine()) {
                HomeUp();
            }
            else {
                HomeLeft();
            }
        }
    }
}

bool RichEdit2::OnImeStartComposition(const EventArgs& /*msg*/)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    HWND hWnd = GetWindowHWND();
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

    int32_t xPos = 0;
    int32_t yPos = 0;
    GetCaretPos(xPos, yPos);

    pt.x = xPos - ptScrollOffset.x;
    pt.y = yPos - ptScrollOffset.y;

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
#endif
    return true;
}

bool RichEdit2::OnImeEndComposition(const EventArgs& /*msg*/)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_bIsComposition = false;
#endif
    return true;
}

void RichEdit2::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    if (msg.eventType == kEventKeyDown) {
        //截获基类的KeyDown事件，优先处理
        if (OnKeyDown(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseWheel) {
        bool bCtrlDown = IsKeyDown(msg, ModifierKey::kControl);
        if (bCtrlDown && IsEnableWheelZoom()) {
            //Ctrl + 滚轮，调整缩放比
            OnMouseWheel(msg.eventData, bCtrlDown);
            return;
        }
    }
    BaseClass::HandleEvent(msg);
}

bool RichEdit2::OnKeyDown(const EventArgs& msg)
{
    //该函数实现支持的各种快捷键
    if (msg.vkCode == kVK_SHIFT) {
        //记录选择的起始位置(当Shift键一致按住的时候，就会一致触发此OnKeyDown事件)
        if (m_nShiftStartIndex == -1) {
            int32_t nSelEnd = 0;
            GetSel(m_nShiftStartIndex, nSelEnd);
        }
    }
    if (msg.vkCode == kVK_CONTROL) {
        //记录选择的起始位置(当Ctrl键一致按住的时候，就会一致触发此OnKeyDown事件)
        if (m_nCtrlStartIndex == -1) {
            int32_t nSelEnd = 0;
            GetSel(m_nCtrlStartIndex, nSelEnd);
        }
    }
    if (OnCtrlArrowKeyDownScrollView(msg)) {
        return true;
    }
    else if (OnArrowKeyDown(msg)) {
        return true;
    }
    else if ((msg.vkCode == kVK_RETURN) || (msg.vkCode == kVK_TAB) ||
             (msg.vkCode == kVK_DELETE) || (msg.vkCode == kVK_BACK)) {
        //在KeyDown的时候，处理回车键, TAB键, 删除键，退格键
        OnInputChar(msg);
    }
    else if (msg.vkCode == kVK_ESCAPE) {
        //ESC键
        SendEvent(kEventEsc);
    }
    else if ((msg.vkCode == 'A') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + A: 全选
        SetSelAll();
    }
    else if ((msg.vkCode == 'C') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + C, 复制
        Copy();
    }
    else if ((msg.vkCode == kVK_INSERT) && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + Insert, 复制
        Copy();
    }
    else if ((msg.vkCode == 'X') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + X, 剪切
        Cut();
    }
    else if ((msg.vkCode == 'V') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + V, 粘贴
        Paste();
    }
    else if ((msg.vkCode == kVK_INSERT) && IsKeyDown(msg, ModifierKey::kShift)) {
        //Shift + Insert, 粘贴
        Paste();
    }
    else if ((msg.vkCode == 'Z') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + Z, 撤销
        Undo();
    }
    else if ((msg.vkCode == 'Y') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + Y, 重做
        Redo();
    }
    else if ((msg.vkCode == 'W') && IsKeyDown(msg, ModifierKey::kControl)) {
        //Ctrl + W, 切换自动换行
        if (IsMultiLine() && !IsPasswordMode() && IsEnabled()) {
            SetWordWrap(!IsWordWrap());
        }
    }
    return true;
}

bool RichEdit2::OnCtrlArrowKeyDownScrollView(const EventArgs& msg)
{
    bool bCtrlArrowKeyDown = false;
    if (IsKeyDown(msg, ModifierKey::kControl)) {
        if ((msg.vkCode == kVK_DOWN) || (msg.vkCode == kVK_UP) ||
            (msg.vkCode == kVK_NEXT) || (msg.vkCode == kVK_PRIOR) ||
            (msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END)) {
            bCtrlArrowKeyDown = true;
        }
    }
    if (!bCtrlArrowKeyDown) {
        return false;
    }

    if (msg.vkCode == kVK_HOME) {
        //Ctrl + Home
        InternalSetSel(0, 0);
        m_nSelXPos = -1;
    }
    else if (msg.vkCode == kVK_END) {
        //Ctrl + End
        int32_t nTextLen = GetTextLength();
        InternalSetSel(nTextLen, nTextLen);
        m_nSelXPos = -1;
    }
    //按住Ctrl+方向键，触发ScrollBox的功能
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid() && pVScrollBar->IsEnabled()) {
        switch (msg.vkCode) {
        case kVK_DOWN:
            LineDown(GetLineScrollDeltaValue(true));
            break;
        case kVK_UP:
            LineUp(GetLineScrollDeltaValue(false));
            break;
        case kVK_NEXT:
            PageDown();
            break;
        case kVK_PRIOR:
            PageUp();
            break;
        case kVK_HOME:
            HomeUp();
            break;
        case kVK_END:
            EndDown(false);
            break;
        default:
            break;
        }
    }
    else if ((pHScrollBar != nullptr) && pHScrollBar->IsValid() && pHScrollBar->IsEnabled()) {
        switch (msg.vkCode) {
        case kVK_DOWN:
            LineRight();
            break;
        case kVK_UP:
            LineLeft();
            break;
        case kVK_NEXT:
            PageRight();
            break;
        case kVK_PRIOR:
            PageLeft();
            break;
        case kVK_HOME:
            HomeLeft();
            break;
        case kVK_END:
            EndRight();
            break;
        default:
            break;
        }
    }
    return true;
}

bool RichEdit2::OnArrowKeyDown(const EventArgs& msg)
{
    bool bArrowKeyDown = false;
    if ((msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT) ||
        (msg.vkCode == kVK_DOWN) || (msg.vkCode == kVK_UP)    ||
        (msg.vkCode == kVK_NEXT) || (msg.vkCode == kVK_PRIOR) ||
        (msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END)) {
        bArrowKeyDown = true;
    }
    if (!bArrowKeyDown) {
        return false;
    }
    if (IsNumberOnly() && !IsReadOnly() && IsEnabled() && ((msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN))) {
        //数字模式，方向键调整数字大小
        if (msg.vkCode == kVK_UP) {
            AdjustTextNumber(1);
        }
        else {
            AdjustTextNumber(-1);
        }        
        return true;
    }

    if (msg.vkCode == kVK_LEFT) {
        //Left键
        m_nSelXPos = -1;
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        bool bCtrlDown = IsKeyDown(msg, ModifierKey::kControl);
        if (bShiftDown && (m_nShiftStartIndex != -1)) {
            //Shift + Left键
            if (nSelEndChar <= m_nShiftStartIndex) {
                nSelStartChar = m_pTextData->GetPrevValidCharIndex(nSelStartChar);
                nSelEndChar = m_nShiftStartIndex;
                m_bSelForward = false;
            }
            else {
                nSelStartChar = m_nShiftStartIndex;
                nSelEndChar = m_pTextData->GetPrevValidCharIndex(nSelEndChar);
                m_bSelForward = true;
            }
            InternalSetSel(nSelStartChar, nSelEndChar);
            EnsureCharVisible(nSelStartChar);
        }
        else if (bCtrlDown && (m_nCtrlStartIndex != -1)) {
            //Ctrl + Left键
            if (nSelEndChar <= m_nCtrlStartIndex) {
                nSelStartChar = m_pTextData->GetPrevValidWordIndex(nSelStartChar);
                nSelEndChar = m_nCtrlStartIndex;
                m_bSelForward = false;
            }
            else {
                nSelStartChar = m_nCtrlStartIndex;
                nSelEndChar = m_pTextData->GetPrevValidWordIndex(nSelEndChar);
                m_bSelForward = true;
            }
            InternalSetSel(nSelStartChar, nSelEndChar);
            EnsureCharVisible(nSelStartChar);
        }
        else {
            //Left键
            nSelEndChar = m_pTextData->GetPrevValidCharIndex(nSelEndChar);
            InternalSetSel(nSelEndChar, nSelEndChar);            
            EnsureCharVisible(nSelEndChar);
            m_bSelForward = true;
        }
    }
    else if (msg.vkCode == kVK_RIGHT) {
        m_nSelXPos = -1;
        //Right键
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        bool bCtrlDown = IsKeyDown(msg, ModifierKey::kControl);
        if (bShiftDown && (m_nShiftStartIndex != -1)) {
            //Shift + Right键
            if (nSelEndChar <= m_nShiftStartIndex) {
                nSelStartChar = m_pTextData->GetNextValidCharIndex(nSelStartChar);
                nSelEndChar = m_nShiftStartIndex;
                m_bSelForward = false;
            }
            else {
                nSelStartChar = m_nShiftStartIndex;
                nSelEndChar = m_pTextData->GetNextValidCharIndex(nSelEndChar);
                m_bSelForward = true;
            }
            InternalSetSel(nSelStartChar, nSelEndChar);
            EnsureCharVisible(nSelEndChar);
        }
        else if (bCtrlDown && (m_nCtrlStartIndex != -1)) {
            //Ctrl + Right键
            if (nSelEndChar <= m_nCtrlStartIndex) {
                nSelStartChar = m_pTextData->GetNextValidWordIndex(nSelStartChar);
                nSelEndChar = m_nCtrlStartIndex;
                m_bSelForward = false;
            }
            else {
                nSelStartChar = m_nCtrlStartIndex;
                nSelEndChar = m_pTextData->GetNextValidWordIndex(nSelEndChar);
                m_bSelForward = true;
            }
            InternalSetSel(nSelStartChar, nSelEndChar);
            EnsureCharVisible(nSelEndChar);
        }
        else {
            //Right键
            nSelEndChar = m_pTextData->GetNextValidCharIndex(nSelEndChar);
            InternalSetSel(nSelEndChar, nSelEndChar);
            EnsureCharVisible(nSelEndChar);
            m_bSelForward = true;
        }
    }
    else if (msg.vkCode == kVK_DOWN) {
        //Down键
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        ASSERT(m_nRowHeight > 0);
        UiPoint pt = PosFromChar(nSelEndChar);
        if (m_nSelXPos == -1) {
            m_nSelXPos = pt.x;
        }
        else {
            pt.x = m_nSelXPos;
        }
        pt.y += m_nRowHeight;
        nSelEndChar = CharFromPos(pt);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown) {
            //Shift + Down键
            InternalSetSel(nSelStartChar, nSelEndChar);
        }
        else {
            InternalSetSel(nSelEndChar, nSelEndChar);
        }
        EnsureCharVisible(nSelEndChar);
        m_bSelForward = true;
    }
    else if (msg.vkCode == kVK_UP) {
        //Up键
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        ASSERT(m_nRowHeight > 0);
        UiPoint pt = PosFromChar(nSelStartChar);
        if (m_nSelXPos == -1) {
            m_nSelXPos = pt.x;
        }
        else {
            pt.x = m_nSelXPos;
        }
        pt.y -= m_nRowHeight;
        nSelStartChar = CharFromPos(pt);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown) {
            //Shift + Up键
            InternalSetSel(nSelStartChar, nSelEndChar);
        }
        else {
            InternalSetSel(nSelStartChar, nSelStartChar);
        }
        EnsureCharVisible(nSelStartChar);
        m_bSelForward = false;
    }
    else if (msg.vkCode == kVK_NEXT) {
        //PageDown键
        int32_t nPageHeight = GetPageScrollDeltaValue(true);
        ASSERT(nPageHeight > 0);

        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        UiPoint pt = PosFromChar(nSelEndChar);
        if (m_nSelXPos == -1) {
            m_nSelXPos = pt.x;
        }
        else {
            pt.x = m_nSelXPos;
        }
        pt.y += nPageHeight;
        nSelEndChar = CharFromPos(pt);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown) {
            //Shift + PageDown键
            InternalSetSel(nSelStartChar, nSelEndChar);
        }
        else {
            InternalSetSel(nSelEndChar, nSelEndChar);
        }
        EnsureCharVisible(nSelEndChar);
        m_bSelForward = true;
    }
    else if (msg.vkCode == kVK_PRIOR) {
        //PageUp键
        int32_t nPageHeight = GetPageScrollDeltaValue(false);
        ASSERT(nPageHeight > 0);

        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        UiPoint pt = PosFromChar(nSelStartChar);
        if (m_nSelXPos == -1) {
            m_nSelXPos = pt.x;
        }
        else {
            pt.x = m_nSelXPos;
        }
        pt.y -= nPageHeight;
        nSelStartChar = CharFromPos(pt);
        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown) {
            //Shift + PageUp键
            InternalSetSel(nSelStartChar, nSelEndChar);
        }
        else {
            InternalSetSel(nSelStartChar, nSelStartChar);
        }
        EnsureCharVisible(nSelStartChar);
        m_bSelForward = false;
    }
    else if (msg.vkCode == kVK_HOME) {
        m_nSelXPos = -1;
        //HOME键
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown && (m_nShiftStartIndex != -1)) {
            //Shift + Home 键
            int32_t nShiftRowStartIndex = m_pTextData->GetRowStartCharIndex(m_nShiftStartIndex);
            int32_t nRowStartIndex = m_pTextData->GetRowStartCharIndex(nSelStartChar);
            int32_t nRowStartIndex2 = m_pTextData->GetRowStartCharIndex(nSelEndChar);
            if (nRowStartIndex == nRowStartIndex2) {
                //在同一行, 不需要区分操作方向
                if (nShiftRowStartIndex == nRowStartIndex) {
                    InternalSetSel(nRowStartIndex, m_nShiftStartIndex);
                }
                else {
                    InternalSetSel(nRowStartIndex, nSelEndChar);
                }
                EnsureCharVisible(nRowStartIndex);
            }
            else {
                //在不同行
                if (m_bSelForward) {
                    //操作方向：向前
                    nSelEndChar = m_pTextData->GetRowStartCharIndex(nSelEndChar);
                    InternalSetSel(nSelStartChar, nSelEndChar);
                }
                else {
                    //操作方向：向后
                    nSelStartChar = m_pTextData->GetRowStartCharIndex(nSelStartChar);
                    InternalSetSel(nSelStartChar, nSelEndChar);
                }
                EnsureCharVisible(nSelStartChar);
            }
        }
        else {
            //Home 键
            nSelStartChar = m_pTextData->GetRowStartCharIndex(nSelStartChar);
            InternalSetSel(nSelStartChar, nSelStartChar);
            EnsureCharVisible(nSelStartChar);
            m_bSelForward = true;
        }
    }
    else if (msg.vkCode == kVK_END) {
        m_nSelXPos = -1;
        //END键
        int32_t nSelStartChar = -1;
        int32_t nSelEndChar = -1;
        GetSel(nSelStartChar, nSelEndChar);
        ASSERT(nSelEndChar >= nSelStartChar);

        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        if (bShiftDown && (m_nShiftStartIndex != -1)) {
            //Shift + End 键
            int32_t nShiftRowEndIndex = m_pTextData->GetRowEndCharIndex(m_nShiftStartIndex);
            int32_t nRowEndIndex = m_pTextData->GetRowEndCharIndex(nSelStartChar);
            int32_t nRowEndIndex2 = m_pTextData->GetRowEndCharIndex(nSelEndChar);
            if (nRowEndIndex == nRowEndIndex2) {
                //在同一行, 不需要区分操作方向
                if (nShiftRowEndIndex == nRowEndIndex) {
                    InternalSetSel(m_nShiftStartIndex, nRowEndIndex);
                }
                else {
                    InternalSetSel(nSelStartChar, nRowEndIndex);
                }
                EnsureCharVisible(nRowEndIndex);
            }
            else {
                //在不同行
                if (m_bSelForward) {
                    //操作方向：向前
                    nSelEndChar = m_pTextData->GetRowEndCharIndex(nSelEndChar);
                    InternalSetSel(nSelStartChar, nSelEndChar);
                }
                else {
                    //操作方向：向后
                    nSelStartChar = m_pTextData->GetRowEndCharIndex(nSelStartChar);
                    InternalSetSel(nSelStartChar, nSelEndChar);
                }
                EnsureCharVisible(nSelEndChar);
            }
        }
        else {
            //End 键
            nSelEndChar = m_pTextData->GetRowEndCharIndex(nSelEndChar);
            InternalSetSel(nSelEndChar, nSelEndChar);
            EnsureCharVisible(nSelEndChar);
            m_bSelForward = true;
        }
    }
    return true;
}

int32_t RichEdit2::GetLineScrollDeltaValue(bool bLineDown) const
{
    int32_t nLineDeltaValue = DUI_NOSET_VALUE;
    if (m_nRowHeight > 0) {
        nLineDeltaValue = m_nRowHeight;
        UiSize64 scrollPos = GetScrollPos();
        if ((scrollPos.cy % nLineDeltaValue) != 0) {
            //确保按行对齐
            if (bLineDown) {
                nLineDeltaValue = nLineDeltaValue - (scrollPos.cy % nLineDeltaValue);
            }
            else {
                nLineDeltaValue = scrollPos.cy % nLineDeltaValue;
            }
        }
    }
    return nLineDeltaValue;
}

int32_t RichEdit2::GetPageScrollDeltaValue(bool bPageDown) const
{
    UiRect rcDrawRect = GetTextDrawRect(GetRect());
    int32_t nPageDeltaValue = rcDrawRect.Height();
    //按行对齐
    if (m_nRowHeight > 0) {
        UiSize64 scrollPos = GetScrollPos();
        if ((scrollPos.cy % m_nRowHeight) != 0) {
            //确保按行对齐
            if (bPageDown) {
                nPageDeltaValue = nPageDeltaValue - (scrollPos.cy % m_nRowHeight);
            }
            else {
                nPageDeltaValue += scrollPos.cy % m_nRowHeight;
                nPageDeltaValue -= m_nRowHeight;
            }
            if (nPageDeltaValue <= 0) {
                nPageDeltaValue = rcDrawRect.Height();
            }
        }        
    }    
    return nPageDeltaValue;
}

void RichEdit2::CheckKeyDownStartIndex(const EventArgs& msg)
{
    if ((m_nShiftStartIndex != -1) && !IsKeyDown(msg, ModifierKey::kShift)) {
        //恢复选择的起始位置
        m_nShiftStartIndex = -1;
    }
    else if ((m_nCtrlStartIndex != -1) && !IsKeyDown(msg, ModifierKey::kControl)) {
        //恢复选择的起始位置
        m_nCtrlStartIndex = -1;
    }
}

bool RichEdit2::OnKeyUp(const EventArgs& msg)
{
    CheckKeyDownStartIndex(msg);
    return BaseClass::OnKeyUp(msg);
}

bool RichEdit2::OnChar(const EventArgs& msg)
{
    //输入一个字符
    OnInputChar(msg);
    return true;
}

bool RichEdit2::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }

    // 记录鼠标按下位置，用于检测拖动文本功能
    m_bDraggingOutMouseDown = false;
    CheckDragOutStart(msg.ptMouse);

    if (!m_bDraggingOutMouseDown) {
        //鼠标点击时，检查按键状态（因按住Shift/Ctrl的时候，如果按组合键，则Shift/Ctrl的Up消息会丢失，导致状态异常）
        CheckKeyDownStartIndex(msg);

        //响应鼠标按下事件，记录状态
        OnLButtonDown(msg.ptMouse, msg.GetSender(), IsKeyDown(msg, ModifierKey::kShift));
    }
    return bRet;
}

bool RichEdit2::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnLButtonUp(msg.ptMouse, msg.GetSender());
    if (m_bDraggingOutMouseDown) {
        //调整光标位置到鼠标点击位置
        int32_t nCharPosIndex = CharFromPos(msg.ptMouse);
        InternalSetSel(nCharPosIndex, nCharPosIndex);
        m_bDraggingOutMouseDown = false;
    }    
    return bRet;
}

bool RichEdit2::ButtonDoubleClick(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDoubleClick(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnLButtonDoubleClick(msg.ptMouse, msg.GetSender());
    m_bDraggingOutMouseDown = false;
    return bRet;
}

bool RichEdit2::RButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnRButtonDown(msg.ptMouse, msg.GetSender());
    m_bDraggingOutMouseDown = false;
    return bRet;
}

bool RichEdit2::RButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnRButtonUp(msg.ptMouse, msg.GetSender());
    return bRet;
}

bool RichEdit2::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }

    if (IsEnableDragOut() && m_bDraggingOutMouseDown) {
        // 检测拖放操作
        CheckDoDragDrop(msg.ptMouse);
    }
    else {
        //检测鼠标框选功能（通过按住鼠标，滑动位置来选择文本）
        OnMouseMove(msg.ptMouse, msg.GetSender());
    }   
    return bRet;
}

bool RichEdit2::MouseWheel(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseWheel(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnMouseWheel(msg.eventData, IsKeyDown(msg, ModifierKey::kControl));
    return bRet;
}

bool RichEdit2::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnWindowKillFocus(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    OnWindowKillFocus();
    return bRet;
}

void RichEdit2::OnLButtonDown(const UiPoint& ptMouse, Control* pSender, bool bShiftDown)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        Invalidate();
    }
    m_bMouseDownInView = (pSender == this) ? true : false;
    m_bMouseDown = true;
    m_pMouseSender = pSender;
    UiSize64 scrollPos = GetScrollPos();
    m_ptMouseDown.cx = ptMouse.x + scrollPos.cx;
    m_ptMouseDown.cy = ptMouse.y + scrollPos.cy;

    //调整光标位置到鼠标点击位置
    int32_t nCharPosIndex = CharFromPos(ptMouse);
    if (bShiftDown && (m_nShiftStartIndex != -1)) {
        //按住Shift键时，选择与原来起点的范围
        InternalSetSel(m_nShiftStartIndex, nCharPosIndex);
        m_bSelForward = (nCharPosIndex >= m_nShiftStartIndex) ? true : false;
    }
    else {
        InternalSetSel(nCharPosIndex, nCharPosIndex);
        m_bSelForward = true;
    }
    m_nSelXPos = -1;

    CheckSelAllOnFocus();
}

void RichEdit2::OnLButtonUp(const UiPoint& /*ptMouse*/, Control* /*pSender*/)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        Invalidate();
    }
    m_bMouseDownInView = false;
    m_bMouseDown = false;
    m_pMouseSender = nullptr;
}

void RichEdit2::OnLButtonDoubleClick(const UiPoint& ptMouse, Control* /*pSender*/)
{
    if (IsReadOnly()) {
        //只读模式下，双击则全选文本
        SetSelAll();
    }
    else {
        //非只读模式下，选择单词
        int32_t nCharPosIndex = CharFromPos(ptMouse);
        int32_t nWordStartIndex = 0;
        int32_t nWordEndIndex = 0;
        if (m_pTextData->GetCurrentWordIndex(nCharPosIndex, nWordStartIndex, nWordEndIndex)) {
            InternalSetSel(nWordStartIndex, nWordEndIndex);
            m_nSelXPos = -1;
        }        
    }
}

void RichEdit2::OnRButtonDown(const UiPoint& ptMouse, Control* pSender)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        Invalidate();
    }
    m_bMouseDownInView = (pSender == this) ? true : false;
    m_bRMouseDown = true;
    m_pMouseSender = pSender;
    UiSize64 scrollPos = GetScrollPos();
    m_ptMouseDown.cx = ptMouse.x + scrollPos.cx;
    m_ptMouseDown.cy = ptMouse.y + scrollPos.cy;

    if (!HasSelText()) {
        //调整光标位置到鼠标点击位置
        int32_t nCharPosIndex = CharFromPos(ptMouse);
        InternalSetSel(nCharPosIndex, nCharPosIndex);
        m_nSelXPos = -1;
    }

    CheckSelAllOnFocus();
}

void RichEdit2::OnRButtonUp(const UiPoint& /*ptMouse*/, Control* /*pSender*/)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        Invalidate();
    }
    m_bMouseDownInView = false;
    m_bRMouseDown = false;
    m_pMouseSender = nullptr;
}

void RichEdit2::OnMouseMove(const UiPoint& ptMouse, Control* pSender)
{
    if ((m_bMouseDown || m_bRMouseDown) &&
        (pSender != nullptr) &&
        (m_pMouseSender == pSender) && pSender->IsMouseFocused()) {
        UiSize64 scrollPos = GetScrollPos();
        m_ptMouseMove.cx = ptMouse.x + scrollPos.cx;
        m_ptMouseMove.cy = ptMouse.y + scrollPos.cy;

        //鼠标移动超过指定像素数的时候，才开始按移动操作，避免将正常的点击操作识别为框选操作
        constexpr const int32_t minPt = 8;
        if (!m_bInMouseMove) {
            if ((std::abs(m_ptMouseMove.cx - m_ptMouseDown.cx) > minPt) ||
                (std::abs(m_ptMouseMove.cy - m_ptMouseDown.cy) > minPt)) {
                //开始框选操作
                m_bInMouseMove = true;
                OnCheckScrollView();
            }
        }
        else {
            //按需滚动视图，并更新鼠标在滚动后的位置            
            OnCheckScrollView();
        }
    }
    else if (m_bInMouseMove) {
        m_bInMouseMove = false;
        Invalidate();
    }
}

void RichEdit2::OnMouseWheel(int32_t wheelDelta, bool bCtrlDown)
{
    if (bCtrlDown && IsEnableWheelZoom()) {
        bool bZoomIn = wheelDelta > 0 ? true : false;
        uint32_t nZoomPercent = GetNextZoomPercent(GetZoomPercent(), bZoomIn);
        SetZoomPercent(nZoomPercent);        
    }
}

uint32_t RichEdit2::GetNextZoomPercent(uint32_t nOldZoomPercent, bool bZoomIn) const
{
    //通过查表的方式设置缩放比例(放大和缩小的时候，能够按照原来的比例复原)
    std::vector<uint32_t> zoomPercentList;
    uint32_t nZoomPercent = 100;
    while (nZoomPercent > 1) {
        nZoomPercent = (uint32_t)(nZoomPercent * 0.90f);
        zoomPercentList.insert(zoomPercentList.begin(), nZoomPercent);
    }
    nZoomPercent = 100;
    while (nZoomPercent < MAX_ZOOM_PERCENT) {
        zoomPercentList.insert(zoomPercentList.end(), nZoomPercent);
        nZoomPercent = (uint32_t)(nZoomPercent * 1.10f);
    }
    zoomPercentList.insert(zoomPercentList.end(), MAX_ZOOM_PERCENT);

    bool bFound = false;
    nZoomPercent = nOldZoomPercent;
    const size_t nPercentCount = zoomPercentList.size();
    for (size_t nPercentIndex = 0; nPercentIndex < nPercentCount; ++nPercentIndex) {
        if ((zoomPercentList[nPercentIndex] > nZoomPercent) || (nPercentIndex == (nPercentCount - 1))) {
            if (nPercentIndex <= 1) {
                size_t nCurrentIndex = 0;//当前是第一个元素
                if (bZoomIn) {
                    //放大
                    nZoomPercent = zoomPercentList[nCurrentIndex + 1];
                }
                else {
                    //缩小（已经到达最小值，无法再缩小）
                    nZoomPercent = zoomPercentList[nCurrentIndex];
                }
            }
            else if (zoomPercentList[nPercentIndex] > nZoomPercent) {
                size_t nCurrentIndex = nPercentIndex - 1;//中间元素
                if (bZoomIn) {
                    //放大
                    nZoomPercent = zoomPercentList[nCurrentIndex + 1];
                }
                else {
                    //缩小
                    nZoomPercent = zoomPercentList[nCurrentIndex - 1];
                }
            }
            else if (nPercentIndex == (nPercentCount - 1)) {
                size_t nCurrentIndex = nPercentCount - 1;//当前是最后一个元素
                if (bZoomIn) {
                    //放大（已经到达最大值，无法再放大）
                    nZoomPercent = zoomPercentList[nCurrentIndex];
                }
                else {
                    //缩小
                    nZoomPercent = zoomPercentList[nCurrentIndex - 1];
                }
            }
            bFound = true;
            break;
        }
    }

    if (!bFound) {
        //如果查表得不到结果，则按比例放大或者缩小
        if (bZoomIn) {
            //放大
            nZoomPercent = (uint32_t)(nZoomPercent * 1.10f);
            if (nZoomPercent == nOldZoomPercent) {
                //避免数值过小时无法放大
                ++nZoomPercent;
            }
        }
        else {
            //缩小
            nZoomPercent = (uint32_t)(nZoomPercent * 0.91f);
        }
    }
    if (nZoomPercent < 1) {
        nZoomPercent = 1;
    }
    return nZoomPercent;
}

void RichEdit2::OnWindowKillFocus()
{
    if (m_bInMouseMove) {
        Invalidate();
    }
    m_bMouseDownInView = false;
    m_bMouseDown = false;
    m_bRMouseDown = false;
    m_bInMouseMove = false;
    m_pMouseSender = nullptr;
}

void RichEdit2::OnCheckScrollView()
{
    if (!m_bInMouseMove) {
        //取消定时器
        m_scrollViewFlag.Cancel();
        return;
    }
    bool bScrollView = false;
    const UiSize64 scrollPos = GetScrollPos();
    UiSize64 pt = m_ptMouseMove;
    pt.cx -= scrollPos.cx;
    pt.cy -= scrollPos.cy;
    const UiSize64 ptMouseMove = pt; //记录原值

    if (m_bInMouseMove) {
        int32_t nHScrollValue = DUI_NOSET_VALUE;
        int32_t nVScrollValue = DUI_NOSET_VALUE;
        UiRect viewRect = GetRect();
        if (pt.cx <= viewRect.left) {
            //向左滚动视图
            LineLeft(nHScrollValue);
            bScrollView = true;
        }
        else if (pt.cx >= viewRect.right) {
            //向右滚动视图
            LineRight(nHScrollValue);
            bScrollView = true;
        }
        if (pt.cy <= viewRect.top) {
            //向上滚动视图
            LineUp(nVScrollValue);
            bScrollView = true;
        }
        else if (pt.cy >= viewRect.bottom) {
            //向下滚动视图
            LineDown(nVScrollValue);
            bScrollView = true;
        }
    }

    if (bScrollView) {
        UiSize64 scrollPosNew = GetScrollPos();
        if (scrollPos != scrollPosNew) {
            //更新鼠标位置
            m_ptMouseMove.cx = ptMouseMove.cx + scrollPosNew.cx;
            m_ptMouseMove.cy = ptMouseMove.cy + scrollPosNew.cy;
        }

        //启动定时器
        m_scrollViewFlag.Cancel();
        GlobalManager::Instance().Timer().AddTimer(m_scrollViewFlag.GetWeakFlag(),
                                                   UiBind(&RichEdit2::OnCheckScrollView, this),
                                                   50, 1); //只执行一次
    }
    else {
        //取消定时器
        m_scrollViewFlag.Cancel();
    }

    OnFrameSelection(m_ptMouseDown, m_ptMouseMove);
}

void RichEdit2::OnFrameSelection(UiSize64 ptMouseDown64, UiSize64 ptMouseMove64)
{
    const UiSize64 scrollPos = GetScrollPos();//用于恢复字符绘制区域的坐标系
    UiPoint ptMouseDown;    
    ptMouseDown.x = ui::TruncateToInt32(ptMouseDown64.cx - scrollPos.cx);
    ptMouseDown.y = ui::TruncateToInt32(ptMouseDown64.cy - scrollPos.cy);

    UiPoint ptMouseMove;
    ptMouseMove.x = ui::TruncateToInt32(ptMouseMove64.cx - scrollPos.cx);
    ptMouseMove.y = ui::TruncateToInt32(ptMouseMove64.cy - scrollPos.cy);

    //触发重绘, 但不需要重新计算
    Invalidate();

    int32_t nStart = CharFromPos(ptMouseDown);
    int32_t nEnd = CharFromPos(ptMouseMove);
    m_bSelForward = nEnd >= nStart ? true : false;

    InternalSetSel(nStart, nEnd);
    m_nSelXPos = -1;
}

void RichEdit2::UpdateScrollRange()
{
    //通过基类的SetPos完成
    SetPos(GetPos());
}

#ifndef DUILIB_BUILD_FOR_SDL
/** 获取输入的字符(Windows API实现部分)
*/
DStringW RichEdit2::GetInputTextW(UINT uMsg, WPARAM wParam)
{
    ASSERT((uMsg == WM_CHAR) || (uMsg == WM_UNICHAR));
    DStringW text;
    if ((uMsg != WM_CHAR) && (uMsg != WM_UNICHAR)) {
        return text;
    }
    // 过滤：所有其他 0~31 控制符 + DEL
    if (wParam <= 0x1F || wParam == 0x7F) {
#ifdef DUILIB_UNICODE
        m_chHighSurrogate = L'\0';
#else
        m_pendingChars.clear();
        m_dwLastCharTime = 0;
#endif
        return text;
    }

    if (uMsg == WM_UNICHAR) {
        text = (DStringW::value_type)wParam;
        return text;
    }

#ifdef DUILIB_UNICODE
    //Windows API模式：一次输入一个字符
    if (IS_HIGH_SURROGATE(wParam)) {
        m_chHighSurrogate = (WCHAR)wParam;
    }
    else {
        WCHAR utf16[3];
        utf16[0] = m_chHighSurrogate ? m_chHighSurrogate : (WCHAR)wParam;
        utf16[1] = m_chHighSurrogate ? (WCHAR)wParam : L'\0';
        utf16[2] = L'\0';
        text = utf16;
        m_chHighSurrogate = L'\0';
    }
#else
    //MBCS模式: 只支持1字节和2字节的文字输入，不支持4字节的文字输入
    bool bHandled = false;
    if (m_pendingChars.empty()) {
        if (IsDBCSLeadByte((BYTE)wParam)) {
            m_pendingChars.push_back((BYTE)wParam);
            bHandled = true;
        }
    }
    else {
        if (m_pendingChars.size() == 1) {
            BYTE chMBCS[8] = { m_pendingChars.front(), (BYTE)wParam, 0, };
            DStringW::value_type chWideChar[4] = { 0, };
            ::MultiByteToWideChar(CP_ACP, 0, (const char*)chMBCS, 2, chWideChar, 4);
            if (chWideChar[0] != 0) {
                text = chWideChar;//获取到文本内容
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
        BYTE chMBCS[8] = { (BYTE)wParam, 0, 0, };
        DStringW::value_type chWideChar[4] = { 0, };
        ::MultiByteToWideChar(CP_ACP, 0, (const char*)chMBCS, 2, chWideChar, 4);
        if (chWideChar[0] != 0) {
            text = chWideChar;//获取到文本内容
        }
    }
#endif // DUILIB_UNICODE
    return text;
}
#endif

void RichEdit2::OnInputChar(const EventArgs& msg)
{
    //校验当前状态是否可以编辑
    m_nSelXPos = -1;
    if (IsReadOnly() || !IsEnabled()) {
        //只读或者Disable状态，禁止编辑
        return;
    }

    //校验输入参数是否合法
    if (msg.eventType == ui::kEventKeyDown) {
        //仅处理：回车键, TAB键, 删除键，退格键的处理逻辑
        ASSERT((msg.vkCode == kVK_RETURN) || (msg.vkCode == kVK_TAB) || (msg.vkCode == kVK_DELETE) || (msg.vkCode == kVK_BACK));
        if ((msg.vkCode != kVK_RETURN) && (msg.vkCode != kVK_TAB) && (msg.vkCode != kVK_DELETE) && (msg.vkCode != kVK_BACK)) {            
            return;
        }
    }
    else {
        ASSERT(msg.eventType == ui::kEventChar);
        if (msg.eventType != ui::kEventChar) {
            return;
        }
#ifdef DUILIB_BUILD_FOR_SDL
        ASSERT(msg.eventData == SDL_EVENT_TEXT_INPUT);
        if (msg.eventData != SDL_EVENT_TEXT_INPUT) {
            return;
        }
#elif defined (DUILIB_BUILD_FOR_WIN)
        ASSERT((msg.eventData == WM_CHAR) || (msg.eventData == WM_SYSCHAR) || (msg.eventData == WM_UNICHAR));
        if (msg.eventData == WM_SYSCHAR) {
            //该消息不处理
            return;
        }
#endif
    }

    //对TAB键和回车键的预处理（这两个键是可配置的，根据选项确定是否可以输入）    
    if (msg.vkCode == kVK_TAB) {
        //按下TAB键
        bool bEnableInputChar = true;
        if (!m_bWantTab) {
            //不接受TAB键，触发TAB按键事件
            bEnableInputChar = false;
            SendEvent(kEventTab);
        }
        if (bEnableInputChar && IsPasswordMode()) {
            //密码模式下，不支持输入TAB键字符
            bEnableInputChar = false;
        }
        if (!bEnableInputChar) {
            //无需编辑文本
            return;
        }
    }
    else if (msg.vkCode == kVK_RETURN) {
        //按下回车键
        bool bEnableInputChar = true;
        bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
        bool bCtrlDown = IsKeyDown(msg, ModifierKey::kControl);
        if (bCtrlDown && !bShiftDown) {
            if (!m_bWantCtrlReturn) {
                //不接受Ctrl + Enter，触发回车键事件
                bEnableInputChar = false;
                SendEvent(kEventReturn);
            }
        }
        else {
            if (!m_bWantReturn) {
                //不接受Enter键，触发回车键事件
                bEnableInputChar = false;
                SendEvent(kEventReturn);                
            }
        }
        if (bEnableInputChar && (!IsMultiLine() || IsPasswordMode())) {
            //单行模式下，或者密码模式下，不支持输入换行符
            bEnableInputChar = false;
        }
        if (!bEnableInputChar) {
            //无需编辑文本
            return;
        }
    }

    //获取本次输入的文本
    DStringW text;
    if (msg.eventType == ui::kEventKeyDown) {
        if (msg.vkCode == kVK_RETURN) {
            //回车: 转换成换行："\r\n" 或者 "\n"
#if defined (DUILIB_BUILD_FOR_WIN)
            text = L"\r\n";
#else
            text = L"\n";
#endif
        }
        else if (msg.vkCode == kVK_TAB) {
            //TAB键
            text = L"\t";
        }
        else if (msg.vkCode == kVK_DELETE) {
            text.clear();
        }
        else if (msg.vkCode == kVK_BACK) {
            text.clear();
        }
        else {
            ASSERT(0);
            return;
        }
    }
    else {
        //文字输入
        ASSERT(msg.eventType == ui::kEventChar);
        if (msg.eventType != ui::kEventChar) {
            return;
        }
        text.clear();
#ifdef DUILIB_BUILD_FOR_SDL
        ASSERT(msg.eventData == SDL_EVENT_TEXT_INPUT);
        ASSERT(msg.vkCode == kVK_None);
        if ((msg.eventData == SDL_EVENT_TEXT_INPUT) && (msg.wParam != 0) && (msg.lParam > 0)) {
            //当前输入的字符或者字符串（比如中文输入时，候选词是一次输入，而不像Windows SDK那样按字符逐次输入）
            text = (DStringW::value_type*)msg.wParam;
        }
#else
        //Windows API实现
        text = GetInputTextW((UINT)msg.eventData, msg.wParam);
#endif  //DUILIB_BUILD_FOR_SDL
        if (text.empty()) {
            //无有效文本输入
            return;
        }
    }

    //密码模式下：删除非法字符
    if (!text.empty() && IsPasswordMode()) {
        RemoveInvalidPasswordChar(text);
    }

    //数字模式下，检查是否存在不允许输入的字符（数字模式：只允许输入数字）
    if (!text.empty() && IsNumberOnly()) {
        size_t nTextIndex = 0;
        if (text[0] == _T('-')) {
            //首字符是减号，对应的是负数
            if (GetTextLength() > 0) {
                //不是第一个字符，禁止输入减号
                return;
            }
            else if (GetMinNumber() >= 0) {
                //最小数字是0或者正数，禁止输入减号
                return;
            }
            nTextIndex += 1;
        }
        const size_t nTextCount = text.size();
        for (; nTextIndex < nTextCount; ++nTextIndex) {
            if (text[nTextIndex] < L'0' || text[nTextIndex] > L'9') {
                //遇到非数字字符，禁止输入
                return;
            }
        }
    }

    //有限制字符的模式下，检查是否存在不允许输入的字符
    if (!text.empty() && (m_pLimitChars != nullptr)) {
        const size_t nTextCount = text.size();
        for (size_t nIndex = 0; nIndex < nTextCount; ++nIndex) {
            if (!IsInLimitChars(text[nIndex])) {
                //字符不在列表里面，禁止输入
                return;
            }
        }
    }

    //输入字符
    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    ASSERT(nSelEndChar >= nSelStartChar);
    if (nSelEndChar < nSelStartChar) {
        //出错
        return;
    }

    if (msg.vkCode == kVK_DELETE) {
        //删除键
        if (nSelEndChar > nSelStartChar) {
            //有选择文本：删除选择内容
            text.clear();
        }
        else {
            //无选择文本：删除后一个字符
            text.clear();
            bool bMatchWord = IsKeyDown(msg, ModifierKey::kControl); //Ctrl + Delete键，删除光标后面的单词
            if (bMatchWord) {
                nSelEndChar = m_pTextData->GetNextValidWordIndex(nSelStartChar);
            }
            else {
                nSelEndChar = m_pTextData->GetNextValidCharIndex(nSelStartChar);
            }            
        }
        if ((nSelEndChar == nSelStartChar) && text.empty()) {
            //不满足删除条件
            return;
        }
    }
    else if (msg.vkCode == kVK_BACK) {
        //Backspace键
        if (nSelEndChar > nSelStartChar) {
            //有选择文本：删除选择内容
            text.clear();
        }
        else {
            //无选择文本：删除前一个字符
            text.clear();
            bool bMatchWord = IsKeyDown(msg, ModifierKey::kControl); //Ctrl + Backspace键，删除光标前面的单词
            if (bMatchWord) {
                nSelStartChar = m_pTextData->GetPrevValidWordIndex(nSelStartChar);
            }
            else {
                nSelStartChar = m_pTextData->GetPrevValidCharIndex(nSelStartChar);
            }            
        }
        if ((nSelEndChar == nSelStartChar) && text.empty()) {
            //不满足删除条件
            return;
        }
    }
    else if (text.empty()) {
        //无输入文本
        return;
    }

    //是否检测数字模式
    bool bCheckNumberOnly = IsNumberOnly() && ((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX));
    DStringW oldText;
    if (bCheckNumberOnly) {
        oldText = m_pTextData->GetText();
    }

    bool bRet = m_pTextData->ReplaceText(nSelStartChar, nSelEndChar, text, true);
    if (!bRet) {
        return;
    }
    int32_t nNewSelChar = nSelStartChar + (int32_t)text.size();
    InternalSetSel(nNewSelChar, nNewSelChar);

    //闪现密码功能(仅当从尾部输入时，提供字符闪现功能)
    if (IsPasswordMode()) {
        m_falshPasswordFlag.Cancel();
    }
    if (IsPasswordMode() && !IsShowPassword() && IsFlashPasswordChar()) {
        if (nNewSelChar == GetTextLength()) {
            m_bInputPasswordChar = true;            
            std::function<void()> closure = UiBind(&RichEdit2::StopFlashPasswordChar, this);
            GlobalManager::Instance().Timer().AddTimer(m_falshPasswordFlag.GetWeakFlag(), closure, 1500);
        }
    }

    //更新滚动条
    UpdateScrollRange();

    //确保光标可见
    EnsureCharVisible(nNewSelChar);

    bool bTextChanged = true;
    if (bCheckNumberOnly) {
        //数字模式，检查文本对应的数字是否在范围内
        DString newText = GetText();
        if (!newText.empty()) {
            int64_t n = StringUtil::StringToInt64(newText);
            if (n < GetMinNumber()) {
                //超过最小数字，进行修正
                int32_t newValue = GetMinNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                bTextChanged = (oldText != m_pTextData->GetText()) ? true : false;
            }
            else if (n > GetMaxNumber()) {
                //超过最大数字，进行修正
                int32_t newValue = GetMaxNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                bTextChanged = (oldText != m_pTextData->GetText()) ? true : false;
            }
        }
    }

    //触发文本变化事件
    if (bTextChanged) {
        OnTextChanged();
    }
}

void RichEdit2::SetEnableDragDrop(bool bEnable)
{
    BaseClass::SetEnableDragDrop(bEnable);
    if (bEnable) {
        if (m_pControlDropTarget == nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
            m_pControlDropTarget = new RichEditDropTarget_Windows(this);
#elif defined (DUILIB_BUILD_FOR_SDL)
            m_pControlDropTarget = new RichEditDropTarget_SDL(this);
#endif
        }
    }
    else {
        if (m_pControlDropTarget != nullptr) {
            delete m_pControlDropTarget;
            m_pControlDropTarget = nullptr;
        }
    }
}

bool RichEdit2::IsEnableDragDrop() const
{
    return (m_pControlDropTarget != nullptr);
}

ControlDropTarget_Windows* RichEdit2::GetControlDropTarget()
{
    if (IsReadOnly() || IsPasswordMode() || !IsEnabled()) {
        //只读模式、密码模式、不可用模式，关闭拖放功能
        return nullptr;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    return m_pControlDropTarget;
#else
    return nullptr;
#endif
}

ControlDropTarget_SDL* RichEdit2::GetControlDropTarget_SDL()
{
    if (IsReadOnly() || IsPasswordMode() || !IsEnabled()) {
        //只读模式、密码模式、不可用模式，关闭拖放功能
        return nullptr;
    }
#ifdef DUILIB_BUILD_FOR_SDL
    return m_pControlDropTarget;
#else
    return nullptr;
#endif
}

void RichEdit2::SetEnableDragOut(bool bEnable)
{
#if defined(DUILIB_BUILD_FOR_WIN)
    m_bEnableDragOut = bEnable;
#else
    (void)bEnable;
#endif
}

bool RichEdit2::IsEnableDragOut() const
{
#if defined(DUILIB_BUILD_FOR_WIN)
    return m_bEnableDragOut;
#else
    return false;
#endif
}

void RichEdit2::CheckDragOutStart(const UiPoint& ptMouse)
{
    m_bDraggingOutMouseDown = false;
    m_bDraggingOut = false;
#if defined(DUILIB_BUILD_FOR_WIN)
    if (IsEnableDragOut() && !IsPasswordMode() && IsEnabled()) {
        const UiSize64 scrollPos = GetScrollPos();
        m_ptDragOutStart.cx = ptMouse.x + scrollPos.cx;
        m_ptDragOutStart.cy = ptMouse.y + scrollPos.cy;

        if (IsMouseOnSelectionText(ptMouse)) {
            //有选中文本, 并且鼠标点击在选中的文本上时，开始检测拖出操作
            m_bDraggingOutMouseDown = true;
        }
    }
#else
    (void)ptMouse;
#endif
}

bool RichEdit2::IsMouseOnSelectionText(const UiPoint& ptMouse)
{
    bool bRet = false;
    int32_t nSelStartChar = -1;
    int32_t nSelEndChar = -1;
    GetSel(nSelStartChar, nSelEndChar);
    if ((nSelEndChar > nSelStartChar) && (nSelStartChar >= 0)) {
        //有选中文本, 并且鼠标点击在选中的文本上
        int32_t nCharPosIndex = CharFromPos(ptMouse);
        if ((nCharPosIndex >= nSelStartChar) && (nCharPosIndex < nSelEndChar)) {
            bRet = true;
        }
    }
    return bRet;
}

bool RichEdit2::CanDropTextOnMousePosition(const UiPoint& ptMouse)
{
#if defined(DUILIB_BUILD_FOR_WIN)
    if (IsEnableDragOut() && !IsPasswordMode() && IsEnabled() && !IsReadOnly()) {
        if (m_bDraggingOutMouseDown && m_bDraggingOut) {
            if (IsMouseOnSelectionText(ptMouse)) {
                //不能拖放到正在拖出的文本处
                return false;
            }
        }
    }
    return true;
#else
    return true;
#endif
}

void RichEdit2::SetDropTextPosition(int32_t nDropPos)
{
    m_nDropTextPos = nDropPos;
}

void RichEdit2::CheckDoDragDrop(const UiPoint& ptMouse)
{
#if defined(DUILIB_BUILD_FOR_WIN)
    // 检查是否已启用拖出功能
    if (!IsEnableDragOut() || IsPasswordMode() || !IsEnabled()) {
        m_bDraggingOutMouseDown = false;
        m_bDraggingOut = false;
        return;
    }

    // 检查是否正在拖动（避免重复触发）
    if (m_bDraggingOut) {
        return;
    }

    // 检查是否有选中的文本
    int32_t nDragOutStart = 0;
    int32_t nDragOutEnd = 0;
    GetSel(nDragOutStart, nDragOutEnd);
    if (nDragOutStart >= nDragOutEnd) {
        // 没有选中的文本
        return;
    }

    // 计算鼠标移动距离
    const UiSize64 scrollPos = GetScrollPos();
    int64_t dx = abs(ptMouse.x + scrollPos.cx - m_ptDragOutStart.cx);
    int64_t dy = abs(ptMouse.y + scrollPos.cy - m_ptDragOutStart.cy);
    
    // 定义拖动阈值（3像素）
    const int32_t nDragThreshold = Dpi().GetScaleInt(3);
    
    // 如果移动超过阈值，开始拖放
    if (dx > nDragThreshold || dy > nDragThreshold) {
        m_bDraggingOut = true;
        
        // 获取选中的文本
#ifdef DUILIB_UNICODE
        DStringW selectedText = GetSelText();
#else
        DStringW selectedText = StringConvert::UTF8ToWString(GetSelText());
#endif
        
        if (!selectedText.empty()) {
            // 执行拖放操作
            m_nDropTextPos = -1;
            uint32_t effect = DoDragDrop(selectedText);
            
            // 根据拖放结果处理
            if ((effect == DROPEFFECT_MOVE) && !IsReadOnly()) {
                // 移动操作：删除原文本
                if (m_nDropTextPos == -1) {
                    //Drop操作不在本RichEdit内部
                    ReplaceSel(_T(""), true);
                }
                else {
                    //Drop操作在本RichEdit内部
                    if (m_nDropTextPos < nDragOutStart) {
                        //Drop位置在选中文本之前
                        int32_t nDropStart = 0;
                        int32_t nDropEnd = 0;
                        GetSel(nDropStart, nDropEnd);

                        nDragOutStart += (int32_t)selectedText.size();
                        nDragOutEnd += (int32_t)selectedText.size();
                        SetSel(nDragOutStart, nDragOutEnd);
                        ReplaceSel(_T(""), true);

                        SetSel(nDropStart, nDropEnd);
                    }
                    else if (m_nDropTextPos >= nDragOutEnd) {
                        //Drop位置在选中文本之后
                        int32_t nDropStart = 0;
                        int32_t nDropEnd = 0;
                        GetSel(nDropStart, nDropEnd);

                        SetSel(nDragOutStart, nDragOutEnd);
                        ReplaceSel(_T(""), true);

                        nDropStart -= (int32_t)selectedText.size();
                        nDropEnd -= (int32_t)selectedText.size();
                        SetSel(nDropStart, nDropEnd);
                    }
                    else {
                        ASSERT(0);
                    }
                }
            }
        }
        m_bDraggingOutMouseDown = false;
        m_bDraggingOut = false;
    }
#else
    (void)ptMouse;
#endif
}

uint32_t RichEdit2::DoDragDrop(const DStringW& text)
{
#if defined(DUILIB_BUILD_FOR_WIN)
    if (text.empty() || !IsEnableDragOut() || IsPasswordMode() || !IsEnabled()) {
        return DROPEFFECT_NONE;
    }

    RichEditDataObject_Windows* pDataObject = RichEditDataObject_Windows::Create(text);
    if (pDataObject == nullptr) {
        return DROPEFFECT_NONE;
    }
    pDataObject->AddRef();

    RichEditDragSource_Windows* pDropSource = RichEditDragSource_Windows::Create();
    if (pDropSource == nullptr) {
        pDataObject->Release();
        return DROPEFFECT_NONE;
    }
    pDropSource->AddRef();

    DWORD dwEffect = DROPEFFECT_COPY;
    HRESULT hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);

    pDropSource->Release();
    pDataObject->Release();

    if (hr != DRAGDROP_S_DROP) {
        return DROPEFFECT_NONE;
    }
    return dwEffect;
#else
    return 0;
#endif
}

} // namespace ui
