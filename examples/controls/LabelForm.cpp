#include "LabelForm.h"

namespace
{
/** 读取语言资源 */
DString LabelLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}

/** 换行符替换演示所用的文本（包含"\\n"两个字符，开启替换后显示为多行） */
const DString kNewlineSample = _T("Line 1\\nLine 2\\nLine 3");
}//namespace

LabelForm::LabelForm()
{
}

LabelForm::~LabelForm()
{
}

DString LabelForm::GetSkinFolder()
{
    return _T("controls");
}

DString LabelForm::GetSkinFile()
{
    return _T("label.xml");
}

void LabelForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pTarget = dynamic_cast<ui::Label*>(FindControl(_T("label_interactive")));
    m_pStatus = dynamic_cast<ui::Label*>(FindControl(_T("label_status")));
    BindApiButtons();
}

void LabelForm::OnInitLayout()
{
    BaseClass::OnInitLayout();
    UpdateStatus();
}

void LabelForm::BindApiButtons()
{
    if (m_pTarget == nullptr) {
        return;
    }

    //单行/多行切换
    ui::Button* pSingleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_single")));
    if (pSingleBtn != nullptr) {
        pSingleBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pTarget->SetSingleLine(!m_pTarget->IsSingleLine());
            UpdateStatus();
            return true;
            });
    }

    //水平对齐方式轮换
    ui::Button* pAlignBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_align")));
    if (pAlignBtn != nullptr) {
        pAlignBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            CycleTextAlign();
            UpdateStatus();
            return true;
            });
    }

    //纵向文本开关
    ui::Button* pVerticalBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_vertical")));
    if (pVerticalBtn != nullptr) {
        pVerticalBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pTarget->SetVerticalText(!m_pTarget->IsVerticalText());
            UpdateStatus();
            return true;
            });
    }

    //换行符替换开关
    ui::Button* pNewlineBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_newline")));
    if (pNewlineBtn != nullptr) {
        pNewlineBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            const bool bReplace = m_pTarget->IsReplaceNewline();
            if (!bReplace) {
                //演示文本中包含"\\n"两个字符，开启替换后显示为三行
                m_pTarget->SetText(kNewlineSample);
            }
            m_pTarget->SetReplaceNewline(!bReplace);
            UpdateStatus();
            return true;
            });
    }

    //RichText模式开关
    ui::Button* pRichBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_rich")));
    if (pRichBtn != nullptr) {
        pRichBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            const bool bRichText = !m_pTarget->IsRichText();
            m_pTarget->SetRichText(bRichText);
            m_pTarget->SetText(LabelLang(bRichText ? _T("STRID_LABEL_RICH_SAMPLE")
                                                   : _T("STRID_LABEL_INTERACTIVE_TEXT")));
            UpdateStatus();
            return true;
            });
    }

    //文字内边距开关
    ui::Button* pPaddingBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_padding")));
    if (pPaddingBtn != nullptr) {
        pPaddingBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            const bool bHasPadding = (m_pTarget->GetTextPadding().left > 0);
            m_pTarget->SetTextPadding(bHasPadding ? ui::UiPadding(0, 0, 0, 0)
                                                  : ui::UiPadding(12, 6, 12, 6), false);
            UpdateStatus();
            return true;
            });
    }

    //自动ToolTip开关
    ui::Button* pToolTipBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_tooltip")));
    if (pToolTipBtn != nullptr) {
        pToolTipBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            m_pTarget->SetAutoToolTip(!m_pTarget->IsAutoToolTip());
            UpdateStatus();
            return true;
            });
    }

    //字体切换
    ui::Button* pFontBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_label_font")));
    if (pFontBtn != nullptr) {
        pFontBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            static const DString kFontIds[] = {
                _T("system_regular_14"), _T("system_bold_15"), _T("system_regular_18")
            };
            static int32_t nFontIndex = 0;
            ++nFontIndex;
            const int32_t nCount = (int32_t)(sizeof(kFontIds) / sizeof(kFontIds[0]));
            m_pTarget->SetFontId(kFontIds[nFontIndex % nCount]);
            UpdateStatus();
            return true;
            });
    }
}

void LabelForm::CycleTextAlign()
{
    if (m_pTarget == nullptr) {
        return;
    }
    //通过属性设置对齐方式，justify（两端对齐）只能用属性表达
    static const DString kAlignValues[] = {
        _T("left,vcenter"), _T("hcenter,vcenter"), _T("right,vcenter"), _T("hjustify,vcenter")
    };
    const int32_t nCount = (int32_t)(sizeof(kAlignValues) / sizeof(kAlignValues[0]));
    m_nAlignIndex = (m_nAlignIndex + 1) % nCount;
    m_pTarget->SetAttribute(_T("text_align"), kAlignValues[m_nAlignIndex]);
}

void LabelForm::UpdateStatus()
{
    if ((m_pTarget == nullptr) || (m_pStatus == nullptr)) {
        return;
    }
    const DString strFormat = LabelLang(_T("STRID_LABEL_STATUS_FMT"));
    m_pStatus->SetText(ui::StringUtil::Printf(strFormat.c_str(),
                       GetAlignName().c_str(),
                       LabelLang(m_pTarget->IsSingleLine() ? _T("STRID_LABEL_ON") : _T("STRID_LABEL_OFF")).c_str(),
                       LabelLang(m_pTarget->IsVerticalText() ? _T("STRID_LABEL_ON") : _T("STRID_LABEL_OFF")).c_str(),
                       LabelLang(m_pTarget->IsReplaceNewline() ? _T("STRID_LABEL_ON") : _T("STRID_LABEL_OFF")).c_str(),
                       LabelLang(m_pTarget->IsRichText() ? _T("STRID_LABEL_ON") : _T("STRID_LABEL_OFF")).c_str()));
}

DString LabelForm::GetAlignName() const
{
    static const DString kAlignTextIds[] = {
        _T("STRID_LABEL_ALIGN_LEFT"),
        _T("STRID_LABEL_ALIGN_CENTER"),
        _T("STRID_LABEL_ALIGN_RIGHT"),
        _T("STRID_LABEL_ALIGN_JUSTIFY")
    };
    const int32_t nCount = (int32_t)(sizeof(kAlignTextIds) / sizeof(kAlignTextIds[0]));
    return LabelLang(kAlignTextIds[m_nAlignIndex % nCount]);
}
