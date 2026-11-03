#include "ChatMessageList.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/Clipboard.h"
#include <algorithm>

namespace ui
{

namespace
{
    /** 时间分隔条高度（像素，未做 DPI 缩放）
    */
    constexpr int32_t kTimeDividerHeight = 28;

    /** 状态文字与气泡之间的间距（像素，未做 DPI 缩放）
    */
    constexpr int32_t kStatusGap = 4;

    /** 拖选判定阈值（像素）：鼠标移动超过该距离才认为开始拖选，避免把点击误判为拖选
    */
    constexpr int32_t kSelectMinMovePixels = 8;

    /** 气泡与滚动条之间的间距（像素，未做 DPI 缩放）
    */
    constexpr int32_t kContentScrollbarGap = 10;
}

ChatMessageList::ChatMessageList(Window* pWindow) :
    BaseClass(pWindow),
    m_bLayoutDirty(true),
    m_nLayoutWidth(-1),
    m_nAvatarSize(36),
    m_nBubbleMaxWidth(480),
    m_nBubblePaddingH(12),
    m_nBubblePaddingV(8),
    m_nBubbleRadius(10),
    m_nBubbleSpacing(14),
    m_bShowTime(true),
    m_strTimeFormat(_T("HH:mm")),
    m_strAvatarFont(_T("system_bold_14")),
    m_strTextFont(_T("system_regular_14")),
    m_strTimeFont(_T("system_regular_12")),
    m_strBubbleColorSelf(_T("color_accent")),
    m_strBubbleColorOther(_T("bg_window_card")),
    m_strTextColorSelf(_T("color_white")),
    m_strTextColorOther(_T("text_default")),
    m_strAvatarTextColor(_T("color_white")),
    m_strSystemTextColor(_T("text_muted")),
    m_strSelectionBkColor(_T("bg_list_item_selected")),
    m_strSelectionTextColor(_T("text_selected")),
    m_strScrollbarTrackColor(_T("bg_scrollbar_normal")),
    m_strScrollbarThumbColor(_T("bg_scrollbar_thumb_normal")),
    m_bAutoHideScrollbar(true),
    m_bAutoScrollToBottom(true),
    m_bEnableSelection(true),
    m_nScrollPos(0),
    m_nContentHeight(0),
    m_bShowStatus(true),
    m_strStatusFont(_T("system_regular_12")),
    m_strStatusTextColor(_T("text_muted")),
    m_strStatusFailedColor(_T("color_error")),
    m_bPendingClick(false),
    m_nClickIndex(0),
    m_eClickArea(ChatMessageHitArea::kNone),
    m_bSelecting(false),
    m_bPendingSelect(false),
    m_bHasSelection(false),
    m_ptMouseDown(),
    m_nAnchorIndex(0),
    m_nAnchorChar(0),
    m_nSelStartIndex(0),
    m_nSelEndIndex(0),
    m_nSelStartChar(0),
    m_nSelEndChar(0),
    m_pContentPlaceholder(nullptr)
{
}

ChatMessageList::~ChatMessageList()
{
    //占位控件由容器统一管理，此处只需解除引用
    m_pContentPlaceholder = nullptr;
}

DString ChatMessageList::GetType() const
{
    return DUI_CTR_CHATMESSAGELIST;
}

void ChatMessageList::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("avatar_size")) {
        m_nAvatarSize = _wtoi(strValue.c_str());
        InvalidateWidthCache();
    }
    else if (strName == _T("bubble_max_width")) {
        m_nBubbleMaxWidth = _wtoi(strValue.c_str());
        InvalidateWidthCache();
    }
    else if (strName == _T("bubble_padding_h")) {
        m_nBubblePaddingH = _wtoi(strValue.c_str());
        InvalidateWidthCache();
    }
    else if (strName == _T("bubble_padding_v")) {
        m_nBubblePaddingV = _wtoi(strValue.c_str());
        InvalidateWidthCache();
    }
    else if (strName == _T("bubble_radius")) {
        m_nBubbleRadius = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_spacing")) {
        m_nBubbleSpacing = _wtoi(strValue.c_str());
        InvalidateLayoutCache();
    }
    else if (strName == _T("show_time")) {
        m_bShowTime = (strValue != _T("false"));
        InvalidateLayoutCache();
    }
    else if (strName == _T("time_format")) {
        m_strTimeFormat = strValue;
    }
    else if (strName == _T("avatar_font")) {
        m_strAvatarFont = strValue;
    }
    else if (strName == _T("text_font")) {
        m_strTextFont = strValue;
        InvalidateWidthCache();
    }
    else if (strName == _T("time_font")) {
        m_strTimeFont = strValue;
        InvalidateWidthCache();
    }
    else if (strName == _T("bubble_color_self")) {
        m_strBubbleColorSelf = strValue;
    }
    else if (strName == _T("bubble_color_other")) {
        m_strBubbleColorOther = strValue;
    }
    else if (strName == _T("text_color_self")) {
        m_strTextColorSelf = strValue;
    }
    else if (strName == _T("text_color_other")) {
        m_strTextColorOther = strValue;
    }
    else if (strName == _T("avatar_text_color")) {
        m_strAvatarTextColor = strValue;
    }
    else if (strName == _T("system_text_color")) {
        m_strSystemTextColor = strValue;
    }
    else if (strName == _T("selection_bk_color")) {
        m_strSelectionBkColor = strValue;
    }
    else if (strName == _T("selection_text_color")) {
        m_strSelectionTextColor = strValue;
    }
    else if (strName == _T("show_status")) {
        SetShowStatus(strValue != _T("false"));
    }
    else if (strName == _T("status_font")) {
        m_strStatusFont = strValue;
        InvalidateWidthCache();
    }
    else if (strName == _T("status_text_color")) {
        m_strStatusTextColor = strValue;
    }
    else if (strName == _T("status_failed_color")) {
        m_strStatusFailedColor = strValue;
    }
    else if (strName == _T("scrollbar_track_color")) {
        m_strScrollbarTrackColor = strValue;
    }
    else if (strName == _T("scrollbar_thumb_color")) {
        m_strScrollbarThumbColor = strValue;
    }
    else if (strName == _T("auto_hide_scrollbar")) {
        m_bAutoHideScrollbar = (strValue != _T("false"));
    }
    else if (strName == _T("enable_selection")) {
        m_bEnableSelection = (strValue != _T("false"));
        if (!m_bEnableSelection) {
            ClearSelection();
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ChatMessageList::SetPos(UiRect rc)
{
    //先用新尺寸算好消息布局与占位控件高度，
    //这样 ScrollBox 在本次布局里就能用正确的内容高度推导滚动范围。
    UpdateContentSize(rc);
    BaseClass::SetPos(rc);

    //滚动条位置：贴内容区右边缘，高度固定为内容区高度。
    //纯自绘容器里没有子控件驱动滚动条尺寸，不固定高度的话，
    //ScrollBox 重排时会把它压成一小段，看起来像"浮在消息区中间"。
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        UiRect rcBar = rc;
        rcBar.Deflate(GetPadding());
        rcBar.Deflate(GetScrollBarPadding());
        const int32_t nBarWidth = pVScrollBar->GetFixedWidth().GetInt32();
        if ((nBarWidth > 0) && (rcBar.Height() > 0)) {
            pVScrollBar->SetFixedHeight(UiFixedInt(rcBar.Height()), false, false);
            pVScrollBar->SetPos(UiRect(rcBar.right - nBarWidth, rcBar.top, rcBar.right, rcBar.bottom));
        }
        //注意：这里不修改滚动条的可见性。
        //本控件为纯自绘内容，没有子控件来驱动 ScrollBox 的滚动条可见性，
        //可见性改由框架的 ScrollBox::MouseEnter 机制处理（auto_hide 模式悬停显示）。
    }

    //布局后 ScrollBox 可能钳位了滚动位置，同步自绘偏移量
    m_nScrollPos = static_cast<int32_t>(BaseClass::GetScrollPos().cy);
}

void ChatMessageList::UpdateContentSize(const UiRect& rc)
{
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return;
    }
    const int32_t nContentWidth = CalcContentWidthFor(rc);
    if (nContentWidth <= 0) {
        return;
    }
    if (!EnsureLayout(pRender, nContentWidth)) {
        return;
    }
    UpdateContentPlaceholder(m_nContentHeight);
}

void ChatMessageList::SetWindow(Window* pWindow)
{
    const bool bHadVScrollBar = (GetVScrollBar() != nullptr);
    BaseClass::SetWindow(pWindow);
    if (!bHadVScrollBar && (pWindow != nullptr)) {
        //纯自绘内容没有子控件，需要显式启用滚动条并设置样式类
        EnableScrollBar(true, false);
        ScrollBar* pVScrollBar = GetVScrollBar();
        if (pVScrollBar != nullptr) {
            //样式类由 XML 的 vscrollbar_class 指定，这里不覆盖
            pVScrollBar->SetAutoHideScroll(m_bAutoHideScrollbar);
        }

        //创建用于承载内容高度的透明占位控件：
        //ScrollBox 的滚动范围由子控件布局尺寸推导，没有子控件时框架会隐藏滚动条。
        //占位控件必须保持可见（否则不参与布局计算），通过透明背景让它不可见。
        m_pContentPlaceholder = new Control(pWindow);
        if (m_pContentPlaceholder != nullptr) {
            m_pContentPlaceholder->SetName(_T("content_placeholder"));
            m_pContentPlaceholder->SetFixedWidth(ui::UiFixedInt(0), false, false);
            m_pContentPlaceholder->SetFixedHeight(ui::UiFixedInt(0), false, false);
            m_pContentPlaceholder->SetBkColor(UiColor(0x00000000));
            m_pContentPlaceholder->SetMouseEnabled(false);
            AddItem(m_pContentPlaceholder);
        }
    }
}

void ChatMessageList::OnScrollOffsetChanged(const UiSize& oldScrollOffset, const UiSize& newScrollOffset)
{
    BaseClass::OnScrollOffsetChanged(oldScrollOffset, newScrollOffset);
    //滚轮、拖动滑块、点击轨道等操作由框架滚动条驱动，
    //自绘内容需要跟上这个偏移量，否则画面不会跟着滚动。
    const int32_t nPos = static_cast<int32_t>(newScrollOffset.cy);
    if (nPos != m_nScrollPos) {
        m_nScrollPos = nPos;
        Invalidate();
    }
}

void ChatMessageList::UpdateContentPlaceholder(int32_t nContentHeight)
{
    if (m_pContentPlaceholder == nullptr) {
        return;
    }
    //占位控件的高度即内容总高度；宽度固定为 0，避免影响内容区宽度计算
    m_pContentPlaceholder->SetFixedHeight(ui::UiFixedInt(std::max(0, nContentHeight)), false, false);
}

void ChatMessageList::InvalidateLayoutCache()
{
    //消息内容变化：偏移表失效，未标记失效的消息高度仍然可以复用
    m_bLayoutDirty = true;
    Invalidate();
}

void ChatMessageList::InvalidateWidthCache()
{
    //宽度或字体变化：所有消息的高度都需要重新测量
    m_msgHeights.clear();
    m_msgOffsets.clear();
    m_nLayoutWidth = -1;
    m_bLayoutDirty = true;
    Invalidate();
}

void ChatMessageList::MarkMessageHeightDirty(size_t nIndex)
{
    if (nIndex < m_msgHeights.size()) {
        m_msgHeights[nIndex] = -1;
    }
    m_bLayoutDirty = true;
    Invalidate();
}

bool ChatMessageList::IsTimeDividerShown(size_t nIndex) const
{
    if (!m_bShowTime || (nIndex >= m_messages.size())) {
        return false;
    }
    const DString& strTime = m_messages[nIndex].time;
    if (strTime.empty()) {
        return false;
    }
    if (nIndex == 0) {
        return true;
    }
    return strTime != m_messages[nIndex - 1].time;
}

bool ChatMessageList::EnsureLayout(IRender* pRender, int32_t nContentWidth)
{
    if ((pRender == nullptr) || (nContentWidth <= 0)) {
        return false;
    }
    const size_t nCount = m_messages.size();

    //宽度变化：所有消息的高度都需要重新测量
    if (m_nLayoutWidth != nContentWidth) {
        m_nLayoutWidth = nContentWidth;
        m_msgHeights.assign(nCount, -1);
        m_bLayoutDirty = true;
    }
    if (m_msgHeights.size() != nCount) {
        m_msgHeights.resize(nCount, -1);
        m_bLayoutDirty = true;
    }
    if (!m_bLayoutDirty && (m_msgOffsets.size() == nCount)) {
        return true;
    }

    //重建偏移表：只测量标记失效的消息，其余复用缓存高度
    m_msgOffsets.assign(nCount, 0);
    const int32_t nDividerHeight = Dpi().GetScaleInt(kTimeDividerHeight);
    const int32_t nSpacing = Dpi().GetScaleInt(m_nBubbleSpacing);
    int32_t nOffset = 0;
    for (size_t index = 0; index < nCount; ++index) {
        if (IsTimeDividerShown(index)) {
            nOffset += nDividerHeight;
        }
        m_msgOffsets[index] = nOffset;
        if (m_msgHeights[index] < 0) {
            m_msgHeights[index] = CalcMessageHeight(pRender, m_messages[index], nContentWidth);
        }
        nOffset += m_msgHeights[index] + nSpacing;
    }
    m_nContentHeight = (nCount > 0) ? std::max(0, nOffset - nSpacing) : 0;
    m_bLayoutDirty = false;
    return true;
}

bool ChatMessageList::GetMessageRect(size_t nIndex, UiRect& rcMessage) const
{
    if ((nIndex >= m_msgOffsets.size()) || (nIndex >= m_msgHeights.size())) {
        return false;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth <= 0) {
        return false;
    }
    const int32_t nTop = rc.top - m_nScrollPos + m_msgOffsets[nIndex];
    rcMessage = UiRect(rc.left, nTop, rc.left + nContentWidth, nTop + m_msgHeights[nIndex]);
    return true;
}

size_t ChatMessageList::FindFirstVisibleIndex(int32_t nContentOffset) const
{
    //偏移表单调递增，二分查找第一条底部越过该位置的消息
    size_t nLow = 0;
    size_t nHigh = m_msgOffsets.size();
    while (nLow < nHigh) {
        const size_t nMid = nLow + (nHigh - nLow) / 2;
        const int32_t nBottom = m_msgOffsets[nMid] + m_msgHeights[nMid];
        if (nBottom <= nContentOffset) {
            nLow = nMid + 1;
        }
        else {
            nHigh = nMid;
        }
    }
    return nLow;
}

int64_t ChatMessageList::AddMessage(const Message& message)
{
    return InsertMessageInternal(m_messages.size(), message);
}

int64_t ChatMessageList::AddMessage(ChatMessageSide side, const DString& strText,
                                    const DString& strAvatarText, const DString& strTime)
{
    Message message;
    message.side = side;
    message.type = (side == ChatMessageSide::kSystem) ? ChatMessageType::kSystem : ChatMessageType::kText;
    message.text = strText;
    message.avatarText = strAvatarText;
    message.time = strTime;
    return AddMessage(message);
}

int64_t ChatMessageList::InsertMessage(size_t index, const Message& message)
{
    return InsertMessageInternal(index, message);
}

int64_t ChatMessageList::InsertMessageInternal(size_t index, const Message& message)
{
    Message msg = message;
    if (msg.id == 0) {
        msg.id = GenerateMessageId();
    }
    if (index > m_messages.size()) {
        index = m_messages.size();
    }
    const size_t nOldCount = m_messages.size();
    m_messages.insert(m_messages.begin() + static_cast<std::ptrdiff_t>(index), msg);

    //高度缓存与偏移表同步插入（先补齐到插入前的数量，避免 insert 越界）
    if (m_msgHeights.size() < nOldCount) {
        m_msgHeights.resize(nOldCount, -1);
    }
    if (m_msgOffsets.size() < nOldCount) {
        m_msgOffsets.resize(nOldCount, 0);
    }
    m_msgHeights.insert(m_msgHeights.begin() + static_cast<std::ptrdiff_t>(index), -1);
    m_msgOffsets.insert(m_msgOffsets.begin() + static_cast<std::ptrdiff_t>(index), 0);
    InvalidateLayoutCache();

    if (m_bAutoScrollToBottom && (index >= m_messages.size() - 1)) {
        //延迟到下一轮消息循环再滚动，此时控件已完成布局、内容高度才准确
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            ChatMessageList* pThis = this;
            GlobalManager::Instance().Thread().PostTask(kThreadUI, [pThis]() {
                //任务执行时窗口可能已被销毁，此时不能再操作控件
                if ((pThis != nullptr) && (pThis->GetWindow() != nullptr) &&
                    !pThis->GetWindow()->IsClosingWnd()) {
                    pThis->ScrollToBottom();
                }
                });
        }
    }
    Invalidate();
    return msg.id;
}

bool ChatMessageList::UpdateMessage(int64_t id, const Message& message)
{
    if (id == 0) {
        return false;
    }
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].id == id) {
            //保留原 id，其余字段整体替换
            Message msg = message;
            msg.id = id;
            m_messages[index] = msg;
            //只让这一条消息的高度缓存失效
            MarkMessageHeightDirty(index);
            return true;
        }
    }
    return false;
}

bool ChatMessageList::RemoveMessage(int64_t id)
{
    if (id == 0) {
        return false;
    }
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].id == id) {
            m_messages.erase(m_messages.begin() + static_cast<std::ptrdiff_t>(index));
            //高度缓存与偏移表同步删除
            if (index < m_msgHeights.size()) {
                m_msgHeights.erase(m_msgHeights.begin() + static_cast<std::ptrdiff_t>(index));
            }
            if (index < m_msgOffsets.size()) {
                m_msgOffsets.erase(m_msgOffsets.begin() + static_cast<std::ptrdiff_t>(index));
            }
            //删除后清理选区，避免索引失效
            ClearSelection();
            InvalidateLayoutCache();
            return true;
        }
    }
    return false;
}

const ChatMessageList::Message* ChatMessageList::GetMessageById(int64_t id) const
{
    if (id == 0) {
        return nullptr;
    }
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].id == id) {
            return &m_messages[index];
        }
    }
    return nullptr;
}

int64_t ChatMessageList::GenerateMessageId()
{
    static int64_t s_nextId = 1;
    return s_nextId++;
}

void ChatMessageList::ClearMessages()
{
    m_messages.clear();
    m_msgHeights.clear();
    m_msgOffsets.clear();
    m_nLayoutWidth = -1;
    m_nScrollPos = 0;
    m_nContentHeight = 0;
    m_bPendingClick = false;
    ClearSelection();
    InvalidateLayoutCache();
}

const ChatMessageList::Message* ChatMessageList::GetMessageAt(size_t index) const
{
    if (index >= m_messages.size()) {
        return nullptr;
    }
    return &m_messages[index];
}

void ChatMessageList::ScrollToBottom()
{
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    UpdateScrollRange(pRender);
    //占位控件高度可能刚刚变化，需要让 ScrollBox 立即重算滚动范围。
    //这里调用基类实现、绕过本类的 SetPos 覆写，避免"布局->更新尺寸->布局"的重入。
    const UiRect rc = GetRect();
    if ((rc.Width() > 0) && (rc.Height() > 0)) {
        BaseClass::SetPos(rc);
    }
    //滚动范围由 ScrollBox 维护，这里直接滚到范围末端
    BaseClass::SetScrollPos(UiSize64(0, BaseClass::GetScrollRange().cy));
    m_nScrollPos = static_cast<int32_t>(BaseClass::GetScrollPos().cy);
    Invalidate();
}

void ChatMessageList::SetChatScrollPos(int32_t nPos)
{
    if (nPos < 0) {
        nPos = 0;
    }
    BaseClass::SetScrollPos(UiSize64(0, nPos));
    m_nScrollPos = static_cast<int32_t>(BaseClass::GetScrollPos().cy);
    Invalidate();
}

int32_t ChatMessageList::GetVisibleHeight() const
{
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    return std::max(0, rc.Height());
}

void ChatMessageList::SetAutoScrollToBottom(bool bAutoScroll)
{
    m_bAutoScrollToBottom = bAutoScroll;
}

void ChatMessageList::SetAvatarSize(int32_t nSize)
{
    m_nAvatarSize = nSize;
    InvalidateWidthCache();
}

void ChatMessageList::SetBubbleMaxWidth(int32_t nWidth)
{
    m_nBubbleMaxWidth = nWidth;
    InvalidateWidthCache();
}

void ChatMessageList::SetBubbleRadius(int32_t nRadius)
{
    m_nBubbleRadius = nRadius;
    Invalidate();
}

void ChatMessageList::SetShowTime(bool bShowTime)
{
    m_bShowTime = bShowTime;
    InvalidateLayoutCache();
}

void ChatMessageList::SetBubbleColor(ChatMessageSide side, const DString& strColor)
{
    if (side == ChatMessageSide::kSelf) {
        m_strBubbleColorSelf = strColor;
    }
    else if (side == ChatMessageSide::kOther) {
        m_strBubbleColorOther = strColor;
    }
    Invalidate();
}

void ChatMessageList::SetTextColor(ChatMessageSide side, const DString& strColor)
{
    if (side == ChatMessageSide::kSelf) {
        m_strTextColorSelf = strColor;
    }
    else if (side == ChatMessageSide::kOther) {
        m_strTextColorOther = strColor;
    }
    Invalidate();
}

void ChatMessageList::SetAvatarTextColor(const DString& strColor)
{
    m_strAvatarTextColor = strColor;
    Invalidate();
}

void ChatMessageList::SetSystemTextColor(const DString& strColor)
{
    m_strSystemTextColor = strColor;
    Invalidate();
}

void ChatMessageList::SetSelectionBkColor(const DString& strColor)
{
    m_strSelectionBkColor = strColor;
    Invalidate();
}

void ChatMessageList::SetSelectionTextColor(const DString& strColor)
{
    m_strSelectionTextColor = strColor;
    Invalidate();
}

void ChatMessageList::SetScrollbarTrackColor(const DString& strColor)
{
    m_strScrollbarTrackColor = strColor;
    Invalidate();
}

void ChatMessageList::SetScrollbarThumbColor(const DString& strColor)
{
    m_strScrollbarThumbColor = strColor;
    Invalidate();
}

void ChatMessageList::SetAutoHideScrollbar(bool bAutoHide)
{
    m_bAutoHideScrollbar = bAutoHide;
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetAutoHideScroll(bAutoHide);
    }
}

void ChatMessageList::SetEnableSelection(bool bEnable)
{
    m_bEnableSelection = bEnable;
    if (!m_bEnableSelection) {
        ClearSelection();
    }
}

void ChatMessageList::AttachMessageClick(const ChatMessageClickEvent& callback)
{
    m_msgClickCallback = callback;
}

void ChatMessageList::AttachMessageDoubleClick(const ChatMessageActionEvent& callback)
{
    m_msgDoubleClickCallback = callback;
}

void ChatMessageList::AttachMessageRightClick(const ChatMessageActionEvent& callback)
{
    m_msgRightClickCallback = callback;
}

bool ChatMessageList::SetMessageStatus(int64_t id, ChatMessageStatus status)
{
    if (id == 0) {
        return false;
    }
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].id == id) {
            if (m_messages[index].status != status) {
                m_messages[index].status = status;
                //状态文字会影响消息高度，只让这一条消息的高度缓存失效
                MarkMessageHeightDirty(index);
            }
            return true;
        }
    }
    return false;
}

bool ChatMessageList::SetMessageStatusAt(size_t index, ChatMessageStatus status)
{
    if (index >= m_messages.size()) {
        return false;
    }
    if (m_messages[index].status != status) {
        m_messages[index].status = status;
        MarkMessageHeightDirty(index);
    }
    return true;
}

ChatMessageStatus ChatMessageList::GetMessageStatus(size_t index) const
{
    if (index >= m_messages.size()) {
        return ChatMessageStatus::kNone;
    }
    return m_messages[index].status;
}

void ChatMessageList::SetStatusText(ChatMessageStatus status, const DString& strText)
{
    switch (status) {
    case ChatMessageStatus::kSending:
        m_strStatusTextSending = strText;
        break;
    case ChatMessageStatus::kFailed:
        m_strStatusTextFailed = strText;
        break;
    case ChatMessageStatus::kDelivered:
        m_strStatusTextDelivered = strText;
        break;
    case ChatMessageStatus::kRead:
        m_strStatusTextRead = strText;
        break;
    default:
        return;
    }
    //状态文字宽度变化会影响气泡下方的状态行布局
    InvalidateWidthCache();
}

DString ChatMessageList::GetStatusText(ChatMessageStatus status) const
{
    //优先使用应用层自定义的文案，未设置时取多语言资源中的默认文案
    const DString* pCustomText = nullptr;
    const wchar_t* pDefaultId = nullptr;
    switch (status) {
    case ChatMessageStatus::kSending:
        pCustomText = &m_strStatusTextSending;
        pDefaultId = _T("STRID_PUBLIC_CHAT_STATUS_SENDING");
        break;
    case ChatMessageStatus::kFailed:
        pCustomText = &m_strStatusTextFailed;
        pDefaultId = _T("STRID_PUBLIC_CHAT_STATUS_FAILED");
        break;
    case ChatMessageStatus::kDelivered:
        pCustomText = &m_strStatusTextDelivered;
        pDefaultId = _T("STRID_PUBLIC_CHAT_STATUS_DELIVERED");
        break;
    case ChatMessageStatus::kRead:
        pCustomText = &m_strStatusTextRead;
        pDefaultId = _T("STRID_PUBLIC_CHAT_STATUS_READ");
        break;
    default:
        return DString();
    }
    if ((pCustomText != nullptr) && !pCustomText->empty()) {
        return *pCustomText;
    }
    if (pDefaultId == nullptr) {
        return DString();
    }
    return GlobalManager::Instance().Lang().GetStringByID(pDefaultId);
}

DString ChatMessageList::GetStatusColor(ChatMessageStatus status) const
{
    if (status == ChatMessageStatus::kFailed) {
        return m_strStatusFailedColor;
    }
    return m_strStatusTextColor;
}

void ChatMessageList::SetShowStatus(bool bShowStatus)
{
    if (m_bShowStatus == bShowStatus) {
        return;
    }
    m_bShowStatus = bShowStatus;
    //状态文字参与高度计算，开关变化后需要重新测量
    InvalidateWidthCache();
}

void ChatMessageList::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }
    if (m_messages.empty()) {
        return;
    }

    //内容区宽度固定预留滚动条宽度（见 CalcContentWidth 的说明）
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth <= 0) {
        return;
    }
    //更新滚动范围（内部会按需重建消息高度与偏移表）
    UpdateScrollRange(pRender);
    if ((m_msgOffsets.size() != m_messages.size()) || (m_msgHeights.size() != m_messages.size())) {
        return;
    }

    const UiRect rcContent(rc.left, rc.top, rc.left + nContentWidth, rc.bottom);
    //裁剪到控件区域，避免消息超出边界
    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);

    const int32_t nViewTop = m_nScrollPos;
    const int32_t nViewBottom = m_nScrollPos + rc.Height();
    const int32_t nContentTop = rc.top - m_nScrollPos;
    const int32_t nDividerHeight = Dpi().GetScaleInt(kTimeDividerHeight);

    //二分定位第一条可见消息，只绘制可见区间，避免长列表每帧全量遍历
    size_t index = FindFirstVisibleIndex(nViewTop);
    for (; index < m_messages.size(); ++index) {
        if (m_msgOffsets[index] >= nViewBottom) {
            break;
        }
        const Message& message = m_messages[index];
        if (IsTimeDividerShown(index)) {
            const int32_t nDividerTop = nContentTop + m_msgOffsets[index] - nDividerHeight;
            PaintTimeDivider(pRender, message.time,
                             UiRect(rcContent.left, nDividerTop, rcContent.right, nDividerTop + nDividerHeight));
        }
        const int32_t nTop = nContentTop + m_msgOffsets[index];
        const UiRect rcMessage(rcContent.left, nTop, rcContent.right, nTop + m_msgHeights[index]);
        PaintMessage(pRender, index, message, rcMessage);
    }
    spClip.reset();
}

bool ChatMessageList::CalcBubbleLayout(IRender* pRender, const Message& message, const UiRect& rcMessage,
                                       BubbleLayout& layout)
{
    layout = BubbleLayout();
    if (pRender == nullptr) {
        return false;
    }

    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    const int32_t nPadH = Dpi().GetScaleInt(m_nBubblePaddingH);
    const int32_t nPadV = Dpi().GetScaleInt(m_nBubblePaddingV);
    const int32_t nGap = Dpi().GetScaleInt(10);

    //气泡最大宽度
    const int32_t nMaxBubbleWidth = CalcMaxBubbleWidth(rcMessage.Width());
    const int32_t nMaxTextWidth = std::max(1, nMaxBubbleWidth - 2 * nPadH);

    //文本宽度：取自然宽度与最大宽度的较小值
    int32_t nTextWidth = MeasureNaturalTextWidth(pRender, message.text, m_strTextFont);
    if (nTextWidth > nMaxTextWidth) {
        nTextWidth = nMaxTextWidth;
    }
    if (nTextWidth < 1) {
        nTextWidth = 1;
    }

    //按文本宽度换行：绘制、选区高亮、命中测试都复用这份行数据，
    //避免三方各自推算换行位置导致文字与高亮框错位。
    WrapTextLines(pRender, message.text, nTextWidth, m_strTextFont, layout.lines);
    layout.nLineHeight = GetTextLineHeight(pRender, m_strTextFont);
    const int32_t nTextHeight = static_cast<int32_t>(layout.lines.size()) * layout.nLineHeight;

    const int32_t nBubbleHeight = nTextHeight + 2 * nPadV;
    const int32_t nBubbleWidth = nTextWidth + 2 * nPadH;

    //气泡水平位置：自己在右侧，对方在左侧
    int32_t nBubbleLeft = rcMessage.left;
    if (message.side == ChatMessageSide::kSelf) {
        nBubbleLeft = rcMessage.right - nAvatarSize - nGap - nBubbleWidth;
    }
    else {
        nBubbleLeft = rcMessage.left + nAvatarSize + nGap;
    }

    layout.rcBubble = UiRect(nBubbleLeft, rcMessage.top, nBubbleLeft + nBubbleWidth, rcMessage.top + nBubbleHeight);
    layout.rcText = UiRect(layout.rcBubble.left + nPadH, layout.rcBubble.top + nPadV,
                            layout.rcBubble.right - nPadH, layout.rcBubble.bottom - nPadV);
    //头像位置：自己在右侧，对方在左侧
    if (message.side == ChatMessageSide::kSelf) {
        layout.rcAvatar = UiRect(rcMessage.right - nAvatarSize, rcMessage.top,
                                  rcMessage.right, rcMessage.top + nAvatarSize);
    }
    else {
        layout.rcAvatar = UiRect(rcMessage.left, rcMessage.top,
                                  rcMessage.left + nAvatarSize, rcMessage.top + nAvatarSize);
    }
    layout.nTextWidth = nTextWidth;
    layout.nTextHeight = nTextHeight;

    //状态文字：仅自己发送的消息显示，绘制在气泡下方、与气泡右边缘对齐
    layout.bShowStatus = false;
    if (m_bShowStatus && (message.side == ChatMessageSide::kSelf) &&
        (message.status != ChatMessageStatus::kNone)) {
        const DString strStatus = GetStatusText(message.status);
        if (!strStatus.empty()) {
            const int32_t nStatusWidth = MeasureNaturalTextWidth(pRender, strStatus, m_strStatusFont);
            if (nStatusWidth > 0) {
                const int32_t nStatusHeight = MeasureTextHeight(pRender, strStatus, nStatusWidth, m_strStatusFont);
                if (nStatusHeight > 0) {
                    const int32_t nStatusGap = Dpi().GetScaleInt(kStatusGap);
                    layout.rcStatus = UiRect(layout.rcBubble.right - nStatusWidth,
                                             layout.rcBubble.bottom + nStatusGap,
                                             layout.rcBubble.right,
                                             layout.rcBubble.bottom + nStatusGap + nStatusHeight);
                    layout.bShowStatus = true;
                }
            }
        }
    }
    layout.bValid = true;
    return true;
}

int32_t ChatMessageList::CalcMaxBubbleWidth(int32_t nContentWidth) const
{
    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    const int32_t nGap = Dpi().GetScaleInt(10);
    int32_t nMaxBubbleWidth = Dpi().GetScaleInt(m_nBubbleMaxWidth);
    //受控件宽度限制（减头像、间距、边距）
    const int32_t nAvailable = nContentWidth - nAvatarSize - nGap - Dpi().GetScaleInt(24);
    if (nMaxBubbleWidth > nAvailable) {
        nMaxBubbleWidth = std::max(nAvailable, Dpi().GetScaleInt(80));
    }
    return std::max(nMaxBubbleWidth, Dpi().GetScaleInt(40));
}

int32_t ChatMessageList::MeasureNaturalTextWidth(IRender* pRender, const DString& strText,
                                                  const DString& strFontId) const
{
    if ((pRender == nullptr) || strText.empty()) {
        return 0;
    }
    MeasureStringParam measureParam;
    measureParam.pFont = GetIFontById(strFontId);
    measureParam.uFormat = TEXT_SINGLELINE;
    return pRender->MeasureString(strText, measureParam).Width();
}

int32_t ChatMessageList::CalcMessageHeight(IRender* pRender, const Message& message, int32_t nContentWidth)
{
    if (pRender == nullptr) {
        return 0;
    }
    if (message.side == ChatMessageSide::kSystem || message.type == ChatMessageType::kSystem) {
        //系统提示：单行小字
        return MeasureTextHeight(pRender, message.text, nContentWidth, m_strTimeFont);
    }

    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    UiRect rcMessage(0, 0, nContentWidth, 0);
    BubbleLayout layout;
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout)) {
        return nAvatarSize;
    }
    int32_t nHeight = std::max(nAvatarSize, layout.rcBubble.Height());
    if (layout.bShowStatus) {
        //状态文字在气泡下方，消息高度需要覆盖到状态行底部
        nHeight = std::max(nHeight, layout.rcStatus.bottom - rcMessage.top);
    }
    return nHeight;
}

int32_t ChatMessageList::CalcContentWidth() const
{
    return CalcContentWidthFor(GetRect());
}

int32_t ChatMessageList::CalcContentWidthFor(const UiRect& rcControl) const
{
    UiRect rc = rcControl;
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    int32_t nWidth = rc.Width();
    //固定预留滚动条宽度，避免内容宽度随滚动条出现/消失而跳变
    ScrollBar* pVScrollBar = const_cast<ChatMessageList*>(this)->GetVScrollBar();
    if ((pVScrollBar != nullptr) && !GetScrollBarFloat()) {
        nWidth -= pVScrollBar->GetFixedWidth().GetInt32();
    }
    //气泡与滚动条之间留出间距，否则气泡会紧贴滚动条，视觉上过于拥挤
    nWidth -= Dpi().GetScaleInt(kContentScrollbarGap);
    return std::max(0, nWidth);
}

void ChatMessageList::UpdateScrollRange(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    //内容宽度始终预留滚动条宽度。
    //注意：这里不能用 ScrollBar::IsValid() 判断是否扣除——IsValid 取决于滚动范围，
    //而滚动范围又取决于内容高度，内容高度又取决于内容宽度，会形成循环依赖，
    //导致宽度反复跳变、缓存反复失效。这里固定预留，宽度保持恒定。
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth <= 0) {
        return;
    }

    //高度与偏移表按需计算：宽度未变、无消息变更时直接复用缓存
    if (!EnsureLayout(pRender, nContentWidth)) {
        return;
    }

    //只同步占位控件高度。滚动范围与滑块位置统一由 ScrollBox 根据占位控件推导，
    //这里不再手动设置，避免两套范围互相覆盖导致滑块位置和实际内容对不上。
    UpdateContentPlaceholder(m_nContentHeight);

    //框架可能已经钳位过滚动位置，自绘偏移量跟随它
    const int32_t nScrollPos = static_cast<int32_t>(BaseClass::GetScrollPos().cy);
    if (nScrollPos != m_nScrollPos) {
        m_nScrollPos = nScrollPos;
    }
}

void ChatMessageList::PaintMessage(IRender* pRender, size_t nMessageIndex, const Message& message,
                                   const UiRect& rcMessage)
{
    if (message.side == ChatMessageSide::kSystem || message.type == ChatMessageType::kSystem) {
        //系统提示：居中小字
        UiRect rcText = rcMessage;
        DrawChatText(pRender, message.text, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                     m_strSystemTextColor, m_strTimeFont);
        return;
    }

    BubbleLayout layout;
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout)) {
        return;
    }

    //头像
    PaintAvatar(pRender, layout.rcAvatar, message.avatarText);

    //气泡底色
    UiColor colorBubble = GetUiColor(message.side == ChatMessageSide::kSelf ?
                                     m_strBubbleColorSelf : m_strBubbleColorOther);
    if (colorBubble.IsEmpty()) {
        colorBubble = UiColor(0xFFFFFFFF);
    }
    const int32_t nRadius = Dpi().GetScaleInt(m_nBubbleRadius);
    pRender->FillRoundRect(UiRectF(static_cast<float>(layout.rcBubble.left), static_cast<float>(layout.rcBubble.top),
                                   static_cast<float>(layout.rcBubble.right), static_cast<float>(layout.rcBubble.bottom)),
                            static_cast<float>(nRadius), static_cast<float>(nRadius), colorBubble);

    //选中区域背景：必须画在气泡底色之上，否则会被气泡盖住
    PaintSelection(pRender, nMessageIndex, layout);

    //气泡文字：选区部分单独用选中文字色绘制
    DString strTextColor = (message.side == ChatMessageSide::kSelf) ?
                           m_strTextColorSelf : m_strTextColorOther;
    DrawMessageText(pRender, nMessageIndex, message, layout, strTextColor);

    //状态文字：发送中/发送失败/已送达/已读
    if (layout.bShowStatus) {
        PaintStatus(pRender, message, layout);
    }
}

void ChatMessageList::PaintStatus(IRender* pRender, const Message& message, const BubbleLayout& layout)
{
    if (!layout.bShowStatus) {
        return;
    }
    const DString strStatus = GetStatusText(message.status);
    if (strStatus.empty()) {
        return;
    }
    DrawChatText(pRender, strStatus, layout.rcStatus,
                 TEXT_RIGHT | TEXT_TOP | TEXT_SINGLELINE, GetStatusColor(message.status), m_strStatusFont);
}

void ChatMessageList::DrawMessageText(IRender* pRender, size_t nMessageIndex, const Message& message,
                                       const BubbleLayout& layout, const DString& strTextColor)
{
    const DString& strText = message.text;
    if (strText.empty() || layout.lines.empty() || (layout.nLineHeight <= 0)) {
        return;
    }
    const int32_t nTextLen = static_cast<int32_t>(strText.length());

    //本消息中被选中的字符区间（没有选中内容时按空区间处理）
    bool bHasSel = m_bHasSelection && (nMessageIndex >= m_nSelStartIndex) && (nMessageIndex <= m_nSelEndIndex);
    int32_t nSelStart = nTextLen;
    int32_t nSelEnd = nTextLen;
    if (bHasSel) {
        nSelStart = (nMessageIndex == m_nSelStartIndex) ? std::max(0, m_nSelStartChar) : 0;
        nSelEnd = (nMessageIndex == m_nSelEndIndex) ? std::min(nTextLen, m_nSelEndChar) : nTextLen;
        bHasSel = (nSelEnd > nSelStart);
    }

    //逐行绘制：每行的首尾两段用普通文字色，选中段用选中文字色。
    //注意：必须按行绘制，不能把整段文本当单行文本绘制，否则多行文本会被画乱。
    const int32_t nLineCount = static_cast<int32_t>(layout.lines.size());
    for (int32_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        const TextLine& line = layout.lines[nLineIndex];
        if (line.nEnd <= line.nStart) {
            continue;
        }
        const int32_t nTop = layout.rcText.top + nLineIndex * layout.nLineHeight;
        const UiRect rcLine(layout.rcText.left, nTop, layout.rcText.right, nTop + layout.nLineHeight);

        //本行与选区的交集
        const int32_t nLineSelStart = bHasSel ? std::max(line.nStart, nSelStart) : line.nEnd;
        const int32_t nLineSelEnd = bHasSel ? std::min(line.nEnd, nSelEnd) : line.nEnd;
        if (nLineSelEnd <= nLineSelStart) {
            DrawChatText(pRender, strText.substr(static_cast<size_t>(line.nStart),
                                                 static_cast<size_t>(line.nEnd - line.nStart)),
                         rcLine, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, strTextColor, m_strTextFont);
            continue;
        }

        const int32_t nX1 = MeasureTextRangeWidth(pRender, strText, line.nStart, nLineSelStart, m_strTextFont);
        const int32_t nX2 = MeasureTextRangeWidth(pRender, strText, line.nStart, nLineSelEnd, m_strTextFont);

        //选中前段（普通色）
        if (nLineSelStart > line.nStart) {
            UiRect rcHead = rcLine;
            rcHead.right = rcHead.left + nX1;
            DrawChatText(pRender, strText.substr(static_cast<size_t>(line.nStart),
                                                 static_cast<size_t>(nLineSelStart - line.nStart)),
                         rcHead, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, strTextColor, m_strTextFont);
        }
        //选中段（选中文字色）
        {
            const UiRect rcSel(rcLine.left + nX1, rcLine.top, rcLine.left + nX2 + 1, rcLine.bottom);
            DrawChatText(pRender, strText.substr(static_cast<size_t>(nLineSelStart),
                                                 static_cast<size_t>(nLineSelEnd - nLineSelStart)),
                         rcSel, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE,
                         m_strSelectionTextColor, m_strTextFont);
        }
        //选中后段（普通色）
        if (nLineSelEnd < line.nEnd) {
            const UiRect rcTail(rcLine.left + nX2, rcLine.top, rcLine.right, rcLine.bottom);
            DrawChatText(pRender, strText.substr(static_cast<size_t>(nLineSelEnd),
                                                 static_cast<size_t>(line.nEnd - nLineSelEnd)),
                         rcTail, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, strTextColor, m_strTextFont);
        }
    }
}

void ChatMessageList::PaintSelection(IRender* pRender, size_t nMessageIndex, const BubbleLayout& layout)
{
    if (!m_bHasSelection || !layout.bValid || layout.lines.empty() || (layout.nLineHeight <= 0)) {
        return;
    }
    if ((nMessageIndex < m_nSelStartIndex) || (nMessageIndex > m_nSelEndIndex)) {
        return;
    }

    //当前消息中被选中的字符区间
    const DString& strText = m_messages[nMessageIndex].text;
    const int32_t nTextLen = static_cast<int32_t>(strText.length());
    int32_t nStart = (nMessageIndex == m_nSelStartIndex) ? m_nSelStartChar : 0;
    int32_t nEnd = (nMessageIndex == m_nSelEndIndex) ? m_nSelEndChar : nTextLen;
    nStart = std::max(0, nStart);
    nEnd = std::min(nTextLen, nEnd);
    if (nStart >= nEnd) {
        return;
    }

    UiColor colorBk = GetUiColor(m_strSelectionBkColor);
    if (colorBk.IsEmpty()) {
        colorBk = UiColor(0xFF3399FF);
    }

    //逐行绘制高亮背景：行数据与文字绘制共用 layout.lines，
    //这样高亮框与文字行严格对齐，不会出现跨行或超出气泡的色块。
    const int32_t nLineCount = static_cast<int32_t>(layout.lines.size());
    for (int32_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        const TextLine& line = layout.lines[nLineIndex];
        const int32_t nLineSelStart = std::max(line.nStart, nStart);
        const int32_t nLineSelEnd = std::min(line.nEnd, nEnd);
        if (nLineSelEnd <= nLineSelStart) {
            continue;
        }
        const int32_t nX1 = MeasureTextRangeWidth(pRender, strText, line.nStart, nLineSelStart, m_strTextFont);
        const int32_t nX2 = MeasureTextRangeWidth(pRender, strText, line.nStart, nLineSelEnd, m_strTextFont);
        const int32_t nTop = layout.rcText.top + nLineIndex * layout.nLineHeight;
        const UiRectF rcSel(static_cast<float>(layout.rcText.left + nX1), static_cast<float>(nTop),
                            static_cast<float>(layout.rcText.left + nX2), static_cast<float>(nTop + layout.nLineHeight));
        pRender->FillRect(rcSel, colorBk);
    }
}

void ChatMessageList::PaintTimeDivider(IRender* pRender, const DString& strTime, const UiRect& rcDivider)
{
    if (strTime.empty()) {
        return;
    }
    DrawChatText(pRender, strTime, rcDivider, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                 m_strSystemTextColor, m_strTimeFont);
}

void ChatMessageList::PaintAvatar(IRender* pRender, const UiRect& rcAvatar, const DString& strText)
{
    if (rcAvatar.Width() <= 0 || rcAvatar.Height() <= 0) {
        return;
    }
    UiColor colorAvatar = GetAvatarColor(strText);
    //圆形头像：以矩形中心为圆心绘制
    const UiPointF ptCenter(static_cast<float>((rcAvatar.left + rcAvatar.right) / 2),
                            static_cast<float>((rcAvatar.top + rcAvatar.bottom) / 2));
    const float fRadius = static_cast<float>(rcAvatar.Width()) / 2.0f;
    pRender->FillCircle(ptCenter, fRadius, colorAvatar);

    if (strText.empty()) {
        return;
    }
    //取第一个字符绘制在圆形中心
    const DString strOne = strText.substr(0, 1);
    UiColor colorText = GetUiColor(m_strAvatarTextColor);
    if (colorText.IsEmpty()) {
        colorText = UiColor(0xFFFFFFFF);
    }
    DrawChatText(pRender, strOne, rcAvatar, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                 m_strAvatarTextColor, m_strAvatarFont);
}

UiColor ChatMessageList::GetAvatarColor(const DString& strText) const
{
    //根据文字内容生成稳定的颜色，同一个人始终是同一个颜色
    static const UiColor s_colors[] = {
        UiColor(0xFF5B9BF8), UiColor(0xFF7ED3A0), UiColor(0xFFF5A623),
        UiColor(0xFFB07CE8), UiColor(0xFFEB6F6F), UiColor(0xFF3BC9DB),
        UiColor(0xFFF08A5D), UiColor(0xFF8C9EFF)
    };
    if (strText.empty()) {
        return s_colors[0];
    }
    uint32_t uHash = 0;
    for (size_t i = 0; i < strText.length(); ++i) {
        uHash = uHash * 31 + static_cast<uint32_t>(strText[i]);
    }
    return s_colors[uHash % (sizeof(s_colors) / sizeof(s_colors[0]))];
}

int32_t ChatMessageList::MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth,
                                           const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return 0;
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(strFontId);
    param.rectSize = nWidth;
    UiRect rc = pRender->MeasureString(strText, param);
    return rc.Height();
}

int32_t ChatMessageList::MeasureTextPrefixWidth(IRender* pRender, const DString& strText, int32_t nCharCount,
                                                const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty() || (nCharCount <= 0)) {
        return 0;
    }
    if (nCharCount >= static_cast<int32_t>(strText.length())) {
        return MeasureNaturalTextWidth(pRender, strText, strFontId);
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = TEXT_SINGLELINE;
    const DString strPrefix = strText.substr(0, static_cast<size_t>(nCharCount));
    return pRender->MeasureString(strPrefix, param).Width();
}

int32_t ChatMessageList::MeasureTextRangeWidth(IRender* pRender, const DString& strText, int32_t nStart, int32_t nEnd,
                                                const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty() || (nEnd <= nStart)) {
        return 0;
    }
    const int32_t nStartWidth = MeasureTextPrefixWidth(pRender, strText, nStart, strFontId);
    const int32_t nEndWidth = MeasureTextPrefixWidth(pRender, strText, nEnd, strFontId);
    return std::max(0, nEndWidth - nStartWidth);
}

int32_t ChatMessageList::GetTextLineHeight(IRender* pRender, const DString& strFontId)
{
    if (pRender == nullptr) {
        return 0;
    }
    //用几个典型字符测量单行高度（不能用整段文本测量，那样得到的是多行文本的总高度）
    const int32_t nHeight = MeasureTextHeight(pRender, _T("国Ag"), 4096, strFontId);
    return (nHeight > 0) ? nHeight : Dpi().GetScaleInt(16);
}

void ChatMessageList::WrapTextLines(IRender* pRender, const DString& strText, int32_t nMaxWidth,
                                     const DString& strFontId, std::vector<TextLine>& lines)
{
    lines.clear();
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    const int32_t nTextLen = static_cast<int32_t>(strText.length());
    const int32_t nWidth = std::max(1, nMaxWidth);

    int32_t nParaStart = 0;
    while (nParaStart <= nTextLen) {
        //先按显式换行符切出段落
        int32_t nParaEnd = nParaStart;
        while ((nParaEnd < nTextLen) && (strText[nParaEnd] != L'\n') && (strText[nParaEnd] != L'\r')) {
            ++nParaEnd;
        }

        if (nParaEnd == nParaStart) {
            //空行（含文本以换行开头/结尾的情况）
            lines.push_back(TextLine{ nParaStart, nParaStart });
        }
        else {
            int32_t nLineStart = nParaStart;
            while (nLineStart < nParaEnd) {
                //二分查找本行最多能容纳的字符数
                int32_t nLow = nLineStart + 1;
                int32_t nHigh = nParaEnd;
                int32_t nBest = nLineStart + 1;
                while (nLow <= nHigh) {
                    const int32_t nMid = (nLow + nHigh) / 2;
                    if (MeasureTextRangeWidth(pRender, strText, nLineStart, nMid, strFontId) <= nWidth) {
                        nBest = nMid;
                        nLow = nMid + 1;
                    }
                    else {
                        nHigh = nMid - 1;
                    }
                }
                int32_t nLineEnd = nBest;
                //需要折行时优先在空格/连字符处断开，避免把英文单词从中间截断
                if (nLineEnd < nParaEnd) {
                    for (int32_t nIndex = nLineEnd; nIndex > nLineStart; --nIndex) {
                        const wchar_t ch = strText[nIndex - 1];
                        if ((ch == L' ') || (ch == L'\t') || (ch == L'-')) {
                            nLineEnd = nIndex;
                            break;
                        }
                    }
                }
                lines.push_back(TextLine{ nLineStart, nLineEnd });
                nLineStart = nLineEnd;
            }
        }

        if (nParaEnd >= nTextLen) {
            break;
        }
        //跳过换行符（兼容 CRLF）
        int32_t nNext = nParaEnd;
        while ((nNext < nTextLen) && ((strText[nNext] == L'\n') || (strText[nNext] == L'\r'))) {
            ++nNext;
        }
        if (nNext >= nTextLen) {
            //文本以换行结尾：补一个空行，与常见编辑器的行为一致
            lines.push_back(TextLine{ nTextLen, nTextLen });
            break;
        }
        nParaStart = nNext;
    }
}

int32_t ChatMessageList::HitTestTextChar(IRender* pRender, const DString& strText, const BubbleLayout& layout,
                                          const UiPoint& ptMouse)
{
    if ((pRender == nullptr) || strText.empty() || layout.lines.empty() || (layout.nLineHeight <= 0)) {
        return 0;
    }
    const int32_t nLineCount = static_cast<int32_t>(layout.lines.size());

    //先按纵向位置定位到行，再在行内按横向位置二分定位字符
    int32_t nLineIndex = (ptMouse.y - layout.rcText.top) / layout.nLineHeight;
    if (nLineIndex < 0) {
        nLineIndex = 0;
    }
    if (nLineIndex >= nLineCount) {
        nLineIndex = nLineCount - 1;
    }
    const TextLine& line = layout.lines[nLineIndex];

    const int32_t nOffsetX = ptMouse.x - layout.rcText.left;
    if (nOffsetX <= 0) {
        return line.nStart;
    }
    if (nOffsetX >= MeasureTextRangeWidth(pRender, strText, line.nStart, line.nEnd, m_strTextFont)) {
        return line.nEnd;
    }
    int32_t nLow = line.nStart;
    int32_t nHigh = line.nEnd;
    while (nLow < nHigh) {
        const int32_t nMid = (nLow + nHigh) / 2;
        if (MeasureTextRangeWidth(pRender, strText, line.nStart, nMid, m_strTextFont) < nOffsetX) {
            nLow = nMid + 1;
        }
        else {
            nHigh = nMid;
        }
    }
    return nLow;
}

void ChatMessageList::DrawChatText(IRender* pRender, const DString& strText, const UiRect& rc,
                                   uint32_t uFormat, const DString& strColor, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    UiColor color = GetUiColor(strColor);
    if (color.IsEmpty()) {
        color = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = color;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

bool ChatMessageList::HitTestMessageClamped(const UiPoint& ptMouse, size_t& nMessageIndex, int32_t& nCharIndex)
{
    nMessageIndex = 0;
    nCharIndex = 0;
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if ((pRender == nullptr) || m_messages.empty()) {
        return false;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (!EnsureLayout(pRender, CalcContentWidth())) {
        return false;
    }

    const int32_t nOffset = ptMouse.y - rc.top + m_nScrollPos;
    size_t index = FindFirstVisibleIndex(nOffset);
    if (index >= m_messages.size()) {
        //在最后一条消息之后：钳位到最后一条消息的末尾
        index = m_messages.size() - 1;
        nMessageIndex = index;
        nCharIndex = static_cast<int32_t>(m_messages[index].text.length());
        return true;
    }
    if (nOffset < m_msgOffsets[index]) {
        //落在时间分隔条或消息间距上：归到上一条消息的末尾；第一条之前则归到开头
        if (index == 0) {
            nMessageIndex = 0;
            nCharIndex = 0;
            return true;
        }
        --index;
        nMessageIndex = index;
        nCharIndex = static_cast<int32_t>(m_messages[index].text.length());
        return true;
    }

    nMessageIndex = index;
    const Message& message = m_messages[index];
    if ((message.side == ChatMessageSide::kSystem) || (message.type == ChatMessageType::kSystem)) {
        return true;
    }

    UiRect rcMessage;
    BubbleLayout layout;
    if (!GetMessageRect(index, rcMessage) || !CalcBubbleLayout(pRender, message, rcMessage, layout)) {
        return true;
    }
    nCharIndex = HitTestTextChar(pRender, message.text, layout, ptMouse);
    return true;
}

bool ChatMessageList::GetMessageTextRect(IRender* pRender, size_t nMessageIndex, UiRect& rcText)
{
    if ((pRender == nullptr) || (nMessageIndex >= m_messages.size())) {
        return false;
    }
    const Message& message = m_messages[nMessageIndex];
    if (message.side == ChatMessageSide::kSystem || message.type == ChatMessageType::kSystem) {
        return false;
    }
    if (!EnsureLayout(pRender, CalcContentWidth())) {
        return false;
    }
    UiRect rcMessage;
    if (!GetMessageRect(nMessageIndex, rcMessage)) {
        return false;
    }
    BubbleLayout layout;
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout)) {
        return false;
    }
    rcText = layout.rcText;
    return true;
}

bool ChatMessageList::HitTestMessage(const UiPoint& ptMouse, size_t& nMessageIndex,
                                     ChatMessageHitArea& hitArea, int32_t& nCharIndex)
{
    nMessageIndex = 0;
    hitArea = ChatMessageHitArea::kNone;
    nCharIndex = -1;

    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return false;
    }

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (!rc.ContainsPt(ptMouse)) {
        return false;
    }
    const int32_t nContentWidth = CalcContentWidth();
    if (!EnsureLayout(pRender, nContentWidth)) {
        return false;
    }

    //把鼠标位置换算成"相对内容顶部"的偏移，用偏移表二分定位命中的消息
    const int32_t nOffset = ptMouse.y - rc.top + m_nScrollPos;
    size_t index = FindFirstVisibleIndex(nOffset);
    if (index >= m_messages.size()) {
        return false;
    }
    if (nOffset < m_msgOffsets[index]) {
        //落在时间分隔条或消息之间的间距上
        return false;
    }
    nMessageIndex = index;

    const Message& message = m_messages[index];
    if ((message.side == ChatMessageSide::kSystem) || (message.type == ChatMessageType::kSystem)) {
        //系统提示没有可交互区域
        return true;
    }

    UiRect rcMessage;
    if (!GetMessageRect(index, rcMessage)) {
        return false;
    }
    BubbleLayout layout;
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout)) {
        return true;
    }

    //状态文字区域优先级最高：发送失败时点击这里是"重试"
    if (layout.bShowStatus && layout.rcStatus.ContainsPt(ptMouse)) {
        hitArea = ChatMessageHitArea::kStatus;
        return true;
    }
    if (layout.rcAvatar.ContainsPt(ptMouse)) {
        hitArea = ChatMessageHitArea::kAvatar;
        return true;
    }
    if (layout.rcBubble.ContainsPt(ptMouse)) {
        hitArea = ChatMessageHitArea::kBubble;
        if (layout.rcText.ContainsPt(ptMouse)) {
            //按行定位字符索引（UTF-16 码元，用于选区）
            nCharIndex = HitTestTextChar(pRender, message.text, layout, ptMouse);
        }
        return true;
    }
    //消息矩形内但不在气泡/头像上（左右留白），不算命中
    hitArea = ChatMessageHitArea::kNone;
    return false;
}

bool ChatMessageList::ButtonDown(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseButtonDown) {
        const UiPoint pt = msg.ptMouse;
        size_t nMessageIndex = 0;
        ChatMessageHitArea hitArea = ChatMessageHitArea::kNone;
        int32_t nCharIndex = -1;
        const bool bHit = HitTestMessage(pt, nMessageIndex, hitArea, nCharIndex);

        //记录点击目标：抬起时若没有发生拖选，则触发点击回调
        m_bPendingClick = bHit && (hitArea != ChatMessageHitArea::kNone);
        m_nClickIndex = nMessageIndex;
        m_eClickArea = hitArea;

        if (m_bEnableSelection && bHit && (hitArea == ChatMessageHitArea::kBubble) && (nCharIndex >= 0)) {
            //记录鼠标按下位置，移动超过阈值后才开始拖选，避免把普通点击误判为拖选
            m_ptMouseDown = pt;
            m_bSelecting = false;
            m_bPendingSelect = true;
            //锚点固定为按下的位置，拖选过程中不随移动方向改变
            m_nAnchorIndex = nMessageIndex;
            m_nAnchorChar = nCharIndex;
            m_nSelStartIndex = nMessageIndex;
            m_nSelEndIndex = nMessageIndex;
            m_nSelStartChar = nCharIndex;
            m_nSelEndChar = nCharIndex;
            m_bHasSelection = false;
            //获取焦点，这样 Ctrl+C / Ctrl+A 等快捷键可以派发到本控件
            SetFocus();
            Invalidate();
            return true;
        }
        m_bPendingSelect = false;
        m_bSelecting = false;
        //点击空白处清除选区
        if (m_bHasSelection && !bHit) {
            ClearSelection();
            Invalidate();
        }
    }
    return BaseClass::ButtonDown(msg);
}

bool ChatMessageList::ButtonUp(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseButtonUp) {
        //拖选过程中的抬起不算点击
        const bool bWasSelecting = m_bSelecting;
        m_bPendingSelect = false;
        m_bSelecting = false;

        if (m_bPendingClick && !bWasSelecting) {
            m_bPendingClick = false;
            //确认鼠标抬起时仍落在按下时的同一条消息、同一个区域，避免误触发
            size_t nMessageIndex = 0;
            ChatMessageHitArea hitArea = ChatMessageHitArea::kNone;
            int32_t nCharIndex = -1;
            if (HitTestMessage(msg.ptMouse, nMessageIndex, hitArea, nCharIndex) &&
                (nMessageIndex == m_nClickIndex) && (hitArea == m_eClickArea) &&
                (hitArea != ChatMessageHitArea::kNone) && (m_msgClickCallback != nullptr)) {
                ChatMessageEvent messageEvent;
                messageEvent.nIndex = nMessageIndex;
                messageEvent.id = m_messages[nMessageIndex].id;
                messageEvent.status = m_messages[nMessageIndex].status;
                messageEvent.hitArea = hitArea;
                messageEvent.ptMouse = msg.ptMouse;
                m_msgClickCallback(messageEvent);
            }
        }
        m_bPendingClick = false;
    }
    return BaseClass::ButtonUp(msg);
}

bool ChatMessageList::MouseMove(const EventArgs& msg)
{
    if (m_bPendingSelect || m_bSelecting) {
        const UiPoint pt = msg.ptMouse;
        //移动超过阈值后才真正开始拖选
        if (m_bPendingSelect) {
            if ((std::abs(pt.x - m_ptMouseDown.x) <= kSelectMinMovePixels) &&
                (std::abs(pt.y - m_ptMouseDown.y) <= kSelectMinMovePixels)) {
                return true;
            }
            m_bPendingSelect = false;
            m_bSelecting = true;
        }

        size_t nMessageIndex = 0;
        int32_t nCharIndex = 0;
        //拖选使用钳位版本：鼠标穿过消息之间的间隙时，选区仍然连续
        if (HitTestMessageClamped(pt, nMessageIndex, nCharIndex)) {
            //锚点保持不变，每次按"锚点 + 当前位置"重算选区区间。
            //注意：不能把移动端交换成锚点，否则来回拖动时选区会被越拖越小。
            if ((nMessageIndex < m_nAnchorIndex) ||
                ((nMessageIndex == m_nAnchorIndex) && (nCharIndex < m_nAnchorChar))) {
                m_nSelStartIndex = nMessageIndex;
                m_nSelStartChar = nCharIndex;
                m_nSelEndIndex = m_nAnchorIndex;
                m_nSelEndChar = m_nAnchorChar;
            }
            else {
                m_nSelStartIndex = m_nAnchorIndex;
                m_nSelStartChar = m_nAnchorChar;
                m_nSelEndIndex = nMessageIndex;
                m_nSelEndChar = nCharIndex;
            }
            m_bHasSelection = (m_nSelStartIndex != m_nSelEndIndex) ||
                              (m_nSelStartChar != m_nSelEndChar);
            Invalidate();
        }
        return true;
    }
    return BaseClass::MouseMove(msg);
}

bool ChatMessageList::MouseWheel(const EventArgs& msg)
{
    return BaseClass::MouseWheel(msg);
}

bool ChatMessageList::MouseEnter(const EventArgs& msg)
{
    const bool bRet = BaseClass::MouseEnter(msg);
    //滚动条为纯自绘内容，无法依赖 ScrollBox 的子控件驱动显示，
    //这里在鼠标进入时直接显示（不用淡入动画，避免动画未完成时不可见）。
    if (m_bAutoHideScrollbar) {
        ScrollBar* pSB = GetVScrollBar();
        if ((pSB != nullptr) && pSB->IsValid() && !pSB->IsVisible()) {
            pSB->SetVisible(true);
        }
    }
    return bRet;
}

bool ChatMessageList::MouseLeave(const EventArgs& msg)
{
    if (m_bSelecting || m_bPendingSelect) {
        m_bSelecting = false;
        m_bPendingSelect = false;
    }
    const bool bRet = BaseClass::MouseLeave(msg);
    //鼠标离开时隐藏滚动条（auto_hide 模式）；鼠标在滚动条上时不隐藏
    if (m_bAutoHideScrollbar) {
        ScrollBar* pSB = GetVScrollBar();
        if ((pSB != nullptr) && pSB->IsVisible() && !pSB->IsMouseFocused()) {
            pSB->SetVisible(false);
        }
    }
    return bRet;
}

bool ChatMessageList::ButtonDoubleClick(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseDoubleClick) {
        const UiPoint pt = msg.ptMouse;
        size_t nMessageIndex = 0;
        ChatMessageHitArea hitArea = ChatMessageHitArea::kNone;
        int32_t nCharIndex = -1;
        if (HitTestMessage(pt, nMessageIndex, hitArea, nCharIndex)) {
            //先通知应用层：返回 true 表示已处理，跳过默认的"选中该次点击位置所在的词"
            if ((m_msgDoubleClickCallback != nullptr) && (hitArea != ChatMessageHitArea::kNone)) {
                ChatMessageEvent messageEvent;
                messageEvent.nIndex = nMessageIndex;
                messageEvent.id = m_messages[nMessageIndex].id;
                messageEvent.status = m_messages[nMessageIndex].status;
                messageEvent.hitArea = hitArea;
                messageEvent.ptMouse = pt;
                if (m_msgDoubleClickCallback(messageEvent)) {
                    return true;
                }
            }
            if (!m_bEnableSelection || (hitArea != ChatMessageHitArea::kBubble) || (nCharIndex < 0)) {
                return BaseClass::ButtonDoubleClick(msg);
            }

            //双击选词：选中光标两侧的非空白字符
            const DString& strText = m_messages[nMessageIndex].text;
            const int32_t nTextLen = static_cast<int32_t>(strText.length());
            int32_t nStart = nCharIndex;
            int32_t nEnd = nCharIndex;
            auto isWordChar = [](wchar_t ch) {
                return (ch != L' ') && (ch != L'\t') && (ch != L'\n') && (ch != L'\r');
            };
            while ((nStart > 0) && isWordChar(strText[nStart - 1])) {
                --nStart;
            }
            while ((nEnd < nTextLen) && isWordChar(strText[nEnd])) {
                ++nEnd;
            }
            if (nEnd > nStart) {
                m_bHasSelection = true;
                m_nAnchorIndex = nMessageIndex;
                m_nAnchorChar = nStart;
                m_nSelStartIndex = nMessageIndex;
                m_nSelEndIndex = nMessageIndex;
                m_nSelStartChar = nStart;
                m_nSelEndChar = nEnd;
                //获取焦点，使 Ctrl+C 能复制选中的文本
                SetFocus();
                Invalidate();
            }
            return true;
        }
    }
    return BaseClass::ButtonDoubleClick(msg);
}

void ChatMessageList::HandleEvent(const EventArgs& msg)
{
    if (msg.eventType == kEventKeyDown) {
        //Ctrl+C 复制选中文本
        if (IsKeyDown(msg, ModifierKey::kControl) && (msg.vkCode == kVK_C) && m_bHasSelection) {
            CopySelectedText();
            return;
        }
        //Ctrl+A 全选
        if (IsKeyDown(msg, ModifierKey::kControl) && (msg.vkCode == kVK_A)) {
            SelectAll();
            return;
        }
        //ESC 清除选区
        if ((msg.vkCode == kVK_ESCAPE) && m_bHasSelection) {
            ClearSelection();
            Invalidate();
            return;
        }
    }
    else if (msg.eventType == kEventContextMenu) {
        //右键菜单事件：先通知应用层，返回 true 表示应用层自行处理，
        //此时不再弹出由 AttachPopupMenu 绑定的默认菜单。
        if (m_msgRightClickCallback != nullptr) {
            size_t nMessageIndex = 0;
            ChatMessageHitArea hitArea = ChatMessageHitArea::kNone;
            int32_t nCharIndex = -1;
            if (HitTestMessage(msg.ptMouse, nMessageIndex, hitArea, nCharIndex) &&
                (hitArea != ChatMessageHitArea::kNone)) {
                ChatMessageEvent messageEvent;
                messageEvent.nIndex = nMessageIndex;
                messageEvent.id = m_messages[nMessageIndex].id;
                messageEvent.status = m_messages[nMessageIndex].status;
                messageEvent.hitArea = hitArea;
                messageEvent.ptMouse = msg.ptMouse;
                if (m_msgRightClickCallback(messageEvent)) {
                    return;
                }
            }
        }
    }
    BaseClass::HandleEvent(msg);
}

bool ChatMessageList::HasSelection() const
{
    return m_bHasSelection;
}

DString ChatMessageList::GetSelectedText() const
{
    if (!m_bHasSelection) {
        return DString();
    }
    DString strResult;
    for (size_t index = m_nSelStartIndex; index <= m_nSelEndIndex && index < m_messages.size(); ++index) {
        const DString& strText = m_messages[index].text;
        int32_t nStart = 0;
        int32_t nEnd = static_cast<int32_t>(strText.length());
        if (index == m_nSelStartIndex) {
            nStart = m_nSelStartChar;
        }
        if (index == m_nSelEndIndex) {
            nEnd = m_nSelEndChar;
        }
        if (nStart < 0) {
            nStart = 0;
        }
        if (nEnd > static_cast<int32_t>(strText.length())) {
            nEnd = static_cast<int32_t>(strText.length());
        }
        if (nStart >= nEnd) {
            continue;
        }
        if (!strResult.empty()) {
            strResult += _T("\r\n");
        }
        strResult += strText.substr(static_cast<size_t>(nStart), static_cast<size_t>(nEnd - nStart));
    }
    return strResult;
}

void ChatMessageList::ClearSelection()
{
    m_bSelecting = false;
    m_bPendingSelect = false;
    m_bHasSelection = false;
    m_nAnchorIndex = 0;
    m_nAnchorChar = 0;
    m_nSelStartIndex = 0;
    m_nSelEndIndex = 0;
    m_nSelStartChar = 0;
    m_nSelEndChar = 0;
}

void ChatMessageList::SelectAll()
{
    if (m_messages.empty()) {
        return;
    }
    //从第一条有文本的消息开始，到最后一条有文本的消息结束
    size_t nFirst = m_messages.size();
    size_t nLast = 0;
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].text.empty()) {
            continue;
        }
        if (nFirst == m_messages.size()) {
            nFirst = index;
        }
        nLast = index;
    }
    if (nFirst == m_messages.size()) {
        return;
    }
    m_bHasSelection = true;
    m_nAnchorIndex = nFirst;
    m_nAnchorChar = 0;
    m_nSelStartIndex = nFirst;
    m_nSelStartChar = 0;
    m_nSelEndIndex = nLast;
    m_nSelEndChar = static_cast<int32_t>(m_messages[nLast].text.length());
    Invalidate();
}

bool ChatMessageList::SelectMessageText(size_t nIndex)
{
    if (nIndex >= m_messages.size()) {
        return false;
    }
    const DString& strText = m_messages[nIndex].text;
    if (strText.empty()) {
        return false;
    }
    m_bHasSelection = true;
    m_nAnchorIndex = nIndex;
    m_nAnchorChar = 0;
    m_nSelStartIndex = nIndex;
    m_nSelEndIndex = nIndex;
    m_nSelStartChar = 0;
    m_nSelEndChar = static_cast<int32_t>(strText.length());
    Invalidate();
    return true;
}

bool ChatMessageList::CopySelectedText()
{
    if (!m_bHasSelection) {
        return false;
    }
    const DString strText = GetSelectedText();
    if (strText.empty()) {
        return false;
    }
    return Clipboard::SetClipboardText(strText);
}

bool ChatMessageList::CopyAllText()
{
    if (m_messages.empty()) {
        return false;
    }
    DString strText;
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].text.empty()) {
            continue;
        }
        if (!strText.empty()) {
            strText += _T("\r\n");
        }
        strText += m_messages[index].text;
    }
    if (strText.empty()) {
        return false;
    }
    return Clipboard::SetClipboardText(strText);
}

bool ChatMessageList::AttachPopupMenu(const DString& menuXml,
                                       const ContextMenuActionEvent& actionCallback,
                                       const ContextMenuCreatingEvent& creatingCallback)
{
    //未指定菜单时，使用消息列表专用的右键菜单（仅"复制"与"全选"，文案已国际化）。
    //消息列表是只读的，不提供剪切/粘贴/删除等富文本菜单项。
    DString strXml = menuXml;
    if (strXml.empty()) {
        strXml = _T("public/menu/chat_message_menu.xml");
    }
    //未指定回调时，交给本控件的 OnContextMenuAction 处理内置菜单项：
    //框架只在actionCallback非空时才会把菜单点击转给应用层，
    //不包一层的话"复制/全选"点了不会有任何反应。
    ContextMenuActionEvent callback = actionCallback;
    if (callback == nullptr) {
        ChatMessageList* pThis = this;
        callback = [pThis](const DString& strItemName, size_t /*nItemIndex*/) {
            if (pThis != nullptr) {
                pThis->OnContextMenuAction(strItemName);
            }
            };
    }
    return BaseClass::AttachPopupMenu(strXml, callback, creatingCallback);
}

void ChatMessageList::OnContextMenuAction(const DString& strItemName)
{
    if (strItemName == _T("edit_menu_copy")) {
        if (m_bHasSelection) {
            CopySelectedText();
        }
        else {
            CopyAllText();
        }
        return;
    }
    if (strItemName == _T("edit_menu_sel_all")) {
        SelectAll();
        return;
    }
}

} // namespace ui
