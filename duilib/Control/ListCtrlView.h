#ifndef UI_CONTROL_LIST_CTRL_VIEW_H_
#define UI_CONTROL_LIST_CTRL_VIEW_H_

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
/** ListCtrl列表视图UI控件的基类（实现子项选择功能，包括方向键，快捷键，鼠标选择逻辑等）
*   使用这个类的时候，需要用 ListCtrlItemTemplate 模板来形成子类作为ListBoxItem
*/
class ListCtrlView : public VirtualListBox
{
    typedef VirtualListBox BaseClass;
public:
    ListCtrlView(Window* pWindow, Layout* pLayout);
    virtual ~ListCtrlView() override;

    virtual DString GetType() const override { return _T("ListCtrlView"); }
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 选择子项
    *  @param [in] iIndex 子项目的ID
    *  @param [in] bTakeFocus 是否让子项控件成为焦点控件
    *  @param [in] bTriggerEvent 是否触发选择事件, 如果为true，会触发一个kEventSelect事件
    *  @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
    */
    virtual bool SelectItem(size_t iIndex, bool bTakeFocus,
                            bool bTriggerEvent, uint64_t vkFlag) override;

public:
    /** 监听选择项发生变化的事件
     * @param[in] callback 选择子项时的回调函数
     */
    void AttachSelChange(const EventCallback& callback) { AttachEvent(kEventSelChange, callback); }

    /** 发送事件的函数
    */
    virtual void SendEventMsg(const EventArgs& msg) override;

public:
    /** 获取顶部元素的索引号
    */
    virtual size_t GetTopElementIndex() const;

    /** 判断一个数据项是否可见
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    virtual bool IsDataItemDisplay(size_t itemIndex) const;

    /** 获取当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    virtual void GetDisplayDataItems(std::vector<size_t>& itemIndexList) const;

    /** 确保数据索引项可见
    * @param [in] itemIndex 数据项的索引号
    * @param [in] bToTop 是否确保在最上方
    */
    virtual bool EnsureDataItemVisible(size_t itemIndex, bool bToTop);

protected:
    /** 控件初始化
    */
    virtual void OnInit() override;
};

/** ListCtrl各个视图中数据项的基类模板
*/
template<typename InheritType>
class UILIB_API ListCtrlItemTemplate : public ListBoxItemTemplate<InheritType>
{
    typedef ListBoxItemTemplate<InheritType> BaseClass;
public:
    explicit ListCtrlItemTemplate(Window* pWindow):
        ListBoxItemTemplate<InheritType>(pWindow)
    {
    }
    virtual ~ListCtrlItemTemplate() override = default;

    /** 设置控件是否选择状态
  * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
  * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
  * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
  */
    virtual void Selected(bool bSelect, bool bTriggerEvent, uint64_t vkFlag) override
    {
        if (BaseClass::IsSelected() != bSelect) {
            BaseClass::Selected(bSelect, bTriggerEvent, vkFlag);
        }
    }

protected:
    /** 激活函数
    */
    virtual void Activate(const EventArgs* pMsg) override
    {
        //重写基类的实现逻辑，这里只发出一个Click事件
        if (this->IsActivatable()) {
            if (pMsg != nullptr) {
                EventArgs newMsg = *pMsg;
                newMsg.eventData = pMsg->eventType;
                newMsg.eventType = kEventNone;
                this->SendEvent(kEventClick, newMsg);
            }
            else {
                this->SendEvent(kEventClick);
            }            
        }
    }

    /** 鼠标左键按下事件：触发选择子项事件
    */
    virtual bool ButtonDown(const EventArgs& msg) override
    {
        if (this->IsEnabled() && this->IsActivatable() && this->IsPointInWithScrollOffset(msg.ptMouse)) {
            uint64_t vkFlag = kVkLButton;
            if (this->IsKeyDown(msg, ModifierKey::kControl)) {
                vkFlag |= kVkControl;
            }
            if (this->IsKeyDown(msg, ModifierKey::kShift)) {
                vkFlag |= kVkShift;
            }
            //左键按下的时候，选择
            SelectItem(vkFlag);
        }
        return BaseClass::ButtonDown(msg);
    }

    /** 鼠标右键按下事件：触发选择子项事件
    */
    virtual bool RButtonDown(const EventArgs& msg) override
    {
        if (this->IsEnabled() && this->IsActivatable() && this->IsPointInWithScrollOffset(msg.ptMouse)) {
            uint64_t vkFlag = kVkRButton;            
            if (this->IsKeyDown(msg, ModifierKey::kControl)) {
                vkFlag |= kVkControl;
            }
            if (this->IsKeyDown(msg, ModifierKey::kShift)) {
                vkFlag |= kVkShift;
            }
            //右键按下的时候，选择
            SelectItem(vkFlag);
        }
        return BaseClass::RButtonDown(msg);
    }

    /** 执行选择功能
    * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
    */
    void SelectItem(uint64_t vkFlag)
    {
        IListBoxOwner* pOwner = this->GetOwner();
        ASSERT(pOwner != nullptr);
        if (pOwner != nullptr) {
            size_t nListBoxIndex = this->GetListBoxIndex();
            pOwner->SelectItem(nListBoxIndex, true, true, vkFlag);
        }
    }
};

typedef ListCtrlItemTemplate<Box> ListCtrlItemBase;   //基类为：ListBoxItem
typedef ListCtrlItemTemplate<HBox> ListCtrlItemBaseH; //基类为：ListBoxItemH
typedef ListCtrlItemTemplate<VBox> ListCtrlItemBaseV; //基类为：ListBoxItemV

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_VIEW_H_
