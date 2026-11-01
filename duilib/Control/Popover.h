#ifndef UI_CONTROL_POPOVER_H_
#define UI_CONTROL_POPOVER_H_

#include "duilib/Control/Label.h"

namespace ui
{
class Flyout;

/** Popover 的触发方式
 */
enum class PopoverTrigger : uint8_t
{
    kClick  = 0,    //鼠标左键点击锚点：打开；再次点击或点击外部：关闭（默认）
    kHover  = 1,    //鼠标移入锚点：打开；移出锚点与卡片：关闭
    kFocus  = 2,    //锚点获得焦点：打开；失去焦点：关闭
    kManual = 3     //只能通过 Show() / Hide() / Toggle() 控制
};

/** Popover 的弹出方位（语义与 Ant Design Popover 的 12 个方位一致）
 */
enum class PopoverPlacement : uint8_t
{
    kTop         = 0,   //上方，水平居中
    kTopLeft     = 1,   //上方，左边缘对齐
    kTopRight    = 2,   //上方，右边缘对齐
    kBottom      = 3,   //下方，水平居中
    kBottomLeft  = 4,   //下方，左边缘对齐
    kBottomRight = 5,   //下方，右边缘对齐
    kLeft        = 6,   //左侧，垂直居中
    kLeftTop     = 7,   //左侧，上边缘对齐
    kLeftBottom  = 8,   //左侧，下边缘对齐
    kRight       = 9,   //右侧，垂直居中
    kRightTop    = 10,  //右侧，上边缘对齐
    kRightBottom = 11   //右侧，下边缘对齐
};

/** Popover 卡片的关闭原因（与 Flyout::CloseReason 的取值保持一致）
 */
enum class PopoverCloseReason : uint32_t
{
    kManual        = 0,    //主动关闭（Hide()、再次点击锚点、hover 移出）
    kClickOutside  = 1,    //点击卡片与锚点以外的区域
    kEscape        = 2,    //按下 Esc 键
    kAnchorLost    = 3     //锚点控件或其窗口失效（销毁/隐藏/最小化/DPI 变化）
};

/** Popover 气泡卡片（继承 Label，锚点自身显示文本，卡片在独立浮层窗口中显示）
 *
 *  解决"点击/悬停一个元素后显示一段带标题的说明卡片"的语义：卡片由标题 + 正文组成，
 *  显示在锚点周围的 12 个方位之一，空间不足时自动翻转到对侧并夹持在显示器工作区内。
 *
 *  与 Flyout 的差异：Flyout 是"由调用方自己创建、自己提供整窗 XML"的通用浮层；
 *  Popover 是"控件自身即锚点、卡片内容由属性/API 描述、生命周期由控件自动管理"的封装控件。
 *  与 Tooltip 的差异：Tooltip 只有纯文本且跟随鼠标，Popover 是带标题的结构化卡片，可交互。
 *
 *  交互：
 *  - trigger="click"（默认）：点击锚点打开，再次点击锚点或点击外部关闭；
 *  - trigger="hover"：移入锚点打开，移出锚点与卡片后延迟关闭；
 *  - trigger="focus"：锚点获得焦点打开，失去焦点关闭；
 *  - trigger="manual"：仅 Show()/Hide()/Toggle() 生效。
 *  卡片显示期间不抢焦点（WS_EX_NOACTIVATE），锚点所在窗口保持可操作。
 *  锚点位于滚动容器内时，卡片随锚点位置移动；锚点被滚出可视区域时卡片窗口暂时隐藏
 *  （打开状态保留），锚点重新可见后自动恢复显示。
 *
 *  XML 属性（除 Label 通用属性外）：
 *    title          卡片标题（字面文本，可省略）
 *    title_id       卡片标题的多语言 ID（与 title 互斥，优先生效）
 *    content        卡片正文（字面文本，多行用 \\n 分隔，可省略）
 *    content_id     卡片正文的多语言 ID（与 content 互斥，优先生效）
 *    content_xml    富内容 XML 片段（形如 "<Label .../>"，作为卡片子控件插入；与 content 互斥）
 *    trigger        触发方式：click（默认）/ hover / focus / manual
 *    placement      弹出方位：top（默认）/ top_left / top_right / bottom / bottom_left /
 *                   bottom_right / left / left_top / left_bottom / right / right_top / right_bottom
 *    gap            卡片与锚点的间距（像素，DPI 缩放），默认 6；
 *                   显示尖角箭头时，该间距是"尖角尖端"到锚点的距离
 *    card_width     卡片宽度（像素，DPI 缩放），默认 240；正文自动换行
 *    arrow          是否显示指向锚点的尖角箭头，默认 true
 *                   （空间不足导致卡片翻转到锚点另一侧时，箭头朝向随之改变）
 *    auto_dismiss   是否自动关闭，默认 true：
 *                   点击卡片与锚点以外的区域关闭（trigger="manual" 时该行为默认关闭，
 *                   避免鼠标按在外部受控按钮上时卡片先被关闭、导致按钮逻辑失效；
 *                   显式设置 auto_dismiss="true" 可恢复）；
 *                   按 Esc 键关闭始终受该属性控制
 *    hover_delay    trigger="hover" 时移入后打开前的延迟（毫秒），默认 100
 *    close_delay    trigger="hover" 时移出后关闭前的延迟（毫秒），默认 200
 *
 *  事件：
 *  - kEventPopoverOpenChanged：打开状态变化时触发，wParam 为 1（打开）/ 0（关闭），
 *    lParam 为关闭原因（PopoverCloseReason，打开时为 0）
 *
 *  皮肤类（global.xml 中定义）：popover_trigger / popover_card / popover_arrow_up /
 *  popover_arrow_down / popover_arrow_left / popover_arrow_right / popover_title / popover_content
 *
 *  说明：尖角箭头默认开启（arrow="true"），颜色取卡片背景与卡片边线的语义色，随主题切换；
 *        语言切换时若卡片正在显示，会自动关闭。
 */
class DUILIB_API Popover : public Label
{
    typedef Label BaseClass;

public:
    explicit Popover(Window* pWindow);
    Popover(const Popover& r) = delete;
    Popover& operator=(const Popover& r) = delete;
    ~Popover() override;

    /** 获取控件类型（DUI_CTR_POPOVER）
     */
    DString GetType() const override;

    /** 设置XML属性
     */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 语言切换：关闭正在显示的卡片（卡片内容在打开时生成）
     */
    void OnLanguageChanged(bool bRedraw) override;

public:
    /** 设置卡片标题（字面文本）
     */
    void SetTitle(const DString& strTitle);
    /** 获取卡片标题（多语言 ID 优先，否则为字面文本） */
    DString GetTitle() const;

    /** 设置卡片标题（多语言 ID） */
    void SetTitleId(const DString& strTitleId);
    /** 获取卡片标题的多语言 ID */
    const DString& GetTitleId() const { return m_strTitleId; }

    /** 设置卡片正文（字面文本，多行用 \\n 分隔） */
    void SetContent(const DString& strContent);
    /** 获取卡片正文（多语言 ID 优先，否则为字面文本） */
    DString GetContent() const;

    /** 设置卡片正文（多语言 ID） */
    void SetContentId(const DString& strContentId);
    /** 获取卡片正文的多语言 ID */
    const DString& GetContentId() const { return m_strContentId; }

    /** 设置富内容 XML 片段（形如 "<Label .../>"；设空串表示不使用富内容）
     * @note 片段作为卡片根容器（class="popover_card"）的子控件插入，颜色/字体请使用语义色名
     */
    void SetContentXml(const DString& strContentXml);
    /** 获取富内容 XML 片段 */
    const DString& GetContentXml() const { return m_strContentXml; }

    /** 设置触发方式 */
    void SetTrigger(PopoverTrigger trigger);
    /** 获取触发方式 */
    PopoverTrigger GetTrigger() const { return m_trigger; }

    /** 设置弹出方位 */
    void SetPlacement(PopoverPlacement placement);
    /** 获取弹出方位 */
    PopoverPlacement GetPlacement() const { return m_placement; }

    /** 设置卡片与锚点的间距（像素，DPI 缩放） */
    void SetGap(int32_t nGap);
    /** 获取卡片与锚点的间距（未缩放原始值） */
    int32_t GetGap() const { return m_nGap; }

    /** 设置卡片宽度（像素，DPI 缩放；小于 120 时按 120 处理） */
    void SetCardWidth(int32_t nWidth);
    /** 获取卡片宽度（未缩放原始值） */
    int32_t GetCardWidth() const { return m_nCardWidth; }

    /** 设置是否显示指向锚点的尖角箭头，默认 true
     * @note 箭头朝向随弹出方位自动调整；卡片因空间不足翻转到锚点另一侧时，箭头也会随之转向
     */
    void SetArrow(bool bArrow);
    /** 是否显示尖角箭头 */
    bool IsArrow() const { return m_bArrow; }

    /** 设置点击外部/按 Esc 是否自动关闭，默认 true
     * @note trigger="manual" 时，"点击外部关闭"默认不生效（Esc 仍按此属性控制），
     *       调用本函数（或在 XML 中显式设置 auto_dismiss）后按显式值生效
     */
    void SetAutoDismiss(bool bAutoDismiss);
    /** 是否自动关闭 */
    bool IsAutoDismiss() const { return m_bAutoDismiss; }

    /** 设置 hover 触发时移入后打开前的延迟（毫秒），默认 100 */
    void SetHoverDelay(int32_t nDelayMs);
    /** 获取 hover 打开延迟（毫秒） */
    int32_t GetHoverDelay() const { return m_nHoverDelayMs; }

    /** 设置 hover 触发时移出后关闭前的延迟（毫秒），默认 200 */
    void SetCloseDelay(int32_t nDelayMs);
    /** 获取 hover 关闭延迟（毫秒） */
    int32_t GetCloseDelay() const { return m_nCloseDelayMs; }

    /** 显示卡片（禁用、不可见、窗口未创建时不生效；已显示时为空操作）
     */
    void Show();

    /** 关闭卡片（未显示时为空操作）
     */
    void Hide();

    /** 切换卡片的显示/隐藏状态
     */
    void Toggle();

    /** 卡片是否处于显示状态
     */
    bool IsOpen() const { return m_bOpen; }

    /** 获取当前显示中的卡片窗口（卡片未显示时返回 nullptr）
     * @note 卡片在每次打开时重建，卡片内的控件只在本次显示期间有效；
     *       如需绑定卡片内控件（例如卡片内的按钮/链接），请在 kEventPopoverOpenChanged 回调中
     *       判断 wParam == 1 后取到窗口并查找控件
     */
    Window* GetCardWindow() const;

    /** 在当前显示中的卡片内按名称查找控件（卡片未显示或未找到时返回 nullptr）
     * @param [in] strName 控件名称（对应卡片XML中的 name 属性）
     */
    Control* FindCardControl(const DString& strName) const;

    /** 监听卡片打开状态变化事件
     * @param [in] callback 状态变化后的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明：
     *   wParam: 1 = 已打开，0 = 已关闭
     *   lParam: 关闭原因（PopoverCloseReason；打开时为 0）
     */
    void AttachOpenChanged(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventPopoverOpenChanged, callback, callbackID);
    }

public:
    //Label 接口
    bool MouseEnter(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;
    bool ButtonDown(const EventArgs& msg) override;
    bool ButtonUp(const EventArgs& msg) override;

protected:
    //Control 接口
    bool OnSetFocus(const EventArgs& msg) override;
    bool OnKillFocus(const EventArgs& msg) override;
    void OnSetVisible(bool bChanged) override;

private:
    /** 获取当前正在显示、且锚点为本控件的浮层（没有时返回 nullptr）
     */
    Flyout* GetOpenFlyout() const;

    /** 构造卡片浮层的 XML 文本
     * @param [in] placement 卡片实际使用的弹出方位（翻转后可能与 m_placement 不同，用于决定箭头朝向与位置）
     */
    DString BuildCardXml(PopoverPlacement placement) const;

    /** 卡片创建完成后初始化：绑定卡片自身的鼠标进入/离开事件（hover 触发的关闭判定）
     */
    void InitializeCard(Flyout* pFlyout);

    /** 按卡片的实际布局矩形定位尖角箭头（在浮层布局完成后调用）
     *   箭头沿卡片边移动到锚点中心对应的位置，并压入卡片边线一侧，覆盖卡片边线
     */
    void PositionCardArrow(Flyout* pFlyout);

    /** 派发打开状态变化事件 */
    void EmitOpenChanged(bool bOpen, PopoverCloseReason reason);

    /** 浮层关闭回调（更新状态并派发事件）
     */
    void OnFlyoutClosed(int32_t nCloseReason);

    /** 判断鼠标当前是否位于本控件或卡片窗口内（物理像素坐标）
     */
    bool IsMouseOverAnchorOrCard() const;

    /** 获取本控件在屏幕坐标系下的矩形（物理像素）
     */
    UiRect GetScreenRectPhysical() const;

    /** hover 触发：延时打开 */
    void ScheduleOpen();
    /** hover 触发：延时检查是否需要关闭 */
    void ScheduleCloseCheck();

    /** 取消延时打开/延时关闭任务 */
    void CancelOpenTask();
    void CancelCloseTask();
    void CancelAllTasks();

private:
    /** 卡片标题（字面文本 / 多语言 ID） */
    DString m_strTitle;
    DString m_strTitleId;

    /** 卡片正文（字面文本 / 多语言 ID / 富内容 XML） */
    DString m_strContent;
    DString m_strContentId;
    DString m_strContentXml;

    /** 触发方式与弹出方位 */
    PopoverTrigger m_trigger = PopoverTrigger::kClick;
    PopoverPlacement m_placement = PopoverPlacement::kTop;

    /** 与锚点的间距（未经 DPI 缩放） */
    int32_t m_nGap = 6;

    /** 卡片宽度（未经 DPI 缩放） */
    int32_t m_nCardWidth = 240;

    /** 是否显示指向锚点的尖角箭头 */
    bool m_bArrow = true;

    /** 是否点击外部/按 Esc 自动关闭 */
    bool m_bAutoDismiss = true;

    /** auto_dismiss 是否被显式设置（属性或 API），用于 manual 触发的默认值判断 */
    bool m_bAutoDismissDefined = false;

    /** hover 打开/关闭延迟（毫秒） */
    int32_t m_nHoverDelayMs = 100;
    int32_t m_nCloseDelayMs = 200;

    /** 卡片是否处于显示状态 */
    bool m_bOpen = false;

    /** 鼠标是否位于锚点上 */
    bool m_bHoverInside = false;

    /** 是否正在析构（析构期间忽略浮层关闭回调） */
    bool m_bDestroying = false;

    /** 是否正在因"方位翻转"重建卡片（重建期间忽略旧浮层的关闭回调） */
    bool m_bRebuilding = false;

    /** 延时任务 ID */
    size_t m_nOpenTaskId = 0;
    size_t m_nCloseTaskId = 0;
};

} // namespace ui

#endif // UI_CONTROL_POPOVER_H_
