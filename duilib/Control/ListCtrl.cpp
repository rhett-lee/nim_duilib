#include "ListCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Split.h"

namespace ui
{

/** 列表项的数据管理器
*/
class ListCtrlItemProvider : public ui::VirtualListBoxElement
{
public:
    ListCtrlItemProvider()
    {
        for (int i = 0; i < 1000; ++i) {
            m_listItems.push_back(i);
        }
    }

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateElement() override
    {
        ListCtrlItem* pItem = new ListCtrlItem;        
        return pItem;
    }

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override
    {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(pControl);
        if (pItem == nullptr) {
            return false;
        }
        pItem->SetAttribute(L"border_size", L"2");
        pItem->SetAttribute(L"border_color", L"orange");
        pItem->SetAttribute(L"text_align", L"vcenter,hcenter");
        pItem->SetBkColor(nElementIndex % 2 ? L"blue" : L"gray");
        pItem->SetText(StringHelper::Printf((L"%d"), (int32_t)nElementIndex));
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

private:

    /** 列表项数据
    */
    std::vector<int> m_listItems;
};

/** 列表数据显示控件
*/
class ListCtrlData : public VirtualVTileListBox
{
public:
    ListCtrlData() {}
    virtual ~ListCtrlData() {}
};

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
    if (strName == L"") {
        
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrl::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;

    //初始化Header
    ASSERT(m_pListCtrlHeader == nullptr);
    m_pListCtrlHeader = new ListCtrlHeader;
    //AddItem(m_pListCtrlHeader);

    for (size_t i = 0; i < 6; ++i) {
        ListCtrlHeaderItem* pHeaderItem = new ListCtrlHeaderItem;
        Label* pLabel = new Label;
        pLabel->SetText(L"111");
        pLabel->SetBkColor(L"Yellow");
        pLabel->SetAttribute(L"width", L"100%");
        pLabel->SetAttribute(L"height", L"32");
        pLabel->SetAttribute(L"text_align", L"vcenter,hcenter");
        pHeaderItem->AddItem(pLabel);

        if (i != 0) {
            Split* pSplit = new Split;
            pSplit->SetAttribute(L"width", L"2");
            pSplit->SetAttribute(L"height", L"32");
            pSplit->SetBkColor(L"splitline_level1");
            m_pListCtrlHeader->AddItem(pSplit);
        }
        pHeaderItem->SetAttribute(L"width", L"200");
        m_pListCtrlHeader->AddItem(pHeaderItem);
    }

    //最后加一个空的
    Split* pSplit = new Split;
    pSplit->SetAttribute(L"width", L"2");
    pSplit->SetAttribute(L"height", L"32");
    pSplit->SetBkColor(L"splitline_level1");
    m_pListCtrlHeader->AddItem(pSplit);
    ListCtrlHeaderItem* pHeaderItem = new ListCtrlHeaderItem;
    pHeaderItem->SetAttribute(L"width", L"100%");
    pHeaderItem->SetAttribute(L"height", L"32");
    pHeaderItem->SetBkColor(L"Yellow");

    m_pListCtrlHeader->SetAttribute(L"width", L"1000");
    m_pListCtrlHeader->SetAttribute(L"height", L"32");
    m_pListCtrlHeader->AddItem(pHeaderItem);

    //初始化Body
    ASSERT(m_pListCtrlData == nullptr);
    m_pListCtrlData = new ListCtrlData;
    //m_pListCtrlData->SetBkColor(L"red");
    m_pListCtrlData->SetAttribute(L"item_size", L"1100,80");
    m_pListCtrlData->SetAttribute(L"columns", L"1");
    m_pListCtrlData->SetAttribute(L"vscrollbar", L"true");
    m_pListCtrlData->SetAttribute(L"hscrollbar", L"true");
    m_pListCtrlData->SetAttribute(L"width", L"1000");
    m_pListCtrlData->SetAttribute(L"padding", L"0, 50, 0, 0");
    m_pListCtrlData->SetDataProvider(m_spItemProvider.get());

    m_pListCtrlHeader->SetFloat(true);
    //m_pListCtrlData->AddItem(m_pListCtrlHeader);

    
    AddItem(m_pListCtrlData);
    AddItem(m_pListCtrlHeader);
}

}//namespace ui

