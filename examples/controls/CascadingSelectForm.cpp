#include "CascadingSelectForm.h"

CascadingSelectForm::CascadingSelectForm()
{
}

CascadingSelectForm::~CascadingSelectForm()
{
}

DString CascadingSelectForm::GetSkinFolder()
{
    return _T("controls");
}

DString CascadingSelectForm::GetSkinFile()
{
    return _T("cascading_select.xml");
}

void CascadingSelectForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::CascadingSelect* pBasic = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
    if (pBasic != nullptr) {
        //初始回显
        UpdateSelectionText(pBasic);

        //选择变化事件：新选择通过 pEventData 携带（CascadingSelectValue）
        pBasic->AttachSelectionChanged([this](const ui::EventArgs& args) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(args.GetSender());
            if (p != nullptr) {
                UpdateSelectionText(p);
            }
            return true;
            });
    }

    //选中"华东 / 上海 / 浦东"（第 1 / 1 / 1 项）
    ui::Button* pSelectBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_select")));
    if ((pSelectBtn != nullptr) && (pBasic != nullptr)) {
        pSelectBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            if (p != nullptr) {
                p->SetSelection({ 0, 0, 0 });
            }
            return true;
            });
    }

    //清空选择
    ui::Button* pClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_clear")));
    if ((pClearBtn != nullptr) && (pBasic != nullptr)) {
        pClearBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            if (p != nullptr) {
                p->ClearSelection();
            }
            return true;
            });
    }

    //追加一条路径（华中 / 武汉 / 武昌），演示动态增数据
    ui::Button* pAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_add")));
    if ((pAddBtn != nullptr) && (pBasic != nullptr)) {
        pAddBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            if (p != nullptr) {
                //items_id 模式下，AddPath 的每一段都按语言 ID 解析，因此这里传 ID 而不是译后的文本
                p->AddPath(_T("STRID_CS_CENTRAL/STRID_CS_WUHAN/STRID_CS_WUCHANG"));
                UpdateSelectionText(p);
            }
            return true;
            });
    }

    //禁用"华东 / 上海"（禁用后其整棵子树不可选，已选中的深层选择会被重置）
    ui::Button* pDisableBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_disable")));
    if ((pDisableBtn != nullptr) && (pBasic != nullptr)) {
        pDisableBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            if (p != nullptr) {
                DString strPath = GetLangText(_T("STRID_CS_EAST")) + _T("/") + GetLangText(_T("STRID_CS_SHANGHAI"));
                p->SetDisabledPaths({ strPath });
                UpdateSelectionText(p);
            }
            return true;
            });
    }

    //恢复全部可用
    ui::Button* pEnableBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_enable")));
    if ((pEnableBtn != nullptr) && (pBasic != nullptr)) {
        pEnableBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            if (p != nullptr) {
                p->SetDisabledPaths({});
                UpdateSelectionText(p);
            }
            return true;
            });
    }

    //读取选择（演示 GetSelection / GetItemCount / GetSelectionPath）
    ui::Button* pReadBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cs_read")));
    if ((pReadBtn != nullptr) && (pBasic != nullptr)) {
        pReadBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascading_basic")));
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("cascading_event")));
            if ((p != nullptr) && (pLabel != nullptr)) {
                const DString strPrefix = GetLangText(_T("STRID_CASCADING_SELECT_EVENT_PREFIX"));
                int32_t nSelected = 0;
                for (int32_t nIndex : p->GetSelection()) {
                    if (nIndex >= 0) {
                        ++nSelected;
                    }
                }
                const DString strInfo = ui::StringUtil::Printf(_T("%d/%d"),
                    nSelected, static_cast<int32_t>(p->GetDepth()));
                pLabel->SetText(strPrefix + strInfo + _T("  ") + p->GetSelectionPath());
            }
            return true;
            });
    }

    //下拉列表框形态：选择框 + 弹出多列级联浮层
    ui::CascadingSelect* pDropdownBasic = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascader_basic")));
    if (pDropdownBasic != nullptr) {
        //初始回显
        UpdateDropdownText(pDropdownBasic, _T("cascader_event"));

        //选择变化事件
        pDropdownBasic->AttachSelectionChanged([this](const ui::EventArgs& args) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(args.GetSender());
            if (p != nullptr) {
                UpdateDropdownText(p, _T("cascader_event"));
            }
            return true;
            });
    }

    //选中"华东 / 上海 / 浦东"
    ui::Button* pDropdownSelectBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cascader_select")));
    if ((pDropdownSelectBtn != nullptr) && (pDropdownBasic != nullptr)) {
        pDropdownSelectBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascader_basic")));
            if (p != nullptr) {
                p->SetSelection({ 0, 0, 0 });
            }
            return true;
            });
    }

    //清空选择
    ui::Button* pDropdownClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cascader_clear")));
    if ((pDropdownClearBtn != nullptr) && (pDropdownBasic != nullptr)) {
        pDropdownClearBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascader_basic")));
            if (p != nullptr) {
                p->ClearSelection();
            }
            return true;
            });
    }

    //切换自定义显示：选择框文本与浮层内的选项文本分离
    ui::Button* pDropdownDisplayBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_cascader_display")));
    if (pDropdownDisplayBtn != nullptr) {
        pDropdownDisplayBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CascadingSelect* p = dynamic_cast<ui::CascadingSelect*>(FindControl(_T("cascader_custom")));
            if (p == nullptr) {
                return true;
            }
            m_bCustomDisplay = !m_bCustomDisplay;
            if (m_bCustomDisplay) {
                p->AttachDisplayText([this](const ui::CascadingSelectValue& value) -> DString {
                    if (value.text.empty()) {
                        return DString();
                    }
                    return GetLangText(_T("STRID_CASCADING_SELECT_DISPLAY_PREFIX")) + value.text;
                    });
            }
            else {
                p->AttachDisplayText(nullptr);
            }
            UpdateDropdownText(p, _T("cascader_event"));
            return true;
            });
    }
}

void CascadingSelectForm::UpdateSelectionText(ui::CascadingSelect* pCascadingSelect)
{
    if (pCascadingSelect == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("cascading_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPath = pCascadingSelect->GetSelectionPath();
    const DString strPrefix = GetLangText(_T("STRID_CASCADING_SELECT_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + (strPath.empty() ? GetLangText(_T("STRID_CASCADING_SELECT_EMPTY")) : strPath));
}

void CascadingSelectForm::UpdateDropdownText(ui::CascadingSelect* pCascadingSelect, const DString& strLabelName)
{
    if (pCascadingSelect == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(strLabelName));
    if (pLabel == nullptr) {
        return;
    }
    const DString strText = pCascadingSelect->GetDisplayText();
    const DString strPrefix = GetLangText(_T("STRID_CASCADING_SELECT_EVENT_DROPDOWN_PREFIX"));
    pLabel->SetText(strPrefix + (strText.empty() ? GetLangText(_T("STRID_CASCADING_SELECT_EVENT_DROPDOWN_NONE")) : strText));
}

DString CascadingSelectForm::GetLangText(const DString& strId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
