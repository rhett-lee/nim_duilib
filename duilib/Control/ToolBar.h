#ifndef UI_CONTROL_TOOL_BAR_H_
#define UI_CONTROL_TOOL_BAR_H_

#include "duilib/Box/HBox.h"
#include "duilib/Core/ControlPtrT.h"
#include <functional>
#include <vector>

namespace ui
{

/** 工具条（工具栏）控件
*   继承 HBox，用于承载一排工具按钮、分隔线等元素，是编辑器 / IDE / 办公类软件的常用壳层元素。
*
*   主要能力：
*   1. 子控件水平排列，沿用 HBox 的全部布局能力；工具条中的分隔线可直接使用 Line 控件（vertical="true"）；
*   2. 溢出收纳（overflow）：当可用宽度不足、尾部子控件放不下时，自动隐藏这些子控件，
*      并在右端显示一个"更多"按钮；点击该按钮会弹出浮层，列出被收纳的项，
*      用户点击浮层中的项时，通过 OverflowItemActivatedEvent 回调通知应用层。
*
*   XML 节点名称为 "ToolBar"，继承 HBox 的全部属性。
*/
class DUILIB_API ToolBar : public HBox
{
    typedef HBox BaseClass;
public:
    explicit ToolBar(Window* pWindow);
    virtual ~ToolBar() override;

public:
    /// 重写父类接口，提供个性化功能。方法具体说明请查看 Control / Box 控件
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetPos(UiRect rc) override;

public:
    /** 溢出项被激活（鼠标点击）的回调函数原型
    * @param [in] itemName 被点击项的控件名称（即子控件的 name 属性）
    */
    typedef std::function<void(const DString& itemName)> OverflowItemActivatedEvent;

    /** 注册溢出项激活的回调函数
    * @param [in] callback 回调函数
    */
    void AttachOverflowItemActivated(OverflowItemActivatedEvent callback);

    /** 清空所有回调函数
    */
    void ClearOverflowItemActivated();

    /** 设置是否启用溢出收纳（默认启用）
    */
    void SetOverflowEnabled(bool bEnabled);
    bool IsOverflowEnabled() const;

    /** 设置"更多"按钮的样式 Class 名称（引用 global.xml 中的样式，为空表示不设置样式）
    */
    void SetOverflowButtonClass(const DString& strClass);
    DString GetOverflowButtonClass() const;

    /** 设置"更多"按钮的宽度（像素，0 表示按内容自适应）
    */
    void SetOverflowButtonWidth(int32_t nWidth);
    int32_t GetOverflowButtonWidth() const;

    /** 设置"更多"按钮显示的文本（默认为 ">>"）
    */
    void SetOverflowButtonText(const DString& strText);
    DString GetOverflowButtonText() const;

    /** 当前是否存在被自动收纳（隐藏）的子控件
    */
    bool IsOverflowing() const;

private:
    /** 创建"更多"按钮（幂等：已创建则不重复创建）
    */
    void CreateOverflowButton();

    /** 计算并应用溢出收纳（在布局时调用）
    * @param [in] rc 本控件的布局区域
    */
    void UpdateOverflow(const UiRect& rc);

    /** 弹出被收纳项的浮层列表
    */
    void ShowOverflowFlyout();

    /** 触发溢出项激活的回调函数
    */
    void FireOverflowItemActivated(const DString& itemName);

    /** 判断某个控件是否为当前容器的子控件
    */
    bool IsChildControl(Control* pControl) const;

    /** 判断集合中是否包含某个控件
    */
    static bool ContainsItem(const std::vector<Control*>& items, Control* pControl);

private:
    //是否启用溢出收纳
    bool m_bOverflowEnabled;

    //"更多"按钮
    ControlPtr m_pOverflowButton;

    //"更多"按钮的样式 Class 名称
    UiString m_overflowButtonClass;

    //"更多"按钮的宽度（0 表示按内容自适应）
    int32_t m_nOverflowButtonWidth;

    //"更多"按钮显示的文本
    UiString m_overflowButtonText;

    //当前被自动收纳（隐藏）的子控件
    std::vector<Control*> m_hiddenItems;

    //溢出项激活的回调函数
    std::vector<OverflowItemActivatedEvent> m_callbackList;

    //布局重入保护标记
    bool m_bInLayout;
};

} // namespace ui

#endif // UI_CONTROL_TOOL_BAR_H_
