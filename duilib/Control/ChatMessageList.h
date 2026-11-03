#ifndef UI_CONTROL_CHATMESSAGELIST_H_
#define UI_CONTROL_CHATMESSAGELIST_H_

#include "duilib/Core/Control.h"
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

/** 聊天消息列表控件（继承 Control，纯自绘）
 *
 *  模拟真实 IM 聊天界面：消息气泡按左右方向分栏、气泡宽度随文本自适应、
 *  头像为彩色圆形（可用首字母）、可选时间分隔条，支持滚动与自动滚动到底部。
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
 *  - scrollbar_track_color：滚动条轨道颜色，默认 bg_scrollbar_normal
 *  - scrollbar_thumb_color：滚动条滑块颜色，默认 bg_scrollbar_thumb_normal
 *  - auto_hide_scrollbar：是否仅在鼠标悬停时显示滚动条，默认 true
 */
class DUILIB_API ChatMessageList : public Control
{
    typedef Control BaseClass;

public:
    /** 单条消息
    */
    struct Message
    {
        ChatMessageSide side = ChatMessageSide::kOther;
        DString text;          //消息文本
        DString avatarText;    //头像文字（通常为姓名首字）
        DString time;          //时间文本（用于时间分隔条）
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

    /** 绘制消息列表
    */
    void PaintStateImages(IRender* pRender) override;

    /** 鼠标交互：滚轮滚动、滚动条拖动
    */
    bool ButtonDown(const EventArgs& msg) override;
    bool ButtonUp(const EventArgs& msg) override;
    bool MouseMove(const EventArgs& msg) override;
    bool MouseWheel(const EventArgs& msg) override;
    bool MouseEnter(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;

public:
    /** 追加一条消息
    */
    void AddMessage(const Message& message);

    /** 追加一条消息（便捷重载）
    */
    void AddMessage(ChatMessageSide side, const DString& strText,
                    const DString& strAvatarText = _T(""), const DString& strTime = _T(""));

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

    /** 获取当前滚动偏移（像素）
    */
    int32_t GetScrollPos() const { return m_nScrollPos; }

    /** 设置当前滚动偏移（像素，自动钳位）
    */
    void SetScrollPos(int32_t nPos);

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

private:
    /** 获取可见内容区高度（控件高度扣除内边距）
    */
    int32_t GetVisibleHeight() const;

    /** 绘制一条消息
    */
    void PaintMessage(IRender* pRender, const Message& message, const UiRect& rcMessage);

    /** 绘制时间分隔条
    */
    void PaintTimeDivider(IRender* pRender, const DString& strTime, const UiRect& rcDivider);

    /** 计算单条消息占用的高度（含上方时间分隔条）
    */
    int32_t CalcMessageHeight(IRender* pRender, const Message& message, int32_t nContentWidth);

    /** 计算全部消息总高度
    */
    int32_t CalcTotalHeight(IRender* pRender, int32_t nContentWidth);

    /** 更新滚动范围
    */
    void UpdateScrollRange(IRender* pRender);

    /** 计算气泡矩形
    */
    UiRect CalcBubbleRect(const UiRect& rcMessage, const Message& message, int32_t nBubbleWidth,
                          int32_t nMessageHeight) const;

    /** 测量文本高度（支持多行换行）
    */
    int32_t MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId);

    /** 绘制文本（命名避免与 Windows SDK 的 DrawText 宏冲突）
    */
    void DrawChatText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                      const DString& strColor, const DString& strFontId);

    /** 绘制头像（彩色圆形 + 首字母）
    */
    void PaintAvatar(IRender* pRender, const UiRect& rcAvatar, const DString& strText);

    /** 根据头像文字生成稳定的配色
    */
    UiColor GetAvatarColor(const DString& strText) const;

    /** 计算滚动条滑块矩形
    */
    UiRect CalcScrollThumbRect(const UiRect& rc) const;

    /** 绘制滚动条
    */
    void DrawScrollbar(IRender* pRender, const UiRect& rc);

    /** 判断鼠标是否命中滚动条滑块
    */
    bool HitTestScrollThumb(const UiPoint& ptMouse) const;

private:
    /** 消息数据
    */
    std::vector<Message> m_messages;

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
    DString m_strScrollbarTrackColor;
    DString m_strScrollbarThumbColor;

    /** 是否仅在鼠标悬停时显示滚动条
    */
    bool m_bAutoHideScrollbar;

    /** 是否自动滚动到底部
    */
    bool m_bAutoScrollToBottom;

    /** 当前滚动偏移（像素）
    */
    int32_t m_nScrollPos;

    /** 内容总高度（像素）
    */
    int32_t m_nContentHeight;

    /** 滚动条交互状态
    */
    bool m_bDraggingScrollbar;
    bool m_bMouseHover;
    int32_t m_dragStartY;
    int32_t m_dragStartScrollPos;
};

} // namespace ui

#endif // UI_CONTROL_CHATMESSAGELIST_H_
