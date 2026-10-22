#ifndef UI_CONTROL_BREADCRUMB_H_
#define UI_CONTROL_BREADCRUMB_H_

#include "duilib/Core/Control.h"
#include <vector>

namespace ui {

/** Breadcrumb 当前项变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
*/
struct BreadcrumbValue
{
    size_t currentIndex = 0; //新的当前项索引
    size_t oldIndex = 0;     //变化前的当前项索引
};

/** Breadcrumb 面包屑导航控件（继承 Control，纯自绘）
 *
 *  用于文件 / 目录路径、站点层级、设置页层级等导航场景：
 *  用一行可点击的祖先路径 + 当前项，表达"我在哪一层、怎么回到上层"。
 *  与 AddressBar（浏览器地址栏）、Stepper（流程状态机）语义不同，故独立实现。
 *
 *  数据模型：条目文本按 `|` 分隔；当前项默认跟随最后一项，
 *  也可由 `current` 属性或 C++ API 显式指定；被禁用的条目不可点击。
 *
 *  XML 属性：
 *  - item_text：条目文本，`|` 分隔多条
 *  - item_text_id：条目文本的多语言 ID，`|` 分隔多条（语言切换自动刷新）
 *  - current / current_index：当前项索引（0 起），默认 -1 = 自动跟随最后一项
 *  - separator / separator_text：条目之间的分隔符，默认 "›"
 *  - spacing：分隔符两侧的间距（像素，DPI 缩放），默认 4
 *  - item_padding：条目点击区域左右内边距（像素，DPI 缩放），默认 6
 *  - item_round：悬停/按下胶囊背景的圆角半径（像素，DPI 缩放），默认 4
 *  - click_to_jump：是否允许点击祖先项切换当前项，默认 true
 *  - disabled_items：禁用项索引列表，`|` 或 `,` 分隔，如 "1|2"
 *  - item_color / current_color / hovered_color / pressed_color / disabled_color：各状态文本颜色
 *  - hover_bg_color / pressed_bg_color：悬停/按下状态的胶囊背景色（留空则不绘制背景）
 *  - separator_color：分隔符颜色
 *  - font：普通条目与分隔符的字体 ID，默认 system_regular_14
 *  - current_font：当前项字体 ID，默认 system_bold_14（加粗强调）
 *
 *  说明：宽度不足时整体右对齐绘制（左端裁剪），保证末项 / 当前项可见；
 *        文本颜色优先级：禁用 > 按下 > 悬停 > 当前 > 普通；
 *        可交互条目悬停/按下时绘制圆角胶囊背景，当前项以加粗字体区分层级。
 */
class DUILIB_API Breadcrumb : public Control
{
    typedef Control BaseClass;

public:
    /** 条目数据
    */
    struct Item
    {
        DString text; //条目文本
    };

public:
    explicit Breadcrumb(Window* pWindow);
    Breadcrumb(const Breadcrumb& r) = delete;
    Breadcrumb& operator=(const Breadcrumb& r) = delete;
    ~Breadcrumb() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制面包屑内容
    */
    void PaintStateImages(IRender* pRender) override;

    /** 语言切换时刷新多语言数据绑定
    */
    void OnLanguageChanged(bool bRedraw) override;

    /** 鼠标左键按下：点击祖先项切换当前项
    */
    bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标左键抬起：结束按下态
    */
    bool ButtonUp(const EventArgs& msg) override;

    /** 鼠标移动：更新悬停项（用于 hover 态与光标）
    */
    bool MouseMove(const EventArgs& msg) override;

    /** 鼠标离开控件：清除悬停/按下态
    */
    bool MouseLeave(const EventArgs& msg) override;

public:
    /** 追加一个条目
    */
    void AddItem(const DString& text);

    /** 在指定位置插入条目
    */
    bool InsertItem(size_t index, const DString& text);

    /** 删除指定条目
    */
    bool RemoveItemAt(size_t index);

    /** 清空所有条目（同时清空禁用集合）
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

    /** 获取当前项索引（已钳位到有效范围；空数据时返回 0）
    */
    size_t GetCurrentIndex() const;

    /** 设置当前项索引（自动钳位，索引变化时触发 kEventValueChanged 事件）
    */
    void SetCurrentIndex(size_t nIndex);

    /** 设置当前项索引（不触发事件，供程序内部同步使用）
    */
    void SetCurrentIndexNoEvent(size_t nIndex);

    /** 跳转到指定条目（目标禁用或索引越界时返回 false，触发事件）
    */
    bool GoTo(size_t nIndex);

    /** 跳转到指定条目（不触发事件）
    */
    bool GoToNoEvent(size_t nIndex);

    /** 设置指定条目是否禁用
    */
    void SetItemEnabled(size_t index, bool bEnabled);

    /** 获取指定条目是否可用（未禁用）
    */
    bool IsItemEnabled(size_t index) const;

    /** 批量设置禁用条目（替换全部）
    */
    void SetDisabledItems(const std::vector<size_t>& disabledItems);

    /** 获取禁用条目列表
    */
    const std::vector<size_t>& GetDisabledItems() const { return m_disabledItems; }

    /** 设置分隔符文本
    */
    void SetSeparator(const DString& strSeparator);

    /** 获取分隔符文本
    */
    const DString& GetSeparator() const { return m_strSeparator; }

    /** 设置分隔符两侧间距（像素，自动 DPI 缩放）
    */
    void SetSpacing(int32_t nSpacing);

    /** 获取分隔符两侧间距（未缩放原始值）
    */
    int32_t GetSpacing() const { return m_nSpacing; }

    /** 设置条目点击区域左右内边距（像素，自动 DPI 缩放）
    */
    void SetItemPadding(int32_t nPadding);

    /** 获取条目点击区域左右内边距（未缩放原始值）
    */
    int32_t GetItemPadding() const { return m_nItemPadding; }

    /** 设置是否允许点击祖先项跳转
    */
    void SetClickToJump(bool bEnable);

    /** 获取是否允许点击祖先项跳转
    */
    bool IsClickToJump() const { return m_bClickToJump; }

    /** 设置可点击条目颜色（语义色名或颜色值）
    */
    void SetItemColor(const DString& strColor);

    /** 获取可点击条目颜色
    */
    const DString& GetItemColor() const { return m_strItemColor; }

    /** 设置当前项颜色
    */
    void SetCurrentColor(const DString& strColor);

    /** 获取当前项颜色
    */
    const DString& GetCurrentColor() const { return m_strCurrentColor; }

    /** 设置悬停项颜色
    */
    void SetHoveredColor(const DString& strColor);

    /** 获取悬停项颜色
    */
    const DString& GetHoveredColor() const { return m_strHoveredColor; }

    /** 设置按下项颜色
    */
    void SetPressedColor(const DString& strColor);

    /** 获取按下项颜色
    */
    const DString& GetPressedColor() const { return m_strPressedColor; }

    /** 设置禁用项颜色
    */
    void SetDisabledColor(const DString& strColor);

    /** 获取禁用项颜色
    */
    const DString& GetDisabledColor() const { return m_strDisabledColor; }

    /** 设置分隔符颜色
    */
    void SetSeparatorColor(const DString& strColor);

    /** 获取分隔符颜色
    */
    const DString& GetSeparatorColor() const { return m_strSeparatorColor; }

    /** 设置字体 ID
    */
    void SetFont(const DString& strFontId);

    /** 获取字体 ID
    */
    const DString& GetFont() const { return m_strFont; }

    /** 设置当前项字体 ID（加粗强调）
    */
    void SetCurrentFont(const DString& strFontId);

    /** 获取当前项字体 ID
    */
    const DString& GetCurrentFont() const { return m_strCurrentFont; }

    /** 设置悬停胶囊背景色（留空则不绘制背景）
    */
    void SetHoveredBgColor(const DString& strColor);

    /** 获取悬停胶囊背景色
    */
    const DString& GetHoveredBgColor() const { return m_strHoveredBgColor; }

    /** 设置按下胶囊背景色（留空则不绘制背景）
    */
    void SetPressedBgColor(const DString& strColor);

    /** 获取按下胶囊背景色
    */
    const DString& GetPressedBgColor() const { return m_strPressedBgColor; }

    /** 设置胶囊背景圆角半径（像素，自动 DPI 缩放）
    */
    void SetItemRound(int32_t nRound);

    /** 获取胶囊背景圆角半径（未缩放原始值）
    */
    int32_t GetItemRound() const { return m_nItemRound; }

    /** 监听当前项变化事件
    * @param [in] callback 当前项变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 3（面包屑模式标记）
    *   lParam: 0
    *   pEventData: 指向 BreadcrumbValue（含 currentIndex / oldIndex），
    *               仅在本次回调派发期间有效，请勿保存该指针
    */
    void AttachCurrentChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

private:
    /** 布局条目（绘制与命中检测共用）
    */
    struct LayoutEntry
    {
        int32_t nItemIndex = -1; //条目索引；-1 表示分隔符
        UiRect rc;               //条目/分隔符的绘制与命中矩形
    };

    /** 解析 XML 数据绑定字段（`|` 分隔，整体替换）
    * @param [in] strValue 字段值字符串
    * @param [in] bTranslate 是否为多语言 ID 模式（每个分段按 STRID 从语言文件取实际文本）
    */
    void ParseItemsField(const DString& strValue, bool bTranslate = false);

    /** 解析禁用条目索引列表
    */
    void ParseDisabledItems(const DString& strValue);

    /** 派发当前项变化事件（m_bMuteEvent 为 true 时不派发）
    * @param [in] nOldIndex 变化前的索引
    */
    void EmitCurrentChanged(size_t nOldIndex);

    /** 把显式当前索引钳位到有效范围（-1 表示自动跟随末项，保持不变）
    */
    void ClampCurrentIndex();

    /** 判断条目是否可交互（可点击祖先项）
    */
    bool IsItemInteractive(size_t index) const;

    /** 获取指定条目是否被禁用
    */
    bool IsItemDisabled(size_t index) const;

    /** 测量文本宽度
    * @param [in] strFontId 测量使用的字体 ID
    */
    int32_t MeasureTextWidth(IRender* pRender, const DString& strText, const DString& strFontId);

    /** 计算布局并更新 m_layout（绘制时调用）
    */
    void BuildLayout(IRender* pRender, const UiRect& rc);

    /** 命中检测：返回鼠标所在的条目索引，未命中返回 -1
    */
    int32_t HitTestItem(const UiPoint& ptMouse) const;

    /** 绘制文本（封装 DrawStringParam）
    * @param [in] strFontId 绘制使用的字体 ID
    */
    void DrawBreadcrumbText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                            UiColor textColor, const DString& strFontId);

    /** 按悬停项更新鼠标光标
    */
    void UpdateCursor(int32_t nHoveredIndex);

private:
    /** 全部条目
    */
    std::vector<Item> m_items;

    /** 禁用条目索引集合
    */
    std::vector<size_t> m_disabledItems;

    /** 多语言条目 ID 串（item_text_id 的原始值，语言切换时重新解析）
    */
    DString m_strItemTextIds;

    /** 分隔符文本
    */
    DString m_strSeparator;

    /** 各状态文本颜色（语义色名或颜色值）
    */
    DString m_strItemColor;
    DString m_strCurrentColor;
    DString m_strHoveredColor;
    DString m_strPressedColor;
    DString m_strDisabledColor;
    DString m_strSeparatorColor;

    /** 悬停 / 按下状态的胶囊背景色（留空则不绘制）
    */
    DString m_strHoveredBgColor;
    DString m_strPressedBgColor;

    /** 字体 ID（普通条目与分隔符）
    */
    DString m_strFont;

    /** 当前项字体 ID（加粗强调）
    */
    DString m_strCurrentFont;

    /** 当前项索引（-1 表示自动跟随最后一项）
    */
    int32_t m_nCurrentIndex;

    /** 分隔符两侧间距（DPI 缩放前原始值）
    */
    int32_t m_nSpacing;

    /** 条目点击区域左右内边距（DPI 缩放前原始值）
    */
    int32_t m_nItemPadding;

    /** 胶囊背景圆角半径（DPI 缩放前原始值）
    */
    int32_t m_nItemRound;

    /** 是否允许点击祖先项跳转
    */
    bool m_bClickToJump;

    /** 是否抑制事件派发（NoEvent 期间为 true）
    */
    bool m_bMuteEvent;

    /** 当前悬停的条目索引（-1 表示无）
    */
    int32_t m_nHoveredIndex;

    /** 当前按下的条目索引（-1 表示无）
    */
    int32_t m_nPressedIndex;

    /** 最近一次绘制得到的布局（用于命中检测）
    */
    std::vector<LayoutEntry> m_layout;
};

} // namespace ui

#endif // UI_CONTROL_BREADCRUMB_H_
