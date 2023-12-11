#include "PropertyGrid.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Control/ColorPicker.h"
#include "duilib/Control/ColorPickerRegular.h"

namespace ui
{

PropertyGrid::PropertyGrid():
    m_bHeaderCtrl(true),
    m_pHeaderCtrl(nullptr),
    m_pHeaderLeft(nullptr),
    m_pHeaderRight(nullptr),
    m_pHeaderSplit(nullptr),
    m_pDescriptionArea(nullptr),
    m_bDescriptionArea(true),
    m_pDescriptionAreaSplit(nullptr),
    m_pTreeView(nullptr)
{
    m_nLeftColumnWidth = GlobalManager::Instance().Dpi().GetScaleInt(130);
}

std::wstring PropertyGrid::GetType() const { return DUI_CTR_PROPERTY_GRID; }

void PropertyGrid::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"property_grid_xml") {
        if (!strValue.empty()) {
            m_configXml = strValue;
        }
    }
    else if (strName == L"row_grid_line_width") {
        SetRowGridLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"row_grid_line_color") {
        SetRowGridLineColor(strValue);
    }
    else if (strName == L"column_grid_line_width") {
        SetColumnGridLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"column_grid_line_color") {
        SetColumnGridLineColor(strValue);
    }
    else if (strName == L"header_class") {
        SetHeaderClass(strValue);
    }
    else if (strName == L"group_class") {
        SetGroupClass(strValue);
    }
    else if (strName == L"propterty_class") {
        SetPropertyClass(strValue);
    }
    else if (strName == L"left_column_width") {
        SetLeftColumnWidth(_wtoi(strValue.c_str()), true);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void PropertyGrid::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    //初始化基本结构
    if (m_configXml.empty()) {
        //默认的配置文件
        m_configXml = L"public/property_grid/property_grid.xml";
    }
    ui::GlobalManager::Instance().FillBoxWithCache(this, m_configXml.c_str());
    m_pHeaderCtrl = FindSubControl(L"duilib_property_grid_header");
    m_pHeaderLeft = dynamic_cast<Label*>(FindSubControl(L"duilib_property_grid_header_left"));
    m_pHeaderRight = dynamic_cast<Label*>(FindSubControl(L"duilib_property_grid_header_right"));
    m_pHeaderSplit = dynamic_cast<Split*>(FindSubControl(L"duilib_property_grid_header_split"));
    if (m_pHeaderCtrl != nullptr) {
        ASSERT(m_pHeaderLeft != nullptr);
        ASSERT(m_pHeaderRight != nullptr);
        ASSERT(m_pHeaderSplit != nullptr);
        m_pHeaderCtrl->SetClass(GetHeaderClass());
    }
    else {
        m_bHeaderCtrl = false;
    }

    m_pDescriptionArea = dynamic_cast<RichText*>(FindSubControl(L"duilib_property_grid_description_area"));
    m_bDescriptionArea = m_pDescriptionArea != nullptr;
    m_pDescriptionAreaSplit = dynamic_cast<Split*>(FindSubControl(L"duilib_property_grid_description_area_split"));

    m_pTreeView = dynamic_cast<TreeView*>(FindSubControl(L"duilib_property_grid_tree"));
    ASSERT(m_pTreeView != nullptr);

    //挂载拖动响应事件, 调整列的宽度
    if (m_pHeaderSplit != nullptr) {
        m_pHeaderSplit->AttachSplitDraged([this](const EventArgs& /*args*/) {
            OnHeaderColumnResized();
            return true;
            });
    }

    //初始化第一列宽度
    SetLeftColumnWidth(m_nLeftColumnWidth, false);

    //关联描述区域
    if ((m_pDescriptionArea != nullptr) && (m_pTreeView != nullptr)) {
        m_pTreeView->AttachSelect([this](const EventArgs& args) {
            Control* pItem = nullptr;
            size_t nItem = args.wParam;
            if (nItem != Box::InvalidIndex) {
                pItem = m_pTreeView->GetItemAt(nItem);
            }
            std::wstring description;
            std::wstring name;
            if (pItem != nullptr) {
                PropertyGridGroup* pGroup = dynamic_cast<PropertyGridGroup*>(pItem);
                if (pGroup != nullptr) {
                    description = pGroup->GetDescriptiion();
                    name = pGroup->GetGroupName();
                }
                else {
                    PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(pItem);
                    if (pProperty != nullptr) {
                        description = pProperty->GetDescriptiion();
                        name = pProperty->GetPropertyName();
                    }
                }
            }
            if (m_pDescriptionArea != nullptr) {
                if (!name.empty()) {
                    description = L"<b>" + name + L"</b><br/>" + description;
                }
                m_pDescriptionArea->SetRichText(description);
            }
            return true;
            });
    }
}

void PropertyGrid::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);

    //网格线的绘制
    PaintGridLines(pRender);
}

void PropertyGrid::PaintGridLines(IRender* pRender)
{
    if (m_pTreeView == nullptr) {
        return;
    }
    int32_t nColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度        
    int32_t nRowLineWidth = GetRowGridLineWidth();   //横向边线宽度
    UiColor columnLineColor;
    UiColor rowLineColor;
    std::wstring color = GetColumnGridLineColor();
    if (!color.empty()) {
        columnLineColor = GetUiColor(color);
    }
    color = GetRowGridLineColor();
    if (!color.empty()) {
        rowLineColor = GetUiColor(color);
    }

    if ((nColumnLineWidth > 0) && !columnLineColor.IsEmpty()) {
        //绘制纵向网格线
        UiRect viewRect = m_pTreeView->GetRect();
        UiPoint viewScrollPos = m_pTreeView->GetScrollOffsetInScrollBox();
        viewRect.Offset(-viewScrollPos.x, -viewScrollPos.y);
        int32_t yTop = viewRect.top;
        std::vector<int32_t> xPosList;
        const size_t itemCount = m_pTreeView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            PropertyGridProperty* pItem = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetHeight() <= 0)) {
                continue;
            }
            if (pItem->GetLabelBoxLeft() != nullptr) {
                UiRect leftRect = pItem->GetLabelBoxLeft()->GetRect();
                UiPoint scrollBoxOffset = pItem->GetLabelBoxLeft()->GetScrollOffsetInScrollBox();
                leftRect.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
                xPosList.push_back(leftRect.right);
                break;
            }
        }

        for (int32_t xPos : xPosList) {
            //横坐标位置放在每个子项控件的右侧部            
            UiPoint pt1(xPos, yTop);
            UiPoint pt2(xPos, viewRect.bottom);
            pRender->DrawLine(pt1, pt2, columnLineColor, nColumnLineWidth);
        }
    }
    if ((nRowLineWidth > 0) && !rowLineColor.IsEmpty()) {
        //绘制横向网格线
        UiRect viewRect = m_pTreeView->GetRect();
        UiPoint viewScrollPos = m_pTreeView->GetScrollOffsetInScrollBox();
        viewRect.Offset(-viewScrollPos.x, -viewScrollPos.y);
        const size_t itemCount = m_pTreeView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            PropertyGridProperty* pItem = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetHeight() <= 0)) {
                continue;
            }
            //纵坐标位置放在每个子项控件的底部（Header控件的底部不画线）
            UiRect rcItemRect = pItem->GetRect();
            UiPoint scrollBoxOffset = pItem->GetScrollOffsetInScrollBox();
            rcItemRect.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
            int32_t yPos = rcItemRect.bottom;

            int32_t nChildMarginY = 0;
            Layout* pLayout = GetLayout();
            if (pLayout != nullptr) {
                nChildMarginY = pLayout->GetChildMarginY();
            }
            yPos += nChildMarginY / 2;
            if (yPos <= viewRect.top) {
                //位置不在矩形区域内，不需要画线
                continue;
            }
            if (yPos > viewRect.bottom) {
                //已经超越底部边线，不再继续绘制
                break;
            }

            UiRect leftRect = viewRect;
            if (pItem->GetLabelBoxLeft() != nullptr) {
                UiRect rcRect = pItem->GetLabelBoxLeft()->GetRect();
                UiPoint scrollOffset = pItem->GetLabelBoxLeft()->GetScrollOffsetInScrollBox();
                rcRect.Offset(-scrollOffset.x, -scrollOffset.y);
                leftRect = rcRect;
            }
            UiRect rightRect = viewRect;
            if (pItem->GetLabelBoxRight() != nullptr) {
                UiRect rcRect = pItem->GetLabelBoxRight()->GetRect();
                UiPoint scrollOffset = pItem->GetLabelBoxRight()->GetScrollOffsetInScrollBox();
                rcRect.Offset(-scrollOffset.x, -scrollOffset.y);
                rightRect = rcRect;
            }
            UiPoint pt1(leftRect.left, yPos);
            UiPoint pt2(rightRect.right, yPos);
            pRender->DrawLine(pt1, pt2, rowLineColor, nRowLineWidth);
        }
    }
}

void PropertyGrid::OnHeaderColumnResized()
{
    Control* pLeftHeaderItem = m_pHeaderLeft;
    Control* pRightHeaderItem = m_pHeaderRight;
    if ((pLeftHeaderItem == nullptr) && (pRightHeaderItem == nullptr)) {
        return;
    }
    ASSERT(m_pTreeView != nullptr);
    if (m_pTreeView == nullptr) {
        return;
    }
    int32_t nLeftColumnWidth = GetLeftColumnWidth();
    if (nLeftColumnWidth < 0) {
        return;
    }
    TreeNode* pRootNode = m_pTreeView->GetRootNode();
    if (pRootNode == nullptr) {
        return;
    }
    size_t nCount = pRootNode->GetChildNodeCount();
    for (size_t i = 0; i < nCount; ++i) {
        //第一层是分组节点，不需要调整
        TreeNode* pNode = pRootNode->GetChildNode(i);
        if (pNode == nullptr) {
            continue;
        }
        size_t nChildCount = pNode->GetChildNodeCount();
        for (size_t iChild = 0; iChild < nChildCount; ++iChild) {
            //第二层是属性节点，需要调整
            TreeNode* pChildNode = pNode->GetChildNode(iChild);
            if (pChildNode == nullptr) {
                continue;
            }
            ResizePropertyColumn(pChildNode, nLeftColumnWidth);
        }
    }
    //重绘树控件，避免网格线不刷新
    m_pTreeView->Invalidate();
}

void PropertyGrid::ResizePropertyColumn(TreeNode* pPropertyNode, int32_t nLeftColumnWidth)
{
    PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(pPropertyNode);
    if (pProperty == nullptr) {
        return;
    }
    if (nLeftColumnWidth < 0) {
        return;
    }
    LabelBox* pLabelBoxLeft = pProperty->GetLabelBoxLeft();
    LabelBox* pLabelBoxRight = pProperty->GetLabelBoxRight();
    if ((pLabelBoxLeft != nullptr) && (nLeftColumnWidth >= 0)) {
        int32_t nLabelWidth = nLeftColumnWidth;
        UiPadding rcPadding = pProperty->GetPadding();
        nLabelWidth -= rcPadding.left;
        pLabelBoxLeft->SetFixedWidth(UiFixedInt(nLabelWidth), true, false);
    }
    if (pLabelBoxRight != nullptr) {
        pLabelBoxRight->SetFixedWidth(UiFixedInt::MakeStretch(), true, false);
    }
}

void PropertyGrid::SetEnableHeaderCtrl(bool bEnable,
                                       const std::wstring& sLeftColumn,
                                       const std::wstring& sRightColumn)
{
    ASSERT(IsInited());
    m_bHeaderCtrl = bEnable;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetVisible(bEnable);
    }
    if (IsEnableHeaderCtrl()) {
        if (m_pHeaderLeft != nullptr) {
            m_pHeaderLeft->SetText(sLeftColumn);
        }
        if (m_pHeaderRight != nullptr) {
            m_pHeaderRight->SetText(sRightColumn);
        }
    }
}

void PropertyGrid::SetEnableDescriptionArea(bool bEnable)
{
    ASSERT(IsInited());
    m_bDescriptionArea = bEnable;
    if (m_pDescriptionArea != nullptr) {
        m_pDescriptionArea->SetVisible(bEnable);
    }
}

void PropertyGrid::SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale)
{
    ASSERT(IsInited());
    if (nHeight < 0) {
        nHeight = 0;
    }
    if (m_pDescriptionArea != nullptr) {
        m_pDescriptionArea->SetFixedHeight(UiFixedInt(nHeight), true, bNeedDpiScale);
    }
}

int32_t PropertyGrid::GetDescriptionAreaHeight() const
{
    int32_t nHeight = 0;
    if (m_pDescriptionArea != nullptr) {
        nHeight = m_pDescriptionArea->GetFixedHeight().GetInt32();
    }
    return nHeight;
}

void PropertyGrid::SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nRowGridLineWidth != nLineWidth) {
        m_nRowGridLineWidth = nLineWidth;
        Invalidate();
    }
}

int32_t PropertyGrid::GetRowGridLineWidth() const
{
    return m_nRowGridLineWidth;
}

void PropertyGrid::SetRowGridLineColor(const std::wstring& color)
{
    if (m_rowGridLineColor != color) {
        m_rowGridLineColor = color;
        Invalidate();
    }
}

std::wstring PropertyGrid::GetRowGridLineColor() const
{
    return m_rowGridLineColor.c_str();
}

void PropertyGrid::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nColumnGridLineWidth != nLineWidth) {
        m_nColumnGridLineWidth = nLineWidth;
        Invalidate();
    }
}

int32_t PropertyGrid::GetColumnGridLineWidth() const
{
    return m_nColumnGridLineWidth;
}

void PropertyGrid::SetColumnGridLineColor(const std::wstring& color)
{
    if (m_columnGridLineColor != color) {
        m_columnGridLineColor = color;
        Invalidate();
    }
}

std::wstring PropertyGrid::GetColumnGridLineColor() const
{
    return m_columnGridLineColor.c_str();
}

void PropertyGrid::SetHeaderClass(const std::wstring& headerClass)
{
    if (m_headerClass != headerClass) {
        m_headerClass = headerClass;
        if (m_pHeaderCtrl != nullptr) {
            m_pHeaderCtrl->SetClass(headerClass);
        }
    }
}

std::wstring PropertyGrid::GetHeaderClass() const
{
    return m_headerClass.c_str();
}

void PropertyGrid::SetGroupClass(const std::wstring& groupClass)
{
    if (m_groupClass != groupClass) {
        m_groupClass = groupClass;
        std::vector<PropertyGridGroup*> groups;
        GetGroups(groups);
        for (PropertyGridGroup* pGroup : groups) {
            if (pGroup != nullptr) {
                pGroup->SetClass(groupClass);
            }
        }
    }
}

std::wstring PropertyGrid::GetGroupClass() const
{
    return m_groupClass.c_str();
}

void PropertyGrid::SetPropertyClass(const std::wstring& propertyClass)
{
    if (m_propertyClass != propertyClass) {
        m_propertyClass = propertyClass;

        std::vector<PropertyGridProperty*> properties;
        std::vector<PropertyGridGroup*> groups;
        GetGroups(groups);
        for (PropertyGridGroup* pGroup : groups) {
            if (pGroup != nullptr) {
                pGroup->GetProperties(properties);
                for (PropertyGridProperty* pProperty : properties) {
                    if (pProperty != nullptr) {
                        pProperty->SetClass(propertyClass);
                    }
                }
            }
        }
    }
}

std::wstring PropertyGrid::GetPropertyClass() const
{
    return m_propertyClass.c_str();
}

PropertyGridGroup* PropertyGrid::AddGroup(const std::wstring& groupName,
                                          const std::wstring& description,
                                          size_t nGroupData)
{
    ASSERT(m_pTreeView != nullptr);
    if (m_pTreeView == nullptr) {
        return nullptr;
    }
    PropertyGridGroup* pGroup = new PropertyGridGroup(groupName, description, nGroupData);
    pGroup->SetWindow(GetWindow());
    pGroup->SetClass(GetGroupClass());
    m_pTreeView->GetRootNode()->AddChildNode(pGroup);    
    pGroup->SetExpand(true);
    return pGroup;
}

void PropertyGrid::GetGroups(std::vector<PropertyGridGroup*>& groups) const
{
    groups.clear();
    if (m_pTreeView == nullptr) {
        return;
    }
    size_t nCount = m_pTreeView->GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        PropertyGridGroup* pGroup = dynamic_cast<PropertyGridGroup*>(m_pTreeView->GetItemAt(i));
        if (pGroup != nullptr) {
            groups.push_back(pGroup);
        }
    }
}

bool PropertyGrid::RemoveGroup(PropertyGridGroup* pGroup)
{
    if ((m_pTreeView == nullptr) || (pGroup == nullptr)){
        return false;
    }
    return m_pTreeView->GetRootNode()->RemoveChildNode(pGroup);
}

void PropertyGrid::RemoveAllGroups()
{
    if (m_pTreeView != nullptr) {
        m_pTreeView->GetRootNode()->RemoveAllChildNodes();
    }
}

bool PropertyGrid::AddProperty(PropertyGridGroup* pGroup, PropertyGridProperty* pProperty)
{
    ASSERT((pGroup != nullptr) && (pProperty != nullptr));
    if ((pGroup == nullptr) || (pProperty == nullptr)){
        return false;
    }
    size_t nIndex = pGroup->GetChildNodeIndex(pProperty);
    ASSERT(nIndex == Box::InvalidIndex);
    if (nIndex != Box::InvalidIndex) {
        return false;
    }
    pProperty->SetWindow(GetWindow());
    pProperty->SetClass(GetPropertyClass());
    pGroup->AddChildNode(pProperty);
    int32_t nLeftColumnWidth = GetLeftColumnWidth();
    if (nLeftColumnWidth >= 0) {
        ResizePropertyColumn(pProperty, nLeftColumnWidth);
    }
    return true;
}

PropertyGridTextProperty* PropertyGrid::AddTextProperty(PropertyGridGroup* pGroup,
                                                        const std::wstring& propertyName,
                                                        const std::wstring& propertyValue,
                                                        const std::wstring& description,
                                                        size_t nPropertyData)
{
    PropertyGridTextProperty* pProperty = new PropertyGridTextProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridComboProperty* PropertyGrid::AddComboProperty(PropertyGridGroup* pGroup,
                                                          const std::wstring& propertyName,
                                                          const std::wstring& propertyValue,
                                                          const std::wstring& description,
                                                          size_t nPropertyData)
{
    PropertyGridComboProperty* pProperty = new PropertyGridComboProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontProperty* PropertyGrid::AddFontProperty(PropertyGridGroup* pGroup,
                                                        const std::wstring& propertyName,
                                                        const std::wstring& propertyValue,
                                                        const std::wstring& description,
                                                        size_t nPropertyData)
{
    PropertyGridFontProperty* pProperty = new PropertyGridFontProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizeProperty(PropertyGridGroup* pGroup,
                                                                const std::wstring& propertyName,
                                                                const std::wstring& propertyValue,
                                                                const std::wstring& description,
                                                                size_t nPropertyData)
{
    PropertyGridFontSizeProperty* pProperty = new PropertyGridFontSizeProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridColorProperty* PropertyGrid::AddColorProperty(PropertyGridGroup* pGroup,
                                                          const std::wstring& propertyName,
                                                          const std::wstring& propertyValue,
                                                          const std::wstring& description,
                                                          size_t nPropertyData)
{
    PropertyGridColorProperty* pProperty = new PropertyGridColorProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimeProperty(PropertyGridGroup* pGroup,
                                                                const std::wstring& propertyName,
                                                                const std::wstring& dateTimeValue,                                                                
                                                                const std::wstring& description,
                                                                size_t nPropertyData,
                                                                DateTime::EditFormat editFormat)
{
    PropertyGridDateTimeProperty* pProperty = new PropertyGridDateTimeProperty(propertyName, dateTimeValue, description, nPropertyData, editFormat);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressProperty(PropertyGridGroup* pGroup,
                                                                  const std::wstring& propertyName,
                                                                  const std::wstring& propertyValue,
                                                                  const std::wstring& description,
                                                                  size_t nPropertyData)
{
    PropertyGridIPAddressProperty* pProperty = new PropertyGridIPAddressProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyProperty(PropertyGridGroup* pGroup,
                                                            const std::wstring& propertyName,
                                                            const std::wstring& propertyValue,
                                                            const std::wstring& description,
                                                            size_t nPropertyData)
{
    PropertyGridHotKeyProperty* pProperty = new PropertyGridHotKeyProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFileProperty* PropertyGrid::AddFileProperty(PropertyGridGroup* pGroup,
                                                        const std::wstring& propertyName,
                                                        const std::wstring& propertyValue,                                                        
                                                        const std::wstring& description,
                                                        size_t nPropertyData,
                                                        bool bOpenFileDialog,
                                                        const std::vector<FileDialog::FileType>& fileTypes,
                                                        int32_t nFileTypeIndex,
                                                        const std::wstring& defaultExt)
{
    PropertyGridFileProperty* pProperty = new PropertyGridFileProperty(propertyName, propertyValue,
                                                                       description, nPropertyData,
                                                                       bOpenFileDialog, fileTypes,
                                                                       nFileTypeIndex, defaultExt);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryProperty(PropertyGridGroup* pGroup,
                                                                  const std::wstring& propertyName,
                                                                  const std::wstring& propertyValue,
                                                                  const std::wstring& description,
                                                                  size_t nPropertyData)
{
    PropertyGridDirectoryProperty* pProperty = new PropertyGridDirectoryProperty(propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

void PropertyGrid::SetLeftColumnWidth(int32_t nLeftColumnWidth, bool bNeedDpiScale)
{
    if (nLeftColumnWidth <= 0) {
        return;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLeftColumnWidth);
    }
    if (m_nLeftColumnWidth != nLeftColumnWidth) {
        m_nLeftColumnWidth = nLeftColumnWidth;
    }
    if (IsInited()) {
        int32_t nSplitWidth = 0;
        if (m_pHeaderSplit != nullptr) {
            nSplitWidth = m_pHeaderSplit->GetFixedWidth().GetInt32();
        }
        nLeftColumnWidth -= nSplitWidth / 2;
        if (nLeftColumnWidth < 0) {
            nLeftColumnWidth = 0;
        }
        if (m_pHeaderLeft != nullptr) {
            m_pHeaderLeft->SetFixedWidth(UiFixedInt(nLeftColumnWidth), true, false);
        }
        if (m_pHeaderRight != nullptr) {
            m_pHeaderRight->SetFixedWidth(UiFixedInt::MakeStretch(), true, false);
        }
        OnHeaderColumnResized();
    }
}

int32_t PropertyGrid::GetLeftColumnWidth() const
{
    int32_t nLeftColumnWidth = m_nLeftColumnWidth;
    if (IsInited() && (m_pHeaderLeft != nullptr)) {
        int32_t nSplitWidth = 0;
        if (m_pHeaderSplit != nullptr) {
            nSplitWidth = m_pHeaderSplit->GetFixedWidth().GetInt32();
        }
        nLeftColumnWidth = m_pHeaderLeft->GetFixedWidth().GetInt32() + nSplitWidth / 2;
    }
    return nLeftColumnWidth;
}

////////////////////////////////////////////////////////////////////////////
///

PropertyGridGroup::PropertyGridGroup(const std::wstring& groupName, 
                                     const std::wstring& description,
                                     size_t nGroupData) :
    m_pLabelBox(nullptr),
    m_nGroupData(nGroupData)
{
    m_groupName = groupName;
    m_description = description;
}

void PropertyGridGroup::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    SetTabStop(false);

    HBox* pHBox = new HBox;
    AddItem(pHBox);

    pHBox->SetMouseEnabled(false);
    pHBox->SetNoFocus();

    m_pLabelBox = new LabelBox;
    pHBox->AddItem(m_pLabelBox);
    m_pLabelBox->SetMouseEnabled(false);
    m_pLabelBox->SetNoFocus();
    m_pLabelBox->SetText(m_groupName.c_str());
    TreeView* pTreeView = GetTreeView();
    if (pTreeView != nullptr) {
        //设置与属性对齐
        int32_t nIndent = pTreeView->GetIndent();
        UiPadding rcPadding = m_pLabelBox->GetPadding();
        rcPadding.left += GetDepth() * nIndent;
        m_pLabelBox->SetPadding(rcPadding, false);
    }
}

void PropertyGridGroup::GetProperties(std::vector<PropertyGridProperty*>& properties) const
{
    properties.clear();
    size_t nCount = GetChildNodeCount();
    for (size_t i = 0; i < nCount; ++i) {
        PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(GetChildNode(i));
        if (pProperty != nullptr) {
            properties.push_back(pProperty);
        }
    }
}

bool PropertyGridGroup::RemoveProperty(PropertyGridProperty* pProperty)
{
    if (pProperty == nullptr) {
        return false;
    }
    return RemoveChildNode(pProperty);
}

void PropertyGridGroup::RemoveAllProperties()
{
    RemoveAllChildNodes();
}

////////////////////////////////////////////////////////////////////////////
///

class PropertyGridLabelBox : public LabelBox
{
public:
    PropertyGridLabelBox()
    {
        SetAutoToolTip(true);
    }
    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override
    {
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEvent(msg);
        }
        if (!IsDisabledEvents(msg)) {
            __super::HandleEvent(msg);
        }
    }

    /** 初始化函数
     */
    virtual void OnInit() override
    {
        if (IsInited()) {
            return;
        }
        __super::OnInit();
        SetShowFocusRect(true);
        SetTabStop(false);
    }
};

PropertyGridProperty::PropertyGridProperty(const std::wstring& propertyName, 
                                           const std::wstring& propertyValue,
                                           const std::wstring& description,
                                           size_t nPropertyData):
    m_pHBox(nullptr),
    m_pLabelBoxLeft(nullptr),
    m_pLabelBoxRight(nullptr),
    m_nPropertyData(nPropertyData),
    m_bReadOnly(false)
{
    m_propertyName = propertyName;
    m_propertyValue = propertyValue;
    m_description = description;
}

void PropertyGridProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    SetTabStop(false);

    m_pHBox = new HBox;
    AddItem(m_pHBox);
    //背景色：在property_grid.xml中定义
    m_pHBox->SetBkColor(L"property_grid_propterty_bkcolor");

    m_pHBox->SetMouseEnabled(false);
    m_pHBox->SetNoFocus();

    m_pLabelBoxLeft = new PropertyGridLabelBox;
    m_pHBox->AddItem(m_pLabelBoxLeft);
    m_pLabelBoxLeft->SetText(m_propertyName.c_str());

    m_pLabelBoxRight = new PropertyGridLabelBox;
    m_pHBox->AddItem(m_pLabelBoxRight);
    m_pLabelBoxRight->SetText(m_propertyValue.c_str());
    //属性值的正常字体：在property_grid.xml中定义
    m_pLabelBoxRight->SetFontId(L"property_grid_propterty_font_normal");

    //挂载鼠标左键按下事件
    m_pLabelBoxRight->AttachButtonDown([this](const EventArgs&) {
        if (!IsReadOnly() && IsEnabled()) {
            Control* pControl = ShowEditControl(true);
            if (pControl != nullptr) {
                int32_t nWidth = GetEditControlMarginRight();
                UiMargin rcMargin = pControl->GetMargin();
                rcMargin.right = nWidth;
                pControl->SetMargin(rcMargin, false);
            }
        }
        return true;
        });

    //允许或者禁止编辑控件
    EnableEditControl(!IsReadOnly() && IsEnabled());

    //滚动条滚动事件
    TreeView* pTreeView = GetTreeView();
    if (pTreeView != nullptr) {
        pTreeView->AttachScrollChange([this](const EventArgs&) {
            OnScrollPosChanged();
            return true;
            });
    }
}

int32_t PropertyGridProperty::GetEditControlMarginRight() const
{
    int32_t nWidth = 0;
    ScrollBar* pVScrollBar = nullptr;
    TreeView* pTreeView = GetTreeView();
    if (pTreeView != nullptr) {
        pVScrollBar = pTreeView->GetVScrollBar();
    }
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
        nWidth = pVScrollBar->GetWidth();
    }
    if (nWidth < 0) {
        nWidth = 0;
    }
    return nWidth;
}

void PropertyGridProperty::SetPropertyText(const std::wstring& text, bool bChanged)
{
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        m_pLabelBoxRight->SetText(text);
        if (bChanged) {
            m_pLabelBoxRight->SetFontId(L"property_grid_propterty_font_bold");
        }
        else {
            m_pLabelBoxRight->SetFontId(L"property_grid_propterty_font_normal");
        }
    }
}

void PropertyGridProperty::SetPropertyTextColor(const std::wstring& textColor)
{
    if (GetUiColor(textColor).IsEmpty()) {
        //无效颜色值
        return;
    }
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        m_pLabelBoxRight->SetStateTextColor(kControlStateNormal, textColor);
    }
}

std::wstring PropertyGridProperty::GetPropertyText() const
{
    std::wstring text;
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        text = m_pLabelBoxRight->GetText();
    }
    return text;
}

void PropertyGridProperty::SetPropertyFocus()
{
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        m_pLabelBoxRight->SetFocus();
    }
}

bool PropertyGridProperty::AddPropertySubItem(Control* pControl)
{
    bool bRet = false;
    ASSERT(m_pLabelBoxRight != nullptr);
    ASSERT(pControl != nullptr);
    if ((m_pLabelBoxRight != nullptr) && (pControl != nullptr)) {
        bRet = m_pLabelBoxRight->AddItem(pControl);
    }
    return bRet;
}

bool PropertyGridProperty::RemovePropertySubItem(Control* pControl)
{
    bool bRet = false;
    ASSERT(m_pLabelBoxRight != nullptr);
    ASSERT(pControl != nullptr);
    if ((m_pLabelBoxRight != nullptr) && (pControl != nullptr)) {
        bRet = m_pLabelBoxRight->RemoveItem(pControl);
    }
    return bRet;
}

bool PropertyGridProperty::HasPropertySubItem(Control* pControl) const
{
    bool bRet = false;
    ASSERT(m_pLabelBoxRight != nullptr);
    ASSERT(pControl != nullptr);
    if ((m_pLabelBoxRight != nullptr) && (pControl != nullptr)) {
        bRet = m_pLabelBoxRight->GetItemIndex(pControl) != Box::InvalidIndex;
    }
    return bRet;
}

void PropertyGridProperty::SetReadOnly(bool bReadOnly)
{
    if (m_bReadOnly != bReadOnly) {
        m_bReadOnly = bReadOnly;
        EnableEditControl(!bReadOnly);
    }
}

std::wstring PropertyGridProperty::GetPropertyNewValue() const
{
    return GetPropertyValue();
}

////////////////////////////////////////////////////////////////////////////
/// 控件的基类
template<typename InheritType = Control>
class PropertyGridEditTemplate : public InheritType
{
public:
    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override
    {
        if (this->IsDisabledEvents(msg)) {
            //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
            Box* pParent = this->GetParent();
            if (pParent != nullptr) {
                pParent->SendEvent(msg);
            }
        }
        else {
            if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
                //鼠标消息，转给父控件
                Box* pParent = this->GetParent();
                if (pParent != nullptr) {
                    pParent->SendEvent(msg);
                }
            }
            __super::HandleEvent(msg);
        }
    }

    /** 初始化函数
     */
    virtual void OnInit() override
    {
        if (this->IsInited()) {
            return;
        }
        __super::OnInit();
        this->SetShowFocusRect(false);
        this->SetTabStop(false);
    }
};

/** 编辑框控件
*/
typedef PropertyGridEditTemplate<RichEdit> PropertyGridRichEdit;

PropertyGridTextProperty::PropertyGridTextProperty(const std::wstring& propertyName,
                                                   const std::wstring& propertyValue,
                                                   const std::wstring& description,
                                                   size_t nPropertyData):
    PropertyGridProperty(propertyName, propertyValue, description, nPropertyData),
    m_pRichEdit(nullptr),
    m_bPassword(false)
{
}

void PropertyGridTextProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pRichEdit);
        m_pRichEdit = nullptr;
        return;
    }
    if (m_pRichEdit != nullptr) {
        return;
    }
    m_pRichEdit = new PropertyGridRichEdit;
    m_pRichEdit->SetWindow(GetWindow());
    m_pRichEdit->SetClass(L"property_grid_propterty_edit");
    if (!AddPropertySubItem(m_pRichEdit)) {
        delete m_pRichEdit;
        m_pRichEdit = nullptr;
        return;
    }
    //编辑框的属性：在property_grid.xml中定义    
    m_pRichEdit->SetText(GetPropertyText());
    m_pRichEdit->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pRichEdit->AttachReturn([this](const EventArgs&) {
        SetPropertyFocus();
        return true;
        });
    m_pRichEdit->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });
}

Control* PropertyGridTextProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pRichEdit == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_pRichEdit->SetVisible(true);
        m_pRichEdit->SetFocus();
    }
    else {
        std::wstring newText = m_pRichEdit->GetText();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        if (IsPassword()) {
            std::wstring showText;
            showText.resize(newText.size(), L'*');
            SetPropertyText(showText, bChanged);
        }
        else {
            SetPropertyText(newText, bChanged);
        }
        m_pRichEdit->SetVisible(false);
    }
    return m_pRichEdit;
}

void PropertyGridTextProperty::SetPassword(bool bPassword)
{
    m_bPassword = bPassword;
    if (m_pRichEdit == nullptr) {
        return;
    }
    m_pRichEdit->SetPassword(bPassword);
    m_pRichEdit->SetFlashPasswordChar(true);
    std::wstring text = m_pRichEdit->GetText();
    bool bChanged = text != GetPropertyValue(); //相对原值，是否有修改
    if (bPassword) {
        std::wstring showText;
        showText.resize(text.size(), L'*');
        SetPropertyText(showText, bChanged);
    }
    else {
        SetPropertyText(text, bChanged);
    }
}

void PropertyGridTextProperty::SetEnableSpin(bool bEnable, int32_t nMin, int32_t nMax)
{
    RichEdit* pRichEdit = GetRichEdit();
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit != nullptr) {
        std::wstring spinClass = L"property_grid_spin_box,property_grid_spin_btn_up,property_grid_spin_btn_down";
        pRichEdit->SetEnableSpin(bEnable, spinClass, nMin, nMax);
    }
}

std::wstring PropertyGridTextProperty::GetPropertyNewValue() const
{
    std::wstring propertyValue = GetPropertyValue();
    if (!IsReadOnly() && (m_pRichEdit != nullptr)) {
        propertyValue = m_pRichEdit->GetText();
    }
    return propertyValue;
}

////////////////////////////////////////////////////////////////////////////
///

/** 编辑框控件
*/
typedef PropertyGridEditTemplate<Combo> PropertyGridCombo;

PropertyGridComboProperty::PropertyGridComboProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridProperty(propertyName, propertyValue, description, nPropertyData),
    m_pCombo(nullptr)
{
}

void PropertyGridComboProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pCombo);
        m_pCombo = nullptr;
        return;
    }
    if (m_pCombo != nullptr) {
        return;
    }
    m_pCombo = new PropertyGridCombo;
    m_pCombo->SetWindow(GetWindow());
    m_pCombo->SetClass(L"property_grid_combo");
    if (!AddPropertySubItem(m_pCombo)) {
        delete m_pCombo;
        m_pCombo = nullptr;
        return;
    }
    //编辑框的属性：在property_grid.xml中定义    
    m_pCombo->SetText(GetPropertyText());
    m_pCombo->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pCombo->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });
}

Control* PropertyGridComboProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pCombo == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_pCombo->SetVisible(true);
        m_pCombo->SetFocus();
    }
    else {
        std::wstring newText = m_pCombo->GetText();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pCombo->SetVisible(false);
    }
    return m_pCombo;
}

void PropertyGridComboProperty::OnScrollPosChanged()
{
    if ((m_pCombo != nullptr) && m_pCombo->IsVisible()) {
        m_pCombo->UpdateComboWndPos();
    }
}

std::wstring PropertyGridComboProperty::GetPropertyNewValue() const
{
    std::wstring propertyValue = GetPropertyValue();
    if (!IsReadOnly() && (m_pCombo != nullptr)) {
        propertyValue = m_pCombo->GetText();
    }
    return propertyValue;
}

size_t PropertyGridComboProperty::AddOption(const std::wstring& optionText)
{
    size_t nIndex = Box::InvalidIndex;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        nIndex = m_pCombo->AddTextItem(optionText);
    }
    return nIndex;
}

void PropertyGridComboProperty::SetOptionData(size_t nIndex, size_t nOptionData)
{
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        m_pCombo->SetItemData(nIndex, nOptionData);
    }
}

size_t PropertyGridComboProperty::GetOptionData(size_t nIndex) const
{
    size_t nOptionData = 0;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        nOptionData = m_pCombo->GetItemData(nIndex);
    }
    return nOptionData;
}

size_t PropertyGridComboProperty::GetOptionCount() const
{
    size_t nCount = 0;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        nCount = m_pCombo->GetCount();
    }
    return nCount;
}

std::wstring PropertyGridComboProperty::GetOption(size_t nIndex) const
{
    std::wstring text;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        text = m_pCombo->GetItemText(nIndex);
    }
    return text;
}

bool PropertyGridComboProperty::RemoveOption(size_t nIndex)
{
    bool bRet = false;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        bRet = m_pCombo->DeleteItem(nIndex);
    }
    return bRet;
}

void PropertyGridComboProperty::RemoveAllOptions()
{
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        m_pCombo->DeleteAllItems();
    }
}

size_t PropertyGridComboProperty::GetCurSel() const
{
    size_t nIndex = Box::InvalidIndex;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        nIndex = m_pCombo->GetCurSel();
    }
    return nIndex;
}

bool PropertyGridComboProperty::SetCurSel(size_t nIndex)
{
    bool bRet = false;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        bRet = m_pCombo->SetCurSel(nIndex);
    }
    return bRet;
}

void PropertyGridComboProperty::SetComboListMode(bool bListMode)
{
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        m_pCombo->SetComboType(bListMode ? Combo::ComboType::kCombo_DropList : Combo::ComboType::kCombo_DropDown);
    }
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFontProperty::PropertyGridFontProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridComboProperty(propertyName, propertyValue, description, nPropertyData)
{
}

std::wstring PropertyGridFontProperty::GetPropertyNewValue() const
{
    return __super::GetPropertyNewValue();
}

void PropertyGridFontProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    std::vector<std::wstring> fontList; 
    GetSystemFontList(fontList);
    for (const std::wstring& fontName : fontList) {
        AddOption(fontName);
    }
}

namespace PropertyGridFontPropertyImpl
{
    struct FontInfo
    {
        LOGFONT lf;
        DWORD fontType;
    };

    //枚举字体的回调函数
    static int EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* /*lpntme*/, DWORD fontType, LPARAM lParam)
    {
        std::vector<FontInfo>* pFontList = (std::vector<FontInfo>*)lParam;
        if (pFontList != nullptr) {
            FontInfo fontInfo;
            if (lpelfe != nullptr) {
                fontInfo.lf = *lpelfe;
            }
            else {
                fontInfo.lf = {};
            }
            fontInfo.fontType = fontType;
            pFontList->emplace_back(std::move(fontInfo));
            return 1;
        }
        else {
            return 0;
        }
    }

    static void GetSystemFontList(Window* pWindow, std::vector<std::wstring>& fontNameList)
    {
        fontNameList.clear();
        std::vector<FontInfo> fontList;
        HDC hDC = pWindow != nullptr ? pWindow->GetPaintDC() : nullptr;        
        LOGFONT logfont = {};
        logfont.lfCharSet = DEFAULT_CHARSET;
        logfont.lfFaceName[0] = L'\0';
        logfont.lfPitchAndFamily = 0;
        ::EnumFontFamiliesEx(hDC, &logfont, EnumFontFamExProc, (LPARAM)&fontList, 0);

        //字体名称列表
        std::map<std::wstring, FontInfo> fontMap;
        for (auto font : fontList) {
            if (font.lf.lfWeight != FW_NORMAL) {
                continue;
            }
            if (font.lf.lfFaceName[0] == L'@') {
                continue;
            }
            fontMap[font.lf.lfFaceName] = font;
        }
        for (auto iter : fontMap) {
            fontNameList.push_back(iter.second.lf.lfFaceName);
        }
    }

} //end of namepsace PropertyGridFontPropertyImpl

void PropertyGridFontProperty::GetSystemFontList(std::vector<std::wstring>& fontList) const
{
    PropertyGridFontPropertyImpl::GetSystemFontList(GetWindow(), fontList);
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFontSizeProperty::PropertyGridFontSizeProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridComboProperty(propertyName, propertyValue, description, nPropertyData)
{
}

std::wstring PropertyGridFontSizeProperty::GetPropertyNewValue() const
{
    return __super::GetPropertyNewValue();
}

void PropertyGridFontSizeProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    if (m_fontSizeList.empty()) {
        GetSystemFontSizeList(m_fontSizeList);
        const size_t nCount = m_fontSizeList.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            size_t nItem = AddOption(m_fontSizeList[nIndex].fontSizeName);
            SetOptionData(nItem, nIndex);
        }
    }    
}

void PropertyGridFontSizeProperty::GetSystemFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const
{
    fontSizeList.clear();
    fontSizeList.push_back({ L"8",  8.0f, 0 });
    fontSizeList.push_back({ L"9",  9.0f, 0 });
    fontSizeList.push_back({ L"10", 10.0f, 0 });
    fontSizeList.push_back({ L"11", 11.0f, 0 });
    fontSizeList.push_back({ L"12", 12.0f, 0 });
    fontSizeList.push_back({ L"14", 14.0f, 0 });
    fontSizeList.push_back({ L"16", 16.0f, 0 });
    fontSizeList.push_back({ L"18", 18.0f, 0 });
    fontSizeList.push_back({ L"20", 20.0f, 0 });
    fontSizeList.push_back({ L"22", 22.0f, 0 });
    fontSizeList.push_back({ L"24", 24.0f, 0 });
    fontSizeList.push_back({ L"26", 26.0f, 0 });
    fontSizeList.push_back({ L"28", 28.0f, 0 });
    fontSizeList.push_back({ L"32", 32.0f, 0 });
    fontSizeList.push_back({ L"36", 36.0f, 0 });
    fontSizeList.push_back({ L"48", 48.0f, 0 });
    fontSizeList.push_back({ L"72", 72.0f, 0 });
    fontSizeList.push_back({ L"1英寸", 95.6f, 0 });
    fontSizeList.push_back({ L"大特号", 83.7f, 0 });
    fontSizeList.push_back({ L"特号", 71.7f, 0 });
    fontSizeList.push_back({ L"初号", 56.0f, 0 });
    fontSizeList.push_back({ L"小初", 48.0f, 0 });
    fontSizeList.push_back({ L"一号", 34.7f, 0 });
    fontSizeList.push_back({ L"小一", 32.0f, 0 });
    fontSizeList.push_back({ L"二号", 29.3f, 0 });
    fontSizeList.push_back({ L"小二", 24.0f, 0 });
    fontSizeList.push_back({ L"三号", 21.3f, 0 });
    fontSizeList.push_back({ L"小三", 20.0f, 0 });
    fontSizeList.push_back({ L"四号", 18.7f, 0 });
    fontSizeList.push_back({ L"小四", 16.0f, 0 });
    fontSizeList.push_back({ L"五号", 14.0f, 0 });
    fontSizeList.push_back({ L"小五", 12.0f, 0 });
    fontSizeList.push_back({ L"六号", 10.0f, 0 });
    fontSizeList.push_back({ L"小六", 8.7f, 0 });
    fontSizeList.push_back({ L"七号", 7.3f, 0 });
    fontSizeList.push_back({ L"八号", 6.7f, 0 });

    //更新DPI自适应值
    for (FontSizeInfo& fontSize : fontSizeList) {
        int32_t nSize = static_cast<int32_t>(fontSize.fFontSize * 1000);
        ui::GlobalManager::Instance().Dpi().ScaleInt(nSize);
        fontSize.fDpiFontSize = nSize / 1000.0f;
    }
}

std::wstring PropertyGridFontSizeProperty::GetFontSize() const
{
    std::wstring fontSize;
    size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        size_t nIndex = GetOptionData(nCurSel);
        if (nIndex < m_fontSizeList.size()) {
            fontSize = StringHelper::Printf(L"%.01f", m_fontSizeList[nIndex].fFontSize);
        }
    }
    if (fontSize.empty()) {
        fontSize = GetFontSize(GetPropertyNewValue());
    }
    return fontSize;
}

std::wstring PropertyGridFontSizeProperty::GetDpiFontSize() const
{
    std::wstring fontSize;
    size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        size_t nIndex = GetOptionData(nCurSel);
        if (nIndex < m_fontSizeList.size()) {
            fontSize = StringHelper::Printf(L"%.01f", m_fontSizeList[nIndex].fDpiFontSize);
        }
    }
    if (fontSize.empty()) {
        fontSize = GetDpiFontSize(GetPropertyNewValue());
    }
    return fontSize;
}

std::wstring PropertyGridFontSizeProperty::GetFontSize(const std::wstring& fontSizeName) const
{
    std::wstring fontSize;
    const size_t nCount = m_fontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringHelper::Printf(L"%.01f", m_fontSizeList[nIndex].fFontSize);
            break;
        }
    }
    return fontSize;
}

std::wstring PropertyGridFontSizeProperty::GetDpiFontSize(const std::wstring& fontSizeName) const
{
    std::wstring fontSize;
    const size_t nCount = m_fontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringHelper::Printf(L"%.01f", m_fontSizeList[nIndex].fDpiFontSize);
            break;
        }
    }
    return fontSize;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridColorProperty::PropertyGridColorProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridProperty(propertyName, propertyValue, description, nPropertyData),
    m_pComboButton(nullptr)
{
}

void PropertyGridColorProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pComboButton);
        m_pComboButton = nullptr;
        return;
    }
    if (m_pComboButton != nullptr) {
        return;
    }
    m_pComboButton = new ComboButton;
    m_pComboButton->SetWindow(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pComboButton->SetClass(L"property_grid_combo_button");
    if (!AddPropertySubItem(m_pComboButton)) {
        delete m_pComboButton;
        m_pComboButton = nullptr;
        return;
    }

    Label* pLabelText = m_pComboButton->GetLabelTop();
    if (pLabelText != nullptr) {
        pLabelText->SetText(GetPropertyText());
    }
    Label* pLabelColor = m_pComboButton->GetLabelBottom();
    if (pLabelColor != nullptr) {
        pLabelColor->SetBkColor(GetPropertyText());
    }

    //更新字体颜色
    SetPropertyTextColor(GetPropertyText());

    m_pComboButton->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pComboButton->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });

    InitColorCombo();
}

Control* PropertyGridColorProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pComboButton == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_pComboButton->SetVisible(true);
        m_pComboButton->SetFocus();
    }
    else {
        std::wstring newText;
        Label* pColorLabel = m_pComboButton->GetLabelTop();
        if (pColorLabel != nullptr) {
            newText = pColorLabel->GetText();
        }
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        SetPropertyTextColor(newText);
        m_pComboButton->SetVisible(false);
    }
    return m_pComboButton;
}

void PropertyGridColorProperty::OnScrollPosChanged()
{
    if ((m_pComboButton != nullptr) && m_pComboButton->IsVisible()) {
        m_pComboButton->UpdateComboWndPos();
    }
}

void PropertyGridColorProperty::InitColorCombo()
{
    ComboButton* pColorComboBtn = m_pComboButton;
    if (pColorComboBtn == nullptr) {
        return;
    }
    UiSize boxSize = pColorComboBtn->GetDropBoxSize();
    Box* pComboBox = pColorComboBtn->GetComboBox();
    if (pComboBox == nullptr) {
        return;
    }
    pComboBox->SetWindow(GetWindow());
    GlobalManager::Instance().FillBoxWithCache(pComboBox, L"public/property_grid/color_combox.xml");
    pComboBox->SetFixedHeight(UiFixedInt(boxSize.cy), false, false);
    pComboBox->SetFixedWidth(UiFixedInt(boxSize.cx), false, false);

    if (pComboBox->GetItemAt(0) != nullptr) {
        pComboBox->GetItemAt(0)->SetFixedHeight(UiFixedInt(boxSize.cy), false, false);
        pComboBox->GetItemAt(0)->SetFixedWidth(UiFixedInt(boxSize.cx), false, false);
    }

    ColorPickerRegular* pColorPicker = dynamic_cast<ColorPickerRegular*>(pComboBox->FindSubControl(L"color_combo_picker"));
    if (pColorPicker != nullptr) {
        //响应选择颜色事件
        pColorPicker->AttachSelectColor([this, pColorComboBtn](const EventArgs& args) {
            UiColor newColor((uint32_t)args.wParam);
            //设置选择后的颜色
            Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
            if (pLeftColorLabel != nullptr) {
                pLeftColorLabel->SetBkColor(newColor);
                OnSelectColor(pLeftColorLabel->GetBkColor());
            }
            return true;
            });
    }

    Button* pMoreColorButton = dynamic_cast<Button*>(pComboBox->FindSubControl(L"color_combo_picker_more"));
    if (pMoreColorButton != nullptr) {
        pMoreColorButton->AttachClick([this](const EventArgs& /*args*/) {
            ShowColorPicker();
            return true;
            });
    }
}

void PropertyGridColorProperty::ShowColorPicker()
{
    ComboButton* pColorComboBtn = m_pComboButton;
    if (pColorComboBtn == nullptr) {
        return;
    }
    Label* pColorLabel = pColorComboBtn->GetLabelBottom();//颜色显示控件
    if (pColorLabel == nullptr) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    std::wstring oldTextColor = pColorLabel->GetBkColor(); //原来的颜色

    ColorPicker* pColorPicker = new ColorPicker;
    pColorPicker->CreateWnd(pWindow->GetHWND(), ColorPicker::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    pColorPicker->CenterWindow();
    pColorPicker->ShowModalFake(pWindow->GetHWND());

    if (!oldTextColor.empty() && (pColorPicker != nullptr)) {
        pColorPicker->SetSelectedColor(GetUiColor(oldTextColor));
    }
    //如果在界面选择颜色，则临时更新控件文本的颜色
    pColorPicker->AttachSelectColor([this, pColorLabel](const ui::EventArgs& args) {
        ui::UiColor newColor = ui::UiColor((uint32_t)args.wParam);
        pColorLabel->SetBkColor(newColor);
        OnSelectColor(GetColorString(newColor));
        return true;
        });

    //窗口关闭事件
    pColorPicker->AttachWindowClose([this, pColorPicker, oldTextColor, pColorLabel](const ui::EventArgs& args) {
        ui::UiColor newColor = pColorPicker->GetSelectedColor();
        if ((args.wParam == 0) && !newColor.IsEmpty()) {
            //如果是"确认"，则设置控件的文本颜色
            pColorLabel->SetBkColor(newColor);
            OnSelectColor(GetColorString(newColor));
        }
        else {
            //如果是"取消"或者关闭窗口，则恢复原来的颜色
            pColorLabel->SetBkColor(newColor);
            OnSelectColor(oldTextColor);
        }
        return true;
        });
}

void PropertyGridColorProperty::OnSelectColor(const std::wstring& color)
{
    if (m_pComboButton == nullptr) {
        return;
    }
    Label* pLabelText = m_pComboButton->GetLabelTop();
    if (pLabelText != nullptr) {
        pLabelText->SetText(color);
    }
    Label* pLabelColor = m_pComboButton->GetLabelBottom();
    if (pLabelColor != nullptr) {
        pLabelColor->SetBkColor(color);
    }
}

////////////////////////////////////////////////////////////////////////////
///
/** 日期时间控件
*/
typedef PropertyGridEditTemplate<DateTime> PropertyGridDateTime;

PropertyGridDateTimeProperty::PropertyGridDateTimeProperty(const std::wstring& propertyName,
        const std::wstring& dateTimeValue,
        const std::wstring& description,
        size_t nPropertyData,
        DateTime::EditFormat editFormat):
    PropertyGridProperty(propertyName, dateTimeValue, description, nPropertyData),
    m_pDateTime(nullptr)
{
    m_editFormat = editFormat;
}

void PropertyGridDateTimeProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pDateTime);
        m_pDateTime = nullptr;
        return;
    }
    if (m_pDateTime != nullptr) {
        return;
    }
    m_pDateTime = new PropertyGridDateTime;
    m_pDateTime->SetWindow(GetWindow());
    //属性：在property_grid.xml中定义
    m_pDateTime->SetClass(L"property_grid_date_time");
    if (!AddPropertySubItem(m_pDateTime)) {
        delete m_pDateTime;
        m_pDateTime = nullptr;
        return;
    }

    m_pDateTime->SetEditFormat(m_editFormat);
    bool bValid = m_pDateTime->SetDateTimeString(GetPropertyText());
    m_pDateTime->SetVisible(false);
    m_pDateTime->SetText(GetPropertyText());

    ASSERT_UNUSED_VARIABLE(bValid);
    auto s0 = m_pDateTime->GetDateTimeString();
    auto s1 = GetPropertyText();
    ASSERT(m_pDateTime->GetDateTimeString() == GetPropertyText());

    //挂载焦点切换事件
    m_pDateTime->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });
}

Control* PropertyGridDateTimeProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pDateTime == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        int32_t nWidth = GetEditControlMarginRight();
        UiMargin rcMargin = m_pDateTime->GetMargin();
        rcMargin.right = nWidth;
        m_pDateTime->SetMargin(rcMargin, false);
        m_pDateTime->SetVisible(true);
        m_pDateTime->SetFocus();
    }
    else {
        std::wstring newText = m_pDateTime->GetText();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pDateTime->SetVisible(false);
    }
    return m_pDateTime;
}

void PropertyGridDateTimeProperty::OnScrollPosChanged()
{
    if ((m_pDateTime != nullptr) && m_pDateTime->IsVisible()) {
        m_pDateTime->UpdateEditWndPos();
    }
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridIPAddressProperty::PropertyGridIPAddressProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridProperty(propertyName, propertyValue, description, nPropertyData),
    m_pIPAddress(nullptr)
{
}

void PropertyGridIPAddressProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pIPAddress);
        m_pIPAddress = nullptr;
        return;
    }
    if (m_pIPAddress != nullptr) {
        return;
    }
    m_pIPAddress = new IPAddress;
    m_pIPAddress->SetWindow(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pIPAddress->SetClass(L"property_grid_ip_address");
    if (!AddPropertySubItem(m_pIPAddress)) {
        delete m_pIPAddress;
        m_pIPAddress = nullptr;
        return;
    }

    m_pIPAddress->SetIPAddress(GetPropertyText());
    m_pIPAddress->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pIPAddress->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });
}

Control* PropertyGridIPAddressProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pIPAddress == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_pIPAddress->SetVisible(true);
        m_pIPAddress->SetFocus();
    }
    else {
        std::wstring newText = m_pIPAddress->GetIPAddress();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pIPAddress->SetVisible(false);
    }
    return m_pIPAddress;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridHotKeyProperty::PropertyGridHotKeyProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridProperty(propertyName, propertyValue, description, nPropertyData),
    m_pHotKey(nullptr)
{
}

void PropertyGridHotKeyProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pHotKey);
        m_pHotKey = nullptr;
        return;
    }
    if (m_pHotKey != nullptr) {
        return;
    }
    m_pHotKey = new HotKey;
    m_pHotKey->SetWindow(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pHotKey->SetClass(L"property_grid_hot_key");
    if (!AddPropertySubItem(m_pHotKey)) {
        delete m_pHotKey;
        m_pHotKey = nullptr;
        return;
    }

    m_pHotKey->SetHotKeyName(GetPropertyText());
    m_pHotKey->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pHotKey->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false);
        return true;
        });
}

Control* PropertyGridHotKeyProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pHotKey == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_pHotKey->SetVisible(true);
        m_pHotKey->SetFocus();
    }
    else {
        std::wstring newText = m_pHotKey->GetHotKeyName();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pHotKey->SetVisible(false);
    }
    return m_pHotKey;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFileProperty::PropertyGridFileProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData,
    bool bOpenFileDialog,
    const std::vector<FileDialog::FileType>& fileTypes,
    int32_t nFileTypeIndex,
    const std::wstring& defaultExt) :
    PropertyGridTextProperty(propertyName, propertyValue, description, nPropertyData),
    m_pBrowseBtn(nullptr),
    m_bOpenFileDialog(bOpenFileDialog),
    m_fileTypes(fileTypes),
    m_nFileTypeIndex(nFileTypeIndex),
    m_defaultExt(defaultExt)
{
}

void PropertyGridFileProperty::EnableEditControl(bool bEnable)
{
    __super::EnableEditControl(bEnable);
    if (!bEnable) {
        return;
    }
    RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    if (m_pBrowseBtn != nullptr) {
        size_t nItemIndex = pRichEdit->GetItemIndex(m_pBrowseBtn);
        if (nItemIndex == Box::InvalidIndex) {
            m_pBrowseBtn = nullptr;
        }
    }
    if (m_pBrowseBtn == nullptr) {
        m_pBrowseBtn = new Button;
        m_pBrowseBtn->SetWindow(GetWindow());
        m_pBrowseBtn->SetClass(L"property_grid_button");
        m_pBrowseBtn->SetNoFocus();
        pRichEdit->AddItem(m_pBrowseBtn);

        //点击事件
        m_pBrowseBtn->AttachClick([this](const EventArgs&) {
            OnBrowseButtonClicked();
            return true;
            });
    }
}

void PropertyGridFileProperty::OnBrowseButtonClicked()
{
    std::wstring filePath;
    FileDialog fileDlg;
    if (fileDlg.BrowseForFile(GetWindow(), filePath, m_bOpenFileDialog, m_fileTypes, m_nFileTypeIndex, m_defaultExt)) {
        RichEdit* pRichEdit = GetRichEdit();
        if (pRichEdit != nullptr) {
            pRichEdit->SetText(filePath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridDirectoryProperty::PropertyGridDirectoryProperty(const std::wstring& propertyName,
    const std::wstring& propertyValue,
    const std::wstring& description,
    size_t nPropertyData) :
    PropertyGridTextProperty(propertyName, propertyValue, description, nPropertyData),
    m_pBrowseBtn(nullptr)
{
}

void PropertyGridDirectoryProperty::EnableEditControl(bool bEnable)
{
    __super::EnableEditControl(bEnable);
    if (!bEnable) {
        return;
    }
    RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    if (m_pBrowseBtn != nullptr) {
        size_t nItemIndex = pRichEdit->GetItemIndex(m_pBrowseBtn);
        if (nItemIndex == Box::InvalidIndex) {
            m_pBrowseBtn = nullptr;
        }
    }
    if (m_pBrowseBtn == nullptr) {
        m_pBrowseBtn = new Button;
        m_pBrowseBtn->SetWindow(GetWindow());
        m_pBrowseBtn->SetClass(L"property_grid_button");
        m_pBrowseBtn->SetNoFocus();
        pRichEdit->AddItem(m_pBrowseBtn);

        //点击事件
        m_pBrowseBtn->AttachClick([this](const EventArgs&) {
            OnBrowseButtonClicked();
            return true;
            });
    }
}

void PropertyGridDirectoryProperty::OnBrowseButtonClicked()
{
    std::wstring folderPath;
    FileDialog fileDlg;
    if (fileDlg.BrowseForFolder(GetWindow(), folderPath)) {
        RichEdit* pRichEdit = GetRichEdit();
        if (pRichEdit != nullptr) {
            pRichEdit->SetText(folderPath);
        }
    }
}

}//namespace ui

