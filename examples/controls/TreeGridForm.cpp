#include "TreeGridForm.h"
#include "duilib/Control/TreeGrid.h"
#include "duilib/Utils/StringUtil.h"
#include <functional>

namespace ui
{
/** 解析语言 ID 得到显示文本
*/
static DString TreeGridLangText(const DString& strTextId)
{
    return GlobalManager::Instance().Lang().GetStringByID(strTextId);
}
}

TreeGridForm::TreeGridForm()
{
}

TreeGridForm::~TreeGridForm()
{
}

DString TreeGridForm::GetSkinFolder()
{
    return _T("controls");
}

DString TreeGridForm::GetSkinFile()
{
    return _T("tree_grid.xml");
}

void TreeGridForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //基础树表格：多列 + 展开收起 + 点击事件
    ui::TreeGrid* pBasic = dynamic_cast<ui::TreeGrid*>(FindControl(_T("tree_grid_basic")));
    if (pBasic != nullptr) {
        InitBasicTree(pBasic);
        SetAllExpanded(pBasic, true);
        pBasic->AttachNodeClick([this](const ui::EventArgs& args) {
            ui::TreeGridNode* pNode = (ui::TreeGridNode*)args.pEventData;
            if (pNode != nullptr) {
                UpdateBasicEventText(ui::TreeGridLangText(_T("STRID_TREE_GRID_EVENT_CLICK_PREFIX")) + pNode->GetCellText(0));
            }
            return true;
            });
    }
    ui::Button* pExpandAll = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_expand_all")));
    if (pExpandAll != nullptr) {
        pExpandAll->AttachClick([this, pBasic](const ui::EventArgs&) {
            SetAllExpanded(pBasic, true);
            return true;
            });
    }
    ui::Button* pCollapseAll = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_collapse_all")));
    if (pCollapseAll != nullptr) {
        pCollapseAll->AttachClick([this, pBasic](const ui::EventArgs&) {
            SetAllExpanded(pBasic, false);
            return true;
            });
    }

    //勾选树表格：多选 + 复选框父子级联 + 勾选事件
    ui::TreeGrid* pCheck = dynamic_cast<ui::TreeGrid*>(FindControl(_T("tree_grid_check")));
    if (pCheck != nullptr) {
        InitCheckTree(pCheck);
        SetAllExpanded(pCheck, true);
        pCheck->AttachCheckChanged([this](const ui::EventArgs& args) {
            ui::TreeGridCheckValue* pValue = (ui::TreeGridCheckValue*)args.pEventData;
            if ((pValue != nullptr) && (pValue->pNode != nullptr)) {
                const DString strState = pValue->bChecked ?
                    ui::TreeGridLangText(_T("STRID_TREE_GRID_STATE_CHECKED")) :
                    ui::TreeGridLangText(_T("STRID_TREE_GRID_STATE_UNCHECKED"));
                UpdateCheckEventText(ui::TreeGridLangText(_T("STRID_TREE_GRID_EVENT_CHECK_PREFIX")) +
                                     pValue->pNode->GetCellText(0) + _T(" (") + strState + _T(")"));
            }
            return true;
            });
        pCheck->AttachNodeSelect([this](const ui::EventArgs& args) {
            ui::TreeGridNode* pNode = (ui::TreeGridNode*)args.pEventData;
            if (pNode != nullptr) {
                UpdateCheckEventText(ui::TreeGridLangText(_T("STRID_TREE_GRID_EVENT_SELECT_PREFIX")) + pNode->GetCellText(0));
            }
            return true;
            });
    }
    ui::Button* pCheckAll = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_check_all")));
    if (pCheckAll != nullptr) {
        pCheckAll->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                pCheck->SetAllNodesChecked(true);
            }
            return true;
            });
    }
    ui::Button* pCheckNone = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_check_none")));
    if (pCheckNone != nullptr) {
        pCheckNone->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                pCheck->SetAllNodesChecked(false);
            }
            return true;
            });
    }

    //功能测试：动态增删节点 / 模式切换 / 读取勾选状态（用于验证控件基本功能）
    ui::Button* pAddNode = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_add_node")));
    if (pAddNode != nullptr) {
        pAddNode->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                AddNewNode(pCheck, pCheck->GetRootNode(), false);
            }
            return true;
            });
    }
    ui::Button* pAddChild = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_add_child")));
    if (pAddChild != nullptr) {
        pAddChild->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                //未选中节点时，默认挂到根节点下
                std::vector<ui::TreeGridNode*> selectNodes = pCheck->GetSelectNodes();
                ui::TreeGridNode* pParent = selectNodes.empty() ? pCheck->GetRootNode() : selectNodes[0];
                AddNewNode(pCheck, pParent, true);
            }
            return true;
            });
    }
    ui::Button* pRemoveNode = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_remove_node")));
    if (pRemoveNode != nullptr) {
        pRemoveNode->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                std::vector<ui::TreeGridNode*> selectNodes = pCheck->GetSelectNodes();
                if (selectNodes.empty()) {
                    UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_NO_SELECT")));
                }
                else {
                    ui::TreeGridNode* pNode = selectNodes[0];
                    const DString strName = pNode->GetCellText(0);
                    pCheck->RemoveNode(pNode);
                    UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_REMOVED")) + strName);
                }
            }
            return true;
            });
    }
    ui::Button* pToggleCheckMode = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_check_mode")));
    if (pToggleCheckMode != nullptr) {
        pToggleCheckMode->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                pCheck->SetCheckMode(!pCheck->IsCheckMode());
                UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_CHECK_MODE")) +
                               (pCheck->IsCheckMode() ? ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_ON")) :
                                                        ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_OFF"))));
            }
            return true;
            });
    }
    ui::Button* pToggleCascade = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_cascade")));
    if (pToggleCascade != nullptr) {
        pToggleCascade->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                pCheck->SetCheckCascade(!pCheck->IsCheckCascade());
                UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_CASCADE")) +
                               (pCheck->IsCheckCascade() ? ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_ON")) :
                                                           ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_OFF"))));
            }
            return true;
            });
    }
    ui::Button* pToggleMulti = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_multi")));
    if (pToggleMulti != nullptr) {
        pToggleMulti->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                pCheck->SetMultiSelect(!pCheck->IsMultiSelect());
                UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_MULTI")) +
                               (pCheck->IsMultiSelect() ? ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_ON")) :
                                                          ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_OFF"))));
            }
            return true;
            });
    }
    ui::Button* pShowChecked = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_show_checked")));
    if (pShowChecked != nullptr) {
        pShowChecked->AttachClick([this, pCheck](const ui::EventArgs&) {
            if (pCheck != nullptr) {
                std::vector<ui::TreeGridNode*> checkedNodes = pCheck->GetCheckedNodes();
                if (checkedNodes.empty()) {
                    UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_CHECKED")) +
                                   ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_EMPTY")));
                }
                else {
                    DString strText = ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_CHECKED"));
                    for (size_t i = 0; i < checkedNodes.size(); ++i) {
                        if (i > 0) {
                            strText += _T(", ");
                        }
                        strText += checkedNodes[i]->GetCellText(0);
                    }
                    UpdateTestText(strText);
                }
            }
            return true;
            });
    }
    ui::Button* pExpandLevel2 = dynamic_cast<ui::Button*>(FindControl(_T("tree_grid_test_expand2")));
    if (pExpandLevel2 != nullptr) {
        pExpandLevel2->AttachClick([this, pCheck](const ui::EventArgs&) {
            ExpandToLevel(pCheck, 2);
            UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_EXPANDED2")));
            return true;
            });
    }
}

void TreeGridForm::InitBasicTree(ui::TreeGrid* pTreeGrid)
{
    if (pTreeGrid == nullptr) {
        return;
    }
    const DString strFolder = ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FOLDER"));
    const DString strFile = ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FILE"));

    ui::TreeGridNode* pRoot = pTreeGrid->GetRootNode();

    //一级：Source Files（文件夹）
    ui::TreeGridNode* pSource = pTreeGrid->AddNode(pRoot);
    pSource->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_SOURCE")));
    pSource->SetCellText(1, _T("12.4 MB"));
    pSource->SetCellText(2, strFolder);

    ui::TreeGridNode* pMain = pTreeGrid->AddNode(pSource);
    pMain->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_MAIN")));
    pMain->SetCellText(1, _T("8.1 KB"));
    pMain->SetCellText(2, strFile);

    ui::TreeGridNode* pRenderer = pTreeGrid->AddNode(pSource);
    pRenderer->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_RENDERER")));
    pRenderer->SetCellText(1, _T("36.7 KB"));
    pRenderer->SetCellText(2, strFile);

    //一级：Resources（文件夹），二级：Images（文件夹），三级：logo.png
    ui::TreeGridNode* pResources = pTreeGrid->AddNode(pRoot);
    pResources->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_RESOURCES")));
    pResources->SetCellText(1, _T("4.6 MB"));
    pResources->SetCellText(2, strFolder);

    ui::TreeGridNode* pImages = pTreeGrid->AddNode(pResources);
    pImages->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_IMAGES")));
    pImages->SetCellText(1, _T("3.2 MB"));
    pImages->SetCellText(2, strFolder);

    ui::TreeGridNode* pLogo = pTreeGrid->AddNode(pImages);
    pLogo->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_LOGO")));
    pLogo->SetCellText(1, _T("18.2 KB"));
    pLogo->SetCellText(2, strFile);

    ui::TreeGridNode* pThemes = pTreeGrid->AddNode(pResources);
    pThemes->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_THEMES")));
    pThemes->SetCellText(1, _T("42.0 KB"));
    pThemes->SetCellText(2, strFile);

    //一级：README.md（文件，无子节点）
    ui::TreeGridNode* pReadme = pTreeGrid->AddNode(pRoot);
    pReadme->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_README")));
    pReadme->SetCellText(1, _T("2.3 KB"));
    pReadme->SetCellText(2, strFile);
}

void TreeGridForm::InitCheckTree(ui::TreeGrid* pTreeGrid)
{
    if (pTreeGrid == nullptr) {
        return;
    }
    const DString strFolder = ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FOLDER"));
    const DString strFile = ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FILE"));

    ui::TreeGridNode* pRoot = pTreeGrid->GetRootNode();

    //Design：一个子项勾选，另一个不勾选，演示父节点"部分勾选"
    ui::TreeGridNode* pDesign = pTreeGrid->AddNode(pRoot);
    pDesign->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_DESIGN")));
    pDesign->SetCellText(1, _T("2"));
    pDesign->SetCellText(2, strFolder);

    ui::TreeGridNode* pSpec = pTreeGrid->AddNode(pDesign);
    pSpec->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_SPEC")));
    pSpec->SetCellText(1, _T("156 KB"));
    pSpec->SetCellText(2, strFile);

    ui::TreeGridNode* pMock = pTreeGrid->AddNode(pDesign);
    pMock->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_MOCK")));
    pMock->SetCellText(1, _T("328 KB"));
    pMock->SetCellText(2, strFile);

    //Development：子文件夹整体勾选，演示向下级联 + 向上回溯
    ui::TreeGridNode* pDevelopment = pTreeGrid->AddNode(pRoot);
    pDevelopment->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_DEVELOPMENT")));
    pDevelopment->SetCellText(1, _T("2"));
    pDevelopment->SetCellText(2, strFolder);

    ui::TreeGridNode* pCore = pTreeGrid->AddNode(pDevelopment);
    pCore->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_CORE")));
    pCore->SetCellText(1, _T("1"));
    pCore->SetCellText(2, strFolder);

    ui::TreeGridNode* pWidget = pTreeGrid->AddNode(pCore);
    pWidget->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_WIDGET")));
    pWidget->SetCellText(1, _T("24.5 KB"));
    pWidget->SetCellText(2, strFile);

    ui::TreeGridNode* pBuild = pTreeGrid->AddNode(pDevelopment);
    pBuild->SetCellText(0, ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_BUILD")));
    pBuild->SetCellText(1, _T("1.1 KB"));
    pBuild->SetCellText(2, strFile);

    //预置勾选：Spec 勾选 → Design 呈"部分勾选"；Core 勾选 → 其子项级联勾选
    pSpec->SetChecked(true);
    pCore->SetChecked(true);
}

void TreeGridForm::SetAllExpanded(ui::TreeGrid* pTreeGrid, bool bExpand)
{
    if (pTreeGrid == nullptr) {
        return;
    }
    std::function<void(ui::TreeGridNode*)> walk = [&walk, bExpand](ui::TreeGridNode* pParent) {
        for (size_t i = 0; i < pParent->GetChildNodeCount(); ++i) {
            ui::TreeGridNode* pChild = pParent->GetChildNode(i);
            if (pChild != nullptr) {
                pChild->SetExpand(bExpand);
                walk(pChild);
            }
        }
    };
    walk(pTreeGrid->GetRootNode());
}

void TreeGridForm::UpdateBasicEventText(const DString& strText)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("tree_grid_basic_event")));
    if (pLabel != nullptr) {
        pLabel->SetText(strText);
    }
}

void TreeGridForm::UpdateCheckEventText(const DString& strText)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("tree_grid_check_event")));
    if (pLabel != nullptr) {
        pLabel->SetText(strText);
    }
}

void TreeGridForm::AddNewNode(ui::TreeGrid* pTreeGrid, ui::TreeGridNode* pParent, bool bChild)
{
    if ((pTreeGrid == nullptr) || (pParent == nullptr)) {
        return;
    }
    ui::TreeGridNode* pNode = pTreeGrid->AddNode(pParent);
    if (pNode == nullptr) {
        return;
    }
    //节点名：NewNode + 自增序号
    const DString strName = ui::TreeGridLangText(_T("STRID_TREE_GRID_ROW_NEW")) +
                            ui::StringUtil::Int32ToString(++m_nAddCounter);
    pNode->SetCellText(0, strName);
    pNode->SetCellText(1, _T("0"));
    pNode->SetCellText(2, bChild ? ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FILE")) :
                                   ui::TreeGridLangText(_T("STRID_TREE_GRID_TYPE_FOLDER")));
    //展开父节点，并把新节点滚动到可见区域
    pTreeGrid->SetNodeExpand(pParent, true);
    pTreeGrid->EnsureNodeVisible(pNode);
    UpdateTestText(ui::TreeGridLangText(_T("STRID_TREE_GRID_TEST_ADDED")) + strName);
}

void TreeGridForm::ExpandToLevel(ui::TreeGrid* pTreeGrid, int32_t nLevel)
{
    if (pTreeGrid == nullptr) {
        return;
    }
    //展开深度小于 nLevel 的节点，使第 nLevel 层的节点可见
    std::function<void(ui::TreeGridNode*, int32_t)> walk = [&walk, nLevel](ui::TreeGridNode* pParent, int32_t nDepth) {
        for (size_t i = 0; i < pParent->GetChildNodeCount(); ++i) {
            ui::TreeGridNode* pChild = pParent->GetChildNode(i);
            if (pChild != nullptr) {
                if (pChild->GetChildNodeCount() > 0) {
                    pChild->SetExpand(nDepth < nLevel);
                }
                walk(pChild, nDepth + 1);
            }
        }
    };
    walk(pTreeGrid->GetRootNode(), 0);
}

void TreeGridForm::UpdateTestText(const DString& strText)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("tree_grid_test_status")));
    if (pLabel != nullptr) {
        pLabel->SetText(strText);
    }
}
