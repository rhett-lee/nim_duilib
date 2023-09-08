#include "FilterCombo.h"

namespace ui 
{

FilterCombo::FilterCombo()
{
    SetComboType(kCombo_DropDown);
}

FilterCombo::~FilterCombo()
{
}

std::wstring FilterCombo::GetType() const { return DUI_CTR_FILTER_COMBO; }

void FilterCombo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"combo_type") {
        //���Ը���������
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void FilterCombo::DoInit()
{
    bool bInited = IsInited();
    __super::DoInit();
    SetComboType(kCombo_DropDown);
    if (!bInited) {
        RichEdit* pRichEdit = GetEditControl();
        if (pRichEdit != nullptr) {
            pRichEdit->AttachSetFocus(nbase::Bind(&FilterCombo::OnEditSetFocus, this, std::placeholders::_1));
            pRichEdit->AttachKillFocus(nbase::Bind(&FilterCombo::OnEditKillFocus, this, std::placeholders::_1));
            pRichEdit->AttachTextChange(nbase::Bind(&FilterCombo::OnEditTextChanged, this, std::placeholders::_1));
        }
    }
}

void FilterCombo::ShowComboList(bool /*bActivated*/)
{
    __super::ShowComboList(false);
}

bool FilterCombo::OnEditButtonDown(const EventArgs& /*args*/)
{
    return true;
}

bool FilterCombo::OnEditButtonUp(const EventArgs& /*args*/)
{
    return true;
}

bool FilterCombo::OnEditTextChanged(const ui::EventArgs& /*args*/)
{
    std::wstring editText = GetText();
    //ת����Сд���Ƚϵ�ʱ�򣬲����ִ�Сд
    editText = StringHelper::MakeLowerString(editText);
    FilterComboList(editText);
    return true;
}

bool FilterCombo::OnEditSetFocus(const EventArgs& /*args*/)
{
    ShowComboList(false);
    return true;
}

bool FilterCombo::OnEditKillFocus(const EventArgs& /*args*/)
{
    HideComboList();
    return true;
}

void FilterCombo::FilterComboList(const std::wstring& filterText)
{
    TreeView* pTreeView = GetTreeView();
    if (pTreeView == nullptr) {
        return;
    }
    size_t itemCount = pTreeView->GetItemCount();
    for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
        Control* pControl = pTreeView->GetItemAt(iIndex);
        if (pControl != nullptr) {
            TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pControl);
            ASSERT(pTreeNode != nullptr);
            if (pTreeNode != nullptr) {
                pTreeNode->SetExpand(true, false);
                if (IsFilterText(filterText, pTreeNode->GetText())) {
                    pTreeNode->SetFadeVisible(true);
                }
                else {
                    pTreeNode->SetFadeVisible(false);
                }
            }
        }
    }
    UpdateComboList();
}

bool FilterCombo::IsFilterText(const std::wstring& filterText, const std::wstring& itemText) const
{
    std::wstring lowerItemText = StringHelper::MakeLowerString(itemText);
    if (filterText.empty()) {
        return true;
    }
    return (lowerItemText.find(filterText) != std::wstring::npos) ? true : false;
}

} // namespace ui
