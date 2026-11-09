#include "ListViewForm.h"

ListViewForm::ListViewForm()
{
}

ListViewForm::~ListViewForm()
{
}

DString ListViewForm::GetSkinFolder()
{
    return _T("controls");
}

DString ListViewForm::GetSkinFile()
{
    return _T("list_view.xml");
}

void ListViewForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //纵向列表：动态追加数据
    ui::ListBox* pListV = dynamic_cast<ui::ListBox*>(FindControl(_T("list_v")));
    if (pListV != nullptr) {
        for (int32_t i = (int32_t)pListV->GetItemCount(); i < 20; ++i) {
            ui::ListBoxItem* pItem = new ui::ListBoxItem(this);
            pItem->SetClass(_T("list_item"));
            pItem->SetFixedHeight(ui::UiFixedInt(24), true, true);
            pItem->SetText(ui::StringUtil::Printf(_T("ListBoxItem %d"), i));
            pListV->AddItem(pItem);
        }
        pListV->AttachSelect([this, pListV](const ui::EventArgs& args) {
            size_t index = (size_t)args.wParam;
            ui::Control* pItem = pListV->GetItemAt(index);
            ui::ListBoxItem* pListBoxItem = dynamic_cast<ui::ListBoxItem*>(pItem);
            if (pListBoxItem != nullptr) {
                UpdateEventText(ui::StringUtil::Printf(_T("VListBox 选中第 %d 项：%s"),
                                                       (int32_t)index, pListBoxItem->GetText().c_str()));
            }
            return true;
            });
        pListV->AttachDoubleClick([this, pListV](const ui::EventArgs& args) {
            size_t index = (size_t)args.wParam;
            ui::Control* pItem = pListV->GetItemAt(index);
            ui::ListBoxItem* pListBoxItem = dynamic_cast<ui::ListBoxItem*>(pItem);
            if (pListBoxItem != nullptr) {
                ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("双击：%s"), pListBoxItem->GetText().c_str()),
                                   ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
            }
            return true;
            });
    }

    //树控件：动态添加节点
    ui::TreeView* pTreeView = dynamic_cast<ui::TreeView*>(FindControl(_T("tree_view")));
    if (pTreeView != nullptr) {
        ui::TreeNode* pRootNode = pTreeView->GetRootNode();
        ui::TreeNode* pNode2 = pRootNode != nullptr ? pRootNode->FindChildNodeByText(_T("Root Node 2"), true) : nullptr;
        if (pNode2 != nullptr) {
            for (int32_t i = 0; i < 4; ++i) {
                ui::TreeNode* pChild = new ui::TreeNode(this);
                pChild->SetClass(_T("list_item"));
                pChild->SetText(ui::StringUtil::Printf(_T("Dynamic Child %d"), i));
                pNode2->AddChildNode(pChild);
            }
        }
        pTreeView->AttachSelect([this, pTreeView](const ui::EventArgs& args) {
            size_t index = (size_t)args.wParam;
            ui::Control* pItem = pTreeView->GetItemAt(index);
            ui::TreeNode* pTreeNode = dynamic_cast<ui::TreeNode*>(pItem);
            if (pTreeNode != nullptr) {
                UpdateEventText(ui::StringUtil::Printf(_T("TreeView 选中：%s"), pTreeNode->GetText().c_str()));
            }
            return true;
            });
    }
}

void ListViewForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("list_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
