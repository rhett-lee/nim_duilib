#ifndef UI_CONTROL_CHATMESSAGELIST_H_
#define UI_CONTROL_CHATMESSAGELIST_H_

#include "duilib/Box/ScrollBox.h"
#include <vector>
#include <functional>

namespace ui {

/** 聊天消息方向
*/
enum class ChatMessageSide
{
    kOther = 0,   //对方消息（左侧）
    kSelf = 1,    //自己消息（右侧）
    kSystem = 2   //系统提示（居中）
};

/** 消息类型
*/
enum class ChatMessageType
{
    kText = 0,    //普通文本消息
    kSystem = 1,  //系统提示（居中小字，不带头像）
};

/** 消息状态
 *
 *  仅自己发送的消息（ChatMessageSide::kSelf）会绘制状态文字，
 *  显示在气泡下方、与气泡右边缘对齐。
 */
enum class ChatMessageStatus
{
    kNone = 0,       //无状态（默认，不绘制状态文字）
    kSending = 1,    //发送中
    kFailed = 2,     //发送失败（点击状态区域可重试）
    kDelivered = 3,  //已送达
    kRead = 4        //已读
};

/** 消息命中区域（点击/右键回调中用于区分点到的是哪一部分）
*/
enum class ChatMessageHitArea
{
    kNone = 0,     //未命中任何消息
    kBubble = 1,   //气泡区域（含气泡内文本）
    kAvatar = 2,   //头像区域
    kStatus = 3    //状态文字区域（发送中/发送失败/已送达/已读）
};

/** 消息事件参数：点击、右键、双击回调的回调参数
*/
struct ChatMessageEvent
{
    size_t nIndex = 0;                                       //消息在列表中的索引
    int64_t id = 0;                                          //消息 id
    ChatMessageStatus status = ChatMessageStatus::kNone;     //消息状态
    ChatMessageHitArea hitArea = ChatMessageHitArea::kNone;  //命中的区域
    UiPoint ptMouse;                                         //鼠标位置（控件客户区坐标）
};

/** 消息点击回调原型：仅用于通知，返回值无意义
*/
typedef std::function<void (const ChatMessageEvent& messageEvent)> ChatMessageClickEvent;

/** 消息双击/右键回调原型
 *  @return 返回 true 表示应用层已处理，控件跳过默认行为：
 *          双击时不再选中该次点击位置所在的词；右键时不再弹出默认上下文菜单。
 */
typedef std::function<bool (const ChatMessageEvent& messageEvent)> ChatMessageActionEvent;

/** 聊天消息列表控件（继承 ScrollBox，自绘消息 + 框架滚动条）
 *
 *  模拟真实 IM 聊天界面：消息气泡按左右方向分栏、气泡宽度随文本自适应、
 *  头像为彩色圆形（可用首字母）、可选时间分隔条，支持滚动与自动滚动到底部。
 *  支持鼠标拖选文本、双击选词、Ctrl+C 复制、右键菜单。
 *  自己发送的消息可显示发送状态（发送中/发送失败/已送达/已读），
 *  并提供点击/右键/双击事件回调。
 *
 *  XML 属性：
 *  - avatar_size：头像直径（像素，DPI 缩放），默认 36
 *  - bubble_max_width：气泡最大宽度（像素，DPI 缩放），默认 480
 *  - bubble_padding_h / bubble_padding_v：气泡内边距（像素，DPI 缩放），默认 12 / 8
 *  - bubble_radius：气泡圆角半径（像素，DPI 缩放），默认 10
 *  - bubble_spacing：相邻消息间距（像素，DPI 缩放），默认 14
 *  - show_time：是否显示时间分隔条，默认 true
 *  - avatar_font：头像文字字体，默认 system_bold_14
 *  - text_font：消息正文字体，默认 system_regular_14
 *  - time_font：时间文字字体，默认 system_regular_12
 *  - bubble_color_self：自己消息气泡底色（语义色名），默认 color_accent
 *  - bubble_color_other：对方消息气泡底色（语义色名），默认 bg_window_card
 *  - text_color_self：自己消息文字色（语义色名），默认 color_white
 *  - text_color_other：对方消息文字色（语义色名），默认 text_default
 *  - avatar_color：头像文字色（语义色名），默认 color_white
 *  - system_text_color：系统提示文字色（语义色名），默认 text_muted
 *  - selection_bk_color：文本选中背景色（语义色名），默认 bg_control_selected
 *  - selection_text_color：文本选中文字色（语义色名），默认 text_default
 *  - show_status：是否显示消息状态文字，默认 true
 *  - status_font：状态文字字体，默认 system_regular_12
 *  - status_text_color：状态文字颜色（语义色名），默认 text_muted
 *  - status_failed_color：发送失败时状态文字颜色（语义色名），默认 color_error
 *  - scrollbar_track_color：滚动条轨道颜色，默认 bg_scrollbar_normal
 *  - scrollbar_thumb_color：滚动条滑块颜色，默认 bg_scrollbar_thumb_normal
 *  - auto_hide_scrollbar：是否仅在鼠标悬停时显示滚动条，默认 true
 *  - enable_selection：是否允许鼠标拖选文本，默认 true
 *  - show_new_message_tip：是否在未滚动到底部时显示"N 条新消息"提示，默认 true
 */
class DUILIB_API ChatMessageList : public ScrollBox
{
    typedef ScrollBox BaseClass;

public:
    /** 单条消息
    */
    struct Message
    {
        ChatMessageSide side = ChatMessageSide::kOther;
        ChatMessageType type = ChatMessageType::kText;
        ChatMessageStatus status = ChatMessageStatus::kNone;
        int64_t id = 0;            //消息唯一标识（0 表示未指定）
        DString text;              //消息文本
        DString avatarText;        //头像文字（通常为姓名首字）
        DString time;              //时间文本（用于时间分隔条）
    };

public:
    explicit ChatMessageList(Window* pWindow);
    ChatMessageList(const ChatMessageList& r) = delete;
    ChatMessageList& operator=(const ChatMessageList& r) = delete;
    ~ChatMessageList() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 设置位置：重写以在自绘内容模式下主动驱动滚动条范围
    */
    void SetPos(UiRect rc) override;

    /** 设置关联窗口：重写以启用滚动条
    */
    void SetWindow(Window* pWindow) override;

    /** 滚动偏移变化：自绘内容需要跟随框架滚动条同步偏移量
    */
    void OnScrollOffsetChanged(const UiSize& oldScrollOffset, const UiSize& newScrollOffset) override;

    /** 绘制消息列表
    */
    void PaintStateImages(IRender* pRender) override;

    /** 鼠标交互：滚轮滚动、文本选择
    */
    bool ButtonDown(const EventArgs& msg) override;
    bool ButtonUp(const EventArgs& msg) override;
    bool MouseMove(const EventArgs& msg) override;
    bool MouseWheel(const EventArgs& msg) override;
    bool MouseEnter(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;
    bool ButtonDoubleClick(const EventArgs& msg) override;

    /** 键盘：处理 Ctrl+C / Ctrl+A
    */
    void HandleEvent(const EventArgs& msg) override;

public:
    /** 追加一条消息
     *  @return 消息 id（可用于后续 SetMessageStatus / UpdateMessage / RemoveMessage）
    */
    int64_t AddMessage(const Message& message);

    /** 追加一条消息（便捷重载）
     *  @return 消息 id
    */
    int64_t AddMessage(ChatMessageSide side, const DString& strText,
                       const DString& strAvatarText = _T(""), const DString& strTime = _T(""));

    /** 在指定位置插入一条消息
     *  @return 消息 id
    */
    int64_t InsertMessage(size_t index, const Message& message);

    /** 按消息 id 更新消息内容（找不到 id 时不做任何事）
    */
    bool UpdateMessage(int64_t id, const Message& message);

    /** 按消息 id 删除消息（找不到 id 时返回 false）
    */
    bool RemoveMessage(int64_t id);

    /** 按消息 id 查找消息
    */
    const Message* GetMessageById(int64_t id) const;

    /** 清空所有消息
    */
    void ClearMessages();

    /** 获取消息数量
    */
    size_t GetMessageCount() const { return m_messages.size(); }

    /** 获取指定消息
    */
    const Message* GetMessageAt(size_t index) const;

public:
    /** 消息状态相关接口
    */
    /** 按消息 id 设置状态（找不到 id 时返回 false）
    */
    bool SetMessageStatus(int64_t id, ChatMessageStatus status);

    /** 按消息索引设置状态（索引越界时返回 false）
    */
    bool SetMessageStatusAt(size_t index, ChatMessageStatus status);

    /** 获取指定消息的状态（索引越界时返回 kNone）
    */
    ChatMessageStatus GetMessageStatus(size_t index) const;

    /** 自定义某个状态显示的文字（传空字符串表示恢复默认文案）
     *
     *  默认文案通过多语言资源获取（STRID_PUBLIC_CHAT_STATUS_*），
     *  在语言切换后会自动更新。
     */
    void SetStatusText(ChatMessageStatus status, const DString& strText);

    /** 获取某个状态当前显示的文字（已按多语言解析）
    */
    DString GetStatusText(ChatMessageStatus status) const;

    /** 设置是否显示消息状态文字
    */
    void SetShowStatus(bool bShowStatus);
    bool IsShowStatus() const { return m_bShowStatus; }

public:
    /** 消息事件回调
    */
    /** 点击消息（左键按下后抬起，且期间没有拖动选择文本）
     *
     *  点到发送失败的消息状态区域时，hitArea 为 kStatus 且 status 为 kFailed，
     *  应用层可据此执行重发。
     */
    void AttachMessageClick(const ChatMessageClickEvent& callback);

    /** 双击消息：默认行为是选中鼠标位置所在的词，回调返回 true 则跳过
    */
    void AttachMessageDoubleClick(const ChatMessageActionEvent& callback);

    /** 右键消息：默认行为是弹出 AttachPopupMenu 绑定的菜单，回调返回 true 则跳过
    */
    void AttachMessageRightClick(const ChatMessageActionEvent& callback);

    /** 滚动到底部（显示最新消息）
    */
    void ScrollToBottom();

    /** 获取当前滚动偏移（像素，垂直方向）
    */
    int32_t GetChatScrollPos() const { return static_cast<int32_t>(BaseClass::GetScrollPos().cy); }

    /** 设置当前滚动偏移（像素，自动钳位）
    */
    void SetChatScrollPos(int32_t nPos);

    /** 是否自动滚动到底部（追加消息时生效），默认 true
     *
     *  仅在当前视图已经位于底部时才会自动滚动；用户向上翻看历史时，
     *  新消息不会把视图拽走，而是累加到"新消息"计数（见 GetUnreadBelowCount）。
    */
    void SetAutoScrollToBottom(bool bAutoScroll);
    bool IsAutoScrollToBottom() const { return m_bAutoScrollToBottom; }

    /** 当前视图是否已经位于底部（允许少量像素误差）
    */
    bool IsNearBottom() const;

    /** 获取视图下方尚未查看的新消息条数（滚动到底部后清零）
    */
    int32_t GetUnreadBelowCount() const { return m_nUnreadBelow; }

    /** 清零未读新消息计数（不改变滚动位置）
    */
    void ClearUnreadBelowCount();

    /** 是否显示"回到最新消息"提示，默认 true
    */
    void SetShowNewMessageTip(bool bShow);
    bool IsShowNewMessageTip() const { return m_bShowNewMessageTip; }

    /** 设置头像直径（像素，自动 DPI 缩放）
    */
    void SetAvatarSize(int32_t nSize);
    int32_t GetAvatarSize() const { return m_nAvatarSize; }

    /** 设置气泡最大宽度（像素，自动 DPI 缩放）
    */
    void SetBubbleMaxWidth(int32_t nWidth);
    int32_t GetBubbleMaxWidth() const { return m_nBubbleMaxWidth; }

    /** 设置气泡圆角半径（像素，自动 DPI 缩放）
    */
    void SetBubbleRadius(int32_t nRadius);
    int32_t GetBubbleRadius() const { return m_nBubbleRadius; }

    /** 设置是否显示时间分隔条
    */
    void SetShowTime(bool bShowTime);
    bool IsShowTime() const { return m_bShowTime; }

    /** 设置气泡底色（语义色名）
    */
    void SetBubbleColor(ChatMessageSide side, const DString& strColor);

    /** 设置消息文字色（语义色名）
    */
    void SetTextColor(ChatMessageSide side, const DString& strColor);

    /** 设置头像文字色（语义色名）
    */
    void SetAvatarTextColor(const DString& strColor);

    /** 设置系统提示文字色（语义色名）
    */
    void SetSystemTextColor(const DString& strColor);

    /** 设置选中背景色（语义色名）
    */
    void SetSelectionBkColor(const DString& strColor);

    /** 设置选中文字色（语义色名）
    */
    void SetSelectionTextColor(const DString& strColor);

    /** 设置滚动条轨道颜色（语义色名或颜色值）
    */
    void SetScrollbarTrackColor(const DString& strColor);

    /** 设置滚动条滑块颜色（语义色名或颜色值）
    */
    void SetScrollbarThumbColor(const DString& strColor);

    /** 设置是否仅在鼠标悬停时显示滚动条
    */
    void SetAutoHideScrollbar(bool bAutoHide);
    bool IsAutoHideScrollbar() const { return m_bAutoHideScrollbar; }

    /** 设置是否允许鼠标拖选文本
    */
    void SetEnableSelection(bool bEnable);
    bool IsEnableSelection() const { return m_bEnableSelection; }

public:
    /** 文本选择相关接口
    */
    /** 是否当前存在选中文本
    */
    bool HasSelection() const;

    /** 获取选中的文本
    */
    DString GetSelectedText() const;

    /** 清除选区
    */
    void ClearSelection();

    /** 全选所有可见消息的文本
    */
    void SelectAll();

    /** 选中某条消息的全部文本（索引越界或文本为空时返回 false）
    */
    bool SelectMessageText(size_t nIndex);

    /** 复制选中文本到剪贴板（无选区时返回 false）
    */
    bool CopySelectedText();

    /** 复制全部消息文本到剪贴板
    */
    bool CopyAllText();

    /** 绑定右键上下文菜单（菜单项为内置的"复制/全选"）
    */
    bool AttachPopupMenu(const DString& menuXml = DString(),
                         const ContextMenuActionEvent& actionCallback = nullptr,
                         const ContextMenuCreatingEvent& creatingCallback = nullptr);

    /** 处理上下文菜单项被激活
     *
     *  AttachPopupMenu 未指定回调时，控件内置菜单（复制/全选）的点击会转到这里。
     */
    void OnContextMenuAction(const DString& strItemName);

    /** 按当前状态刷新右键菜单项的可用性
     *
     *  消息列表是只读的：没有选区时"复制"没有意义，没有消息时"全选"没有意义，
     *  这里把对应菜单项置为禁用，避免点了没反应或误复制全部内容。
     */
    void UpdateContextMenuItemEnabled(Menu* pMenu);

    /** 右键菜单弹出时，若当前没有选区且右键点落在某条消息上，则选中该条
     *
     *  框架是"先创建菜单、再把右键事件派发给应用层"的，应用层在回调里再选中消息已经太晚，
     *  菜单项的可用状态会停留在"没有选区"上，因此控件自己补这一步。
     */
    void PrepareContextMenuSelection();

private:
    /** 获取可见内容区高度（控件高度扣除内边距）
    */
    int32_t GetVisibleHeight() const;

    /** 获取内容区宽度（固定预留滚动条宽度，保持恒定，避免与滚动范围互相影响）
    */
    int32_t CalcContentWidth() const;

    /** 按指定的控件矩形计算内容区宽度（用于布局开始前预先算出内容尺寸）
    */
    int32_t CalcContentWidthFor(const UiRect& rcControl) const;

    /** 按控件矩形更新消息布局与占位控件高度，使 ScrollBox 能推导出正确的滚动范围
    */
    void UpdateContentSize(const UiRect& rcControl);

    /** 绘制一条消息
    */
    void PaintMessage(IRender* pRender, size_t nMessageIndex, const Message& message, const UiRect& rcMessage);

    /** 绘制时间分隔条
    */
    void PaintTimeDivider(IRender* pRender, const DString& strTime, const UiRect& rcDivider);

    /** 获取"回到最新消息"提示的矩形（未显示时返回空矩形）
    */
    UiRect GetNewMessageTipRect(IRender* pRender);

    /** 获取"回到最新消息"提示的文案（含未读条数）
    */
    DString GetNewMessageTipText() const;

    /** 绘制"回到最新消息"提示
    */
    void PaintNewMessageTip(IRender* pRender);

    /** 消息插入后同步选区索引，避免选区错位到别的消息上
    */
    void SyncSelectionOnInsert(size_t nInsertIndex);

    /** 让某条消息的换行缓存失效（nIndex 超出范围时全部失效）
    */
    void InvalidateLineCache(size_t nIndex);

    /** 计算单条消息占用的高度（含状态文字，不含上方时间分隔条）
     *  @param [in] nMessageIndex 消息索引（用于复用换行结果缓存）
    */
    int32_t CalcMessageHeight(IRender* pRender, const Message& message, int32_t nContentWidth,
                             size_t nMessageIndex);

    /** 是否需要为该消息绘制时间分隔条
    */
    bool IsTimeDividerShown(size_t nIndex) const;

    /** 确保布局缓存有效：按需重算高度、重建消息偏移表
     *  @param [in] nContentWidth 内容区宽度（与缓存宽度不同时，所有消息高度失效）
     *  @return 布局是否可用
    */
    bool EnsureLayout(IRender* pRender, int32_t nContentWidth);

    /** 获取某条消息的矩形（需先调用 EnsureLayout）
    */
    bool GetMessageRect(size_t nIndex, UiRect& rcMessage) const;

    /** 查找指定内容偏移处（相对内容顶部）的第一条可见消息（需先调用 EnsureLayout）
    */
    size_t FindFirstVisibleIndex(int32_t nContentOffset) const;

    /** 更新滚动范围
    */
    void UpdateScrollRange(IRender* pRender);

    /** 文本行：按气泡宽度自动换行后的一行
     *
     *  控件的文本绘制、选区高亮与命中测试都基于这份行数据，
     *  这样文字的排版结果与高亮框能够严格对齐。
    */
    struct TextLine
    {
        int32_t nStart = 0;   //起始字符索引（含）
        int32_t nEnd = 0;     //结束字符索引（不含，换行符不计入）
    };

    /** 消息布局信息：气泡与文本的矩形
    */
    struct BubbleLayout
    {
        UiRect rcBubble;      //气泡矩形
        UiRect rcText;        //气泡内文本矩形
        UiRect rcAvatar;      //头像矩形
        UiRect rcStatus;      //状态文字矩形（bShowStatus 为 false 时无意义）
        int32_t nTextWidth = 0;
        int32_t nTextHeight = 0;
        int32_t nLineHeight = 0;            //单行文本的行高
        std::vector<TextLine> lines;        //换行后各行的字符区间
        bool bShowStatus = false;
        bool bValid = false;
    };

    /** 换行结果缓存：同一条消息在相同文本宽度下的行数据只需计算一次
     *
     *  绘制、命中测试与高度计算都会用到行数据，若不缓存，滚动时每条可见消息
     *  每一帧都要重新做 O(log n) 次文本测量。
    */
    struct LineCache
    {
        int32_t nMaxTextWidth = -1;   //缓存对应的文本最大宽度，-1 表示尚未缓存
        int32_t nLineHeight = 0;      //单行行高
        std::vector<TextLine> lines;  //换行后的各行字符区间
    };

    /** 计算消息的气泡布局（气泡宽度自适应文本，高度按换行结果）
     *  @param [out] layout 计算结果（含换行后的行数据）
     *  @param [in] nMessageIndex 消息索引（用于复用换行结果缓存）
    */
    bool CalcBubbleLayout(IRender* pRender, const Message& message, const UiRect& rcMessage,
                          BubbleLayout& layout, size_t nMessageIndex);

    /** 计算气泡最大宽度
    */
    int32_t CalcMaxBubbleWidth(int32_t nContentWidth) const;

    /** 计算文本的自然宽度（单行）
    */
    int32_t MeasureNaturalTextWidth(IRender* pRender, const DString& strText, const DString& strFontId) const;

    /** 测量文本高度（支持多行换行）
    */
    int32_t MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId);

    /** 绘制文本（命名避免与 Windows SDK 的 DrawText 宏冲突）
    */
    void DrawChatText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                      const DString& strColor, const DString& strFontId);

    /** 绘制消息文本，选中部分使用选中文字色分段绘制
    */
    void DrawMessageText(IRender* pRender, size_t nMessageIndex, const Message& message,
                         const BubbleLayout& layout, const DString& strTextColor);

    /** 绘制头像（彩色圆形 + 首字母）
    */
    void PaintAvatar(IRender* pRender, const UiRect& rcAvatar, const DString& strText);

    /** 根据头像文字生成稳定的配色
    */
    UiColor GetAvatarColor(const DString& strText) const;

    /** 消息命中测试：找到鼠标位置对应的消息、命中区域与字符索引
     *  @param [out] nMessageIndex 消息索引
     *  @param [out] hitArea 命中区域
     *  @param [out] nCharIndex 文本字符索引（UTF-16 码元索引），未命中文本区时为 -1
     *  @return 是否命中某条消息
    */
    bool HitTestMessage(const UiPoint& ptMouse, size_t& nMessageIndex,
                        ChatMessageHitArea& hitArea, int32_t& nCharIndex);

    /** 消息命中测试（拖选专用）：始终返回最近的字符位置
     *
     *  与 HitTestMessage 的区别：鼠标落在消息之间的间距、时间分隔条或消息两侧空白时，
     *  会把位置钳位到最近的字符（上一条消息末尾/本条消息首尾），
     *  这样跨消息拖选时选区可以连续延伸，不会因为穿过间隙而中断。
     */
    bool HitTestMessageClamped(const UiPoint& ptMouse, size_t& nMessageIndex, int32_t& nCharIndex);

    /** 按"锚点 + 拖选终点"重算选区区间并刷新绘制
    */
    void ApplySelectionRange(size_t nMessageIndex, int32_t nCharIndex);

    /** 拖选时更新自动滚动状态：鼠标拖出可视区域后按固定间隔滚动内容
    */
    void UpdateDragAutoScroll(const UiPoint& ptMouse);

    /** 提交一次自动滚动任务
    */
    void PostDragScrollTask();

    /** 自动滚动任务：滚动一帧并重新计算选区，需要时继续排队
    */
    void OnDragScrollTask();

    /** 停止拖选自动滚动
    */
    void StopDragAutoScroll();

    /** 绘制消息状态文字（发送中/发送失败/已送达/已读）
    */
    void PaintStatus(IRender* pRender, const Message& message, const BubbleLayout& layout);

    /** 获取状态文字颜色（语义色名）
    */
    DString GetStatusColor(ChatMessageStatus status) const;

    /** 计算文本前缀的宽度（用于字符定位）
    */
    int32_t MeasureTextPrefixWidth(IRender* pRender, const DString& strText, int32_t nCharCount,
                                    const DString& strFontId);

    /** 计算文本区间 [nStart, nEnd) 的宽度（用于行内字符定位）
    */
    int32_t MeasureTextRangeWidth(IRender* pRender, const DString& strText, int32_t nStart, int32_t nEnd,
                                  const DString& strFontId);

    /** 获取文本的单行行高
     *
     *  注意：不能用整段文本去测量（那样得到的是多行文本的总高度），
     *  这里用几个典型字符测量单行高度，多行文本的行距统一使用该值。
    */
    int32_t GetTextLineHeight(IRender* pRender, const DString& strFontId);

    /** 按最大宽度对文本换行，输出每一行的字符区间
     *
     *  文本里显式的换行符（'\n' 或 '\r'）会强制换行，其余部分按宽度贪心折行，
     *  折行时优先在空格/连字符处断开，避免把英文单词从中间截断。
    */
    void WrapTextLines(IRender* pRender, const DString& strText, int32_t nMaxWidth,
                       const DString& strFontId, std::vector<TextLine>& lines);

    /** 文本区域内的命中测试：由鼠标坐标定位字符索引（按行定位，支持多行文本）
    */
    int32_t HitTestTextChar(IRender* pRender, const DString& strText, const BubbleLayout& layout,
                            const UiPoint& ptMouse);

    /** 绘制选中区域背景
    */
    void PaintSelection(IRender* pRender, size_t nMessageIndex, const BubbleLayout& layout);

    /** 追加一条消息的内部实现
     *  @return 消息 id
    */
    int64_t InsertMessageInternal(size_t index, const Message& message);

    /** 生成唯一消息 id
    */
    int64_t GenerateMessageId();

    /** 标记某条消息的高度缓存失效（下次布局时重新测量）
    */
    void MarkMessageHeightDirty(size_t nIndex);

    /** 更新内容高度缓存（消息变化或宽度变化时调用）
    */
    void InvalidateLayoutCache();

    /** 宽度变化时调用：所有消息高度需要重新测量
    */
    void InvalidateWidthCache();

    /** 创建/更新用于承载内容高度的占位控件
     *
     *  ScrollBox 的滚动范围由子控件的布局尺寸推导，纯自绘内容没有子控件，
     *  会导致 ScrollBox 判定"无需滚动"并把滚动条隐藏。这里用一个透明的占位控件
     *  承载内容总高度，使框架能自动管理滚动条的范围、位置与可见性。
     */
    void UpdateContentPlaceholder(int32_t nContentHeight);

private:
    /** 消息数据
    */
    std::vector<Message> m_messages;

    /** 用于承载内容总高度的透明占位控件（供 ScrollBox 推导滚动范围）
    */
    Control* m_pContentPlaceholder;

    /** 单条消息的缓存高度（含状态文字），-1 表示待计算
    */
    std::vector<int32_t> m_msgHeights;

    /** 消息顶部相对内容顶部的偏移（单调递增），布局变化时重建
    */
    std::vector<int32_t> m_msgOffsets;

    /** 每条消息的换行结果缓存（与消息数量一一对应）
    */
    std::vector<LineCache> m_msgLineCache;

    /** 布局缓存是否失效（消息增删改、间距或时间分隔条配置变化时置位）
    */
    bool m_bLayoutDirty;

    /** 布局缓存对应的内容宽度，-1 表示尚未布局
    */
    int32_t m_nLayoutWidth;

    /** 头像直径（像素，DPI 缩放）
    */
    int32_t m_nAvatarSize;

    /** 气泡最大宽度（像素，DPI 缩放）
    */
    int32_t m_nBubbleMaxWidth;

    /** 气泡内边距（像素，DPI 缩放）
    */
    int32_t m_nBubblePaddingH;
    int32_t m_nBubblePaddingV;

    /** 气泡圆角半径（像素，DPI 缩放）
    */
    int32_t m_nBubbleRadius;

    /** 相邻消息间距（像素，DPI 缩放）
    */
    int32_t m_nBubbleSpacing;

    /** 是否显示时间分隔条
    */
    bool m_bShowTime;

    /** 字体
    */
    DString m_strAvatarFont;
    DString m_strTextFont;
    DString m_strTimeFont;

    /** 颜色（语义色名）
    */
    DString m_strBubbleColorSelf;
    DString m_strBubbleColorOther;
    DString m_strTextColorSelf;
    DString m_strTextColorOther;
    DString m_strAvatarTextColor;
    DString m_strSystemTextColor;
    DString m_strSelectionBkColor;
    DString m_strSelectionTextColor;
    DString m_strScrollbarTrackColor;
    DString m_strScrollbarThumbColor;

    /** 是否仅在鼠标悬停时显示滚动条
    */
    bool m_bAutoHideScrollbar;

    /** 是否自动滚动到底部
    */
    bool m_bAutoScrollToBottom;

    /** 视图下方尚未查看的新消息条数（不在底部时累加，滚到底部清零）
    */
    int32_t m_nUnreadBelow;

    /** 是否显示"回到最新消息"提示
    */
    bool m_bShowNewMessageTip;

    /** 是否允许鼠标拖选文本
    */
    bool m_bEnableSelection;

    /** 当前滚动偏移（像素）
    */
    int32_t m_nScrollPos;

    /** 内容总高度（像素）
    */
    int32_t m_nContentHeight;

    /** 状态文字配置
    */
    bool m_bShowStatus;
    DString m_strStatusFont;
    DString m_strStatusTextColor;
    DString m_strStatusFailedColor;

    /** 自定义状态文案（空字符串表示使用多语言资源中的默认文案）
    */
    DString m_strStatusTextSending;
    DString m_strStatusTextFailed;
    DString m_strStatusTextDelivered;
    DString m_strStatusTextRead;

    /** 消息事件回调
    */
    ChatMessageClickEvent m_msgClickCallback;
    ChatMessageActionEvent m_msgDoubleClickCallback;
    ChatMessageActionEvent m_msgRightClickCallback;

    /** 点击状态：按下时记录命中的消息与区域，抬起时若未发生拖选则触发点击回调
    */
    bool m_bPendingClick;
    size_t m_nClickIndex;
    ChatMessageHitArea m_eClickArea;

    /** 文本选择状态
    */
    bool m_bSelecting;          //正在拖动选择
    bool m_bPendingSelect;       //鼠标已按下，等待移动超过阈值后开始选择
    bool m_bHasSelection;        //当前是否有选区
    UiPoint m_ptMouseDown;       //鼠标按下位置
    UiPoint m_ptSelectEnd;       //拖选过程中最后一次鼠标位置（用于自动滚动后重算选区）
    size_t m_nAnchorIndex;       //拖选锚点所在的消息索引（按下时的位置，拖选过程中不变）
    int32_t m_nAnchorChar;       //拖选锚点所在的字符索引
    size_t m_nSelStartIndex;     //选区起始消息索引
    size_t m_nSelEndIndex;       //选区结束消息索引
    int32_t m_nSelStartChar;     //选区起始字符索引
    int32_t m_nSelEndChar;       //选区结束字符索引

    /** 拖选时的自动滚动状态：鼠标拖出可视区域后，按固定间隔滚动内容
    */
    int32_t m_nDragScrollDir;    //滚动方向：-1 向上，1 向下，0 不滚动
    bool m_bDragScrollPosted;    //是否已提交滚动任务（避免重复排队）
};

} // namespace ui

#endif // UI_CONTROL_CHATMESSAGELIST_H_
