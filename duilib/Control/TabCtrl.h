#ifndef UI_CONTROL_TABCTRL_H_
#define UI_CONTROL_TABCTRL_H_

#include "duilib/Box/ListBox.h"
#include "duilib/Core/ControlDragable.h"
#include "duilib/Control/IconControl.h"

namespace ui
{
/** 多标签控件（类似浏览器的多标签）
*/
class TabBox;
class TabCtrl: public ListBox
{
    typedef ListBox BaseClass;
public:
    explicit TabCtrl(Window* pWindow);

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

    /** 移除所有子项
     */
    virtual void RemoveAllItems() override;

protected:
    /** 初始化接口
    */
    virtual void OnInit() override;

    /** 消息处理函数
    * @param [in] msg 消息内容
    */
    virtual void HandleEvent(const EventArgs& msg) override;

private:
    /** 默认选择的子项
    */
    size_t m_nSelectedId;

    /** 绑定的TabBox名称
    */
    UiString m_tabBoxName;

    /** 绑定的TabBox接口
    */
    TabBox* m_pTabBox;

    /** 是否支持拖动改变列的顺序(功能开关)
    */
    bool m_bEnableDragOrder;
};

/** 多标签控件的一个标签页
*/
class TabCtrlItem : public ControlDragableT<ListBoxItemH>
{
    typedef ControlDragableT<ListBoxItemH> BaseClass;
public:
    explicit TabCtrlItem(Window* pWindow);

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

    /** 设置图标的位图数据(数据格式：ARGB格式，alpha type为kPremul_SkAlphaType)
    * @param [in] nWidth 宽度
    * @param [in] nHeight 高度
    * @param [in] pPixelBits 位图数据
    * @param [in] nPixelBitsSize 位图数据的长度（按字节）
    */
    bool SetIconData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize);

    /** 设置文字内容
    */
    void SetTitle(const DString& title);

    /** 获取文字内容
    */
    DString GetTitle() const;

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

public:
    /** 图标控件的Class
    */
    void SetIconClass(const DString& iconClass);
    DString GetIconClass() const;

    /** 文本控件的Class
    */
    void SetTitleClass(const DString& titleClass);
    DString GetTitleClass() const;

    /** 关闭按钮控件的Class
    */
    void SetCloseButtonClass(const DString& closeButtonClass);
    DString GetCloseButtonClass() const;

    /** 分割线控件的Class
    */
    void SetLineClass(const DString& lineClass);
    DString GetLineClass() const;

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
    void SetHotRoundCorner(UiSize szCorner, bool bNeedDpiScale);

    /** 获取悬停状态的标签圆角大小
    */
    UiSize GetHotRoundCorner() const;

    /** 设置Hot状态的背景色的内边距
    * @param [in] rcPadding 内边距数据
    * @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
    */
    void SetHotPadding(UiPadding rcPadding, bool bNeedDpiScale);

    /** 获取Hot状态的背景色的内边距
    */
    UiPadding GetHotPadding() const;

    /** 设置关闭按钮是否自动隐藏
    * @param [in] bAutoHideCloseBtn true表示自动隐藏关闭按钮，false表示关闭按钮始终显示
    */
    void SetAutoHideCloseButton(bool bAutoHideCloseBtn);

    /** 获取关闭按钮是否自动隐藏
    */
    bool IsAutoHideCloseButton() const;

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

    /** 绘制控件状态颜色的函数
    */
    virtual void PaintStateColors(IRender* pRender) override;

    /** 选择状态变化事件(m_bSelected变量发生变化)
    */
    virtual void OnPrivateSetSelected() override;

    /** 绘制标签页(选择状态)
    */
    virtual void PaintTabItemSelected(IRender* pRender) ;

    /** 绘制标签页(悬停状态)
    */
    virtual void PaintTabItemHot(IRender* pRender);

    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

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

    /** 检查并设置图标的可见性
    */
    void CheckIconVisible();

private:
    /** 选择标签的圆角大小
    */
    struct RoundCorner
    {
        uint8_t cx; //圆角的宽度
        uint8_t cy; //圆角的高度
    };

    /** Hot标签的状态Padding值
    */
    struct HotPadding
    {
        uint8_t left;
        uint8_t top;
        uint8_t right;
        uint8_t bottom;
    };

    /** 选择状态的标签圆角大小
    */
    RoundCorner m_rcSelected;

    /** 悬停状态的标签圆角大小
    */
    RoundCorner m_rcHot;

    /** Hot标签的状态Padding值
    */
    HotPadding m_hotPadding;

    /** 关闭按钮是否自动隐藏
    */
    bool m_bAutoHideCloseBtn;

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

    /** 图标控件的Class
    */
    UiString m_iconClass;

    /** 文本控件的Class
    */
    UiString m_titleClass;

    /** 关闭按钮控件的Class
    */
    UiString m_closeBtnClass;

    /** 分割线的Class
    */
    UiString m_lineClass;

    /** 文本内容
    */
    UiString m_title;

    /** 图标资源字符串
    */
    UiString m_iconImageString;

private:
    /** 绑定的TabBox子项索引号
    */
    size_t m_nTabBoxItemIndex;
};

}//namespace ui

#endif //UI_CONTROL_TABCTRL_H_
