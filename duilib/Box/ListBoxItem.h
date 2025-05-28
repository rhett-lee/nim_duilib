#ifndef UI_BOX_LISTBOX_ITEM_H_
#define UI_BOX_LISTBOX_ITEM_H_

#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Control/Option.h"
#include "duilib/Core/Keyboard.h"

namespace ui 
{
class IListBoxItem;

/** 确保可见的附加标志(垂直方向，垂直滚动条)
*/
enum class ListBoxVerVisible
{
    kVisible,           //保证显示在可见区域
    kVisibleAtTop,      //保证显示在可见区域的顶部
    kVisibleAtCenter,   //保证显示在可见区域的中间
    kVisibleAtBottom,   //保证显示在可见区域的低部
};

/** 确保可见的附加标志(水平方向，水平滚动条)
*/
enum class ListBoxHorVisible
{
    kVisible,           //保证显示在可见区域
    kVisibleAtLeft,     //保证显示在可见区域的左侧
    kVisibleAtCenter,   //保证显示在可见区域的中间
    kVisibleAtRight,    //保证显示在可见区域的右侧
};

/** ListBox所有者接口
*/
class UILIB_API IListBoxOwner
{
public:
    /** 触发事件
    */
    virtual void SendEventMsg(const EventArgs& msg) = 0;

    /** 列表项的子项收到鼠标事件
    * @return true表示截获该消息，子项不再处理该消息；返回false表示子项继续处理该消息
    */
    virtual bool OnListBoxItemMouseEvent(const EventArgs& msg) = 0;

    /** 列表项的子项收到窗口失去焦点事件
    */
    virtual void OnListBoxItemWindowKillFocus() = 0;

    /** 获取当前选择的索引，(如果无有效索引，则返回Box::InvalidIndex)
    */
    virtual size_t GetCurSel() const = 0;

    /** 设置当前选择的索引
    * @param [in] iIndex 子项目的ID
    */
    virtual void SetCurSel(size_t iIndex) = 0;

    /** 选择子项
    * @param [in] iIndex 子项目的ID
    * @param [in] bTakeFocus 是否让子项控件成为焦点控件
    * @param [in] bTriggerEvent 是否触发选择事件, 如果为true，会触发一个kEventSelect事件
    * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
    * @return 返回true代表内部选择状态发生变化，返回false代表内部状态无变化
    */
    virtual bool SelectItem(size_t iIndex, bool bTakeFocus,
                            bool bTriggerEvent, uint64_t vkFlag = 0) = 0;

    /** 取消选择子项
    * @param [in] iIndex 子项目的ID
    * @param [in] bTriggerEvent 是否触发选择事件, 如果为true，会触发一个kEventUnSelect事件
    * @return 返回true代表内部选择状态发生变化，返回false代表内部状态无变化
    */
    virtual bool UnSelectItem(size_t iIndex, bool bTriggerEvent) = 0;

    /** 子项的选择状态变化事件，用于状态同步
    * @param [in] iIndex 子项目的ID，范围是：[0, GetItemCount())
    * @param [in] pListBoxItem 关联的列表项接口
    */
    virtual void OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem) = 0;

    /** 子项的勾选状态变化事件，用于状态同步
    * @param [in] iIndex 子项目的ID，范围是：[0, GetItemCount())
    * @param [in] pListBoxItem 关联的列表项接口
    */
    virtual void OnItemCheckedChanged(size_t iIndex, IListBoxItem* pListBoxItem) = 0;

    /** 确保矩形区域可见
    * @param [in] rcItem 可见区域的矩形范围
    * @param [in] vVisibleType 垂直方向可见的附加标志
    * @param [in] hVisibleType 水平方向可见的附加标志
    */
    virtual void EnsureVisible(const UiRect& rcItem,
                               ListBoxVerVisible vVisibleType,
                               ListBoxHorVisible hVisibleType) = 0;

    /** 停止滚动条动画
    */
    virtual void StopScroll() = 0;

    /** 是否绘制选择状态下的背景色，提供虚函数作为可选项
        （比如ListBox/TreeView节点在多选时，由于有勾选项，并不需要绘制选择状态的背景色）
        @param [in] bHasStateImages 当前列表项是否有CheckBox勾选项
    */
    virtual bool CanPaintSelectedColors(bool bHasStateImages) const = 0;

    /** 是否允许多选
    */
    virtual bool IsMultiSelect() const = 0;

    /** 选择子项后的事件，单选时用于保证只有一个选中项
    */
    virtual void EnsureSingleSelection() = 0;
};

/** ListBoxItem 接口
*/
class UILIB_API IListBoxItem
{
public:
    virtual ~IListBoxItem() = default;

    /** 调用Option类的选择函数, 只更新界面的选择状态
    * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。
    */
    virtual void OptionSelected(bool bSelect, bool bTriggerEvent) = 0;

    /** 设置选择状态, 处理存储状态同步，但不触发任何事件(适用于同步状态)
    */
    virtual void SetItemSelected(bool bSelected) = 0;

    /** 判断当前是否是选择状态
     * @return 返回 true 为选择状态，否则为 false
     */
    virtual bool IsSelected() const = 0;

    /** 获取父容器
     */
    virtual IListBoxOwner* GetOwner() = 0;

    /** 设置父容器
     * @param[in] pOwner 父容器指针
     */
    virtual void SetOwner(IListBoxOwner* pOwner) = 0;

    /** 获取容器索引号，范围：[0, GetItemCount())
     */
    virtual size_t GetListBoxIndex() const = 0;

    /** 设置容器子项索引号
     * @param[in] iIndex 索引号, 范围：[0, GetItemCount())
     */
    virtual void SetListBoxIndex(size_t iIndex) = 0;

    /** 获取虚表数据元素索引号，用于支持虚表，范围：[0, GetElementCount())
     */
    virtual size_t GetElementIndex() const = 0;

    /** 设置虚表数据元素索引号
     * @param[in] iIndex 索引号, 用于支持虚表，范围：[0, GetElementCount())
     */
    virtual void SetElementIndex(size_t iIndex) = 0;
};

/** 列表项的数据子项，用于在列表中展示数据的子项
*/
template<typename InheritType = Box>
class UILIB_API ListBoxItemTemplate:
    public OptionTemplate<InheritType>,
    public IListBoxItem
{
public:
    explicit ListBoxItemTemplate(Window* pWindow);

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;    
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 是否绘制选择状态下的背景色，提供虚函数作为可选项
       （比如ListBox/TreeView节点在多选时，由于有勾选项，并不需要绘制选择状态的背景色）
    */
    virtual bool CanPaintSelectedColors() const override;

    /** 判断控件类型是否为可选择的
     * @return 返回true
     */
    virtual bool IsSelectableType() const override;

    /** 设置控件是否选择状态
     * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false
     * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
     */
    virtual void Selected(bool bSelect, bool bTriggerEvent, uint64_t vkFlag) override;

    /** 调用Option类的选择函数, 只更新界面的选择状态
    * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。
    */
    virtual void OptionSelected(bool bSelect, bool bTriggerEvent) override;

    /** 设置选择状态, 处理存储状态同步，但不触发任何事件(适用于同步状态)
    */
    virtual void SetItemSelected(bool bSelected) override;

    /** 判断当前是否是选择状态
     * @return 返回 true 为选择状态，否则为 false
     */
    virtual bool IsSelected() const override;

    /** 获取父容器
     */
    virtual IListBoxOwner* GetOwner() override;

    /** 设置父容器
     * @param[in] pOwner 父容器指针
     */
    virtual void SetOwner(IListBoxOwner* pOwner) override;

    /** 获取容器索引号，范围：[0, GetItemCount())
     */
    virtual size_t GetListBoxIndex() const override;

    /** 设置容器子项索引号
     * @param[in] iIndex 索引号, 范围：[0, GetItemCount())
     */
    virtual void SetListBoxIndex(size_t iIndex) override;

    /** 获取虚表数据元素索引号，用于支持虚表，范围：[0, GetElementCount())
     */
    virtual size_t GetElementIndex() const override;

    /** 设置虚表数据元素索引号
     * @param[in] iIndex 索引号, 用于支持虚表，范围：[0, GetElementCount())
     */
    virtual void SetElementIndex(size_t iIndex) override;

public:
    /** 绑定鼠标点击处理函数
    * @param[in] callback 要绑定的回调函数
    */
    void AttachClick(const EventCallback& callback) { this->AttachEvent(kEventClick, callback); }

    /** 绑定鼠标右键点击处理函数
    * @param[in] callback 要绑定的回调函数
    */
    void AttachRClick(const EventCallback& callback) { this->AttachEvent(kEventRClick, callback); }

    /** 监听控件双击事件
     * @param[in] callback 收到双击消息时的回调函数
     */
    void AttachDoubleClick(const EventCallback& callback) { this->AttachEvent(kEventMouseDoubleClick, callback); }

    /** 监听回车事件
     * @param[in] callback 收到回车时的回调函数
     */
    void AttachReturn(const EventCallback& callback) { this->AttachEvent(kEventReturn, callback); }

protected:
    /** 选择状态变化事件(m_bSelected变量发生变化)
    */
    virtual void OnPrivateSetSelected() override;

    /** 勾选状态变化事件(m_bChecked变量发生变化)
    */
    virtual void OnPrivateSetChecked() override;

private:
    /** 在ListBox容器中的子项索引号，范围：[0, GetItemCount())
    */
    size_t m_iListBoxIndex;

    /** 虚表数据元素子项索引号，用于支持虚表，范围：[0, GetElementCount())
    */
    size_t m_iElementIndex;

    /** 在ListBox容器接口
    */
    IListBoxOwner* m_pOwner;
};

/////////////////////////////////////////////////////////////////////////////////////

template<typename InheritType>
ListBoxItemTemplate<InheritType>::ListBoxItemTemplate(Window* pWindow):
    OptionTemplate<InheritType>(pWindow),
    m_iListBoxIndex(Box::InvalidIndex),
    m_iElementIndex(Box::InvalidIndex),
    m_pOwner(nullptr)
{
    this->SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, false);
}

template<typename InheritType>
DString ListBoxItemTemplate<InheritType>::GetType() const { return DUI_CTR_LISTBOX_ITEM; }

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetItemSelected(bool bSelected)
{
    if (OptionTemplate<InheritType>::IsSelected() == bSelected) {
        return;
    }
    //直接修改内部状态
    OptionTemplate<InheritType>::SetSelected(bSelected);
    if (m_pOwner == nullptr) {
        return;
    }

    //同步ListBox的选择ID
    bool bChanged = false;
    if (bSelected) {
        m_pOwner->SetCurSel(m_iListBoxIndex);
        bChanged = true;
    }
    else {
        if (m_pOwner->GetCurSel() == m_iListBoxIndex) {
            m_pOwner->SetCurSel(Box::InvalidIndex);
            bChanged = true;
        }
    }

    if (bChanged && !m_pOwner->IsMultiSelect()) {
        //单选：需要调用选择函数, 保证只有一个选中项
        m_pOwner->EnsureSingleSelection();
    }
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OptionSelected(bool bSelect, bool bTriggerEvent)
{
    return OptionTemplate<InheritType>::Selected(bSelect, bTriggerEvent);
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::Selected(bool bSelected, bool bTriggerEvent, uint64_t vkFlag)
{
    //界面点击等操作触发选择操作
    if (!this->IsEnabled()) {
        return;
    }
    if (m_pOwner != nullptr) {
        if (bSelected) {
            m_pOwner->SelectItem(m_iListBoxIndex, false, bTriggerEvent, vkFlag);
        }
        else {
            m_pOwner->UnSelectItem(m_iListBoxIndex, bTriggerEvent);
        }
    }
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::IsSelected() const
{
    return OptionTemplate<InheritType>::IsSelected();
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OnPrivateSetSelected()
{
    if (m_pOwner != nullptr) {
        m_pOwner->OnItemSelectedChanged(m_iListBoxIndex, this);
    }
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OnPrivateSetChecked()
{
    if (m_pOwner != nullptr) {
        m_pOwner->OnItemCheckedChanged(m_iListBoxIndex, this);
    }
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::CanPaintSelectedColors() const
{
    bool bHasStateImages = this->HasStateImages();
    if (m_pOwner != nullptr) {
        return m_pOwner->CanPaintSelectedColors(bHasStateImages);
    }
    return OptionTemplate<InheritType>::CanPaintSelectedColors();
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::HandleEvent(const EventArgs& msg)
{
    if (this->IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给Owner控件
        if (m_pOwner != nullptr) {
            m_pOwner->SendEventMsg(msg);
        }
        else {
            OptionTemplate<InheritType>::HandleEvent(msg);
        }
        return;
    }
    if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
        //将鼠标相关消息转发给父容器优先处理
        if ((m_pOwner != nullptr) && m_pOwner->OnListBoxItemMouseEvent(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventWindowKillFocus) {
        if (m_pOwner != nullptr) {
            m_pOwner->OnListBoxItemWindowKillFocus();
        }
    }

    if (msg.eventType == kEventMouseDoubleClick) {
        if (!this->IsActivatable()) {
            return;
        }
    }
    else if (msg.eventType == kEventKeyDown && this->IsEnabled()) {
        if (msg.vkCode == kVK_RETURN) {
            if (this->IsActivatable()) {
                this->SendEvent(kEventReturn);
            }
            return;
        }
    }
    OptionTemplate<InheritType>::HandleEvent(msg);
}

template<typename InheritType>
IListBoxOwner* ListBoxItemTemplate<InheritType>::GetOwner()
{
    return m_pOwner;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetOwner(IListBoxOwner* pOwner)
{
    m_pOwner = pOwner;
}

template<typename InheritType>
size_t ListBoxItemTemplate<InheritType>::GetListBoxIndex() const
{
    return m_iListBoxIndex;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetListBoxIndex(size_t iIndex)
{
    m_iListBoxIndex = iIndex;
}

template<typename InheritType>
size_t ListBoxItemTemplate<InheritType>::GetElementIndex() const
{
    return m_iElementIndex;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetElementIndex(size_t iIndex)
{
    m_iElementIndex = iIndex;
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::IsSelectableType() const
{
    return true;
}

/** 类型定义
*/
typedef ListBoxItemTemplate<Box> ListBoxItem;
typedef ListBoxItemTemplate<HBox> ListBoxItemH;
typedef ListBoxItemTemplate<VBox> ListBoxItemV;

} // namespace ui

#endif // UI_BOX_LISTBOX_ITEM_H_
