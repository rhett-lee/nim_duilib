#ifndef UI_CONTROL_CHATMESSAGELIST_H_
#define UI_CONTROL_CHATMESSAGELIST_H_

#include "duilib/Box/ScrollBox.h"
#include <vector>

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

/** 聊天消息列表控件（继承 ScrollBox，自绘消息 + 框架滚动条）
 *
 *  模拟真实 IM 聊天界面：消息气泡按左右方向分栏、气泡宽度随文本自适应、
 *  头像为彩色圆形（可用首字母）、可选时间分隔条，支持滚动与自动滚动到底部。
 *  支持鼠标拖选文本、双击选词、Ctrl+C 复制、右键菜单。
 *
 *  XML 属性：
 *  - avatar_size：头像直径（像素，DPI 缩放），默认 36
 *  - bubble_max_width：气泡最大宽度（像素，DPI 缩放），默认 480
 *  - bubble_padding_h / bubble_padding_v：气泡内边距（像素，DPI 缩放），默认 12 / 8
 *  - bubble_radius：气泡圆角半径（像素，DPI 缩放），默认 10
 *  - bubble_spacing：相邻消息间距（像素，DPI 缩放），默认 14
 *  - show_time：是否显示时间分隔条，默认 true
 *  - time_format：时间分隔条格式，默认 "HH:mm"
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
 *  - scrollbar_track_color：滚动条轨道颜色，默认 bg_scrollbar_normal
 *  - scrollbar_thumb_color：滚动条滑块颜色，默认 bg_scrollbar_thumb_normal
 *  - auto_hide_scrollbar：是否仅在鼠标悬停时显示滚动条，默认 false（常显）
 *  - enable_selection：是否允许鼠标拖选文本，默认 true
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
    */
    void AddMessage(const Message& message);

    /** 追加一条消息（便捷重载）
    */
    void AddMessage(ChatMessageSide side, const DString& strText,
                    const DString& strAvatarText = _T(""), const DString& strTime = _T(""));

    /** 在指定位置插入一条消息
    */
    void InsertMessage(size_t index, const Message& message);

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
    */
    void SetAutoScrollToBottom(bool bAutoScroll);
    bool IsAutoScrollToBottom() const { return m_bAutoScrollToBottom; }

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
    */
    void OnContextMenuAction(const DString& strItemName);

private:
    /** 获取可见内容区高度（控件高度扣除内边距）
    */
    int32_t GetVisibleHeight() const;

    /** 获取内容区宽度（固定预留滚动条宽度，保持恒定，避免与滚动范围互相影响）
    */
    int32_t CalcContentWidth() const;

    /** 绘制一条消息
    */
    void PaintMessage(IRender* pRender, size_t nMessageIndex, const Message& message, const UiRect& rcMessage);

    /** 绘制时间分隔条
    */
    void PaintTimeDivider(IRender* pRender, const DString& strTime, const UiRect& rcDivider);

    /** 计算单条消息占用的高度（含上方时间分隔条）
    */
    int32_t CalcMessageHeight(IRender* pRender, const Message& message, int32_t nContentWidth);

    /** 计算单条消息占用的高度（带缓存，宽度变化时失效）
    */
    int32_t CalcMessageHeightCached(IRender* pRender, size_t nIndex, int32_t nContentWidth);

    /** 计算全部消息总高度
    */
    int32_t CalcTotalHeight(IRender* pRender, int32_t nContentWidth);

    /** 更新滚动范围
    */
    void UpdateScrollRange(IRender* pRender);

    /** 消息布局信息：气泡与文本的矩形
    */
    struct BubbleLayout
    {
        UiRect rcBubble;      //气泡矩形
        UiRect rcText;        //气泡内文本矩形
        UiRect rcAvatar;      //头像矩形
        int32_t nTextWidth = 0;
        int32_t nTextHeight = 0;
        bool bValid = false;
    };

    /** 计算消息的气泡布局（气泡宽度自适应文本，高度按换行结果）
     *  @param [out] layout 计算结果
     *  @param [in] bCalcTextHeight 是否需要计算文本高度（绘制时为true，仅算高度时可按需跳过）
    */
    bool CalcBubbleLayout(IRender* pRender, const Message& message, const UiRect& rcMessage,
                          BubbleLayout& layout, bool bCalcTextHeight);

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

    /** 文本命中测试：找到鼠标位置对应的消息与字符索引
     *  @param [out] nCharIndex 字符索引（UTF-16 码元索引）
     *  @return 是否命中某条消息的文本
    */
    bool HitTestText(const UiPoint& ptMouse, size_t& nMessageIndex, int32_t& nCharIndex);

    /** 获取某条消息的文本区域（用于命中测试与选择绘制）
    */
    bool GetMessageTextRect(IRender* pRender, size_t nMessageIndex, UiRect& rcText);

    /** 计算文本前缀的宽度（用于字符定位）
    */
    int32_t MeasureTextPrefixWidth(IRender* pRender, const DString& strText, int32_t nCharCount,
                                    const DString& strFontId);

    /** 绘制选中区域背景
    */
    void PaintSelection(IRender* pRender, size_t nMessageIndex, const BubbleLayout& layout);

    /** 追加一条消息的内部实现
    */
    void InsertMessageInternal(size_t index, const Message& message);

    /** 生成唯一消息 id
    */
    int64_t GenerateMessageId();

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

    /** 同步消息高度缓存与消息数量
    */
    void SyncMessageHeightCache();

private:
    /** 消息数据
    */
    std::vector<Message> m_messages;

    /** 用于承载内容总高度的透明占位控件（供 ScrollBox 推导滚动范围）
    */
    Control* m_pContentPlaceholder;

    /** 单条消息的缓存高度（含上方时间分隔条），宽度变化时失效
    */
    std::vector<int32_t> m_msgHeights;

    /** 缓存对应的内容宽度，-1 表示无效
    */
    int32_t m_nCachedLayoutWidth;

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

    /** 时间分隔条格式
    */
    DString m_strTimeFormat;

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

    /** 是否允许鼠标拖选文本
    */
    bool m_bEnableSelection;

    /** 当前滚动偏移（像素）
    */
    int32_t m_nScrollPos;

    /** 内容总高度（像素）
    */
    int32_t m_nContentHeight;

    /** 内容总高度缓存（宽度变化时失效）
    */
    int32_t m_nCachedContentHeight;
    int32_t m_nCachedContentWidth;
    /** 文本选择状态
    */
    bool m_bSelecting;          //正在拖动选择
    bool m_bPendingSelect;       //鼠标已按下，等待移动超过阈值后开始选择
    bool m_bHasSelection;        //当前是否有选区
    UiPoint m_ptMouseDown;       //鼠标按下位置
    size_t m_nSelStartIndex;     //选区起始消息索引
    size_t m_nSelEndIndex;       //选区结束消息索引
    int32_t m_nSelStartChar;     //选区起始字符索引
    int32_t m_nSelEndChar;       //选区结束字符索引
};

} // namespace ui

#endif // UI_CONTROL_CHATMESSAGELIST_H_
