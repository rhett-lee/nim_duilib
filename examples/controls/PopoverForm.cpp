#include "PopoverForm.h"

PopoverForm::PopoverForm()
{
}

PopoverForm::~PopoverForm()
{
}

DString PopoverForm::GetSkinFolder()
{
    return _T("controls");
}

DString PopoverForm::GetSkinFile()
{
    return _T("popover.xml");
}

void PopoverForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    BindBasicPopover();
    BindTriggerPopovers();
    BindControlledPopover();
    BindRichPopover();
}

void PopoverForm::BindBasicPopover()
{
    ui::Popover* pPopover = dynamic_cast<ui::Popover*>(FindControl(_T("popover_basic")));
    if (pPopover == nullptr) {
        return;
    }

    //基础用法：卡片标题来自 title_id，正文用 content_xml 拼装成两行（对应 Ant Design 的 Title + Content + Content）
    DString strContentXml;
    strContentXml += _T("<VBox width=\"stretch\" height=\"auto\">");
    strContentXml += _T("<Label class=\"popover_content\" width=\"stretch\" height=\"auto\"");
    strContentXml += _T(" text_id=\"STRID_POPOVER_BASIC_CONTENT_1\"/>");
    strContentXml += _T("<Label class=\"popover_content\" width=\"stretch\" height=\"auto\" margin=\"0,6,0,0\"");
    strContentXml += _T(" text_id=\"STRID_POPOVER_BASIC_CONTENT_2\"/>");
    strContentXml += _T("</VBox>");
    pPopover->SetContentXml(strContentXml);
}

void PopoverForm::BindTriggerPopovers()
{
    //手动触发的锚点只响应 API，这里用一个按钮演示 Show() / Toggle()
    ui::Button* pManualBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popover_manual")));
    if (pManualBtn != nullptr) {
        pManualBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Popover* pPopover = dynamic_cast<ui::Popover*>(FindControl(_T("popover_manual")));
            if (pPopover != nullptr) {
                pPopover->Toggle();
            }
            return true;
            });
    }
}

void PopoverForm::BindControlledPopover()
{
    ui::Popover* pPopover = dynamic_cast<ui::Popover*>(FindControl(_T("popover_controlled")));
    if (pPopover == nullptr) {
        return;
    }
    m_pControlled = pPopover;

    //打开状态变化事件：wParam 为 1/0，lParam 为关闭原因（见 ui::PopoverCloseReason）
    pPopover->AttachOpenChanged([this](const ui::EventArgs& args) {
        const bool bOpen = (static_cast<int32_t>(args.wParam) != 0);
        const int32_t nReason = (bOpen ? 0 : static_cast<int32_t>(args.lParam));
        UpdateStateText(bOpen, nReason);
        return true;
        });

    ui::Button* pShowBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popover_show")));
    if (pShowBtn != nullptr) {
        pShowBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pControlled != nullptr) {
                m_pControlled->Show();
            }
            return true;
            });
    }

    ui::Button* pHideBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popover_hide")));
    if (pHideBtn != nullptr) {
        pHideBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pControlled != nullptr) {
                m_pControlled->Hide();
            }
            return true;
            });
    }

    ui::Button* pToggleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popover_toggle")));
    if (pToggleBtn != nullptr) {
        pToggleBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pControlled != nullptr) {
                m_pControlled->Toggle();
            }
            return true;
            });
    }

    //修改卡片宽度、间距与方位（演示运行时配置对下一次打开生效）
    ui::Button* pStyleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popover_style")));
    if (pStyleBtn != nullptr) {
        pStyleBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pControlled == nullptr) {
                return true;
            }
            if (m_pControlled->GetCardWidth() >= 320) {
                m_pControlled->SetCardWidth(240);
                m_pControlled->SetGap(6);
                m_pControlled->SetPlacement(ui::PopoverPlacement::kTop);
            }
            else {
                m_pControlled->SetCardWidth(320);
                m_pControlled->SetGap(12);
                m_pControlled->SetPlacement(ui::PopoverPlacement::kRight);
            }
            if (m_pControlled->IsOpen()) {
                //已打开的卡片按新配置重建
                m_pControlled->Hide();
                m_pControlled->Show();
            }
            return true;
            });
    }
}

void PopoverForm::BindRichPopover()
{
    ui::Popover* pPopover = dynamic_cast<ui::Popover*>(FindControl(_T("popover_rich")));
    if (pPopover == nullptr) {
        return;
    }

    //复杂内容：content_xml 片段作为卡片根容器（class="popover_card"）的子控件插入，
    //其中放置"从浮层内关闭"按钮（对应 Ant Design 的同名示例）
    DString strContentXml;
    strContentXml += _T("<VBox width=\"stretch\" height=\"auto\">");
    strContentXml += _T("<Label class=\"popover_content\" width=\"stretch\" height=\"auto\"");
    strContentXml += _T(" text_id=\"STRID_POPOVER_RICH_NOTE\"/>");
    strContentXml += _T("<Label class=\"popover_content\" width=\"stretch\" height=\"auto\" margin=\"0,6,0,0\"");
    strContentXml += _T(" normal_text_color=\"text_link_normal\" text_id=\"STRID_POPOVER_RICH_HINT\"/>");
    strContentXml += _T("<HBox height=\"30\" margin=\"0,12,0,0\">");
    strContentXml += _T("<Button class=\"btn_global\" name=\"popover_rich_close\" width=\"120\" height=\"28\"");
    strContentXml += _T(" border_round=\"3,3\" text_id=\"STRID_POPOVER_RICH_CLOSE_BTN\" valign=\"center\"/>");
    strContentXml += _T("<Control width=\"stretch\" height=\"stretch\" mouse_enabled=\"false\"/>");
    strContentXml += _T("</HBox>");
    strContentXml += _T("</VBox>");
    pPopover->SetContentXml(strContentXml);

    //卡片每次打开都会重建，因此在打开状态变化事件中重新绑定卡片内的按钮
    pPopover->AttachOpenChanged([pPopover](const ui::EventArgs& args) {
        if (static_cast<int32_t>(args.wParam) == 0) {
            return true;
        }
        ui::Button* pCloseBtn = dynamic_cast<ui::Button*>(pPopover->FindCardControl(_T("popover_rich_close")));
        if (pCloseBtn != nullptr) {
            pCloseBtn->AttachClick([pPopover](const ui::EventArgs& /*args*/) {
                pPopover->Hide();
                return true;
                });
        }
        return true;
        });
}

void PopoverForm::UpdateStateText(bool bOpen, int32_t nReason)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("popover_state")));
    if (pLabel == nullptr) {
        return;
    }

    //先拼出状态短语，再填入前缀模板的 {0}：语言文件会去掉首尾空白，
    //若把分隔空格写在英文前缀尾部会被裁掉（中文用全角冒号不需要空格）
    DString strValue;
    if (bOpen) {
        strValue = GetLangText(_T("STRID_POPOVER_STATE_OPEN"));
    }
    else {
        DString strReason;
        switch (static_cast<ui::PopoverCloseReason>(static_cast<uint32_t>(nReason))) {
        case ui::PopoverCloseReason::kManual:       strReason = _T("STRID_POPOVER_REASON_MANUAL"); break;
        case ui::PopoverCloseReason::kClickOutside: strReason = _T("STRID_POPOVER_REASON_CLICK_OUTSIDE"); break;
        case ui::PopoverCloseReason::kEscape:       strReason = _T("STRID_POPOVER_REASON_ESCAPE"); break;
        case ui::PopoverCloseReason::kAnchorLost:   strReason = _T("STRID_POPOVER_REASON_ANCHOR_LOST"); break;
        default:                                    strReason = _T("STRID_POPOVER_REASON_UNKNOWN"); break;
        }
        strValue = GetLangText(_T("STRID_POPOVER_STATE_CLOSED"));
        strValue += GetLangText(strReason.c_str());
        strValue += _T(")");
    }
    DString strState = GetLangText(_T("STRID_POPOVER_STATE_PREFIX"));
    ui::StringUtil::ReplaceAll(_T("{0}"), strValue, strState);
    pLabel->SetText(strState);
}

DString PopoverForm::GetLangText(const DString& strId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
