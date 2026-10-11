#include "Timeline.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ui
{

Timeline::Timeline(Window* pWindow) :
    BaseClass(pWindow),
    m_strLineColor(_T("border_control_normal")),
    m_strScrollbarTrackColor(_T("bg_scrollbar_normal")),
    m_strScrollbarThumbColor(_T("bg_scrollbar_thumb_normal")),
    m_bAutoHideScrollbar(true),
    m_bMouseHover(false),
    m_lineStyle(TimelineLineStyle::kSolid),
    m_bReverse(false),
    m_nNodeSize(12),
    m_nItemSpacing(24),
    m_bLoading(false),
    m_nScrollPos(0),
    m_nContentHeight(0),
    m_bDraggingScrollbar(false),
    m_dragStartY(0),
    m_dragStartScrollPos(0)
{
    //节点状态默认色（语义色名）
    m_nodeColors[static_cast<int>(TimelineNodeStatus::kPending)] = _T("text_muted");
    m_nodeColors[static_cast<int>(TimelineNodeStatus::kInProgress)] = _T("color_accent");
    m_nodeColors[static_cast<int>(TimelineNodeStatus::kCompleted)] = _T("color_success");
    m_nodeColors[static_cast<int>(TimelineNodeStatus::kFailed)] = _T("color_error");
    m_nodeColors[static_cast<int>(TimelineNodeStatus::kCustom)] = _T("");
}

Timeline::~Timeline()
{
}

DString Timeline::GetType() const
{
    return DUI_CTR_TIMELINE;
}

void Timeline::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("item_time")) || (strName == _T("itemtime"))) {
        m_strItemTimeIds.clear();
        ParseItemsField(strValue, 0);
    }
    else if ((strName == _T("item_title")) || (strName == _T("itemtitle"))) {
        m_strItemTitleIds.clear();
        ParseItemsField(strValue, 1);
    }
    else if ((strName == _T("item_content")) || (strName == _T("itemcontent"))) {
        m_strItemContentIds.clear();
        ParseItemsField(strValue, 2);
    }
    else if ((strName == _T("item_status")) || (strName == _T("itemstatus"))) {
        ParseItemsField(strValue, 3);
    }
    else if ((strName == _T("item_color")) || (strName == _T("itemcolor"))) {
        ParseItemsField(strValue, 4);
    }
    else if ((strName == _T("item_time_id")) || (strName == _T("itemtimeid"))) {
        m_strItemTimeIds = strValue;
        ParseItemsField(strValue, 0, true);
    }
    else if ((strName == _T("item_title_id")) || (strName == _T("itemtitleid"))) {
        m_strItemTitleIds = strValue;
        ParseItemsField(strValue, 1, true);
    }
    else if ((strName == _T("item_content_id")) || (strName == _T("itemcontentid"))) {
        m_strItemContentIds = strValue;
        ParseItemsField(strValue, 2, true);
    }
    else if ((strName == _T("node_color_pending")) || (strName == _T("nodecolorpending"))) {
        SetNodeColor(TimelineNodeStatus::kPending, strValue);
    }
    else if ((strName == _T("node_color_inprogress")) || (strName == _T("nodecolorinprogress"))) {
        SetNodeColor(TimelineNodeStatus::kInProgress, strValue);
    }
    else if ((strName == _T("node_color_completed")) || (strName == _T("nodecolorcompleted"))) {
        SetNodeColor(TimelineNodeStatus::kCompleted, strValue);
    }
    else if ((strName == _T("node_color_failed")) || (strName == _T("nodecolorfailed"))) {
        SetNodeColor(TimelineNodeStatus::kFailed, strValue);
    }
    else if ((strName == _T("line_color")) || (strName == _T("linecolor"))) {
        SetLineColor(strValue);
    }
    else if ((strName == _T("scrollbar_track_color")) || (strName == _T("scrollbartrackcolor"))) {
        SetScrollbarTrackColor(strValue);
    }
    else if ((strName == _T("scrollbar_thumb_color")) || (strName == _T("scrollbarthumbcolor"))) {
        SetScrollbarThumbColor(strValue);
    }
    else if ((strName == _T("auto_hide_scrollbar")) || (strName == _T("autohidescrollbar"))) {
        SetAutoHideScrollbar(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("line_style")) || (strName == _T("linestyle"))) {
        if (strValue == _T("dashed")) {
            SetLineStyle(TimelineLineStyle::kDashed);
        }
        else {
            SetLineStyle(TimelineLineStyle::kSolid);
        }
    }
    else if (strName == _T("reverse")) {
        SetReverse(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("node_size")) || (strName == _T("nodesize"))) {
        SetNodeSize(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("item_spacing")) || (strName == _T("itemspacing"))) {
        SetItemSpacing(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("empty_text")) || (strName == _T("emptytext"))) {
        SetEmptyText(strValue);
    }
    else if ((strName == _T("empty_text_id")) || (strName == _T("emptytextid"))) {
        SetEmptyTextId(strValue);
    }
    else if (strName == _T("loading")) {
        SetLoading(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Timeline::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //语言切换时，重新解析多语言数据绑定（item_*_id 与 empty_text_id）
    if (!m_strItemTimeIds.empty()) {
        ParseItemsField(m_strItemTimeIds, 0, true);
    }
    if (!m_strItemTitleIds.empty()) {
        ParseItemsField(m_strItemTitleIds, 1, true);
    }
    if (!m_strItemContentIds.empty()) {
        ParseItemsField(m_strItemContentIds, 2, true);
    }
    if (!m_strEmptyTextId.empty()) {
        m_strEmptyText = GlobalManager::GetTextById(m_strEmptyTextId);
        Invalidate();
    }
}

void Timeline::ParseItemsField(const DString& strValue, int32_t nField, bool bTranslate)
{
    //按 `|` 分隔条目，逐条填充指定字段；不足的条目自动补空条目
    std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
    size_t nIndex = 0;
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (bTranslate && !s.empty()) {
            //多语言ID模式：按 STRID 从语言文件取实际文本
            s = GlobalManager::GetTextById(s);
        }
        if (nIndex >= m_items.size()) {
            m_items.push_back(Item());
        }
        Item& item = m_items[nIndex];
        switch (nField) {
        case 0: item.time = s; break;
        case 1: item.title = s; break;
        case 2: item.content = s; break;
        case 3:
            if (s == _T("pending")) {
                item.status = TimelineNodeStatus::kPending;
            }
            else if ((s == _T("inprogress")) || (s == _T("in_progress"))) {
                item.status = TimelineNodeStatus::kInProgress;
            }
            else if (s == _T("completed")) {
                item.status = TimelineNodeStatus::kCompleted;
            }
            else if (s == _T("failed")) {
                item.status = TimelineNodeStatus::kFailed;
            }
            else {
                item.status = TimelineNodeStatus::kCustom;
            }
            break;
        case 4: item.customColor = s; break;
        default: break;
        }
        ++nIndex;
    }
    m_nContentHeight = 0;
    Invalidate();
}

void Timeline::AddItem(const Item& item)
{
    m_items.push_back(item);
    m_nContentHeight = 0;
    //追加后让新条目可见：倒序时新条目在视觉顶部（滚到顶），正序时在视觉底部（滚到底，超出部分由绘制时钳位）
    m_nScrollPos = m_bReverse ? 0 : std::numeric_limits<int32_t>::max();
    Invalidate();
}

void Timeline::AddItem(const DString& time, const DString& title, const DString& content,
                       TimelineNodeStatus status)
{
    Item item;
    item.time = time;
    item.title = title;
    item.content = content;
    item.status = status;
    AddItem(item);
}

bool Timeline::InsertItem(size_t index, const Item& item)
{
    if (index > m_items.size()) {
        return false;
    }
    m_items.insert(m_items.begin() + index, item);
    m_nContentHeight = 0;
    Invalidate();
    return true;
}

bool Timeline::RemoveItemAt(size_t index)
{
    if (index >= m_items.size()) {
        return false;
    }
    m_items.erase(m_items.begin() + index);
    m_nContentHeight = 0;
    Invalidate();
    return true;
}

void Timeline::ClearItems()
{
    if (!m_items.empty()) {
        m_items.clear();
        m_nContentHeight = 0;
        m_nScrollPos = 0;
        Invalidate();
    }
}

void Timeline::SetItems(const std::vector<Item>& items)
{
    m_items = items;
    m_nContentHeight = 0;
    m_nScrollPos = 0;
    Invalidate();
}

const Timeline::Item* Timeline::GetItemAt(size_t index) const
{
    if (index >= m_items.size()) {
        return nullptr;
    }
    return &m_items[index];
}

void Timeline::SetNodeColor(TimelineNodeStatus status, const DString& strColor)
{
    int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 5)) {
        m_nodeColors[nIndex] = strColor;
        Invalidate();
    }
}

const DString& Timeline::GetNodeColor(TimelineNodeStatus status) const
{
    int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 5)) {
        return m_nodeColors[nIndex];
    }
    static const DString kEmpty;
    return kEmpty;
}

void Timeline::SetLineColor(const DString& strColor)
{
    m_strLineColor = strColor;
    Invalidate();
}

void Timeline::SetScrollbarTrackColor(const DString& strColor)
{
    if (m_strScrollbarTrackColor != strColor) {
        m_strScrollbarTrackColor = strColor;
        Invalidate();
    }
}

void Timeline::SetScrollbarThumbColor(const DString& strColor)
{
    if (m_strScrollbarThumbColor != strColor) {
        m_strScrollbarThumbColor = strColor;
        Invalidate();
    }
}

void Timeline::SetAutoHideScrollbar(bool bAutoHide)
{
    if (m_bAutoHideScrollbar != bAutoHide) {
        m_bAutoHideScrollbar = bAutoHide;
        Invalidate();
    }
}

void Timeline::SetLineStyle(TimelineLineStyle style)
{
    if (m_lineStyle != style) {
        m_lineStyle = style;
        Invalidate();
    }
}

void Timeline::SetReverse(bool bReverse)
{
    if (m_bReverse != bReverse) {
        m_bReverse = bReverse;
        Invalidate();
    }
}

void Timeline::SetNodeSize(int32_t nSize)
{
    if (nSize < 4) {
        nSize = 4;
    }
    if (nSize > 64) {
        nSize = 64;
    }
    if (m_nNodeSize != nSize) {
        m_nNodeSize = nSize;
        Invalidate();
    }
}

void Timeline::SetItemSpacing(int32_t nSpacing)
{
    if (nSpacing < 0) {
        nSpacing = 0;
    }
    if (m_nItemSpacing != nSpacing) {
        m_nItemSpacing = nSpacing;
        Invalidate();
    }
}

void Timeline::SetEmptyText(const DString& strText)
{
    if (m_strEmptyText != strText) {
        m_strEmptyText = strText;
        m_strEmptyTextId.clear();
        Invalidate();
    }
}

void Timeline::SetEmptyTextId(const DString& strTextId)
{
    if (m_strEmptyTextId != strTextId) {
        m_strEmptyTextId = strTextId;
        m_strEmptyText = GlobalManager::GetTextById(strTextId);
        Invalidate();
    }
}

void Timeline::SetLoading(bool bLoading)
{
    if (m_bLoading != bLoading) {
        m_bLoading = bLoading;
        Invalidate();
    }
}

void Timeline::SetItemDrawCallback(const ItemDrawCallback& callback)
{
    m_itemDrawCallback = callback;
    Invalidate();
}

void Timeline::AttachItemClick(const EventCallback& callback)
{
    AttachEvent(kEventTimelineItemClick, callback, 0);
}

void Timeline::SetScrollPos(int32_t nPos)
{
    if (nPos < 0) {
        nPos = 0;
    }
    if (m_nScrollPos != nPos) {
        m_nScrollPos = nPos;
        Invalidate();
    }
}

void Timeline::ScrollToItem(size_t index)
{
    //计算目标条目顶部 Y，滚动使其可见（简化：按索引比例估算）
    if (index >= m_items.size()) {
        return;
    }
    //由 Paint 精确计算，这里做粗估并触发重绘后由 UpdateScrollRange 钳位
    if (m_items.empty()) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if (rc.Height() <= 0) {
        return;
    }
    //粗略估算：条目索引占总数的比例
    int32_t nRough = static_cast<int32_t>(static_cast<double>(index) * static_cast<double>(m_nContentHeight) /
                                         static_cast<double>(m_items.size()));
    nRough -= rc.Height() / 2;
    SetScrollPos(nRough);
}

void Timeline::UpdateScrollRange(IRender* pRender, const UiRect& rcChart)
{
    //计算内容总高度，并据此钳位滚动偏移
    UiRect rcContent = CalcContentRect(rcChart);
    m_nContentHeight = CalcTotalHeight(pRender, rcContent);
    int32_t nViewHeight = rcChart.Height();
    int32_t nMaxScroll = std::max(0, m_nContentHeight - nViewHeight);
    if (m_nScrollPos > nMaxScroll) {
        m_nScrollPos = nMaxScroll;
    }
    if (m_nScrollPos < 0) {
        m_nScrollPos = 0;
    }
}

UiRect Timeline::CalcContentRect(const UiRect& rcChart) const
{
    UiRect rc = rcChart;
    //节点列宽：节点直径 + 左右边距
    int32_t nNodeColumn = Dpi().GetScaleInt(m_nNodeSize) + Dpi().GetScaleInt(16);
    rc.left += nNodeColumn;
    rc.right -= Dpi().GetScaleInt(8);
    return rc;
}

int32_t Timeline::GetNodeCenterX(const UiRect& rcChart) const
{
    return rcChart.left + Dpi().GetScaleInt(m_nNodeSize) / 2 + Dpi().GetScaleInt(8);
}

void Timeline::PaintStateImages(IRender* pRender)
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

    //加载状态
    if (m_bLoading) {
        UiRect rcText = rc;
        DrawTimelineText(pRender, _T("..."), rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE, _T("text_muted"), _T("system_regular_14"));
        return;
    }

    //空数据
    if (m_items.empty()) {
        if (!m_strEmptyText.empty()) {
            UiRect rcText = rc;
            DrawTimelineText(pRender, m_strEmptyText, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE, _T("text_muted"), _T("system_regular_14"));
        }
        return;
    }

    //更新滚动范围
    UpdateScrollRange(pRender, rc);

    UiRect rcContent = CalcContentRect(rc);
    //int32_t nNodeCenterX = GetNodeCenterX(rc);
    int32_t nNodeRadius = Dpi().GetScaleInt(m_nNodeSize) / 2;
    int32_t nSpacing = Dpi().GetScaleInt(m_nItemSpacing);

    //裁剪到控件区域
    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);

    //逐条绘制（正序或倒序）
    int32_t y = rc.top - m_nScrollPos;
    const size_t nCount = m_items.size();
    for (size_t vi = 0; vi < nCount; ++vi) {
        size_t index = m_bReverse ? (nCount - 1 - vi) : vi;
        const Item& item = m_items[index];
        int32_t nItemHeight = CalcItemHeight(pRender, item, rcContent.Width());
        UiRect rcItem(rc.left, y, rc.right, y + nItemHeight);
        //仅绘制可见条目
        if (rcItem.bottom > rc.top && rcItem.top < rc.bottom) {
            int32_t nNodeCenterY = y + Dpi().GetScaleInt(10) + nNodeRadius;
            bool bIsLast = (vi == nCount - 1);
            PaintItem(pRender, index, item, rcItem, rcContent, nNodeCenterY, bIsLast);
        }
        y += nItemHeight + nSpacing;
    }
    spClip.reset();

    //绘制滚动条（内容超出时，指示视图外还有更多数据）
    DrawScrollbar(pRender, rc);
}

int32_t Timeline::CalcItemHeight(IRender* pRender, const Item& item, int32_t nContentWidth)
{
    if (pRender == nullptr) {
        return 0;
    }
    int32_t nTimeH = MeasureTextHeight(pRender, item.time, nContentWidth, _T("system_regular_12"));
    int32_t nTitleH = MeasureTextHeight(pRender, item.title, nContentWidth, _T("system_bold_14"));
    int32_t nContentH = MeasureTextHeight(pRender, item.content, nContentWidth, _T("system_regular_14"));
    return nTimeH + nTitleH + nContentH + Dpi().GetScaleInt(8);
}

int32_t Timeline::CalcTotalHeight(IRender* pRender, const UiRect& rcContent)
{
    int32_t nTotal = 0;
    int32_t nSpacing = Dpi().GetScaleInt(m_nItemSpacing);
    for (const Item& item : m_items) {
        nTotal += CalcItemHeight(pRender, item, rcContent.Width());
    }
    if (!m_items.empty()) {
        nTotal += static_cast<int32_t>(m_items.size() - 1) * nSpacing;
    }
    return nTotal;
}

void Timeline::PaintItem(IRender* pRender, size_t index, const Item& item, const UiRect& rcItem,
                         const UiRect& rcContent, int32_t nNodeCenterY, bool bIsLast)
{
    //自定义绘制回调
    if (m_itemDrawCallback) {
        UiRect rcNode(GetNodeCenterX(GetRect()) - Dpi().GetScaleInt(m_nNodeSize) / 2,
                      nNodeCenterY - Dpi().GetScaleInt(m_nNodeSize) / 2,
                      GetNodeCenterX(GetRect()) + Dpi().GetScaleInt(m_nNodeSize) / 2,
                      nNodeCenterY + Dpi().GetScaleInt(m_nNodeSize) / 2);
        if (m_itemDrawCallback(pRender, item, rcItem, rcNode, index)) {
            return;
        }
    }

    int32_t nNodeCenterX = GetNodeCenterX(GetRect());
    int32_t nNodeRadius = Dpi().GetScaleInt(m_nNodeSize) / 2;
    UiColor lineColor = GetUiColor(m_strLineColor);
    if (lineColor.IsEmpty()) {
        lineColor = UiColor(0xFFD0D0D0);
    }
    UiColor nodeColor = GetNodeUiColor(item);

    //连接线（节点下方，向下延伸到下一条目的节点）
    if (!bIsLast) {
        int32_t nSpacing = Dpi().GetScaleInt(m_nItemSpacing);
        int32_t nLineTop = nNodeCenterY + nNodeRadius;
        int32_t nLineBottom = rcItem.bottom + nSpacing - Dpi().GetScaleInt(10);
        DrawLine(pRender, nNodeCenterX, nLineTop, nLineBottom, lineColor);
    }

    //节点圆（填充 + 描边）
    pRender->FillCircle(UiPointF(static_cast<float>(nNodeCenterX), static_cast<float>(nNodeCenterY)),
                        static_cast<float>(nNodeRadius), nodeColor, GetAlpha());
    //白色描边，让节点更醒目
    pRender->DrawCircle(UiPointF(static_cast<float>(nNodeCenterX), static_cast<float>(nNodeCenterY)),
                        static_cast<float>(nNodeRadius), UiColor(0xFFFFFFFF), 2.0f);

    //内容区文本
    int32_t nTextY = rcItem.top;
    //time（小字）
    if (!item.time.empty()) {
        UiRect rcTime(rcContent.left, nTextY, rcContent.right, nTextY + Dpi().GetScaleInt(18));
        DrawTimelineText(pRender, item.time, rcTime, TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE, _T("text_muted"), _T("system_regular_12"));
        nTextY += Dpi().GetScaleInt(18);
    }
    //title（粗体）
    if (!item.title.empty()) {
        int32_t nTitleH = MeasureTextHeight(pRender, item.title, rcContent.Width(), _T("system_bold_14"));
        UiRect rcTitle(rcContent.left, nTextY, rcContent.right, nTextY + nTitleH);
        DrawTimelineText(pRender, item.title, rcTitle, TEXT_LEFT | TEXT_TOP, _T("text_default"), _T("system_bold_14"));
        nTextY += nTitleH;
    }
    //content（正文，多行）
    if (!item.content.empty()) {
        int32_t nContentH = MeasureTextHeight(pRender, item.content, rcContent.Width(), _T("system_regular_14"));
        UiRect rcText(rcContent.left, nTextY, rcContent.right, nTextY + nContentH);
        DrawTimelineText(pRender, item.content, rcText, TEXT_LEFT | TEXT_TOP, _T("text_default"), _T("system_regular_14"));
    }
}

UiColor Timeline::GetNodeUiColor(const Item& item) const
{
    if (item.status == TimelineNodeStatus::kCustom) {
        UiColor c = GetUiColor(item.customColor);
        if (!c.IsEmpty()) {
            return c;
        }
        //自定义色为空，回退到进行中颜色
        return GetUiColor(m_nodeColors[static_cast<int>(TimelineNodeStatus::kInProgress)]);
    }
    UiColor c = GetUiColor(m_nodeColors[static_cast<int>(item.status)]);
    if (c.IsEmpty()) {
        c = UiColor(0xFF999999);
    }
    return c;
}

void Timeline::DrawLine(IRender* pRender, int32_t x, int32_t y1, int32_t y2, UiColor color)
{
    if (pRender == nullptr) {
        return;
    }
    if (m_lineStyle == TimelineLineStyle::kSolid) {
        pRender->DrawLine(UiPointF(static_cast<float>(x), static_cast<float>(y1)),
                          UiPointF(static_cast<float>(x), static_cast<float>(y2)),
                          color, 2.0f);
    }
    else {
        //虚线：按 4px 线段 + 4px 间隔分段绘制
        const int32_t nDash = Dpi().GetScaleInt(4);
        const int32_t nGap = Dpi().GetScaleInt(4);
        for (int32_t y = y1; y < y2; y += nDash + nGap) {
            int32_t yEnd = std::min(y + nDash, y2);
            pRender->DrawLine(UiPointF(static_cast<float>(x), static_cast<float>(y)),
                              UiPointF(static_cast<float>(x), static_cast<float>(yEnd)),
                              color, 2.0f);
        }
    }
}

void Timeline::DrawTimelineText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                                const DString& strColor, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    UiColor textColor = GetUiColor(strColor);
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = textColor;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

int32_t Timeline::MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId)
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

int32_t Timeline::HitTestNode(IRender* pRender, const UiPoint& ptMouse)
{
    if ((pRender == nullptr) || m_items.empty()) {
        return -1;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return -1;
    }
    UiRect rcContent = CalcContentRect(rc);
    int32_t nNodeCenterX = GetNodeCenterX(rc);
    int32_t nNodeRadius = Dpi().GetScaleInt(m_nNodeSize) / 2;
    int32_t nHitRadius = nNodeRadius + Dpi().GetScaleInt(4);
    int32_t nSpacing = Dpi().GetScaleInt(m_nItemSpacing);

    int32_t y = rc.top - m_nScrollPos;
    const size_t nCount = m_items.size();
    for (size_t vi = 0; vi < nCount; ++vi) {
        size_t index = m_bReverse ? (nCount - 1 - vi) : vi;
        const Item& item = m_items[index];
        int32_t nItemHeight = CalcItemHeight(pRender, item, rcContent.Width());
        int32_t nNodeCenterY = y + Dpi().GetScaleInt(10) + nNodeRadius;
        int32_t dx = ptMouse.x - nNodeCenterX;
        int32_t dy = ptMouse.y - nNodeCenterY;
        if ((dx * dx + dy * dy) <= nHitRadius * nHitRadius) {
            return static_cast<int32_t>(index);
        }
        y += nItemHeight + nSpacing;
    }
    return -1;
}

bool Timeline::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::ButtonDown(msg);
    }
    //滚动条滑块拖动优先
    if (HitTestScrollThumb(msg.ptMouse)) {
        m_bDraggingScrollbar = true;
        m_dragStartY = msg.ptMouse.y;
        m_dragStartScrollPos = m_nScrollPos;
        if (GetWindow() != nullptr) {
            GetWindow()->SetCapture();
        }
        return true;
    }
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    int32_t nHit = HitTestNode(pRender, msg.ptMouse);
    if (nHit >= 0) {
        SendItemClickEvent(static_cast<size_t>(nHit));
        return true;
    }
    return BaseClass::ButtonDown(msg);
}

bool Timeline::ButtonUp(const EventArgs& msg)
{
    if (m_bDraggingScrollbar) {
        m_bDraggingScrollbar = false;
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }
        return true;
    }
    return BaseClass::ButtonUp(msg);
}

bool Timeline::MouseMove(const EventArgs& msg)
{
    if (m_bDraggingScrollbar) {
        //拖动滑块更新滚动偏移
        UiRect rc = GetRect();
        UiPadding rcPadding = GetControlPadding();
        rc.Deflate(rcPadding);
        int32_t nViewHeight = rc.Height();
        int32_t nMaxScroll = std::max(0, m_nContentHeight - nViewHeight);
        if (nMaxScroll <= 0) {
            return true;
        }
        //可拖动轨道范围 = 轨道高度 - 滑块高度
        int32_t nTrackHeight = rc.Height() - Dpi().GetScaleInt(4);
        int32_t nThumbHeight = m_rcScrollThumb.Height();
        int32_t nDraggableRange = std::max(1, nTrackHeight - nThumbHeight);
        int32_t dy = msg.ptMouse.y - m_dragStartY;
        int32_t nNewPos = m_dragStartScrollPos +
                          static_cast<int32_t>(static_cast<int64_t>(dy) * nMaxScroll / nDraggableRange);
        SetScrollPos(nNewPos);
        return true;
    }
    return BaseClass::MouseMove(msg);
}

bool Timeline::MouseWheel(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::MouseWheel(msg);
    }
    //滚轮滚动
    int32_t wheelDelta = msg.eventData;
    if (wheelDelta != 0) {
        //每个滚轮单位约 120，转换为像素（约 60px/单位）
        int32_t nDelta = -(wheelDelta / 40) * Dpi().GetScaleInt(20);
        int32_t nNewPos = m_nScrollPos + nDelta;
        SetScrollPos(nNewPos);
        return true;
    }
    return BaseClass::MouseWheel(msg);
}

bool Timeline::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    if (!m_bMouseHover) {
        m_bMouseHover = true;
        Invalidate();
    }
    return bRet;
}

bool Timeline::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    if (m_bMouseHover) {
        m_bMouseHover = false;
        Invalidate();
    }
    return bRet;
}

void Timeline::SendItemClickEvent(size_t index)
{
    EventArgs args;
    args.SetSender(this);
    args.wParam = static_cast<WPARAM>(index);
    SendEvent(kEventTimelineItemClick, args);
}

UiRect Timeline::CalcScrollThumbRect(const UiRect& rcChart) const
{
    UiRect rcThumb;
    int32_t nViewHeight = rcChart.Height();
    if ((nViewHeight <= 0) || (m_nContentHeight <= nViewHeight)) {
        return rcThumb; //无需滚动
    }
    const int32_t nTrackTop = rcChart.top + Dpi().GetScaleInt(2);
    const int32_t nTrackBottom = rcChart.bottom - Dpi().GetScaleInt(2);
    const int32_t nTrackHeight = nTrackBottom - nTrackTop;
    if (nTrackHeight <= 0) {
        return rcThumb;
    }
    //滑块高度按视图/内容比例，最小 20px
    int32_t nThumbHeight = static_cast<int32_t>(
        static_cast<int64_t>(nTrackHeight) * nViewHeight / m_nContentHeight);
    nThumbHeight = std::max(nThumbHeight, Dpi().GetScaleInt(20));
    nThumbHeight = std::min(nThumbHeight, nTrackHeight);
    //滑块位置按滚动比例
    int32_t nMaxScroll = m_nContentHeight - nViewHeight;
    int32_t nThumbTop = nTrackTop;
    if (nMaxScroll > 0) {
        nThumbTop += static_cast<int32_t>(
            static_cast<int64_t>(nTrackHeight - nThumbHeight) * m_nScrollPos / nMaxScroll);
    }
    const int32_t nBarWidth = Dpi().GetScaleInt(6);
    rcThumb.left = rcChart.right - nBarWidth - Dpi().GetScaleInt(2);
    rcThumb.right = rcThumb.left + nBarWidth;
    rcThumb.top = nThumbTop;
    rcThumb.bottom = nThumbTop + nThumbHeight;
    return rcThumb;
}

void Timeline::DrawScrollbar(IRender* pRender, const UiRect& rcChart)
{
    if ((pRender == nullptr) || (m_nContentHeight <= rcChart.Height())) {
        return;
    }
    //仅鼠标悬停时显示滚动条（auto_hide_scrollbar 默认开启）
    if (m_bAutoHideScrollbar && !m_bMouseHover) {
        return;
    }
    UiColor trackColor = GetUiColor(m_strScrollbarTrackColor);
    if (trackColor.IsEmpty()) {
        trackColor = UiColor(0xFFE0E0E0);
    }
    UiColor thumbColor = GetUiColor(m_strScrollbarThumbColor);
    if (thumbColor.IsEmpty()) {
        thumbColor = UiColor(0xFFC0C0C0);
    }
    //轨道
    const int32_t nBarWidth = Dpi().GetScaleInt(6);
    UiRectF rcTrack(static_cast<float>(rcChart.right - nBarWidth - Dpi().GetScaleInt(2)),
                    static_cast<float>(rcChart.top + Dpi().GetScaleInt(2)),
                    static_cast<float>(rcChart.right - Dpi().GetScaleInt(2)),
                    static_cast<float>(rcChart.bottom - Dpi().GetScaleInt(2)));
    const float fRadius = static_cast<float>(nBarWidth) / 2.0f;
    pRender->FillRoundRect(rcTrack, fRadius, fRadius, trackColor, 120);
    //滑块
    UiRect rcThumb = CalcScrollThumbRect(rcChart);
    if ((rcThumb.Width() > 0) && (rcThumb.Height() > 0)) {
        m_rcScrollThumb = rcThumb;
        UiRectF rcThumbF(static_cast<float>(rcThumb.left), static_cast<float>(rcThumb.top),
                         static_cast<float>(rcThumb.right), static_cast<float>(rcThumb.bottom));
        pRender->FillRoundRect(rcThumbF, fRadius, fRadius, thumbColor, 200);
    }
}

bool Timeline::HitTestScrollThumb(const UiPoint& ptMouse) const
{
    //仅鼠标悬停时滚动条不可见，此时不参与命中
    if (m_bAutoHideScrollbar && !m_bMouseHover) {
        return false;
    }
    if ((m_rcScrollThumb.Width() <= 0) || (m_rcScrollThumb.Height() <= 0)) {
        return false;
    }
    //扩大命中区域便于拖动
    UiRect rcHit = m_rcScrollThumb;
    rcHit.left -= Dpi().GetScaleInt(4);
    rcHit.right += Dpi().GetScaleInt(4);
    return rcHit.ContainsPt(ptMouse);
}

} // namespace ui
