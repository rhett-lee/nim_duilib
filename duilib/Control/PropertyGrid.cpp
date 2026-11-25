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
    m_nSelectedItemIndex(Box::InvalidIndex),
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
    else if (strName == _T("property_edit_class")) {
        SetPropertyEditClass(strValue);
    }
    else if (strName == _T("property_combo_class")) {
        SetPropertyComboClass(strValue);
    }
    else if (strName == _T("property_combo_button_class")) {
        SetPropertyComboButtonClass(strValue);
    }
    else if (strName == _T("property_date_time_class")) {
        SetPropertyDateTimeClass(strValue);
    }
    else if (strName == _T("property_ip_address_class")) {
        SetPropertyIPAddressClass(strValue);
    }
    else if (strName == _T("property_hot_key_class")) {
        SetPropertyHotKeyClass(strValue);
    }
    else if (strName == _T("property_button_class")) {
        SetPropertyButtonClass(strValue);
    }
    else if (strName == _T("property_spin_class")) {
        SetPropertySpinClass(strValue);
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
            m_nSelectedItemIndex = args.wParam;
            UpdateDescriptionArea();
            return true;
            });
    }

    //树控件滚动时，统一处理各属性的弹出子窗口位置（避免每个属性分别注册回调）
    if (m_pTreeView != nullptr) {
        m_pTreeView->AttachScrollPosChanged([this](const EventArgs& /*args*/) {
            OnTreeViewScrollPosChanged();
            return true;
            });
    }
}

void PropertyGrid::UpdateDescriptionArea()
{
    if (m_pDescriptionArea == nullptr) {
        return;
    }
    Control* pItem = nullptr;
    if ((m_pTreeView != nullptr) && (m_nSelectedItemIndex != Box::InvalidIndex)) {
        pItem = m_pTreeView->GetItemAt(m_nSelectedItemIndex);
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
    if (!name.empty()) {
        //属性名称需要转义，避免名称中包含的富文本标记被解析
        DString escapedName = name;
        StringUtil::ReplaceAll(_T("&"), _T("&amp;"), escapedName);
        StringUtil::ReplaceAll(_T("<"), _T("&lt;"), escapedName);
        StringUtil::ReplaceAll(_T(">"), _T("&gt;"), escapedName);
        description = _T("<b>") + escapedName + _T("</b><br/>") + description;
    }
    m_pDescriptionArea->SetText(description);
}

void PropertyGrid::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    UpdateDescriptionArea();
    if (bRedraw) {
        Invalidate();
    }
}

void PropertyGrid::OnTreeViewScrollPosChanged()
{
    if (m_pTreeView == nullptr) {
        return;
    }
    const UiRect viewRect = m_pTreeView->GetRect();
    const size_t itemCount = m_pTreeView->GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
        if ((pProperty == nullptr) || !pProperty->IsVisible()) {
            continue;
        }
        //仅处理当前可见的属性，避免属性较多时做无用的遍历
        const UiRect itemRect = pProperty->GetRect();
        if (itemRect.IsEmpty() || (itemRect.bottom <= viewRect.top) || (itemRect.top >= viewRect.bottom)) {
            continue;
        }
        pProperty->OnScrollPosChanged();
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
    const float fColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度
    const float fRowLineWidth = GetRowGridLineWidth();      //横向边线宽度
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
    if (((fColumnLineWidth <= 0.01) || columnLineColor.IsEmpty()) &&
        ((fRowLineWidth <= 0.01) || rowLineColor.IsEmpty())) {
        return;
    }

    const UiRect viewRect = m_pTreeView->GetRect();
    if (viewRect.IsEmpty()) {
        return;
    }
    const size_t itemCount = m_pTreeView->GetItemCount();
    if (itemCount == 0) {
        return;
    }

    //横向网格线位置：使用树控件的布局边距（各行由树控件布局）
    int32_t nChildMarginY = 0;
    Layout* pLayout = m_pTreeView->GetLayout();
    if (pLayout != nullptr) {
        nChildMarginY = pLayout->GetChildMarginY();
    }

    //滚动条所在区域不绘制横向网格线
    int32_t nVScrollBarWidth = 0;
    ScrollBar* pVScrollBar = m_pTreeView->GetVScrollBar();
    if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible() && !m_pTreeView->IsVScrollBarAtLeft()) {
        nVScrollBarWidth = pVScrollBar->GetWidth();
    }

    const bool bDrawRowLine = (fRowLineWidth > 0.01) && !rowLineColor.IsEmpty();
    const UiPoint treeScrollBoxOffset = m_pTreeView->GetScrollOffsetInScrollBox();
    int32_t xColumnLine = -1;
    int32_t nLastRowBottom = viewRect.top;
    for (size_t index = 0; index < itemCount; ++index) {
        PropertyGridProperty* pItem = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        const UiRect itemRect = pItem->GetRect();
        if (itemRect.IsEmpty()) {
            continue;
        }
        //纵向网格线的位置：取第一个可见属性的左侧单元格的右边界（各行的列宽一致，只需要计算一次）
        if ((xColumnLine < 0) && (pItem->GetLabelBoxLeft() != nullptr)) {
            xColumnLine = pItem->GetLabelBoxLeft()->GetRect().right;
        }

        //纵坐标位置放在每个子项控件的底部（Header控件的底部不画线）
        const UiPoint scrollBoxOffset = pItem->GetScrollOffsetInScrollBox();
        UiRect rcItemRect = itemRect;
        rcItemRect.Offset(-(scrollBoxOffset.x - treeScrollBoxOffset.x), -(scrollBoxOffset.y - treeScrollBoxOffset.y));
        if (rcItemRect.bottom > nLastRowBottom) {
            //记录最后一个可见行的底部位置，用于限制纵向网格线的绘制范围
            nLastRowBottom = rcItemRect.bottom;
        }
        if (!bDrawRowLine) {
            continue;
        }
        const int32_t yPos = rcItemRect.bottom + nChildMarginY / 2;
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
            leftRect = pItem->GetLabelBoxLeft()->GetRect();
        }
        UiRect rightRect = viewRect;
        if (pItem->GetLabelBoxRight() != nullptr) {
            rightRect = pItem->GetLabelBoxRight()->GetRect();
            rightRect.right -= nVScrollBarWidth;
        }
        pRender->DrawLine(UiPointF(leftRect.left, yPos), UiPointF(rightRect.right, yPos), rowLineColor, fRowLineWidth);
    }

    //绘制纵向网格线：仅绘制到最后一个可见行的底部，避免所有分组折叠后线条贯穿空白区域
    int32_t nColumnLineBottom = nLastRowBottom + nChildMarginY;
    if (nColumnLineBottom > viewRect.bottom) {
        nColumnLineBottom = viewRect.bottom;
    }
    if ((fColumnLineWidth > 0.01) && !columnLineColor.IsEmpty() &&
        (xColumnLine > viewRect.left) && (nColumnLineBottom > viewRect.top)) {
        pRender->DrawLine(UiPointF(xColumnLine, viewRect.top), UiPointF(xColumnLine, nColumnLineBottom),
                          columnLineColor, fColumnLineWidth);
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
    m_bHeaderCtrl = bEnable;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetVisible(bEnable);
    }
    if (IsEnableHeaderCtrl()) {
        //文本ID未指定时，使用默认的语言资源ID
        DString sLeft = sLeftColumn;
        DString sRight = sRightColumn;
        if (bTextId) {
            if (sLeft.empty()) {
                sLeft = _T("STRID_PUBLIC_PROPERTY_NAME");
            }
            if (sRight.empty()) {
                sRight = _T("STRID_PUBLIC_PROPERTY_VALUE");
            }
        }
        if (m_pHeaderLeft != nullptr) {
            if (bTextId) {
                m_pHeaderLeft->SetTextId(sLeft);
            }
            else {
                m_pHeaderLeft->SetText(sLeft);
            }
        }
        if (m_pHeaderRight != nullptr) {
            if (bTextId) {
                m_pHeaderRight->SetTextId(sRight);
            }
            else {
                m_pHeaderRight->SetText(sRight);
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
    //描述区域隐藏时，分割条也一并隐藏
    if (m_pDescriptionAreaSplit != nullptr) {
        m_pDescriptionAreaSplit->SetVisible(bEnable);
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
        if (nHeight <= 0) {
            //没有设置固定高度时，返回实际显示高度
            nHeight = m_pDescriptionArea->GetRect().Height();
        }
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

void PropertyGrid::SetPropertyEditClass(const DString& editClass)
{
    m_propertyEditClass = editClass;
}

DString PropertyGrid::GetPropertyEditClass() const
{
    return m_propertyEditClass.c_str();
}

void PropertyGrid::SetPropertyComboClass(const DString& comboClass)
{
    m_propertyComboClass = comboClass;
}

DString PropertyGrid::GetPropertyComboClass() const
{
    return m_propertyComboClass.c_str();
}

void PropertyGrid::SetPropertyComboButtonClass(const DString& comboButtonClass)
{
    m_propertyComboButtonClass = comboButtonClass;
}

DString PropertyGrid::GetPropertyComboButtonClass() const
{
    return m_propertyComboButtonClass.c_str();
}

void PropertyGrid::SetPropertyDateTimeClass(const DString& dateTimeClass)
{
    m_propertyDateTimeClass = dateTimeClass;
}

DString PropertyGrid::GetPropertyDateTimeClass() const
{
    return m_propertyDateTimeClass.c_str();
}

void PropertyGrid::SetPropertyIPAddressClass(const DString& ipAddressClass)
{
    m_propertyIPAddressClass = ipAddressClass;
}

DString PropertyGrid::GetPropertyIPAddressClass() const
{
    return m_propertyIPAddressClass.c_str();
}

void PropertyGrid::SetPropertyHotKeyClass(const DString& hotKeyClass)
{
    m_propertyHotKeyClass = hotKeyClass;
}

DString PropertyGrid::GetPropertyHotKeyClass() const
{
    return m_propertyHotKeyClass.c_str();
}

void PropertyGrid::SetPropertyButtonClass(const DString& buttonClass)
{
    m_propertyButtonClass = buttonClass;
}

DString PropertyGrid::GetPropertyButtonClass() const
{
    return m_propertyButtonClass.c_str();
}

void PropertyGrid::SetPropertySpinClass(const DString& spinClass)
{
    m_propertySpinClass = spinClass;
}

DString PropertyGrid::GetPropertySpinClass() const
{
    return m_propertySpinClass.c_str();
}

int32_t PropertyGrid::AutoAdjustLeftColumnWidth(int32_t nMinWidth, int32_t nMaxWidth, int32_t nExtraWidth)
{
    if (m_pTreeView == nullptr) {
        return GetLeftColumnWidth();
    }
    IRender* pRender = nullptr;
    if (GetWindow() != nullptr) {
        pRender = GetWindow()->GetRender();
    }
    if (pRender == nullptr) {
        return GetLeftColumnWidth();
    }

    //取所有属性名称中最宽的文本，加上缩进、内边距与额外留白
    int32_t nMaxTextWidth = 0;
    const int32_t nIndent = m_pTreeView->GetIndent();
    const size_t nCount = m_pTreeView->GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        PropertyGridProperty* pProperty = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(i));
        if ((pProperty == nullptr) || (pProperty->GetLabelBoxLeft() == nullptr)) {
            continue;
        }
        LabelBox* pLabelBox = pProperty->GetLabelBoxLeft();
        MeasureStringParam measureParam = pLabelBox->GetMeasureParam();
        if (measureParam.pFont == nullptr) {
            continue;
        }
        const int32_t nTextWidth = pRender->MeasureString(pLabelBox->GetText(), measureParam).Width();
        const UiPadding rcTextPadding = pLabelBox->GetTextPadding();
        const UiPadding rcControlPadding = pLabelBox->GetControlPadding();
        const UiPadding rcRowPadding = pProperty->GetPadding();
        int32_t nWidth = nTextWidth + rcTextPadding.left + rcTextPadding.right
                         + rcControlPadding.left + rcControlPadding.right
                         + rcRowPadding.left + rcRowPadding.right
                         + nIndent * static_cast<int32_t>(pProperty->GetDepth());
        if (nWidth > nMaxTextWidth) {
            nMaxTextWidth = nWidth;
        }
    }
    if (nMaxTextWidth <= 0) {
        return GetLeftColumnWidth();
    }

    int32_t nWidth = nMaxTextWidth + nExtraWidth;
    if ((nMinWidth > 0) && (nWidth < nMinWidth)) {
        nWidth = nMinWidth;
    }
    if ((nMaxWidth > 0) && (nWidth > nMaxWidth)) {
        nWidth = nMaxWidth;
    }
    SetLeftColumnWidth(nWidth, false);
    return nWidth;
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
    if (!m_pTreeView->GetRootNode()->AddChildNode(pGroup)) {
        delete pGroup;
        return nullptr;
    }
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
    if (!m_pTreeView->GetRootNode()->AddChildNode(pGroup)) {
        delete pGroup;
        return nullptr;
    }
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

namespace
{
/** 构造属性的参数（普通文本形式）
*/
PropertyGridParam MakePropertyParam(const DString& propertyName,
                                    const DString& propertyValue,
                                    const DString& description,
                                    size_t nPropertyData)
{
    PropertyGridParam param;
    param.m_propertyName = propertyName;
    param.m_propertyValue = propertyValue;
    param.m_description = description;
    param.m_nPropertyData = nPropertyData;
    return param;
}

/** 构造属性的参数（文本ID形式）
*/
PropertyGridParam MakePropertyParamById(const DString& propertyNameId,
                                        const DString& propertyValue,
                                        const DString& descriptionId,
                                        size_t nPropertyData,
                                        bool bPropertyValueId)
{
    PropertyGridParam param;
    param.m_propertyNameId = propertyNameId;
    if (bPropertyValueId) {
        param.m_propertyValueId = propertyValue;
    }
    else {
        param.m_propertyValue = propertyValue;
    }
    param.m_descriptionId = descriptionId;
    param.m_nPropertyData = nPropertyData;
    return param;
}
}//namespace

bool PropertyGrid::AddProperty(PropertyGridGroup* pGroup, PropertyGridProperty* pProperty)
{
    ASSERT((pGroup != nullptr) && (pProperty != nullptr));
    if ((pGroup == nullptr) || (pProperty == nullptr)){
        return false;
    }
    ASSERT(pGroup->GetPropertyGrid() == this);
    if (pGroup->GetPropertyGrid() != this) {
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
    if (!pGroup->AddChildNode(pProperty)) {
        return false;
    }
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
    return AddPropertyImpl<PropertyGridTextProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridTextProperty* PropertyGrid::AddTextPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId,
                                                            size_t nPropertyData,
                                                            bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridTextProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridTextProperty* PropertyGrid::AddTextProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridTextProperty>(pGroup, param);
}

PropertyGridComboProperty* PropertyGrid::AddComboProperty(PropertyGridGroup* pGroup,
                                                          const DString& propertyName,
                                                          const DString& propertyValue,
                                                          const DString& description,
                                                          size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridComboProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridComboProperty* PropertyGrid::AddComboPropertyById(PropertyGridGroup* pGroup,
                                                              const DString& propertyNameId,
                                                              const DString& propertyValue,
                                                              const DString& descriptionId,
                                                              size_t nPropertyData,
                                                              bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridComboProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridComboProperty* PropertyGrid::AddComboProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridComboProperty>(pGroup, param);
}

PropertyGridFontProperty* PropertyGrid::AddFontProperty(PropertyGridGroup* pGroup,
                                                        const DString& propertyName,
                                                        const DString& propertyValue,
                                                        const DString& description,
                                                        size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridFontProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridFontProperty* PropertyGrid::AddFontPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId,
                                                            size_t nPropertyData,
                                                            bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridFontProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridFontProperty* PropertyGrid::AddFontProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridFontProperty>(pGroup, param);
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizeProperty(PropertyGridGroup* pGroup,
                                                                const DString& propertyName,
                                                                const DString& propertyValue,
                                                                const DString& description,
                                                                size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridFontSizeProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizePropertyById(PropertyGridGroup* pGroup,
                                                                    const DString& propertyNameId,
                                                                    const DString& propertyValue,
                                                                    const DString& descriptionId,
                                                                    size_t nPropertyData,
                                                                    bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridFontSizeProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridFontSizeProperty* PropertyGrid::AddFontSizeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridFontSizeProperty>(pGroup, param);
}

PropertyGridColorProperty* PropertyGrid::AddColorProperty(PropertyGridGroup* pGroup,
                                                          const DString& propertyName,
                                                          const DString& propertyValue,
                                                          const DString& description,
                                                          size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridColorProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridColorProperty* PropertyGrid::AddColorPropertyById(PropertyGridGroup* pGroup,
                                                             const DString& propertyNameId,
                                                             const DString& propertyValue,
                                                             const DString& descriptionId,
                                                             size_t nPropertyData,
                                                             bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridColorProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridColorProperty* PropertyGrid::AddColorProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridColorProperty>(pGroup, param);
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimeProperty(PropertyGridGroup* pGroup,
                                                                const DString& propertyName,
                                                                const DString& dateTimeValue,                                                                
                                                                const DString& description,
                                                                size_t nPropertyData,
                                                                DateTime::EditFormat editFormat)
{
    return AddPropertyImpl<PropertyGridDateTimeProperty>(pGroup, MakePropertyParam(propertyName, dateTimeValue, description, nPropertyData), editFormat);
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimePropertyById(PropertyGridGroup* pGroup,
                                                                    const DString& propertyNameId,
                                                                    const DString& dateTimeValue,
                                                                    const DString& descriptionId,
                                                                    size_t nPropertyData,
                                                                    bool bPropertyValueId,
                                                                    DateTime::EditFormat editFormat)
{
    return AddPropertyImpl<PropertyGridDateTimeProperty>(pGroup, MakePropertyParamById(propertyNameId, dateTimeValue, descriptionId, nPropertyData, bPropertyValueId), editFormat);
}

PropertyGridDateTimeProperty* PropertyGrid::AddDateTimeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                                                ui::DateTime::EditFormat editFormat)
{
    return AddPropertyImpl<PropertyGridDateTimeProperty>(pGroup, param, editFormat);
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressProperty(PropertyGridGroup* pGroup,
                                                                  const DString& propertyName,
                                                                  const DString& propertyValue,
                                                                  const DString& description,
                                                                  size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridIPAddressProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressPropertyById(PropertyGridGroup* pGroup,
                                                                      const DString& propertyNameId,
                                                                      const DString& propertyValue,
                                                                      const DString& descriptionId,
                                                                      size_t nPropertyData,
                                                                      bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridIPAddressProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridIPAddressProperty* PropertyGrid::AddIPAddressProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridIPAddressProperty>(pGroup, param);
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyProperty(PropertyGridGroup* pGroup,
                                                            const DString& propertyName,
                                                            const DString& propertyValue,
                                                            const DString& description,
                                                            size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridHotKeyProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyPropertyById(PropertyGridGroup* pGroup,
                                                                const DString& propertyNameId,
                                                                const DString& propertyValue,
                                                                const DString& descriptionId,
                                                                size_t nPropertyData,
                                                                bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridHotKeyProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridHotKeyProperty* PropertyGrid::AddHotKeyProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridHotKeyProperty>(pGroup, param);
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
    return AddPropertyImpl<PropertyGridFileProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData),
                                                     bOpenFileDialog, fileTypes, nFileTypeIndex, defaultExt);
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
    return AddPropertyImpl<PropertyGridFileProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId),
                                                     bOpenFileDialog, fileTypes, nFileTypeIndex, defaultExt);
}

PropertyGridFileProperty* PropertyGrid::AddFileProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                                        bool bOpenFileDialog,
                                                        const std::vector<FileDialog::FileType>& fileTypes,
                                                        int32_t nFileTypeIndex,
                                                        const DString& defaultExt)
{
    return AddPropertyImpl<PropertyGridFileProperty>(pGroup, param, bOpenFileDialog, fileTypes, nFileTypeIndex, defaultExt);
}

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryProperty(PropertyGridGroup* pGroup,
                                                                  const DString& propertyName,
                                                                  const DString& propertyValue,
                                                                  const DString& description,
                                                                  size_t nPropertyData)
{
    return AddPropertyImpl<PropertyGridDirectoryProperty>(pGroup, MakePropertyParam(propertyName, propertyValue, description, nPropertyData));
}

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryPropertyById(PropertyGridGroup* pGroup,
                                                                      const DString& propertyNameId,
                                                                      const DString& propertyValue,
                                                                      const DString& descriptionId,
                                                                      size_t nPropertyData,
                                                                      bool bPropertyValueId)
{
    return AddPropertyImpl<PropertyGridDirectoryProperty>(pGroup, MakePropertyParamById(propertyNameId, propertyValue, descriptionId, nPropertyData, bPropertyValueId));
}

PropertyGridDirectoryProperty* PropertyGrid::AddDirectoryProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param)
{
    return AddPropertyImpl<PropertyGridDirectoryProperty>(pGroup, param);
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

DString PropertyGrid::GetPropertyNormalFontId() const
{
    return m_propertyNormalFontId.c_str();
}

void PropertyGrid::SetPropertyModifiedFontId(const DString& fontId)
{
    m_propertyModifiedFontId = fontId;
}

DString PropertyGrid::GetPropertyModifiedFontId() const
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
        return GlobalManager::GetTextById(m_descriptionId.c_str());
    }
    return DString();
}

void PropertyGridGroup::SetDescription(const DString& description)
{
    m_description = description;
    if (GetPropertyGrid() != nullptr) {
        GetPropertyGrid()->UpdateDescriptionArea();
    }
}

DString PropertyGridGroup::GetDescriptionId() const
{
    return m_descriptionId.c_str();
}

void PropertyGridGroup::SetDescriptionId(const DString& descriptionId)
{
    m_descriptionId = descriptionId;
    if (GetPropertyGrid() != nullptr) {
        GetPropertyGrid()->UpdateDescriptionArea();
    }
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
        //选中状态由整行的高亮显示，此处不再显示单元格级别的焦点框
        SetShowFocusedRect(false);
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
        m_propertyValueId.clear();
        if (m_pLabelBoxRight != nullptr) {
            m_pLabelBoxRight->SetText(propertyValue);
            m_pLabelBoxRight->SetTextId(_T(""));
            //设置原值时，恢复为正常字体（清除"已修改"的显示状态）
            DString propertyNormalFontId;
            if (m_pPropertyGrid != nullptr) {
                propertyNormalFontId = m_pPropertyGrid->GetPropertyNormalFontId();
            }
            if (!propertyNormalFontId.empty()) {
                m_pLabelBoxRight->SetFontId(propertyNormalFontId);
            }
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
        m_propertyValue.clear();
        if (m_pLabelBoxRight != nullptr) {
            m_pLabelBoxRight->SetText(_T(""));
            m_pLabelBoxRight->SetTextId(propertyValueId);
            //设置原值时，恢复为正常字体（清除"已修改"的显示状态）
            DString propertyNormalFontId;
            if (m_pPropertyGrid != nullptr) {
                propertyNormalFontId = m_pPropertyGrid->GetPropertyNormalFontId();
            }
            if (!propertyNormalFontId.empty()) {
                m_pLabelBoxRight->SetFontId(propertyNormalFontId);
            }
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
    if (GetPropertyGrid() != nullptr) {
        GetPropertyGrid()->UpdateDescriptionArea();
    }
}

DString PropertyGridProperty::GetDescriptionId() const
{
    return m_descriptionId.c_str();
}

void PropertyGridProperty::SetDescriptionId(const DString& descriptionId)
{
    m_descriptionId = descriptionId;
    if (GetPropertyGrid() != nullptr) {
        GetPropertyGrid()->UpdateDescriptionArea();
    }
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
        propertyNormalFontId = m_pPropertyGrid->GetPropertyNormalFontId();
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

    //编辑控件按需创建（首次点击属性值时创建），此处不再预创建
    //编辑控件相关的事件，由具体的属性子类在创建编辑控件时挂载
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

DString PropertyGridProperty::GetValueEditorClass(const DString& defaultClass) const
{
    if (m_pPropertyGrid == nullptr) {
        return defaultClass;
    }
    DString className;
    switch (GetPropertyType()) {
    case PropertyGridPropertyType::kText:
    case PropertyGridPropertyType::kFont:
    case PropertyGridPropertyType::kFontSize:
    case PropertyGridPropertyType::kFile:
    case PropertyGridPropertyType::kDirectory:
        className = m_pPropertyGrid->GetPropertyEditClass();
        break;
    case PropertyGridPropertyType::kCombo:
        className = m_pPropertyGrid->GetPropertyComboClass();
        break;
    case PropertyGridPropertyType::kColor:
        className = m_pPropertyGrid->GetPropertyComboButtonClass();
        break;
    case PropertyGridPropertyType::kDateTime:
        className = m_pPropertyGrid->GetPropertyDateTimeClass();
        break;
    case PropertyGridPropertyType::kIPAddress:
        className = m_pPropertyGrid->GetPropertyIPAddressClass();
        break;
    case PropertyGridPropertyType::kHotKey:
        className = m_pPropertyGrid->GetPropertyHotKeyClass();
        break;
    default:
        break;
    }
    if (className.empty()) {
        className = defaultClass;
    }
    return className;
}

DString PropertyGridProperty::GetValueButtonClass(const DString& defaultClass) const
{
    DString className;
    if (m_pPropertyGrid != nullptr) {
        className = m_pPropertyGrid->GetPropertyButtonClass();
    }
    if (className.empty()) {
        className = defaultClass;
    }
    return className;
}

DString PropertyGridProperty::GetValueSpinClass(const DString& defaultClass) const
{
    DString className;
    if (m_pPropertyGrid != nullptr) {
        className = m_pPropertyGrid->GetPropertySpinClass();
    }
    if (className.empty()) {
        className = defaultClass;
    }
    return className;
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
                propertyModifiedFontId = m_pPropertyGrid->GetPropertyModifiedFontId();
            }
            if (!propertyModifiedFontId.empty()) {
                m_pLabelBoxRight->SetFontId(propertyModifiedFontId);
            }
        }
        else {
            DString propertyNormalFontId;
            ASSERT(m_pPropertyGrid != nullptr);
            if (m_pPropertyGrid != nullptr) {
                propertyNormalFontId = m_pPropertyGrid->GetPropertyNormalFontId();
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
        if (bReadOnly) {
            //只读时销毁编辑控件；解除只读时按需创建（首次点击属性值时创建）
            EnableEditControl(false);
        }
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
    m_bTextEdited(false),
    m_realText(),
    m_bSpinEnable(false),
    m_nSpinMin(0),
    m_nSpinMax(0)
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
    m_bTextEdited(false),
    m_realText(),
    m_bSpinEnable(false),
    m_nSpinMin(0),
    m_nSpinMax(0)
{
}

PropertyGridTextProperty::PropertyGridTextProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pRichEdit(nullptr),
    m_bPasswordMode(false),
    m_bTextEdited(false),
    m_realText(),
    m_bSpinEnable(false),
    m_nSpinMin(0),
    m_nSpinMax(0)
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
    m_pRichEdit->SetClass(GetValueEditorClass(_T("property_grid_property_edit")));
    if (!AddPropertySubItem(m_pRichEdit.get())) {
        delete m_pRichEdit.get();
        m_pRichEdit = nullptr;
        return;
    }
    //编辑框的属性：在property_grid.xml中定义
    DString initText;
    if (!m_realText.empty()) {
        initText = m_realText.c_str();
    }
    else {
        //密码模式下，显示文本是掩码，不能作为编辑框的初始值
        initText = m_bPasswordMode ? GetPropertyValue() : GetPropertyText();
    }
    m_pRichEdit->SetText(initText);
    m_pRichEdit->SetVisible(false);

    //回放密码模式与Spin控件的设置
    if (m_bPasswordMode) {
        m_pRichEdit->SetPasswordMode(true);
        m_pRichEdit->SetFlashPasswordChar(true);
    }
    if (m_bSpinEnable) {
        DString spinClass = GetValueSpinClass(_T("property_grid_spin_box,property_grid_spin_btn_up,property_grid_spin_btn_down"));
        m_pRichEdit->SetEnableSpin(true, spinClass, m_nSpinMin, m_nSpinMax);
    }

    //挂载回车和焦点切换事件
    m_pRichEdit->AttachReturn([this](const EventArgs&) {
        SetPropertyFocus();
        return true;
        });
    m_pRichEdit->AttachEsc([this](const EventArgs&) {
        m_pRichEdit->SetText(m_oldText.c_str());
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

    if (!m_bTextEdited) {
        //未编辑过，同步显示和编辑数据
        DString propValue = GetPropertyValue();
        m_realText = propValue;
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetText(propValue);
        }
        if (m_bPasswordMode && !propValue.empty()) {
            DString showText;
            showText.resize(propValue.size(), _T('*'));
            SetPropertyText(showText, false);
        }
        else {
            SetPropertyText(propValue, false);
        }
    }
}

Control* PropertyGridTextProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pRichEdit == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pRichEdit == nullptr) {
            return nullptr;
        }
        m_bTextEdited = true;
        m_oldText = m_pRichEdit->GetText();
        m_pRichEdit->SetVisible(true);
        m_pRichEdit->SetFocus();
    }
    else {
        if (m_pRichEdit == nullptr) {
            return nullptr;
        }
        DString newText;
        if (bCancel) {
            //取消编辑
            m_pRichEdit->SetText(m_oldText.c_str());
            newText = m_oldText.c_str();
        }
        else {
            newText = m_pRichEdit->GetText();
        }
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
        m_realText = newText;
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
    if (m_bPasswordMode == bPasswordMode) {
        return;
    }
    m_bPasswordMode = bPasswordMode;

    DString text = m_realText.empty() ? GetPropertyText() : m_realText.c_str();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetPasswordMode(bPasswordMode);
        m_pRichEdit->SetFlashPasswordChar(true);
        m_pRichEdit->SetText(text);
    }
    bool bChanged = (text != GetPropertyValue()); //相对原值，是否有修改
    if (bPasswordMode) {
        DString showText;
        showText.resize(text.size(), _T('*'));
        SetPropertyText(showText, bChanged);
    }
    else {
        SetPropertyText(text, bChanged);
    }
}

void PropertyGridTextProperty::SetEnableSpin(bool bEnable, int64_t nMin, int64_t nMax)
{
    m_bSpinEnable = bEnable;
    m_nSpinMin = nMin;
    m_nSpinMax = nMax;

    if (m_pRichEdit != nullptr) {
        DString spinClass = GetValueSpinClass(_T("property_grid_spin_box,property_grid_spin_btn_up,property_grid_spin_btn_down"));
        m_pRichEdit->SetEnableSpin(bEnable, spinClass, nMin, nMax);
    }
}

DString PropertyGridTextProperty::GetPropertyNewValue() const
{
    DString propertyValue = GetPropertyValue();
    if (!IsReadOnly()) {
        if (m_pRichEdit != nullptr) {
            propertyValue = m_pRichEdit->GetText();
        }
        else if (IsPasswordMode() && !m_realText.empty()) {
            propertyValue = m_realText.c_str();
        }
    }
    return propertyValue;
}

void PropertyGridTextProperty::SetNewTextValue(const DString& newText)
{
    if (IsReadOnly()) {
        return;
    }
    m_bTextEdited = true;
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetText(newText);
    }
    m_realText = newText;
    bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
    if (IsPasswordMode()) {
        DString showText;
        showText.resize(newText.size(), _T('*'));
        SetPropertyText(showText, bChanged);
    }
    else {
        SetPropertyText(newText, bChanged);
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
    m_oldText(),
    m_oldSelItem(Box::InvalidIndex),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_optionList(),
    m_curSel(Box::InvalidIndex),
    m_bListMode(false)
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
    m_oldText(),
    m_oldSelItem(Box::InvalidIndex),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_optionList(),
    m_curSel(Box::InvalidIndex),
    m_bListMode(false)
{
}

PropertyGridComboProperty::PropertyGridComboProperty(Window* pWindow, const PropertyGridParam& param):
    PropertyGridProperty(pWindow, param),
    m_pCombo(nullptr),
    m_oldText(),
    m_oldSelItem(Box::InvalidIndex),
    m_bComboEdited(false),
    m_bComboEditing(false),
    m_optionList(),
    m_curSel(Box::InvalidIndex),
    m_bListMode(false)
{
}

void PropertyGridComboProperty::ApplyComboOptions()
{
    if (m_pCombo == nullptr) {
        return;
    }
    m_pCombo->DeleteAllItems();
    for (size_t i = 0; i < m_optionList.size(); ++i) {
        const ComboOption& option = m_optionList[i];
        if (!option.m_textId.empty()) {
            m_pCombo->AddTextIdItem(option.m_textId.c_str());
        }
        else {
            m_pCombo->AddTextItem(option.m_text.c_str());
        }
        if (option.m_bHasData) {
            m_pCombo->SetItemData(i, option.m_nData);
        }
    }
    if ((m_curSel != Box::InvalidIndex) && (m_curSel < m_optionList.size())) {
        m_pCombo->SetCurSel(m_curSel, false);
    }
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
    m_pCombo->SetClass(GetValueEditorClass(_T("property_grid_combo")));
    if (!AddPropertySubItem(m_pCombo.get())) {
        delete m_pCombo.get();
        m_pCombo = nullptr;
        return;
    }
    //编辑框的属性：在property_grid.xml中定义
    m_pCombo->SetVisible(false);
    m_pCombo->SetComboType(m_bListMode ? Combo::ComboType::kCombo_DropList : Combo::ComboType::kCombo_DropDown);

    //回放暂存的选项
    ApplyComboOptions();
    m_pCombo->SetText(GetPropertyText());

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
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pCombo == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pCombo == nullptr) {
            return nullptr;
        }
        m_bComboEditing = true;
        m_bComboEdited = true;
        m_pCombo->SetVisible(true);
        m_pCombo->SetFocus();
        m_oldSelItem = m_pCombo->GetCurSel();
        m_oldText = m_pCombo->GetText();
    }
    else {
        if (m_pCombo == nullptr) {
            return nullptr;
        }
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
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
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
        //仅在编辑状态下，或者属性值使用多语言文本ID时，才用下拉框的文本覆盖显示值
        //避免应用调用SetPropertyValue设置新的显示值时，被下拉框的旧文本覆盖
        if (m_pCombo->IsVisible() || UsingPropertyValueId()) {
            DString newText = m_pCombo->GetText();
            bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
            SetPropertyText(newText, bChanged);
        }
    }
}

DString PropertyGridComboProperty::GetPropertyNewValue() const
{
    DString propertyValue = GetPropertyValue();
    if (!IsReadOnly()) {
        if (m_pCombo != nullptr) {
            propertyValue = m_pCombo->GetText();
        }
        else if (m_curSel != Box::InvalidIndex) {
            propertyValue = GetOption(m_curSel);
        }
    }
    return propertyValue;
}

size_t PropertyGridComboProperty::AddOption(const DString& optionText)
{
    ComboOption option;
    option.m_text = optionText;
    m_optionList.push_back(option);

    size_t nIndex = m_optionList.size() - 1;
    if (m_pCombo != nullptr) {
        ASSERT(m_pCombo->AddTextItem(optionText) == nIndex);
    }
    return nIndex;
}

size_t PropertyGridComboProperty::AddOptionById(const DString& optionTextId)
{
    ComboOption option;
    option.m_textId = optionTextId;
    m_optionList.push_back(option);

    size_t nIndex = m_optionList.size() - 1;
    if (m_pCombo != nullptr) {
        ASSERT(m_pCombo->AddTextIdItem(optionTextId) == nIndex);
    }
    return nIndex;
}

void PropertyGridComboProperty::SetOptionData(size_t nIndex, size_t nOptionData)
{
    ASSERT(nIndex < m_optionList.size());
    if (nIndex < m_optionList.size()) {
        m_optionList[nIndex].m_nData = nOptionData;
        m_optionList[nIndex].m_bHasData = true;
    }
    if (m_pCombo != nullptr) {
        m_pCombo->SetItemData(nIndex, nOptionData);
    }
}

size_t PropertyGridComboProperty::GetOptionData(size_t nIndex) const
{
    size_t nOptionData = 0;
    if (nIndex < m_optionList.size()) {
        nOptionData = m_optionList[nIndex].m_nData;
    }
    return nOptionData;
}

bool PropertyGridComboProperty::HasOptionData(size_t nIndex) const
{
    if (nIndex < m_optionList.size()) {
        return m_optionList[nIndex].m_bHasData;
    }
    return false;
}

size_t PropertyGridComboProperty::GetOptionCount() const
{
    return m_optionList.size();
}

DString PropertyGridComboProperty::GetOption(size_t nIndex) const
{
    DString text;
    if (nIndex < m_optionList.size()) {
        const ComboOption& option = m_optionList[nIndex];
        if (!option.m_text.empty()) {
            text = option.m_text.c_str();
        }
        else if (!option.m_textId.empty()) {
            text = GlobalManager::GetTextById(option.m_textId.c_str());
        }
    }
    return text;
}

bool PropertyGridComboProperty::RemoveOption(size_t nIndex)
{
    if (nIndex >= m_optionList.size()) {
        return false;
    }
    m_optionList.erase(m_optionList.begin() + nIndex);
    if (m_curSel == nIndex) {
        m_curSel = Box::InvalidIndex;
    }
    else if ((m_curSel != Box::InvalidIndex) && (m_curSel > nIndex)) {
        m_curSel -= 1;
    }
    if (m_pCombo != nullptr) {
        m_pCombo->DeleteItem(nIndex);
    }
    return true;
}

void PropertyGridComboProperty::RemoveAllOptions()
{
    m_optionList.clear();
    m_curSel = Box::InvalidIndex;
    if (m_pCombo != nullptr) {
        m_pCombo->DeleteAllItems();
    }
}

size_t PropertyGridComboProperty::GetCurSel() const
{
    if (m_pCombo != nullptr) {
        return m_pCombo->GetCurSel();
    }
    return m_curSel;
}

bool PropertyGridComboProperty::SetCurSel(size_t nIndex, bool bTriggerEvent)
{
    if (nIndex >= m_optionList.size()) {
        return false;
    }
    m_curSel = nIndex;
    if (m_pCombo != nullptr) {
        return m_pCombo->SetCurSel(nIndex, bTriggerEvent);
    }
    return true;
}

void PropertyGridComboProperty::SetComboListMode(bool bListMode)
{
    m_bListMode = bListMode;
    if (m_pCombo != nullptr) {
        m_pCombo->SetComboType(bListMode ? Combo::ComboType::kCombo_DropList : Combo::ComboType::kCombo_DropDown);
    }
}

void PropertyGridComboProperty::UpdateEditText(bool bTriggerEvent)
{
    DString newText = GetOption(GetCurSel());
    if (m_pCombo != nullptr) {
        m_pCombo->SetText(newText);
    }
    bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
    SetPropertyText(newText, bChanged, bTriggerEvent);
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
    if (IsInited()) {
        //已经初始化时，立即刷新选项列表
        UpdateFontSizeOptionList();
    }
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
    m_pComboButton->SetClass(GetValueEditorClass(_T("property_grid_combo_button")));
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
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pComboButton == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pComboButton == nullptr) {
            return nullptr;
        }
        m_oldColor.clear();
        Label* pColorLabel = m_pComboButton->GetLabelTop();
        if (pColorLabel != nullptr) {
            m_oldColor = pColorLabel->GetText();
        }
        m_pComboButton->SetVisible(true);
        m_pComboButton->SetFocus();
    }
    else {
        if (m_pComboButton == nullptr) {
            return nullptr;
        }
        if (bCancel) {
            //取消
            OnSelectColor(m_oldColor.c_str());
        }
        DString newText;
        Label* pColorLabel = m_pComboButton->GetLabelTop();
        if (pColorLabel != nullptr) {            
            newText = pColorLabel->GetText();
        }
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
        SetPropertyText(newText, bChanged);
        SetPropertyTextColor(newText);
        m_pComboButton->SetVisible(false);
    }
    return m_pComboButton.get();
}

void PropertyGridColorProperty::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    PropertyGridProperty::PaintChild(pRender, rcPaint);
    LabelBox* pLabelBoxRight = GetLabelBoxRight();
    if ((pRender == nullptr) || (pLabelBoxRight == nullptr)) {
        return;
    }
    if ((m_pComboButton != nullptr) && m_pComboButton->IsVisible()) {
        //编辑状态下，颜色由编辑控件显示
        return;
    }
    //在属性值右侧绘制当前颜色的色块，使得未进入编辑状态时也能看到颜色
    const UiColor bkColor = GetUiColor(GetPropertyText());
    if (bkColor.IsEmpty()) {
        return;
    }
    //属性值单元格的右边界：扣除垂直滚动条占用的宽度（行宽可能延伸到滚动条下方）
    int32_t nVScrollBarWidth = 0;
    TreeView* pTreeView = GetTreeView();
    if (pTreeView != nullptr) {
        ScrollBar* pVScrollBar = pTreeView->GetVScrollBar();
        if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible() && !pTreeView->IsVScrollBarAtLeft()) {
            nVScrollBarWidth = pVScrollBar->GetWidth();
        }
    }
    const UiRect rcLabel = pLabelBoxRight->GetRect();
    const int32_t nSize = 14;
    const int32_t nMarginRight = 10;
    const int32_t nRight = rcLabel.right - nVScrollBarWidth - nMarginRight;
    const int32_t nLeft = nRight - nSize;
    if (nLeft <= rcLabel.left) {
        //显示区域不足时，不绘制色块
        return;
    }
    const int32_t nTop = rcLabel.top + (rcLabel.Height() - nSize) / 2;
    const UiRectF rcSwatch((float)nLeft, (float)nTop, (float)nRight, (float)(nTop + nSize));
    pRender->FillRect(rcSwatch, bkColor);
    pRender->DrawRect(rcSwatch, UiColor(0xFFA0A0A0), 1.0f, true);
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
            pColorLabel->SetBkColor(oldTextColor);
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
    m_pDateTime->SetClass(GetValueEditorClass(_T("property_grid_date_time")));
    if (!AddPropertySubItem(m_pDateTime.get())) {
        delete m_pDateTime.get();
        m_pDateTime = nullptr;
        return;
    }

    m_pDateTime->SetEditFormat(m_editFormat);
    m_pDateTime->SetDateTimeString(GetPropertyText());
    m_pDateTime->SetVisible(false);
    m_pDateTime->SetText(GetPropertyText());

    //挂载焦点切换事件
    m_pDateTime->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false, false);
        return true;
        });
}

Control* PropertyGridDateTimeProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pDateTime == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pDateTime == nullptr) {
            return nullptr;
        }
        m_oldText = m_pDateTime->GetText();
        int32_t nWidth = GetEditControlMarginRight();
        UiMargin rcMargin = m_pDateTime->GetMargin();
        rcMargin.right = nWidth;
        m_pDateTime->SetMargin(rcMargin, false);
        m_pDateTime->SetVisible(true);
        m_pDateTime->SetFocus();
    }
    else {
        if (m_pDateTime == nullptr) {
            return nullptr;
        }
        if (bCancel) {
            //取消编辑，恢复原来的值
            m_pDateTime->SetDateTimeString(m_oldText.c_str());
        }
        DString newText = m_pDateTime->GetText();
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
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
    m_pIPAddress->SetClass(GetValueEditorClass(_T("property_grid_ip_address")));
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

Control* PropertyGridIPAddressProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pIPAddress == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pIPAddress == nullptr) {
            return nullptr;
        }
        m_oldText = m_pIPAddress->GetIPAddress();
        m_pIPAddress->SetVisible(true);
        m_pIPAddress->SetFocus();
    }
    else {
        if (m_pIPAddress == nullptr) {
            return nullptr;
        }
        if (bCancel) {
            //取消编辑，恢复原来的值
            m_pIPAddress->SetIPAddress(m_oldText.c_str());
        }
        DString newText = m_pIPAddress->GetIPAddress();
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
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
    m_pHotKey->SetClass(GetValueEditorClass(_T("property_grid_hot_key")));
    if (!AddPropertySubItem(m_pHotKey.get())) {
        delete m_pHotKey.get();
        m_pHotKey = nullptr;
        return;
    }
    m_pHotKey->SetVisible(false);

    //挂载回车和焦点切换事件
    m_pHotKey->AttachKillFocus([this](const EventArgs&) {
        ShowEditControl(false, false);
        return true;
        });
}

Control* PropertyGridHotKeyProperty::ShowEditControl(bool bShow, bool bCancel)
{
    if (bShow) {
        if (IsReadOnly()) {
            return nullptr;
        }
        if (m_pHotKey == nullptr) {
            //首次编辑时创建编辑控件
            EnableEditControl(true);
        }
        if (m_pHotKey == nullptr) {
            return nullptr;
        }
        //每次进入编辑状态时，都用属性当前的值同步热键控件，确保编辑的是属性的当前值
        m_pHotKey->ClearHotKey();
        m_pHotKey->SetHotKeyName(GetPropertyText());
        m_oldText = m_pHotKey->GetHotKeyName();
        m_nOldHotKeyValue = m_pHotKey->GetHotKey();
        m_pHotKey->SetVisible(true);
        m_pHotKey->SetFocus();
    }
    else {
        if (m_pHotKey == nullptr) {
            return nullptr;
        }
        if (bCancel) {
            //取消编辑：恢复原来的值，属性显示的内容保持不变
            m_pHotKey->SetHotKeyName(m_oldText.c_str());
            m_pHotKey->SetVisible(false);
            return m_pHotKey.get();
        }
        if (m_pHotKey->GetHotKey() == m_nOldHotKeyValue) {
            //热键值没有变化（比如仅点击了属性，或者输入了相同的热键）：
            //保持属性原有的显示文本和字体，不做任何修改
            m_pHotKey->SetVisible(false);
            return m_pHotKey.get();
        }
        DString newText = m_pHotKey->GetHotKeyName();
        bool bChanged = (newText != GetPropertyValue()); //相对原值，是否有修改
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
        m_pBrowseBtn->SetClass(GetValueButtonClass(_T("property_grid_button")));
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
    //以当前的属性值(文件路径)作为初始的目录和文件名
    const FilePath currentPath(GetPropertyValue());
    FilePath defaultLocation = currentPath.GetParentPath();
    DString fileName = currentPath.GetFileName();

    FilePath filePath;
    FileDialog fileDlg;
    if (fileDlg.BrowseForFile(GetWindow(), filePath, m_bOpenFileDialog, m_fileTypes, m_nFileTypeIndex, m_defaultExt, fileName, defaultLocation)) {
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
        m_pBrowseBtn->SetClass(GetValueButtonClass(_T("property_grid_button")));
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

