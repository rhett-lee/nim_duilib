#include "RichEdit_Windows.h"
#include "RichEditHost_Windows.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/BitmapHelper_Windows.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Menu.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/Button.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Core/ControlDropTargetImpl_Windows.h"
#include "duilib/Core/ControlDropTargetUtils.h"

namespace ui {

/** 拖放操作接口的实现（仅是拖入操作）
*/
class RichEditDropTarget : public ControlDropTarget_Windows
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
        m_dropTextList.clear();
        m_dropFileList.clear();

        ControlDropTargetImpl_Windows::ParseWindowsDataObject(pDataObj, m_dropTextList, m_dropFileList);
        if ((m_pRichEdit != nullptr) && !m_dropFileList.empty()){
            if (!m_pRichEdit->IsEnableDropFile()) {
                //不支持文件拖放操作
                return hr;
            }
            //支持文件拖放操作，直接返回
            return S_OK;
        }

        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            DWORD dwEffect = DROPEFFECT_NONE;
            if (pdwEffect != nullptr) {
                dwEffect = *pdwEffect;
            }
            hr = pDropTarget->DragEnter((IDataObject*)pDataObj, grfKeyState, POINTL{ pt.x, pt.y }, &dwEffect);
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

        if ((m_pRichEdit != nullptr) && !m_dropFileList.empty()) {
            if (!m_pRichEdit->IsEnableDropFile()) {
                //不支持文件拖放操作
                return hr;
            }
            //支持文件拖放操作，判断是否满足过滤条件
            DString fileTypes = m_pRichEdit->GetDropFileTypes();
            if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_dropFileList)) {
                //文件类型不满足过滤条件
                return hr;
            }
            //支持文件拖放操作，直接返回
            return S_OK;
        }
        if ((m_pRichEdit != nullptr) && !m_dropTextList.empty()) {
            //拖入文本操作：进行有效性判断
            if (!m_pRichEdit->IsMultiLine()) {
                if (m_dropTextList.size() > 1) {
                    //拖入为多行文本，无效
                    return hr;
                }
            }
            if (m_pRichEdit->IsNumberOnly()) {
                //数字模式
                if (m_dropTextList.size() > 1) {
                    //拖入为多行文本，无效
                    return hr;
                }

                DString dropText = m_dropTextList.front();
                if (!dropText.empty()) {
                    size_t count = dropText.size();
                    for (size_t index = 0; index < count; ++index) {
                        if (dropText[index] == L'\0') {
                            break;
                        }
                        if ((dropText[index] > L'9') || (dropText[index] < L'0')) {
                            //有不是数字的字符，禁止拖入
                            return hr;
                        }
                    }
                }
            }
            DString limitChars = m_pRichEdit->GetLimitChars();
            if (!limitChars.empty()) {
                //有设置限制字符
                for (const DString& dropText: m_dropTextList) {
                    size_t count = dropText.size();
                    for (size_t index = 0; index < count; ++index) {
                        if (dropText[index] == L'\0') {
                            break;
                        }
                        bool bMatch = false;
                        for (const DString::value_type ch : limitChars) {
                            if (ch == dropText[index]) {
                                bMatch = true;
                                break;
                            }
                        }
                        if (!bMatch) {
                            //有不是有效的字符，禁止拖入
                            return hr;
                        }
                    }
                }
            }
        }

        IDropTarget* pDropTarget = nullptr;
        HRESULT txResult = m_pTextServices->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(txResult) && (pDropTarget != nullptr)) {
            //转接给文字服务
            DWORD dwEffect = DROPEFFECT_NONE;
            if (pdwEffect != nullptr) {
                dwEffect = *pdwEffect;
            }
            hr = pDropTarget->DragOver(grfKeyState, POINTL{ pt.x, pt.y }, &dwEffect);
            if (pdwEffect != nullptr) {
                *pdwEffect = dwEffect;
            }
            pDropTarget->Release();
            pDropTarget = nullptr;

            if ((hr == S_OK) && (dwEffect != DROPEFFECT_NONE)) {
                //在成功时，设置当前RichEdit控件的光标到鼠标所在位置，方便查看拖放目标位置
                if (m_pRichEdit != nullptr) {
                    UiPoint clientPt = pt;
                    m_pRichEdit->ScreenToClient(clientPt);
                    if (!m_pRichEdit->IsFocused()) {
                        //必须设置为焦点控件，否则CharFromPos会失败
                        m_pRichEdit->SetFocus();
                    }                    
                    int32_t pos = m_pRichEdit->CharFromPos(clientPt);
                    if (pos >= 0) {
                        UiPoint charPt = m_pRichEdit->PosFromChar(pos);
                        m_pRichEdit->SetCaretPos(charPt.x, charPt.y);
                        m_pRichEdit->ShowCaret(true);
                    }
                }
            }
        }
        return hr;
    }

    //IDropTarget::DragLeave
    virtual int32_t DragLeave(void) override
    {
        m_dropTextList.clear();
        m_dropFileList.clear();

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
        m_dropTextList.clear();
        m_dropFileList.clear();

        HRESULT hr = S_FALSE;

        std::vector<DString> dropTextList;
        std::vector<DString> dropFileList;
        ControlDropTargetImpl_Windows::ParseWindowsDataObject(pDataObj, dropTextList, dropFileList);
        if (m_pRichEdit != nullptr) {
            if (!dropFileList.empty()) {
                //拖入文件操作
                if (!m_pRichEdit->IsEnableDropFile()) {
                    //不支持文件拖放操作
                    return hr;
                }
                //支持文件拖放操作，判断是否满足过滤条件
                DString fileTypes = m_pRichEdit->GetDropFileTypes();
                if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, dropFileList)) {
                    //文件类型不满足过滤条件
                    return hr;
                }

                //移除不支持的文件
                ControlDropTargetUtils::RemoveUnsupportedFiles(dropFileList, fileTypes);
                if (dropFileList.empty()) {
                    return hr;
                }

                ControlDropData_Windows data;
                data.m_pDataObj = pDataObj;
                data.m_grfKeyState = grfKeyState;
                data.m_screenX = pt.x;
                data.m_screenY = pt.y;
                data.m_dwEffect = (pdwEffect != nullptr) ? *pdwEffect : 0;
                data.m_hResult = S_OK;

                data.m_fileList = dropFileList;

                EventArgs msg;
                msg.SetSender(m_pRichEdit);
                msg.eventType = EventType::kEventDropData;
                msg.vkCode = VirtualKeyCode::kVK_None;
                msg.wParam = kControlDropTypeWindows;
                msg.lParam = (LPARAM)&data;
                msg.ptMouse = pt;
                m_pRichEdit->ScreenToClient(msg.ptMouse);
                msg.modifierKey = 0;
                msg.eventData = 0;

                m_pRichEdit->SendEventMsg(msg);
                if (pdwEffect != nullptr) {
                    *pdwEffect = data.m_dwEffect;
                }
                return data.m_hResult;
            }
        }

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
            hr = pDropTarget->Drop((IDataObject*)pDataObj, grfKeyState, POINTL{ pt.x, pt.y }, &dwEffect);
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

    /** 文本数据
    */
    std::vector<DString> m_dropTextList;

    /** 文件路径数据
    */
    std::vector<DString> m_dropFileList;
};

RichEdit::RichEdit(Window* pWindow) :
    ScrollBox(pWindow, new Layout),
    m_pRichHost(nullptr), 
    m_bVScrollBarFixing(false), 
    m_bWantTab(false),
    m_bWantReturn(false),
    m_bWantCtrlReturn(false),
    m_bAllowPrompt(false),
    m_bSelAllEver(false),         
    m_bNoSelOnKillFocus(true), 
    m_bSelAllOnFocus(false),
    m_bHideSelection(false),
    m_bContextMenuShown(false),
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
    m_pFocusedImage(nullptr),
    m_bUseControlCursor(false),
    m_bEnableWheelZoom(false),
    m_bEnableDefaultContextMenu(false),
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
    m_pControlDropTarget(nullptr),
#endif
    m_bDisableTextChangeEvent(false),
    m_maxNumber(INT_MAX),
    m_minNumber(INT_MIN),
    m_pSpinBox(nullptr),
    m_pClearButton(nullptr),
    m_pShowPasswordButton(nullptr),
    m_nFocusBottomBorderSize(0)
{
    //这个标记必须为false，否则绘制有问题
    SetUseCache(false);

    //创建RichEditHost接口
    m_pRichHost = new RichEditHost(this);
    ASSERT(m_pRichHost->GetTextServices() != nullptr);
    m_richCtrl.SetTextServices(m_pRichHost->GetTextServices());

    //初始化是否隐藏选择文本
    m_bHideSelection = m_pRichHost->IsHideSelection();
}

RichEdit::~RichEdit()
{
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
    if (m_pControlDropTarget != nullptr) {
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
            EnableScrollBar(true, GetHScrollBar() != nullptr);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetVScrollBar(true);
            }
        }
        else {
            EnableScrollBar(false, GetHScrollBar() != nullptr);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetVScrollBar(false);
            }
        }
    }
    else if (strName == _T("hscrollbar")) {
        //横向滚动条
        if (strValue == _T("true")) {
            EnableScrollBar(GetVScrollBar() != nullptr, true);
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetHScrollBar(true);
            }
        }
        else {
            EnableScrollBar(GetVScrollBar() != nullptr, false);
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
        SetPasswordMode(strValue == _T("true"));
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
    else if (strName == _T("number_format")) {
        SetNumberFormat64(strValue);
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
    else if ((strName == _T("want_return")) || (strName == _T("want_return_msg")) || (strName == _T("wantreturnmsg"))) {
        SetWantReturn(strValue == _T("true"));
    }
    else if ((strName == _T("want_ctrl_return")) || (strName == _T("return_msg_want_ctrl")) || (strName == _T("returnmsgwantctrl"))) {
        SetWantCtrlReturn(strValue == _T("true"));
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
    else if (strName == _T("hide_selection")) {
        //是否隐藏选择内容
        SetHideSelection(strValue == _T("true"));
    }
    else if (strName == _T("focus_bottom_border_size")) {
        //焦点状态时，底部边框的大小
        SetFocusBottomBorderSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("focus_bottom_border_color")) {
        //焦点状态时，底部边框的颜色
        SetFocusBottomBorderColor(strValue);
    }
    else if (strName == _T("select_all_on_focus")) {
        //获取焦点的时候，是否全选
        SetSelAllOnFocus(strValue == _T("true"));
    }

#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
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
#else
    else if (strName == _T("zoom")) {
        //缩放比例：
        //设置缩放比例：设 wParam：缩放比例的分子，lParam：缩放比例的分母，
        // "wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。
        // 举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50% 
        //UiSize zoomValue;
        //AttributeUtil::ParseSizeValue(strValue.c_str(), zoomValue);
        //if ((zoomValue.cx >= 0) && (zoomValue.cx <= 64) &&
        //    (zoomValue.cy >= 0) && (zoomValue.cy <= 64)) {
        //    m_richCtrl.SetZoom(zoomValue.cx, zoomValue.cy);
        //}
    }    
    else if ((strName == _T("auto_vscroll")) || (strName == _T("autovscroll"))) {
        //当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
        //if (m_pRichHost != nullptr) {
        //    m_pRichHost->SetAutoVScroll(strValue == _T("true"));
        //}
    }
    else if ((strName == _T("auto_hscroll")) || (strName == _T("autohscroll"))) {
        //当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
        //当用户按 Enter 时，控件会将所有文本滚动回零位置。
        //if (m_pRichHost != nullptr) {
        //    m_pRichHost->SetAutoHScroll(strValue == _T("true"));
        //}
    }
    else if ((strName == _T("rich_text")) || (strName == _T("rich"))) {
        //是否为富文本属性
        //SetRichText(strValue == _T("true"));
    }
    else if (strName == _T("auto_detect_url")) {
        //是否自动检测URL，如果是URL则显示为超链接
        //SetAutoURLDetect(strValue == _T("true"));
    }
    else if (strName == _T("allow_beep")) {
        //是否允许发出Beep声音
        //SetAllowBeep(strValue == _T("true"));
    }
    else if (strName == _T("save_selection")) {
        //如果 为 TRUE，则当控件处于非活动状态时，应保存所选内容的边界。
        //如果 为 FALSE，则当控件再次处于活动状态时，可以选择边界重置为 start = 0，length = 0。
        //SetSaveSelection(strValue == _T("true"));
    }
#endif

    //几个SDL版本支持但该版本不支持的属性，需要跳过
    else if (strName == _T("selection_bkcolor")) {
    }
    else if (strName == _T("inactive_selection_bkcolor")) {
    }
    else if (strName == _T("current_row_bkcolor")) {
    }
    else if (strName == _T("inactive_current_row_bkcolor")) {
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
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

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
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

bool RichEdit::IsWantReturn() const
{
    return m_bWantReturn;
}

void RichEdit::SetWantReturn(bool bWantReturn)
{
    m_bWantReturn = bWantReturn;
}

bool RichEdit::IsWantCtrlReturn() const
{
    return m_bWantCtrlReturn;
}

void RichEdit::SetWantCtrlReturn(bool bWantCtrlReturn)
{
    m_bWantCtrlReturn = bWantCtrlReturn;
}

bool RichEdit::IsReadOnly() const
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
}

bool RichEdit::IsPasswordMode() const
{
    if (m_pRichHost != nullptr) {
        return m_pRichHost->IsPassword();
    }
    return false;
}

void RichEdit::SetPasswordMode(bool bPasswordMode)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetPassword(bPasswordMode);
    }
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

void RichEdit::SetPasswordChar(DStringW::value_type ch)
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

bool RichEdit::IsFlashPasswordChar() const
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

void RichEdit::SetNumberFormat64(const DString& numberFormat)
{
    m_numberFormat = numberFormat;
}

DString RichEdit::GetNumberFormat64() const
{
    return m_numberFormat.c_str();
}

bool RichEdit::IsWordWrap() const
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

bool RichEdit::IsMultiLine() const
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

UiFont RichEdit::GetFontInfo() const
{
    CHARFORMAT2W cf = { {0} };
    cf.cbSize = sizeof(CHARFORMAT2W);
    if (IsRichText()) {
        GetSelectionCharFormat(cf);
    }
    else {
        GetDefaultCharFormat(cf);
    }

    UiFont uiFont;
    if (cf.dwMask & CFM_SIZE) {
        HWND hWnd = GetWindowHWND();
        HDC hDC = ::GetDC(hWnd);
        constexpr const int32_t LY_PER_INCH = 1440;
        uiFont.m_fontSize = ::MulDiv(cf.yHeight, ::GetDeviceCaps(hDC, LOGPIXELSY), LY_PER_INCH);
        if (uiFont.m_fontSize < 0) {
            uiFont.m_fontSize = -uiFont.m_fontSize;
        }
        ::ReleaseDC(hWnd, hDC);
    }
    else {
        //富文本模式下，如果混合选择，那么可能不含字体大小信息
        uiFont.m_fontSize = 0;
    }
    if (cf.dwMask & CFM_BOLD) {
        uiFont.m_bBold = cf.dwEffects & CFE_BOLD ? true : false;
    }
    if (cf.dwMask & CFM_ITALIC) {
        uiFont.m_bItalic = cf.dwEffects & CFE_ITALIC ? true : false;
    }
    if (cf.dwMask & CFM_UNDERLINE) {
        uiFont.m_bUnderline = cf.dwEffects & CFE_UNDERLINE ? true : false;
    }
    if (cf.dwMask & CFM_STRIKEOUT) {
        uiFont.m_bStrikeOut = cf.dwEffects & CFE_STRIKEOUT ? true : false;
    }
    if (cf.dwMask & CFM_FACE) {
        uiFont.m_fontName = StringConvert::WStringToT(cf.szFaceName);
    }
    else {
        //富文本模式下，如果混合选择，那么可能不含字体名称信息
        uiFont.m_fontName.clear();
    }
    return uiFont;
}

bool RichEdit::SetFontInfo(const UiFont& fontInfo)
{
    UiFont oldFontInfo = GetFontInfo();
    if (fontInfo == oldFontInfo) {
        return false;
    }

    CHARFORMAT2W charFormat = { {0} };
    charFormat.cbSize = sizeof(CHARFORMAT2W);
    if (IsRichText()) {
        GetSelectionCharFormat(charFormat);
    }
    else {
        GetDefaultCharFormat(charFormat);
    }
    charFormat.dwMask = 0;
    if (!fontInfo.m_fontName.empty() && (oldFontInfo.m_fontName != fontInfo.m_fontName)) {
        charFormat.dwMask |= CFM_FACE;
        DStringW fontName = StringConvert::TToWString(fontInfo.m_fontName.c_str());
        ui::StringUtil::StringCopy(charFormat.szFaceName, fontName.c_str());
    }
    if ((fontInfo.m_fontSize > 0) && (oldFontInfo.m_fontSize != fontInfo.m_fontSize)) {
        charFormat.dwMask |= CFM_SIZE;
        charFormat.yHeight = ConvertToFontHeight(fontInfo.m_fontSize);
    }
    if (oldFontInfo.m_bBold != fontInfo.m_bBold) {
        charFormat.dwMask |= CFM_BOLD;
        if (fontInfo.m_bBold) {
            charFormat.dwEffects |= CFE_BOLD;
        }
        else {
            charFormat.dwEffects &= ~CFE_BOLD;
        }
    }
    if (oldFontInfo.m_bItalic != fontInfo.m_bItalic) {
        charFormat.dwMask |= CFM_ITALIC;
        if (fontInfo.m_bItalic) {
            charFormat.dwEffects |= CFE_ITALIC;
        }
        else {
            charFormat.dwEffects &= ~CFE_ITALIC;
        }
    }
    if (oldFontInfo.m_bUnderline != fontInfo.m_bUnderline) {
        charFormat.dwMask |= CFM_UNDERLINE;
        if (fontInfo.m_bUnderline) {
            charFormat.dwEffects |= CFE_UNDERLINE;
        }
        else {
            charFormat.dwEffects &= ~CFE_UNDERLINE;
        }
    }
    if (oldFontInfo.m_bStrikeOut != fontInfo.m_bStrikeOut) {
        charFormat.dwMask |= CFM_STRIKEOUT;
        if (fontInfo.m_bStrikeOut) {
            charFormat.dwEffects |= CFE_STRIKEOUT;
        }
        else {
            charFormat.dwEffects &= ~CFE_STRIKEOUT;
        }
    }
    if (IsRichText()) {
        SetSelectionCharFormat(charFormat);
    }
    else {
        SetDefaultCharFormat(charFormat);
    }
    ASSERT(GetFontInfo() == fontInfo);
    return true;
}

DString RichEdit::GetCurrentFontId() const
{
    return GetFontId();
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

DString RichEdit::GetSelectionTextColor() const
{
    CHARFORMAT2W cf;
    ZeroMemory(&cf, sizeof(CHARFORMAT2W));
    cf.cbSize = sizeof(CHARFORMAT2W);
    m_richCtrl.GetSelectionCharFormat(cf);
    UiColor dwTextColor;
    dwTextColor.SetFromCOLORREF(cf.crTextColor);
    return GetColorString(dwTextColor);
}

void RichEdit::SetSelectionTextColor(const DString& textColor)
{
    if (!textColor.empty()) {
        UiColor dwTextColor = GetUiColor(textColor);
        CHARFORMAT2W cf;
        ZeroMemory(&cf, sizeof(CHARFORMAT2W));
        cf.cbSize = sizeof(CHARFORMAT2W);
        m_richCtrl.GetSelectionCharFormat(cf);
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = dwTextColor.ToCOLORREF();
        cf.dwEffects &= ~CFE_AUTOCOLOR;
        BOOL bRet = m_richCtrl.SetSelectionCharFormat(cf);
        ASSERT_UNUSED_VARIABLE(bRet);
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
        return StringConvert::WStringToT(m_pLimitChars.get());
    }
    else {
        return DString();
    }
}

void RichEdit::SetLimitChars(const DString& limitChars)
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

int32_t RichEdit::GetTextLength() const
{
    return m_richCtrl.GetTextLengthEx(GTL_DEFAULT, 1200);
}

DString RichEdit::GetText() const
{
    if ((m_pRichHost != nullptr) && m_pRichHost->IsPassword()) {
        //密码模式: 使用底层接口直接获取文本
        return m_pRichHost->GetPasswordText();
    }

    UINT uCodePage = 1200;
    int32_t nTextLen = m_richCtrl.GetTextLengthEx(GTL_DEFAULT, uCodePage);
    if (nTextLen < 1) {
        return DString();
    }
    nTextLen += 1;
    DStringW::value_type* pText = new DStringW::value_type[nTextLen];
    memset(pText, 0, sizeof(DStringW::value_type) * nTextLen);
    m_richCtrl.GetTextEx(pText, nTextLen, GTL_DEFAULT, uCodePage);
    std::wstring sText(pText);
    delete[] pText;
    pText = nullptr;
#ifdef DUILIB_UNICODE
    return sText;
#else
    return StringConvert::WStringToT(sText);
#endif
}

void RichEdit::SetText(const DStringW& strText)
{
    m_bDisableTextChangeEvent = false;
    SetSel(0, -1);

#ifdef DUILIB_UNICODE
    ReplaceSel(strText, FALSE);
#else
    DString text = StringConvert::WStringToT(strText);
    ReplaceSel(text, FALSE);
#endif
}

void RichEdit::SetText(const DStringA& strText)
{
    m_bDisableTextChangeEvent = false;
    SetSel(0, -1);
#ifdef DUILIB_UNICODE
    DString text = StringConvert::UTF8ToT(strText);
    ReplaceSel(text, FALSE);
#else
    ReplaceSel(strText, FALSE);
#endif
}

void RichEdit::SetTextNoEvent(const DString& strText)
{
    m_bDisableTextChangeEvent = true;
    SetSel(0, -1);
    ReplaceSel(strText, FALSE);
    m_bDisableTextChangeEvent = false;
}

void RichEdit::SetTextId(const DString& strTextId)
{
    DString strText = GlobalManager::Instance().Lang().GetStringViaID(strTextId);
    SetText(strText);
}

bool RichEdit::GetModify() const
{ 
    return m_richCtrl.GetModify();
}

void RichEdit::SetModify(bool bModified)
{ 
    m_richCtrl.SetModify(bModified);
}

void RichEdit::GetSel(int32_t& nStartChar, int32_t& nEndChar) const
{
    LONG nStart = 0;
    LONG nEnd = 0;
    m_richCtrl.GetSel(nStart, nEnd);
    nStartChar = nStart;
    nEndChar = nEnd;
}

int32_t RichEdit::SetSel(int32_t nStartChar, int32_t nEndChar)
{
    return m_richCtrl.SetSel(nStartChar, nEndChar);
}

void RichEdit::ReplaceSel(const DString& lpszNewText, bool bCanUndo)
{
#ifdef DUILIB_UNICODE
    m_richCtrl.ReplaceSel(lpszNewText.c_str(), bCanUndo);
#else
    std::wstring newText = StringConvert::TToWString(lpszNewText);
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
    text = StringConvert::WStringToUTF8(textW);
#endif
    return text;
}

int32_t RichEdit::SetSelAll()
{
    return m_richCtrl.SetSelAll();
}

void RichEdit::SetSelNone()
{
    m_richCtrl.SetSelNone();
}

DString RichEdit::GetTextRange(int32_t nStartChar, int32_t nEndChar) const
{
    TEXTRANGEW tr = { {0, 0}, nullptr };
    tr.chrg.cpMin = nStartChar;
    tr.chrg.cpMax = nEndChar;
    LPWSTR lpText = nullptr;
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
    return StringConvert::WStringToT(sText);
#endif
}

void RichEdit::HideSelection(bool bHide, bool bChangeStyle)
{
    m_richCtrl.HideSelection(bHide, bChangeStyle);
}

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

int32_t RichEdit::GetLineCount() const
{
    return m_richCtrl.GetLineCount();
}

DString RichEdit::GetLine(int32_t nIndex, int32_t nMaxLength) const
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
    return StringConvert::WStringToUTF8(sText);
#endif
}

int32_t RichEdit::LineIndex(int32_t nLine) const
{
    return m_richCtrl.LineIndex(nLine);
}

int32_t RichEdit::LineLength(int32_t nLine) const
{
    return m_richCtrl.LineLength(nLine);
}

bool RichEdit::LineScroll(int32_t nLines)
{
    return m_richCtrl.LineScroll(nLines);
}

int32_t RichEdit::LineFromChar(int32_t nIndex) const
{
    return m_richCtrl.LineFromChar((LONG)nIndex);
}

UiPoint RichEdit::PosFromChar(int32_t lChar) const
{ 
    POINT pt = m_richCtrl.PosFromChar(lChar);
    return UiPoint(pt.x, pt.y);
}

int32_t RichEdit::CharFromPos(UiPoint pt) const
{
    POINT ptValue = { pt.x, pt.y };
    return m_richCtrl.CharFromPos(ptValue);
}

void RichEdit::EmptyUndoBuffer()
{
    m_richCtrl.EmptyUndoBuffer();
}

uint32_t RichEdit::SetUndoLimit(uint32_t nLimit)
{
    return m_richCtrl.SetUndoLimit(nLimit);
}

void RichEdit::OnTxNotify(DWORD iNotify, void *pv)
{
    switch(iNotify)
    { 
    case EN_LINK:   
        {
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
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
                LONG nId =  ::GetWindowLong(GetWindowHWND(), GWL_ID);
                NMHDR  *phdr = (NMHDR *)pv;   
                phdr->hwndFrom = GetWindowHWND();
                phdr->idFrom = nId;   
                phdr->code = iNotify;  

                ::SendMessage(GetWindowHWND(), WM_NOTIFY, (WPARAM)nId, (LPARAM)pv);
            }    
        }
        break;
    default:
        break;
    }
}

HWND RichEdit::GetWindowHWND() const
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
                                        nullptr,
                                        nullptr,
                                        TXTNS_FITTOCONTENT,
                                        &szExtent,
                                        &lWidth,
                                        &lHeight);
    }
    sz.cx = (int)lWidth;
    sz.cy = (int)lHeight;
    return sz;
}

void RichEdit::SetTimer(UINT idTimer, UINT uTimeout)
{
    auto timeFlag = m_timerFlagMap.find(idTimer);
    if (timeFlag != m_timerFlagMap.end()) {
        timeFlag->second.Cancel();
    }

    auto callback = [this, idTimer]() {
        m_richCtrl.TxSendMessage(WM_TIMER, idTimer, 0);
    };
    GlobalManager::Instance().Timer().AddTimer(m_timerFlagMap[idTimer].GetWeakFlag(), callback, uTimeout);
}

void RichEdit::KillTimer(UINT idTimer)
{
    auto timeFlag = m_timerFlagMap.find(idTimer);
    if (timeFlag != m_timerFlagMap.end()) {
        timeFlag->second.Cancel();
        m_timerFlagMap.erase(timeFlag);
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
        bRet = BaseClass::ScreenToClient(pt);
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
        bRet = BaseClass::ClientToScreen(pt);
    }
    return bRet;
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void RichEdit::SetScrollPos(UiSize64 szPos)
{
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
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

void RichEdit::LineUp(int32_t /*deltaValue*/, bool /*withAnimation*/)
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L);
}

void RichEdit::LineDown(int32_t /*deltaValue*/, bool /*withAnimation*/)
{
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
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

void RichEdit::EndDown(bool /*arrange*/, bool /*withAnimation*/)
{
    m_richCtrl.TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L);
}

void RichEdit::LineLeft(int32_t /*deltaValue*/)
{
    m_richCtrl.TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L);
}

void RichEdit::LineRight(int32_t /*deltaValue*/)
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
    BaseClass::OnInit();

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

void RichEdit::OnSetEnabled(bool bChanged)
{
    BaseClass::OnSetEnabled(bChanged);
    if (IsEnabled()) {
        SetState(kControlStateNormal);
        UiColor dwTextColor = GetUiColor(GetTextColor());
        SetTextColorInternal(dwTextColor);
    }
    else {
        SetState(kControlStateDisabled);
        UiColor dwTextColor = GetUiColor(GetDisabledTextColor());
        SetTextColorInternal(dwTextColor);
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

    if (iWidth > 0) {
        iWidth += (rcPadding.left + rcPadding.right);
        iWidth += (rcTextPadding.left + rcTextPadding.right);
    }

    if (iHeight > 0) {
        iHeight += (rcPadding.top + rcPadding.bottom);
        iHeight += (rcTextPadding.top + rcTextPadding.bottom);
    }

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
        int32_t nHScrollbarHeight = 0; //横向滚动条的高度
        UiRect rcScrollBarPos(rc.right, rc.top, rc.right + pVScrollBar->GetFixedWidth().GetInt32(), rc.bottom);
        if ((pHScrollBar != nullptr) && pHScrollBar->IsValid()) {
            //纵向滚动条的底部，需要到容器的底部
            nHScrollbarHeight = pHScrollBar->GetFixedHeight().GetInt32();
            rcScrollBarPos.bottom += nHScrollbarHeight;
        }
        pVScrollBar->SetHScrollbarHeight(nHScrollbarHeight);
        pVScrollBar->SetPos(rcScrollBarPos);
    }
    if (pHScrollBar != nullptr && pHScrollBar->IsValid()) {
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
            BaseClass::HandleEvent(msg);
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
            uint32_t nZoomPercent = 100;
            if ((nNum > 0) && (nDen > 0)) {
                nZoomPercent = nNum * 100 / nDen;
            }
            SendEvent(kEventZoom, (WPARAM)nZoomPercent, 0);
        }
        else {
            ScrollBox::HandleEvent(msg);
        }
        return;
    }

    if (msg.eventType == kEventMouseButtonDown) {
        OnMouseMessage(WM_LBUTTONDOWN, msg);
        CheckSelAllOnFocus();
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {        
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
        CheckSelAllOnFocus();
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
        return BaseClass::OnSetCursor(msg);
    }

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
        m_pRichHost->OnTxInPlaceActivate(nullptr);
        m_richCtrl.TxSendMessage(WM_SETFOCUS, 0, 0);
        ShowCaret(true);
    }

    //获得焦点时，打开输入法
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        bool bEnableIME = IsVisible() && !IsReadOnly() && IsEnabled();
        pWindow->NativeWnd()->SetImeOpenStatus(bEnableIME);
    }

    if ((m_pClearButton != nullptr) && !IsReadOnly()){
        m_pClearButton->SetFadeVisible(true);
    }
    if ((m_pShowPasswordButton != nullptr) && IsPasswordMode() && !IsShowPassword()) {
        m_pShowPasswordButton->SetFadeVisible(true);
    }

    //不调用基类的方法
    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHot);
    }

    //确保获取焦点时，需要显示选择状态，否则选择的文本状态无法正常看到
    if (!IsReadOnly() && IsEnabled()) {
        if (IsHideSelection()) {
            if (m_pRichHost != nullptr) {
                m_pRichHost->SetHideSelection(false);
            }
        }
    }
    Invalidate();
    return true;
}

bool RichEdit::OnKillFocus(const EventArgs& msg)
{
    if (m_pRichHost) {
        m_pRichHost->OnTxInPlaceDeactivate();
        m_richCtrl.TxSendMessage(WM_KILLFOCUS, 0, 0);
        ShowCaret(false);
    }

    m_bSelAllEver = false;
    if (m_bNoSelOnKillFocus && IsReadOnly() && IsEnabled()) {
        SetSelNone();
    }

    if (m_pClearButton != nullptr) {
        m_pClearButton->SetFadeVisible(false);
    }
    if (m_pShowPasswordButton != nullptr) {
        m_pShowPasswordButton->SetFadeVisible(false);
    }

    if (!m_bContextMenuShown) {
        //失去焦点时，同步选择文本状态
        if ((m_pRichHost != nullptr) && (m_bHideSelection != m_pRichHost->IsHideSelection())) {
            m_pRichHost->SetHideSelection(m_bHideSelection);
        }
    }
    return BaseClass::OnKillFocus(msg);
}

void RichEdit::CheckSelAllOnFocus()
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
        if (!IsInLimitChars((DStringW::value_type)msg.vkCode)) {
            //字符不在列表里面，禁止输入
            return true;
        }
    }
#ifdef DUILIB_UNICODE
    WPARAM wParam = msg.wParam;
    WPARAM lParam = msg.lParam;
    if (msg.modifierKey & ModifierKey::kIsSystemKey) {
        m_richCtrl.TxSendMessage(WM_SYSCHAR, wParam, lParam);
    }
    else {
        m_richCtrl.TxSendMessage(WM_CHAR, wParam, lParam);
    }    
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
            DStringW::value_type chWideChar[4] = {0, };
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

bool RichEdit::IsInLimitChars(DStringW::value_type charValue) const
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
    if (msg.vkCode == kVK_RETURN && ::GetKeyState(VK_SHIFT) >= 0) {
        bool bCtrlDown = (::GetKeyState(VK_CONTROL) < 0);
        if (bCtrlDown && !m_bWantCtrlReturn) {
            //按了Ctrl + 回车键
            SendEvent(kEventReturn);
            return true;
        }
        else if (!m_bWantReturn) {
            //按了回车键
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
    m_richCtrl.TxSendMessage(WM_KEYDOWN, wParam, lParam);
    if (msg.vkCode == kVK_ESCAPE) {
        //按了ESC键
        SendEvent(kEventEsc);
    }
    return true;
}

bool RichEdit::OnImeStartComposition(const EventArgs& /*msg*/)
{
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

void RichEdit::OnMouseMessage(uint32_t uMsg, const EventArgs& msg)
{
    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    WPARAM wParam = msg.wParam;
    m_richCtrl.TxSendMessage(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
}

void RichEdit::Paint(IRender* pRender, const UiRect& rcPaint)
{
    PerformanceStat statPerformance(_T("PaintWindow, RichEdit::Paint"));
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

    if (bNeedPaint) {
#if !defined (DUILIB_RICH_EDIT_DRAW_OPT) 
        HDC hdc = pRender->GetRenderDC(GetWindow()->NativeWnd()->GetHWND());
#else
        HDC hdc = nullptr;
#endif
        if(hdc != nullptr){
            RECT paintRect = { rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom };
            pTextServices->TxDraw(DVASPECT_CONTENT,     // Draw Aspect
                                  /*-1*/0,              // Lindex
                                  nullptr,              // Info for drawing optimazation
                                  nullptr,              // target device information
                                  hdc,                  // Draw device HDC
                                  nullptr,              // Target device HDC
                                  (RECTL*)&rc,          // Bounding client rectangle
                                  nullptr,              // Clipping rectangle for metafiles
                                  &paintRect,           // Update rectangle
                                  nullptr,              // Call back function
                                  0,                    // Call back parameter
                                  0);                   // What view of the object

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
        for (const UiRect& clipRect : clipRects) {
            UiRect rcCheck = rcUpdate;
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


    constexpr const int32_t nColorBits = sizeof(uint32_t); //每个颜色点所占字节数
    uint8_t* pRowStart = nullptr; //每行Alpha通道值起始的位置
    uint8_t* pRowEnd = nullptr;   //每行Alpha通道值结束的位置

    for (int32_t i = nTop; i < nBottom; ++i) {
        pRowStart = (uint8_t*)pBitmapBits + (i * nWidth + nLeft) * nColorBits + 3;
        pRowEnd = (uint8_t*)pBitmapBits + (i * nWidth + nRight) * nColorBits;
        while (pRowStart < pRowEnd) {
            *pRowStart = 0;
            pRowStart += 4;
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

            HRGN hRgn = ::ExtCreateRegion(nullptr, (DWORD)nSize, rgnData);
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
    pTextServices->TxDraw(DVASPECT_CONTENT,     // Draw Aspect
                            /*-1*/0,            // Lindex
                            nullptr,            // Info for drawing optimazation
                            nullptr,            // target device information
                            hDrawDC,            // Draw device HDC
                            nullptr,            // Target device HDC
                            &rcBitmap,          // Bounding client rectangle
                            nullptr,            // Clipping rectangle for metafiles
                            &rectUpdate,        // Update rectangle
                            nullptr,            // Call back function
                            0,                  // Call back parameter
                            0);                 // What view of the object

    //恢复Alpha(绘制过程中，会导致绘制区域部分的Alpha通道出现异常)
    for (int32_t i = nTop; i < nBottom; ++i) {
        pRowStart = (uint8_t*)pBitmapBits + (i * nWidth + nLeft) * nColorBits + 3;
        pRowEnd = (uint8_t*)pBitmapBits + (i * nWidth + nRight) * nColorBits;
        while (pRowStart < pRowEnd) {
            if (*pRowStart == 0) {
                *pRowStart = 255;
            }
            pRowStart += 4;
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

void RichEdit::PaintBorder(IRender* pRender)
{
    BaseClass::PaintBorder(pRender);
    if (!IsFocused() || IsReadOnly() || !IsEnabled()) {
        return;
    }
    //绘制下边线
    DString borderColor = GetFocusBottomBorderColor();
    int32_t borderSize = GetFocusBottomBorderSize();
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

void RichEdit::CreateCaret(int32_t xWidth, int32_t yHeight)
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

void RichEdit::GetCaretSize(int32_t& xWidth, int32_t& yHeight) const
{
    xWidth = m_iCaretWidth;
    yHeight = m_iCaretHeight;
}

void RichEdit::ShowCaret(bool fShow)
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
}

void RichEdit::SetCaretColor(const DString& dwColor)
{
    m_sCaretColor = dwColor;
}

DString RichEdit::GetCaretColor() const
{
    return m_sCaretColor.c_str();
}

UiRect RichEdit::GetCaretRect() const
{
    UiRect rc = { m_iCaretPosX, m_iCaretPosY, m_iCaretPosX + m_iCaretWidth, m_iCaretPosY + m_iCaretHeight };
    return rc;
}

void RichEdit::SetCaretPos(int32_t xPos, int32_t yPos)
{
    m_iCaretPosX = xPos;
    m_iCaretPosY = yPos;
    ShowCaret(!m_richCtrl.HasSelText());
}

void RichEdit::GetCaretPos(int32_t& xPos, int32_t& yPos) const
{
    xPos = m_iCaretPosX;
    yPos = m_iCaretPosY;
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
    std::string strOut = StringConvert::TToUTF8(GetPromptText());
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
    DString strOut = StringConvert::UTF8ToT(strText);
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
    DString strOut = StringConvert::UTF8ToT(strTextId);
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
        BaseClass::PaintStateImages(pRender);
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

void RichEdit::ClearImageCache()
{
    BaseClass::ClearImageCache();
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
    BOOL ret = ::OpenClipboard(nullptr);
    if(ret) {
        if(::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
            if(h != INVALID_HANDLE_VALUE) {
                DStringW::value_type* buf = (DStringW::value_type*)::GlobalLock(h);
                if(buf != nullptr)    {
                    DStringW str(buf, GlobalSize(h)/sizeof(DStringW::value_type));
                    out = str;
                    ::GlobalUnlock(h);
                }
            }
        }
        else if(::IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE h = ::GetClipboardData(CF_TEXT);
            if(h != INVALID_HANDLE_VALUE) {
                char* buf = (char*)::GlobalLock(h);
                if(buf != nullptr)    {
                    std::string str(buf, GlobalSize(h));
                    out = StringConvert::MBCSToUnicode(str);
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
    if ((pRichEdit == nullptr) || !pRichEdit->IsEnabled() || pRichEdit->IsPasswordMode()) {
        return;
    }

    //如果没有选中文本，则将光标切换到当前点击的位置
    int32_t nStartChar = 0; 
    int32_t nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    if (nStartChar == nEndChar) {
        int32_t pos = pRichEdit->m_richCtrl.CharFromPos(POINT{ point.x, point.y });
        if (pos > 0) {
            pRichEdit->SetSel(pos, pos);
            pRichEdit->GetSel(nStartChar, nEndChar);
        }
    }
    
    DString skinFolder = _T("public/menu/");
    Menu* menu = new Menu(GetWindow());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(skinFolder);
    DString xml(_T("rich_edit_menu.xml"));

    //菜单显示过程中，不隐藏当前选择的文本
    m_bContextMenuShown = true;
    if ((m_pRichHost != nullptr) && m_pRichHost->IsHideSelection()) {
        m_pRichHost->SetHideSelection(false);
    }
    //菜单关闭事件
    std::weak_ptr<WeakFlag> richEditFlag = GetWeakFlag();
    menu->AttachWindowClose([this, richEditFlag](const ui::EventArgs&) {
        if (!richEditFlag.expired()) {
            m_bContextMenuShown = false;
            //恢复HideSelection属性
            if((m_pRichHost != nullptr) && (m_pRichHost->IsHideSelection() != m_bHideSelection) && !IsFocused()) {
                m_pRichHost->SetHideSelection(m_bHideSelection);
            }
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
        wcscpy_s(cf.szFaceName, StringConvert::TToWString(pFont->FontName()).c_str());
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

bool RichEdit::FindRichText(const FindTextParam& findParam, TextCharRange& chrgText) const
{
    DWORD dwFlags = findParam.bMatchCase ? FR_MATCHCASE : 0;
    dwFlags |= findParam.bMatchWholeWord ? FR_WHOLEWORD : 0;
    dwFlags |= findParam.bFindDown ? FR_DOWN : 0;
    FINDTEXTEXW ft;
    ZeroMemory(&ft, sizeof(ft));
    ft.chrg.cpMin = findParam.chrg.cpMin;
    ft.chrg.cpMax = findParam.chrg.cpMax;
    DStringW findText = StringConvert::TToWString(findParam.findText);
    ft.lpstrText = findText.c_str();
    LONG nIndex = FindRichText(dwFlags, ft);
    if (nIndex != -1) {
        //查找到了内容，选择所查到的内容
        chrgText.cpMin = ft.chrgText.cpMin;
        chrgText.cpMax = ft.chrgText.cpMax;
        return true;
    }
    return false;
}

void RichEdit::SetHideSelection(bool fHideSelection)
{
    m_bHideSelection = fHideSelection;//记录状态
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetHideSelection(fHideSelection);
    }
}

bool RichEdit::IsHideSelection() const
{
    bool bRet = false;
    if (m_pRichHost != nullptr) {
        bRet = m_pRichHost->IsHideSelection();
    }
    return bRet;
}

void RichEdit::SetFocusBottomBorderSize(int32_t nBottomBorderSize)
{
    ASSERT(nBottomBorderSize > 0);
    if (nBottomBorderSize < 0) {
        nBottomBorderSize = 0;
    }
    m_nFocusBottomBorderSize = ui::TruncateToUInt8(nBottomBorderSize);
}

int32_t RichEdit::GetFocusBottomBorderSize() const
{
    return (int32_t)(uint32_t)m_nFocusBottomBorderSize;
}

void RichEdit::SetFocusBottomBorderColor(const DString& bottomBorderColor)
{
    m_sFocusBottomBorderColor = bottomBorderColor;
}

DString RichEdit::GetFocusBottomBorderColor() const
{
    return m_sFocusBottomBorderColor.c_str();
}

void RichEdit::SetEnableDragDrop(bool bEnable)
{
    BaseClass::SetEnableDragDrop(bEnable);
    ASSERT(m_pRichHost != nullptr);
    if (m_pRichHost == nullptr) {
        return;
    }
    if (bEnable) {
        m_pControlDropTarget = new RichEditDropTarget(this, m_pRichHost->GetTextServices());
    }
    else {
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

ControlDropTarget_Windows* RichEdit::GetControlDropTarget()
{
    if (IsReadOnly() || IsPasswordMode() || !IsEnabled()) {
        //只读模式、密码模式、不可用模式，关闭拖放功能
        return nullptr;
    }
    return m_pControlDropTarget;
}

ControlDropTarget_SDL* RichEdit::GetControlDropTarget_SDL()
{
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DUILIB_RICHEDIT_SUPPORT_RICHTEXT
void RichEdit::SetSaveSelection(bool fSaveSelection)
{
    if (m_pRichHost != nullptr) {
        m_pRichHost->SetSaveSelection(fSaveSelection);
    }
}

void RichEdit::AddColorText(const DString& str, const DString& color)
{
    if (!IsRichText() || str.empty() || color.empty()) {
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

void RichEdit::AddLinkColorTextEx(const DString& str, const DString& color, const DString& linkInfo, const DString& strFontId)
{
    if (!IsRichText() || str.empty() || color.empty()) {
        ASSERT(FALSE);
        return;
    }

    std::string link = StringConvert::TToMBCS(linkInfo);
    std::string text = StringConvert::TToMBCS(str);
    std::string font_face;

    CHARFORMAT2W cf;
    GetCharFormat(strFontId, cf);
    font_face = StringConvert::UnicodeToMBCS(cf.szFaceName);
    UiColor dwTextColor = GlobalManager::Instance().Color().GetColor(color);
    static std::string font_format = "{\\fonttbl{\\f0\\fnil\\fcharset%d %s;}}";
    static std::string color_format = "{\\colortbl ;\\red%d\\green%d\\blue%d;}";
    static std::string link_format = "{\\rtf1%s%s\\f0\\fs%d{\\field{\\*\\fldinst{HYPERLINK \"%s\"}}{\\fldrslt{\\cf1 %s}}}}";
    char sfont[255] = { 0 };
    sprintf_s(sfont, font_format.c_str(), cf.bCharSet, font_face.c_str());
    char scolor[255] = { 0 };
    sprintf_s(scolor, color_format.c_str(), dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB());
    char slinke[1024] = { 0 };
    sprintf_s(slinke, link_format.c_str(), sfont, scolor, ((int)(cf.yHeight * 1.5)) / 2 * 2, link.c_str(), text.c_str());

    SETTEXTEX st;
    st.codepage = ((UINT32)~((UINT32)0));
    st.flags = ST_SELECTION | ST_KEEPUNDO;
    m_richCtrl.TxSendMessage(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)(LPCTSTR)slinke);
    return;
}

bool RichEdit::IsRichText() const
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

int32_t RichEdit::SetSel(CHARRANGE& cr)
{
    return m_richCtrl.SetSel(cr);
}
void RichEdit::GetSel(CHARRANGE& cr) const
{
    m_richCtrl.GetSel(cr);
}

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

void RichEdit::SetZoomPercent(uint32_t nZoomPercent)
{
    bool bZoomIn = nZoomPercent > GetZoomPercent();
    //缩放百分比的最大值
    const uint32_t MAX_ZOOM_PERCENT = 800;
    if (nZoomPercent > MAX_ZOOM_PERCENT) {
        nZoomPercent = MAX_ZOOM_PERCENT;
    }
    if ((nZoomPercent == 0) || (nZoomPercent == 100)) {
        m_richCtrl.SetZoomOff();
    }
    else {
        for (int32_t nNum = 1; nNum < 64; ++nNum) {
            for (int32_t nDen = 1; nDen < 64; ++nDen) {
                if ((nNum * 100 / nDen) == (int32_t)nZoomPercent) {
                    m_richCtrl.SetZoom(nNum, nDen);
                    return;
                }
            }
        }
        //无法精准匹配的时候，估算一个值
        int32_t nDen = 63;
        int32_t nNum = (int32_t)(bZoomIn ? ui::CEILF(nZoomPercent * nDen / 100.0f) : (nZoomPercent * nDen / 100.0f));
        while (nNum > 63) {
            --nDen;            
            if (nDen < 1) {
                nDen = 1;
                nNum = (int32_t)(bZoomIn ? ui::CEILF(nZoomPercent * nDen / 100.0f) : (nZoomPercent * nDen / 100.0f));
                break;
            }
            else {
                nNum = (int32_t)(bZoomIn ? ui::CEILF(nZoomPercent * nDen / 100.0f) : (nZoomPercent * nDen / 100.0f));
            }
        }
        m_richCtrl.SetZoom(nNum, nDen);
    }
}

uint32_t RichEdit::GetZoomPercent() const
{
    uint32_t nZoomPercent = 100;
    int32_t nNum = 0;
    int32_t nDen = 0;
    GetZoom(nNum, nDen);
    if ((nNum > 0) && (nDen > 0)) {
        nZoomPercent = nNum * 100 / nDen;
    }
    return nZoomPercent;
}

WORD RichEdit::GetSelectionType() const
{
    return m_richCtrl.GetSelectionType();
}

int32_t RichEdit::FindRichText(DWORD dwFlags, FINDTEXTW& ft) const
{
    return (int32_t)m_richCtrl.FindTextW(dwFlags, ft);
}

int32_t RichEdit::FindRichText(DWORD dwFlags, FINDTEXTEXW& ft) const
{
    return (int32_t)m_richCtrl.FindTextW(dwFlags, ft);
}

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

DWORD RichEdit::GetEventMask() const
{
    return m_richCtrl.GetEventMask();
}

DWORD RichEdit::SetEventMask(DWORD dwEventMask)
{
    return m_richCtrl.SetEventMask(dwEventMask);
}

void RichEdit::ScrollCaret()
{
    m_richCtrl.ScrollCaret();
}

int32_t RichEdit::InsertText(int32_t nInsertAfterChar, const DString& text, bool bCanUndo)
{
#ifdef DUILIB_UNICODE
    return m_richCtrl.InsertText(nInsertAfterChar, text.c_str(), bCanUndo);
#else
    return m_richCtrl.InsertText(nInsertAfterChar, StringConvert::TToWString(text).c_str(), bCanUndo);
#endif
}

int32_t RichEdit::AppendText(const DString& text, bool bCanUndo, bool bScrollBottom)
{
    int32_t nRet = -1;
#ifdef DUILIB_UNICODE
    nRet = m_richCtrl.AppendText(text.c_str(), bCanUndo);
#else
    nRet = m_richCtrl.AppendText(StringConvert::TToWString(text).c_str(), bCanUndo);
#endif
    if (bScrollBottom) {
        int64_t nScrollRangeY = GetScrollRange().cy;
        if (nScrollRangeY > 0) {
            SetScrollPosY(nScrollRangeY);
        }
    }
    return nRet;
}

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

bool RichEdit::SetWordCharFormat(CHARFORMAT2W& cf)
{
    return m_richCtrl.SetWordCharFormat(cf);
}

DWORD RichEdit::GetParaFormat(PARAFORMAT2& pf) const
{
    return m_richCtrl.GetParaFormat(pf);
}

bool RichEdit::SetParaFormat(PARAFORMAT2& pf)
{
    if (m_richCtrl.SetParaFormat(pf)) {
        return true;
    }
    return false;
}

long RichEdit::StreamIn(UINT nFormat, EDITSTREAM& es)
{
    return m_richCtrl.StreamIn(nFormat, es);
}

long RichEdit::StreamOut(UINT nFormat, EDITSTREAM& es)
{
    return m_richCtrl.StreamOut(nFormat, es);
}

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

#endif //DUILIB_RICHEDIT_SUPPORT_RICHTEXT

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
