#ifndef UI_CONTROL_TABCTRL_H_
#define UI_CONTROL_TABCTRL_H_

#include "duilib/Box/ListBox.h"
#include "duilib/Core/ControlDragable.h"
#include "duilib/Core/Callback.h"
#include "duilib/Control/IconControl.h"
#include "duilib/Control/Badge.h"
#include <memory>
#include <cstdint>

namespace ui
{
/** TabCtrl 的布局方向
*/
enum class TabCtrlOrientation
{
    kHorizontal = 0,  //水平方向（默认值）
    kVertical        //垂直方向
};

/** TabCtrl 底边线的断开策略
*/
enum class TabCtrlBottomLineMode
{
    kAuto = 0,   //自动：选中项绘制背景色时断开（默认值）
    kBreak,      //始终在选中项区域断开
    kFull        //始终绘制连续的整条边线
};

/** TabCtrlItem 关闭按钮的显示策略
*/
enum class TabCtrlCloseButtonShowMode
{
    kAuto = 0,   //自动：沿用 auto_hide_close_button 的原有逻辑（默认值）
    kHover,      //仅鼠标悬停时显示（选中时也不常显）
    kAlways      //始终显示
};

/** 多标签控件（类似浏览器的多标签）
*/
class TabBox;
class TabCtrlItem;
class IBrush;
class IPath;
class IPen;
class DUILIB_API TabCtrl: public ListBox
{
    typedef ListBox BaseClass;
    friend class TabCtrlItem;
    friend class TabBox;
public:
    explicit TabCtrl(Window* pWindow);
    virtual ~TabCtrl() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

public:
    /** 设置绑定的TabBox控件名称
    */
    void SetTabBoxName(const DString& tabBoxName);

    /** 获取绑定的TabBox控件名称
    */
    DString GetTabBoxName() const;

    /** 设置绑定的TabBox接口
    */
    void SetTabBox(TabBox* pTabBox);

    /** 获取绑定的TabBox接口
    */
    TabBox* GetTabBox() const;

    /** 设置是否支持拖动改变控件的顺序
    */
    void SetEnableDragOrder(bool bEnable);

    /** 判断是否支持拖动改变控件的顺序
    */
    bool IsEnableDragOrder() const;

    /** 处理分割线的显示或者隐藏
    */
    void AdjustItemLineStatus();

    /** 设置标签栏的布局方向（水平 / 垂直）
    * @param [in] orientation 布局方向，默认水平
    */
    void SetTabCtrlOrientation(TabCtrlOrientation orientation);

    /** 获取标签栏的布局方向
    */
    TabCtrlOrientation GetTabCtrlOrientation() const;

    /** 设置是否启用溢出滚动（标签超出可视区域时显示滚动条）
    * @param [in] bEnable true表示启用，默认启用
    */
    void SetEnableOverflowScroll(bool bEnable);

    /** 判断是否启用溢出滚动
    */
    bool IsEnableOverflowScroll() const;

    /** 获取固定标签的数量（固定标签恒位于标签栏前端，构成连续的固定块）
    * @return 返回固定标签的数量
    */
    size_t GetFixedItemCount() const;

public:
    /** 监听标签关闭请求事件（点击关闭按钮，或者鼠标中键点击标签时触发）
    * 注意：TabCtrl 不会自动删除标签，需要在事件响应中自行删除
    * @param [in] callback 事件处理的回调函数，wParam 为标签索引
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachTabClose(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventTabClose, callback, callbackID); }

    /** 监听"新建标签"按钮的点击事件
    * @param [in] callback 事件处理的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachTabNew(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventTabNew, callback, callbackID); }

    /** 设置是否启用鼠标滚轮切换标签
    * @param [in] bEnable true表示滚轮切换标签，false表示滚轮仍用于滚动标签栏（默认值）
    */
    void SetEnableWheelSwitch(bool bEnable);

    /** 判断是否启用鼠标滚轮切换标签
    */
    bool IsEnableWheelSwitch() const;

    /** 设置是否启用鼠标中键关闭标签（点击后仅发送 kEventTabClose 事件，不删除标签）
    * @param [in] bEnable true表示启用（默认值），false表示禁用
    */
    void SetEnableMiddleClickClose(bool bEnable);

    /** 判断是否启用鼠标中键关闭标签
    */
    bool IsEnableMiddleClickClose() const;

    /** 设置拖动排序时插入位置提示线的颜色
    * @param [in] insertColor 提示线颜色，为空时不绘制
    */
    void SetDragInsertColor(const DString& insertColor);

    /** 获取拖动排序时插入位置提示线的颜色
    */
    const DString& GetDragInsertColor() const;

    /** 设置标签溢出时是否显示左右（上下）滚动箭头按钮
    * 启用时隐藏悬浮滚动条（两者互斥显示），点击箭头按可视范围翻页滚动
    * @param [in] bEnable true表示显示（默认值），false表示不显示
    */
    void SetEnableOverflowButtons(bool bEnable);

    /** 判断标签溢出时是否显示滚动箭头按钮
    */
    bool IsEnableOverflowButtons() const;

    /** 设置标签是否等宽填充（按标签栏可用宽度平分，垂直方向按高度平分）
    * @param [in] bJustify true表示等宽填充，false表示按标签内容自适应宽度（默认值）
    */
    void SetTabItemJustify(bool bJustify);

    /** 判断标签是否等宽填充
    */
    bool IsTabItemJustify() const;

    /** 设置是否在标签栏末尾显示"新建标签"按钮
    * @param [in] bEnable true表示显示，false表示不显示（默认值）
    */
    void SetEnableNewTabButton(bool bEnable);

    /** 判断是否显示"新建标签"按钮
    */
    bool IsEnableNewTabButton() const;

public:
    /** 获取当前选中的标签项
    * @return 返回选中的标签项指针，未选中时返回 nullptr
    */
    TabCtrlItem* GetSelectedTabItem() const;

    /** 设置当前选中的标签项
    * @param [in] pItem 要选中的标签项指针
    * @param [in] bTriggerEvent 是否触发选择事件，默认触发
    * @return 成功返回 true，否则返回 false
    */
    bool SetSelectedTabItem(TabCtrlItem* pItem, bool bTriggerEvent = true);

    /** 获取指定索引处的标签项
    * @param [in] iIndex 标签项索引，范围是：[0, GetItemCount())
    * @return 返回标签项指针，索引无效或不是标签项时返回 nullptr
    */
    TabCtrlItem* GetTabItemAt(size_t iIndex) const;

    /** 追加一个标签项到末尾（如果是固定标签，则仍会归位到固定标签区域）
    * @param [in] pItem 标签项指针
    * @return 成功返回 true，否则返回 false
    */
    bool AddTabItemToEnd(Control* pItem);

    /** 选择下一个标签（循环选择）
    * @param [in] bTakeFocus 是否让控件获取焦点，默认获取
    * @param [in] bTriggerEvent 是否触发选择事件，默认触发
    * @return 选择成功返回 true
    */
    bool SelectNextTab(bool bTakeFocus = true, bool bTriggerEvent = true);

    /** 选择上一个标签（循环选择）
    * @param [in] bTakeFocus 是否让控件获取焦点，默认获取
    * @param [in] bTriggerEvent 是否触发选择事件，默认触发
    * @return 选择成功返回 true
    */
    bool SelectPrevTab(bool bTakeFocus = true, bool bTriggerEvent = true);

public:
    /** 设置选择标签项的外部轮廓边线的宽度
    * @param [in] fOutlineWidth 外部轮廓边线的宽度，宽度值未进行DPI缩放
    */
    void SetSelectedTabItemOutlineWidth(float fOutlineWidth);

    /** 获取选择标签项的外部轮廓边线的宽度
    * @return 返回外部轮廓边线的宽度，宽度值未进行DPI缩放
    */
    float GetSelectedTabItemOutlineWidth() const;

    /** 设置选择标签项的外部轮廓边线的颜色
    * @param [in] outlineColor 轮廓边线的颜色
    */
    void SetSelectedTabItemOutlineColor(const DString& outlineColor);

    /** 获取选择标签项的外部轮廓边线的颜色
    */
    const DString& GetSelectedTabItemOutlineColor() const;

    /** 设置标签栏底部的边线高度（这个边线，选择标签的区域不绘制，其他区域绘制）
    * @param [in] fLineHeight 标签栏底部的边线高度, 高度值未进行DPI缩放
    */
    void SetTabCtrlBottomLineHeight(float fLineHeight);

    /** 获取标签栏底部的边线高度
    * @return 标签栏底部的边线高度, 高度值未进行DPI缩放
    */
    float GetTabCtrlBottomLineHeight() const;

    /** 设置标签栏底部的边线颜色(该底部边线，不包括选择标签项的区域)
    * @param [in] lineColor 标签栏底部的边线颜色
    */
    void SetTabCtrlBottomLineColor(const DString& lineColor);

    /** 设置标签栏底部的边线颜色
    */
    const DString& GetTabCtrlBottomLineColor() const;

    /** 设置选中标签的指示条宽度（用于"底部指示条"样式的选中效果）
    * @param [in] fIndicatorWidth 指示条宽度，宽度值未进行DPI缩放；小于等于0时表示不绘制指示条（默认值）
    */
    void SetSelectedTabItemIndicatorWidth(float fIndicatorWidth);

    /** 获取选中标签的指示条宽度
    * @return 返回指示条的宽度，宽度值未进行DPI缩放
    */
    float GetSelectedTabItemIndicatorWidth() const;

    /** 设置选中标签的指示条宽度是否自动跟随标签标题文字宽度
    * @param [in] bAuto true表示自动跟随标题文字宽度（此时忽略指示条宽度值），默认false
    */
    void SetSelectedTabItemIndicatorAutoWidth(bool bAuto);

    /** 选中标签的指示条宽度是否自动跟随标签标题文字宽度
    */
    bool IsSelectedTabItemIndicatorAutoWidth() const;

    /** 设置选中标签的指示条高度
    * @param [in] fIndicatorHeight 指示条高度，高度值未进行DPI缩放
    */
    void SetSelectedTabItemIndicatorHeight(float fIndicatorHeight);

    /** 获取选中标签的指示条高度
    * @return 返回指示条的高度，高度值未进行DPI缩放
    */
    float GetSelectedTabItemIndicatorHeight() const;

    /** 设置选中标签的指示条颜色（为空时不绘制指示条）
    * @param [in] indicatorColor 指示条颜色
    */
    void SetSelectedTabItemIndicatorColor(const DString& indicatorColor);

    /** 获取选中标签的指示条颜色
    */
    const DString& GetSelectedTabItemIndicatorColor() const;

    /** 设置是否绘制选中标签的背景色
    * @param [in] bShow true表示绘制选中标签的背景色（默认值），false表示不绘制背景色（可配合指示条使用）
    */
    void SetSelectedTabItemShowBg(bool bShow);

    /** 是否绘制选中标签的背景色
    */
    bool IsSelectedTabItemShowBg() const;

    /** 设置选中标签的指示条是否使用位置/宽度插值动画
    * @param [in] bEnable true表示启用动画（默认值），false表示瞬间切换
    */
    void SetEnableIndicatorAnimation(bool bEnable);

    /** 判断选中标签的指示条是否使用插值动画
    */
    bool IsEnableIndicatorAnimation() const;

    /** 设置指示条动画的时长
    * @param [in] nDurationMs 动画时长（毫秒），小于等于0时视为关闭动画
    */
    void SetIndicatorAnimationDuration(uint32_t nDurationMs);

    /** 获取指示条动画的时长（毫秒）
    */
    uint32_t GetIndicatorAnimationDuration() const;

    /** 设置标签栏底边线的断开策略
    * @param [in] mode kAuto：选中项绘制背景色时断开（默认值）；kBreak：始终断开；kFull：始终连续
    */
    void SetTabCtrlBottomLineMode(TabCtrlBottomLineMode mode);

    /** 获取标签栏底边线的断开策略
    */
    TabCtrlBottomLineMode GetTabCtrlBottomLineMode() const;

public:
    /** 设置子项的位置索引
     * @param [in] pControl 子项指针
     * @param [in] iIndex 索引号，范围是：[0, GetItemCount())
     */
    virtual bool SetItemIndex(Control* pControl, size_t iIndex) override;

    /** 追加一个子项到末尾
     * @param [in] pControl 子项指针
     */
    virtual bool AddItem(Control* pControl) override;

    /** 在指定位置之后插入一个子项
     * @param [in] pControl 子项指针
     * @param[in] iIndex 要插入的位置索引，范围是：[0, GetItemCount())
     */
    virtual bool AddItemAt(Control* pControl, size_t  iIndex) override;

    /** 根据子项指针
     * @param [in] pControl 子项指针
     */
    virtual bool RemoveItem(Control* pControl) override;

    /** 根据索引移除一个子项
     * @param [in] iIndex 子项索引，范围是：[0, GetItemCount())
     */
    virtual bool RemoveItemAt(size_t iIndex) override;

    /** 设置控件位置（等宽填充模式下，在基类布局完成后重新分配各标签项的尺寸）
    */
    virtual void SetPos(UiRect rc) override;

protected:
    /** 初始化接口
    */
    virtual void OnInit() override;

    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 鼠标左键按下（仅处理标签栏空白区域与溢出箭头按钮）
    */
    virtual bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标进入（溢出箭头模式下保持滚动条隐藏，避免淡入）
    */
    virtual bool MouseEnter(const EventArgs& msg) override;

    /** 键盘消息处理：支持 Ctrl+Tab / Ctrl+Shift+Tab / Ctrl+1..9
    */
    virtual bool OnListBoxKeyDown(const EventArgs& msg) override;

    /** 绘制控件状态颜色的函数
    */
    virtual void PaintStateColors(IRender* pRender) override;

    /** 绘制前景色（在子控件绘制完成后调用，用于绘制选中标签的指示条）
    */
    virtual void PaintForeColor(IRender* pRender) override;

    /** 绘制焦点框（TabCtrl 的焦点框绘制在选中标签上，避免整条标签栏外框）
    */
    virtual void PaintFocusRect(IRender* pRender) override;

    /** 子项的选择状态变化事件（用于启动指示条动画）
    */
    virtual void OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem) override;

    /** 主题发生变化，清除缓存的绘制资源
    */
    virtual void OnThemeChanged(bool bRedraw) override;

    /** DPI发生变化，清除缓存的文字测量结果
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

private:
    /** 清除缓存的绘制资源（颜色解析结果、画笔、文字测量结果等）
    */
    void ClearPaintCache();

    /** 按当前布局方向确保布局对象正确（已匹配时不重建）
    */
    void EnsureLayout();

    /** 按当前配置启用/禁用滚动条（滚动位置依赖滚动条对象，因此箭头模式下仍需保留）
    */
    void UpdateScrollBarConfig();

    /** 更新滚动条的可见性与鼠标响应（溢出箭头模式下隐藏滚动条，避免与箭头叠加）
    */
    void UpdateScrollBarVisibility();

    /** 绘制选中标签的指示条（仅在配置了指示条宽度、高度和颜色时绘制）
    */
    void PaintSelectedTabIndicator(IRender* pRender);

    /** 计算自动宽度模式下指示条的长度
    * @param [in] pRender 绘制接口
    * @param [in] pSelectedItem 当前选中的标签项
    * @param [in] rcItem 标签项（或标题文字）的矩形
    * @return 返回指示条的长度（像素）
    */
    int32_t CalcAutoIndicatorLength(IRender* pRender, Control* pSelectedItem, const UiRect& rcItem);

    /** 计算选中标签指示条的目标矩形（未进行动画插值）
    * @param [in] pRender 绘制接口
    * @param [out] rcIndicator 返回指示条矩形
    * @return 指示条可绘制时返回 true
    */
    bool CalcSelectedIndicatorRect(IRender* pRender, UiRect& rcIndicator);

    /** 启动指示条动画（从当前绘制位置过渡到目标位置）
    */
    void StartIndicatorAnimation(const UiRectF& rcFrom, const UiRectF& rcTo);

    /** 重置指示条动画状态（停止定时器并清空缓存）
    */
    void ResetIndicatorAnimation();

    /** 指示条动画的定时器回调（仅触发重绘）
    */
    void OnIndicatorAnimationTimer();

    /** 判断当前焦点是否位于本控件或其子控件内
    */
    bool IsFocusWithin() const;

    /** 在选中标签上绘制焦点框
    */
    void PaintTabFocusRect(IRender* pRender);

    /** 固定标签归位：把固定标签稳定地移到标签栏前端（保持各自原有相对顺序）
    */
    void NormalizeFixedItems();

    /** 把当前选中的标签同步到绑定的TabBox
    */
    void SyncTabBoxSelection();

    /** TabBox的选择项变化时，同步到TabCtrl（避免与 SyncTabBoxSelection 互相触发）
    * @param [in] nTabBoxIndex TabBox当前选择的子项索引
    */
    void OnBoundTabBoxSelectChanged(size_t nTabBoxIndex);

    /** 根据TabBox子项索引查找对应的标签项
    * @param [in] nTabBoxIndex TabBox的子项索引
    * @return 返回标签项指针，未找到时返回 nullptr
    */
    TabCtrlItem* FindTabItemByTabBoxIndex(size_t nTabBoxIndex) const;

    /** 发送标签关闭请求事件（不删除标签）
    * @param [in] pItem 请求关闭的标签项
    */
    void NotifyTabCloseRequest(TabCtrlItem* pItem);

    /** 处理标签栏上的点击（溢出箭头按钮、新建标签按钮）
    * @param [in] pt 鼠标坐标（窗口坐标）
    * @return 已处理时返回 true
    */
    bool HandleTabStripClick(const UiPoint& pt);

    /** 更新所有标签项的布局（垂直方向铺满宽度，并更新分隔线方向）
    */
    void UpdateItemsLayoutForOrientation();

    /** 等宽填充模式下的布局
    */
    void LayoutJustify();

    /** 绘制溢出滚动箭头按钮（仅当标签溢出时显示）
    */
    void PaintOverflowButtons(IRender* pRender);

    /** 绘制"新建标签"按钮
    */
    void PaintNewTabButton(IRender* pRender);

    /** 绘制拖动排序时的插入位置提示线
    */
    void PaintDragInsertIndicator(IRender* pRender);

    /** 计算溢出滚动箭头按钮的矩形
    * @param [out] rcPrev 返回"向前"按钮的矩形
    * @param [out] rcNext 返回"向后"按钮的矩形
    * @return 需要显示箭头按钮时返回 true
    */
    bool CalcOverflowButtonRects(UiRect& rcPrev, UiRect& rcNext) const;

    /** 计算"新建标签"按钮的矩形
    * @param [out] rcButton 返回按钮矩形
    * @return 需要显示"新建标签"按钮时返回 true
    */
    bool CalcNewTabButtonRect(UiRect& rcButton) const;

    /** 判断标签是否处于溢出状态
    */
    bool IsTabOverflow() const;

    /** 默认选择的子项索引（仅初始化时生效）
    */
    size_t m_nDefaultSelectedIndex;

    /** 绑定的TabBox名称
    */
    UiString m_tabBoxName;

    /** 绑定的TabBox接口
    */
    TabBox* m_pTabBox;

    /** 是否支持拖动改变列的顺序(功能开关)
    */
    bool m_bEnableDragOrder;

    /** 标签栏的布局方向（水平/垂直）
    */
    TabCtrlOrientation m_orientation;

    /** 是否启用溢出滚动（标签超出可视区域时显示滚动条）
    */
    bool m_bEnableOverflowScroll;

    /** 是否正在执行 TabBox 与 TabCtrl 之间的选择同步（防止互相触发形成死循环）
    */
    bool m_bSyncingTabBox;

    /** 是否启用鼠标滚轮切换标签
    */
    bool m_bEnableWheelSwitch;

    /** 是否启用鼠标中键关闭标签
    */
    bool m_bEnableMiddleClickClose;

    /** 标签溢出时是否显示滚动箭头按钮
    */
    bool m_bEnableOverflowButtons;

    /** 标签是否等宽填充
    */
    bool m_bTabItemJustify;

    /** 是否显示"新建标签"按钮
    */
    bool m_bEnableNewTabButton;

    /** 拖动排序时插入位置提示线的颜色
    */
    DString m_dragInsertColor;

    /** 选择标签项的外部轮廓边线的宽度，宽度值未进行DPI缩放
    */
    float m_fSelectedTabItemOutlineWidth;

    /** 选择标签项的外部轮廓边线的颜色
    */
    DString m_selectedTabItemOutlineColor;

    /** 标签栏底部的边线高度, 高度值未进行DPI缩放（这个边线，选择标签的区域不绘制，其他区域绘制）
    */
    float m_fTabCtrlBottomLineHeight;

    /** 设置标签栏底部的边线颜色
    */
    DString m_tabCtrlBottomLineColor;

    /** 选中标签的指示条宽度，宽度值未进行DPI缩放（小于等于0时不绘制）
    */
    float m_fSelectedTabItemIndicatorWidth;

    /** 选中标签的指示条宽度是否自动跟随标签标题文字宽度
    */
    bool m_bSelectedTabItemIndicatorAutoWidth;

    /** 选中标签的指示条高度，高度值未进行DPI缩放
    */
    float m_fSelectedTabItemIndicatorHeight;

    /** 选中标签的指示条颜色（为空时不绘制）
    */
    DString m_selectedTabItemIndicatorColor;

    /** 是否绘制选中标签的背景色
    */
    bool m_bSelectedTabItemShowBg;

    /** 是否启用选中标签指示条的滑动动画
    */
    bool m_bEnableIndicatorAnimation;

    /** 指示条动画时长（毫秒）
    */
    uint32_t m_nIndicatorAnimationDuration;

    /** 标签栏底边线的断开策略
    */
    TabCtrlBottomLineMode m_bottomLineMode;

    /** 是否正在播放指示条动画
    */
    bool m_bIndicatorAnimating;

    /** 下一次绘制是否需要启动指示条动画（选择变化时置位）
    */
    bool m_bIndicatorAnimateNext;

    /** 上一次绘制的指示条矩形是否有效
    */
    bool m_bIndicatorLastValid;

    /** 上一次绘制的指示条矩形（动画起点/重定向基准）
    */
    UiRectF m_rcIndicatorLast;

    /** 动画起始指示条矩形
    */
    UiRectF m_rcIndicatorFrom;

    /** 动画目标指示条矩形
    */
    UiRectF m_rcIndicatorTo;

    /** 动画起始时间（steady_clock 毫秒）
    */
    uint64_t m_nIndicatorAnimStartMs;

    /** 指示条动画定时器的ID（0表示未启动）
    */
    size_t m_nIndicatorTimerId;

    /** 指示条动画定时器的生命周期标志
    */
    WeakCallbackFlag m_indicatorTimerFlag;

    /** 标签栏底部边线的画笔缓存（颜色或高度变化时重建）
    */
    std::unique_ptr<IPen> m_spBottomLinePen;

    /** 底部边线画笔缓存对应的颜色字符串
    */
    UiString m_bottomLinePenColor;

    /** 底部边线画笔缓存对应的边线高度
    */
    float m_fBottomLinePenHeight;

    /** 指示条颜色缓存对应的颜色字符串与颜色值
    */
    UiString m_indicatorColorKey;
    UiColor m_indicatorColorValue;

    /** 指示条文字测量结果的缓存（文本或字体变化时失效）
    */
    UiString m_indicatorTextCache;
    UiString m_indicatorFontIdCache;
    int32_t m_indicatorTextLength;
    bool m_bIndicatorTextLengthValid;
};

/** 多标签控件的一个标签页
*/
class DUILIB_API TabCtrlItem : public ControlDragableT<ListBoxItemH>
{
    typedef ControlDragableT<ListBoxItemH> BaseClass;
public:
    explicit TabCtrlItem(Window* pWindow);
    virtual ~TabCtrlItem() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual DString GetToolTipText() const override;

public:
    /** 设置图标
    * @param [in] iconImageString 图标资源字符串
    */
    void SetIcon(const DString& iconImageString);

    /** 获取图标
    * @return 返回图标资源字符串
    */
    DString GetIcon() const;

    /** 设置图标的位图数据(数据格式：ARGB格式，Alpha类型为预乘（AlphaType::kPremul）)
    * @param [in] nWidth 宽度
    * @param [in] nHeight 高度
    * @param [in] pPixelBits 位图数据
    * @param [in] nPixelBitsSize 位图数据的长度（按字节）
    */
    bool SetIconData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize);

    /** 清除图标数据
    */
    void ClearIconData();

    /** 设置文字内容
    */
    void SetTitle(const DString& title);

    /** 获取文字内容
    */
    DString GetTitle() const;

    /** 设置文字内容ID（支持多语版）
    */
    void SetTitleId(const DString& titleId);

    /** 获取文字内容（支持多语版）
    */
    DString GetTitleId() const;

public:
    /** 设置绑定的TabBox子项索引号
    */
    void SetTabBoxItemIndex(size_t nTabBoxItemIndex);

    /** 获取绑定的TabBox子项ID索引号
    */
    size_t GetTabBoxItemIndex() const;

public:
    /** 获取图标控件
    */
    IconControl* GetIconControl() const { return m_pIcon; }

    /** 获取文本控件
    */
    Label* GetTextLabel() const { return m_pLabel; }

    /** 获取关闭按钮
    */
    Button* GetCloseButton() const { return m_pCloseBtn; }

    /** 获取标签之间的分割线的控件
    */
    Control* GetLineControl() const { return m_pLine; }

    /** 获取角标控件（未读数/小红点，未设置角标Class时返回nullptr）
    */
    Badge* GetBadgeControl() const { return m_pBadge; }

public:
    /** 设置角标数量：大于0时显示角标，小于等于0时自动隐藏
    */
    void SetBadgeCount(int64_t nCount);

    /** 获取角标数量
    */
    int64_t GetBadgeCount() const;

    /** 设置角标数量上限：超过上限时显示"上限+"（如"99+"），默认99
    */
    void SetBadgeMaxCount(int64_t nMaxCount);

    /** 获取角标数量上限
    */
    int64_t GetBadgeMaxCount() const;

    /** 设置角标是否为红点模式：true为纯小圆点，不显示数字
    */
    void SetBadgeDot(bool bDot);

    /** 角标是否为红点模式
    */
    bool IsBadgeDot() const;

    /** 角标控件的Class
    */
    void SetBadgeClass(const DString& badgeClass);
    DString GetBadgeClass() const;

    /** 设置角标是否为右上角浮动模式（不占用标题的布局宽度）
    * @param [in] bFloat true表示浮动到右上角，false表示按流式布局排列（默认值）
    */
    void SetBadgeFloat(bool bFloat);

    /** 判断角标是否为右上角浮动模式
    */
    bool IsBadgeFloat() const;

public:
    /** 图标控件的Class
    */
    void SetIconClass(const DString& iconClass);
    DString GetIconClass() const;

    /** 文本控件的Class
    */
    void SetTitleClass(const DString& titleClass);
    DString GetTitleClass() const;

    /** 设置标签标题文字在普通状态下的颜色（为空时保持原有颜色，不干预）
    */
    void SetTitleColor(const DString& titleColor);
    DString GetTitleColor() const;

    /** 设置标签标题文字在悬停/按下状态下的颜色（为空时使用普通状态颜色）
    */
    void SetHoveredTitleColor(const DString& hoveredTitleColor);
    DString GetHoveredTitleColor() const;

    /** 设置标签标题文字在选中状态下的颜色（为空时使用普通状态颜色）
    */
    void SetSelectedTitleColor(const DString& selectedTitleColor);
    DString GetSelectedTitleColor() const;

    /** 设置标签标题文字在选中且悬停状态下的颜色（为空时使用选中状态颜色）
    */
    void SetSelectedHoveredTitleColor(const DString& selectedHoveredTitleColor);
    DString GetSelectedHoveredTitleColor() const;

    /** 设置标签标题文字在禁用状态下的颜色（为空时保持原有颜色）
    */
    void SetDisabledTitleColor(const DString& disabledTitleColor);
    DString GetDisabledTitleColor() const;

    /** 设置标签在选中状态下的标题字体ID（为空时保持原有字体）
    */
    void SetSelectedTitleFont(const DString& selectedTitleFont);
    DString GetSelectedTitleFont() const;

    /** 关闭按钮控件的Class
    */
    void SetCloseButtonClass(const DString& closeButtonClass);
    DString GetCloseButtonClass() const;

    /** 分割线控件的Class
    */
    void SetLineClass(const DString& lineClass);
    DString GetLineClass() const;

public:
    /** 设置标签是否可关闭（为 false 时隐藏关闭按钮，且中键点击不触发关闭请求）
    * @param [in] bClosable true表示可关闭（默认值），false表示不可关闭
    */
    void SetClosable(bool bClosable);

    /** 判断标签是否可关闭
    */
    bool IsClosable() const;

    /** 根据标签栏方向更新分割线的布局
    * @param [in] bVertical true表示垂直标签栏（分割线为底部横线），false表示水平标签栏（分割线为右侧竖线）
    */
    void UpdateLineOrientation(bool bVertical);

public:
    /** 设置选择状态的标签圆角大小
    * @param [in] szCorner 圆角的宽度和高度
    * @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
    */
    void SetSelectedRoundCorner(UiSize szCorner, bool bNeedDpiScale);

    /** 获取选择状态的标签圆角大小
    */
    UiSize GetSelectedRoundCorner() const;

    /** 设置悬停状态的标签圆角大小
    * @param [in] szCorner 圆角的宽度和高度
    * @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
    */
    void SetHoveredRoundCorner(UiSize szCorner, bool bNeedDpiScale);

    /** 获取悬停状态的标签圆角大小
    */
    UiSize GetHoveredRoundCorner() const;

    /** 设置Hovered状态的背景色的内边距
    * @param [in] rcPadding 内边距数据
    * @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
    */
    void SetHoveredPadding(UiPadding rcPadding, bool bNeedDpiScale);

    /** 获取Hovered状态的背景色的内边距
    */
    UiPadding GetHoveredPadding() const;

    /** 设置关闭按钮是否自动隐藏
    * @param [in] bAutoHideCloseBtn true表示自动隐藏关闭按钮，false表示关闭按钮始终显示
    */
    void SetAutoHideCloseButton(bool bAutoHideCloseBtn);

    /** 获取关闭按钮是否自动隐藏
    */
    bool IsAutoHideCloseButton() const;

    /** 设置关闭按钮的显示策略
    * @param [in] mode kAuto：沿用 auto_hide_close_button 的逻辑（默认值）；kHover：仅悬停显示；kAlways：始终显示
    */
    void SetCloseButtonShowMode(TabCtrlCloseButtonShowMode mode);

    /** 获取关闭按钮的显示策略
    */
    TabCtrlCloseButtonShowMode GetCloseButtonShowMode() const;

    /** 处理分割线的显示或者隐藏
    */
    void AdjustItemLineStatus();

    /** 当前标签的分割线是否处于显示状态
    */
    bool IsItemLineVisible() const;

    /** 设置标签分割线的显示或者隐藏
    */
    void SetItemLineVisible(bool bVisible);

public:
    /** 判断是否支持拖动改变控件的顺序
    */
    virtual bool IsEnableDragOrder() const override;

    /** 设置标签是否为固定标签（固定标签不可拖动、不可作为拖放目标，且不显示关闭按钮）
    * @param [in] bFixed true表示为固定标签
    */
    void SetFixed(bool bFixed);

    /** 判断标签是否为固定标签
    */
    bool IsFixed() const;

protected:
    /** 初始化接口
    */
    virtual void OnInit() override;

    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 处理鼠标移入消息
    */
    virtual bool MouseEnter(const EventArgs& msg) override;

    /** 处理鼠标移出消息
    */
    virtual bool MouseLeave(const EventArgs& msg) override;

    /** 鼠标左键按下消息
    */
    virtual bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标中键按下消息（用于中键关闭标签）
    */
    virtual bool MButtonDown(const EventArgs& msg) override;

    /** 鼠标滚轮消息（转发给TabCtrl，用于滚轮切换标签或滚动标签栏）
    */
    virtual bool MouseWheel(const EventArgs& msg) override;

    /** 绘制控件状态颜色的函数
    */
    virtual void PaintStateColors(IRender* pRender) override;

    /** 选择状态变化事件(m_bSelected变量发生变化)
    */
    virtual void OnPrivateSetSelected() override;

    /** 获得焦点（通知父控件刷新焦点框）
    */
    virtual bool OnSetFocus(const EventArgs& msg) override;

    /** 失去焦点（通知父控件刷新焦点框）
    */
    virtual bool OnKillFocus(const EventArgs& msg) override;

    /** 绘制标签页(选择状态)
    */
    virtual void PaintTabItemSelected(IRender* pRender) ;

    /** 绘制标签页(悬停状态)
    */
    virtual void PaintTabItemHovered(IRender* pRender);

    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

    /** 主题发生变化，清除缓存的绘制资源
    */
    virtual void OnThemeChanged(bool bRedraw) override;

protected:
    /** 填充路径, 形成圆角矩形
    */
    void AddTabItemPath(IPath* path, const UiRect& rect, UiSize roundSize) const;

    /** 调整子控件的顺序
    */
    void AdjustSubItemIndex();

    /** 获取TabCtrl接口
    */
    TabCtrl* GetTabCtrl() const;

    /** 清除缓存的绘制资源（画刷、画笔、路径、颜色解析结果等）
    */
    void ClearPaintCache();

    /** 更新关闭按钮的可见性（综合自动隐藏与固定标签两个因素）
    */
    void UpdateCloseButtonVisible();

    /** 更新标题文字的颜色（根据选中、悬停状态应用配置的颜色）
    */
    void UpdateTitleTextColor();

    /** 更新标题文字的字体（根据选中状态应用配置的字体）
    */
    void UpdateTitleFont();

    /** 更新角标的布局模式（流式 / 右上角浮动）
    */
    void UpdateBadgeLayoutMode();

    /** 检查并设置图标的可见性
    */
    void CheckIconVisible();

private:
    /** 选择标签的圆角大小
    */
    struct RoundCorner
    {
        int32_t cx; //圆角的宽度，小于0表示未配置（使用默认值）
        int32_t cy; //圆角的高度，小于0表示未配置（使用默认值）
    };

    /** Hovered标签的状态Padding值
    */
    struct HoveredPadding
    {
        int32_t left;   //左侧内边距，小于0表示未配置（使用默认值）
        int32_t top;
        int32_t right;
        int32_t bottom;
    };

    /** 选择状态的标签圆角大小
    */
    RoundCorner m_rcSelected;

    /** 悬停状态的标签圆角大小
    */
    RoundCorner m_rcHovered;

    /** Hovered标签的状态Padding值
    */
    HoveredPadding m_hoveredPadding;

    /** 关闭按钮是否自动隐藏
    */
    bool m_bAutoHideCloseBtn;

    /** 关闭按钮的显示策略
    */
    TabCtrlCloseButtonShowMode m_closeBtnShowMode;

    /** 标签是否可关闭（为 false 时隐藏关闭按钮）
    */
    bool m_bClosable;

    /** 分割线是否已按垂直方向调整过布局（避免重复设置属性）
    */
    bool m_bLineVertical;

    /** 分割线方向布局是否已生效
    */
    bool m_bLineOrientationValid;

    /** 用户配置的是否支持拖动排序（固定标签期间会被临时关闭，取消固定后按此值恢复）
    */
    bool m_bUserDragOrder;

    /** 是否为固定标签（固定标签不可拖动、不显示关闭按钮）
    */
    bool m_bFixed;

    /** 分割线是否可见（缓存，状态未变化时不重复设置控件Alpha）
    */
    bool m_bLineVisible;

    /** 图标控件
    */
    IconControl* m_pIcon;

    /** 文本控件
    */
    Label* m_pLabel;

    /** 关闭按钮
    */
    Button* m_pCloseBtn;

    /** 标签之间的分割线
    */
    Control* m_pLine;

    /** 角标控件（未读数/小红点）
    */
    Badge* m_pBadge;

    /** 图标控件的Class
    */
    UiString m_iconClass;

    /** 文本控件的Class
    */
    UiString m_titleClass;

    /** 标签标题文字在普通状态下的颜色
    */
    UiString m_titleColor;

    /** 标签标题文字在悬停/按下状态下的颜色
    */
    UiString m_hoveredTitleColor;

    /** 标签标题文字在选中状态下的颜色
    */
    UiString m_selectedTitleColor;

    /** 标签标题文字的原始颜色（未配置状态颜色时用于恢复）
    */
    UiString m_defaultTitleColor;

    /** 标签标题文字的原始字体ID（未配置选中字体时用于恢复）
    */
    UiString m_defaultTitleFontId;

    /** 标签标题文字在选中且悬停状态下的颜色
    */
    UiString m_selectedHoveredTitleColor;

    /** 标签标题文字在禁用状态下的颜色
    */
    UiString m_disabledTitleColor;

    /** 标签在选中状态下的标题字体ID
    */
    UiString m_selectedTitleFont;

    /** 角标是否使用右上角浮动模式
    */
    bool m_bBadgeFloat;

    /** 关闭按钮控件的Class
    */
    UiString m_closeBtnClass;

    /** 分割线的Class
    */
    UiString m_lineClass;

    /** 角标控件的Class
    */
    UiString m_badgeClass;

    /** 角标数量（小于等于0时自动隐藏）
    */
    int64_t m_nBadgeCount;

    /** 角标数量上限，超过时显示"上限+"（如"99+"）
    */
    int64_t m_nBadgeMaxCount;

    /** 角标是否为红点模式（纯小圆点，不显示数字）
    */
    bool m_bBadgeDot;

    /** 文本内容
    */
    UiString m_title;

    /** 文本内容ID
    */
    UiString m_titleId;

    /** 图标资源字符串
    */
    UiString m_iconImageString;

    /** 选中状态背景画刷缓存（颜色变化时重建）
    */
    std::unique_ptr<IBrush> m_spSelectedBgBrush;

    /** 选中背景画刷缓存对应的颜色字符串
    */
    UiString m_selectedBgBrushColor;

    /** 选中状态轮廓画笔缓存（颜色或宽度变化时重建）
    */
    std::unique_ptr<IPen> m_spSelectedOutlinePen;
    UiString m_selectedOutlinePenColor;
    float m_fSelectedOutlinePenWidth;

    /** 选中状态背景路径缓存（矩形或圆角变化时重建）
    */
    std::unique_ptr<IPath> m_spSelectedPath;
    UiRect m_rcSelectedPath;
    UiSize m_szSelectedPathRound;
    bool m_bSelectedPathValid;

    /** 悬停状态背景颜色缓存（颜色字符串变化时重新解析）
    */
    UiString m_hoveredColorKey;
    UiColor m_hoveredColorValue;

private:
    /** 绑定的TabBox子项索引号
    */
    size_t m_nTabBoxItemIndex;
};

}//namespace ui

#endif //UI_CONTROL_TABCTRL_H_
