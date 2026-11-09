#include "ComboForm.h"

namespace
{
/** 读取语言资源（代码动态生成的文本必须走语言文件，才能支持中英文切换） */
DString ComboLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
}//namespace

ComboForm::ComboForm()
{
}

ComboForm::~ComboForm()
{
}

DString ComboForm::GetSkinFolder()
{
    return _T("controls");
}

DString ComboForm::GetSkinFile()
{
    return _T("combo.xml");
}

void ComboForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::Combo* pDropList = dynamic_cast<ui::Combo*>(FindControl(_T("combo_drop_list")));
    InitComboItems(pDropList, _T("Combo Item"), 12);
    if (pDropList != nullptr) {
        pDropList->AttachSelect([this, pDropList](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(ComboLang(_T("STRID_COMBO_EVENT_DROP_LIST")).c_str(), pDropList->GetText().c_str()));
            return true;
            });
    }

    ui::Combo* pDropDown = dynamic_cast<ui::Combo*>(FindControl(_T("combo_drop_down")));
    InitComboItems(pDropDown, _T("Editable Item"), 12);
    if (pDropDown != nullptr) {
        pDropDown->AttachSelect([this, pDropDown](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(ComboLang(_T("STRID_COMBO_EVENT_DROP_DOWN")).c_str(), pDropDown->GetText().c_str()));
            return true;
            });
    }

    //禁用状态的下拉框：填充数据但不响应
    ui::Combo* pDisabled = dynamic_cast<ui::Combo*>(FindControl(_T("combo_disabled")));
    InitComboItems(pDisabled, _T("Disabled Item"), 6);
    if (pDisabled != nullptr) {
        pDisabled->SetCurSel(0, false);
    }

    //FilterCombo：输入关键字后过滤
    ui::FilterCombo* pFilterCombo = dynamic_cast<ui::FilterCombo*>(FindControl(_T("filter_combo")));
    if (pFilterCombo != nullptr) {
        for (int32_t i = 0; i < 40; ++i) {
            pFilterCombo->AddTextItem(ui::StringUtil::Printf(_T("Filter Item %02d"), i));
        }
        pFilterCombo->AttachSelect([this, pFilterCombo](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(ComboLang(_T("STRID_COMBO_EVENT_FILTER")).c_str(), pFilterCombo->GetText().c_str()));
            return true;
            });
    }

    //CheckCombo：多选下拉
    ui::CheckCombo* pCheckCombo = dynamic_cast<ui::CheckCombo*>(FindControl(_T("check_combo")));
    if (pCheckCombo != nullptr) {
        pCheckCombo->AddTextItem(_T("Email"));
        pCheckCombo->AddTextItem(_T("SMS"));
        pCheckCombo->AddTextItem(_T("Push Notification"));
        pCheckCombo->AddTextItem(_T("In-App Message"));
    }
}

bool ComboForm::OnLanguageChanged()
{
    const bool bRet = BaseClass::OnLanguageChanged();
    UpdateEventText(ComboLang(_T("STRID_COMBO_EVENT_DEFAULT")));
    return bRet;
}

void ComboForm::InitComboItems(ui::Combo* pCombo, const DString& prefix, int32_t nCount)
{
    if (pCombo == nullptr) {
        return;
    }
    ui::TreeView* pTreeView = pCombo->GetTreeView();
    if (pTreeView == nullptr) {
        return;
    }
    ui::TreeNode* pRootNode = pTreeView->GetRootNode();
    if (pRootNode == nullptr) {
        return;
    }
    for (int32_t i = 0; i < nCount; ++i) {
        ui::TreeNode* pNode = new ui::TreeNode(this);
        pNode->SetClass(_T("tree_node"));
        pNode->SetText(ui::StringUtil::Printf(_T("%s %d"), prefix.c_str(), i));
        pRootNode->AddChildNode(pNode);
    }
    pCombo->SetCurSel(0, false);
}

void ComboForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("combo_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
