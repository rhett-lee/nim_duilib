#ifndef UI_CONTROL_TOOL_BAR_H_
#define UI_CONTROL_TOOL_BAR_H_

#include "duilib/Box/HBox.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Control/Line.h"
#include <functional>
#include <vector>

namespace ui
{

/** 工具条（工具栏）控件
*   继承 HBox，用于承载一排工具按钮、分隔线等元素，是编辑器 / IDE / 办公类软件的常用壳层元素。
*
*   主要能力：
*   1. 子控件水平排列，沿用 HBox 的全部布局能力；
*   2. 工具项管理：通过 AddToolButton / AddToolCheckBox / AddToolSeparator 动态创建工具项，
*      支持按名称删除、查询、设置勾选/启用/可见状态，无需应用层手工 new + AddItem；
*   3. 工具项事件：AttachToolItemClick / AttachToolItemCheckedChanged 统一接收点击与勾选变化；
*   4. 溢出收纳（overflow）：当可用宽度不足、尾部子控件放不下时，自动隐藏这些子控件，
*      并在右端显示一个"更多"按钮；点击该按钮会弹出浮层，列出被收纳的项，
*      用户点击浮层中的项时，会走与直接点击相同的工具项回调；
*      同时仍通过 OverflowItemActivatedEvent 回调通知应用层（兼容旧用法）。
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

    /** 重写子项添加接口：XML 静态添加的 Button/CheckBox 也会自动绑定工具项事件
    */
    virtual bool AddItem(Control* pControl) override;
    virtual bool AddItemAt(Control* pControl, size_t iIndex) override;

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

    /** 工具项被点击（鼠标点击 / 键盘激活 / 溢出浮层激活）时的回调函数原型
    * @param [in] itemName 工具项的控件名称（name 属性）
    */
    typedef std::function<void(const DString& itemName)> ToolItemClickEvent;

    /** 注册工具项点击的回调函数
    * @param [in] callback 回调函数
    */
    void AttachToolItemClick(ToolItemClickEvent callback);

    /** 清空工具项点击的回调函数
    */
    void ClearToolItemClick();

    /** 工具项勾选状态变化（仅 CheckBox 类型）时的回调函数原型
    * @param [in] itemName 工具项的控件名称（name 属性）
    * @param [in] bChecked 变化后的勾选状态
    */
    typedef std::function<void(const DString& itemName, bool bChecked)> ToolItemCheckedChangedEvent;

    /** 注册工具项勾选状态变化的回调函数
    * @param [in] callback 回调函数
    */
    void AttachToolItemCheckedChanged(ToolItemCheckedChangedEvent callback);

    /** 清空工具项勾选状态变化的回调函数
    */
    void ClearToolItemCheckedChanged();

public:
    /** 设置动态创建工具按钮时使用的默认样式 Class（为空表示不设置样式）
    */
    void SetToolClass(const DString& strClass);
    DString GetToolClass() const;

    /** 设置动态创建勾选类工具项时使用的默认样式 Class（为空时使用 GetToolClass()）
    */
    void SetToolCheckBoxClass(const DString& strClass);
    DString GetToolCheckBoxClass() const;

    /** 设置动态创建分隔线时使用的默认样式 Class（为空表示不设置样式）
    */
    void SetToolSeparatorClass(const DString& strClass);
    DString GetToolSeparatorClass() const;

public:
    /** 添加一个工具按钮
    * @param [in] strName 控件名称，为空时自动生成（tool_1、tool_2……）；与已有工具项重名时添加失败
    * @param [in] strText 按钮文本；bTextId 为 true 时表示多语言 ID
    * @param [in] strClass 样式 Class，为空时使用 GetToolClass()
    * @param [in] bTextId 是否把 strText 作为多语言 ID 解析
    * @return 返回创建成功的按钮指针（可用于继续设置图标/尺寸等），失败返回 nullptr
    */
    Button* AddToolButton(const DString& strName, const DString& strText,
                          const DString& strClass = DString(), bool bTextId = false);

    /** 添加一个勾选类工具项（CheckBox，使用选中态表示勾选）
    * @param [in] strName 控件名称，为空时自动生成；与已有工具项重名时添加失败
    * @param [in] strText 文本；bTextId 为 true 时表示多语言 ID
    * @param [in] bChecked 初始是否勾选
    * @param [in] strClass 样式 Class，为空时使用 GetToolCheckBoxClass()，仍为空则回退 GetToolClass()
    * @param [in] bTextId 是否把 strText 作为多语言 ID 解析
    * @return 返回创建成功的勾选控件指针，失败返回 nullptr
    */
    CheckBox* AddToolCheckBox(const DString& strName, const DString& strText,
                              bool bChecked = false, const DString& strClass = DString(),
                              bool bTextId = false);

    /** 添加一个垂直分隔线
    * @param [in] strName 控件名称，为空时自动生成；与已有工具项重名时添加失败
    * @param [in] strClass 样式 Class，为空时使用 GetToolSeparatorClass()
    * @return 返回创建成功的分隔线控件指针，失败返回 nullptr
    */
    Line* AddToolSeparator(const DString& strName = DString(), const DString& strClass = DString());

    /** 添加任意控件作为工具项
    *   若控件是 Button（含 CheckBox），会自动绑定工具项点击/勾选事件
    * @param [in] pControl 控件指针（成功后由 ToolBar 管理其生命周期）
    * @param [in] strName 控件名称，为空时自动生成；与已有工具项重名时添加失败
    * @return 添加成功返回 true，失败返回 false（失败时调用方仍拥有该控件）
    */
    bool AddTool(Control* pControl, const DString& strName = DString());

    /** 在指定位置插入任意控件作为工具项
    * @param [in] nIndex 工具项索引（不含内部"更多"按钮），超过范围时追加到末尾
    * @param [in] pControl 控件指针（成功后由 ToolBar 管理其生命周期）
    * @param [in] strName 控件名称，为空时自动生成
    */
    bool InsertTool(size_t nIndex, Control* pControl, const DString& strName = DString());

public:
    /** 工具项数量（不含内部"更多"按钮）
    */
    size_t GetToolCount() const;

    /** 获取指定索引的工具项（不含内部"更多"按钮），越界返回 nullptr
    */
    Control* GetToolAt(size_t nIndex) const;

    /** 按名称查找工具项，找不到返回 nullptr
    */
    Control* FindTool(const DString& strName) const;

    /** 获取工具项索引（不含内部"更多"按钮），找不到返回 Box::InvalidIndex
    */
    size_t GetToolIndex(const DString& strName) const;

    /** 是否存在指定名称的工具项
    */
    bool HasTool(const DString& strName) const;

public:
    /** 按名称删除工具项（控件会被销毁）
    */
    bool RemoveTool(const DString& strName);

    /** 按索引删除工具项（控件会被销毁），索引不含内部"更多"按钮
    */
    bool RemoveToolAt(size_t nIndex);

    /** 删除全部工具项（保留内部"更多"按钮）
    */
    void RemoveAllTools();

public:
    /** 设置工具项的勾选状态（仅 CheckBox 类型有效）
    * @param [in] strName 工具项名称
    * @param [in] bChecked 是否勾选
    * @param [in] bTriggerEvent 是否触发 ToolItemCheckedChanged 回调（不会触发点击回调）
    * @return 找到并设置成功返回 true
    */
    bool SetToolChecked(const DString& strName, bool bChecked, bool bTriggerEvent = false);

    /** 获取工具项的勾选状态（仅 CheckBox 类型有效），非勾选类返回 false
    */
    bool IsToolChecked(const DString& strName) const;

    /** 设置工具项是否可用
    */
    bool SetToolEnabled(const DString& strName, bool bEnabled);

    /** 获取工具项是否可用
    */
    bool IsToolEnabled(const DString& strName) const;

    /** 设置工具项是否显示（隐藏的项不参与溢出收纳）
    */
    bool SetToolVisible(const DString& strName, bool bVisible);

    /** 获取工具项是否显示
    */
    bool IsToolVisible(const DString& strName) const;

public:
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

    /** 触发工具项点击的回调函数
    */
    void FireToolItemClick(const DString& itemName);

    /** 触发工具项勾选状态变化的回调函数
    */
    void FireToolItemCheckedChanged(const DString& itemName, bool bChecked);

    /** 激活一个工具项（点击 / 键盘 / 溢出浮层）
    *   Button 直接派发点击回调；CheckBox 先切换选中态再派发点击与勾选回调
    */
    void ActivateTool(Control* pControl);

    /** 为 Button / CheckBox 绑定工具项事件（不会重复绑定）
    */
    void BindToolEvents(Control* pControl);

    /** 生成一个未被占用的工具项名称
    */
    DString MakeToolName() const;

    /** 校验名称是否可用（为空或未被占用）
    */
    bool IsToolNameAvailable(const DString& strName) const;

    /** 从自动收纳列表中移除指定控件（避免悬空指针）
    */
    void ForgetHiddenItem(Control* pControl);

    /** 判断某个控件是否为当前容器的子控件（内部"更多"按钮除外）
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

    //动态创建工具按钮的默认 Class
    UiString m_toolClass;

    //动态创建勾选类工具项的默认 Class
    UiString m_toolCheckBoxClass;

    //动态创建分隔线的默认 Class
    UiString m_toolSeparatorClass;

    //当前被自动收纳（隐藏）的子控件
    std::vector<Control*> m_hiddenItems;

    //溢出项激活的回调函数
    std::vector<OverflowItemActivatedEvent> m_callbackList;

    //工具项点击的回调函数
    std::vector<ToolItemClickEvent> m_toolClickCallbacks;

    //工具项勾选状态变化的回调函数
    std::vector<ToolItemCheckedChangedEvent> m_toolCheckedCallbacks;

    //已被绑定事件工具项的地址集合（避免重复绑定）
    std::vector<Control*> m_boundToolItems;

    //布局重入保护标记
    bool m_bInLayout;
};

} // namespace ui

#endif // UI_CONTROL_TOOL_BAR_H_
