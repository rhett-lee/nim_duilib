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
    m_fRowGridLineWidth(0),
    m_fColumnGridLineWidth(0)
{
    SetLeftColumnWidth(130, true);
    SetRowGridLineWidth(1.0f, true);
    SetColumnGridLineWidth(1.0f, true);

    //设置属性值默认字体
    m_propertyNormalFontId = _T("system_regular_14");
    m_propertyModifiedFontId = _T("system_bold_14");
}

DString PropertyGrid::GetType() const { return DUI_CTR_PROPERTY_GRID; }

void PropertyGrid::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("property_grid_xml")) {
        if (!strValue.empty()) {
            m_configXml = strValue;
        }
    }
    else if (strName == _T("row_grid_line_width")) {
        SetRowGridLineWidth(StringUtil::StringToFloat(strValue.c_str()), true);
    }
    else if (strName == _T("row_grid_line_color")) {
        SetRowGridLineColor(strValue);
    }
    else if (strName == _T("column_grid_line_width")) {
        SetColumnGridLineWidth(StringUtil::StringToFloat(strValue.c_str()), true);
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
    else if (strName == _T("group_label_class")) {
        SetGroupLabelClass(strValue);
    }
    else if (strName == _T("property_class")) {
        SetPropertyClass(strValue);
    }
    else if (strName == _T("property_name_label_class")) {
        SetPropertyNameLabelClass(strValue);
    }
    else if (strName == _T("property_value_label_class")) {
        SetPropertyValueLabelClass(strValue);
    }
    else if (strName == _T("left_column_width")) {
        SetLeftColumnWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("property_font_normal")) {
        SetPropertyNormalFontId(strValue);
    }
    else if (strName == _T("property_font_modified")) {
        SetPropertyModifiedFontId(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void PropertyGrid::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    float fValue = GetRowGridLineWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetRowGridLineWidth(fValue, false);

    fValue = GetColumnGridLineWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetColumnGridLineWidth(fValue, false);

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
        m_configXml = DString(DUILIB_PUBLIC_RES_DIR) + _T("/property_grid/property_grid.xml");
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
                    description = pGroup->GetDescription();
                    name = pGroup->GetGroupName();
                }
                else {
                    PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(pItem);
                    if (pProperty != nullptr) {
                        description = pProperty->GetDescription();
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
    float fColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度        
    float fRowLineWidth = GetRowGridLineWidth();   //横向边线宽度
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

    if ((fColumnLineWidth > 0.01) && !columnLineColor.IsEmpty()) {
        //绘制纵向网格线
        UiRect viewRect = m_pTreeView->GetRect();
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
                xPosList.push_back(leftRect.right);
                break;
            }
        }

        for (int32_t xPos : xPosList) {
            //横坐标位置放在每个子项控件的右侧部            
            UiPointF pt1(xPos, yTop);
            UiPointF pt2(xPos, viewRect.bottom);
            pRender->DrawLine(pt1, pt2, columnLineColor, fColumnLineWidth);
        }
    }
    if ((fRowLineWidth > 0.01) && !rowLineColor.IsEmpty()) {
        //绘制横向网格线
        UiRect viewRect = m_pTreeView->GetRect();
        const size_t itemCount = m_pTreeView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            PropertyGridProperty* pItem = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetHeight() <= 0)) {
                continue;
            }
            //纵坐标位置放在每个子项控件的底部（Header控件的底部不画线）
            UiRect rcItemRect = pItem->GetRect();
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
                leftRect = rcRect;
            }
            UiRect rightRect = viewRect;
            if (pItem->GetLabelBoxRight() != nullptr) {
                UiRect rcRect = pItem->GetLabelBoxRight()->GetRect();
                rightRect = rcRect;

                //横向网格线的右侧：如果滚动条正在显示，则不覆盖滚动条
                if (m_pTreeView != nullptr) {
                    ScrollBar* pVScrollBar = m_pTreeView->GetVScrollBar();
                    if ((pVScrollBar != nullptr) && !m_pTreeView->IsVScrollBarAtLeft()) {
                        if (pVScrollBar->IsVisible()) {
                            rightRect.right -= pVScrollBar->GetWidth();
                        }
                    }
                }
            }
            UiPointF pt1(leftRect.left, yPos);
            UiPointF pt2(rightRect.right, yPos);
            pRender->DrawLine(pt1, pt2, rowLineColor, fRowLineWidth);
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

void PropertyGrid::SetEnableHeaderCtrl(bool bEnable, const DString& sLeftColumn,
                                       const DString& sRightColumn, bool bTextId)
{
    ASSERT(IsInited());
    m_bHeaderCtrl = bEnable;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetVisible(bEnable);
    }
    if (IsEnableHeaderCtrl()) {
        if (m_pHeaderLeft != nullptr) {
            if (bTextId) {
                m_pHeaderLeft->SetTextId(sLeftColumn);
            }
            else {
                m_pHeaderLeft->SetText(sLeftColumn);
            }
        }
        if (m_pHeaderRight != nullptr) {
            if (bTextId) {
                m_pHeaderRight->SetTextId(sRightColumn);
            }
            else {
                m_pHeaderRight->SetText(sRightColumn);
            }            
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

void PropertyGrid::SetRowGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fLineWidth = Dpi().GetScaleFloat(fLineWidth);
    }
    if (fLineWidth < 0) {
        fLineWidth = 0;
    }
    if (m_fRowGridLineWidth != fLineWidth) {
        m_fRowGridLineWidth = fLineWidth;
        Invalidate();
    }
}

float PropertyGrid::GetRowGridLineWidth() const
{
    return m_fRowGridLineWidth;
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

void PropertyGrid::SetColumnGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fLineWidth = Dpi().GetScaleFloat(fLineWidth);
    }
    if (fLineWidth < 0) {
        fLineWidth = 0;
    }
    if (m_fColumnGridLineWidth != fLineWidth) {
        m_fColumnGridLineWidth = fLineWidth;
        Invalidate();
    }
}

float PropertyGrid::GetColumnGridLineWidth() const
{
    return m_fColumnGridLineWidth;
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

void PropertyGrid::SetGroupLabelClass(const DString& groupLabelClass)
{
    if (m_groupLabelClass != groupLabelClass) {
        m_groupLabelClass = groupLabelClass;
        std::vector<PropertyGridGroup*> groups;
        GetGroups(groups);
        for (PropertyGridGroup* pGroup : groups) {
            if ((pGroup != nullptr) && (pGroup->GetLabelBox() != nullptr)) {
                pGroup->GetLabelBox()->SetClass(groupLabelClass);
            }
        }
    }
}

DString PropertyGrid::GetGroupLabelClass() const
{
    return m_groupLabelClass.c_str();
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

void PropertyGrid::SetPropertyNameLabelClass(const DString& propertyNameLabelClass)
{
    if (m_propertyNameLabelClass != propertyNameLabelClass) {
        m_propertyNameLabelClass = propertyNameLabelClass;

        std::vector<PropertyGridProperty*> properties;
        std::vector<PropertyGridGroup*> groups;
        GetGroups(groups);
        for (PropertyGridGroup* pGroup : groups) {
            if (pGroup != nullptr) {
                pGroup->GetProperties(properties);
                for (PropertyGridProperty* pProperty : properties) {
                    if ((pProperty != nullptr) && (pProperty->GetLabelBoxLeft() != nullptr)) {
                        pProperty->GetLabelBoxLeft()->SetClass(propertyNameLabelClass);
                    }
                }
            }
        }
    }
}

DString PropertyGrid::GetPropertyNameLabelClass() const
{
    return m_propertyNameLabelClass.c_str();
}

void PropertyGrid::SetPropertyValueLabelClass(const DString& propertyValueLabelClass)
{
    if (m_propertyValueLabelClass != propertyValueLabelClass) {
        m_propertyValueLabelClass = propertyValueLabelClass;

        std::vector<PropertyGridProperty*> properties;
        std::vector<PropertyGridGroup*> groups;
        GetGroups(groups);
        for (PropertyGridGroup* pGroup : groups) {
            if (pGroup != nullptr) {
                pGroup->GetProperties(properties);
                for (PropertyGridProperty* pProperty : properties) {
                    if ((pProperty != nullptr) && (pProperty->GetLabelBoxRight() != nullptr)) {
                        pProperty->GetLabelBoxRight()->SetClass(propertyValueLabelClass);
                    }
                }
            }
        }
    }
}

DString PropertyGrid::GetPropertyValueLabelClass() const
{
    return m_propertyValueLabelClass.c_str();
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
    pGroup->SetPropertyGrid(this);
    m_pTreeView->GetRootNode()->AddChildNode(pGroup);
    pGroup->SetExpand(true);
    return pGroup;
}

PropertyGridGroup* PropertyGrid::AddGroupById(const DString& groupNameId,
                                              const DString& descriptionId,
                                              size_t nGroupData)
{
    ASSERT(m_pTreeView != nullptr);
    if (m_pTreeView == nullptr) {
        return nullptr;
    }
    PropertyGridGroup* pGroup = new PropertyGridGroup(GetWindow(), true, groupNameId, true, descriptionId, nGroupData);
    pGroup->SetWindow(GetWindow());
    pGroup->SetClass(GetGroupClass());
    pGroup->SetPropertyGrid(this);
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
            ASSERT(pGroup->GetPropertyGrid() == this);
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
    pProperty->SetPropertyGrid(this);
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

PropertyGridTextProperty* PropertyGrid::AddTextPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId,
                                                            size_t nPropertyData,
                                                            bool bPropertyValueId)
{
    PropertyGridTextProperty* pProperty = new PropertyGridTextProperty(GetWindow(), true, propertyNameId,
                                                                       bPropertyValueId, propertyValue,
                                                                       true, descriptionId, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridTextProperty* PropertyGrid::AddTextProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridTextProperty* pProperty = new PropertyGridTextProperty(GetWindow(), param);
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

PropertyGridComboProperty* PropertyGrid::AddComboPropertyById(PropertyGridGroup* pGroup,
                                                              const DString& propertyNameId,
                                                              const DString& propertyValue,
                                                              const DString& descriptionId,
                                                              size_t nPropertyData,
                                                              bool bPropertyValueId)
{
    PropertyGridComboProperty* pProperty = new PropertyGridComboProperty(GetWindow(),
                                                                         true, propertyNameId,
                                                                         bPropertyValueId, propertyValue,
                                                                         true, descriptionId, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridComboProperty* PropertyGrid::AddComboProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridComboProperty* pProperty = new PropertyGridComboProperty(GetWindow(), param);
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

PropertyGridFontProperty* PropertyGrid::AddFontPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId,
                                                            size_t nPropertyData,
                                                            bool bPropertyValueId)
{
    PropertyGridFontProperty* pProperty = new PropertyGridFontProperty(GetWindow(),
                                                                       true, propertyNameId,
                                                                       bPropertyValueId, propertyValue,
                                                                       true, descriptionId, nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontProperty* PropertyGrid::AddFontProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridFontProperty* pProperty = new PropertyGridFontProperty(GetWindow(), param);
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

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizePropertyById(PropertyGridGroup* pGroup,
                                                                    const DString& propertyNameId,
                                                                    const DString& propertyValue,
                                                                    const DString& descriptionId,
                                                                    size_t nPropertyData,
                                                                    bool bPropertyValueId)
{
    PropertyGridFontSizeProperty* pProperty = new PropertyGridFontSizeProperty(GetWindow(),
                                                                               true, propertyNameId,
                                                                               bPropertyValueId, propertyValue,
                                                                               true, descriptionId,
                                                                               nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridFontSizeProperty* pProperty = new PropertyGridFontSizeProperty(GetWindow(), param);
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

PropertyGridColorProperty* PropertyGrid::AddColorPropertyById(PropertyGridGroup* pGroup,
                                                             const DString& propertyNameId,
                                                             const DString& propertyValue,
                                                             const DString& descriptionId,
                                                             size_t nPropertyData,
                                                             bool bPropertyValueId)
{
    PropertyGridColorProperty* pProperty = new PropertyGridColorProperty(GetWindow(),
                                                                         true, propertyNameId,
                                                                         bPropertyValueId, propertyValue,
                                                                         true, descriptionId,
                                                                         nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridColorProperty* PropertyGrid::AddColorProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridColorProperty* pProperty = new PropertyGridColorProperty(GetWindow(), param);
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

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimePropertyById(PropertyGridGroup* pGroup,
                                                                    const DString& propertyNameId,
                                                                    const DString& dateTimeValue,
                                                                    const DString& descriptionId,
                                                                    size_t nPropertyData,
                                                                    bool bPropertyValueId,
                                                                    DateTime::EditFormat editFormat)
{
    PropertyGridDateTimeProperty* pProperty = new PropertyGridDateTimeProperty(GetWindow(),
                                                                               true, propertyNameId,
                                                                               bPropertyValueId, dateTimeValue,
                                                                               true, descriptionId,
                                                                               nPropertyData, editFormat);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                                                ui::DateTime::EditFormat editFormat)
{
    PropertyGridDateTimeProperty* pProperty = new PropertyGridDateTimeProperty(GetWindow(), param, editFormat);
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

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressPropertyById(PropertyGridGroup* pGroup,
                                                                      const DString& propertyNameId,
                                                                      const DString& propertyValue,
                                                                      const DString& descriptionId,
                                                                      size_t nPropertyData,
                                                                      bool bPropertyValueId)
{
    PropertyGridIPAddressProperty* pProperty = new PropertyGridIPAddressProperty(GetWindow(),
                                                                                 true, propertyNameId,
                                                                                 bPropertyValueId, propertyValue,
                                                                                 true, descriptionId,
                                                                                 nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridIPAddressProperty* pProperty = new PropertyGridIPAddressProperty(GetWindow(), param);
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

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyPropertyById(PropertyGridGroup* pGroup,
                                                                const DString& propertyNameId,
                                                                const DString& propertyValue,
                                                                const DString& descriptionId,
                                                                size_t nPropertyData,
                                                                bool bPropertyValueId)
{
    PropertyGridHotKeyProperty* pProperty = new PropertyGridHotKeyProperty(GetWindow(),
                                                                           true, propertyNameId,
                                                                           bPropertyValueId, propertyValue,
                                                                           true, descriptionId,
                                                                           nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridHotKeyProperty* pProperty = new PropertyGridHotKeyProperty(GetWindow(), param);
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

PropertyGridFileProperty* PropertyGrid::AddFilePropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId,
                                                            size_t nPropertyData,
                                                            bool bPropertyValueId,
                                                            bool bOpenFileDialog,
                                                            const std::vector<FileDialog::FileType>& fileTypes,
                                                            int32_t nFileTypeIndex,
                                                            const DString& defaultExt)
{
    PropertyGridFileProperty* pProperty = new PropertyGridFileProperty(GetWindow(),
                                                                       true, propertyNameId,
                                                                       bPropertyValueId, propertyValue,
                                                                       true, descriptionId,
                                                                       nPropertyData,
                                                                       bOpenFileDialog, fileTypes,
                                                                       nFileTypeIndex, defaultExt);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridFileProperty* PropertyGrid::AddFileProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                                        bool bOpenFileDialog,
                                                        const std::vector<FileDialog::FileType>& fileTypes,
                                                        int32_t nFileTypeIndex,
                                                        const DString& defaultExt)
{
    PropertyGridFileProperty* pProperty = new PropertyGridFileProperty(GetWindow(), param,
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

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryPropertyById(PropertyGridGroup* pGroup,
                                                                      const DString& propertyNameId,
                                                                      const DString& propertyValue,
                                                                      const DString& descriptionId,
                                                                      size_t nPropertyData,
                                                                      bool bPropertyValueId)
{
    PropertyGridDirectoryProperty* pProperty = new PropertyGridDirectoryProperty(GetWindow(),
                                                                                 true, propertyNameId,
                                                                                 bPropertyValueId, propertyValue,
                                                                                 true, descriptionId,
                                                                                 nPropertyData);
    if (!AddProperty(pGroup, pProperty)) {
        delete pProperty;
        pProperty = nullptr;
    }
    return pProperty;
}

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    PropertyGridDirectoryProperty* pProperty = new PropertyGridDirectoryProperty(GetWindow(), param);
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

void PropertyGrid::SetPropertyNormalFontId(const DString& fontId)
{
    m_propertyNormalFontId = fontId;
}

DString PropertyGrid::GetProptertyNormalFontId() const
{
    return m_propertyNormalFontId.c_str();
}

void PropertyGrid::SetPropertyModifiedFontId(const DString& fontId)
{
    m_propertyModifiedFontId = fontId;
}

DString PropertyGrid::GetProptertyModifiedFontId() const
{
    return m_propertyModifiedFontId.c_str();
}

////////////////////////////////////////////////////////////////////////////
///

PropertyGridGroup::PropertyGridGroup(Window* pWindow):
    TreeNode(pWindow),
    m_pLabelBox(nullptr),
    m_nGroupData(0),
    m_pPropertyGrid(nullptr)
{
}

PropertyGridGroup::PropertyGridGroup(Window* pWindow,
                                     const DString& groupName,
                                     const DString& description,
                                     size_t nGroupData) :
    PropertyGridGroup(pWindow)
{
    m_groupName = groupName;
    m_description = description;
    m_nGroupData = nGroupData;
}

PropertyGridGroup::PropertyGridGroup(Window* pWindow,
                                     bool bGroupNameId,
                                     const DString& groupName,
                                     bool bDescriptionId,
                                     const DString& description,
                                     size_t nGroupData):
    PropertyGridGroup(pWindow)
{
    if (bGroupNameId) {
        m_groupNameId = groupName;
    }
    else {
        m_groupName = groupName;
    }
    if (bDescriptionId) {
        m_descriptionId = description;
    }
    else {
        m_description = description;
    }
    m_nGroupData = nGroupData;
}

DString PropertyGridGroup::GetGroupName() const
{
    if (!m_groupName.empty()) {
        return m_groupName.c_str();
    }
    else if (!m_groupNameId.empty()) {
        return GlobalManager::GetTextById(m_groupNameId.c_str());
    }
    return DString();
}

void PropertyGridGroup::SetGroupName(const DString& groupName)
{
    if (m_groupName != groupName) {
        m_groupName = groupName;
        if (m_pLabelBox != nullptr) {
            m_pLabelBox->SetText(groupName);
        }
    }
}

DString PropertyGridGroup::GetGroupNameId() const
{
    return m_groupNameId.c_str();
}

void PropertyGridGroup::SetGroupNameId(const DString& groupNameId)
{
    if (m_groupNameId != groupNameId) {
        m_groupNameId = groupNameId;
        if (m_pLabelBox != nullptr) {
            m_pLabelBox->SetTextId(groupNameId);
        }
    }
}

DString PropertyGridGroup::GetDescription() const
{
    if (!m_description.empty()) {
        return m_description.c_str();
    }
    else if (!m_descriptionId.empty()) {
        return GlobalManager::GetTextById(m_description.c_str());
    }
    return DString();
}

void PropertyGridGroup::SetDescription(const DString& description)
{
    m_description = description;
}

DString PropertyGridGroup::GetDescriptionId() const
{
    return m_descriptionId.c_str();
}

void PropertyGridGroup::SetDescriptionId(const DString& descriptionId)
{
    m_descriptionId = descriptionId;
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
    pHBox->AddItem(m_pLabelBox.get());
    ASSERT(m_pPropertyGrid != nullptr);
    if (m_pPropertyGrid != nullptr) {
        DString groupLabelClass = m_pPropertyGrid->GetGroupLabelClass();
        if (!groupLabelClass.empty()) {
            m_pLabelBox->SetClass(groupLabelClass);
        }
    }
    m_pLabelBox->SetMouseEnabled(false);
    m_pLabelBox->SetNoFocus();

    //设置组名称
    m_pLabelBox->SetTextId(m_groupNameId.c_str());
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

void PropertyGridGroup::SetPropertyGrid(PropertyGrid* pPropertyGrid)
{
    m_pPropertyGrid = pPropertyGrid;
}

PropertyGrid* PropertyGridGroup::GetPropertyGrid() const
{
    return m_pPropertyGrid.get();
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
        SetShowFocusedRect(true);
        SetTabStop(false);
    }
};

PropertyGridProperty::PropertyGridProperty(Window* pWindow):
    TreeNode(pWindow),
    m_pHBox(nullptr),
    m_pLabelBoxLeft(nullptr),
    m_pLabelBoxRight(nullptr),
    m_nPropertyData(0),
    m_bReadOnly(false),
    m_pPropertyGrid(nullptr)
{
}

PropertyGridProperty::PropertyGridProperty(Window* pWindow, 
                                           const DString& propertyName,
                                           const DString& propertyValue,
                                           const DString& description,
                                           size_t nPropertyData):
    PropertyGridProperty(pWindow)
{
    m_propertyName = propertyName;
    m_propertyValue = propertyValue;
    m_description = description;
    m_nPropertyData = nPropertyData;
}

PropertyGridProperty::PropertyGridProperty(Window* pWindow,
                                           bool bPropertyNameId,
                                           const DString& propertyName,
                                           bool bPropertyValueId,
                                           const DString& propertyValue,
                                           bool bDescriptionId,
                                           const DString& description,
                                           size_t nPropertyData):
    PropertyGridProperty(pWindow)
{
    if (bPropertyNameId) {
        m_propertyNameId = propertyName;
    }
    else {
        m_propertyName = propertyName;
    }
    if (bPropertyValueId) {
        m_propertyValueId = propertyValue;
    }
    else {
        m_propertyValue = propertyValue;
    }
    if (bDescriptionId) {
        m_descriptionId = description;
    }
    else {
        m_description = description;
    }
    m_nPropertyData = nPropertyData;
}

PropertyGridProperty::PropertyGridProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow)
{
    m_propertyName = param.m_propertyName;
    m_propertyNameId = param.m_propertyNameId;
    m_propertyValue = param.m_propertyValue;
    m_propertyValueId = param.m_propertyValueId;
    m_description = param.m_description;
    m_descriptionId = param.m_descriptionId;
    m_nPropertyData = param.m_nPropertyData;
}

DString PropertyGridProperty::GetPropertyName() const
{
    if (!m_propertyName.empty()) {
        return m_propertyName.c_str();
    }
    else if (!m_propertyNameId.empty()) {
        return ui::GlobalManager::GetTextById(m_propertyNameId.c_str());
    }
    return DString();
}

void PropertyGridProperty::SetPropertyName(const DString& propertyName)
{
    if (m_propertyName != propertyName) {
        m_propertyName = propertyName;
        if (m_pLabelBoxLeft != nullptr) {
            m_pLabelBoxLeft->SetText(propertyName);
        }
    }
}

DString PropertyGridProperty::GetPropertyNameId() const
{
    return m_propertyNameId.c_str();
}

void PropertyGridProperty::SetPropertyNameId(const DString& propertyNameId)
{
    if (m_propertyNameId != propertyNameId) {
        m_propertyNameId = propertyNameId;
        if (m_pLabelBoxLeft != nullptr) {
            m_pLabelBoxLeft->SetTextId(propertyNameId);
        }
    }
}

DString PropertyGridProperty::GetPropertyValue() const
{
    if (!m_propertyValue.empty()) {
        return m_propertyValue.c_str();
    }
    else if (!m_propertyValueId.empty()) {
        return ui::GlobalManager::GetTextById(m_propertyValueId.c_str());
    }
    return DString();
}

void PropertyGridProperty::SetPropertyValue(const DString& propertyValue)
{
    if (m_propertyValue != propertyValue) {
        m_propertyValue = propertyValue;
        if (m_pLabelBoxRight != nullptr) {
            m_pLabelBoxRight->SetText(propertyValue);
        }
        OnPropertyValueChanged();
    }
}

DString PropertyGridProperty::GetPropertyValueId() const
{
    return m_propertyValueId.c_str();
}

void PropertyGridProperty::SetPropertyValueId(const DString& propertyValueId)
{
    if (m_propertyValueId != propertyValueId) {
        m_propertyValueId = propertyValueId;
        if (m_pLabelBoxRight != nullptr) {
            m_pLabelBoxRight->SetText(propertyValueId);
        }
        OnPropertyValueChanged();
    }
}

bool PropertyGridProperty::UsingPropertyValueId() const
{
    return m_propertyValue.empty() && !m_propertyValueId.empty();
}

DString PropertyGridProperty::GetDescription() const
{
    if (!m_description.empty()) {
        return m_description.c_str();
    }
    else if (!m_descriptionId.empty()) {
        return ui::GlobalManager::GetTextById(m_descriptionId.c_str());
    }
    return DString();
}

void PropertyGridProperty::SetDescription(const DString& description)
{
    m_description = description;
}

DString PropertyGridProperty::GetDescriptionId() const
{
    return m_descriptionId.c_str();
}

void PropertyGridProperty::SetDescriptionId(const DString& descriptionId)
{
    m_descriptionId = descriptionId;
}

void PropertyGridProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    SetTabStop(false);

    m_pHBox = new HBox(GetWindow());
    AddItem(m_pHBox.get());

    m_pHBox->SetMouseEnabled(false);
    m_pHBox->SetNoFocus();

    m_pLabelBoxLeft = new PropertyGridLabelBox(GetWindow());
    m_pHBox->AddItem(m_pLabelBoxLeft.get());
    ASSERT(m_pPropertyGrid != nullptr);
    if (m_pPropertyGrid != nullptr) {
        DString nameLabelClass = m_pPropertyGrid->GetPropertyNameLabelClass();
        if (!nameLabelClass.empty()) {
            m_pLabelBoxLeft->SetClass(nameLabelClass);
        }
    }
    //属性值
    m_pLabelBoxLeft->SetText(m_propertyName.c_str());
    m_pLabelBoxLeft->SetTextId(m_propertyNameId.c_str());

    m_pLabelBoxRight = new PropertyGridLabelBox(GetWindow());
    m_pHBox->AddItem(m_pLabelBoxRight.get());
    if (m_pPropertyGrid != nullptr) {
        DString valueLabelClass = m_pPropertyGrid->GetPropertyValueLabelClass();
        if (!valueLabelClass.empty()) {
            m_pLabelBoxRight->SetClass(valueLabelClass);
        }
    }
    //属性值
    m_pLabelBoxRight->SetText(m_propertyValue.c_str());
    m_pLabelBoxRight->SetTextId(m_propertyValueId.c_str());

    DString propertyNormalFontId;
    ASSERT(m_pPropertyGrid != nullptr);
    if (m_pPropertyGrid != nullptr) {
        propertyNormalFontId = m_pPropertyGrid->GetProptertyNormalFontId();
    }
    if (!propertyNormalFontId.empty()) {
        m_pLabelBoxRight->SetFontId(propertyNormalFontId.c_str());
    }    

    //挂载鼠标左键按下事件
    m_pLabelBoxRight->AttachButtonDown([this](const EventArgs&) {
        if (!IsReadOnly() && IsEnabled()) {
            Control* pControl = ShowEditControl(true, false);
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
        pTreeView->AttachScrollPosChanged([this](const EventArgs&) {
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

void PropertyGridProperty::SetPropertyText(const DString& text, bool bChanged, bool bTriggerEvent)
{
    DString oldText;
    ASSERT(m_pLabelBoxRight != nullptr);
    if (m_pLabelBoxRight != nullptr) {
        oldText = m_pLabelBoxRight->GetText();
        m_pLabelBoxRight->SetText(text);
        m_pLabelBoxRight->SetTextId(_T("")); //需要清空文本ID，因为文本值为空时，会取文本ID的值
        if (bChanged) {
            DString propertyModifiedFontId;
            ASSERT(m_pPropertyGrid != nullptr);
            if (m_pPropertyGrid != nullptr) {
                propertyModifiedFontId = m_pPropertyGrid->GetProptertyModifiedFontId();
            }
            if (!propertyModifiedFontId.empty()) {
                m_pLabelBoxRight->SetFontId(propertyModifiedFontId);
            }
        }
        else {
            DString propertyNormalFontId;
            ASSERT(m_pPropertyGrid != nullptr);
            if (m_pPropertyGrid != nullptr) {
                propertyNormalFontId = m_pPropertyGrid->GetProptertyNormalFontId();
            }
            if (!propertyNormalFontId.empty()) {
                m_pLabelBoxRight->SetFontId(propertyNormalFontId);
            }            
        }
    }

    if (bTriggerEvent && bChanged && (text != oldText)) {
        //回调函数，供子类使用
        if (OnPropertyTextChanged(oldText, text)) {
            //触发事件，供应用层使用
            SendEvent(kEventValueChanged, (WPARAM)&oldText, (LPARAM)&text);
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

void PropertyGridProperty::SetPropertyGrid(PropertyGrid* pPropertyGrid)
{
    m_pPropertyGrid = pPropertyGrid;
}

PropertyGrid* PropertyGridProperty::GetPropertyGrid() const
{
    return m_pPropertyGrid.get();
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
        this->SetShowFocusedRect(false);
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
    m_bPasswordMode(false),
    m_bTextEdited(false)
{
}

PropertyGridTextProperty::PropertyGridTextProperty(Window* pWindow,
                                                   bool bPropertyNameId,
                                                   const DString& propertyName,
                                                   bool bPropertyValueId,
                                                   const DString& propertyValue,
                                                   bool bDescriptionId,
                                                   const DString& description,
                                                   size_t nPropertyData):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bPropertyValueId, propertyValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pRichEdit(nullptr),
    m_bPasswordMode(false),
    m_bTextEdited(false)
{
}

PropertyGridTextProperty::PropertyGridTextProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pRichEdit(nullptr),
    m_bPasswordMode(false),
    m_bTextEdited(false)
{
}

void PropertyGridTextProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        if (m_pRichEdit != nullptr) {
            RemovePropertySubItem(m_pRichEdit.get());
            m_pRichEdit = nullptr;
        }
        return;
    }
    if (m_pRichEdit != nullptr) {
        return;
    }
    m_bTextEdited = false;
    m_pRichEdit = new PropertyGridRichEdit(GetWindow());
    m_pRichEdit->SetClass(_T("property_grid_property_edit"));
    if (!AddPropertySubItem(m_pRichEdit.get())) {
        delete m_pRichEdit.get();
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
        ShowEditControl(false, false);
        return true;
        });
}

void PropertyGridTextProperty::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (m_pRichEdit == nullptr) {
        return;
    }
    if (!UsingPropertyValueId()) {
        //未使用多语言版本的属性值ID，无需处理
        return;
    }

    //使用旧值变化事件处理该事件
    OnPropertyValueChanged();
}

void PropertyGridTextProperty::OnPropertyValueChanged()
{
    BaseClass::OnPropertyValueChanged();

    if (!m_bTextEdited && (m_pRichEdit != nullptr)) {
        //未编辑过，同步显示和编辑数据
        DString propValue = GetPropertyValue();
        m_pRichEdit->SetText(propValue);
        SetPropertyText(propValue, false);
    }
}

Control* PropertyGridTextProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (IsReadOnly() || (m_pRichEdit == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_bTextEdited = true;
        m_oldText = m_pRichEdit->GetText();
        m_pRichEdit->SetVisible(true);
        m_pRichEdit->SetFocus();
    }
    else {
        if (bCancel) {
            //取消编辑
            m_pRichEdit->SetText(m_oldText.c_str());
        }
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
        if (!bChanged) {
            m_bTextEdited = false;
        }
    }
    return m_pRichEdit.get();
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
        m_bTextEdited = true;
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
    m_pCombo(nullptr),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_oldSelItem(Box::InvalidIndex)
{
}

PropertyGridComboProperty::PropertyGridComboProperty(Window* pWindow,
                                                     bool bPropertyNameId,
                                                     const DString& propertyName,
                                                     bool bPropertyValueId,
                                                     const DString& propertyValue,
                                                     bool bDescriptionId,
                                                     const DString& description,
                                                     size_t nPropertyData):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bPropertyValueId, propertyValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pCombo(nullptr),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_oldSelItem(Box::InvalidIndex)
{
}

PropertyGridComboProperty::PropertyGridComboProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pCombo(nullptr),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_oldSelItem(Box::InvalidIndex)
{
}

void PropertyGridComboProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        if (m_pCombo != nullptr) {
            RemovePropertySubItem(m_pCombo.get());
            m_pCombo = nullptr;
        }
        return;
    }
    if (m_pCombo != nullptr) {
        return;
    }
    m_bComboEdited = false;
    m_pCombo = new PropertyGridCombo(GetWindow());
    m_pCombo->SetClass(_T("property_grid_combo"));
    if (!AddPropertySubItem(m_pCombo.get())) {
        delete m_pCombo.get();
        m_pCombo = nullptr;
        return;
    }
    //编辑框的属性：在property_grid.xml中定义    
    m_pCombo->SetText(GetPropertyText());
    m_pCombo->SetVisible(false);

    //挂载事件
    m_pCombo->AttachKillFocus([this](const EventArgs&) {
        //失去焦点后，隐藏组合框
        ShowEditControl(false, false);
        return true;
        });
    m_pCombo->AttachSelect([this](const EventArgs&) {
        //选中列表项后，隐藏组合框
        ShowEditControl(false, false);
        return true;
        });
    RichEdit* pRichEdit = m_pCombo->GetEditControl();
    if (pRichEdit != nullptr) {
        pRichEdit->AttachReturn([this](const EventArgs&) {
            //按回车键后，隐藏组合框
            ShowEditControl(false, false);
            return true;
            });
        pRichEdit->AttachEsc([this](const EventArgs&) {
            //按ESC键后，隐藏组合框，同时取消编辑
            ShowEditControl(false, true);
            return true;
            });
    }
}

Control* PropertyGridComboProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (IsReadOnly() || (m_pCombo == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_bComboEditing = true;
        m_bComboEdited = true;
        m_pCombo->SetVisible(true);
        m_pCombo->SetFocus();
        m_oldSelItem = m_pCombo->GetCurSel();
        m_oldText = m_pCombo->GetText();
    }
    else {
        //取消
        if (bCancel) {
            m_pCombo->SetCurSel(m_oldSelItem, false);
            if (m_pCombo->GetCurSel() != m_oldSelItem) {
                m_pCombo->SetText(m_oldText.c_str());
            }            
            m_oldText.clear();
            m_oldSelItem = Box::InvalidIndex;
        }

        DString newText = m_pCombo->GetText();
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        m_pCombo->SetVisible(false);
        if (!bChanged) {
            m_bComboEdited = false;
        }
        m_bComboEditing = false; //需要放在最后
    }
    return m_pCombo.get();
}

void PropertyGridComboProperty::OnScrollPosChanged()
{
    if ((m_pCombo != nullptr) && m_pCombo->IsVisible()) {
        m_pCombo->UpdateComboWndPos();
    }
}

void PropertyGridComboProperty::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (m_pCombo == nullptr) {
        return;
    }
    if (!UsingPropertyValueId()) {
        //未使用多语言版本的属性值ID，无需处理
        return;
    }

    //使用旧值变化事件处理该事件
    OnPropertyValueChanged();
}

void PropertyGridComboProperty::OnPropertyValueChanged()
{
    BaseClass::OnPropertyValueChanged();
    if (m_pCombo != nullptr) {
        bool bUpdateText = !m_bComboEdited; //未编辑过或者有选择项
        if (m_pCombo->GetCurSel() != ui::Box::InvalidIndex) {
            bUpdateText = true;
        }
        if (bUpdateText) {
            //同步显示和编辑数据
            DString newText = m_pCombo->GetText();
            bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
            SetPropertyText(newText, bChanged);
        }
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

size_t PropertyGridComboProperty::AddOptionById(const DString& optionTextId)
{
    size_t nIndex = Box::InvalidIndex;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        nIndex = m_pCombo->AddTextIdItem(optionTextId);
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

bool PropertyGridComboProperty::HasOptionData(size_t nIndex) const
{
    if (m_pCombo != nullptr) {
        return m_pCombo->HasItemData(nIndex);
    }
    return false;
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

bool PropertyGridComboProperty::SetCurSel(size_t nIndex, bool bTriggerEvent)
{
    bool bRet = false;
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        bRet = m_pCombo->SetCurSel(nIndex, bTriggerEvent);
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

void PropertyGridComboProperty::UpdateEditText(bool bTriggerEvent)
{
    if (m_pCombo != nullptr) {
        DString newText = m_pCombo->GetItemText(GetCurSel());
        m_pCombo->SetText(newText);
        bool bChanged = newText != GetPropertyValue(); //相对原值，是否有修改
        SetPropertyText(newText, bChanged, bTriggerEvent);
    }
}

bool PropertyGridComboProperty::IsComboEditing() const
{
    return m_bComboEditing;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFontProperty::PropertyGridFontProperty(Window* pWindow, 
                                                   const DString& propertyName,
                                                   const DString& propertyValue,
                                                   const DString& description,
                                                   size_t nPropertyData) :
    PropertyGridComboProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_bCheckingNewValue(false),
    m_bFontNameValidation(true)
{
}

PropertyGridFontProperty::PropertyGridFontProperty(Window* pWindow,
                                                   bool bPropertyNameId,
                                                   const DString& propertyName,
                                                   bool bPropertyValueId,
                                                   const DString& propertyValue,
                                                   bool bDescriptionId,
                                                   const DString& description,
                                                   size_t nPropertyData):
    PropertyGridComboProperty(pWindow, bPropertyNameId, propertyName,
                              bPropertyValueId, propertyValue,
                              bDescriptionId, description,
                              nPropertyData),
    m_bCheckingNewValue(false),
    m_bFontNameValidation(true)
{
}

PropertyGridFontProperty::PropertyGridFontProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridComboProperty(pWindow, param),
    m_bCheckingNewValue(false),
    m_bFontNameValidation(true)
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
    if (m_fontNameList.empty()) {
        size_t nSelItemIndex = Box::InvalidIndex;
        DString defaultFontName = GetPropertyValue();
        GlobalManager::Instance().Font().GetFontNameList(m_fontNameList);
        const size_t nCount = m_fontNameList.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            const DString& fontName = m_fontNameList[nIndex];
            size_t nItem = AddOption(fontName);
            SetOptionData(nItem, nIndex);
            if (fontName == defaultFontName) {
                nSelItemIndex = nIndex;
            }
        }
        if (nSelItemIndex != Box::InvalidIndex) {
            SetCurSel(nSelItemIndex, false);
            UpdateEditText(false); //强制同步选项与显示保持一致
        }
    }
}

void PropertyGridFontProperty::SetFontNameValidation(bool bValidation)
{
    m_bFontNameValidation = bValidation;
}

bool PropertyGridFontProperty::OnPropertyTextChanged(const DString& /*oldText*/, const DString& newText)
{
    //属性值变化时，校验其值是否有效
    bool bRet = true;
    if (m_bCheckingNewValue) {
        //函数避免重入
        return bRet;
    }
    struct AutoCheckFlag
    {
        explicit AutoCheckFlag(bool* bFlag) : m_bFlag(bFlag) {
            *m_bFlag = true;
        }
        ~AutoCheckFlag() {
            *m_bFlag = false;
        }
        bool* m_bFlag;
    };
    AutoCheckFlag checkFlag(&m_bCheckingNewValue);

    //校验新的值，是否合法
    size_t nNewItemIndex = Box::InvalidIndex;
    const size_t nCount = m_fontNameList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontNameList[nIndex] == newText) {
            nNewItemIndex = nIndex;
            break;
        }
    }
    if (nNewItemIndex != Box::InvalidIndex) {
        //合法值，选择对应的列表项，然后退出
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (GetOptionData(nIndex) == nNewItemIndex) {
                SetCurSel(nIndex, false);
                break;
            }
        }
        return bRet;
    }

    if (!m_bFontNameValidation) {
        //禁止校验，退出
        return bRet;
    }
    /////////////////////////////

    //旧的选项内容
    DString oldFontName;
    const size_t nOldSelItem = GetCurSel();
    if (Box::IsValidItemIndex(nOldSelItem) && HasOptionData(nOldSelItem)) {
        size_t nOldSelIndex = GetOptionData(nOldSelItem);
        if (nOldSelIndex < m_fontNameList.size()) {
            oldFontName = m_fontNameList[nOldSelIndex];
        }
    }
    if (oldFontName.empty()) {
        oldFontName = GetPropertyValue();
    }
    size_t nSelectedIndex = Box::InvalidIndex;
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fontNameList[nIndex] == oldFontName) {
            //名字相同，选择该项
            nSelectedIndex = nIndex;
            break;
        }
    }
    bool bChecked = false;
    if (nSelectedIndex != Box::InvalidIndex) {
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (GetOptionData(nIndex) == nSelectedIndex) {
                SetCurSel(nIndex, false);
                UpdateEditText(false); //强制同步选项与显示保持一致
                bChecked = true;
                break;
            }
        }
    }
    if (bChecked) {
        bRet = false;//拦截事件
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////
///
PropertyGridFontSizeProperty::PropertyGridFontSizeProperty(Window* pWindow, 
                                                           const DString& propertyName,
                                                           const DString& propertyValue,
                                                           const DString& description,
                                                           size_t nPropertyData) :
    PropertyGridComboProperty(pWindow, propertyName, propertyValue, description, nPropertyData),
    m_bCheckingNewValue(false),
    m_bFontSizeValidation(true)
{
}

PropertyGridFontSizeProperty::PropertyGridFontSizeProperty(Window* pWindow,
                                                           bool bPropertyNameId,
                                                           const DString& propertyName,
                                                           bool bPropertyValueId,
                                                           const DString& propertyValue,
                                                           bool bDescriptionId,
                                                           const DString& description,
                                                           size_t nPropertyData):
    PropertyGridComboProperty(pWindow, bPropertyNameId, propertyName,
                              bPropertyValueId, propertyValue,
                              bDescriptionId, description,
                              nPropertyData),
    m_bCheckingNewValue(false),
    m_bFontSizeValidation(true)
{
}

PropertyGridFontSizeProperty::PropertyGridFontSizeProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridComboProperty(pWindow, param),
    m_bCheckingNewValue(false),
    m_bFontSizeValidation(true)
{
}

DString PropertyGridFontSizeProperty::GetPropertyNewValue() const
{
    return BaseClass::GetPropertyNewValue();
}

void PropertyGridFontSizeProperty::SetFontSizeList(const std::vector<FontSizeInfo>& fontSizeList)
{
    m_externfontSizeList = fontSizeList;
}

void PropertyGridFontSizeProperty::GetFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const
{
    if (!m_externfontSizeList.empty()) {
        fontSizeList = m_externfontSizeList;
    }
    else {
        GlobalManager::Instance().Font().GetFontSizeList(fontSizeList);
    }
}

void PropertyGridFontSizeProperty::GetDpiFontSizeList(std::vector<FontSizeInfo>& dpiFontSizeList) const
{
    GetFontSizeList(dpiFontSizeList);
    GlobalManager::Instance().Font().DpiScaleFontSizeList(dpiFontSizeList, Dpi());
}

void PropertyGridFontSizeProperty::UpdateFontSizeOptionList()
{
    //记录旧的选项内容
    FontSizeInfo oldSelSizeInfo;
    const size_t nOldSelItem = GetCurSel();
    if (Box::IsValidItemIndex(nOldSelItem) && HasOptionData(nOldSelItem)) {
        size_t nOldSelIndex = GetOptionData(nOldSelItem);
        if (nOldSelIndex < m_fillFontSizeList.size()) {
            oldSelSizeInfo = m_fillFontSizeList[nOldSelIndex];
        }
    }
    DString propValue = GetPropertyNewValue();
    if (!oldSelSizeInfo.fontSizeName.empty() && (propValue == oldSelSizeInfo.fontSizeName)) {
        propValue.clear();
    }

    //删除所有旧的选项内容
    RemoveAllOptions();

    //默认选择项
    size_t nSelItem = Box::InvalidIndex;

    std::vector<FontSizeInfo> dpiFontSizeList;
    GetDpiFontSizeList(dpiFontSizeList);
    const size_t nCount = dpiFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        size_t nItem = AddOption(dpiFontSizeList[nIndex].fontSizeName);
        SetOptionData(nItem, nIndex);
        if (!propValue.empty() && (dpiFontSizeList[nIndex].fontSizeName == propValue)) {
            nSelItem = nItem;
        }
    }
    m_fillFontSizeList = dpiFontSizeList;

    //设置列表选择
    if (nSelItem != Box::InvalidIndex) {
        SetCurSel(nSelItem, false);
        UpdateEditText(false);
    }
    else if (!oldSelSizeInfo.fontSizeName.empty()) {
        size_t nSelectedIndex = Box::InvalidIndex;
        for (int32_t nIndex = (int32_t)nCount - 1; nIndex >= 0; --nIndex) {
            //按倒序匹配
            if (std::abs(dpiFontSizeList[nIndex].fFontSize - oldSelSizeInfo.fFontSize) < 0.001f) {
                //字号相同，选择该项
                nSelectedIndex = nIndex;
                break;
            }
        }

        if (nSelectedIndex != Box::InvalidIndex) {
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                if (GetOptionData(nIndex) == nSelectedIndex) {
                    SetCurSel(nIndex, false);
                    UpdateEditText(false);
                }
            }
        }
    }
}

void PropertyGridFontSizeProperty::SetFontSizeValidation(bool bValidation)
{
    m_bFontSizeValidation = bValidation;
}

void PropertyGridFontSizeProperty::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //初始化字体列表
    UpdateFontSizeOptionList();
}

void PropertyGridFontSizeProperty::OnLanguageChanged(bool /*bRedraw*/)
{
    //重新填充字体列表
    UpdateFontSizeOptionList();
}

void PropertyGridFontSizeProperty::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    if (!m_fillFontSizeList.empty()) {
        //更新字体大小的DPI缩放值
        GlobalManager::Instance().Font().DpiScaleFontSizeList(m_fillFontSizeList, Dpi());
    }
}

bool PropertyGridFontSizeProperty::OnPropertyTextChanged(const DString& /*oldText*/, const DString& newText)
{
    //属性值变化时，校验其值是否有效
    bool bRet = true;
    if (m_fillFontSizeList.empty()) {
        return bRet;
    }
    if (m_bCheckingNewValue) {
        //函数避免重入
        return bRet;
    }
    struct AutoCheckFlag
    {
        explicit AutoCheckFlag(bool* bFlag) : m_bFlag(bFlag) {
            *m_bFlag = true;
        }
        ~AutoCheckFlag() {
            *m_bFlag = false;
        }
        bool* m_bFlag;
    };
    AutoCheckFlag checkFlag(&m_bCheckingNewValue);

    //校验新的值，是否合法
    size_t nNewItemIndex = Box::InvalidIndex;
    const size_t nCount = m_fillFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fillFontSizeList[nIndex].fontSizeName == newText) {
            nNewItemIndex = nIndex;
            break;
        }
    }
    if (nNewItemIndex != Box::InvalidIndex) {
        //合法值，选择对应的列表项，然后退出
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (GetOptionData(nIndex) == nNewItemIndex) {
                SetCurSel(nIndex, false);
                break;
            }
        }
        return bRet;
    }

    if (!m_bFontSizeValidation) {
        //禁止校验，退出
        return bRet;
    }

    //旧的选项内容
    FontSizeInfo oldSelSizeInfo;
    const size_t nOldSelItem = GetCurSel();
    if (Box::IsValidItemIndex(nOldSelItem) && HasOptionData(nOldSelItem)) {
        size_t nOldSelIndex = GetOptionData(nOldSelItem);
        if (nOldSelIndex < m_fillFontSizeList.size()) {
            oldSelSizeInfo = m_fillFontSizeList[nOldSelIndex];
        }
    }
    size_t nSelectedIndex = Box::InvalidIndex;
    if (IsComboEditing() && !oldSelSizeInfo.fontSizeName.empty()) {
        //编辑状态下，优先匹配字体名称
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (m_fillFontSizeList[nIndex].fontSizeName == oldSelSizeInfo.fontSizeName) {
                //名字相同，选择该项
                nSelectedIndex = nIndex;
                break;
            }
        }
    }
    if (nSelectedIndex == Box::InvalidIndex) {
        for (int32_t nIndex = (int32_t)nCount - 1; nIndex >= 0; --nIndex) {
            //按倒序匹配
            if (std::abs(m_fillFontSizeList[nIndex].fFontSize - oldSelSizeInfo.fFontSize) < 0.001f) {
                //字号相同，选择该项
                nSelectedIndex = nIndex;
                break;
            }
        }
    }
    if (nSelectedIndex == Box::InvalidIndex) {
        const DString oldPropValue = GetPropertyValue();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (m_fillFontSizeList[nIndex].fontSizeName == oldPropValue) {
                //选择原值
                nSelectedIndex = nIndex;
                break;
            }
        }
    }
    if (nSelectedIndex != Box::InvalidIndex) {
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            if (GetOptionData(nIndex) == nSelectedIndex) {
                SetCurSel(nIndex, false);
                UpdateEditText(false); //强制同步选项与显示保持一致
                bRet = false;//拦截事件
                break;
            }
        }
    }
    return bRet;
}

DString PropertyGridFontSizeProperty::GetFontSize() const
{
    DString fontSize;
    size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        size_t nIndex = GetOptionData(nCurSel);
        if (nIndex < m_fillFontSizeList.size()) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fFontSize);
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
        if (nIndex < m_fillFontSizeList.size()) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fDpiFontSize);
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
    const size_t nCount = m_fillFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fillFontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fFontSize);
            break;
        }
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetFontSizeById(const DString& fontSizeNameId) const
{
    DString fontSizeName = ui::GlobalManager::GetTextById(fontSizeNameId);
    DString fontSize;
    const size_t nCount = m_fillFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fillFontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fFontSize);
            break;
        }
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetDpiFontSize(const DString& fontSizeName) const
{
    DString fontSize;
    const size_t nCount = m_fillFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fillFontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fDpiFontSize);
            break;
        }
    }
    return fontSize;
}

DString PropertyGridFontSizeProperty::GetDpiFontSizeById(const DString& fontSizeNameId) const
{
    DString fontSizeName = ui::GlobalManager::GetTextById(fontSizeNameId);
    DString fontSize;
    const size_t nCount = m_fillFontSizeList.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (m_fillFontSizeList[nIndex].fontSizeName == fontSizeName) {
            fontSize = StringUtil::Printf(_T("%.01f"), m_fillFontSizeList[nIndex].fDpiFontSize);
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

PropertyGridColorProperty::PropertyGridColorProperty(Window* pWindow,
                                                     bool bPropertyNameId,
                                                     const DString& propertyName,
                                                     bool bPropertyValueId,
                                                     const DString& propertyValue,
                                                     bool bDescriptionId,
                                                     const DString& description,
                                                     size_t nPropertyData):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bPropertyValueId, propertyValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pComboButton(nullptr)
{
}

PropertyGridColorProperty::PropertyGridColorProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pComboButton(nullptr)
{
}

void PropertyGridColorProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pComboButton.get());
        m_pComboButton = nullptr;
        return;
    }
    if (m_pComboButton != nullptr) {
        return;
    }
    m_pComboButton = new ComboButton(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pComboButton->SetClass(_T("property_grid_combo_button"));
    if (!AddPropertySubItem(m_pComboButton.get())) {
        delete m_pComboButton.get();
        m_pComboButton = nullptr;
        return;
    }

    Label* pLabelText = m_pComboButton->GetLabelTop();
    if (pLabelText != nullptr) {
        pLabelText->SetText(GetPropertyText());
    }
    Label* pLabelColor = m_pComboButton->GetLabelBottom();
    if (pLabelColor != nullptr) {
        pLabelColor->SetBkColor(GetPropertyText());//文本值就是颜色字符串
    }

    //更新字体颜色
    SetPropertyTextColor(GetPropertyText());//文本值就是颜色字符串

    m_pComboButton->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pComboButton->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false, false);
        return true;
        });
    m_pComboButton->AttachEvent(ui::kEventKeyDown, [this](const EventArgs& args) {
        if (args.vkCode == ui::kVK_ESCAPE) {
            //取消
            ShowEditControl(false, true);
        }        
        return true;
        }, 0);

    InitColorCombo();
}

Control* PropertyGridColorProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (IsReadOnly() || (m_pComboButton == nullptr)) {
        return nullptr;
    }

    if (bShow) {
        m_oldColor.clear();
        Label* pColorLabel = m_pComboButton->GetLabelTop();
        if (pColorLabel != nullptr) {
            m_oldColor = pColorLabel->GetText();
        }
        m_pComboButton->SetVisible(true);
        m_pComboButton->SetFocus();
    }
    else {
        if (bCancel) {
            //取消
            OnSelectColor(m_oldColor.c_str());
        }
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
    return m_pComboButton.get();
}

void PropertyGridColorProperty::OnScrollPosChanged()
{
    if ((m_pComboButton != nullptr) && m_pComboButton->IsVisible()) {
        m_pComboButton->UpdateComboWndPos();
    }
}

void PropertyGridColorProperty::InitColorCombo()
{
    ComboButton* pColorComboBtn = m_pComboButton.get();
    if (pColorComboBtn == nullptr) {
        return;
    }
    UiSize boxSize = pColorComboBtn->GetDropBoxSize();
    Box* pComboBox = pColorComboBtn->GetComboBox();
    if (pComboBox == nullptr) {
        return;
    }
    pComboBox->SetWindow(GetWindow());
    GlobalManager::Instance().FillBoxWithCache(pComboBox, FilePath(DString(DUILIB_PUBLIC_RES_DIR) + _T("/property_grid/color_combox.xml")));
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
    ComboButton* pColorComboBtn = m_pComboButton.get();
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

PropertyGridDateTimeProperty::PropertyGridDateTimeProperty(Window* pWindow,
                                                           bool bPropertyNameId,
                                                           const DString& propertyName,
                                                           bool bDateTimeValueId,
                                                           const DString& dateTimeValue,
                                                           bool bDescriptionId,
                                                           const DString& description,
                                                           size_t nPropertyData,
                                                           DateTime::EditFormat editFormat):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bDateTimeValueId, dateTimeValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pDateTime(nullptr)
{
    m_editFormat = editFormat;
}

PropertyGridDateTimeProperty::PropertyGridDateTimeProperty(Window* pWindow, const PropertyGridParam& param,
                                                           DateTime::EditFormat editFormat):
    PropertyGridProperty(pWindow, param),
    m_pDateTime(nullptr)
{
    m_editFormat = editFormat;
}

void PropertyGridDateTimeProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pDateTime.get());
        m_pDateTime = nullptr;
        return;
    }
    if (m_pDateTime != nullptr) {
        return;
    }
    m_pDateTime = new PropertyGridDateTime(GetWindow());
    //属性：在property_grid.xml中定义
    m_pDateTime->SetClass(_T("property_grid_date_time"));
    if (!AddPropertySubItem(m_pDateTime.get())) {
        delete m_pDateTime.get();
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
        ShowEditControl(false, false);
        return true;
        });
}

Control* PropertyGridDateTimeProperty::ShowEditControl(bool bShow, bool /*bCancel*/)
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
    return m_pDateTime.get();
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

PropertyGridIPAddressProperty::PropertyGridIPAddressProperty(Window* pWindow,
                                                             bool bPropertyNameId,
                                                             const DString& propertyName,
                                                             bool bPropertyValueId,
                                                             const DString& propertyValue,
                                                             bool bDescriptionId,
                                                             const DString& description,
                                                             size_t nPropertyData):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bPropertyValueId, propertyValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pIPAddress(nullptr)
{
}

PropertyGridIPAddressProperty::PropertyGridIPAddressProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pIPAddress(nullptr)
{
}

void PropertyGridIPAddressProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pIPAddress.get());
        m_pIPAddress = nullptr;
        return;
    }
    if (m_pIPAddress != nullptr) {
        return;
    }
    m_pIPAddress = new IPAddress(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pIPAddress->SetClass(_T("property_grid_ip_address"));
    if (!AddPropertySubItem(m_pIPAddress.get())) {
        delete m_pIPAddress.get();
        m_pIPAddress = nullptr;
        return;
    }

    m_pIPAddress->SetIPAddress(GetPropertyText());
    m_pIPAddress->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pIPAddress->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false, false);
        return true;
        });
}

Control* PropertyGridIPAddressProperty::ShowEditControl(bool bShow, bool /*bCancel*/)
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
    return m_pIPAddress.get();
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

PropertyGridHotKeyProperty::PropertyGridHotKeyProperty(Window* pWindow,
                                                       bool bPropertyNameId,
                                                       const DString& propertyName,
                                                       bool bPropertyValueId,
                                                       const DString& propertyValue,
                                                       bool bDescriptionId,
                                                       const DString& description,
                                                       size_t nPropertyData):
    PropertyGridProperty(pWindow, bPropertyNameId, propertyName,
                         bPropertyValueId, propertyValue,
                         bDescriptionId, description,
                         nPropertyData),
    m_pHotKey(nullptr)
{
}

PropertyGridHotKeyProperty::PropertyGridHotKeyProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pHotKey(nullptr)
{
}

void PropertyGridHotKeyProperty::EnableEditControl(bool bEnable)
{
    ASSERT(IsInited());
    if (!bEnable) {
        RemovePropertySubItem(m_pHotKey.get());
        m_pHotKey = nullptr;
        return;
    }
    if (m_pHotKey != nullptr) {
        return;
    }
    m_pHotKey = new HotKey(GetWindow());
    //属性：在property_grid.xml中定义    
    m_pHotKey->SetClass(_T("property_grid_hot_key"));
    if (!AddPropertySubItem(m_pHotKey.get())) {
        delete m_pHotKey.get();
        m_pHotKey = nullptr;
        return;
    }

    m_pHotKey->SetHotKeyName(GetPropertyText());
    m_pHotKey->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pHotKey->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false, false);
        return true;
        });
}

Control* PropertyGridHotKeyProperty::ShowEditControl(bool bShow, bool /*bCancel*/)
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
    return m_pHotKey.get();
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

PropertyGridFileProperty::PropertyGridFileProperty(Window* pWindow,
                                                   bool bPropertyNameId,
                                                   const DString& propertyName,
                                                   bool bPropertyValueId,
                                                   const DString& propertyValue,
                                                   bool bDescriptionId,
                                                   const DString& description,
                                                   size_t nPropertyData,
                                                   bool bOpenFileDialog,
                                                   const std::vector<FileDialog::FileType>& fileTypes,
                                                   int32_t nFileTypeIndex,
                                                   const DString& defaultExt):
    PropertyGridTextProperty(pWindow, bPropertyNameId, propertyName,
                             bPropertyValueId, propertyValue,
                             bDescriptionId, description, nPropertyData),
    m_pBrowseBtn(nullptr),
    m_bOpenFileDialog(bOpenFileDialog),
    m_fileTypes(fileTypes),
    m_nFileTypeIndex(nFileTypeIndex),
    m_defaultExt(defaultExt)
{
}

PropertyGridFileProperty::PropertyGridFileProperty(Window* pWindow, const PropertyGridParam& param,
                                                   bool bOpenFileDialog,
                                                   const std::vector<FileDialog::FileType>& fileTypes,
                                                   int32_t nFileTypeIndex,
                                                   const DString& defaultExt):
    PropertyGridTextProperty(pWindow, param),
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
        size_t nItemIndex = pRichEdit->GetItemIndex(m_pBrowseBtn.get());
        if (nItemIndex == Box::InvalidIndex) {
            m_pBrowseBtn = nullptr;
        }
    }
    if (m_pBrowseBtn == nullptr) {
        m_pBrowseBtn = new Button(GetWindow());
        m_pBrowseBtn->SetClass(_T("property_grid_button"));
        m_pBrowseBtn->SetNoFocus();
        pRichEdit->AddItem(m_pBrowseBtn.get());

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

PropertyGridDirectoryProperty::PropertyGridDirectoryProperty(Window* pWindow,
                                                             bool bPropertyNameId,
                                                             const DString& propertyName,
                                                             bool bPropertyValueId,
                                                             const DString& propertyValue,
                                                             bool bDescriptionId,
                                                             const DString& description,
                                                             size_t nPropertyData):
    PropertyGridTextProperty(pWindow, bPropertyNameId, propertyName,
                             bPropertyValueId, propertyValue,
                             bDescriptionId, description,
                             nPropertyData),
    m_pBrowseBtn(nullptr)
{
}

PropertyGridDirectoryProperty::PropertyGridDirectoryProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridTextProperty(pWindow, param),
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
        size_t nItemIndex = pRichEdit->GetItemIndex(m_pBrowseBtn.get());
        if (nItemIndex == Box::InvalidIndex) {
            m_pBrowseBtn = nullptr;
        }
    }
    if (m_pBrowseBtn == nullptr) {
        m_pBrowseBtn = new Button(GetWindow());
        m_pBrowseBtn->SetClass(_T("property_grid_button"));
        m_pBrowseBtn->SetNoFocus();
        pRichEdit->AddItem(m_pBrowseBtn.get());

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

