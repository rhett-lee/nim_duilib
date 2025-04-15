#include "DirectoryTree.h"

namespace ui
{
DirectoryTree::DirectoryTree(Window* pWindow):
    TreeView(pWindow)
{
}

DirectoryTree::~DirectoryTree()
{
}

DString DirectoryTree::GetType() const { return DUI_CTR_DIRECTORY_TREE; }

void DirectoryTree::SetAttribute(const DString& strName, const DString& strValue)
{
    //支持的属性列表: 基类实现的直接转发
    if (strName == _T("")) {
        
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

TreeNode* DirectoryTree::ShowVirtualDirectoryNode(VirtualDirectoryType type, const DString& displayName, bool bDisplayNameIsID)
{
    return nullptr;
}

TreeNode* DirectoryTree::ShowAllDiskNode()
{
    return nullptr;
}

bool DirectoryTree::InsertLineBeforeNode(TreeNode* pNode, const DString& lineClassName)
{
    bool bRet = false;
    if (pNode != nullptr) {
        ui::Control* pLineControl = new ui::Control(GetWindow());
        if (!lineClassName.empty()) {
            pLineControl->SetClass(lineClassName);
        }
        else {
            pLineControl->SetClass(_T("splitline_hor_level1"));
            pLineControl->SetMargin(ui::UiMargin(12, 8, 12, 8), true);
        }
        bRet = InsertControlBeforeNode(pNode, pLineControl);
    }
    return bRet;
}

}//namespace ui
