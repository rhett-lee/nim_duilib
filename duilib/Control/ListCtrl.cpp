#include "ListCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{
////////////////////////////////////////////////////////
//ListCtrlHeaderItem

ListCtrlHeaderItem::ListCtrlHeaderItem() :
    m_pSortedDownImage(nullptr),
    m_pSortedUpImage(nullptr),
    m_sortMode(SortMode::kDown),
    m_pSplitBox(nullptr),
    m_bColumnResizeable(true),
    m_nColumnWidth(0)
{
    m_nIconSpacing = GlobalManager::Instance().Dpi().GetScaleInt(6);
}

ListCtrlHeaderItem::~ListCtrlHeaderItem()
{
    if (m_pSortedDownImage != nullptr) {
        delete m_pSortedDownImage;
        m_pSortedDownImage = nullptr;
    }
    if (m_pSortedUpImage != nullptr) {
        delete m_pSortedUpImage;
        m_pSortedUpImage = nullptr;
    }
}
 
std::wstring ListCtrlHeaderItem::GetType() const { return L"ListCtrlHeaderItem"; }

void ListCtrlHeaderItem::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"sorted_up_image") {
        SetSortedUpImage(strValue);
    }
    else if (strName == L"sorted_down_image") {
        SetSortedDownImage(strValue);
    }
    else if (strName == L"icon_spacing") {
        SetIconSpacing(_wtoi(strValue.c_str()), true);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrlHeaderItem::PaintText(IRender* pRender)
{
    __super::PaintText(pRender);
    if (pRender == nullptr) {
        return;
    }

    Image* pImage = nullptr;
    if (m_sortMode == SortMode::kUp) {
        //升序
        pImage = m_pSortedUpImage;
    }
    else if (m_sortMode == SortMode::kDown) {
        //降序
        pImage = m_pSortedDownImage;
    }
    if (pImage == nullptr) {
        return;
    }
    
    int32_t nIconTextSpacing = m_nIconSpacing;
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    rc.Deflate(GetTextPadding());
    uint32_t textStyle = GetTextStyle();
    std::wstring textValue = GetText();
    UiRect textRect = pRender->MeasureString(textValue, GetFontId(), textStyle);
    if (textStyle & TEXT_CENTER) {
        rc.left = rc.CenterX() + textRect.Width() / 2;
        rc.left += nIconTextSpacing;
    }
    else if (textStyle & TEXT_RIGHT) {
        rc.left = rc.right - textRect.Width() - nIconTextSpacing;
        if (pImage != nullptr) {
            if (pImage->GetImageCache() == nullptr) {
                LoadImageData(*pImage);
            }
            if (pImage->GetImageCache() != nullptr) {
                rc.left -= pImage->GetImageCache()->GetWidth();
            }
        }
    }
    else {
        rc.left += textRect.Width();
        rc.left += nIconTextSpacing;
    }
    rc.Validate();

    //绘制排序图标
    PaintImage(pRender, pImage, L"", -1, nullptr, &rc, nullptr);
}

void ListCtrlHeaderItem::Activate()
{
    if (!this->IsActivatable()) {
        return;
    }
    if (m_sortMode == SortMode::kUp) {
        m_sortMode = SortMode::kDown;
        Invalidate();
    }
    else if (m_sortMode == SortMode::kDown) {
        m_sortMode = SortMode::kUp;
        Invalidate();
    }
    __super::Activate();
}

void ListCtrlHeaderItem::SetSortMode(SortMode sortMode)
{
    if (m_sortMode != sortMode) {
        m_sortMode = sortMode;
        Invalidate();
    }
}

ListCtrlHeaderItem::SortMode ListCtrlHeaderItem::GetSortMode() const
{
    return m_sortMode;
}

void ListCtrlHeaderItem::SetSortedDownImage(const std::wstring& sImageString)
{
    if (m_pSortedDownImage == nullptr) {
        m_pSortedDownImage = new Image;
    }
    m_pSortedDownImage->SetImageString(sImageString);
    Invalidate();
}

void ListCtrlHeaderItem::SetSortedUpImage(const std::wstring& sImageString)
{
    if (m_pSortedUpImage == nullptr) {
        m_pSortedUpImage = new Image;
    }
    m_pSortedUpImage->SetImageString(sImageString);
    Invalidate();
}

size_t ListCtrlHeaderItem::GetColomnId() const
{
    return (size_t)this;
}

void ListCtrlHeaderItem::SetSplitBox(SplitBox* pSplitBox)
{
    m_pSplitBox = pSplitBox;
    if (pSplitBox != nullptr) {
        ASSERT(pSplitBox->GetFixedWidth().IsInt32());
        pSplitBox->SetEnabled(IsColumnResizeable() ? true : false);
    }
    if (GetColumnWidth() > 0) {
        CheckColumnWidth();
    }
}

SplitBox* ListCtrlHeaderItem::GetSplitBox() const
{
    return m_pSplitBox;
}

void ListCtrlHeaderItem::SetColumnResizeable(bool bResizeable)
{
    m_bColumnResizeable = bResizeable;
    if (m_pSplitBox != nullptr) {
        m_pSplitBox->SetEnabled(IsColumnResizeable() ? true : false);
    }
}

bool ListCtrlHeaderItem::IsColumnResizeable() const
{
    return m_bColumnResizeable;
}

void ListCtrlHeaderItem::SetColumnWidth(int32_t nWidth, bool bNeedDpiScale)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nWidth);
    }
    m_nColumnWidth = nWidth;
    CheckColumnWidth();
}

int32_t ListCtrlHeaderItem::GetColumnWidth() const
{
    return m_nColumnWidth;
}

void ListCtrlHeaderItem::CheckColumnWidth()
{
    int32_t nSplitWidth = 0;
    if (m_pSplitBox != nullptr) {
        ASSERT(m_pSplitBox->GetFixedWidth().IsInt32());
        nSplitWidth = m_pSplitBox->GetFixedWidth().GetInt32();
    }
    int32_t nWidth = GetFixedWidth().GetInt32();
    if ((nWidth + nSplitWidth) != GetColumnWidth()) {
        nWidth = GetColumnWidth() - nSplitWidth;
        if (nWidth < 0) {
            nWidth = 0;
        }
        SetFixedWidth(UiFixedInt(nWidth), true, false);
    }
}

void ListCtrlHeaderItem::SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nIconSpacing);
    }
    m_nIconSpacing = nIconSpacing;
    if (m_nIconSpacing < 0) {
        m_nIconSpacing = 0;
    }
}

int32_t ListCtrlHeaderItem::GetIconSpacing() const
{
    return m_nIconSpacing;
}

////////////////////////////////////////////////////////////////
/** ListCtrl的表头控件
*/
ListCtrlHeader::ListCtrlHeader() :
    m_pListCtrl(nullptr)
{
}

std::wstring ListCtrlHeader::GetType() const { return L"ListCtrlHeader"; }

ListCtrlHeaderItem* ListCtrlHeader::InsertColumn(int32_t nCol, int32_t nColumnWidth, 
                                                 const std::wstring& text,
                                                 bool bSortable, bool bResizeable, 
                                                 bool bNeedDpiScale)
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nullptr;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nColumnWidth);
    }
    if (nColumnWidth < 0) {
        nColumnWidth = 0;
    }

    ListCtrlHeaderItem* pHeaderItem = new ListCtrlHeaderItem;
    SplitBox* pHeaderSplit = new SplitBox;
    size_t nColumnCount = GetColumnCount();
    if ((size_t)nCol >= nColumnCount) {
        //放在最后
        AddItem(pHeaderItem);
        AddItem(pHeaderSplit);
    }
    else {
        //插入在中间位置        
        AddItemAt(pHeaderSplit, nCol);
        AddItemAt(pHeaderItem, nCol);
    }

    //设置属性
    if (!m_pListCtrl->GetListCtrlHeaderSplitBoxClass().empty()) {
        pHeaderSplit->SetClass(m_pListCtrl->GetListCtrlHeaderSplitBoxClass());
    }

    Control* pSplitCtrl = new Control;
    pSplitCtrl->SetMouseEnabled(false);
    pSplitCtrl->SetMouseFocused(false);
    pSplitCtrl->SetNoFocus();
    if (!m_pListCtrl->GetListCtrlHeaderSplitControlClass().empty()) {
        pSplitCtrl->SetClass(m_pListCtrl->GetListCtrlHeaderSplitControlClass());
    }
    pHeaderSplit->AddItem(pSplitCtrl);

    if (!m_pListCtrl->GetListCtrlHeaderItemClass().empty()) {
        pHeaderItem->SetClass(m_pListCtrl->GetListCtrlHeaderItemClass());
    }
    pHeaderItem->SetText(text);

    //保存关联的Split控件接口
    pHeaderItem->SetSplitBox(pHeaderSplit);
    pHeaderItem->SetColumnWidth(nColumnWidth, false);

    if (bSortable) {
        pHeaderItem->SetSortMode(ListCtrlHeaderItem::SortMode::kUp);
    }
    else {
        pHeaderItem->SetSortMode(ListCtrlHeaderItem::SortMode::kNone);
    }
    pHeaderItem->SetColumnResizeable(bResizeable);

    //挂载排序事件
    pHeaderItem->AttachClick([this, pHeaderItem](const EventArgs& /*args*/) {
        OnHeaderColumnSorted(pHeaderItem);
        return true;
        });

        //挂载拖动响应事件
    pHeaderSplit->AttachSplitDraged([this](const EventArgs& args) {
        OnHeaderColumnResized((Control*)args.wParam, (Control*)args.lParam);
        return true;
        });

    return pHeaderItem;
}

size_t ListCtrlHeader::GetColumnCount() const
{
    size_t nItemCount = GetItemCount();
    if (nItemCount == 0) {
        return 0;
    }
    ASSERT((nItemCount % 2) == 0);
    if ((nItemCount % 2) != 0) {
        return 0;
    }
    const size_t nColumnCount = nItemCount / 2;
#ifdef _DEBUG
    //校验结构是否符合预期    
    for (size_t index = 0; index < nColumnCount; ++index) {
        ASSERT(dynamic_cast<ListCtrlHeaderItem*>(GetItemAt(index * 2)) != nullptr);
        ASSERT(dynamic_cast<SplitBox*>(GetItemAt(index * 2 + 1)) != nullptr);
    }
#endif // _DEBUG  
    
    return nColumnCount;
}

int32_t ListCtrlHeader::GetColumnWidth(size_t columnIndex) const
{
    int32_t nColumnWidth = 0;
    size_t nColumnCount = GetColumnCount();
    ASSERT(columnIndex < nColumnCount);
    if (columnIndex >= nColumnCount) {
        return nColumnWidth;
    }
    ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(GetItemAt(columnIndex * 2));
    ASSERT(pHeaderItem != nullptr);
    if (pHeaderItem != nullptr) {
        nColumnWidth = pHeaderItem->GetColumnWidth();
    }
    return nColumnWidth;
}

ListCtrlHeaderItem* ListCtrlHeader::GetColumnItem(size_t columnIndex) const
{
    int32_t nColumnWidth = 0;
    size_t nColumnCount = GetColumnCount();
    ASSERT(columnIndex < nColumnCount);
    if (columnIndex >= nColumnCount) {
        return nullptr;
    }
    ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(GetItemAt(columnIndex * 2));
    ASSERT(pHeaderItem != nullptr);
    return pHeaderItem;
}

ListCtrlHeaderItem* ListCtrlHeader::GetColumnItemById(size_t columnId) const
{
    ListCtrlHeaderItem* pFoundHeaderItem = nullptr;
    size_t nColumnCount = GetColumnCount();
    for (size_t index = 0; index < nColumnCount; ++index) {
        ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(GetItemAt(index * 2));
        ASSERT(pHeaderItem != nullptr);
        if (pHeaderItem != nullptr) {
            if (pHeaderItem->GetColomnId() == columnId) {
                pFoundHeaderItem = pHeaderItem;
                break;
            }
        }
    }
    return pFoundHeaderItem;
}

bool ListCtrlHeader::GetColumnInfo(size_t columnId, size_t& columnIndex, int32_t& nColumnWidth) const
{
    bool bRet = false;
    columnIndex = Box::InvalidIndex;
    nColumnWidth = -1;
    size_t nColumnCount = GetColumnCount();
    for (size_t index = 0; index < nColumnCount; ++index) {
        ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(GetItemAt(index * 2));
        ASSERT(pHeaderItem != nullptr);
        if (pHeaderItem != nullptr) {
            if (pHeaderItem->GetColomnId() == columnId) {
                nColumnWidth = pHeaderItem->GetColumnWidth();
                columnIndex = index;
                bRet = true;
                break;
            }
        }
    }
    return bRet;
}

void ListCtrlHeader::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

void ListCtrlHeader::OnHeaderColumnResized(Control* pLeftHeaderItem, Control* pRightHeaderItem)
{
    size_t nColumnId1 = Box::InvalidIndex;
    size_t nColumnId2 = Box::InvalidIndex;
    ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(pLeftHeaderItem);
    if (pHeaderItem != nullptr) {
        int32_t nSplitWidth = 0;
        if (pHeaderItem->GetSplitBox() != nullptr) {
            nSplitWidth = pHeaderItem->GetSplitBox()->GetFixedWidth().GetInt32();
        }
        int32_t nItemWidth = pHeaderItem->GetFixedWidth().GetInt32();
        int32_t nColumnWidth = nItemWidth + nSplitWidth;
        pHeaderItem->SetColumnWidth(nColumnWidth, false);
        nColumnId1 = pHeaderItem->GetColomnId();
    }

    pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(pRightHeaderItem);
    if (pHeaderItem != nullptr) {
        int32_t nSplitWidth = 0;
        if (pHeaderItem->GetSplitBox() != nullptr) {
            nSplitWidth = pHeaderItem->GetSplitBox()->GetFixedWidth().GetInt32();
        }
        int32_t nItemWidth = pHeaderItem->GetFixedWidth().GetInt32();
        int32_t nColumnWidth = nItemWidth + nSplitWidth;
        pHeaderItem->SetColumnWidth(nColumnWidth, false);
        nColumnId2 = pHeaderItem->GetColomnId();
    }

    if ((nColumnId1 != Box::InvalidIndex) || (nColumnId2 != Box::InvalidIndex)) {
        if (m_pListCtrl != nullptr) {
            m_pListCtrl->OnColumnWidthChanged(nColumnId1, nColumnId2);
        }
    }
}

void ListCtrlHeader::OnHeaderColumnSorted(ListCtrlHeaderItem* pHeaderItem)
{
    if (pHeaderItem == nullptr) {
        return;
    }
    size_t nColumnId = pHeaderItem->GetColomnId();
    ListCtrlHeaderItem::SortMode sortMode = pHeaderItem->GetSortMode();
    ASSERT(sortMode != ListCtrlHeaderItem::SortMode::kNone);
    if (sortMode == ListCtrlHeaderItem::SortMode::kNone) {
        return;
    }
    if (m_pListCtrl != nullptr) {
        bool bSortedUp = (sortMode == ListCtrlHeaderItem::SortMode::kUp) ? true : false;
        m_pListCtrl->OnColumnSorted(nColumnId, bSortedUp);
    }
}

/////////////////////////////////////////////////////////////////
/** 列表项的数据管理器
*/
class ListCtrlItemProvider : public ui::VirtualListBoxElement
{
public:
    ListCtrlItemProvider():
        m_pListCtrlHeader(nullptr)
    {
        for (int i = 0; i < 10; ++i) {
            m_listItems.push_back(2);
        }
    }

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateElement() override
    {
        ListCtrlItem* pItem = new ListCtrlItem;
        pItem->SetClass(L"list_ctrl_item");
        return pItem;
    }

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override
    {
        if (nElementIndex == 0) {
            return true;
        }
        bool bFirstLine = (nElementIndex == 1);
        //return true;
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(pControl);
        if (pItem == nullptr) {
            return false;
        }
        //pItem->SetAttribute(L"border_size", L"1");
        //pItem->SetAttribute(L"border_color", L"orange");

        //基本结构：
        //  <HBox>
        //      <Label/>
        //      ...
        //  </HBox>

        HBox* pItemHBox = nullptr;
        if (pItem->GetItemCount() > 0) {
            pItemHBox = dynamic_cast<HBox*>(pItem->GetItemAt(0));
            ASSERT(pItemHBox != nullptr);
            if (pItemHBox == nullptr) {
                return false;
            }
        }
        if (pItemHBox == nullptr) {
            pItemHBox = new HBox;
            pItem->AddItem(pItemHBox);
        }

        if (m_pListCtrlHeader != nullptr) {
            size_t columnCount = m_pListCtrlHeader->GetColumnCount();
            for (size_t nColumn = 0; nColumn < columnCount; ++nColumn) {
                Label* pLabel = nullptr;
                if (nColumn < pItemHBox->GetItemCount()) {
                    pLabel = dynamic_cast<Label*>(pItemHBox->GetItemAt(nColumn));
                    ASSERT(pLabel != nullptr);
                    if (pLabel == nullptr) {
                        return false;
                    }
                }
                else {
                    pLabel = new Label;
                    pItemHBox->AddItem(pLabel);
                }

                int32_t width = m_pListCtrlHeader->GetColumnWidth(nColumn);

                pLabel->SetFixedWidth(UiFixedInt(width), true, false);
                pLabel->SetAttribute(L"height", L"100%");
                pLabel->SetAttribute(L"text_align", L"vcenter,hcenter");
                pLabel->SetBkColor((nElementIndex % 2) ? L"#10FF22FF" : L"#1022FFFF");
                pLabel->SetText(StringHelper::Printf((L"%d,%d"), (int32_t)nElementIndex, (int32_t)nColumn));

                //绘制边线
                bool bShowColumnLine = true; //是否显示纵向的边线
                bool bShowRowLine = true;    //是否显示横向的边线
                int32_t mColumnLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);//纵向边线宽度
                int32_t mRowLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);   //横向边线宽度
                if (!bShowColumnLine) {
                    mColumnLineWidth = 0;
                }
                if (!bShowRowLine) {
                    mRowLineWidth = 0;
                }
                if (bFirstLine) {
                    //第一行
                    if (nColumn == 0) {
                        //第一列
                        UiRect rc(mColumnLineWidth, mRowLineWidth, mColumnLineWidth, mRowLineWidth);
                        pLabel->SetBorderSize(rc, false);
                    }
                    else {
                        //非第一列
                        UiRect rc(0, mRowLineWidth, mColumnLineWidth, mRowLineWidth);
                        pLabel->SetBorderSize(rc, false);
                    }
                }
                else {
                    //非第一行
                    if (nColumn == 0) {
                        //第一列
                        UiRect rc(mColumnLineWidth, 0, mColumnLineWidth, mRowLineWidth);
                        pLabel->SetBorderSize(rc, false);
                    }
                    else {
                        //非第一列
                        UiRect rc(0, 0, mColumnLineWidth, mRowLineWidth);
                        pLabel->SetBorderSize(rc, false);
                    }
                }
                pLabel->SetAttribute(L"border_color", L"orange");
            }
        }

        return true;
    }

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() override
    {
        return m_listItems.size();
    }

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override
    {

    }

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) override
    {
        return false;
    }

public:

    /** 设置表头接口
    */
    void SetListCtrlHeader(ListCtrlHeader* pListCtrlHeader)
    {
        m_pListCtrlHeader = pListCtrlHeader;
    }

private:
    /** 列表项数据
    */
    std::vector<int> m_listItems;

private:
    /** 表头控件
    */
    ListCtrlHeader* m_pListCtrlHeader;
};

/** 列表数据显示控件
*/
class ListCtrlData : public VirtualListBox
{
    friend class ListCtrlDataLayout;
public:
    ListCtrlData();
    virtual ~ListCtrlData();

public:
    /** 设置表头接口
    */
    void SetListCtrlHeader(ListCtrlHeader* pListCtrlHeader);

private:
    /** 表头控件
    */
    ListCtrlHeader* m_pListCtrlHeader;
};

/** 列表数据显示控件的布局管理接口
*/
class ListCtrlDataLayout: public VirtualVTileLayout
{
public:    
};

/** 列表数据显示控件
*/
ListCtrlData::ListCtrlData() :
    VirtualListBox(new ListCtrlDataLayout),
    m_pListCtrlHeader(nullptr)
{
    VirtualLayout* pVirtualLayout = dynamic_cast<VirtualLayout*>(GetLayout());
    SetVirtualLayout(pVirtualLayout);
}

ListCtrlData::~ListCtrlData() {}

void ListCtrlData::SetListCtrlHeader(ListCtrlHeader* pListCtrlHeader)
{
    m_pListCtrlHeader = pListCtrlHeader;
}

ListCtrl::ListCtrl():
    m_bInited(false),
    m_pListCtrlHeader(nullptr),
    m_pListCtrlData(nullptr)
{
    m_spItemProvider = std::make_unique<ListCtrlItemProvider>();
}

ListCtrl::~ListCtrl()
{
}

std::wstring ListCtrl::GetType() const { return DUI_CTR_LISTCTRL; }

void ListCtrl::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"list_ctrl_header_class") {
        SetListCtrlHeaderClass(strValue);
    }
    else if (strName == L"list_ctrl_header_item_class") {
        SetListCtrlHeaderItemClass(strValue);
    }
    else if (strName == L"list_ctrl_header_split_box_class") {
        SetListCtrlHeaderSplitBoxClass(strValue);
    }
    else if (strName == L"list_ctrl_header_split_control_class") {
        SetListCtrlHeaderSplitControlClass(strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrl::SetListCtrlHeaderClass(const std::wstring& className)
{
    m_headerClass = className;
    if (m_pListCtrlHeader != nullptr) {
        m_pListCtrlHeader->SetClass(className);
    }
}

void ListCtrl::SetListCtrlHeaderItemClass(const std::wstring& className)
{
    m_headerItemClass = className;
}

std::wstring ListCtrl::GetListCtrlHeaderItemClass() const
{
    return m_headerItemClass.c_str();
}

void ListCtrl::SetListCtrlHeaderSplitBoxClass(const std::wstring& className)
{
    m_headerSplitBoxClass = className;
}

std::wstring ListCtrl::GetListCtrlHeaderSplitBoxClass() const
{
    return m_headerSplitBoxClass.c_str();
}

void ListCtrl::SetListCtrlHeaderSplitControlClass(const std::wstring& className)
{
    m_headerSplitControlClass = className;
}

std::wstring ListCtrl::GetListCtrlHeaderSplitControlClass() const
{
    return m_headerSplitControlClass.c_str();
}

void ListCtrl::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;

    //初始化Header
    ASSERT(m_pListCtrlHeader == nullptr);
    if (m_pListCtrlHeader == nullptr) {
        m_pListCtrlHeader = new ListCtrlHeader;
    }
    m_pListCtrlHeader->SetListCtrl(this);

    if (!m_headerClass.empty()) {
        m_pListCtrlHeader->SetClass(m_headerClass.c_str());
    }

    //初始化Body
    ASSERT(m_pListCtrlData == nullptr);
    m_pListCtrlData = new ListCtrlData;
    m_pListCtrlData->SetBkColor(L"white");
    m_pListCtrlData->SetAttribute(L"item_size", L"1200,32");
    m_pListCtrlData->SetAttribute(L"columns", L"1");
    m_pListCtrlData->SetAttribute(L"vscrollbar", L"true");
    m_pListCtrlData->SetAttribute(L"hscrollbar", L"true");
    m_pListCtrlData->SetAttribute(L"width", L"1200");
    m_pListCtrlData->SetDataProvider(m_spItemProvider.get());

    m_spItemProvider->SetListCtrlHeader(m_pListCtrlHeader);
    m_pListCtrlData->SetListCtrlHeader(m_pListCtrlHeader);

    m_pListCtrlData->AddItem(m_pListCtrlHeader);
    AddItem(m_pListCtrlData);

    //TEST
    int32_t width = ui::GlobalManager::Instance().Dpi().GetScaleInt(200);
    m_pListCtrlHeader->InsertColumn(-1, width, L"1111", true, true, false);
    m_pListCtrlHeader->InsertColumn(-1, width, L"2222", true, true, false);
    m_pListCtrlHeader->InsertColumn(-1, width, L"3333", true, true, false);
    m_pListCtrlHeader->InsertColumn(-1, width, L"4444", true, true, false);
    m_pListCtrlHeader->InsertColumn(-1, width, L"5555", true, true, false);
    //TESTs
}

ListCtrlHeaderItem* ListCtrl::InsertColumn(int32_t nCol, int32_t nColumnWidth, const std::wstring& text,
                                           bool bSortable, bool bResizeable, bool bNeedDpiScale)
{
    ASSERT(m_pListCtrlHeader != nullptr);
    if (m_pListCtrlHeader == nullptr) {
        return nullptr;
    }
    else {
        return m_pListCtrlHeader->InsertColumn(nCol, nColumnWidth, text, bSortable, bResizeable, bNeedDpiScale);
    }
}

size_t ListCtrl::GetColumnCount() const
{
    ASSERT(m_pListCtrlHeader != nullptr);
    if (m_pListCtrlHeader == nullptr) {
        return 0;
    }
    else {
        return m_pListCtrlHeader->GetColumnCount();
    }
}

int32_t ListCtrl::GetColumnWidth(size_t columnIndex) const
{
    ASSERT(m_pListCtrlHeader != nullptr);
    if (m_pListCtrlHeader == nullptr) {
        return 0;
    }
    else {
        return m_pListCtrlHeader->GetColumnWidth(columnIndex);
    }
}

ListCtrlHeaderItem* ListCtrl::GetColumnItem(size_t columnIndex) const
{
    ASSERT(m_pListCtrlHeader != nullptr);
    if (m_pListCtrlHeader == nullptr) {
        return nullptr;
    }
    else {
        return m_pListCtrlHeader->GetColumnItem(columnIndex);
    }
}

ListCtrlHeaderItem* ListCtrl::GetColumnItemById(size_t columnId) const
{
    ASSERT(m_pListCtrlHeader != nullptr);
    if (m_pListCtrlHeader == nullptr) {
        return nullptr;
    }
    else {
        return m_pListCtrlHeader->GetColumnItemById(columnId);
    }
}

ListCtrlHeader* ListCtrl::GetListCtrlHeader() const
{
    return m_pListCtrlHeader;
}

void ListCtrl::OnColumnWidthChanged(size_t nColumnId1, size_t nColumnId2)
{
    if ((m_pListCtrlData == nullptr) || (m_pListCtrlHeader == nullptr)){
        return;
    }

    size_t nColumn1 = Box::InvalidIndex;
    size_t nColumn2 = Box::InvalidIndex;
    int32_t nColumnWidth1 = -1;
    int32_t nColumnWidth2 = -1;
    if (!m_pListCtrlHeader->GetColumnInfo(nColumnId1, nColumn1, nColumnWidth1)) {
        nColumnWidth1 = -1;
    }
    if (!m_pListCtrlHeader->GetColumnInfo(nColumnId2, nColumn2, nColumnWidth2)) {
        nColumnWidth2 = -1;
    }

    if ((nColumnWidth1 < 0) && (nColumnWidth2 < 0)) {
        return;
    }

    size_t itemCount = m_pListCtrlData->GetItemCount();
    for (size_t index = 1; index < itemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(m_pListCtrlData->GetItemAt(index));
        if (pItem == nullptr) {
            continue;
        }
        HBox* pItemHBox = nullptr;
        if (pItem->GetItemCount() > 0) {
            pItemHBox = dynamic_cast<HBox*>(pItem->GetItemAt(0));
            ASSERT(pItemHBox != nullptr);            
        }
        if (pItemHBox == nullptr) {
            continue;
        }

        size_t columnCount = pItemHBox->GetItemCount();
        for (size_t nColumn = 0; nColumn < columnCount; ++nColumn) {
            if ((nColumn == nColumn1) && (nColumnWidth1 >= 0)) {
                Label* pLabel = dynamic_cast<Label*>(pItemHBox->GetItemAt(nColumn));
                if (pLabel != nullptr) {
                    pLabel->SetFixedWidth(UiFixedInt(nColumnWidth1), true, false);
                }
            }
            if ((nColumn == nColumn2) && (nColumnWidth2 >= 0)) {
                Label* pLabel = dynamic_cast<Label*>(pItemHBox->GetItemAt(nColumn));
                if (pLabel != nullptr) {
                    pLabel->SetFixedWidth(UiFixedInt(nColumnWidth2), true, false);
                }
            }
        }
    }
}

void ListCtrl::OnColumnSorted(size_t nColumnId, bool bSortedUp)
{

}

}//namespace ui

