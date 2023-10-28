#ifndef UI_CONTROL_LIST_CTRL_DATA_VIEW_H_
#define UI_CONTROL_LIST_CTRL_DATA_VIEW_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
//包含类：ListCtrlDataView / ListCtrlDataLayout

/** 列表数据显示和布局功能
*/
class ListCtrl;
class ListCtrlDataView : public VirtualListBox
{
    friend class ListCtrlDataLayout;
public:
    ListCtrlDataView();
    virtual ~ListCtrlDataView();

    /** 设置ListCtrl控件接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 获取列表控件的宽度（Header的各个列总宽度之和）
    */
    int32_t GetListCtrlWidth() const;

    /** 设置顶部元素的索引号
    */
    void SetTopElementIndex(size_t nTopElementIndex);

    /** 获取顶部元素的索引号
    */
    size_t GetTopElementIndex() const;

    /** 设置当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    void SetDisplayDataItems(const std::vector<size_t>& itemIndexList);

    /** 获取当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    void GetDisplayDataItems(std::vector<size_t>& itemIndexList) const;

    /** 判断一个数据项是否可见
    * @param [in] itemIndex 数据项的索引号
    */
    bool IsDataItemDisplay(size_t itemIndex) const;

    /** 确保数据索引项可见
    * @param [in] itemIndex 数据项的索引号
    * @param [in] bToTop 是否确保在最上方
    */
    bool EnsureDataItemVisible(size_t itemIndex, bool bToTop);

protected:
    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    /** 执行了刷新操作, 界面的UI控件个数可能会发生变化
    */
    virtual void OnRefresh() override;

    /** 执行了重排操作，界面的UI控件进行了重新数据填充（通过FillElement函数）
    */
    virtual void OnArrangeChild() override;

    /** 查找子控件
    */
    virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint scrollPos = UiPoint()) override;

private:
    /** ListCtrl 控件接口
    */
    ListCtrl* m_pListCtrl;

    /** 顶部元素的索引号(用于画网格线)
    */
    size_t m_nTopElementIndex;

    /** 当前可见的元素列表
    */
    std::vector<size_t> m_diplayItemIndexList;
};

/** 列表数据显示控件的布局管理接口
*/
class ListCtrlDataLayout : public Layout, public VirtualLayout
{
public:
    /** 调整内部所有控件的位置信息
        * @param [in] items 控件列表
        * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
        * @return 返回排列后最终盒子的宽度和高度信息
        */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

    /** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
        * @param[in] items 子控件列表
        * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
        * @return 返回排列后最终布局的大小信息（宽度和高度）；
                包含items中子控件的外边距，包含items中子控件的内边距；
                包含Box控件本身的内边距；
                不包含Box控件本身的外边距；
                返回值中不包含拉伸类型的子控件大小。
        */
    virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;

public:
    /** 延迟加载展示数据
        * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
        */
    virtual void LazyArrangeChild(UiRect rc) const override;

    /** 获取需要展示的真实数据项最大个数（即有Control对象对应的真实数据项）
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    virtual size_t AjustMaxItem(UiRect rc) const override;

    /** 得到可见范围内第一个元素的前一个元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回元素的索引
    */
    virtual size_t GetTopElementIndex(UiRect rc) const override;

    /** 判断某个元素是否在可见范围内
    * @param[in] iIndex 元素索引，范围是：[0, GetElementCount())
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @return 返回 true 表示可见，否则为不可见
    */
    virtual bool IsElementDisplay(UiRect rc, size_t iIndex) const override;

    /** 判断是否要重新布局
    */
    virtual bool NeedReArrange() const override;

    /** 获取当前所有可见控件的数据元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[out] collection 索引列表，范围是：[0, GetElementCount())
    */
    virtual void GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const override;

    /** 让控件在可见范围内
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[in] iIndex 元素索引号，范围是：[0, GetElementCount())
    * @param[in] bToTop 是否在最上方
    */
    virtual void EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const override;

private:
    /** 获取关联的Box接口
    */
    ListCtrlDataView* GetOwnerBox() const;

    /** 获取数据项的高度, 高度不包含表头
    * @param [in] nCount 数据项个数，如果为Box::InvalidIndex，则获取所有数据项的高度总和
    * @return 返回 nCount 个数据项的高度总和, 不包含表头的高度
    */
    int64_t GetElementsHeight(size_t nCount) const;

    /** 获取数据项的高度和宽度
    * @param [in] rcWidth 当前容器的宽度, 外部调用时，需要先剪去内边距
    * @param [in] nElementIndex 数据元素的索引号
    * @return 返回数据元素的高度和宽度
    */
    UiSize GetElementSize(int32_t rcWidth, size_t nElementIndex) const;

    /** 计算列数
    */
    int32_t CalcTileColumns() const;

    /** 获取行宽
    */
    int32_t GetItemWidth() const;

    /** 获取行高(目前仅支持所有行的行高都相等的情况)
    */
    int32_t GetItemHeight() const;

    /** 获取表头控件的高度
    */
    int32_t GetHeaderHeight() const;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_VIEW_H_
