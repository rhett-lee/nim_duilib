#include "PropertyGrid.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{

PropertyGrid::PropertyGrid():
    m_bHeaderCtrl(true),
    m_bInited(false),
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

void PropertyGrid::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;

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
        const size_t itemCount = m_pTreeView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            PropertyGridProperty* pItem = dynamic_cast<PropertyGridProperty*>(m_pTreeView->GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetHeight() <= 0)) {
                continue;
            }
            //纵坐标位置放在每个子项控件的底部（Header控件的底部不画线）
            int32_t yPos = pItem->GetRect().bottom;
            int32_t nChildMarginY = 0;
            Layout* pLayout = GetLayout();
            if (pLayout != nullptr) {
                nChildMarginY = pLayout->GetChildMarginY();
            }
            yPos += nChildMarginY / 2;
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
    ASSERT(m_bInited);
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
    ASSERT(m_bInited);
    m_bDescriptionArea = bEnable;
    if (m_pDescriptionArea != nullptr) {
        m_pDescriptionArea->SetVisible(bEnable);
    }
}

void PropertyGrid::SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale)
{
    ASSERT(m_bInited);
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
    if (m_bInited) {
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
    if (m_bInited && (m_pHeaderLeft != nullptr)) {
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
    m_bInited(false),
    m_pLabelBox(nullptr),
    m_nGroupData(nGroupData)
{
    m_groupName = groupName;
    m_description = description;
}

void PropertyGridGroup::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;
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
    virtual void DoInit() override
    {
        __super::DoInit();
        SetShowFocusRect(true);
        SetTabStop(false);
    }
};

PropertyGridProperty::PropertyGridProperty(const std::wstring& propertyName, 
                                           const std::wstring& propertyValue,
                                           const std::wstring& description,
                                           size_t nPropertyData):
    m_bInited(false),
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

void PropertyGridProperty::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;
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
        if (!IsReadOnly()) {
            ShowEditControl(true);
        }
        return true;
        });

    //允许或者禁止编辑控件
    EnableEditControl(!IsReadOnly());
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
    virtual void DoInit() override
    {
        __super::DoInit();
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

void PropertyGridTextProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pRichEdit == nullptr)) {
        return;
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

void PropertyGridComboProperty::ShowEditControl(bool bShow)
{
    if (IsReadOnly() || (m_pCombo == nullptr)) {
        return;
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
}

std::wstring PropertyGridComboProperty::GetPropertyNewValue() const
{
    std::wstring propertyValue = GetPropertyValue();
    if (!IsReadOnly() && (m_pCombo != nullptr)) {
        propertyValue = m_pCombo->GetText();
    }
    return propertyValue;
}

void PropertyGridComboProperty::AddOption(const std::wstring& optionText)
{
    ASSERT(m_pCombo != nullptr);
    if (m_pCombo != nullptr) {
        m_pCombo->AddTextItem(optionText);
    }
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

}//namespace ui

