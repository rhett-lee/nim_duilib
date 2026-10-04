#include "MessageBoxWnd.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

MessageBoxWnd::MessageBoxWnd() :
    m_buttonFlags(kButtonsOK),
    m_iconType(kIconNone),
    m_pTextLabel(nullptr),
    m_pTitleLabel(nullptr),
    m_pIconControl(nullptr),
    m_bTextId(false)
{
}

MessageBoxWnd::~MessageBoxWnd()
{
}

DString MessageBoxWnd::GetSkinFolder()
{
    return _T("public/messagebox");
}

DString MessageBoxWnd::GetSkinFile()
{
    return _T("messagebox.xml");
}

int32_t MessageBoxWnd::Show(ui::Window* pParentWindow,
                             const DString& text,
                             const DString& title,
                             uint32_t buttonFlags,
                             IconType iconType,
                             const ButtonText* pButtonText,
                             bool bTextId)
{
    //参数兜底：未指定任何按钮时，至少显示"确定"
    if ((buttonFlags & (kButtonOK | kButtonCancel | kButtonYes | kButtonNo | kButtonRetry)) == 0) {
        buttonFlags = kButtonsOK;
    }

    //DoModal 窗口关闭时不会自动删除对象，需要调用方自己管理生命周期
    MessageBoxWnd* pMsgBox = new MessageBoxWnd();
    pMsgBox->m_text = text;
    pMsgBox->m_title = title;
    pMsgBox->m_buttonFlags = buttonFlags;
    pMsgBox->m_iconType = iconType;
    if (pButtonText != nullptr) {
        pMsgBox->m_buttonText = *pButtonText;
    }
    pMsgBox->m_bTextId = bTextId;

    //居中显示，标题同时用于任务栏
    ui::WindowCreateParam createParam(title, true, bTextId ? title : _T(""));

    //bCloseByEsc=true：ESC 关闭，DoModal 返回 kWindowCloseCancel(2)
    //bCloseByEnter=true：Enter 关闭，DoModal 返回 kWindowCloseOK(1)，再按按钮组合归一化
    int32_t nResult = pMsgBox->DoModal(pParentWindow, createParam, true, true);
    delete pMsgBox;

    if (nResult < 0) {
        return nResult;
    }
    return NormalizeResult(nResult, buttonFlags);
}

void MessageBoxWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pTitleLabel = dynamic_cast<ui::Label*>(FindControl(_T("msg_title")));
    if (m_pTitleLabel != nullptr) {
        if (m_bTextId) {
            m_pTitleLabel->SetTextId(m_title);
        }
        else {
            m_pTitleLabel->SetText(m_title);
        }        
    }

    m_pTextLabel = dynamic_cast<ui::Label*>(FindControl(_T("msg_text")));
    if (m_pTextLabel != nullptr) {
        if (m_bTextId) {
            m_pTextLabel->SetTextId(m_text);
        }
        else {
            m_pTextLabel->SetText(m_text);
        }        
    }

    m_pIconControl = FindControl(_T("msg_icon"));
    if (m_pIconControl != nullptr) {
        DString strIcon;
        switch (m_iconType) {
        case kIconInfo:
            strIcon = _T("file='msg_info.svg'");
            break;
        case kIconWarning:
            strIcon = _T("file='msg_warning.svg'");
            break;
        case kIconError:
            strIcon = _T("file='msg_error.svg'");
            break;
        case kIconQuestion:
            strIcon = _T("file='msg_question.svg'");
            break;
        default:
            break;
        }
        if (strIcon.empty()) {
            m_pIconControl->SetVisible(false);
        }
        else {
            m_pIconControl->SetBkImage(strIcon);
        }
    }

    InitButtons();
}

void MessageBoxWnd::InitButtons()
{
    //默认按钮文字（从框架多语言资源表读取，缺失时回退为中文）
    if (m_buttonText.ok.empty()) {
        if (m_bTextId) {
            m_buttonText.ok = _T("STRID_MSGBOX_OK");
        }
        else {
            m_buttonText.ok = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_OK"));
        }        
        if (m_buttonText.ok.empty()) { m_buttonText.ok = _T("Ok"); }
    }
    if (m_buttonText.cancel.empty()) {
        if (m_bTextId) {
            m_buttonText.cancel = _T("STRID_MSGBOX_CANCEL");
        }
        else {
            m_buttonText.cancel = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_CANCEL"));
        }        
        if (m_buttonText.cancel.empty()) { m_buttonText.cancel = _T("Cancel"); }
    }
    if (m_buttonText.yes.empty()) {
        if (m_bTextId) {
            m_buttonText.yes = _T("STRID_MSGBOX_YES");
        }
        else {
            m_buttonText.yes = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_YES"));
        }        
        if (m_buttonText.yes.empty()) { m_buttonText.yes = _T("Yes"); }
    }
    if (m_buttonText.no.empty()) {
        if (m_bTextId) {
            m_buttonText.no = _T("STRID_MSGBOX_NO");
        }
        else {
            m_buttonText.no = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_NO"));
        }        
        if (m_buttonText.no.empty()) { m_buttonText.no = _T("No"); }
    }
    if (m_buttonText.retry.empty()) {
        if (m_bTextId) {
            m_buttonText.retry = _T("STRID_MSGBOX_RETRY");
        }
        else {
            m_buttonText.retry = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RETRY"));
        }        
        if (m_buttonText.retry.empty()) { m_buttonText.retry = _T("Retry"); }
    }

    //按钮定义：显示顺序与 XML 中的排列顺序一致
    struct ButtonInit
    {
        const DString* pName;
        uint32_t flag;
        const DString* pText;
        int32_t result;
    };
    const DString nameRetry  = _T("btn_retry");
    const DString nameYes    = _T("btn_yes");
    const DString nameNo     = _T("btn_no");
    const DString nameOK     = _T("btn_ok");
    const DString nameCancel = _T("btn_cancel");

    const ButtonInit buttons[] = {
        { &nameRetry,  kButtonRetry,  &m_buttonText.retry,  kResultRetry  },
        { &nameYes,    kButtonYes,    &m_buttonText.yes,    kResultYes    },
        { &nameNo,     kButtonNo,     &m_buttonText.no,     kResultNo     },
        { &nameOK,     kButtonOK,     &m_buttonText.ok,     kResultOK     },
        { &nameCancel, kButtonCancel, &m_buttonText.cancel, kResultCancel }
    };

    const int32_t nDefaultResult = GetDefaultResult();
    ui::Button* pDefaultButton = nullptr;

    for (const ButtonInit& item : buttons) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(*item.pName));
        if (pButton == nullptr) {
            continue;
        }
        if ((m_buttonFlags & item.flag) == 0) {
            //不在当前按钮组合中：隐藏且不响应
            pButton->SetVisible(false);
            continue;
        }

        //XML 中按钮默认隐藏（避免全部闪现），命中组合时在此显示
        pButton->SetVisible(true);
        if (m_bTextId) {
            if (GlobalManager::Instance().Lang().HasStringByID(*item.pText)) {
                pButton->SetTextId(*item.pText);
            }
            else {
                pButton->SetText(*item.pText);
            }
        }
        else {
            pButton->SetText(*item.pText);
        }        
        if (item.result == nDefaultResult) {
            //默认按钮使用蓝色高亮样式
            pButton->SetClass(_T("btn_global_blue_80x30"));
            pDefaultButton = pButton;
        }
        else {
            pButton->SetClass(_T("btn_global_white_80x30"));
        }

        pButton->AttachClick([this, item](const ui::EventArgs& /*args*/) {
            CloseWnd(item.result);
            return true;
        });
    }

    //默认按钮获得焦点，直接按 Enter 即等价于点击它
    if (pDefaultButton != nullptr) {
        pDefaultButton->SetFocus();
    }
}

int32_t MessageBoxWnd::GetDefaultResult() const
{
    if ((m_buttonFlags & kButtonOK) != 0) {
        return kResultOK;
    }
    if ((m_buttonFlags & kButtonYes) != 0) {
        return kResultYes;
    }
    if ((m_buttonFlags & kButtonRetry) != 0) {
        return kResultRetry;
    }
    return kResultOK;
}

int32_t MessageBoxWnd::NormalizeResult(int32_t nResult, uint32_t buttonFlags)
{
    //明确点击的按钮：重试/是/否/取消，直接返回
    if ((nResult == kResultRetry) || (nResult == kResultYes) || (nResult == kResultNo)) {
        return nResult;
    }

    //Enter 键返回 kResultOK(1)：组合中没有"确定"时，映射为默认按钮
    if (nResult == kResultOK) {
        if ((buttonFlags & kButtonOK) != 0) {
            return kResultOK;
        }
        if ((buttonFlags & kButtonYes) != 0) {
            return kResultYes;
        }
        if ((buttonFlags & kButtonRetry) != 0) {
            return kResultRetry;
        }
        return kResultOK;
    }

    //ESC 键(kWindowCloseCancel=2)或标题栏关闭(kWindowCloseNormal=0)：
    //有"取消"按钮按取消处理，否则按默认按钮处理
    if ((buttonFlags & kButtonCancel) != 0) {
        return kResultCancel;
    }
    if ((buttonFlags & kButtonOK) != 0) {
        return kResultOK;
    }
    if ((buttonFlags & kButtonYes) != 0) {
        return kResultYes;
    }
    if ((buttonFlags & kButtonRetry) != 0) {
        return kResultRetry;
    }
    return kResultCancel;
}

} //namespace ui
