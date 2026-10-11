#ifndef UI_CONTROL_TIMELINE_H_
#define UI_CONTROL_TIMELINE_H_

#include "duilib/Core/Control.h"
#include <vector>
#include <functional>

namespace ui {

/** Timeline 时间轴节点状态
*/
enum class TimelineNodeStatus
{
    kPending = 0,     //待处理/未开始（灰色）
    kInProgress = 1,  //进行中（强调色）
    kCompleted = 2,   //已完成（绿色）
    kFailed = 3,      //失败（红色）
    kCustom = 4       //自定义颜色（使用 TimelineItem::customColor）
};

/** Timeline 连接线样式
*/
enum class TimelineLineStyle
{
    kSolid = 0,   //实线
    kDashed = 1   //虚线
};

/** Timeline 时间轴控件（继承 Control，纯自绘）
 *
 *  垂直时间轴：左侧节点 + 状态连接线，右侧内容（时间/标题/正文），用于日志、任务流、审批流展示。
 *  数据驱动渲染，支持正序/倒序、节点状态配色、连接线样式、节点点击事件、空数据与加载状态。
 *
 *  XML 属性：
 *  - item_time / item_title / item_content / item_status / item_color：数据绑定，`|` 分隔多条（详见文档）
 *  - item_time_id / item_title_id / item_content_id：多语言数据绑定，`|` 分隔多个 STRID（语言切换时自动刷新）
 *  - node_color_pending / node_color_inprogress / node_color_completed / node_color_failed：各状态节点颜色
 *  - line_color：连接线颜色；line_style：连接线样式（solid/dashed）
 *  - scrollbar_track_color：滚动条轨道颜色，默认语义色 bg_scrollbar_normal
 *  - scrollbar_thumb_color：滚动条滑块颜色，默认语义色 bg_scrollbar_thumb_normal
 *  - auto_hide_scrollbar：是否仅在鼠标悬停时显示滚动条，默认 true
 *  - reverse：是否倒序显示，默认 false（正序）
 *  - node_size：节点直径（像素，DPI 缩放），默认 12
 *  - item_spacing：相邻条目间距（像素，DPI 缩放），默认 24
 *  - empty_text / empty_text_id：空数据提示文本/多语言ID，默认空
 *  - loading：是否显示加载中，默认 false
 */
class DUILIB_API Timeline : public Control
{
    typedef Control BaseClass;

public:
    /** 时间轴条目
    */
    struct Item
    {
        DString time;         //时间/日期文本
        DString title;        //标题
        DString content;      //正文内容（支持换行）
        TimelineNodeStatus status = TimelineNodeStatus::kPending;
        DString customColor;  //自定义节点颜色（status == kCustom 时生效）
    };

    /** 条目自定义绘制回调：返回 true 表示已自行绘制（跳过默认绘制）
    *   @param pRender 渲染接口
    *   @param item 条目数据
    *   @param rcItem 条目整体区域（已含滚动偏移）
    *   @param rcNode 节点圆心所在矩形
    *   @param index 条目索引
    */
    typedef std::function<bool(IRender* pRender, const Item& item, const UiRect& rcItem,
                               const UiRect& rcNode, size_t index)> ItemDrawCallback;

public:
    explicit Timeline(Window* pWindow);
    Timeline(const Timeline& r) = delete;
    Timeline& operator=(const Timeline& r) = delete;
    ~Timeline() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制时间轴内容
    */
    void PaintStateImages(IRender* pRender) override;

    /** 语言切换时刷新多语言数据绑定
    */
    void OnLanguageChanged(bool bRedraw) override;

    // 鼠标交互：节点点击、滚动条拖动、滚轮滚动
    bool ButtonDown(const EventArgs& msg) override;
    bool ButtonUp(const EventArgs& msg) override;
    bool MouseMove(const EventArgs& msg) override;
    bool MouseWheel(const EventArgs& msg) override;
    bool MouseEnter(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;

public:
    /** 追加一条条目
    */
    void AddItem(const Item& item);

    /** 追加一条条目（便捷重载）
    */
    void AddItem(const DString& time, const DString& title, const DString& content,
                 TimelineNodeStatus status = TimelineNodeStatus::kCompleted);

    /** 在指定位置插入条目
    */
    bool InsertItem(size_t index, const Item& item);

    /** 删除指定条目
    */
    bool RemoveItemAt(size_t index);

    /** 清空所有条目
    */
    void ClearItems();

    /** 批量设置条目（替换全部）
    */
    void SetItems(const std::vector<Item>& items);

    /** 获取条目数量
    */
    size_t GetItemCount() const { return m_items.size(); }

    /** 获取指定条目
    */
    const Item* GetItemAt(size_t index) const;

    /** 获取全部条目
    */
    const std::vector<Item>& GetItems() const { return m_items; }

    /** 设置指定状态的节点颜色（语义色名或颜色值）
    */
    void SetNodeColor(TimelineNodeStatus status, const DString& strColor);

    /** 获取指定状态的节点颜色
    */
    const DString& GetNodeColor(TimelineNodeStatus status) const;

    /** 设置连接线颜色
    */
    void SetLineColor(const DString& strColor);

    /** 获取连接线颜色
    */
    const DString& GetLineColor() const { return m_strLineColor; }

    /** 设置滚动条轨道颜色（语义色名或颜色值）
    */
    void SetScrollbarTrackColor(const DString& strColor);

    /** 获取滚动条轨道颜色
    */
    const DString& GetScrollbarTrackColor() const { return m_strScrollbarTrackColor; }

    /** 设置滚动条滑块颜色（语义色名或颜色值）
    */
    void SetScrollbarThumbColor(const DString& strColor);

    /** 获取滚动条滑块颜色
    */
    const DString& GetScrollbarThumbColor() const { return m_strScrollbarThumbColor; }

    /** 设置是否仅在鼠标悬停时显示滚动条
    */
    void SetAutoHideScrollbar(bool bAutoHide);

    /** 获取是否仅在鼠标悬停时显示滚动条
    */
    bool IsAutoHideScrollbar() const { return m_bAutoHideScrollbar; }

    /** 设置连接线样式
    */
    void SetLineStyle(TimelineLineStyle style);

    /** 设置是否倒序显示
    */
    void SetReverse(bool bReverse);

    /** 获取是否倒序显示
    */
    bool IsReverse() const { return m_bReverse; }

    /** 设置节点直径（像素，自动 DPI 缩放）
    */
    void SetNodeSize(int32_t nSize);

    /** 设置相邻条目间距（像素，自动 DPI 缩放）
    */
    void SetItemSpacing(int32_t nSpacing);

    /** 设置空数据提示文本
    */
    void SetEmptyText(const DString& strText);

    /** 设置空数据提示文本（多语言ID，语言切换时自动刷新）
    */
    void SetEmptyTextId(const DString& strTextId);

    /** 设置是否显示加载中状态
    */
    void SetLoading(bool bLoading);

    /** 获取是否加载中
    */
    bool IsLoading() const { return m_bLoading; }

    /** 设置条目自定义绘制回调（覆盖默认绘制，置空恢复默认）
    */
    void SetItemDrawCallback(const ItemDrawCallback& callback);

    /** 条目点击事件回调（EventArgs::wParam 为条目序号）
    */
    void AttachItemClick(const EventCallback& callback);

    /** 获取当前滚动偏移（像素）
    */
    int32_t GetScrollPos() const { return m_nScrollPos; }

    /** 设置滚动偏移（像素，自动钳位）
    */
    void SetScrollPos(int32_t nPos);

    /** 滚动到指定条目
    */
    void ScrollToItem(size_t index);

private:
    /** 计算内容区域（扣除节点列宽）
    */
    UiRect CalcContentRect(const UiRect& rcChart) const;

    /** 计算单条条目的高度（像素）
    *   @param pRender 渲染接口
    *   @param item 条目
    *   @param nContentWidth 内容区宽度（用于正文换行测量）
    */
    int32_t CalcItemHeight(IRender* pRender, const Item& item, int32_t nContentWidth);

    /** 计算全部条目总高度（含间距），并缓存条目布局
    */
    int32_t CalcTotalHeight(IRender* pRender, const UiRect& rcContent);

    /** 命中检测：判断鼠标位置命中的节点索引
    *   @return 命中的条目索引，未命中返回 -1
    */
    int32_t HitTestNode(IRender* pRender, const UiPoint& ptMouse);

    /** 更新滚动偏移并钳位
    */
    void UpdateScrollRange(IRender* pRender, const UiRect& rcChart);

    /** 绘制单条条目
    */
    void PaintItem(IRender* pRender, size_t index, const Item& item, const UiRect& rcItem,
                   const UiRect& rcContent, int32_t nNodeCenterY, bool bIsLast);

    /** 获取节点中心 X 坐标（控件坐标）
    */
    int32_t GetNodeCenterX(const UiRect& rcChart) const;

    /** 发送条目点击事件
    */
    void SendItemClickEvent(size_t index);

    /** 计算滚动条滑块矩形
    */
    UiRect CalcScrollThumbRect(const UiRect& rcChart) const;

    /** 绘制滚动条（内容超出时）
    */
    void DrawScrollbar(IRender* pRender, const UiRect& rcChart);

    /** 判断鼠标是否命中滚动条滑块
    */
    bool HitTestScrollThumb(const UiPoint& ptMouse) const;

    /** 解析 XML 数据绑定字段（`|` 分隔）
    *   @param strValue 字段值字符串
    *   @param nField 字段编号：0=time, 1=title, 2=content, 3=status, 4=color
    *   @param bTranslate 是否为多语言ID模式（每个分段按 STRID 从语言文件取实际文本）
    */
    void ParseItemsField(const DString& strValue, int32_t nField, bool bTranslate = false);

    /** 获取条目节点颜色（解析语义色）
    */
    UiColor GetNodeUiColor(const Item& item) const;

    /** 绘制连接线（实线/虚线）
    */
    void DrawLine(IRender* pRender, int32_t x, int32_t y1, int32_t y2, UiColor color);

    /** 绘制文本（命名为 DrawTimelineText，避免与 Windows SDK 的 DrawText 宏冲突）
    */
    void DrawTimelineText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                          const DString& strColor, const DString& strFontId);

    /** 测量文本高度（支持多行换行）
    */
    int32_t MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId);

private:
    /** 条目数据
    */
    std::vector<Item> m_items;

    /** 各状态节点颜色（语义色名，索引对应 TimelineNodeStatus）
    */
    DString m_nodeColors[5];

    /** 连接线颜色
    */
    DString m_strLineColor;

    /** 滚动条轨道颜色（语义色名或颜色值）
    */
    DString m_strScrollbarTrackColor;

    /** 滚动条滑块颜色（语义色名或颜色值）
    */
    DString m_strScrollbarThumbColor;

    /** 是否仅在鼠标悬停时显示滚动条
    */
    bool m_bAutoHideScrollbar;

    /** 当前鼠标是否悬停在控件上
    */
    bool m_bMouseHover;

    /** 连接线样式
    */
    TimelineLineStyle m_lineStyle;

    /** 是否倒序显示
    */
    bool m_bReverse;

    /** 节点直径（像素，DPI 缩放前原始值）
    */
    int32_t m_nNodeSize;

    /** 相邻条目间距（像素，DPI 缩放前原始值）
    */
    int32_t m_nItemSpacing;

    /** 空数据提示文本
    */
    DString m_strEmptyText;

    /** 空数据提示文本的多语言ID
    */
    DString m_strEmptyTextId;

    /** 数据绑定的多语言ID原始串（item_time_id/item_title_id/item_content_id，语言切换时重新解析）
    */
    DString m_strItemTimeIds;
    DString m_strItemTitleIds;
    DString m_strItemContentIds;

    /** 是否加载中
    */
    bool m_bLoading;

    /** 当前滚动偏移（像素）
    */
    int32_t m_nScrollPos;

    /** 内容总高度（像素，滚动范围）
    */
    int32_t m_nContentHeight;

    /** 条目自定义绘制回调
    */
    ItemDrawCallback m_itemDrawCallback;

    /** 是否正在拖动滚动条滑块
    */
    bool m_bDraggingScrollbar;

    /** 拖动滚动条起始鼠标 Y（控件坐标）
    */
    int32_t m_dragStartY;

    /** 拖动滚动条起始滚动偏移
    */
    int32_t m_dragStartScrollPos;

    /** 滚动条滑块矩形（缓存，供命中检测与绘制）
    */
    UiRect m_rcScrollThumb;
};

} // namespace ui

#endif // UI_CONTROL_TIMELINE_H_
