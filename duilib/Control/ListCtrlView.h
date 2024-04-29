#ifndef UI_CONTROL_LIST_CTRL_VIEW_H_
#define UI_CONTROL_LIST_CTRL_VIEW_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
/** ListCtrl列表视图UI控件的基类（实现子项选择功能，包括方向键，快捷键，鼠标选择逻辑等）
*   使用这个类的时候，需要用 ListCtrlItemTemplate 模板来形成子类作为ListBoxItem
*/
class ListCtrlView : public VirtualListBox
{
public:
    explicit ListCtrlView(Layout* pLayout);
    virtual ~ListCtrlView();

    virtual std::wstring GetType() const override { return L"ListCtrlView"; }
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue);
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
    virtual void SendEvent(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0, TCHAR tChar = 0,
                           const UiPoint& mousePos = UiPoint()) override;
    virtual void SendEvent(const EventArgs& event) override;

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

public:
    /** 设置普通列表项（非Header、非置顶）的top坐标
    */
    void SetNormalItemTop(int32_t nNormalItemTop);

    /** 获取普通列表项（非Header、非置顶）的top坐标
    */
    int32_t GetNormalItemTop() const;

protected:
    /** 控件初始化
    */
    virtual void OnInit() override;

    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    /** 挂载ListBoxItem的鼠标事件，以实现鼠标框选操作
    */
    virtual void AttachMouseEvents(Control* pListBoxItem);

    /** 选择状态发生变化
    */
    virtual void OnSelectStatusChanged();

    /** 获取滚动视图的滚动幅度
    */
    virtual void GetScrollDeltaValue(int32_t& nHScrollValue, int32_t& nVScrollValue) const;

    /** 执行了鼠标框选操作
    * @param [in] left 框选的X坐标left值
    * @param [in] right 框选的X坐标right值
    * @param [in] top 框选的Y坐标top值
    * @param [in] bottom 框选的Y坐标bottom值
    * @return 如果有选择变化返回true，否则返回false
    */
    virtual bool OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom);

    /** 绘制鼠标框选的边框和填充颜色
    */
    void PaintFrameSelection(IRender* pRender);

protected:
    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点

private:
    void OnButtonDown(const UiPoint& ptMouse, Control* pSender);
    void OnButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonDown(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnMouseMove(const UiPoint& ptMouse, Control* pSender);
    void OnWindowKillFocus();

private:
    /** 在视图空白处点击了鼠标左键/右键
    */
    bool OnListCtrlClickedBlank();

    /** 检查是否需要滚动视图
    */
    void OnCheckScrollView();

    /** 横向布局，计算行数
    */
    int32_t CalcRows() const;

    /** 纵向布局，计算列数
    */
    int32_t CalcColumns() const;

    /** 计算一个元素的矩形区域
    */
    void CalcElementRectV(size_t nElemenetIndex, const UiSize& szItem, 
                          int32_t nColumns, int32_t childMarginX, int32_t childMarginY,
                          int64_t& iLeft, int64_t& iTop, int64_t& iRight, int64_t& iBottom) const;

    /** 计算一个元素的矩形区域
    */
    void CalcElementRectH(size_t nElemenetIndex, const UiSize& szItem, 
                          int32_t nRows, int32_t childMarginX, int32_t childMarginY,
                          int64_t& iLeft, int64_t& iTop, int64_t& iRight, int64_t& iBottom) const;


    /** 响应KeyDown消息
    * @return 返回true表示成功处理，返回false表示未处理此消息
    */
    bool OnListCtrlKeyDown(const EventArgs& msg);

private:
    /** 是否鼠标在视图中按下左键或者右键
    */
    bool m_bMouseDownInView;

    /** 是否鼠标左键按下
    */
    bool m_bMouseDown;

    /** 是否鼠标右键按下
    */
    bool m_bRMouseDown;

    /** 是否处于鼠标滑动操作中
    */
    bool m_bInMouseMove;

    /** 鼠标按下时的鼠标位置
    */
    UiSize64 m_ptMouseDown;

    /** 鼠标滑动时的鼠标位置
    */
    UiSize64 m_ptMouseMove;

    /** 鼠标按下时的控件接口
    */
    Control* m_pMouseSender;

    /** 定时器滚动视图时的取消机制
    */
    nbase::WeakCallbackFlag m_scrollViewFlag;

    /** 鼠标框选功能的设置
    */
    bool m_bEnableFrameSelection; //是否支持鼠标框选功能
    UiString m_frameSelectionColor; //框选填充颜色
    uint8_t m_frameSelectionAlpha;  //框选填充颜色的Alpha值
    UiString m_frameSelectionBorderColor; //框选边框颜色
    uint8_t m_frameSelectionBorderSize; //框选边框大小

private:
    /** 没按Shift键时的最后一次选中项，有效范围：[0, GetElementCount())
    */
    size_t m_nLastNoShiftIndex;

private:
    /** 普通列表项（非Header、非置顶）的top坐标
    */
    int32_t m_nNormalItemTop;
};

/** ListCtrl各个视图中数据项的基类模板
*/
template<typename InheritType>
class UILIB_API ListCtrlItemTemplate : public ListBoxItemTemplate<InheritType>
{
public:
    ListCtrlItemTemplate() {}
    virtual ~ListCtrlItemTemplate() {}

    /** 设置控件是否选择状态
  * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
  * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
  */
    virtual void Selected(bool bSelect, bool bTriggerEvent) override
    {
        if (__super::IsSelected() != bSelect) {
            __super::Selected(bSelect, bTriggerEvent);
        }
    }

protected:
    /** 激活函数
    */
    virtual void Activate() override
    {
        //重写基类的实现逻辑，这里只发出一个Click事件
        if (this->IsActivatable()) {
            this->SendEvent(kEventClick);
        }
    }

    /** 鼠标左键按下事件：触发选择子项事件
    */
    virtual bool ButtonDown(const EventArgs& msg) override
    {
        if (this->IsEnabled() && this->IsActivatable() && this->IsPointInWithScrollOffset(msg.ptMouse)) {
            uint64_t vkFlag = kVkLButton;
#ifdef UILIB_IMPL_WINSDK
            if (msg.wParam & MK_CONTROL) {
                vkFlag |= kVkControl;
            }
            if (msg.wParam & MK_SHIFT) {
                vkFlag |= kVkShift;
            }
#endif
            //左键按下的时候，选择
            SelectItem(vkFlag);
        }
        return __super::ButtonDown(msg);
    }

    /** 鼠标右键按下事件：触发选择子项事件
    */
    virtual bool RButtonDown(const EventArgs& msg) override
    {
        if (this->IsEnabled() && this->IsActivatable() && this->IsPointInWithScrollOffset(msg.ptMouse)) {
            uint64_t vkFlag = kVkRButton;
#ifdef UILIB_IMPL_WINSDK
            if (msg.wParam & MK_CONTROL) {
                vkFlag |= kVkControl;
            }
            if (msg.wParam & MK_SHIFT) {
                vkFlag |= kVkShift;
            }
#endif
            //右键按下的时候，选择
            SelectItem(vkFlag);
        }
        return __super::RButtonDown(msg);
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
