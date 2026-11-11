#include "RichTextForm.h"

namespace
{
/** 读取语言资源 */
DString RichTextLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
}//namespace

RichTextForm::RichTextForm()
{
}

RichTextForm::~RichTextForm()
{
}

DString RichTextForm::GetSkinFolder()
{
    return _T("controls");
}

DString RichTextForm::GetSkinFile()
{
    return _T("rich_text.xml");
}

void RichTextForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pInteractive = dynamic_cast<ui::RichText*>(FindControl(_T("richtext_interactive")));
    m_pStatus = dynamic_cast<ui::Label*>(FindControl(_T("rt_status")));

    //链接的点击、悬停、离开事件：回显链接地址
    ui::RichText* pLinks = dynamic_cast<ui::RichText*>(FindControl(_T("richtext_links")));
    if (pLinks != nullptr) {
        pLinks->AttachLinkClick([this](const ui::EventArgs& args) {
            ShowLinkEvent(_T("STRID_RICHTEXT_LINK_EVENT_CLICK"), args);
            return true;
            });
        pLinks->AttachLinkHover([this](const ui::EventArgs& args) {
            ShowLinkEvent(_T("STRID_RICHTEXT_LINK_EVENT_HOVER"), args);
            return true;
            });
        pLinks->AttachLinkLeave([this](const ui::EventArgs& args) {
            ShowLinkEvent(_T("STRID_RICHTEXT_LINK_EVENT_LEAVE"), args);
            return true;
            });
    }

    BindApiButtons();
}

void RichTextForm::OnInitLayout()
{
    BaseClass::OnInitLayout();
    UpdateStatus();
}

void RichTextForm::BindApiButtons()
{
    if (m_pInteractive == nullptr) {
        return;
    }

    //对齐方式轮换
    ui::Button* pAlignBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_align")));
    if (pAlignBtn != nullptr) {
        pAlignBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ++m_nAlignIndex;
            ApplyTextAlign();
            UpdateStatus();
            return true;
            });
    }

    //自动换行开关
    ui::Button* pWrapBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_wrap")));
    if (pWrapBtn != nullptr) {
        pWrapBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->SetWordWrap(!m_pInteractive->IsWordWrap());
            UpdateStatus();
            return true;
            });
    }

    //链接颜色：在主题默认色与自定义配色之间切换
    ui::Button* pColorBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_link_color")));
    if (pColorBtn != nullptr) {
        pColorBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_bCustomLinkColor = !m_bCustomLinkColor;
            if (m_bCustomLinkColor) {
                m_pInteractive->SetLinkColor(_T("#FFD08700"), _T("#FFE85854"), _T("#FF22A95B"), _T("#FF8A5A00"));
            }
            else {
                m_pInteractive->SetLinkColor(_T("text_link_normal"), _T("text_link_hovered"),
                                             _T("text_link_pressed"), _T("text_link_visited"));
            }
            UpdateStatus();
            return true;
            });
    }

    //追加文本
    ui::Button* pAppendBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_append")));
    if (pAppendBtn != nullptr) {
        pAppendBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->AppendText(RichTextLang(_T("STRID_RICHTEXT_APPEND_TEXT")));
            UpdateStatus();
            return true;
            });
    }

    //清空文本
    ui::Button* pClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_clear")));
    if (pClearBtn != nullptr) {
        pClearBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->Clear();
            UpdateStatus();
            return true;
            });
    }

    //恢复初始文本
    ui::Button* pRestoreBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_restore")));
    if (pRestoreBtn != nullptr) {
        pRestoreBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->Clear();
            m_pInteractive->SetTextId(_T("STRID_RICHTEXT_INTERACTIVE_EXAMPLE"));
            UpdateStatus();
            return true;
            });
    }

    //全选文本
    ui::Button* pSelectAllBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_select_all")));
    if (pSelectAllBtn != nullptr) {
        pSelectAllBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->SelectAll();
            UpdateStatus();
            return true;
            });
    }

    //复制选中的文本
    ui::Button* pCopyBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_copy")));
    if (pCopyBtn != nullptr) {
        pCopyBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            const bool bCopied = m_pInteractive->CopySelectedText();
            if (m_pStatus != nullptr) {
                m_pStatus->SetText(RichTextLang(bCopied ? _T("STRID_RICHTEXT_STATUS_COPIED")
                                                        : _T("STRID_RICHTEXT_STATUS_NO_SELECTION")));
            }
            return true;
            });
    }

    //取消选择
    ui::Button* pClearSelectionBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_rt_clear_selection")));
    if (pClearSelectionBtn != nullptr) {
        pClearSelectionBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pInteractive->ClearSelection();
            UpdateStatus();
            return true;
            });
    }
}

void RichTextForm::ApplyTextAlign()
{
    if (m_pInteractive == nullptr) {
        return;
    }
    //通过属性设置对齐方式，justify（两端对齐）只能用属性表达
    static const DString kAlignValues[] = { _T("left"), _T("center"), _T("right"), _T("justify") };
    const int32_t nCount = (int32_t)(sizeof(kAlignValues) / sizeof(kAlignValues[0]));
    m_nAlignIndex = (m_nAlignIndex % nCount + nCount) % nCount;
    m_pInteractive->SetAttribute(_T("text_align"), kAlignValues[m_nAlignIndex]);
}

void RichTextForm::UpdateStatus()
{
    if ((m_pInteractive == nullptr) || (m_pStatus == nullptr)) {
        return;
    }
    const DString strFormat = RichTextLang(_T("STRID_RICHTEXT_STATUS_FMT"));
    m_pStatus->SetText(ui::StringUtil::Printf(strFormat.c_str(),
                       GetAlignName().c_str(),
                       (int32_t)m_pInteractive->GetLinkCount(),
                       (int32_t)m_pInteractive->GetSelectedText().size(),
                       RichTextLang(m_pInteractive->IsWordWrap() ? _T("STRID_RICHTEXT_ON")
                                                                 : _T("STRID_RICHTEXT_OFF")).c_str()));
}

void RichTextForm::ShowLinkEvent(const DString& strTextId, const ui::EventArgs& args)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("link_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString::value_type* pUrl = (const DString::value_type*)args.wParam;
    const DString strFormat = RichTextLang(strTextId);
    pLabel->SetText(ui::StringUtil::Printf(strFormat.c_str(), (pUrl != nullptr) ? pUrl : _T("")));
}

DString RichTextForm::GetAlignName() const
{
    static const DString kAlignTextIds[] = {
        _T("STRID_RICHTEXT_ALIGN_LEFT"),
        _T("STRID_RICHTEXT_ALIGN_CENTER"),
        _T("STRID_RICHTEXT_ALIGN_RIGHT"),
        _T("STRID_RICHTEXT_ALIGN_JUSTIFY")
    };
    const int32_t nCount = (int32_t)(sizeof(kAlignTextIds) / sizeof(kAlignTextIds[0]));
    return RichTextLang(kAlignTextIds[m_nAlignIndex % nCount]);
}
