#include "FilterCombo.h"

namespace ui 
{

FilterCombo::FilterCombo(Window* pWindow):
    Combo(pWindow)
{
    SetComboType(kCombo_DropDown);
}

FilterCombo::~FilterCombo()
{
}

DString FilterCombo::GetType() const { return DUI_CTR_FILTER_COMBO; }

void FilterCombo::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("combo_type")) {
        //忽略该属性设置
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void FilterCombo::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    SetComboType(kCombo_DropDown);
}

bool FilterCombo::OnEditButtonDown(const EventArgs& /*args*/)
{
    ShowComboList();
    return true;
}

bool FilterCombo::OnEditButtonUp(const EventArgs& /*args*/)
{
    return true;
}

bool FilterCombo::OnEditTextChanged(const ui::EventArgs& /*args*/)
{
    DString editText = GetText();
    //转换成小写，比较的时候，不区分大小写
    editText = StringUtil::MakeLowerString(editText);
    ShowComboList();
    FilterComboList(editText);
    return true;
}

void FilterCombo::FilterComboList(const DString& filterText)
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

bool FilterCombo::IsFilterText(const DString& filterText, const DString& itemText) const
{
    DString lowerItemText = StringUtil::MakeLowerString(itemText);
    if (filterText.empty()) {
        return true;
    }
    return (lowerItemText.find(filterText) != DString::npos) ? true : false;
}

} // namespace ui

