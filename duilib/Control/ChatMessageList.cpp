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

ChatMessageList::ChatMessageList(Window* pWindow) :
    BaseClass(pWindow),
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
    m_nCachedContentHeight(-1),
    m_nCachedContentWidth(-1),
    m_bSelecting(false),
    m_bPendingSelect(false),
    m_bHasSelection(false),
    m_ptMouseDown(),
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
    BaseClass::SetPos(rc);
    //本控件为纯自绘内容，没有子控件来驱动滚动条，
    //因此在布局完成后主动计算内容高度、滚动范围，并设置滚动条的位置。
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    UpdateScrollRange(pRender);

    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        UiRect rcBar = rc;
        rcBar.Deflate(GetPadding());
        rcBar.Deflate(GetScrollBarPadding());
        const int32_t nBarWidth = pVScrollBar->GetFixedWidth().GetInt32();
        if (nBarWidth > 0) {
            UiRect rcBarPos(rcBar.right - nBarWidth, rcBar.top, rcBar.right, rcBar.bottom);
            pVScrollBar->SetPos(rcBarPos);
        }
        //注意：这里不修改滚动条的可见性。
        //本控件为纯自绘内容，没有子控件来驱动 ScrollBox 的滚动条可见性，
        //可见性改由框架的 ScrollBox::MouseEnter 机制处理（auto_hide 模式悬停显示）。
    }
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
    //消息内容变化：受影响消息的高度缓存失效，总高度重新计算
    m_nCachedContentHeight = -1;
    m_nCachedContentWidth = -1;
    Invalidate();
}

void ChatMessageList::InvalidateWidthCache()
{
    //宽度变化：所有消息高度都需要重新测量
    m_nCachedContentHeight = -1;
    m_nCachedContentWidth = -1;
    m_msgHeights.clear();
    m_nCachedLayoutWidth = -1;
    Invalidate();
}

void ChatMessageList::SyncMessageHeightCache()
{
    //消息增删后，保证高度缓存与消息数量一致（新增部分置 0 表示待计算）
    if (m_msgHeights.size() != m_messages.size()) {
        m_msgHeights.resize(m_messages.size(), 0);
    }
}

void ChatMessageList::AddMessage(const Message& message)
{
    InsertMessageInternal(m_messages.size(), message);
}

void ChatMessageList::AddMessage(ChatMessageSide side, const DString& strText,
                                 const DString& strAvatarText, const DString& strTime)
{
    Message message;
    message.side = side;
    message.type = (side == ChatMessageSide::kSystem) ? ChatMessageType::kSystem : ChatMessageType::kText;
    message.text = strText;
    message.avatarText = strAvatarText;
    message.time = strTime;
    AddMessage(message);
}

void ChatMessageList::InsertMessage(size_t index, const Message& message)
{
    if (index > m_messages.size()) {
        index = m_messages.size();
    }
    InsertMessageInternal(index, message);
}

void ChatMessageList::InsertMessageInternal(size_t index, const Message& message)
{
    Message msg = message;
    if (msg.id == 0) {
        msg.id = GenerateMessageId();
    }
    m_messages.insert(m_messages.begin() + static_cast<std::ptrdiff_t>(index), msg);
    //高度缓存同步插入（新增位置为 0，表示待计算）
    //注意：必须先把缓存扩容到插入前的消息数量，否则 insert 会越界
    if (m_msgHeights.size() < m_messages.size() - 1) {
        m_msgHeights.resize(m_messages.size() - 1, 0);
    }
    if (index <= m_msgHeights.size()) {
        m_msgHeights.insert(m_msgHeights.begin() + static_cast<std::ptrdiff_t>(index), 0);
    }
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
}

bool ChatMessageList::UpdateMessage(int64_t id, const Message& message)
{
    if (id == 0) {
        return false;
    }
    for (size_t index = 0; index < m_messages.size(); ++index) {
        if (m_messages[index].id == id) {
            Message msg = message;
            msg.id = id;//id 保持不变
            m_messages[index] = msg;
            //该条消息的高度需要重新计算
            if (index < m_msgHeights.size()) {
                m_msgHeights[index] = 0;
            }
            InvalidateLayoutCache();
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
            //高度缓存同步删除
            if (index < m_msgHeights.size()) {
                m_msgHeights.erase(m_msgHeights.begin() + static_cast<std::ptrdiff_t>(index));
            }            //删除后清理选区，避免索引失效
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
    m_nCachedLayoutWidth = -1;
    m_nScrollPos = 0;
    m_nContentHeight = 0;
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
    UpdateScrollRange(GetWindow() != nullptr ? GetWindow()->GetRender() : nullptr);
    const int32_t nMax = std::max(0, m_nContentHeight - GetVisibleHeight());
    SetChatScrollPos(nMax);
}

void ChatMessageList::SetChatScrollPos(int32_t nPos)
{
    const int32_t nMax = std::max(0, m_nContentHeight - GetVisibleHeight());
    if (nPos < 0) {
        nPos = 0;
    }
    if (nPos > nMax) {
        nPos = nMax;
    }
    if (nPos == m_nScrollPos) {
        return;
    }
    m_nScrollPos = nPos;
    //同步到框架滚动条，保证两者一致
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollPos(nPos);
    }
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

    //更新滚动范围
    UpdateScrollRange(pRender);

    //内容区宽度固定预留滚动条宽度（见 CalcContentWidth 的说明）
    ScrollBar* pVScrollBar = GetVScrollBar();
    UiRect rcContent = rc;
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth > 0) {
        rcContent.right = rcContent.left + nContentWidth;
    }
    if (rcContent.Width() <= 0) {
        return;
    }

    //裁剪到控件区域，避免消息超出边界
    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);

    int32_t y = rcContent.top - m_nScrollPos;
    const size_t nCount = m_messages.size();
    for (size_t index = 0; index < nCount; ++index) {
        const Message& message = m_messages[index];
        //时间分隔条：与上一条消息时间不同才显示
        bool bShowDivider = false;
        if (m_bShowTime && !message.time.empty()) {
            if (index == 0) {
                bShowDivider = true;
            }
            else {
                bShowDivider = (message.time != m_messages[index - 1].time);
            }
        }
        int32_t nDividerHeight = 0;
        if (bShowDivider) {
            nDividerHeight = Dpi().GetScaleInt(28);
        }
        const int32_t nMessageHeight = CalcMessageHeightCached(pRender, index, rcContent.Width());
        UiRect rcMessage(rcContent.left, y + nDividerHeight, rcContent.right, y + nDividerHeight + nMessageHeight);
        if (bShowDivider) {
            PaintTimeDivider(pRender, message.time, UiRect(rcContent.left, y, rcContent.right, y + nDividerHeight));
        }
        //仅绘制可见消息
        if (rcMessage.bottom > rc.top && rcMessage.top < rc.bottom) {
            PaintMessage(pRender, index, message, rcMessage);
        }
        y += nDividerHeight + nMessageHeight + Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    spClip.reset();
}

bool ChatMessageList::CalcBubbleLayout(IRender* pRender, const Message& message, const UiRect& rcMessage,
                                       BubbleLayout& layout, bool bCalcTextHeight)
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

    int32_t nTextHeight = 0;
    if (bCalcTextHeight) {
        nTextHeight = MeasureTextHeight(pRender, message.text, nTextWidth, m_strTextFont);
    }
    else {
        //仅计算高度时也要拿到文本高度，用于确定气泡高度
        nTextHeight = MeasureTextHeight(pRender, message.text, nTextWidth, m_strTextFont);
    }

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
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout, true)) {
        return nAvatarSize;
    }
    return std::max(nAvatarSize, layout.rcBubble.Height());
}

int32_t ChatMessageList::CalcContentWidth() const
{
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    int32_t nWidth = rc.Width();
    //固定预留滚动条宽度，避免内容宽度随滚动条出现/消失而跳变
    ScrollBar* pVScrollBar = const_cast<ChatMessageList*>(this)->GetVScrollBar();
    if ((pVScrollBar != nullptr) && !GetScrollBarFloat()) {
        nWidth -= pVScrollBar->GetFixedWidth().GetInt32();
    }
    return std::max(0, nWidth);
}

int32_t ChatMessageList::CalcMessageHeightCached(IRender* pRender, size_t nIndex, int32_t nContentWidth)
{
    if (nIndex >= m_messages.size()) {
        return 0;
    }
    //宽度变化时缓存失效，需要重新计算
    if (m_nCachedLayoutWidth != nContentWidth) {
        m_msgHeights.clear();
        m_nCachedLayoutWidth = nContentWidth;
    }
    if (nIndex >= m_msgHeights.size()) {
        m_msgHeights.resize(m_messages.size(), 0);
    }
    if (m_msgHeights[nIndex] <= 0) {
        m_msgHeights[nIndex] = CalcMessageHeight(pRender, m_messages[nIndex], nContentWidth);
    }
    return m_msgHeights[nIndex];
}

int32_t ChatMessageList::CalcTotalHeight(IRender* pRender, int32_t nContentWidth)
{
    int32_t nTotal = 0;
    const size_t nCount = m_messages.size();
    for (size_t index = 0; index < nCount; ++index) {
        const Message& message = m_messages[index];
        bool bShowDivider = false;
        if (m_bShowTime && !message.time.empty()) {
            bShowDivider = (index == 0) || (message.time != m_messages[index - 1].time);
        }
        if (bShowDivider) {
            nTotal += Dpi().GetScaleInt(28);
        }
        nTotal += CalcMessageHeightCached(pRender, index, nContentWidth);
        nTotal += Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    if (!m_messages.empty()) {
        //最后一条消息之后不保留间距
        nTotal -= Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    return nTotal;
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

    //宽度未变且缓存有效时直接复用，避免每帧重新量测全部文本
    if ((m_nCachedContentWidth == nContentWidth) && (m_nCachedContentHeight >= 0)) {
        m_nContentHeight = m_nCachedContentHeight;
    }
    else {
        m_nContentHeight = CalcTotalHeight(pRender, nContentWidth);
        m_nCachedContentWidth = nContentWidth;
        m_nCachedContentHeight = m_nContentHeight;
    }

    //把滚动范围设置到框架滚动条上
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        const int64_t nRange = std::max<int64_t>(0, m_nContentHeight - rc.Height());
        if (pVScrollBar->GetScrollRange() != nRange) {
            pVScrollBar->SetScrollRange(nRange);
        }
        if (pVScrollBar->GetScrollPos() != m_nScrollPos) {
            pVScrollBar->SetScrollPos(m_nScrollPos);
        }
    }
    //同步占位控件高度，使 ScrollBox 能推导出正确的滚动范围
    UpdateContentPlaceholder(m_nContentHeight);

    //钳位滚动位置
    const int32_t nMax = std::max(0, m_nContentHeight - rc.Height());
    if (m_nScrollPos > nMax) {
        m_nScrollPos = nMax;
    }
    if (m_nScrollPos < 0) {
        m_nScrollPos = 0;
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
    if (!CalcBubbleLayout(pRender, message, rcMessage, layout, true)) {
        return;
    }

    //头像
    PaintAvatar(pRender, layout.rcAvatar, message.avatarText);

    //选中区域背景（画在气泡底色之上、文字之下）
    PaintSelection(pRender, nMessageIndex, layout);

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

    //气泡文字：选区部分单独用选中文字色绘制
    DString strTextColor = (message.side == ChatMessageSide::kSelf) ?
                           m_strTextColorSelf : m_strTextColorOther;
    DrawMessageText(pRender, nMessageIndex, message, layout, strTextColor);
}

void ChatMessageList::DrawMessageText(IRender* pRender, size_t nMessageIndex, const Message& message,
                                       const BubbleLayout& layout, const DString& strTextColor)
{
    const DString& strText = message.text;
    if (strText.empty()) {
        return;
    }
    //无选区时整段绘制
    if (!m_bHasSelection || (nMessageIndex < m_nSelStartIndex) || (nMessageIndex > m_nSelEndIndex)) {
        DrawChatText(pRender, strText, layout.rcText, TEXT_LEFT | TEXT_TOP, strTextColor, m_strTextFont);
        return;
    }

    const int32_t nTextLen = static_cast<int32_t>(strText.length());
    int32_t nSelStart = 0;
    int32_t nSelEnd = nTextLen;
    if (nMessageIndex == m_nSelStartIndex) {
        nSelStart = m_nSelStartChar;
    }
    if (nMessageIndex == m_nSelEndIndex) {
        nSelEnd = m_nSelEndChar;
    }
    if (nSelStart < 0) {
        nSelStart = 0;
    }
    if (nSelEnd > nTextLen) {
        nSelEnd = nTextLen;
    }
    if (nSelStart >= nSelEnd) {
        DrawChatText(pRender, strText, layout.rcText, TEXT_LEFT | TEXT_TOP, strTextColor, m_strTextFont);
        return;
    }

    //选中前段（普通色）
    if (nSelStart > 0) {
        const DString strHead = strText.substr(0, static_cast<size_t>(nSelStart));
        UiRect rcHead = layout.rcText;
        rcHead.right = rcHead.left + MeasureTextPrefixWidth(pRender, strText, nSelStart, m_strTextFont);
        DrawChatText(pRender, strHead, rcHead, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, strTextColor, m_strTextFont);
    }
    //选中段（选中色）
    {
        const DString strSel = strText.substr(static_cast<size_t>(nSelStart),
                                               static_cast<size_t>(nSelEnd - nSelStart));
        const int32_t nX = MeasureTextPrefixWidth(pRender, strText, nSelStart, m_strTextFont);
        UiRect rcSel(layout.rcText.left + nX, layout.rcText.top,
                     layout.rcText.left + nX + MeasureTextPrefixWidth(pRender, strText, nSelEnd, m_strTextFont),
                     layout.rcText.bottom);
        DrawChatText(pRender, strSel, rcSel, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE,
                     m_strSelectionTextColor, m_strTextFont);
    }
    //选中后段（普通色）
    if (nSelEnd < nTextLen) {
        const DString strTail = strText.substr(static_cast<size_t>(nSelEnd));
        const int32_t nX = MeasureTextPrefixWidth(pRender, strText, nSelEnd, m_strTextFont);
        UiRect rcTail(layout.rcText.left + nX, layout.rcText.top, layout.rcText.right, layout.rcText.bottom);
        DrawChatText(pRender, strTail, rcTail, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, strTextColor, m_strTextFont);
    }
}

void ChatMessageList::PaintSelection(IRender* pRender, size_t nMessageIndex, const BubbleLayout& layout)
{
    if (!m_bHasSelection || !layout.bValid) {
        return;
    }
    if (nMessageIndex < m_nSelStartIndex || nMessageIndex > m_nSelEndIndex) {
        return;
    }

    //当前消息中被选中的字符区间
    const DString& strText = m_messages[nMessageIndex].text;
    const int32_t nTextLen = static_cast<int32_t>(strText.length());
    int32_t nStart = 0;
    int32_t nEnd = nTextLen;
    if (nMessageIndex == m_nSelStartIndex) {
        nStart = m_nSelStartChar;
    }
    if (nMessageIndex == m_nSelEndIndex) {
        nEnd = m_nSelEndChar;
    }
    if (nStart < 0) {
        nStart = 0;
    }
    if (nEnd > nTextLen) {
        nEnd = nTextLen;
    }
    if (nStart >= nEnd) {
        return;
    }

    UiColor colorBk = GetUiColor(m_strSelectionBkColor);
    if (colorBk.IsEmpty()) {
        colorBk = UiColor(0xFF3399FF);
    }

    //按显示行分段处理：文本按气泡宽度换行，需要逐行计算选区与行内字符范围
    const int32_t nTextWidth = std::max(1, layout.nTextWidth);
    int32_t nLineStart = 0;      //当前行在全文中的起始字符索引
    int32_t nLineIndex = 0;     //当前行序号
    int32_t nLineHeight = 0;    //行高
    {
        //行高：文本总高度除以总行数（总行数在下面统计），这里先用单行高度兜底
        const int32_t nSingle = MeasureTextHeight(pRender, strText, nTextWidth, m_strTextFont);
        nLineHeight = nSingle > 0 ? nSingle : Dpi().GetScaleInt(16);
    }

    int32_t nCharPos = 0;
    while (nCharPos <= nTextLen) {
        //找到本行的结束位置：换行符，或达到宽度上限
        int32_t nLineEnd = nCharPos;
        while (nLineEnd < nTextLen && strText[nLineEnd] != L'\n') {
            ++nLineEnd;
        }
        //按宽度计算本行实际能容纳的字符数
        int32_t nFitEnd = nLineEnd;
        if (nLineEnd < nTextLen) {
            //存在换行符，宽度限制也生效时取较小者
            const int32_t nWidthLimit = MeasureTextPrefixWidth(pRender, strText, nTextLen, m_strTextFont);
            if (nWidthLimit > nTextWidth && nLineEnd - nCharPos > 0) {
                //二分找出在 nTextWidth 内能容纳的字符数
                int32_t nLow = nCharPos;
                int32_t nHigh = nLineEnd;
                while (nLow < nHigh) {
                    const int32_t nMid = (nLow + nHigh + 1) / 2;
                    const int32_t nW = MeasureTextPrefixWidth(pRender, strText, nMid, m_strTextFont) -
                        MeasureTextPrefixWidth(pRender, strText, nCharPos, m_strTextFont);
                    if (nW <= nTextWidth) {
                        nLow = nMid;
                    }
                    else {
                        nHigh = nMid - 1;
                    }
                }
                nFitEnd = nLow;
            }
        }

        //本行与选区的交集
        const int32_t nSelStart = std::max(nStart, nCharPos);
        const int32_t nSelEnd = std::min(nEnd, nFitEnd);
        if (nSelEnd > nSelStart) {
            const int32_t nX1 = MeasureTextPrefixWidth(pRender, strText, nSelStart, m_strTextFont) -
                MeasureTextPrefixWidth(pRender, strText, nCharPos, m_strTextFont);
            int32_t nX2 = MeasureTextPrefixWidth(pRender, strText, nSelEnd, m_strTextFont) -
                MeasureTextPrefixWidth(pRender, strText, nCharPos, m_strTextFont);
            if (nX2 < nX1) {
                nX2 = nX1 + 1;
            }
            UiRectF rcSel(static_cast<float>(layout.rcText.left + nX1),
                          static_cast<float>(layout.rcText.top + nLineIndex * nLineHeight),
                          static_cast<float>(layout.rcText.left + nX2),
                          static_cast<float>(layout.rcText.top + (nLineIndex + 1) * nLineHeight));
            pRender->FillRect(rcSel, colorBk);
        }

        ++nLineIndex;
        if (nLineEnd >= nTextLen) {
            break;
        }
        nCharPos = nLineEnd + 1;   //跳过换行符
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

bool ChatMessageList::GetMessageTextRect(IRender* pRender, size_t nMessageIndex, UiRect& rcText)
{
    if ((pRender == nullptr) || (nMessageIndex >= m_messages.size())) {
        return false;
    }
    const Message& message = m_messages[nMessageIndex];
    if (message.side == ChatMessageSide::kSystem || message.type == ChatMessageType::kSystem) {
        return false;
    }

    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    ScrollBar* pVScrollBar = GetVScrollBar();
    UiRect rcContent = rc;
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth > 0) {
        rcContent.right = rcContent.left + nContentWidth;
    }
    if (rcContent.Width() <= 0) {
        return false;
    }

    //从上往下累加，定位目标消息
    int32_t y = rcContent.top - m_nScrollPos;
    for (size_t index = 0; index < m_messages.size(); ++index) {
        const Message& msg = m_messages[index];
        bool bShowDivider = false;
        if (m_bShowTime && !msg.time.empty()) {
            bShowDivider = (index == 0) || (msg.time != m_messages[index - 1].time);
        }
        int32_t nDividerHeight = bShowDivider ? Dpi().GetScaleInt(28) : 0;
        const int32_t nMessageHeight = CalcMessageHeightCached(pRender, index, rcContent.Width());
        UiRect rcMessage(rcContent.left, y + nDividerHeight, rcContent.right, y + nDividerHeight + nMessageHeight);
        if (index == nMessageIndex) {
            BubbleLayout layout;
            if (!CalcBubbleLayout(pRender, msg, rcMessage, layout, true)) {
                return false;
            }
            rcText = layout.rcText;
            return true;
        }
        y += nDividerHeight + nMessageHeight + Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    return false;
}

bool ChatMessageList::HitTestText(const UiPoint& ptMouse, size_t& nMessageIndex, int32_t& nCharIndex)
{
    nMessageIndex = 0;
    nCharIndex = 0;
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return false;
    }
    if (!m_bEnableSelection) {
        return false;
    }

    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    ScrollBar* pVScrollBar = GetVScrollBar();
    UiRect rcContent = rc;
    const int32_t nContentWidth = CalcContentWidth();
    if (nContentWidth > 0) {
        rcContent.right = rcContent.left + nContentWidth;
    }
    if (rcContent.Width() <= 0) {
        return false;
    }
    if (!rcContent.ContainsPt(ptMouse)) {
        return false;
    }

    int32_t y = rcContent.top - m_nScrollPos;
    for (size_t index = 0; index < m_messages.size(); ++index) {
        const Message& msg = m_messages[index];
        bool bShowDivider = false;
        if (m_bShowTime && !msg.time.empty()) {
            bShowDivider = (index == 0) || (msg.time != m_messages[index - 1].time);
        }
        int32_t nDividerHeight = bShowDivider ? Dpi().GetScaleInt(28) : 0;
        const int32_t nMessageHeight = CalcMessageHeightCached(pRender, index, rcContent.Width());
        UiRect rcMessage(rcContent.left, y + nDividerHeight, rcContent.right, y + nDividerHeight + nMessageHeight);
        y += nDividerHeight + nMessageHeight + Dpi().GetScaleInt(m_nBubbleSpacing);

        const bool bSystem = (msg.side == ChatMessageSide::kSystem) || (msg.type == ChatMessageType::kSystem);
        if (bSystem) {
            continue;
        }
        if (!rcMessage.ContainsPt(ptMouse)) {
            continue;
        }

        BubbleLayout layout;
        if (!CalcBubbleLayout(pRender, msg, rcMessage, layout, true)) {
            return false;
        }
        if (!layout.rcText.ContainsPt(ptMouse)) {
            //点在气泡内但不在文本区（内边距），视为无效命中
            return false;
        }

        //二分查找：定位字符索引
        const DString& strText = msg.text;
        const int32_t nTextLen = static_cast<int32_t>(strText.length());
        const int32_t nOffsetX = ptMouse.x - layout.rcText.left;
        int32_t nLow = 0;
        int32_t nHigh = nTextLen;
        while (nLow < nHigh) {
            const int32_t nMid = (nLow + nHigh) / 2;
            const int32_t nWidth = MeasureTextPrefixWidth(pRender, strText, nMid, m_strTextFont);
            if (nWidth < nOffsetX) {
                nLow = nMid + 1;
            }
            else {
                nHigh = nMid;
            }
        }
        nMessageIndex = index;
        nCharIndex = nLow;
        return true;
    }
    return false;
}

bool ChatMessageList::ButtonDown(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseButtonDown) {
        const UiPoint pt = msg.ptMouse;
        size_t nMessageIndex = 0;
        int32_t nCharIndex = 0;
        if (HitTestText(pt, nMessageIndex, nCharIndex)) {
            //记录鼠标按下位置，移动超过阈值后才开始拖选，避免把普通点击误判为拖选
            m_ptMouseDown = pt;
            m_bSelecting = false;
            m_bPendingSelect = true;
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
        if (m_bHasSelection) {
            ClearSelection();
            Invalidate();
        }
    }
    return BaseClass::ButtonDown(msg);
}

bool ChatMessageList::ButtonUp(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseButtonUp) {
        m_bPendingSelect = false;
        m_bSelecting = false;
    }
    return BaseClass::ButtonUp(msg);
}

bool ChatMessageList::MouseMove(const EventArgs& msg)
{
    if (m_bPendingSelect || m_bSelecting) {
        const UiPoint pt = msg.ptMouse;
        //移动超过阈值后才真正开始拖选
        if (m_bPendingSelect) {
            constexpr const int32_t kMinMovePixels = 8;
            if ((std::abs(pt.x - m_ptMouseDown.x) <= kMinMovePixels) &&
                (std::abs(pt.y - m_ptMouseDown.y) <= kMinMovePixels)) {
                return true;
            }
            m_bPendingSelect = false;
            m_bSelecting = true;
        }

        size_t nMessageIndex = 0;
        int32_t nCharIndex = 0;
        if (HitTestText(pt, nMessageIndex, nCharIndex)) {
            //保持起始位置不变，更新结束位置
            m_nSelEndIndex = nMessageIndex;
            m_nSelEndChar = nCharIndex;
            //规整选区方向：保证 start <= end
            if ((m_nSelEndIndex < m_nSelStartIndex) ||
                ((m_nSelEndIndex == m_nSelStartIndex) && (m_nSelEndChar < m_nSelStartChar))) {
                std::swap(m_nSelStartIndex, m_nSelEndIndex);
                std::swap(m_nSelStartChar, m_nSelEndChar);
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
        int32_t nCharIndex = 0;
        if (HitTestText(pt, nMessageIndex, nCharIndex)) {
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
    m_nSelStartIndex = nFirst;
    m_nSelStartChar = 0;
    m_nSelEndIndex = nLast;
    m_nSelEndChar = static_cast<int32_t>(m_messages[nLast].text.length());
    Invalidate();
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
    //未指定菜单时，使用框架自带的通用编辑菜单（复制/全选等，文案已国际化）
    DString strXml = menuXml;
    if (strXml.empty()) {
        strXml = _T("public/menu/rich_edit_menu.xml");
    }
    return BaseClass::AttachPopupMenu(strXml, actionCallback, creatingCallback);
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
