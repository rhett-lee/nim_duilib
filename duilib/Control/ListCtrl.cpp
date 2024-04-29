#include "ListCtrl.h"
#include "duilib/Control/ListCtrlReportView.h"
#include "duilib/Control/ListCtrlIconView.h"
#include "duilib/Core/GlobalManager.h"
#include <set>

namespace ui
{

ListCtrl::ListCtrl():
    m_pHeaderCtrl(nullptr),
    m_pReportView(nullptr),
    m_pIconView(nullptr),
    m_pListView(nullptr),
    m_bEnableHeaderDragOrder(true),
    m_bShowHeaderCtrl(true),
    m_bEnableRefresh(true),
    m_bMultiSelect(true),
    m_bEnableColumnWidthAuto(true),
    m_bAutoCheckSelect(false),
    m_bHeaderShowCheckBox(false),
    m_bDataItemShowCheckBox(false),
    m_listCtrlType(ListCtrlType::Report),
    m_pRichEdit(nullptr),
    m_bEnableItemEdit(true)
{
    m_pData = new ListCtrlData;
    m_nItemHeight = GlobalManager::Instance().Dpi().GetScaleInt(32);
    m_nHeaderHeight = m_nItemHeight;

    m_pData->SetAutoCheckSelect(IsAutoCheckSelect());

    m_pReportView = new ListCtrlReportView;
    m_pReportView->SetListCtrl(this);
    m_pReportView->SetDataProvider(m_pData);

    size_t nCount = sizeof(m_imageList) / sizeof(m_imageList[0]);
    for (size_t i = 0; i < nCount; ++i) {
        m_imageList[i] = nullptr;
    }
}

ListCtrl::~ListCtrl()
{
    if (m_pData != nullptr) {
        delete m_pData;
        m_pData = nullptr;
    }
    if (!IsInited() && (m_pReportView != nullptr)) {
        delete m_pReportView;
        m_pReportView = nullptr;
    }
    if (!IsInited() && (m_pIconView != nullptr)) {
        delete m_pIconView;
        m_pIconView = nullptr;
    }
    if (!IsInited() && (m_pListView != nullptr)) {
        delete m_pListView;
        m_pListView = nullptr;
    }
    if (!IsInited() && (m_pRichEdit != nullptr)) {
        delete m_pRichEdit;
        m_pRichEdit = nullptr;
    }
    std::set<ImageList*> pImageSet;
    for (ImageList* pImage : m_imageList) {
        pImageSet.insert(pImage);        
    }
    for (ImageList* pImage : pImageSet) {
        if (pImage != nullptr) {
            delete pImage;
            pImage = nullptr;
        }
    }
}

std::wstring ListCtrl::GetType() const { return DUI_CTR_LISTCTRL; }

void ListCtrl::SetImageList(ListCtrlType type, ImageList* pImageList)
{
    ImageList* pOldImageList = nullptr;
    if (type == ListCtrlType::Report) {
        pOldImageList = m_imageList[0];
        m_imageList[0] = pImageList;
    }
    else if (type == ListCtrlType::Icon) {
        pOldImageList = m_imageList[1];
        m_imageList[1] = pImageList;
    }
    else if (type == ListCtrlType::List) {
        pOldImageList = m_imageList[2];
        m_imageList[2] = pImageList;
    }
    else {
        ASSERT(FALSE);
    }
    std::set<ImageList*> pImageSet;
    for (ImageList* pImage : m_imageList) {
        pImageSet.insert(pImage);
    }
    if ((pOldImageList != nullptr) && (pImageSet.find(pOldImageList) == pImageSet.end())) {
        delete pOldImageList;
        pOldImageList = nullptr;
    }
}

ImageList* ListCtrl::GetImageList(ListCtrlType type)
{
    if (type == ListCtrlType::Report) {
        return m_imageList[0];
    }
    else if (type == ListCtrlType::Icon) {
        return m_imageList[1];
    }
    else if (type == ListCtrlType::List) {
        return m_imageList[2];
    }
    else {
        return nullptr;
    }
}

const ImageList* ListCtrl::GetImageList(ListCtrlType type) const
{
    if (type == ListCtrlType::Report) {
        return m_imageList[0];
    }
    else if (type == ListCtrlType::Icon) {
        return m_imageList[1];
    }
    else if (type == ListCtrlType::List) {
        return m_imageList[2];
    }
    else {
        return nullptr;
    }
}

void ListCtrl::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"header_class") {
        SetHeaderClass(strValue);
    }
    else if (strName == L"header_item_class") {
        SetHeaderItemClass(strValue);
    }
    else if (strName == L"header_split_box_class") {
        SetHeaderSplitBoxClass(strValue);
    }
    else if (strName == L"header_split_control_class") {
        SetHeaderSplitControlClass(strValue);
    }
    else if (strName == L"enable_header_drag_order") {
        SetEnableHeaderDragOrder(strValue == L"true");
    }
    else if (strName == L"check_box_class") {
        SetCheckBoxClass(strValue);
    }
    else if (strName == L"data_item_class") {
        SetDataItemClass(strValue);
    }
    else if (strName == L"data_sub_item_class") {
        SetDataSubItemClass(strValue);
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
    else if (strName == L"report_view_class") {
        SetReportViewClass(strValue);
    }
    else if (strName == L"header_height") {
        SetHeaderHeight(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"data_item_height") {
        SetDataItemHeight(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"show_header") {
        SetHeaderVisible(strValue == L"true");
    }
    else if (strName == L"multi_select") {
        SetMultiSelect(strValue == L"true");
    }
    else if (strName == L"enable_column_width_auto") {
        SetEnableColumnWidthAuto(strValue == L"true");
    }
    else if (strName == L"auto_check_select") {
        SetAutoCheckSelect(strValue == L"true");
    }
    else if (strName == L"show_header_checkbox") {
        SetHeaderShowCheckBox(strValue == L"true");
    }
    else if (strName == L"show_data_item_checkbox") {
        SetDataItemShowCheckBox(strValue == L"true");
    }
    else if (strName == L"type") {
        if (strValue == L"report") {
            SetListCtrlType(ListCtrlType::Report);
        }
        else if (strValue == L"icon") {
            SetListCtrlType(ListCtrlType::Icon);
        }
        else if (strValue == L"list") {
            SetListCtrlType(ListCtrlType::List);
        }
    }
    else if (strName == L"icon_view_class") {
        SetIconViewClass(strValue);
    }
    else if (strName == L"icon_view_item_class") {
        SetIconViewItemClass(strValue);
    }
    else if (strName == L"icon_view_item_image_class") {
        SetIconViewItemImageClass(strValue);
    }
    else if (strName == L"icon_view_item_label_class") {
        SetIconViewItemLabelClass(strValue);
    }
    else if (strName == L"list_view_class") {
        SetListViewClass(strValue);
    }
    else if (strName == L"list_view_item_class") {
        SetListViewItemClass(strValue);
    }
    else if (strName == L"list_view_item_image_class") {
        SetListViewItemImageClass(strValue);
    }
    else if (strName == L"list_view_item_label_class") {
        SetListViewItemLabelClass(strValue);
    }
    else if (strName == L"enable_item_edit") {
        SetEnableItemEdit(strValue == L"true");
    }
    else if (strName == L"list_ctrl_richedit_class") {
        SetRichEditClass(strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrl::HandleEvent(const EventArgs& msg)
{
    __super::HandleEvent(msg);
    if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
        OnViewKeyboardEvents(msg);
    }
    else if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
        OnViewMouseEvents(msg);
    }
}

void ListCtrl::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    //初始化Body
    ASSERT(m_pReportView != nullptr);
    AddItem(m_pReportView);
    m_pReportView->SetClass(GetReportViewClass());

    //初始化Header
    ASSERT(m_pHeaderCtrl == nullptr);
    if (m_pHeaderCtrl == nullptr) {
        m_pHeaderCtrl = new ListCtrlHeader;
    }
    m_pHeaderCtrl->SetListCtrl(this);
    // Header添加到数据视图中管理，作为第一个元素，在Layout的实现中控制显示属性
    m_pReportView->AddItem(m_pHeaderCtrl);

    if (!m_headerClass.empty()) {
        m_pHeaderCtrl->SetClass(m_headerClass.c_str());
    }
    m_pHeaderCtrl->SetAutoCheckSelect(false);
    m_pHeaderCtrl->SetShowCheckBox(m_bHeaderShowCheckBox); //是否显示CheckBox
    m_pHeaderCtrl->SetFixedHeight(UiFixedInt(m_nHeaderHeight), true, false);
    if (!m_bShowHeaderCtrl) {
        SetHeaderVisible(false);
    }

    //同步单选和多选的状态
    m_pReportView->SetDataProvider(m_pData);

    //更新默认的文本属性
    SetDataSubItemClass(GetDataSubItemClass());

    //更新默认的行高
    m_pData->SetDefaultItemHeight(GetDataItemHeight());

    InitReportView();
    SetListCtrlType(GetListCtrlType());
}

void ListCtrl::InitReportView()
{
    ASSERT(m_pReportView != nullptr);
    if (m_pReportView == nullptr) {
        return;
    }
    m_pReportView->SetMultiSelect(IsMultiSelect());

    //事件转接函数
    auto OnReportViewEvent = [this](const EventArgs& args) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pReportView->GetItemAt(nItemIndex);
            ListCtrlItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlItem*>(pControl);
            }
            if (pItem != nullptr) {
                EventArgs msg = args;
                msg.wParam = (WPARAM)pItem;
                msg.lParam = pItem->GetElementIndex();
                msg.pSender = this;
                SendEvent(msg);
            }
            else if (args.Type == kEventSelChange) {
                EventArgs msg = args;
                msg.pSender = this;
                SendEvent(msg);
            }
        };

    //挂载事件，转接给外层
    m_pReportView->AttachSelect([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
    m_pReportView->AttachSelChange([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
    m_pReportView->AttachDoubleClick([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
    m_pReportView->AttachClick([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
    m_pReportView->AttachRClick([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
    m_pReportView->AttachReturn([this, OnReportViewEvent](const EventArgs& args) {
        OnReportViewEvent(args);
        return true;
        });
}

void ListCtrl::InitIconView()
{
    ASSERT(m_pIconView != nullptr);
    if (m_pIconView == nullptr) {
        return;
    }
    m_pIconView->SetMultiSelect(IsMultiSelect());

    //事件转接函数
    auto OnIconViewEvent = [this](const EventArgs& args) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pIconView->GetItemAt(nItemIndex);
            ListCtrlIconViewItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlIconViewItem*>(pControl);
            }
            if (pItem != nullptr) {
                EventArgs msg = args;
                msg.wParam = (WPARAM)pItem;
                msg.lParam = pItem->GetElementIndex();
                msg.pSender = this;
                SendEvent(msg);
            }
            else if (args.Type == kEventSelChange) {
                EventArgs msg = args;
                msg.pSender = this;
                SendEvent(msg);
            }
        };

    //挂载事件，转接给外层
    m_pIconView->AttachSelect([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
    m_pIconView->AttachSelChange([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
    m_pIconView->AttachDoubleClick([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
    m_pIconView->AttachClick([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
    m_pIconView->AttachRClick([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
    m_pIconView->AttachReturn([this, OnIconViewEvent](const EventArgs& args) {
        OnIconViewEvent(args);
        return true;
        });
}

void ListCtrl::InitListView()
{
    ASSERT(m_pListView != nullptr);
    if (m_pListView == nullptr) {
        return;
    }
    m_pIconView->SetMultiSelect(IsMultiSelect());

    //事件转接函数
    auto OnListViewEvent = [this](const EventArgs& args) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pListView->GetItemAt(nItemIndex);
            ListCtrlListViewItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlListViewItem*>(pControl);
            }
            if (pItem != nullptr) {
                EventArgs msg = args;
                msg.wParam = (WPARAM)pItem;
                msg.lParam = pItem->GetElementIndex();
                msg.pSender = this;
                SendEvent(msg);
            }
            else if (args.Type == kEventSelChange) {
                EventArgs msg = args;
                msg.pSender = this;
                SendEvent(msg);
            }
        };

    //挂载事件，转接给外层
    m_pListView->AttachSelect([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
    m_pListView->AttachSelChange([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
    m_pListView->AttachDoubleClick([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
    m_pListView->AttachClick([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
    m_pListView->AttachRClick([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
    m_pListView->AttachReturn([this, OnListViewEvent](const EventArgs& args) {
        OnListViewEvent(args);
        return true;
        });
}

void ListCtrl::SetListCtrlType(ListCtrlType type)
{
    m_listCtrlType = type;
    if (!IsInited()) {
        return;
    }
    if (m_listCtrlType == ListCtrlType::Report) {
        m_pReportView->SetDataProvider(m_pData);
        m_pData->SetListView(m_pReportView);
        m_pReportView->SetVisible(true);
        if (m_pReportView->IsMultiSelect() != IsMultiSelect()) {
            m_pReportView->SetMultiSelect(IsMultiSelect());
        }
        if (m_pIconView != nullptr) {
            m_pIconView->SetVisible(false);
        }
        if (m_pListView != nullptr) {
            m_pListView->SetVisible(false);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView == nullptr) {
            m_pIconView = new ListCtrlIconView(false);
            m_pIconView->SetListCtrl(this);
            AddItem(m_pIconView);
            m_pIconView->SetClass(GetIconViewClass());
            InitIconView();
        }
        m_pIconView->SetDataProvider(m_pData);
        m_pData->SetListView(m_pIconView);
        m_pIconView->SetVisible(true);
        if (m_pIconView->IsMultiSelect() != IsMultiSelect()) {
            m_pIconView->SetMultiSelect(IsMultiSelect());
        }
        if (m_pReportView != nullptr) {
            m_pReportView->SetVisible(false);
        }
        if (m_pListView != nullptr) {
            m_pListView->SetVisible(false);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView == nullptr) {
            m_pListView = new ListCtrlIconView(true);
            m_pListView->SetListCtrl(this);
            AddItem(m_pListView);
            m_pListView->SetClass(GetListViewClass());
            InitListView();
        }
        m_pListView->SetDataProvider(m_pData);
        m_pData->SetListView(m_pListView);
        m_pListView->SetVisible(true);
        if (m_pListView->IsMultiSelect() != IsMultiSelect()) {
            m_pListView->SetMultiSelect(IsMultiSelect());
        }
        if (m_pReportView != nullptr) {
            m_pReportView->SetVisible(false);
        }
        if (m_pIconView != nullptr) {
            m_pIconView->SetVisible(false);
        }
    }
}

ListCtrlType ListCtrl::GetListCtrlType() const
{
    return m_listCtrlType;
}

void ListCtrl::SetHeaderClass(const std::wstring& className)
{
    m_headerClass = className;
    if (IsInited() && (m_pHeaderCtrl != nullptr)) {
        m_pHeaderCtrl->SetClass(className);
    }
}

void ListCtrl::SetHeaderItemClass(const std::wstring& className)
{
    m_headerItemClass = className;
}

std::wstring ListCtrl::GetHeaderItemClass() const
{
    return m_headerItemClass.c_str();
}

void ListCtrl::SetHeaderSplitBoxClass(const std::wstring& className)
{
    m_headerSplitBoxClass = className;
}

std::wstring ListCtrl::GetHeaderSplitBoxClass() const
{
    return m_headerSplitBoxClass.c_str();
}

void ListCtrl::SetHeaderSplitControlClass(const std::wstring& className)
{
    m_headerSplitControlClass = className;
}

std::wstring ListCtrl::GetHeaderSplitControlClass() const
{
    return m_headerSplitControlClass.c_str();
}

void ListCtrl::SetCheckBoxClass(const std::wstring& className)
{
    m_checkBoxClass = className;
}

std::wstring ListCtrl::GetCheckBoxClass() const
{
    return m_checkBoxClass.c_str();
}

void ListCtrl::SetReportViewClass(const std::wstring& className)
{
    m_reportViewClass = className;
    if (IsInited() && (m_pReportView != nullptr)) {
        m_pReportView->SetClass(className);
    }
}

std::wstring ListCtrl::GetReportViewClass() const
{
    return m_reportViewClass.c_str();
}

void ListCtrl::SetDataItemClass(const std::wstring& className)
{
    m_dataItemClass = className;
}

std::wstring ListCtrl::GetDataItemClass() const
{
    return m_dataItemClass.c_str();
}

void ListCtrl::SetDataSubItemClass(const std::wstring& className)
{
    m_dataSubItemClass = className;
    if (IsInited() && !className.empty()) {
        ListCtrlSubItem defaultSubItem;
        defaultSubItem.SetWindow(GetWindow());
        defaultSubItem.SetClass(className);
        m_pData->SetDefaultTextStyle(defaultSubItem.GetTextStyle());
    }
}

std::wstring ListCtrl::GetDataSubItemClass() const
{
    return m_dataSubItemClass.c_str();
}

void ListCtrl::SetIconViewClass(const std::wstring& className)
{
    m_iconViewClass = className;
    if (IsInited() && (m_pIconView != nullptr)) {
        m_pIconView->SetClass(className);
    }
}

std::wstring ListCtrl::GetIconViewClass() const
{
    return m_iconViewClass.c_str();
}

void ListCtrl::SetIconViewItemClass(const std::wstring& className)
{
    m_iconViewItemClass = className;
}

std::wstring ListCtrl::GetIconViewItemClass() const
{
    return m_iconViewItemClass.c_str();
}

void ListCtrl::SetIconViewItemImageClass(const std::wstring& className)
{
    m_iconViewItemImageClass = className;
}

std::wstring ListCtrl::GetIconViewItemImageClass() const
{
    return m_iconViewItemImageClass.c_str();
}

void ListCtrl::SetIconViewItemLabelClass(const std::wstring& className)
{
    m_iconViewItemLabelClass = className;
}

std::wstring ListCtrl::GetIconViewItemLabelClass() const
{
    return m_iconViewItemLabelClass.c_str();
}

void ListCtrl::SetListViewClass(const std::wstring& className)
{
    m_listViewClass = className;
    if (IsInited() && (m_pListView != nullptr)) {
        m_pListView->SetClass(className);
    }
}

std::wstring ListCtrl::GetListViewClass() const
{
    return m_listViewClass.c_str();
}

void ListCtrl::SetListViewItemClass(const std::wstring& className)
{
    m_listViewItemClass = className;
}

std::wstring ListCtrl::GetListViewItemClass() const
{
    return m_listViewItemClass.c_str();
}

void ListCtrl::SetListViewItemImageClass(const std::wstring& className)
{
    m_listViewItemImageClass = className;
}

std::wstring ListCtrl::GetListViewItemImageClass() const
{
    return m_listViewItemImageClass.c_str();
}

void ListCtrl::SetListViewItemLabelClass(const std::wstring& className)
{
    m_listViewItemLabelClass = className;
}

std::wstring ListCtrl::GetListViewItemLabelClass() const
{
    return m_listViewItemLabelClass.c_str();
}

void ListCtrl::SetRichEditClass(const std::wstring& richEditClass)
{
    if (m_listCtrlRichEditClass != richEditClass) {
        m_listCtrlRichEditClass = richEditClass;
        if (IsInited() && (m_pRichEdit != nullptr)) {
            m_pRichEdit->SetClass(richEditClass);
        }
    }
}

void ListCtrl::SetEnableItemEdit(bool bEnableItemEdit)
{
    m_bEnableItemEdit = bEnableItemEdit;
}

bool ListCtrl::IsEnableItemEdit() const
{
    return m_bEnableItemEdit;
}

RichEdit* ListCtrl::GetRichEdit() const
{
    return m_pRichEdit;
}

std::wstring ListCtrl::GetRichEditClass() const
{
    return m_listCtrlRichEditClass.c_str();
}

void ListCtrl::SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    m_pReportView->SetRowGridLineWidth(nLineWidth, bNeedDpiScale);
}

int32_t ListCtrl::GetRowGridLineWidth() const
{
    return m_pReportView->GetRowGridLineWidth();
}

void ListCtrl::SetRowGridLineColor(const std::wstring& color)
{
    m_pReportView->SetRowGridLineColor(color);
}

std::wstring ListCtrl::GetRowGridLineColor() const
{
    return m_pReportView->GetRowGridLineColor();
}

void ListCtrl::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    m_pReportView->SetColumnGridLineWidth(nLineWidth, bNeedDpiScale);
}

int32_t ListCtrl::GetColumnGridLineWidth() const
{
    return m_pReportView->GetColumnGridLineWidth();
}

void ListCtrl::SetColumnGridLineColor(const std::wstring& color)
{
    m_pReportView->SetColumnGridLineColor(color);
}

std::wstring ListCtrl::GetColumnGridLineColor() const
{
    return m_pReportView->GetColumnGridLineColor();
}

void ListCtrl::SetEnableColumnWidthAuto(bool bEnable)
{
    m_bEnableColumnWidthAuto = bEnable;
}

bool ListCtrl::IsEnableColumnWidthAuto() const
{
    return m_bEnableColumnWidthAuto;
}

ListCtrlHeaderItem* ListCtrl::InsertColumn(int32_t columnIndex, const ListCtrlColumn& columnInfo)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->InsertColumn(columnIndex, columnInfo);
    }
}

size_t ListCtrl::GetColumnCount() const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return 0;
    }
    else {
        return m_pHeaderCtrl->GetColumnCount();
    }
}

int32_t ListCtrl::GetColumnWidth(size_t columnIndex) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return 0;
    }
    else {
        return m_pHeaderCtrl->GetColumnWidth(columnIndex);
    }
}

bool ListCtrl::SetColumnWidth(size_t columnIndex, int32_t nWidth, bool bNeedDpiScale)
{
    bool bRet = false;
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl != nullptr) {
        bRet = m_pHeaderCtrl->SetColumnWidth(columnIndex, nWidth, bNeedDpiScale);
    }
    if(bRet) {
        OnColumnWidthChanged(GetColumnId(columnIndex), Box::InvalidIndex);
    }
    return true;
}

bool ListCtrl::SetColumnWidthAuto(size_t columnIndex)
{
    bool bRet = false;
    size_t nColumnId = GetColumnId(columnIndex);
    if (nColumnId == Box::InvalidIndex) {
        return bRet;
    }
    //计算该列的宽度
    int32_t nMaxWidth = m_pData->GetMaxColumnWidth(nColumnId);
    if (nMaxWidth > 0) {
        bRet = SetColumnWidth(columnIndex, nMaxWidth, false);
    }
    return bRet;
}

ListCtrlHeaderItem* ListCtrl::GetColumn(size_t columnIndex) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->GetColumn(columnIndex);
    }
}

ListCtrlHeaderItem* ListCtrl::GetColumnById(size_t columnId) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->GetColumnById(columnId);
    }
}

size_t ListCtrl::GetColumnIndex(size_t columnId) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return Box::InvalidIndex;
    }
    else {
        return m_pHeaderCtrl->GetColumnIndex(columnId);
    }
}

size_t ListCtrl::GetColumnId(size_t columnIndex) const
{
    ListCtrlHeaderItem* pHeaderItem = GetColumn(columnIndex);
    if (pHeaderItem != nullptr) {
        return pHeaderItem->GetColomnId();
    }
    return Box::InvalidIndex;
}

bool ListCtrl::DeleteColumn(size_t columnIndex)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return false;
    }
    else {
        return m_pHeaderCtrl->DeleteColumn(columnIndex);
    }
}

ListCtrlHeader* ListCtrl::GetHeaderCtrl() const
{
    return m_pHeaderCtrl;
}

void ListCtrl::SetEnableHeaderDragOrder(bool bEnable)
{
    m_bEnableHeaderDragOrder = bEnable;
}

bool ListCtrl::IsEnableHeaderDragOrder() const
{
    return m_bEnableHeaderDragOrder;
}

void ListCtrl::SetHeaderVisible(bool bVisible)
{
    m_bShowHeaderCtrl = bVisible;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetVisible(bVisible);
    }
    Refresh();
}

bool ListCtrl::IsHeaderVisible() const
{
    if (m_pHeaderCtrl != nullptr) {
        return m_pHeaderCtrl->IsVisible();
    }
    else if(!IsInited()){
        return m_bShowHeaderCtrl;
    }
    else {
        return false;
    }
}

void ListCtrl::SetHeaderHeight(int32_t nHeaderHeight, bool bNeedDpiScale)
{
    if (nHeaderHeight < 0) {
        nHeaderHeight = 0;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nHeaderHeight);
    }
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetFixedHeight(UiFixedInt(nHeaderHeight), true, false);
    }
    m_nHeaderHeight = nHeaderHeight;
    Refresh();
}

int32_t ListCtrl::GetHeaderHeight() const
{
    int32_t nHeaderHeight = 0;
    if (m_pHeaderCtrl != nullptr) {
        nHeaderHeight = m_pHeaderCtrl->GetFixedHeight().GetInt32();
    }
    else {
        nHeaderHeight = m_nHeaderHeight;
    }
    return nHeaderHeight;
}

void ListCtrl::SetDataItemHeight(int32_t nItemHeight, bool bNeedDpiScale)
{
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nItemHeight);
    }
    if (m_nItemHeight != nItemHeight) {
        m_nItemHeight = nItemHeight;
        //更新默认的行高
        m_pData->SetDefaultItemHeight(nItemHeight);
        Refresh();
    }
}

int32_t ListCtrl::GetDataItemHeight() const
{
    return m_nItemHeight;
}

ListCtrlItem* ListCtrl::GetFirstDisplayItem() const
{
    ListCtrlItem* pItem = nullptr;
    ASSERT(m_pReportView != nullptr);
    if (m_pReportView != nullptr) {
        size_t itemCount = m_pReportView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlItem*>(m_pReportView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlItem* ListCtrl::GetNextDisplayItem(ListCtrlItem* pItem) const
{
    ListCtrlItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayItem();
    }
    else {
        ASSERT(m_pReportView != nullptr);
        if (m_pReportView != nullptr) {
            size_t itemCount = m_pReportView->GetItemCount();
            size_t nStartIndex = m_pReportView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlItem* pCheckItem = dynamic_cast<ListCtrlItem*>(m_pReportView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

ListCtrlIconViewItem* ListCtrl::GetFirstDisplayIconItem() const
{
    ListCtrlIconViewItem* pItem = nullptr;
    ASSERT(m_pIconView != nullptr);
    if (m_pIconView != nullptr) {
        size_t itemCount = m_pIconView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlIconViewItem*>(m_pIconView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlIconViewItem* ListCtrl::GetNextDisplayIconItem(ListCtrlIconViewItem* pItem) const
{
    ListCtrlIconViewItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayIconItem();
    }
    else {
        ASSERT(m_pIconView != nullptr);
        if (m_pIconView != nullptr) {
            size_t itemCount = m_pIconView->GetItemCount();
            size_t nStartIndex = m_pIconView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlIconViewItem* pCheckItem = dynamic_cast<ListCtrlIconViewItem*>(m_pIconView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

ListCtrlListViewItem* ListCtrl::GetFirstDisplayListItem() const
{
    ListCtrlListViewItem* pItem = nullptr;
    ASSERT(m_pListView != nullptr);
    if (m_pListView != nullptr) {
        size_t itemCount = m_pListView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlListViewItem*>(m_pListView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlListViewItem* ListCtrl::GetNextDisplayListItem(ListCtrlListViewItem* pItem) const
{
    ListCtrlListViewItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayListItem();
    }
    else {
        ASSERT(m_pListView != nullptr);
        if (m_pListView != nullptr) {
            size_t itemCount = m_pListView->GetItemCount();
            size_t nStartIndex = m_pListView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlListViewItem* pCheckItem = dynamic_cast<ListCtrlListViewItem*>(m_pListView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

void ListCtrl::OnColumnWidthChanged(size_t nColumnId1, size_t nColumnId2)
{
    if ((m_pReportView == nullptr) || (m_pHeaderCtrl == nullptr)){
        return;
    }

    size_t nColumn1 = Box::InvalidIndex;
    size_t nColumn2 = Box::InvalidIndex;
    int32_t nColumnWidth1 = -1;
    int32_t nColumnWidth2 = -1;
    if (!m_pHeaderCtrl->GetColumnInfo(nColumnId1, nColumn1, nColumnWidth1)) {
        nColumnWidth1 = -1;
    }
    if (!m_pHeaderCtrl->GetColumnInfo(nColumnId2, nColumn2, nColumnWidth2)) {
        nColumnWidth2 = -1;
    }
    std::map<size_t, int32_t> subItemWidths;
    if (nColumnWidth1 >= 0) {
        subItemWidths[nColumn1] = nColumnWidth1;
    }
    if (nColumnWidth2 >= 0) {
        subItemWidths[nColumn2] = nColumnWidth2;
    }
    if (subItemWidths.empty()) {
        return;
    }
    m_pReportView->AdjustSubItemWidth(subItemWidths);
    Arrange();
}

void ListCtrl::OnHeaderColumnAdded(size_t nColumnId)
{
    m_pData->AddColumn(nColumnId);
}

void ListCtrl::OnHeaderColumnRemoved(size_t nColumnId)
{
    m_pData->RemoveColumn(nColumnId);
}

void ListCtrl::OnColumnSorted(size_t nColumnId, bool bSortedUp)
{
    //对数据排序，然后刷新界面显示
    m_pData->SortDataItems(nColumnId, GetColumnIndex(nColumnId), bSortedUp, nullptr, nullptr);
    Refresh();
}

void ListCtrl::OnHeaderColumnOrderChanged()
{
    Refresh();
}

void ListCtrl::OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked)
{
    //界面状态变化，同步到底层存储
    m_pData->SetColumnCheck(nColumnId, bChecked, true);
}

void ListCtrl::OnHeaderCheckStateChanged(bool bChecked)
{
    //界面状态变化，同步到底层存储
    m_pData->SetAllDataItemsCheck(bChecked);
}

void ListCtrl::OnHeaderColumnVisibleChanged()
{
    Refresh();
}

void ListCtrl::OnHeaderColumnSplitDoubleClick(ListCtrlHeaderItem* pHeaderItem)
{
    if (!IsEnableColumnWidthAuto()) {
        //功能关闭
        return;
    }
    //自动调整该列的宽度
    if (pHeaderItem != nullptr) {
        size_t nColumnId = pHeaderItem->GetColomnId();
        size_t nColumIndex = GetColumnIndex(nColumnId);
        if (nColumIndex < GetColumnCount()) {
            SetColumnWidthAuto(nColumIndex);
        }
    }
}

void ListCtrl::UpdateHeaderColumnCheckBox(size_t nColumnId)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return;
    }
    std::vector<size_t> columnIdList;
    if (nColumnId == Box::InvalidIndex) {
        const size_t columnCount = GetColumnCount();
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
                columnIdList.push_back(pHeaderItem->GetColomnId());
            }
        }
    }
    else {
        size_t columnIndex = GetColumnIndex(nColumnId);
        if (columnIndex != Box::InvalidIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())){
                columnIdList.push_back(nColumnId);
            }
        }
    }
    for (size_t columnId : columnIdList) {
        bool bChecked = false;
        bool bPartChecked = false;
        m_pData->GetColumnCheckStatus(columnId, bChecked, bPartChecked);
        ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumnById(columnId);
        if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
            pHeaderItem->SetCheckBoxCheck(bChecked, bPartChecked);
        }
    }
}

void ListCtrl::UpdateHeaderCheckBox()
{
    if (!IsDataItemShowCheckBox()) {
        //不显示CheckBox，忽略
        return;
    }

    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return;
    }
    bool bChecked = false;
    bool bPartChecked = false;
    m_pData->GetDataItemsCheckStatus(bChecked, bPartChecked);
    if ((m_pHeaderCtrl->IsChecked() != bChecked) || 
        (m_pHeaderCtrl->IsPartSelected() != bPartChecked)) {
        bool bOldValue = m_pHeaderCtrl->SetEnableCheckChangeEvent(false);
        m_pHeaderCtrl->SetChecked(bChecked, false);
        m_pHeaderCtrl->SetPartSelected(bPartChecked);
        m_pHeaderCtrl->SetEnableCheckChangeEvent(bOldValue);
    }
}

size_t ListCtrl::GetDataItemCount() const
{
    return m_pData->GetDataItemCount();
}

bool ListCtrl::SetDataItemCount(size_t itemCount)
{
    bool bRet = m_pData->SetDataItemCount(itemCount);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

size_t ListCtrl::AddDataItem(const ListCtrlSubItemData& dataItem)
{
    size_t columnId = GetColumnId(0);
    ASSERT(columnId != Box::InvalidIndex);
    if (columnId == Box::InvalidIndex) {
        return Box::InvalidIndex;
    }
    size_t nItemIndex = m_pData->AddDataItem(columnId, dataItem);
    if (nItemIndex != Box::InvalidIndex) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }    
    return nItemIndex;
}

bool ListCtrl::InsertDataItem(size_t itemIndex, const ListCtrlSubItemData& dataItem)
{
    size_t columnId = GetColumnId(0);
    ASSERT(columnId != Box::InvalidIndex);
    if (columnId == Box::InvalidIndex) {
        return Box::InvalidIndex;
    }
    bool bRet = m_pData->InsertDataItem(itemIndex, columnId, dataItem);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::SetSubItemData(size_t itemIndex, size_t columnIndex, const ListCtrlSubItemData& subItemData)
{
    bool bCheckChanged = false;
    bool bRet = m_pData->SetSubItemData(itemIndex, GetColumnId(columnIndex), subItemData, bCheckChanged);
    if (bCheckChanged) {
        UpdateHeaderColumnCheckBox(GetColumnId(itemIndex));
    }    
    return bRet;
}

bool ListCtrl::GetSubItemData(size_t itemIndex, size_t columnIndex, ListCtrlSubItemData& subItemData) const
{
    return m_pData->GetSubItemData(itemIndex, GetColumnId(columnIndex), subItemData);
}

bool ListCtrl::DeleteDataItem(size_t itemIndex)
{
    bool bRet = m_pData->DeleteDataItem(itemIndex);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::DeleteAllDataItems()
{
    bool bRet = m_pData->DeleteAllDataItems();
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData)
{
    bool bChanged = false;
    bool bCheckChanged = false;
    bool bRet = m_pData->SetDataItemData(itemIndex, itemData, bChanged, bCheckChanged);
    if (bRet && bCheckChanged) {
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const
{
    return m_pData->GetDataItemData(itemIndex, itemData);
}

bool ListCtrl::SetDataItemUserData(size_t itemIndex, size_t userData)
{
    return m_pData->SetDataItemUserData(itemIndex, userData);
}

size_t ListCtrl::GetDataItemUserData(size_t itemIndex) const
{
    return m_pData->GetDataItemUserData(itemIndex);
}

bool ListCtrl::SetDataItemVisible(size_t itemIndex, bool bVisible)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemVisible(itemIndex, bVisible, bChanged);
    if (bChanged) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemVisible(size_t itemIndex) const
{
    return m_pData->IsDataItemVisible(itemIndex);
}

bool ListCtrl::SetDataItemSelected(size_t itemIndex, bool bSelected)
{
    bool bChanged = false;
    bool bOldChecked = m_pData->IsDataItemChecked(itemIndex);
    bool bRet = m_pData->SetDataItemSelected(itemIndex, bSelected, bChanged);
    if (bChanged) {
        if (m_pData->IsDataItemChecked(itemIndex) != bOldChecked) {
            UpdateHeaderCheckBox();
        }
    }
    return bRet;
}

bool ListCtrl::IsDataItemSelected(size_t itemIndex) const
{
    return m_pData->IsDataItemSelected(itemIndex);
}

bool ListCtrl::SetDataItemImageId(size_t itemIndex, int32_t imageId)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemImageId(itemIndex, imageId, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemImageId(size_t itemIndex) const
{
    return m_pData->GetDataItemImageId(itemIndex);
}

bool ListCtrl::SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemAlwaysAtTop(itemIndex, nAlwaysAtTop, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int8_t ListCtrl::GetDataItemAlwaysAtTop(size_t itemIndex) const
{
    return m_pData->GetDataItemAlwaysAtTop(itemIndex);
}

bool ListCtrl::SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemHeight(itemIndex, nItemHeight, bNeedDpiScale, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemHeight(size_t itemIndex) const
{
    return m_pData->GetDataItemHeight(itemIndex);
}

bool ListCtrl::SetSubItemText(size_t itemIndex, size_t columnIndex, const std::wstring& text)
{
    return m_pData->SetSubItemText(itemIndex, GetColumnId(columnIndex), text);
}

std::wstring ListCtrl::GetSubItemText(size_t itemIndex, size_t columnIndex) const
{
    return m_pData->GetSubItemText(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::SetSubItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor)
{
    return m_pData->SetSubItemTextColor(itemIndex, GetColumnId(columnIndex), textColor);
}

UiColor ListCtrl::GetSubItemTextColor(size_t itemIndex, size_t columnIndex) const
{
    UiColor textColor;
    m_pData->GetSubItemTextColor(itemIndex, GetColumnId(columnIndex), textColor);
    return textColor;
}

bool ListCtrl::SetSubItemTextFormat(size_t itemIndex, size_t columnIndex, int32_t nTextFormat)
{
    return m_pData->SetSubItemTextFormat(itemIndex, GetColumnId(columnIndex), nTextFormat);
}

int32_t ListCtrl::GetSubItemTextFormat(size_t itemIndex, size_t columnIndex) const
{
    return m_pData->GetSubItemTextFormat(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::SetSubItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor)
{
    return m_pData->SetSubItemBkColor(itemIndex, GetColumnId(columnIndex), bkColor);
}

UiColor ListCtrl::GetSubItemBkColor(size_t itemIndex, size_t columnIndex) const
{
    UiColor bkColor;
    m_pData->GetSubItemBkColor(itemIndex, GetColumnId(columnIndex), bkColor);
    return bkColor;
}

bool ListCtrl::IsSubItemShowCheckBox(size_t itemIndex, size_t columnIndex) const
{
    return m_pData->IsSubItemShowCheckBox(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::SetSubItemShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox)
{
    return m_pData->SetSubItemShowCheckBox(itemIndex, GetColumnId(columnIndex), bShowCheckBox);
}

bool ListCtrl::SetSubItemCheck(size_t itemIndex, size_t columnIndex, bool bChecked)
{
    return m_pData->SetSubItemCheck(itemIndex, GetColumnId(columnIndex), bChecked, true);
}

bool ListCtrl::IsSubItemChecked(size_t itemIndex, size_t columnIndex) const
{
    bool bChecked = false;
    m_pData->GetSubItemCheck(itemIndex, GetColumnId(columnIndex), bChecked);
    return bChecked;
}

bool ListCtrl::SetSubItemImageId(size_t itemIndex, size_t columnIndex, int32_t imageId)
{
    return m_pData->SetSubItemImageId(itemIndex, GetColumnId(columnIndex), imageId);
}

int32_t ListCtrl::GetSubItemImageId(size_t itemIndex, size_t columnIndex) const
{
    return m_pData->GetSubItemImageId(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::SetSubItemEditable(size_t itemIndex, size_t columnIndex, bool bEditable)
{
    return m_pData->SetSubItemEditable(itemIndex, GetColumnId(columnIndex), bEditable);
}

bool ListCtrl::IsSubItemEditable(size_t itemIndex, size_t columnIndex) const
{
    return m_pData->IsSubItemEditable(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::SortDataItems(size_t columnIndex, bool bSortedUp, 
                             ListCtrlDataCompareFunc pfnCompareFunc,
                             void* pUserData)
{
    size_t nColumnId = GetColumnId(columnIndex);
    ASSERT(nColumnId != Box::InvalidIndex);
    if (nColumnId == Box::InvalidIndex) {
        return false;
    }
    return m_pData->SortDataItems(nColumnId, columnIndex, bSortedUp, pfnCompareFunc, pUserData);
}

void ListCtrl::SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    m_pData->SetSortCompareFunction(pfnCompareFunc, pUserData);
}

bool ListCtrl::IsMultiSelect() const
{
    return m_bMultiSelect;
}

void ListCtrl::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
    if (m_pReportView != nullptr) {
        m_pReportView->SetMultiSelect(bMultiSelect);
    }
    if (m_pIconView != nullptr) {
        m_pIconView->SetMultiSelect(bMultiSelect);
    }
    if (m_pListView != nullptr) {
        m_pListView->SetMultiSelect(bMultiSelect);
    }
    UpdateHeaderCheckBox();
}

void ListCtrl::SetSelectedDataItems(const std::vector<size_t>& selectedIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    m_pData->SetSelectedElements(selectedIndexs, bClearOthers, refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        UpdateHeaderCheckBox();
    }
}

void ListCtrl::GetSelectedDataItems(std::vector<size_t>& itemIndexs) const
{
    m_pData->GetSelectedElements(itemIndexs);
}

void ListCtrl::SetCheckedDataItems(const std::vector<size_t>& itemIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    m_pData->SetCheckedDataItems(itemIndexs, bClearOthers, refreshIndexs);    
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        UpdateHeaderCheckBox();
    }
}

void ListCtrl::GetCheckedDataItems(std::vector<size_t>& itemIndexs) const
{
    m_pData->GetCheckedDataItems(itemIndexs);
}

void ListCtrl::SetSelectAll()
{
    std::vector<size_t> refreshIndexs;
    m_pData->SelectAll(refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }
    }    
}

void ListCtrl::SetSelectNone()
{
    std::vector<size_t> refreshIndexs;
    m_pData->SelectNone(refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }
    }
}

void ListCtrl::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList.clear();
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            m_pReportView->GetDisplayDataItems(itemIndexList);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            m_pIconView->GetDisplayDataItems(itemIndexList);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            m_pListView->GetDisplayDataItems(itemIndexList);
        }
    }
}

size_t ListCtrl::GetTopDataItem() const
{
    size_t nTopItemIndex = Box::InvalidIndex;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            nTopItemIndex = m_pReportView->GetTopElementIndex();
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            nTopItemIndex = m_pIconView->GetTopElementIndex();
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            nTopItemIndex = m_pListView->GetTopElementIndex();
        }
    }
    return nTopItemIndex;
}

bool ListCtrl::IsDataItemDisplay(size_t itemIndex) const
{
    bool bItemVisible = false;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            bItemVisible = m_pReportView->IsDataItemDisplay(itemIndex);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            bItemVisible = m_pIconView->IsDataItemDisplay(itemIndex);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            bItemVisible = m_pListView->IsDataItemDisplay(itemIndex);
        }
    }
    return bItemVisible;
}

bool ListCtrl::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    bool bRet = false;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            bRet = m_pReportView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            bRet = m_pIconView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            bRet = m_pListView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    return bRet;
}

void ListCtrl::Refresh()
{
    if (m_bEnableRefresh) {
        if (m_listCtrlType == ListCtrlType::Report) {
            if (m_pReportView != nullptr) {
                m_pReportView->Refresh();
            }
        }
        else if (m_listCtrlType == ListCtrlType::Icon) {
            if (m_pIconView != nullptr) {
                m_pIconView->Refresh();
            }
        }
        else if (m_listCtrlType == ListCtrlType::List) {
            if (m_pListView != nullptr) {
                m_pListView->Refresh();
            }
        }
    }
}

void ListCtrl::RefreshDataItems(std::vector<size_t> dataItemIndexs)
{
    if (m_bEnableRefresh && !dataItemIndexs.empty()) {
        if (m_listCtrlType == ListCtrlType::Report) {
            if (m_pReportView != nullptr) {
                m_pReportView->RefreshElements(dataItemIndexs);
            }
        }
        else if (m_listCtrlType == ListCtrlType::Icon) {
            if (m_pIconView != nullptr) {
                m_pIconView->RefreshElements(dataItemIndexs);
            }
        }
        else if (m_listCtrlType == ListCtrlType::List) {
            if (m_pListView != nullptr) {
                m_pListView->RefreshElements(dataItemIndexs);
            }
        }        
    }
}

bool ListCtrl::SetEnableRefresh(bool bEnable)
{
    bool bOldEnable = m_bEnableRefresh;
    m_bEnableRefresh = bEnable;
    return bOldEnable;
}

bool ListCtrl::IsEnableRefresh() const
{
    return m_bEnableRefresh;
}

void ListCtrl::SetAutoCheckSelect(bool bAutoCheckSelect)
{
    m_bAutoCheckSelect = bAutoCheckSelect;
    m_pData->SetAutoCheckSelect(bAutoCheckSelect);
}

bool ListCtrl::IsAutoCheckSelect() const
{
    return m_bAutoCheckSelect;
}

bool ListCtrl::SetHeaderShowCheckBox(bool bShow)
{
    m_bHeaderShowCheckBox = bShow;
    bool bRet = false;
    if (IsInited()) {
        ListCtrlHeader* pHeaderCtrl = GetHeaderCtrl();
        ASSERT(pHeaderCtrl != nullptr);
        if (pHeaderCtrl != nullptr) {
            bRet = pHeaderCtrl->SetShowCheckBox(bShow);
        }
    }
    else {
        bRet = true;
    }
    return bRet;
}

bool ListCtrl::IsHeaderShowCheckBox() const
{
    bool bRet = false;
    if (IsInited()) {
        ListCtrlHeader* pHeaderCtrl = GetHeaderCtrl();
        ASSERT(pHeaderCtrl != nullptr);
        if (pHeaderCtrl != nullptr) {
            bRet = pHeaderCtrl->IsShowCheckBox();
        }
    }
    else {
        bRet = m_bHeaderShowCheckBox;
    }
    return bRet;
}

void ListCtrl::SetDataItemShowCheckBox(bool bShow)
{
    if (m_bDataItemShowCheckBox != bShow) {
        m_bDataItemShowCheckBox = bShow;
        if (IsInited()) {
            Refresh();
        }
    }
}

bool ListCtrl::IsDataItemShowCheckBox() const
{
    return m_bDataItemShowCheckBox;
}

bool ListCtrl::SetDataItemCheck(size_t itemIndex, bool bCheck)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemChecked(itemIndex, bCheck, bChanged);
    if (bChanged) {
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemCheck(size_t itemIndex) const
{
    return m_pData->IsDataItemChecked(itemIndex);
}

void ListCtrl::OnItemEnterEditMode(size_t itemIndex, size_t nColumnId,
                                   IListBoxItem* pItem, ListCtrlLabel* pSubItem)
{
    ASSERT(itemIndex < GetDataItemCount());
    ASSERT(GetColumnIndex(nColumnId) < GetColumnCount());
    ASSERT((pItem != nullptr) && (pSubItem != nullptr));
    ASSERT(pItem->GetElementIndex() == itemIndex);
    if (itemIndex >= GetDataItemCount()) {
        return;
    }
    if (pItem->GetElementIndex() != itemIndex) {
        return;
    }
    size_t nColumnIndex = GetColumnIndex(nColumnId);
    if (nColumnIndex >= GetColumnCount()) {
        return;
    }

    ListCtrlEditParam editParam;
    editParam.listCtrlType = m_listCtrlType;
    editParam.nItemIndex = itemIndex;
    editParam.nColumnId = nColumnId;
    editParam.nColumnIndex = nColumnIndex;
    editParam.pItem = pItem;
    editParam.pSubItem = pSubItem;

    if (!IsValidItemEditState(editParam) && !IsValidItemEditParam(editParam)) {
        ASSERT(FALSE);
        return;
    }

    std::wstring editClass = GetRichEditClass();
    ASSERT(!editClass.empty());
    if (editClass.empty()) {
        return;
    }

    //启动定时器, 只执行一次(使用定时器的原因：避免影响双击操作)
    m_editModeFlag.Cancel();
    std::function<void()> editModeCallback = std::bind(&ListCtrl::OnItemEditMode, this, editParam);
    TimerManager& timer = GlobalManager::Instance().Timer();
    timer.AddCancelableTimer(m_editModeFlag.GetWeakFlag(), editModeCallback, 600, 1);
}

bool ListCtrl::IsValidItemEditState(const ListCtrlEditParam& editParam) const
{
    if (!IsEnableItemEdit()) {
        return false;
    }
    if (editParam.listCtrlType != m_listCtrlType) {
        return false;
    }

    if (editParam.listCtrlType == ListCtrlType::Icon) {
        //Icon视图
        ListCtrlIconViewItem* pItem = dynamic_cast<ListCtrlIconViewItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pIconView != nullptr);
        if (m_pIconView == nullptr) {
            return false;
        }
    }
    else if (editParam.listCtrlType == ListCtrlType::List) {
        //List视图
        ListCtrlListViewItem* pItem = dynamic_cast<ListCtrlListViewItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pListView != nullptr);
        if (m_pListView == nullptr) {
            return false;
        }
    }
    else {
        //Report视图
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pReportView != nullptr);
        if (m_pReportView == nullptr) {
            return false;
        }
    }
    return true;
}

bool ListCtrl::IsValidItemEditParam(const ListCtrlEditParam& editParam) const
{
    if (editParam.listCtrlType != m_listCtrlType) {
        return false;
    }
    if (editParam.listCtrlType == ListCtrlType::Icon) {
        //Icon视图
        ListCtrlIconViewItem* pItem = dynamic_cast<ListCtrlIconViewItem*>(editParam.pItem);
        ASSERT(pItem != nullptr);
        if (pItem == nullptr) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;
        ListCtrlIconViewItem* pDestItem = nullptr;
        ListCtrlIconViewItem* pNextItem = GetFirstDisplayIconItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayIconItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }

        size_t nSubItemIndex = pItem->GetItemIndex(editParam.pSubItem);
        ASSERT(nSubItemIndex < pItem->GetItemCount());
        if (nSubItemIndex >= pItem->GetItemCount()) {
            return false;
        }

        size_t nColumnIndex = editParam.nColumnIndex;
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    else if (editParam.listCtrlType == ListCtrlType::List) {
        //List视图
        ListCtrlListViewItem* pItem = dynamic_cast<ListCtrlListViewItem*>(editParam.pItem);
        ASSERT(pItem != nullptr);
        if (pItem == nullptr) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;        
        ListCtrlListViewItem* pDestItem = nullptr;
        ListCtrlListViewItem* pNextItem = GetFirstDisplayListItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayListItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }

        size_t nSubItemIndex = pItem->GetItemIndex(editParam.pSubItem);
        ASSERT(nSubItemIndex < pItem->GetItemCount());
        if (nSubItemIndex >= pItem->GetItemCount()) {
            return false;
        }

        size_t nColumnIndex = editParam.nColumnIndex;
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    else {
        //Report视图
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(editParam.pItem);
        ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(editParam.pSubItem);
        ASSERT((pItem != nullptr) && (pSubItem != nullptr));
        if ((pItem == nullptr) || (pSubItem == nullptr)) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;
        size_t nColumnIndex = editParam.nColumnIndex;
        ListCtrlItem* pDestItem = nullptr;
        ListCtrlItem* pNextItem = GetFirstDisplayItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }
        if (pItem->GetSubItemIndex(pSubItem) != nColumnIndex) {
            return false;
        }
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    return true;
}

void ListCtrl::OnItemEditMode(ListCtrlEditParam editParam)
{
    std::wstring editClass = GetRichEditClass();
    ASSERT(!editClass.empty());
    if (editClass.empty()) {
        return;
    }
    if (!IsValidItemEditState(editParam) && !IsValidItemEditParam(editParam)) {
        //状态已经失效
        return;
    }
    if (m_pRichEdit == nullptr) {
        m_pRichEdit = new RichEdit;
        AddItem(m_pRichEdit);
        m_pRichEdit->SetClass(editClass);
    }
    else if(GetItemCount() > 0) {
        //将Edit控件，调整到最后
        size_t nItem = GetItemIndex(m_pRichEdit);
        size_t nLastItemIndex = GetItemCount() - 1;
        if (nItem != nLastItemIndex) {
            SetItemIndex(m_pRichEdit, nLastItemIndex);
        }
    }
    if (m_pRichEdit == nullptr) {
        return;
    }

    ListCtrlLabel* pSubItem = editParam.pSubItem;
    size_t nDataItemIndex = editParam.nItemIndex;

    std::wstring sOldItemText = pSubItem->GetText();
    UiRect rcItem = pSubItem->GetTextRect();
    UiPoint offsetPt = pSubItem->GetScrollOffsetInScrollBox();
    rcItem.Offset(-offsetPt.x, -offsetPt.y);
    UiRect rect = GetRect();
    UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
    m_pRichEdit->SetMargin(rcMargin, false);
    m_pRichEdit->SetFloat(true);
    m_pRichEdit->SetVisible(true);
    m_pRichEdit->SetText(sOldItemText);
    m_pRichEdit->SetFocus();
    m_pRichEdit->SetSelAll();
    UpdateRichEditSize(pSubItem);

    //还原焦点：将编辑框的焦点还原会原来的列表控件
    auto RestoreItemFocus = [this, nDataItemIndex]() {
            if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible() && m_pRichEdit->IsFocused()) {
                ListCtrlItem* pDestItem = nullptr;
                ListCtrlItem* pNextItem = GetFirstDisplayItem();
                while (pNextItem != nullptr) {
                    if (pNextItem->GetElementIndex() == nDataItemIndex) {
                        pDestItem = pNextItem;
                        break;
                    }
                    pNextItem = GetNextDisplayItem(pNextItem);
                }
                if (pDestItem != nullptr) {
                    pDestItem->SetFocus();
                }
            }
        };

    //触发事件：开始编辑
    ListCtrlEditParam enterEditParam = editParam;
    enterEditParam.sNewText.clear();
    enterEditParam.bCancelled = false;
    SendEvent(kEventEnterEdit, (WPARAM)&enterEditParam);
    if (enterEditParam.bCancelled || !IsValidItemEditState(editParam) && !IsValidItemEditParam(editParam)) {
        //状态已经失效, 或者用户取消编辑
        ClearEditEvents();
        RestoreItemFocus();
        LeaveEditMode();
        return;
    }

    m_pRichEdit->SetFloat(true);
    m_pRichEdit->SetVisible(true);
    m_pRichEdit->SetFocus();

    //文本变化的时候，自动调整编辑框的大小
    m_pRichEdit->DetachEvent(kEventTextChange);
    m_pRichEdit->AttachTextChange([this, pSubItem](const EventArgs&) {
        UpdateRichEditSize(pSubItem);
        return true;
        });

    //编辑结束的时候，触发事件
    auto OnLeaveRichEdit = [this, sOldItemText, RestoreItemFocus, editParam]() {
        std::wstring sNewItemText;
        if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible()) {
            sNewItemText = m_pRichEdit->GetText();
            m_pRichEdit->SetVisible(false);                     
        }
        if ((sNewItemText != sOldItemText) && IsValidItemEditParam(editParam)) {
            //文本内容发生变化
            OnItemEdited(editParam, sNewItemText);
        }
        };

    //按回车：应用修改
    m_pRichEdit->DetachEvent(kEventReturn);
    m_pRichEdit->AttachReturn([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });
    //按ESC键：取消修改
    m_pRichEdit->DetachEvent(kEventKeyDown);
    m_pRichEdit->AttachEvent(kEventKeyDown, [this, RestoreItemFocus](const EventArgs& msg) {
        if ((msg.Type == kEventKeyDown) && (msg.chKey == VK_ESCAPE)) {
            ClearEditEvents();
            RestoreItemFocus();
            return false;
        }
        else {
            return true;
        }
        });

    //控件失去焦点: 应用修改
    m_pRichEdit->DetachEvent(kEventKillFocus);
    m_pRichEdit->AttachKillFocus([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });

    //窗口失去焦点: 应用修改
    m_pRichEdit->DetachEvent(kEventWindowKillFocus);
    m_pRichEdit->AttachWindowKillFocus([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });
}

void ListCtrl::UpdateRichEditSize(ListCtrlLabel* pSubItem)
{
    if ((pSubItem == nullptr) || (m_pRichEdit == nullptr)) {
        return;
    }
    UiRect rcItem = pSubItem->GetTextRect();
    UiPoint offsetPt = pSubItem->GetScrollOffsetInScrollBox();
    rcItem.Offset(-offsetPt.x, -offsetPt.y);
    UiRect rect = GetRect();
    bool bSingleLine = pSubItem->IsSingleLine();
    if (bSingleLine) {
        if (m_pRichEdit->GetMultiLine()) {
            m_pRichEdit->SetMultiLine(false);
        }
        m_pRichEdit->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        m_pRichEdit->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        UiEstSize sz = m_pRichEdit->EstimateSize(UiSize(0, 0));
        //设置宽度
        m_pRichEdit->SetFixedWidth(UiFixedInt(sz.cx.GetInt32()), true, false);

        //设置高度
        const int32_t nEditHeight = sz.cy.GetInt32() + sz.cy.GetInt32() / 2;
        m_pRichEdit->SetFixedHeight(UiFixedInt(nEditHeight), true, false);
        m_pRichEdit->SetMinHeight(nEditHeight, false);

        //设置文本框所在位置和大小限制
        uint32_t textStyle = pSubItem->GetTextStyle();
        if (textStyle & TEXT_VCENTER) {
            //CENTER对齐
            rcItem.top = rcItem.CenterY() - nEditHeight / 2;
            rcItem.bottom = rcItem.top + nEditHeight;
        }
        else if (textStyle & TEXT_BOTTOM) {
            //BOTTOM对齐
            rcItem.top = rcItem.bottom - nEditHeight;
        }

        UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
        m_pRichEdit->SetMargin(rcMargin, false);

        m_pRichEdit->SetMinWidth(rcItem.Width(), false);
        int32_t nMaxWidth = std::max(rcItem.Width(), rect.Width() - rcMargin.left);
        m_pRichEdit->SetMaxWidth(nMaxWidth, false);
    }
    else {
        if (!m_pRichEdit->GetMultiLine()) {
            m_pRichEdit->SetMultiLine(true);
        }

        //宽度固定：与原控件相同
        m_pRichEdit->SetFixedWidth(UiFixedInt(rcItem.Width()), false, false);
        m_pRichEdit->SetMaxWidth(rcItem.Width(), false);
        m_pRichEdit->SetMinWidth(rcItem.Width(), false);
        
        //设置高度和位置
        m_pRichEdit->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);        
        UiEstSize sz = m_pRichEdit->EstimateSize(UiSize(rcItem.Width(), 0));
        m_pRichEdit->SetFixedHeight(UiFixedInt(sz.cy.GetInt32()), true, false);

        UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
        m_pRichEdit->SetMargin(rcMargin, false);

        int32_t nMaxHeight = std::max(rcItem.Height(), rect.Height() - rcMargin.top);
        m_pRichEdit->SetMaxHeight(nMaxHeight, false);
        m_pRichEdit->SetMinHeight(rcItem.Height(), false);
    }
}

void ListCtrl::OnItemEdited(const ListCtrlEditParam& editParam, const std::wstring& newItemText)
{
    //触发事件：结束编辑，如果用户取消编辑，那么不执行修改操作
    ListCtrlEditParam leaveEditParam = editParam;
    leaveEditParam.sNewText = newItemText;
    leaveEditParam.bCancelled = false;
    SendEvent(kEventLeaveEdit, (WPARAM)&leaveEditParam);
    if (!leaveEditParam.bCancelled) {
        //用户未取消编辑
        SetSubItemText(editParam.nItemIndex, editParam.nColumnIndex, newItemText);
    }    
}

void ListCtrl::OnViewMouseEvents(const EventArgs& msg)
{
    if ((msg.Type == kEventMouseWheel) ||
        (msg.Type == kEventMouseButtonDown) ||
        (msg.Type == kEventMouseButtonUp) ||
        (msg.Type == kEventMouseDoubleClick) ||
        (msg.Type == kEventMouseRButtonDown) ||
        (msg.Type == kEventMouseRButtonUp) ||
        (msg.Type == kEventMouseRDoubleClick) ||
        (msg.Type == kEventMouseMenu)) {
        if (msg.pSender != m_pRichEdit) {
            LeaveEditMode();
        }
    }    
}

void ListCtrl::OnViewKeyboardEvents(const EventArgs& msg)
{
    if (msg.pSender != m_pRichEdit) {
        LeaveEditMode();
    }    
}

void ListCtrl::LeaveEditMode()
{
    //取消编辑状态的定时器
    m_editModeFlag.Cancel();
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible() && m_pRichEdit->IsFocused()) {
        m_pRichEdit->SendEvent(kEventKillFocus);
    }
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible()) {
        m_pRichEdit->SetVisible(false);
    }
    ClearEditEvents();
}

void ListCtrl::ClearEditEvents()
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->DetachEvent(kEventWindowKillFocus);
        m_pRichEdit->DetachEvent(kEventKillFocus);
        m_pRichEdit->DetachEvent(kEventReturn);
        m_pRichEdit->DetachEvent(kEventTextChange);
        m_pRichEdit->DetachEvent(kEventKeyDown);
    }
}

}//namespace ui

