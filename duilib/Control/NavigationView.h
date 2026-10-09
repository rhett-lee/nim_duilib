#ifndef UI_CONTROL_NAVIGATION_VIEW_H_
#define UI_CONTROL_NAVIGATION_VIEW_H_

#include "duilib/Box/HBox.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Label.h"
#include <vector>

namespace ui
{

class NavigationView;
class TabBox;
class VBox;
class VScrollBox;
class HBox;
class Line;

/** 侧边栏导航项控件（NavigationViewItem）
*
*   作为 NavigationView 的直接子节点使用，自绘"图标 + 文字"行，
*   支持三种形态：
*     item（默认）：可点击导航项，选中时灰底 + 强调色文字
*     header      ：分组标题（小号、弱化色，不可点击）
*     separator   ：分隔线
*
*   支持键盘导航：item 形态可通过 Tab 聚焦，上/下方向键在可选项间移动焦点，
*   回车/空格选中当前项；禁用态（SetEnabled(false)）不响应点击与键盘。
*
*   XML 属性：
*     text / text_id  项文字（text_id 支持多语言自动切换）
*     icon            图标图片属性串，如 file='public/nav/home.svg' width='16' height='16'
*     page            关联的内容页控件 name（内容页是 NavigationView 的非导航项直接子节点）
*     item_type       item（默认）/ header / separator
*     样式（均可选，默认值等价内置外观）：
*       icon_size / icon_left / text_left / text_right        图标尺寸与左右边距（DIP）
*       pill_radius / pill_inset_x / pill_inset_y             选中/悬停热区圆角与内缩（DIP）
*       selected_bg_color / hovered_bg_color                  选中/悬停背景色名
*       selected_text_color / normal_text_color               选中/普通文字色名
*       disabled_text_color / header_text_color / separator_color  禁用文字/分组标题/分隔线颜色名
*/
class DUILIB_API NavigationViewItem : public Control
{
    typedef Control BaseClass;
public:
    /** 导航项形态
    */
    enum class ItemType : uint8_t
    {
        kItem       = 0,    //可点击导航项
        kHeader     = 1,    //分组标题
        kSeparator  = 2     //分隔线
    };

public:
    explicit NavigationViewItem(Window* pWindow);
    NavigationViewItem(const NavigationViewItem& r) = delete;
    NavigationViewItem& operator=(const NavigationViewItem& r) = delete;
    virtual ~NavigationViewItem() override = default;

    virtual DString GetType() const override;
    virtual void OnInit() override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual bool MouseEnter(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual void OnLanguageChanged(bool bRedraw) override;
    virtual uint32_t GetControlFlags() const override;
    virtual void HandleEvent(const EventArgs& msg) override;

public:
    /** 设置项文字
    */
    void SetItemText(const DString& strText);
    DString GetItemText() const;

    /** 设置项文字 ID（多语言）
    */
    void SetItemTextId(const DString& strTextId);
    DString GetItemTextId() const;

    /** 获取当前应显示的文字（text_id 优先解析为当前语言文本）
    */
    DString GetDisplayText() const;

    /** 设置图标（图片属性串）
    */
    void SetIconAttr(const DString& strIcon);
    DString GetIconAttr() const;

    /** 设置关联内容页 name
    */
    void SetPageName(const DString& strPage);
    DString GetPageName() const;

    /** 设置形态
    */
    void SetItemType(ItemType itemType);
    ItemType GetItemType() const { return m_itemType; }

    /** 是否可点击选择（仅 item 形态）
    */
    bool IsSelectable() const { return m_itemType == ItemType::kItem; }

    /** 设置选中状态（由 NavigationView 统一管理）
    */
    void SetSelected(bool bSelected);
    bool IsSelected() const { return m_bSelected; }

    /** 设置紧凑模式（true=仅显示图标居中，由 NavigationView 在展开/收缩时统一切换）
    */
    void SetCompact(bool bCompact);
    bool IsCompact() const { return m_bCompact; }

    /** 设置所属 NavigationView（加入导航窗格时由其设置）
    */
    void SetOwnerView(NavigationView* pOwner) { m_pOwner = pOwner; }

private:
    /** 绘制三种形态
    */
    void PaintItem(IRender* pRender);
    void PaintHeader(IRender* pRender);
    void PaintSeparator(IRender* pRender);

private:
    /** 项文字
    */
    UiString m_text;

    /** 项文字 ID（多语言）
    */
    UiString m_textId;

    /** 图标图片属性串
    */
    UiString m_iconAttr;

    /** 关联内容页 name
    */
    UiString m_pageName;

    /** 形态
    */
    ItemType m_itemType;

    /** 是否选中
    */
    bool m_bSelected;

    /** 鼠标是否悬停
    */
    bool m_bHovered;

    /** 是否紧凑模式
    */
    bool m_bCompact;

    /** 所属 NavigationView
    */
    NavigationView* m_pOwner;

    //--- 可定制样式（默认值等价于内置外观，可用 XML 属性覆盖）---

    /** 图标尺寸（DIP，默认 16） */
    int32_t m_nIconSize;

    /** 展开态图标左边距（DIP，默认 16） */
    int32_t m_nIconLeft;

    /** 展开态文字左边距（DIP，默认 44） */
    int32_t m_nTextLeft;

    /** 展开态文字右边距（DIP，默认 8） */
    int32_t m_nTextRight;

    /** 选中/悬停热区圆角半径（DIP，默认 6） */
    int32_t m_nPillRadius;

    /** 热区水平内缩（DIP，默认 6） */
    int32_t m_nPillInsetX;

    /** 热区垂直内缩（DIP，默认 3） */
    int32_t m_nPillInsetY;

    /** 选中背景色名（默认 bg_list_item_selected） */
    UiString m_selectedBgColor;

    /** 悬停背景色名（默认 bg_list_item_hovered） */
    UiString m_hoveredBgColor;

    /** 选中文字色名（默认 color_accent） */
    UiString m_selectedTextColor;

    /** 普通文字色名（默认 text_default） */
    UiString m_normalTextColor;

    /** 禁用文字色名（默认 text_disabled） */
    UiString m_disabledTextColor;

    /** 分组标题文字色名（默认 text_muted） */
    UiString m_headerTextColor;

    /** 分隔线颜色名（默认 border_window） */
    UiString m_separatorColor;    
};

/** 侧边栏导航控件（NavigationView）
*
*   内部组合"左侧导航窗格 + 右侧内容区（页头 + TabBox 页面）"：
*   窗格含汉堡按钮、窗格标题、可滚动导航项列表与底部可选设置项；
*   点击导航项联动切换内容页并更新页头；汉堡按钮在展开/紧凑两态间切换。
*
*   XML 用法（推荐：内容页显式放入一个 TabBox，便于外部操控页面容器）：
*   <NavigationView pane_width="220" compact_pane_width="48" pane_title_id="..."
*                   show_header="true" settings_item="true" settings_page="page_settings"
*                   toggle_icon="file='nav/hamburger.svg' ..." selected_id="page_home">
*     <NavigationViewItem text_id="..." icon="..." page="page_home"/>
*     <NavigationViewItem item_type="header" text_id="..."/>
*     <NavigationViewItem item_type="separator"/>
*     <TabBox name="nav_content">
*       <VBox name="page_home"> ... </VBox>
*       <VBox name="page_settings"> ... </VBox>
*     </TabBox>
*   </NavigationView>
*
*   直接子节点的分流规则（重写 AddItem 实现）：
*     NavigationViewItem → 导航项列表（header/separator 同样进入列表，仅渲染形态不同）
*     TabBox             → 作为内容区页面容器接管（仅允许一个；可自行配置切换动画等属性）
*     其他控件           → 作为页面进入内容区；若未显式提供 TabBox，则自动创建一个
*
*   序号约定：序号仅统计"可选中导航项"（item 形态），header/separator 与底部设置项均不计入；
*             设置项固定显示在窗格底部，不随导航列表滚动，也不参与序号统计。
*
*   XML 属性：
*     pane_width           展开态窗格宽度（像素，默认 220，DPI 自适应）
*     compact_pane_width   紧凑态窗格宽度（像素，默认 48，DPI 自适应）
*     pane_title           窗格标题文字
*     pane_title_id        窗格标题文字 ID（多语言）
*     toggle_icon          汉堡按钮图标图片属性串（为空则按钮不显示图标，可用文字样式自行定制）
*     collapsed            初始是否紧凑态（true/false，默认 false）
*     show_header          是否显示内容区页头（true/false，默认 true）
*     settings_item        是否在窗格底部显示设置项（true/false，默认 false）
*     settings_page        设置项关联的内容页 name（默认 settings）
*     settings_text_id     设置项文字 ID（多语言，默认空，由使用方提供）
*     settings_icon        设置项图标图片属性串（默认空，由使用方提供）
*     selected_id          初始选中项对应的 page name（默认第一个可选项）
*
*   事件（均由 NavigationView 自身作为发送者触发，与内部 TabBox 的 kEventTabSelect 互不干扰）：
*     kEventNavigationItemClick         点击可选择导航项时触发（重复点击已选中项也会触发），
*                                       WPARAM 为该项在所有可选项中的序号；适合"点击即刷新/回顶"类需求
*     kEventNavigationSelectionChanged  选中项发生变化时触发，WPARAM 为新序号，LPARAM 为旧序号
*                                       （无选中时为 Box::InvalidIndex）
*     kEventNavigationPaneToggling      窗格即将收起/展开时触发，WPARAM 为目标状态（1=收起，0=展开），
*                                       回调返回 false 可取消本次切换
*     kEventNavigationPaneToggled       窗格收起/展开完成时触发，WPARAM 为当前状态（1=收起，0=展开）
*/
class DUILIB_API NavigationView : public HBox
{
    typedef HBox BaseClass;
public:
    explicit NavigationView(Window* pWindow);
    NavigationView(const NavigationView& r) = delete;
    NavigationView& operator=(const NavigationView& r) = delete;
    virtual ~NavigationView() override;

    virtual DString GetType() const override;
    virtual void OnInit() override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual bool AddItem(Control* pControl) override;
    virtual void OnLanguageChanged(bool bRedraw) override;

public:
    /** 选中指定导航项
    * @param [in] pItem 导航项指针（必须属于本控件且可选择）
    * @param [in] bFireEvent 是否触发 kEventTabSelect 事件
    * @return true 选择成功
    */
    bool SelectItem(NavigationViewItem* pItem, bool bFireEvent = true);

    /** 按关联内容页 name 选中导航项
    */
    bool SelectItem(const DString& strPageName);

    /** 按可选项序号选中（序号仅统计 item 形态，不含 header/separator）
    */
    bool SelectItemByIndex(size_t nIndex);

    /** 获取当前选中项指针（无选中返回 nullptr）
    */
    NavigationViewItem* GetSelectedItem() const;

    /** 获取当前选中项在所有可选项中的序号（无选中返回 Box::InvalidIndex）
    */
    size_t GetSelectedIndex() const;

    /** 获取当前选中项关联的内容页 name
    */
    DString GetSelectedPage() const;

    /** 展开/收起导航窗格
    */
    void SetCollapsed(bool bCollapsed);
    bool IsCollapsed() const { return m_bCollapsed; }

    /** 监听导航项点击事件（kEventNavigationItemClick）
    *   与选中变化事件的区别：重复点击当前已选中项时本事件仍会触发，WPARAM 为可选项序号
    */
    void AttachNavItemClick(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventNavigationItemClick, callback, callbackID);
    }

    /** 监听导航选中变化事件（kEventNavigationSelectionChanged）
    *   WPARAM 为新序号，LPARAM 为旧序号（无选中时为 Box::InvalidIndex）
    */
    void AttachNavSelectionChanged(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventNavigationSelectionChanged, callback, callbackID);
    }

    /** 监听窗格即将收起/展开事件（kEventNavigationPaneToggling，可取消）
    *   WPARAM 为目标状态（1=收起，0=展开），回调返回 false 可阻止本次切换
    */
    void AttachNavPaneToggling(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventNavigationPaneToggling, callback, callbackID);
    }

    /** 监听窗格收起/展开完成事件（kEventNavigationPaneToggled）
    *   WPARAM 为当前状态（1=收起，0=展开）
    */
    void AttachNavPaneToggled(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventNavigationPaneToggled, callback, callbackID);
    }

    /** 动态追加一个导航项（内容页需已存在于内容区）
    * @param [in] strTextId 项文字 ID（多语言），传空则用 strText
    * @param [in] strIcon 图标图片属性串
    * @param [in] strPage 关联内容页 name
    * @return 新建的导航项指针（仍由控件内部管理，外部无需释放）
    */
    NavigationViewItem* AddNavItem(const DString& strTextId, const DString& strIcon, const DString& strPage);

    /** 移除一个导航项（不移除其关联的内容页）
    * @param [in] pItem 导航项指针（必须属于本控件）
    * @return true 移除成功
    */
    bool RemoveNavItem(NavigationViewItem* pItem);

    /** 移除所有导航项（分组标题/分隔线/设置项一并移除；不移除内容页）
    */
    void RemoveAllNavItems();

    /** 设置/获取导航项被禁用能力（false 时不可点击选中）
    */
    void SetItemEnabled(NavigationViewItem* pItem, bool bEnabled);

    /** 导航项被点击时由 NavigationViewItem 回调
    */
    void OnItemClicked(NavigationViewItem* pItem);

    /** 键盘导航：在可选中项之间移动焦点（bForward=true 向下，false 向上）
    */
    void MoveFocusByKey(NavigationViewItem* pFrom, bool bForward);

private:
    /** 惰性创建内部子结构（窗格、分隔线、右侧页头），必须在首个用户子节点分流前完成
    */
    void EnsureInternals();

    /** 确保内容区 TabBox 存在：未显式提供时惰性自动创建
    */
    void EnsureContentHost();

    /** 注册导航项：设置 owner、紧凑态、加入列表
    */
    void RegisterNavItem(NavigationViewItem* pItem);

    /** 注销导航项：从列表移除（不销毁控件）
    */
    void UnregisterNavItem(NavigationViewItem* pItem);

    /** 判断是否为参与序号统计的"可选中导航项"（排除设置项、header、separator）
    */
    bool IsIndexedItem(const NavigationViewItem* pItem) const;

    /** 按内容页 name 查找可选项
    */
    NavigationViewItem* FindItemByPage(const DString& strPageName) const;

    /** 应用当前展开/紧凑状态（窗格宽度、标题可见性、各导航项紧凑标志）
    */
    void ApplyCollapsed();

    /** 应用页头可见性
    */
    void ApplyHeaderVisible();

    /** 用选中项文字同步页头
    */
    void UpdateHeaderText(NavigationViewItem* pItem);

    /** 完成初始选中（OnInit 时调用）
    */
    void DoInitialSelection();

    /** 汉堡按钮点击
    */
    void OnToggleClicked();

private:
    /** 内部结构是否已创建
    */
    bool m_bInternalsReady;

    /** 展开态/紧凑态窗格宽度（未缩放像素值）
    */
    int32_t m_nPaneWidth;
    int32_t m_nCompactPaneWidth;

    /** 是否紧凑态
    */
    bool m_bCollapsed;

    /** 是否显示内容区页头
    */
    bool m_bShowHeader;

    /** 是否显示底部设置项
    */
    bool m_bSettingsItem;

    /** 设置项关联页 name / 文字 ID / 图标
    */
    UiString m_settingsPage;
    UiString m_settingsTextId;
    UiString m_settingsIcon;

    /** 窗格标题文字 / 文字 ID
    */
    UiString m_paneTitle;
    UiString m_paneTitleId;

    /** 汉堡按钮图标图片属性串
    */
    UiString m_toggleIcon;

    /** 初始选中项 page name
    */
    UiString m_initSelectedId;

    /** 内部控件
    */
    VBox* m_pPane;             ///< 左侧导航窗格（垂直容器）
    HBox* m_pTopBar;           ///< 窗格顶部：汉堡按钮 + 窗格标题
    Button* m_pToggleBtn;      ///< 汉堡按钮
    Label* m_pPaneTitleLabel;  ///< 窗格标题
    VScrollBox* m_pItemHost;   ///< 导航项可滚动列表（占据窗格剩余空间，stretch）
    VBox* m_pBottomHost;       ///< 窗格底部固定区（设置项所在，不随列表滚动）
    NavigationViewItem* m_pSettingsNavItem; ///< 底部设置项
    Line* m_pSeparator;        ///< 窗格与内容区间竖分隔线
    VBox* m_pRight;            ///< 右侧：页头 + 内容
    Label* m_pHeader;          ///< 内容区页头
    TabBox* m_pContent;        ///< 内容区页面容器

    /** 所有导航项（含 header/separator，按加入顺序）
    */
    std::vector<NavigationViewItem*> m_items;

    /** 当前选中项
    */
    NavigationViewItem* m_pSelected;
};

} // namespace ui

#endif // UI_CONTROL_NAVIGATION_VIEW_H_
