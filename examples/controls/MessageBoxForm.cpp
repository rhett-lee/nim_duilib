#include "MessageBoxForm.h"

MessageBoxForm::MessageBoxForm() :
    m_pResultLabel(nullptr),
    m_lastResult(0),
    m_bHasResult(false)
{
}

MessageBoxForm::~MessageBoxForm()
{
}

DString MessageBoxForm::GetSkinFolder()
{
    return _T("controls");
}

DString MessageBoxForm::GetSkinFile()
{
    return _T("message_box_test.xml");
}

void MessageBoxForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pResultLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_result")));

    //演示项（字符串按 text_id 取翻译，语言切换后实时生效）
    struct DemoItem
    {
        DString buttonName;
        DString textId;
        DString titleId;
        uint32_t buttonFlags;
        ui::MessageBoxWnd::IconType iconType;
    };
    const DemoItem demos[] = {
        { _T("btn_demo_ok"),
          _T("STRID_MSGBOX_DEMO_OK_TEXT"),
          _T("STRID_MSGBOX_DEMO_OK_TITLE"),
          ui::MessageBoxWnd::kButtonsOK,
          ui::MessageBoxWnd::kIconInfo },
        { _T("btn_demo_ok_cancel"),
          _T("STRID_MSGBOX_DEMO_OK_CANCEL_TEXT"),
          _T("STRID_MSGBOX_DEMO_OK_CANCEL_TITLE"),
          ui::MessageBoxWnd::kButtonsOKCancel,
          ui::MessageBoxWnd::kIconQuestion },
        { _T("btn_demo_yes_no"),
          _T("STRID_MSGBOX_DEMO_YES_NO_TEXT"),
          _T("STRID_MSGBOX_DEMO_YES_NO_TITLE"),
          ui::MessageBoxWnd::kButtonsYesNo,
          ui::MessageBoxWnd::kIconQuestion },
        { _T("btn_demo_yes_no_cancel"),
          _T("STRID_MSGBOX_DEMO_YES_NO_CANCEL_TEXT"),
          _T("STRID_MSGBOX_DEMO_YES_NO_CANCEL_TITLE"),
          ui::MessageBoxWnd::kButtonsYesNoCancel,
          ui::MessageBoxWnd::kIconWarning },
        { _T("btn_demo_retry_cancel"),
          _T("STRID_MSGBOX_DEMO_RETRY_CANCEL_TEXT"),
          _T("STRID_MSGBOX_DEMO_RETRY_CANCEL_TITLE"),
          ui::MessageBoxWnd::kButtonsRetryCancel,
          ui::MessageBoxWnd::kIconError }
    };

    for (const DemoItem& item : demos) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(item.buttonName));
        if (pButton == nullptr) {
            continue;
        }
        DemoItem itemCopy = item;
        pButton->AttachClick([this, itemCopy](const ui::EventArgs& /*args*/) {
            DString textId = itemCopy.textId;
            DString titleId = itemCopy.titleId;
            int32_t nResult = ui::MessageBoxWnd::Show(this, textId, titleId,
                                                       itemCopy.buttonFlags, itemCopy.iconType, nullptr, true);
            ShowResult(nResult);
            return true;
        });
    }

    //四种图标连续弹出
    ui::Button* pIconButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_demo_icons")));
    if (pIconButton != nullptr) {
        pIconButton->AttachClick([this](const ui::EventArgs&) {
            const DString textId = _T("STRID_MSGBOX_DEMO_ICONS_TEXT");
            int32_t nResult = ui::MessageBoxWnd::Show(this, textId,
                                                     _T("STRID_MSGBOX_DEMO_ICONS_TITLE_INFO"),
                                                     ui::MessageBoxWnd::kButtonsOK, ui::MessageBoxWnd::kIconInfo, nullptr, true);
            if (nResult == ui::MessageBoxWnd::kResultOK) {
                nResult = ui::MessageBoxWnd::Show(this, textId,
                                                  _T("STRID_MSGBOX_DEMO_ICONS_TITLE_WARNING"),
                                                  ui::MessageBoxWnd::kButtonsOK, ui::MessageBoxWnd::kIconWarning, nullptr, true);
            }
            if (nResult == ui::MessageBoxWnd::kResultOK) {
                nResult = ui::MessageBoxWnd::Show(this, textId,
                                                  _T("STRID_MSGBOX_DEMO_ICONS_TITLE_ERROR"),
                                                  ui::MessageBoxWnd::kButtonsOK, ui::MessageBoxWnd::kIconError, nullptr, true);
            }
            if (nResult == ui::MessageBoxWnd::kResultOK) {
                nResult = ui::MessageBoxWnd::Show(this, textId,
                                                  _T("STRID_MSGBOX_DEMO_ICONS_TITLE_QUESTION"),
                                                  ui::MessageBoxWnd::kButtonsOKCancel, ui::MessageBoxWnd::kIconQuestion, nullptr, true);
            }
            ShowResult(nResult);
            return true;
        });
    }

    //长文本自适应
    ui::Button* pLongTextButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_demo_long_text")));
    if (pLongTextButton != nullptr) {
        pLongTextButton->AttachClick([this](const ui::EventArgs&) {
            DString textId = _T("STRID_MSGBOX_DEMO_LONG_TEXT_TEXT");
            DString titleId = _T("STRID_MSGBOX_DEMO_LONG_TEXT_TITLE");
            int32_t nResult = ui::MessageBoxWnd::Show(this, textId, titleId,
                                                      ui::MessageBoxWnd::kButtonsOKCancel,
                                                      ui::MessageBoxWnd::kIconNone,
                                                      nullptr, true);
            ShowResult(nResult);
            return true;
        });
    }

    //自定义按钮文字
    ui::Button* pCustomButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_demo_custom_text")));
    if (pCustomButton != nullptr) {
        pCustomButton->AttachClick([this](const ui::EventArgs&) {
            ui::MessageBoxWnd::ButtonText buttonText;
            buttonText.yes    = _T("STRID_MSGBOX_CUSTOM_YES");
            buttonText.no     = _T("STRID_MSGBOX_CUSTOM_NO");
            buttonText.cancel = _T("STRID_MSGBOX_CUSTOM_CANCEL");
            DString textId = _T("STRID_MSGBOX_DEMO_CUSTOM_TEXT");
            DString titleId = _T("STRID_MSGBOX_DEMO_CUSTOM_TITLE");
            int32_t nResult = ui::MessageBoxWnd::Show(this, textId, titleId,
                                                      ui::MessageBoxWnd::kButtonsYesNoCancel,
                                                      ui::MessageBoxWnd::kIconQuestion,
                                                      &buttonText, true);
            ShowResult(nResult);
            return true;
        });
    }
}

bool MessageBoxForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    //结果标签文本是动态拼接的，需要根据新语言重新生成
    if (m_pResultLabel != nullptr) {
        if (m_bHasResult) {
            ShowResult(m_lastResult);
        }
        else {
            m_pResultLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_MAIN_RESULT_NONE")));
        }
    }
    return bRet;
}

void MessageBoxForm::ShowResult(int32_t nResult)
{
    m_lastResult = nResult;
    m_bHasResult = true;

    if (m_pResultLabel == nullptr) {
        return;
    }
    DString strResultName;
    switch (nResult) {
    case ui::MessageBoxWnd::kResultOK:
        strResultName = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RESULT_OK"));
        break;
    case ui::MessageBoxWnd::kResultCancel:
        strResultName = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RESULT_CANCEL"));
        break;
    case ui::MessageBoxWnd::kResultRetry:
        strResultName = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RESULT_RETRY"));
        break;
    case ui::MessageBoxWnd::kResultYes:
        strResultName = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RESULT_YES"));
        break;
    case ui::MessageBoxWnd::kResultNo:
        strResultName = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_RESULT_NO"));
        break;
    default:
        strResultName = ui::StringUtil::Printf(_T("%d"), nResult);
        break;
    }
    m_pResultLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_MAIN_RESULT_PREFIX")) + strResultName);
}
