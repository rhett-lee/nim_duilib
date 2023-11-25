#ifndef UI_CONTROL_LIST_CTRL_ICON_VIEW_H_
#define UI_CONTROL_LIST_CTRL_ICON_VIEW_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表数据显示和布局功能(Icon类型)
*/
class ListCtrl;
class ListCtrlData;
class ListCtrlIconView : public VirtualListBox, public IListCtrlView
{
public:
    /** 设置是否为List模式
    * @param [in] bListMode true表示列表视图，否则为图标视图
    */
    explicit ListCtrlIconView(bool bListMode);
    virtual ~ListCtrlIconView();

    virtual std::wstring GetType() const override { return L"ListCtrlIconView"; }
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue);
    virtual void HandleEvent(const EventArgs& msg) override;

public:
    /** 设置ListCtrl控件接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 设置数据代理对象
    * @param[in] pProvider 数据代理对象
    */
    virtual void SetDataProvider(VirtualListBoxElement* pProvider) override;

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
    /** 设置布局方向（默认情况下，Icon视图是纵向布局，List视图是横向布局）
    * @param [in] bHorizontal true表示横向布局，false表示纵向布局
    */
    void SetHorizontalLayout(bool bHorizontal);

    /** 判断当前布局方向是否为横向布局
    */
    bool IsHorizontalLayout() const;

public:
    /** 获取顶部元素的索引号
    */
    size_t GetTopElementIndex() const;

    /** 判断一个数据项是否可见
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool IsDataItemDisplay(size_t itemIndex) const;

    /** 获取当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    void GetDisplayDataItems(std::vector<size_t>& itemIndexList) const;

    /** 确保数据索引项可见
    * @param [in] itemIndex 数据项的索引号
    * @param [in] bToTop 是否确保在最上方
    */
    bool EnsureDataItemVisible(size_t itemIndex, bool bToTop);

protected:
    /** 控件初始化
    */
    virtual void DoInit() override;

       /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateDataItem() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] itemData 数据项（代表行的属性）
    * @param [in] subItemList 数据子项（代表每一列的数据, 第1个是列的ID，第2个是列的数据）
    */
    virtual bool FillDataItem(Control* pControl,
                              size_t nElementIndex,
                              const ListCtrlItemData& itemData,
                              const std::vector<ListCtrlSubItemData2Pair>& subItemList) override;


    /** 获取某列的宽度最大值
    * @param [in] subItemList 数据子项（代表每一列的数据）
    * @return 返回该列宽度的最大值，返回的是DPI自适应后的值； 如果失败返回-1
    */
    virtual int32_t GetMaxDataItemWidth(const std::vector<ListCtrlSubItemData2Ptr>& subItemList) override;

protected:
    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点

    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

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

    /** 执行了鼠标框选操作
    * @param [in] left 框选的X坐标left值
    * @param [in] right 框选的X坐标right值
    * @param [in] top 框选的Y坐标top值
    * @param [in] bottom 框选的Y坐标bottom值
    * @return 如果有选择变化返回true，否则返回false
    */
    bool OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom);

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

    /** 绘制鼠标框选的边框和填充颜色
    */
    void PaintFrameSelection(IRender* pRender);

    /** 获取滚动视图的滚动幅度
    */
    void GetScrollDeltaValue(int32_t& nHScrollValue, int32_t& nVScrollValue) const;

    /** 响应KeyDown消息
    * @return 返回true表示成功处理，返回false表示未处理此消息
    */
    bool OnListCtrlKeyDown(const EventArgs& msg);

private:
    /** ListCtrl 控件接口
    */
    ListCtrl* m_pListCtrl;

    /** 数据接口
    */
    ListCtrlData* m_pData;

    /** 是否为List模式: true表示列表视图，否则为图标视图 
    */
    const bool m_bListMode;

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
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ICON_VIEW_H_
