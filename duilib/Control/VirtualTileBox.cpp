#include "VirtualTileBox.h"
#include "duilib/Control/ScrollBar.h"
#include <algorithm>

namespace ui {

VirtualTileInterface::VirtualTileInterface()
  :m_CountChangedNotify()
  , m_DataChangedNotify()
{

}


void VirtualTileInterface::RegNotifys(const DataChangedNotify& dcNotify, const CountChangedNotify& ccNotify)
{
  m_DataChangedNotify = dcNotify;
  m_CountChangedNotify = ccNotify;
}



void VirtualTileInterface::EmitDataChanged(int nStartIndex, int nEndIndex)
{
  if (m_DataChangedNotify) m_DataChangedNotify(nStartIndex, nEndIndex);
}

void VirtualTileInterface::EmitCountChanged()
{
  if (m_CountChangedNotify) m_CountChangedNotify();
}


VirtualTileLayout::VirtualTileLayout()
  :m_bAutoCalcColumn(true)
{
  m_nColumns = -1;
}

ui::UiSize VirtualTileLayout::ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc)
{
    (void)items;
  ui::UiSize sz(rc.GetWidth(), rc.GetHeight());

  VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
  (void)pList;
  ASSERT(pList);


  int nTotalHeight = GetElementsHeight(-1);
  sz.cy = std::max(LONG(nTotalHeight), sz.cy);
  LazyArrangeChild();
  return sz;
}

ui::UiSize VirtualTileLayout::AjustSizeByChild(const std::vector<ui::Control*>& items, ui::UiSize szAvailable)
{
  (void)items;
  VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
  (void)pList;
  ASSERT(pList);

  ui::UiSize size = m_pOwner->Control::EstimateSize(szAvailable);
  if (size.cx == DUI_LENGTH_AUTO || size.cx == 0)
  {
    size.cx = m_szItem.cx * m_nColumns + m_iChildMargin * (m_nColumns - 1);
  }
  return size;
}


bool VirtualTileLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
  if (strName == L"column")
  {
    int iValue = _wtoi(strValue.c_str());
    if (iValue > 0)
    {
      SetColumns(iValue);
      m_bAutoCalcColumn = false;
    }
    else {
      m_bAutoCalcColumn = true;
    }
    return true;
  }
  else {
    return __super::SetAttribute(strName, strValue);
  }
}

int VirtualTileLayout::GetElementsHeight(int nCount)
{
  if (nCount <= m_nColumns && nCount != -1) return m_szItem.cy + m_iChildMargin;

  VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
  ASSERT(pList);

  if (nCount < 0)
    nCount = pList->GetElementCount();

  int rows = nCount / m_nColumns;
  if (nCount % m_nColumns != 0) {
    rows += 1;
  }
  if (nCount > 0) {
    int childMarginTotal;
    if (nCount % m_nColumns == 0) {
      childMarginTotal = (nCount / m_nColumns - 1) * m_iChildMargin;
    }
    else {
      childMarginTotal = (nCount / m_nColumns) * m_iChildMargin;
    }
    return m_szItem.cy * (rows + 1) + childMarginTotal;
  }
  return 0;
}

void VirtualTileLayout::LazyArrangeChild()
{
  VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);

  ASSERT(pList);

  // 列在SetPos时已经设置好
  ASSERT(m_nColumns);

  // 获取VirtualTileBox的Rect
  ui::UiRect rc = pList->GetPaddingPos();

  // 子项的左边起始位置 
  int iPosLeft = rc.left;

  // 子项的顶部起始位置 
  int iPosTop = rc.top + pList->GetScrollPos().cy;

  ui::UiPoint ptTile(iPosLeft, iPosTop);

  // 顶部index
  int nTopBottom = 0;
  int nTopIndex = pList->GetTopElementIndex(nTopBottom);

  int iCount = 0;

  for (auto pControl : pList->m_items)
  {
      if (pControl == nullptr) {
          continue;
      }
    // Determine size
    ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + m_szItem.cx, ptTile.y + m_szItem.cy);
    pControl->SetPos(rcTile);

    // 填充数据
    int nElementIndex = nTopIndex + iCount;
    if (nElementIndex < pList->GetElementCount())
    {
      if (!pControl->IsVisible()) pControl->SetFadeVisible(true);
      pList->FillElement(pControl, nElementIndex);
    }
    else {
      if (pControl->IsVisible()) pControl->SetFadeVisible(false);
    }

    if ((++iCount % m_nColumns) == 0) {
      ptTile.x = iPosLeft;
      ptTile.y += m_szItem.cy + m_iChildMargin;
    }
    else {
      ptTile.x += rcTile.GetWidth() + m_iChildMargin;
    }
  }
}

int VirtualTileLayout::AjustMaxItem()
{
  ui::UiRect rc = m_pOwner->GetPaddingPos();

  if (m_bAutoCalcColumn)
  {
    if (m_szItem.cx > 0) m_nColumns = (rc.right - rc.left) / (m_szItem.cx + m_iChildMargin / 2);
    if (m_nColumns == 0) m_nColumns = 1;
  }

  int nHeight = m_szItem.cy + m_iChildMargin;
  int nRow = (rc.bottom - rc.top) / nHeight + 1;
  return nRow * m_nColumns;
}

VirtualTileBox::VirtualTileBox(ui::Layout* pLayout /*= new VirtualTileLayout*/)
  : ui::ListBox(pLayout)
  , m_pDataProvider(nullptr)
  , m_nMaxItemCount(0)
  , m_nOldYScrollPos(0)
  , m_bArrangedOnce(false)
  , m_bForceArrange(false)
{


}

void VirtualTileBox::SetDataProvider(VirtualTileInterface* pProvider)
{
  ASSERT(pProvider);
  m_pDataProvider = pProvider;

  // 注册模型数据变动通知回调
  pProvider->RegNotifys(
    nbase::Bind(&VirtualTileBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
    nbase::Bind(&VirtualTileBox::OnModelCountChanged, this));
}

VirtualTileInterface* VirtualTileBox::GetDataProvider()
{
  return m_pDataProvider;
}

void VirtualTileBox::Refresh()
{
  m_nMaxItemCount = GetTileLayout()->AjustMaxItem();

  int nElementCount = GetElementCount();
  int nItemCount = GetItemCount();

  // 如果现有子项总数大于数据总数， 移出比数据总数多出的子项
  if (nItemCount > nElementCount)
  {

    int n = nItemCount - nElementCount;
    for (int i = 0; i < n; i++)
      this->RemoveItemAt(0);
  }
  // 如果子项总数据小于数据总数
  else if (nItemCount < nElementCount) {
    int n = 0;
    if (nElementCount <= m_nMaxItemCount)
    {
      n = nElementCount - nItemCount;
    }
    else {
      n = m_nMaxItemCount - nItemCount;
    }

    for (int i = 0; i < n; i++) {
      Control* pControl = CreateElement();
      this->AddItem(pControl);
    }
  }

  if (nElementCount <= 0)
    return;

  ReArrangeChild(true);
  Arrange();

}

void VirtualTileBox::RemoveAllItems()
{
  __super::RemoveAllItems();

  if (m_pVerticalScrollBar)
    m_pVerticalScrollBar->SetScrollPos(0);

  m_nOldYScrollPos = 0;
  m_bArrangedOnce = false;
  m_bForceArrange = false;
}

void VirtualTileBox::SetForceArrange(bool bForce)
{
  m_bForceArrange = bForce;
}

void VirtualTileBox::GetDisplayCollection(std::vector<int>& collection)
{
  collection.clear();

  if (GetItemCount() == 0)
    return;

  // 获取Box的Rect
  ui::UiRect rcThis = this->GetPaddingPos();

  int nEleHeight = GetRealElementHeight();

  int min = (GetScrollPos().cy / nEleHeight) * GetColumns();
  int max = min + (rcThis.GetHeight() / nEleHeight) * GetColumns();

  int nCount = GetElementCount();
  if (max >= nCount)
    max = nCount - 1;

  for (auto i = min; i <= max; i++)
    collection.push_back(i);
}

void VirtualTileBox::EnsureVisible(int iIndex, bool bToTop /*= false*/)
{
  if (iIndex < 0 || iIndex >= GetElementCount())
    return;

  if (!m_pVerticalScrollBar)
    return;

  auto nPos = GetScrollPos().cy;
  int nTopIndex = (nPos / GetRealElementHeight()) * GetColumns();
  int nNewPos = 0;

  if (bToTop)
  {
    nNewPos = CalcElementsHeight(iIndex);
    if (nNewPos >= m_pVerticalScrollBar->GetScrollRange())
      return;
  }
  else {
    if (IsElementDisplay(iIndex))
      return;

    if (iIndex > nTopIndex)
    {
      // 向下
      int height = CalcElementsHeight(iIndex + 1);
      nNewPos = height - GetRect().GetHeight();
    }
    else {
      // 向上
      nNewPos = CalcElementsHeight(iIndex);
    }
  }
  ui::UiSize sz(0, nNewPos);
  SetScrollPos(sz);
}

void VirtualTileBox::SetScrollPos(ui::UiSize szPos)
{
  m_nOldYScrollPos = GetScrollPos().cy;
  ListBox::SetScrollPos(szPos);
  ReArrangeChild(false);
}

void VirtualTileBox::HandleEvent(const ui::EventArgs& event)
{
    if (!IsMouseEnabled() && event.Type > ui::kEventMouseBegin && event.Type < ui::kEventMouseEnd) {
        if (GetParent() != nullptr)
            GetParent()->SendEvent(event);
        else
            ui::ScrollableBox::HandleEvent(event);
        return;
    }

    switch (event.Type) {
    case ui::kEventKeyDown: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyDown(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyDown(VK_DOWN);
            return;
        }
        case VK_HOME:
            SetScrollPosY(0);
            return;
        case VK_END: {
            int range = GetScrollPos().cy;
            SetScrollPosY(range);
            return;
        }
        default:
            break;
        }
    }
    case ui::kEventKeyUp: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyUp(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyUp(VK_DOWN);
            return;
        }
        default:
            break;
        }
    default:
        break;
    }
    }

  __super::HandleEvent(event);
}

void VirtualTileBox::SetPos(ui::UiRect rc)
{
  bool bChange = false;
  if (!GetRect().Equal(rc))
    bChange = true;

  ListBox::SetPos(rc);

  if (bChange) {

    Refresh();
  }
}

void VirtualTileBox::ReArrangeChild(bool bForce)
{
  ScrollDirection direction = kScrollUp;
  if (!bForce && !m_bForceArrange) {
    if (!NeedReArrange(direction))
      return;
  }

  LazyArrangeChild();
}


ui::Control* VirtualTileBox::CreateElement()
{
  if (m_pDataProvider)
    return m_pDataProvider->CreateElement();

  return nullptr;
}

void VirtualTileBox::FillElement(Control* pControl, int iIndex)
{
  if (m_pDataProvider)
    m_pDataProvider->FillElement(pControl, iIndex);
}

int VirtualTileBox::GetElementCount()
{
  if (m_pDataProvider)
    return m_pDataProvider->GetElementtCount();

  return 0;
}

int VirtualTileBox::CalcElementsHeight(int nCount)
{
  return GetTileLayout()->GetElementsHeight(nCount);
}

int VirtualTileBox::GetTopElementIndex(int& bottom)
{
  int nPos = GetScrollPos().cy;

  int nHeight = GetRealElementHeight();
  int iIndex = (nPos / nHeight) * GetColumns();
  bottom = iIndex * nHeight;

  return iIndex;
}

bool VirtualTileBox::IsElementDisplay(int iIndex)
{
  if (iIndex < 0)
    return false;

  int nPos = GetScrollPos().cy;
  int nElementPos = CalcElementsHeight(iIndex);
  if (nElementPos >= nPos) {
    int nHeight = this->GetHeight();
    if (nElementPos <= nPos + nHeight)
      return true;
  }

  return false;
}

bool VirtualTileBox::NeedReArrange(ScrollDirection& direction)
{
  direction = kScrollUp;
  if (!m_bArrangedOnce) {
    m_bArrangedOnce = true;
    return true;
  }

  int nCount = GetItemCount();
  if (nCount <= 0)
    return false;

  if (GetElementCount() <= nCount)
    return false;


  ui::UiRect rcThis = this->GetPos();
  if (rcThis.GetWidth() <= 0)
    return false;

  int nPos = GetScrollPos().cy;
  ui::UiRect rcItem;

  rcItem = m_items[0]->GetPos();

  if (nPos >= m_nOldYScrollPos) {
    // 下
    rcItem = m_items[nCount - 1]->GetPos();
    int nSub = (rcItem.bottom - rcThis.top) - (nPos + rcThis.GetHeight());
    if (nSub < 0) {
      direction = kScrollDown;
      return true;
    }
  }
  else {
    // 上
    rcItem = m_items[0]->GetPos();
    if (nPos < (rcItem.top - rcThis.top)) {
      direction = kScrollUp;
      return true;
    }
  }

  return false;

}

VirtualTileLayout* VirtualTileBox::GetTileLayout()
{
  auto* pLayout = dynamic_cast<VirtualTileLayout*>(GetLayout());
  return pLayout;
}

int VirtualTileBox::GetRealElementHeight()
{
  return GetTileLayout()->GetElementsHeight(1);
}

int VirtualTileBox::GetColumns()
{
  return GetTileLayout()->GetColumns();
}

void VirtualTileBox::LazyArrangeChild()
{

  GetTileLayout()->LazyArrangeChild();
}

void VirtualTileBox::OnModelDataChanged(int nStartIndex, int nEndIndex)
{
  for (auto i = nStartIndex; i <= nEndIndex; i++)
  {
    int nItemIndex = ElementIndexToItemIndex(nStartIndex);
    if (nItemIndex >= 0 && nItemIndex < (int)m_items.size()) {
      FillElement(m_items[nItemIndex], i);
    }
  }
}

void VirtualTileBox::OnModelCountChanged()
{
  Refresh();
}

int VirtualTileBox::ElementIndexToItemIndex(int nElementIndex)
{
  if (IsElementDisplay(nElementIndex))
  {
    int nTopItemHeight = 0;
    return nElementIndex - GetTopElementIndex(nTopItemHeight);
  }
  return -1;
}

int VirtualTileBox::ItemIndexToElementIndex(int nItemIndex)
{
  int nTopItemHeight = 0;
  return GetTopElementIndex(nTopItemHeight) + nItemIndex;
}

}
