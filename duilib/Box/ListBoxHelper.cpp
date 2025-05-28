#include "ListBoxHelper.h"
#include "ListBox.h"
#include "duilib/Render/IRender.h"

namespace ui 
{
ListBoxHelper::ListBoxHelper(ListBox* pListBox) :
    m_pListBox(pListBox),
    m_bMouseDown(false),
    m_bRMouseDown(false),
    m_bInMouseMove(false),
    m_pMouseSender(nullptr),
    m_bMouseDownInView(false),
    m_bEnableFrameSelection(false),
    m_frameSelectionBorderSize(1),
    m_frameSelectionBorderColor(_T("#FF0078D7")),
    m_frameSelectionColor(_T("#FFAACCEE")),
    m_frameSelectionAlpha(128),
    m_nNormalItemTop(-1)
{
}

ListBoxHelper::~ListBoxHelper()
{
}

void ListBoxHelper::SetEnableFrameSelection(bool bEnable)
{
    m_bEnableFrameSelection = bEnable;
}

bool ListBoxHelper::IsEnableFrameSelection() const
{
    return m_bEnableFrameSelection;
}

void ListBoxHelper::SetFrameSelectionColor(const DString& frameSelectionColor)
{
    m_frameSelectionColor = frameSelectionColor;
}

DString ListBoxHelper::GetFrameSelectionColor() const
{
    return m_frameSelectionColor.c_str();
}

void ListBoxHelper::SetframeSelectionAlpha(uint8_t frameSelectionAlpha)
{
    m_frameSelectionAlpha = frameSelectionAlpha;
}

uint8_t ListBoxHelper::GetFrameSelectionAlpha() const
{
    return m_frameSelectionAlpha;
}

void ListBoxHelper::SetFrameSelectionBorderColor(const DString& frameSelectionBorderColor)
{
    m_frameSelectionBorderColor = frameSelectionBorderColor;
}

DString ListBoxHelper::GetFrameSelectionBorderColor() const
{
    return m_frameSelectionBorderColor.c_str();
}

void ListBoxHelper::SetFrameSelectionBorderSize(int32_t nBorderSize)
{
    if (nBorderSize < 0) {
        nBorderSize = 0;
    }
    m_frameSelectionBorderSize = (uint8_t)nBorderSize;
}

int32_t ListBoxHelper::GetFrameSelectionBorderSize() const
{
    return m_frameSelectionBorderSize;
}

void ListBoxHelper::SetNormalItemTop(int32_t nNormalItemTop)
{
    m_nNormalItemTop = nNormalItemTop;
}

int32_t ListBoxHelper::GetNormalItemTop() const
{
    return m_nNormalItemTop;
}

void ListBoxHelper::PaintFrameSelection(IRender* pRender)
{
    if (!m_bInMouseMove || (pRender == nullptr)) {
        return;
    }
    UiSize64 scrollPos = m_pListBox->GetScrollPos();
    int64_t left = std::min(m_ptMouseDown.cx, m_ptMouseMove.cx) - scrollPos.cx;
    int64_t right = std::max(m_ptMouseDown.cx, m_ptMouseMove.cx) - scrollPos.cx;
    int64_t top = std::min(m_ptMouseDown.cy, m_ptMouseMove.cy) - scrollPos.cy;
    int64_t bottom = std::max(m_ptMouseDown.cy, m_ptMouseMove.cy) - scrollPos.cy;
    if (m_nNormalItemTop > 0) {
        if (top < m_nNormalItemTop) {
            top = (int64_t)m_nNormalItemTop - m_pListBox->Dpi().GetScaleInt(4);
        }
        if (bottom < m_nNormalItemTop) {
            bottom = m_nNormalItemTop;
        }
    }

    UiRect rect(TruncateToInt32(left), TruncateToInt32(top),
                TruncateToInt32(right), TruncateToInt32(bottom));

    int32_t frameSelectionBorderSize = GetFrameSelectionBorderSize();    
    if ((frameSelectionBorderSize > 0) && !m_frameSelectionBorderColor.empty()) {
        float fSelectionBorderSize = m_pListBox->Dpi().GetScaleFloat(frameSelectionBorderSize);
        pRender->DrawRect(rect, m_pListBox->GetUiColor(m_frameSelectionBorderColor.c_str()), fSelectionBorderSize);
    }
    if (!m_frameSelectionColor.empty()) {
        pRender->FillRect(rect, m_pListBox->GetUiColor(m_frameSelectionColor.c_str()), m_frameSelectionAlpha);
    }
}

void ListBoxHelper::OnButtonDown(const UiPoint& ptMouse, Control* pSender)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        m_pListBox->Invalidate();
    }
    m_bMouseDownInView = (pSender == m_pListBox) ? true : false;
    m_bMouseDown = true;
    m_pMouseSender = pSender;
    UiSize64 scrollPos = m_pListBox->GetScrollPos();
    m_ptMouseDown.cx = ptMouse.x + scrollPos.cx;
    m_ptMouseDown.cy = ptMouse.y + scrollPos.cy;
}

void ListBoxHelper::OnButtonUp(const UiPoint& /*ptMouse*/, Control* pSender)
{
    bool bClickedBlank = false;
    if (m_bMouseDownInView && !m_bInMouseMove && (pSender == m_pListBox)) {
        bClickedBlank = true;
    }
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        m_pListBox->Invalidate();
    }
    m_bMouseDownInView = false;
    m_bMouseDown = false;
    m_pMouseSender = nullptr;
    if (bClickedBlank) {
        if (m_pListBox->OnLButtonClickedBlank()) {
            m_pListBox->SendEvent(kEventSelChange);
        }
    }
}

void ListBoxHelper::OnRButtonDown(const UiPoint& ptMouse, Control* pSender)
{
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        m_pListBox->Invalidate();
    }
    m_bMouseDownInView = (pSender == m_pListBox) ? true : false;
    m_bRMouseDown = true;
    m_pMouseSender = pSender;
    UiSize64 scrollPos = m_pListBox->GetScrollPos();
    m_ptMouseDown.cx = ptMouse.x + scrollPos.cx;
    m_ptMouseDown.cy = ptMouse.y + scrollPos.cy;
}

void ListBoxHelper::OnRButtonUp(const UiPoint& /*ptMouse*/, Control* pSender)
{
    bool bClickedBlank = false;
    if (m_bMouseDownInView && !m_bInMouseMove && (pSender == m_pListBox)) {
        bClickedBlank = true;
    }
    if (m_bInMouseMove) {
        m_bInMouseMove = false;
        m_pListBox->Invalidate();
    }
    m_bMouseDownInView = false;
    m_bRMouseDown = false;
    m_pMouseSender = nullptr;
    if (bClickedBlank) {
        if (m_pListBox->OnRButtonClickedBlank()) {
            m_pListBox->SendEvent(kEventSelChange);
        }
    }
}

void ListBoxHelper::OnMouseMove(const UiPoint& ptMouse, Control* pSender)
{
    if (!IsEnableFrameSelection() || !m_pListBox->IsMultiSelect()) {
        //功能关闭 或者 单选模式
        return;
    }
    if ((m_bMouseDown || m_bRMouseDown) &&
        (pSender != nullptr) &&
        (m_pMouseSender == pSender) && pSender->IsMouseFocused()) {
        UiSize64 scrollPos = m_pListBox->GetScrollPos();
        m_ptMouseMove.cx = ptMouse.x + scrollPos.cx;
        m_ptMouseMove.cy = ptMouse.y + scrollPos.cy;

        //鼠标移动超过指定像素数的时候，才开始按移动操作，避免将正常的点击操作识别为框选操作
        const int32_t minPt = m_pListBox->Dpi().GetScaleInt(8);
        if (!m_bInMouseMove) {
            if ((std::abs(m_ptMouseMove.cx - m_ptMouseDown.cx) > minPt) ||
                (std::abs(m_ptMouseMove.cy - m_ptMouseDown.cy) > minPt)) {
                //开始框选操作
                m_bInMouseMove = true;
                OnCheckScrollView();
            }
        }
        else {
            //按需滚动视图，并更新鼠标在滚动后的位置            
            OnCheckScrollView();
        }
    }
    else if (m_bInMouseMove) {
        m_bInMouseMove = false;
        m_pListBox->Invalidate();
    }
}

void ListBoxHelper::OnWindowKillFocus()
{
    if (m_bInMouseMove) {
        m_pListBox->Invalidate();
    }
    m_bMouseDownInView = false;
    m_bMouseDown = false;
    m_bRMouseDown = false;
    m_bInMouseMove = false;
    m_pMouseSender = nullptr;
}

void ListBoxHelper::OnCheckScrollView()
{
    if (!m_bInMouseMove || !IsEnableFrameSelection()) {
        //取消定时器
        m_scrollViewFlag.Cancel();
        return;
    }
    bool bScrollView = false;
    const UiSize64 scrollPos = m_pListBox->GetScrollPos();
    UiSize64 pt = m_ptMouseMove;
    pt.cx -= scrollPos.cx;
    pt.cy -= scrollPos.cy;
    const UiSize64 ptMouseMove = pt; //记录原值

    if (m_bInMouseMove) {
        int32_t nHScrollValue = DUI_NOSET_VALUE;
        int32_t nVScrollValue = DUI_NOSET_VALUE;
        m_pListBox->GetScrollDeltaValue(nHScrollValue, nVScrollValue);
        UiRect viewRect = m_pListBox->GetRect();
        if (m_nNormalItemTop > 0) {
            viewRect.top = m_nNormalItemTop;
            ASSERT(viewRect.top <= viewRect.bottom);
        }
        if (pt.cx <= viewRect.left) {
            //向左滚动视图
            m_pListBox->LineLeft(nHScrollValue);
            bScrollView = true;
        }
        else if (pt.cx >= viewRect.right) {
            //向右滚动视图
            m_pListBox->LineRight(nHScrollValue);
            bScrollView = true;
        }
        if (pt.cy <= viewRect.top) {
            //向上滚动视图
            m_pListBox->LineUp(nVScrollValue, false);
            bScrollView = true;
        }
        else if (pt.cy >= viewRect.bottom) {
            //向下滚动视图
            m_pListBox->LineDown(nVScrollValue, false);
            bScrollView = true;
        }
    }

    if (bScrollView) {
        UiSize64 scrollPosNew = m_pListBox->GetScrollPos();
        if (scrollPos != scrollPosNew) {
            //更新鼠标位置
            m_ptMouseMove.cx = ptMouseMove.cx + scrollPosNew.cx;
            m_ptMouseMove.cy = ptMouseMove.cy + scrollPosNew.cy;
        }

        //启动定时器
        m_scrollViewFlag.Cancel();
        GlobalManager::Instance().Timer().AddTimer(m_scrollViewFlag.GetWeakFlag(),
                                                   UiBind(&ListBoxHelper::OnCheckScrollView, this),
                                                   50, 1); //只执行一次
    }
    else {
        //取消定时器
        m_scrollViewFlag.Cancel();
    }

    int64_t top = std::min(m_ptMouseDown.cy, m_ptMouseMove.cy);
    int64_t bottom = std::max(m_ptMouseDown.cy, m_ptMouseMove.cy);
    int64_t left = std::min(m_ptMouseDown.cx, m_ptMouseMove.cx);
    int64_t right = std::max(m_ptMouseDown.cx, m_ptMouseMove.cx);
    int32_t offsetTop = m_pListBox->GetRect().top;//当前控件左上角的top坐标
    top -= offsetTop;
    bottom -= offsetTop;

    int32_t offsetLeft = m_pListBox->GetRect().left;//当前控件左上角的left坐标
    left -= offsetLeft;
    right -= offsetLeft;
    bool bRet = m_pListBox->OnFrameSelection(left, right, top, bottom);
    m_pListBox->Invalidate();
    if (bRet) {
        m_pListBox->SendEvent(kEventSelChange);
    }
}

} // namespace ui
