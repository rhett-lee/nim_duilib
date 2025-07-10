#include "PropertyGrid.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Control/ColorPicker.h"
#include "duilib/Control/ColorPickerRegular.h"
#include "duilib/Core/WindowCreateParam.h"

namespace ui
{

PropertyGrid::PropertyGrid(Window* pWindow):
    VBox(pWindow),
    m_bHeaderCtrl(true),
    m_pHeaderCtrl(nullptr),
    m_pHeaderLeft(nullptr),
    m_pHeaderRight(nullptr),
    m_pHeaderSplit(nullptr),
    m_pDescriptionArea(nullptr),
    m_bDescriptionArea(true),
    m_pDescriptionAreaSplit(nullptr),
    m_pTreeView(nullptr),
    m_nLeftColumnWidth(0),
    m_nRowGridLineWidth(0),
    m_nColumnGridLineWidth(0)
{
    SetLeftColumnWidth(130, true);
    SetRowGridLineWidth(1, true);
    SetColumnGridLineWidth(1, true);
}

DString PropertyGrid::GetType() const { return DUI_CTR_PROPERTY_GRID; }

void PropertyGrid::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("property_grid_xml")) {
        if (!strValue.empty()) {
            m_configXml = strValue;
        }
    }
    else if (strName == _T("row_grid_line_width")) {
        SetRowGridLineWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("row_grid_line_color")) {
        SetRowGridLineColor(strValue);
    }
    else if (strName == _T("column_grid_line_width")) {
        SetColumnGridLineWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("column_grid_line_color")) {
        SetColumnGridLineColor(strValue);
    }
    else if (strName == _T("header_class")) {
        SetHeaderClass(strValue);
    }
    else if (strName == _T("group_class")) {
        SetGroupClass(strValue);
    }
    else if (strName == _T("propterty_class")) {
        SetPropertyClass(strValue);
    }
    else if (strName == _T("left_column_width")) {
        SetLeftColumnWidth(StringUtil::StringToInt32(strValue), true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void PropertyGrid::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    int32_t iValue = GetRowGridLineWidth();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetRowGridLineWidth(iValue, false);

    iValue = GetColumnGridLineWidth();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetColumnGridLineWidth(iValue, false);

    if (!IsInited() || (m_pHeaderLeft == nullptr)) {
        m_nLeftColumnWidth = Dpi().GetScaleInt(m_nLeftColumnWidth, nOldDpiScale);
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    if (IsInited() && (m_pHeaderLeft != nullptr)) {
        m_nLeftColumnWidth = GetLeftColumnWidth();
    }
}

void PropertyGrid::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    //初始化基本结构
    if (m_configXml.empty()) {
        //默认的配置文件
        m_configXml = _T("public/property_grid/property_grid.xml");
    }
    ui::GlobalManager::Instance().FillBoxWithCache(this, FilePath(m_configXml.c_str()));
    m_pHeaderCtrl = FindSubControl(_T("duilib_property_grid_header"));
    m_pHeaderLeft = dynamic_cast<Label*>(FindSubControl(_T("duilib_property_grid_header_left")));
    m_pHeaderRight = dynamic_cast<Label*>(FindSubControl(_T("duilib_property_grid_header_right")));
    m_pHeaderSplit = dynamic_cast<Split*>(FindSubControl(_T("duilib_property_grid_header_split")));
    if (m_pHeaderCtrl != nullptr) {
        ASSERT(m_pHeaderLeft != nullptr);
        ASSERT(m_pHeaderRight != nullptr);
        ASSERT(m_pHeaderSplit != nullptr);
        m_pHeaderCtrl->SetClass(GetHeaderClass());
    }
    else {
        m_bHeaderCtrl = false;
    }

    m_pDescriptionArea = dynamic_cast<RichText*>(FindSubControl(_T("duilib_property_grid_description_area")));
    m_bDescriptionArea = m_pDescriptionArea != nullptr;
    m_pDescriptionAreaSplit = dynamic_cast<Split*>(FindSubControl(_T("duilib_property_grid_description_area_split")));

    m_pTreeView = dynamic_cast<TreeView*>(FindSubControl(_T("duilib_property_grid_tree")));
    ASSERT(m_pTreeView != nullptr);

    //挂载拖动响应事件, 调整列的宽度
    if (m_pHeaderSplit != nullptr) {
        m_pHeaderSplit->AttachSplitDraged([this](const EventArgs& /*args*/) {
            OnHeaderColumnResized();
            return true;
            });
    }

    //初始化第一列宽度
    SetLeftColumnWidth(GetLeftColumnWidthValue(), false);

    //关联描述区域
    if ((m_pDescriptionArea != nullptr) && (m_pTreeView != nullptr)) {
        m_pTreeView->AttachSelect([this](const EventArgs& args) {
            Control* pItem = nullptr;
            size_t nItem = args.wParam;
            if (nItem != Box::InvalidIndex) {
                pItem = m_pTreeView->GetItemAt(nItem);
            }
            DString description;
            DString name;
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
                    description = _T("<b>") + name + _T("</b><br/>") + description;
                }
                m_pDescriptionArea->SetText(description);
            }
            return true;
            });
    }
}

void PropertyGrid::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::PaintChild(pRender, rcPaint);

    //网格线的绘制
    PaintGridLines(pRender);
}

void PropertyGrid::PaintGridLines(IRender* pRender)
{
    if ((m_pTreeView == nullptr) || (pRender == nullptr)) {
        return;
    }
    int32_t nColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度        
    int32_t nRowLineWidth = GetRowGridLineWidth();   //横向边线宽度
    UiColor columnLineColor;
    UiColor rowLineColor;
    DString color = GetColumnGridLineColor();
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
                                       const DString& sLeftColumn,
                                       const DString& sRightColumn)
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
        Dpi().ScaleInt(nLineWidth);
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

void PropertyGrid::SetRowGridLineColor(const DString& color)
{
    if (m_rowGridLineColor != color) {
        m_rowGridLineColor = color;
        Invalidate();
    }
}

DString PropertyGrid::GetRowGridLineColor() const
{
    return m_rowGridLineColor.c_str();
}

void PropertyGrid::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nLineWidth);
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

void PropertyGrid::SetColumnGridLineColor(const DString& color)
{
    if (m_columnGridLineColor != color) {
        m_columnGridLineColor = color;
        Invalidate();
    }
}

DString PropertyGrid::GetColumnGridLineColor() const
{
    return m_columnGridLineColor.c_str();
}

void PropertyGrid::SetHeaderClass(const DString& headerClass)
{
    if (m_headerClass != headerClass) {
        m_headerClass = headerClass;
        if (m_pHeaderCtrl != nullptr) {
            m_pHeaderCtrl->SetClass(headerClass);
        }
    }
}

DString PropertyGrid::GetHeaderClass() const
{
    return m_headerClass.c_str();
}

void PropertyGrid::SetGroupClass(const DString& groupClass)
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

DString PropertyGrid::GetGroupClass() const
{
    return m_groupClass.c_str();
}

void PropertyGrid::SetPropertyClass(const DString& propertyClass)
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

DString PropertyGrid::GetPropertyClass() const
{
    return m_propertyClass.c_str();
}

PropertyGridGroup* PropertyGrid::AddGroup(const DString& groupName,
                                          const DString& description,
                                          size_t nGroupData)
{
    ASSERT(m_pTreeView != nullptr);
    if (m_pTreeView == nullptr) {
        return nullptr;
    }
    PropertyGridGroup* pGroup = new PropertyGridGroup(GetWindow(), groupName, description, nGroupData);
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
                                                        const DString& propertyName,
                                                        const DString& propertyValue,
                                                        const DString& description,
                                                        size_t nPropertyData)
{
    PropertyGridTextProperty* pProperty = new PropertyGridTextProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridComboProperty* PropertyGrid::AddComboProperty(PropertyGridGroup* pGroup,
                                                          const DString& propertyName,
                                                          const DString& propertyValue,
                                                          const DString& description,
                                                          size_t nPropertyData)
{
    PropertyGridComboProperty* pProperty = new PropertyGridComboProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontProperty* PropertyGrid::AddFontProperty(PropertyGridGroup* pGroup,
                                                        const DString& propertyName,
                                                        const DString& propertyValue,
                                                        const DString& description,
                                                        size_t nPropertyData)
{
    PropertyGridFontProperty* pProperty = new PropertyGridFontProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizeProperty(PropertyGridGroup* pGroup,
                                                                const DString& propertyName,
                                                                const DString& propertyValue,
                                                                const DString& description,
                                                                size_t nPropertyData)
{
    PropertyGridFontSizeProperty* pProperty = new PropertyGridFontSizeProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridColorProperty* PropertyGrid::AddColorProperty(PropertyGridGroup* pGroup,
                                                          const DString& propertyName,
                                                          const DString& propertyValue,
                                                          const DString& description,
                                                          size_t nPropertyData)
{
    PropertyGridColorProperty* pProperty = new PropertyGridColorProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimeProperty(PropertyGridGroup* pGroup,
                                                                const DString& propertyName,
                                                                const DString& dateTimeValue,                                                                
                                                                const DString& description,
                                                                size_t nPropertyData,
                                                                DateTime::EditFormat editFormat)
{
    PropertyGridDateTimeProperty* pProperty = new PropertyGridDateTimeProperty(GetWindow(), propertyName, dateTimeValue, description, nPropertyData, editFormat);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressProperty(PropertyGridGroup* pGroup,
                                                                  const DString& propertyName,
                                                                  const DString& propertyValue,
                                                                  const DString& description,
                                                                  size_t nPropertyData)
{
    PropertyGridIPAddressProperty* pProperty = new PropertyGridIPAddressProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyProperty(PropertyGridGroup* pGroup,
                                                            const DString& propertyName,
                                                            const DString& propertyValue,
                                                            const DString& description,
                                                            size_t nPropertyData)
{
    PropertyGridHotKeyProperty* pProperty = new PropertyGridHotKeyProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFileProperty* PropertyGrid::AddFileProperty(PropertyGridGroup* pGroup,
                                                        const DString& propertyName,
                                                        const DString& propertyValue,                                                        
                                                        const DString& description,
                                                        size_t nPropertyData,
                                                        bool bOpenFileDialog,
                                                        const std::vector<FileDialog::FileType>& fileTypes,
                                                        int32_t nFileTypeIndex,
                                                        const DString& defaultExt)
{
    PropertyGridFileProperty* pProperty = new PropertyGridFileProperty(GetWindow(), propertyName, propertyValue,
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
                                                                  const DString& propertyName,
                                                                  const DString& propertyValue,
                                                                  const DString& description,
                                                                  size_t nPropertyData)
{
    PropertyGridDirectoryProperty* pProperty = new PropertyGridDirectoryProperty(GetWindow(), propertyName, propertyValue, description, nPropertyData);
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
        Dpi().ScaleInt(nLeftColumnWidth);
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
    int32_t nLeftColumnWidth = GetLeftColumnWidthValue();
    if (IsInited() && (m_pHeaderLeft != nullptr)) {
        int32_t nSplitWidth = 0;
        if (m_pHeaderSplit != nullptr) {
            nSplitWidth = m_pHeaderSplit->GetFixedWidth().GetInt32();
        }
        nLeftColumnWidth = m_pHeaderLeft->GetFixedWidth().GetInt32() + nSplitWidth / 2;
    }
    return nLeftColumnWidth;
}

int32_t PropertyGrid::GetLeftColumnWidthValue() const
{
    return m_nLeftColumnWidth;
}

////////////////////////////////////////////////////////////////////////////
///

PropertyGridGroup::PropertyGridGroup(Window* pWindow, 
                                     const DString& groupName,
                                     const DString& description,
                                     size_t nGroupData) :
    TreeNode(pWindow),
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
    BaseClass::OnInit();
    SetTabStop(false);

    HBox* pHBox = new HBox(GetWindow());
    AddItem(pHBox);

    pHBox->SetMouseEnabled(false);
    pHBox->SetNoFocus();

    m_pLabelBox = new LabelBox(GetWindow());
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
    typedef LabelBox BaseClass;
public:
    explicit PropertyGridLabelBox(Window* pWindow):
        LabelBox(pWindow)
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
            pParent->SendEventMsg(msg);
        }
        if (!IsDisabledEvents(msg)) {
            BaseClass::HandleEvent(msg);
        }
    }

    /** 初始化函数
     */
    virtual void OnInit() override
    {
        if (IsInited()) {
            return;
        }
        BaseClass::OnInit();
        SetShowFocusRect(true);
        SetTabStop(false);
    }
};

PropertyGridProperty::PropertyGridProperty(Window* pWindow, 
                                           const DString& propertyName,
                                           const DString& propertyValue,
                                           const DString& description,
                                           size_t nPropertyData):
    TreeNode(pWindow),
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
    BaseClass::OnInit();
    SetTabStop(false);

    m_pHBox = new HBox(GetWindow());
    AddItem(m_pHBox);
    //背景色：在property_grid.xml中定义
    m_pHBox->SetBkColor(_T("property_grid_propterty_bkcolor"));

    m_pHBox->SetMouseEnabled(false);
    m_pHBox->SetNoFocus();

    m_pLabelBoxLeft = new PropertyGridLabelBox(GetWindow());
    m_pHBox->AddItem(m_pLabelBoxLeft);
    m_pLabelBoxLeft->SetText(m_propertyName.c_str());

    m_pLabelBoxRight = new PropertyGridLabelBox(GetWindow());
    m_pHBox->AddItem(m_pLabelBoxRight);
    m_pLabelBoxRight->SetText(m_propertyValue.c_str());
    //属性值的正常字体：在property_grid.xml中定义
    m_pLabelBoxRight->SetFontId(_T("property_grid_propterty_font_normal"));

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
    EnableEditControl(!IsReadOnly());

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

void PropertyGridProperty::SetPropertyText(const DString& text, bool bChanged)
{
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        m_pLabelBoxRight->SetText(text);
        if (bChanged) {
            m_pLabelBoxRight->SetFontId(_T("property_grid_propterty_font_bold"));
        }
        else {
            m_pLabelBoxRight->SetFontId(_T("property_grid_propterty_font_normal"));
        }
    }
}

void PropertyGridProperty::SetPropertyTextColor(const DString& textColor)
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

DString PropertyGridProperty::GetPropertyText() const
{
    DString text;
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

DString PropertyGridProperty::GetPropertyNewValue() const
{
    return GetPropertyValue();
}

////////////////////////////////////////////////////////////////////////////
/// 控件的基类
template<typename InheritType = Control>
class PropertyGridEditTemplate : public InheritType
{
    typedef InheritType BaseClass;
public:
    explicit PropertyGridEditTemplate(Window* pWindow);

    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override
    {
        if (this->IsDisabledEvents(msg)) {
            //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
            Box* pParent = this->GetParent();
            if (pParent != nullptr) {
                pParent->SendEventMsg(msg);
            }
        }
        else {
            if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
                //鼠标消息，转给父控件
                Box* pParent = this->GetParent();
                if (pParent != nullptr) {
                    pParent->SendEventMsg(msg);
                }
            }
            BaseClass::HandleEvent(msg);
        }
    }

    /** 初始化函数
     */
    virtual void OnInit() override
    {
        if (this->IsInited()) {
            return;
        }
        BaseClass::OnInit();
        this->SetShowFocusRect(false);
        this->SetTabStop(false);
    }
};

template<typename InheritType>
PropertyGridEditTemplate<InheritType>::PropertyGridEditTemplate(Window* pWindow):
InheritType(pWindow)
{
}

/** 编辑框控件
*/
typedef PropertyGridEditTemplate<RichEdit> PropertyGridRichEdit;

PropertyGridTextProperty::PropertyGridTextProperty(Window* pWindow,
                                                   const DString& propertyName,
                                                   const DString& propertyValue,
                                                   const DString& description,
                                                   size_t nPropertyData):
    PropertyGridProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_pRichEdit(nullptr),
    m_bPasswordMode(false)
{
}

void PropertyGridTextProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        if (m_pRichEdit != nullptr) {
            RemovePropertySubItem(m_pRichEdit);
            m_pRichEdit = nullptr;
        }
        return;
    }
    if (m_pRichEdit != nullptr) {
        return;
    }
    m_pRichEdit = new PropertyGridRichEdit(GetWindow());
    m_pRichEdit->SetClass(_T("property_grid_propterty_edit"));
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
    m_pRichEdit->AttachEsc([this](const EventArgs&) {
        m_pRichEdit->SetText(GetPropertyText());
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
        DString newText = m_pRichEdit->GetText();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        if (IsPasswordMode()) {
            DString showText;
            showText.resize(newText.size(), _T('*'));
            SetPropertyText(showText, bChanged);
        }
        else {
            SetPropertyText(newText, bChanged);
        }
        m_pRichEdit->SetVisible(false);
    }
    return m_pRichEdit;
}

void PropertyGridTextProperty::SetPasswordMode(bool bPasswordMode)
{
    m_bPasswordMode = bPasswordMode;
    if (m_pRichEdit == nullptr) {
        return;
    }
    m_pRichEdit->SetPasswordMode(bPasswordMode);
    m_pRichEdit->SetFlashPasswordChar(true);
    DString text = m_pRichEdit->GetText();
    bool bChanged = text != GetPropertyValue(); //相对原值，是否有修改
    if (bPasswordMode) {
        DString showText;
        showText.resize(text.size(), _T('*'));
        SetPropertyText(showText, bChanged);
    }
    else {
        SetPropertyText(text, bChanged);
    }
}

void PropertyGridTextProperty::SetEnableSpin(bool bEnable, int32_t nMin, int32_t nMax)
{
    RichEdit* pRichEdit = GetRichEdit();
    if (IsEnabled()) {
        ASSERT(pRichEdit != nullptr);
    }    
    if (pRichEdit != nullptr) {
        DString spinClass = _T("property_grid_spin_box,property_grid_spin_btn_up,property_grid_spin_btn_down");
        pRichEdit->SetEnableSpin(bEnable, spinClass, nMin, nMax);
    }
}

DString PropertyGridTextProperty::GetPropertyNewValue() const
{
    DString propertyValue = GetPropertyValue();
    if (!IsReadOnly() && (m_pRichEdit != nullptr)) {
        propertyValue = m_pRichEdit->GetText();
    }
    return propertyValue;
}

void PropertyGridTextProperty::SetNewTextValue(const DString& newText)
{
    if (!IsReadOnly() && (m_pRichEdit != nullptr)) {
        m_pRichEdit->SetText(newText);
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        if (IsPasswordMode()) {
            DString showText;
            showText.resize(newText.size(), _T('*'));
            SetPropertyText(showText, bChanged);
        }
        else {
            SetPropertyText(newText, bChanged);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
///

/** 编辑框控件
*/
typedef PropertyGridEditTemplate<Combo> PropertyGridCombo;

PropertyGridComboProperty::PropertyGridComboProperty(Window* pWindow, 
                                                     const DString& propertyName,
                                                     const DString& propertyValue,
                                                     const DString& description,
                                                     size_t nPropertyData) :
    PropertyGridProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_pCombo(nullptr)
{
}

void PropertyGridComboProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        if (m_pCombo != nullptr) {
            RemovePropertySubItem(m_pCombo);
            m_pCombo = nullptr;
        }
        return;
    }
    if (m_pCombo != nullptr) {
        return;
    }
    m_pCombo = new PropertyGridCombo(GetWindow());
    m_pCombo->SetClass(_T("property_grid_combo"));
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
        DString newText = m_pCombo->GetText();
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

DString PropertyGridComboProperty::GetPropertyNewValue() const
{
    DString propertyValue = GetPropertyValue();
    if (!IsReadOnly() && (m_pCombo != nullptr)) {
        propertyValue = m_pCombo->GetText();
    }
    return propertyValue;
}

size_t PropertyGridComboProperty::AddOption(const DString& optionText)
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

DString PropertyGridComboProperty::GetOption(size_t nIndex) const
{
    DString text;
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
PropertyGridFontProperty::PropertyGridFontProperty(Window* pWindow, 
                                                   const DString& propertyName,
                                                   const DString& propertyValue,
                                                   const DString& description,
                                                   size_t nPropertyData) :
    PropertyGridComboProperty(pWindow, propertyName, propertyValue, description, nPropertyData)
{
}

DString PropertyGridFontProperty::GetPropertyNewValue() const
{
    return BaseClass::GetPropertyNewValue();
}

void PropertyGridFontProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    std::vector<DString> fontList; 
    GlobalManager::Instance().Font().GetFontNameList(fontList);
    for (const DString& fontName : fontList) {
        AddOption(fontName);
    }
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFontSizeProperty::PropertyGridFontSizeProperty(Window* pWindow, 
                                                           const DString& propertyName,
                                                           const DString& propertyValue,
                                                           const DString& description,
                                                           size_t nPropertyData) :
    PropertyGridComboProperty(pWindow, propertyName, propertyValue, description, nPropertyData)
{
}

DString PropertyGridFontSizeProperty::GetPropertyNewValue() const
{
    return BaseClass::GetPropertyNewValue();
}

void PropertyGridFontSizeProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    if (m_fontSizeList.empty()) {
        ui::GlobalManager::Instance().Font().GetFontSizeList(Dpi(), m_fontSizeList);
        const size_t nCount = m_fontSizeList.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            size_t nItem = AddOption(m_fontSizeList[nIndex].fontSizeName);
            SetOptionData(nItem, nIndex);
        }
    }    
}

DString PropertyGridFontSizeProperty::GetFontSize() const
{
    DString fontSize;
    size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        size_t nIndex = GetOptionData(nCurSel);
        if (nIndex < m_fontSizeList.size()) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fontSizeList[nIndex].fFontSize);
        }
    }
    if (fontSize.empty()) {
        fontSize = GetFontSize(GetPropertyNewValue());
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetDpiFontSize() const
{
    DString fontSize;
    size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        size_t nIndex = GetOptionData(nCurSel);
        if (nIndex < m_fontSizeList.size()) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fontSizeList[nIndex].fDpiFontSize);
        }
    }
    if (fontSize.empty()) {
        fontSize = GetDpiFontSize(GetPropertyNewValue());
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetFontSize(const DString& fontSizeName) const
{
    DString fontSize;
    const size_t nCount = m_fontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fontSizeList[nIndex].fFontSize);
            break;
        }
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetDpiFontSize(const DString& fontSizeName) const
{
    DString fontSize;
    const size_t nCount = m_fontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fontSizeList[nIndex].fDpiFontSize);
            break;
        }
    }
    return fontSize;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridColorProperty::PropertyGridColorProperty(Window* pWindow, 
                                                     const DString& propertyName,
                                                     const DString& propertyValue,
                                                     const DString& description,
                                                     size_t nPropertyData) :
    PropertyGridProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
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
    m_pComboButton = new ComboButton(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pComboButton->SetClass(_T("property_grid_combo_button"));
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
        DString newText;
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
    GlobalManager::Instance().FillBoxWithCache(pComboBox, FilePath(_T("public/property_grid/color_combox.xml")));
    pComboBox->SetFixedHeight(UiFixedInt(boxSize.cy), false, false);
    pComboBox->SetFixedWidth(UiFixedInt(boxSize.cx), false, false);

    if (pComboBox->GetItemAt(0) != nullptr) {
        pComboBox->GetItemAt(0)->SetFixedHeight(UiFixedInt(boxSize.cy), false, false);
        pComboBox->GetItemAt(0)->SetFixedWidth(UiFixedInt(boxSize.cx), false, false);
    }

    ColorPickerRegular* pColorPicker = dynamic_cast<ColorPickerRegular*>(pComboBox->FindSubControl(_T("color_combo_picker")));
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

    Button* pMoreColorButton = dynamic_cast<Button*>(pComboBox->FindSubControl(_T("color_combo_picker_more")));
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
    DString oldTextColor = pColorLabel->GetBkColor(); //原来的颜色

    ColorPicker* pColorPicker = new ColorPicker;
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_LAYERED;
    createWndParam.m_bCenterWindow = true;
    pColorPicker->CreateWnd(pWindow, createWndParam);
    pColorPicker->ShowModalFake();

    if (!oldTextColor.empty()) {
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
        if ((args.wParam == kWindowCloseOK) && !newColor.IsEmpty()) {
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

void PropertyGridColorProperty::OnSelectColor(const DString& color)
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

PropertyGridDateTimeProperty::PropertyGridDateTimeProperty(Window* pWindow, 
                                                           const DString& propertyName,
                                                           const DString& dateTimeValue,
                                                           const DString& description,
                                                           size_t nPropertyData,
                                                           DateTime::EditFormat editFormat):
    PropertyGridProperty(pWindow, propertyName, dateTimeValue, description, nPropertyData),
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
    m_pDateTime = new PropertyGridDateTime(GetWindow());
    //属性：在property_grid.xml中定义
    m_pDateTime->SetClass(_T("property_grid_date_time"));
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
        DString newText = m_pDateTime->GetText();
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
PropertyGridIPAddressProperty::PropertyGridIPAddressProperty(Window* pWindow, 
                                                             const DString& propertyName,
                                                             const DString& propertyValue,
                                                             const DString& description,
                                                             size_t nPropertyData) :
    PropertyGridProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
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
    m_pIPAddress = new IPAddress(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pIPAddress->SetClass(_T("property_grid_ip_address"));
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
        DString newText = m_pIPAddress->GetIPAddress();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pIPAddress->SetVisible(false);
    }
    return m_pIPAddress;
}

void PropertyGridIPAddressProperty::SetNewIPAddressValue(const DString& newIP)
{
    if (IsReadOnly() || (m_pIPAddress == nullptr)) {
        return;
    }
    m_pIPAddress->SetIPAddress(newIP);
    DString newText = m_pIPAddress->GetIPAddress();
    bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
    SetPropertyText(newText, bChanged);
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridHotKeyProperty::PropertyGridHotKeyProperty(Window* pWindow, 
                                                       const DString& propertyName,
                                                       const DString& propertyValue,
                                                       const DString& description,
                                                       size_t nPropertyData) :
    PropertyGridProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
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
    m_pHotKey = new HotKey(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pHotKey->SetClass(_T("property_grid_hot_key"));
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
        DString newText = m_pHotKey->GetHotKeyName();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pHotKey->SetVisible(false);
    }
    return m_pHotKey;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFileProperty::PropertyGridFileProperty(Window* pWindow, 
                                                   const DString& propertyName,
                                                   const DString& propertyValue,
                                                   const DString& description,
                                                   size_t nPropertyData,
                                                   bool bOpenFileDialog,
                                                   const std::vector<FileDialog::FileType>& fileTypes,
                                                   int32_t nFileTypeIndex,
                                                   const DString& defaultExt) :
    PropertyGridTextProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_pBrowseBtn(nullptr),
    m_bOpenFileDialog(bOpenFileDialog),
    m_fileTypes(fileTypes),
    m_nFileTypeIndex(nFileTypeIndex),
    m_defaultExt(defaultExt)
{
}

void PropertyGridFileProperty::EnableEditControl(bool bEnable)
{
    BaseClass::EnableEditControl(bEnable);
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
        m_pBrowseBtn = new Button(GetWindow());
        m_pBrowseBtn->SetClass(_T("property_grid_button"));
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
    FilePath filePath;
    FileDialog fileDlg;
    if (fileDlg.BrowseForFile(GetWindow(), filePath, m_bOpenFileDialog, m_fileTypes, m_nFileTypeIndex, m_defaultExt)) {
        RichEdit* pRichEdit = GetRichEdit();
        if (pRichEdit != nullptr) {
            pRichEdit->SetText(filePath.ToString());
        }
    }
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridDirectoryProperty::PropertyGridDirectoryProperty(Window* pWindow, 
                                                             const DString& propertyName,
                                                             const DString& propertyValue,
                                                             const DString& description,
                                                             size_t nPropertyData) :
    PropertyGridTextProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_pBrowseBtn(nullptr)
{
}

void PropertyGridDirectoryProperty::EnableEditControl(bool bEnable)
{
    BaseClass::EnableEditControl(bEnable);
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
        m_pBrowseBtn = new Button(GetWindow());
        m_pBrowseBtn->SetClass(_T("property_grid_button"));
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
    FilePath folderPath;
    FileDialog fileDlg;
    if (fileDlg.BrowseForFolder(GetWindow(), folderPath)) {
        RichEdit* pRichEdit = GetRichEdit();
        if (pRichEdit != nullptr) {
            pRichEdit->SetText(folderPath.ToString());
        }
    }
}

}//namespace ui

