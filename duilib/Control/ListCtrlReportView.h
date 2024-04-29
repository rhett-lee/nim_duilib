#ifndef UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_
#define UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_

#pragma once

#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
//包含类：ListCtrlReportView / ListCtrlReportLayout

/** 列表数据显示和布局功能(Report类型)
*/
class ListCtrl;
class ListCtrlData;
struct ListCtrlItemData;
class ListCtrlReportView : public ListCtrlView, public IListCtrlView
{
    friend class ListCtrlReportLayout;
public:
    ListCtrlReportView();
    virtual ~ListCtrlReportView();

    virtual std::wstring GetType() const override { return L"ListCtrlReportView"; }
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 设置ListCtrl控件接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 设置数据代理对象
    * @param[in] pProvider 数据代理对象
    */
    virtual void SetDataProvider(VirtualListBoxElement* pProvider) override;

    /** 刷新列表
    */
    virtual void Refresh() override;

    /** 获取列表控件的宽度（Header的各个列总宽度之和）
    */
    int32_t GetListCtrlWidth() const;

    /** 设置顶部元素的索引号
    */
    void SetTopElementIndex(size_t nTopElementIndex);

    /** 获取顶部元素的索引号
    */
    virtual size_t GetTopElementIndex() const override;

    /** 设置当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    void SetDisplayDataItems(const std::vector<size_t>& itemIndexList);

    /** 获取当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表
    */
    virtual void GetDisplayDataItems(std::vector<size_t>& itemIndexList) const override;

    /** 判断一个数据项是否可见
    * @param [in] itemIndex 数据项的索引号
    */
    virtual bool IsDataItemDisplay(size_t itemIndex) const override;

    /** 确保数据索引项可见
    * @param [in] itemIndex 数据项的索引号
    * @param [in] bToTop 是否确保在最上方
    */
    virtual bool EnsureDataItemVisible(size_t itemIndex, bool bToTop) override;

public:
    /** 得到可见范围内第一个元素的索引
    * @param [in] nScrollPosY 当前Y滚动条的位置
    * @return 返回元素的索引
    */
    size_t GetTopDataItemIndex(int64_t nScrollPosY) const;

    /** 获取指定元素的高度（行高）
    * @param [in] itemIndex 数据项的索引号
    */
    int32_t GetDataItemHeight(size_t itemIndex) const;

    struct ShowItemInfo
    {
        size_t nItemIndex;      //元素索引
        int32_t nItemHeight;    //元素的高度
    };

    /** 获取需要展示的数据：置顶的优先，并且按顺序
    @param [in] nScrollPosY 当前Y滚动条的位置
    @param [in] maxCount 最多取多少条记录(包含置顶和非置顶的)
    @param [out] itemIndexList 返回需要展示的元素序号(非置顶的)
    @param [out] atTopItemIndexList 返回需要展示的元素序号(置顶的)
    @param [out] nPrevItemHeights 第一条可见元素之前所有元素的总高度（不含置顶元素）
    */
    void GetDataItemsToShow(int64_t nScrollPosY, size_t maxCount, 
                            std::vector<ShowItemInfo>& itemIndexList,
                            std::vector<ShowItemInfo>& atTopItemIndexList,
                            int64_t& nPrevItemHeights) const;

    /** 获取指定高度的区域，最多可以展示多少条数据
    @param [in] nScrollPosY 当前Y滚动条的位置
    @param [in] nRectHeight 区域高度
    @param [out] pItemIndexList 返回可以显示的元素序号
    @param [out] pAtTopItemIndexList 置顶项的元素序号
    @return 返回可以展示的数据条数
    */
    int32_t GetMaxDataItemsToShow(int64_t nScrollPosY, int32_t nRectHeight, 
                                  std::vector<size_t>* pItemIndexList = nullptr,
                                  std::vector<size_t>* pAtTopItemIndexList = nullptr) const;

    /** 获取指定元素的显示位置总高度值
    * @param [in] itemIndex 数据项的索引号
    * @parma [in] bIncludeAtTops 是否包含置顶元素的高度值
    * @return 显示位置总高度值，不包含该元素自身
    */
    int64_t GetDataItemTotalHeights(size_t itemIndex, bool bIncludeAtTops) const;

public:
    /** 是否为标准模式（行高都为默认行高，无隐藏行，无置顶行）
    */
    bool IsNormalMode() const;

    /** 设置置顶的UI控件索引号
    */
    void SetAtTopControlIndex(const std::vector<size_t>& atTopControlList);

    /** 调整UI控件个数，以确保足够显示出应显示的数据
    */
    void AjustItemCount();

public:
    /** 横向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetRowGridLineWidth() const;

    /** 横向网格线的颜色
    * @param [in] color 横向网格线的颜色
    */
    void SetRowGridLineColor(const std::wstring& color);
    std::wstring GetRowGridLineColor() const;

    /** 纵向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetColumnGridLineWidth() const;

    /** 纵向网格线的颜色
    * @param [in] color 纵向网格线的颜色
    */
    void SetColumnGridLineColor(const std::wstring& color);
    std::wstring GetColumnGridLineColor() const;

    /** 调整列的宽度: 数组中第1个值是列的序号，第2个值列宽度
    */
    void AdjustSubItemWidth(const std::map<size_t, int32_t>& subItemWidths);

protected:
    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    /** 查找子控件
    */
    virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pProcData, uint32_t uFlags, 
                                 const UiPoint& ptMouse = UiPoint(),
                                 const UiPoint& scrollPos = UiPoint()) override;

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

    /** 计算本页里面显示几个子项
    * @param [in] bIsHorizontal 当前布局是否为水平布局
    * @param [out] nColumns 返回列数
    * @param [out] nRows 返回行数
    * @return 返回可视区域显示的记录数
    */
    virtual size_t GetDisplayItemCount(bool bIsHorizontal, size_t& nColumns, size_t& nRows) const override;

    /** 判断一个数据元素是否为可选择项
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    */
    virtual bool IsSelectableElement(size_t nElementIndex) const override;

    /** 获取下一个可选择的数据元素
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    * @param [in] bForward true表示向前查找，false表示向后查找
    */
    virtual size_t FindSelectableElement(size_t nElementIndex, bool bForward) const override;

    /** 子项的选择状态变化事件，用于状态同步
    * @param [in] iIndex 子项目的ID，范围是：[0, GetItemCount())
    * @param [in] pListBoxItem 关联的列表项接口
    */
    virtual void OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem) override;

    /** 子项的勾选状态变化事件，用于状态同步
    * @param [in] iIndex 子项目的ID，范围是：[0, GetItemCount())
    * @param [in] pListBoxItem 关联的列表项接口
    */
    virtual void OnItemCheckedChanged(size_t iIndex, IListBoxItem* pListBoxItem) override;

    /** 选择状态发生变化
    */
    virtual void OnSelectStatusChanged() override;

    /** 获取滚动视图的滚动幅度
    */
    virtual void GetScrollDeltaValue(int32_t& nHScrollValue, int32_t& nVScrollValue) const override;

    /** 执行了鼠标框选操作
    * @param [in] left 框选的X坐标left值
    * @param [in] right 框选的X坐标right值
    * @param [in] top 框选的Y坐标top值
    * @param [in] bottom 框选的Y坐标bottom值
    * @return 如果有选择变化返回true，否则返回false
    */
    virtual bool OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom) override;

    /** 某个数据项的Check勾选状态变化(列级)
    * @param [in] nElementIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列ID
    * @param [in] bChecked 是否勾选
    */
    void OnSubItemColumnChecked(size_t nElementIndex, size_t nColumnId, bool bChecked);

private:
    /** 绘制网格线
    */
    void PaintGridLines(IRender* pRender);

    /** 将header和置顶项放在最后
    * @param [in,out] items 需要移动的控件列表
    * @param [out] atTopItems 返回置顶的控件列表
    */
    void MoveTopItemsToLast(std::vector<Control*>& items, std::vector<Control*>& atTopItems) const;

private:
    /** ListCtrl 控件接口
    */
    ListCtrl* m_pListCtrl;

    /** 数据接口
    */
    ListCtrlData* m_pData;

    /** 顶部元素的索引号(用于画网格线)
    */
    size_t m_nTopElementIndex;

    /** 当前可见的元素列表
    */
    std::vector<size_t> m_diplayItemIndexList;

    /** 置顶的UI控件索引号
    */
    std::vector<size_t> m_atTopControlList;

private:
    /** 横向网格线的宽度
    */
    int32_t m_nRowGridLineWidth;

    /** 横向网格线的颜色
    */
    UiString m_rowGridLineColor;

    /** 纵向网格线的宽度
    */
    int32_t m_nColumnGridLineWidth;

    /** 纵向网格线的颜色
    */
    UiString m_columnGridLineColor;
};

/** 列表数据显示控件的布局管理接口
*/
class ListCtrlReportLayout : public Layout, public VirtualLayout
{
public:
    ListCtrlReportLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const { return LayoutType::ListCtrlReportLayout; }

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

public:
    /** 设置关联的Box接口
    */
    void SetDataView(ListCtrlReportView* pDataView);

private:
    /** 获取关联的Box接口
    */
    ListCtrlReportView* GetDataView() const { return m_pDataView; }

    /** 获取数据项的高度, 高度不包含表头
    * @param [in] nCount 数据项个数，如果为Box::InvalidIndex，则获取所有数据项的高度总和
    * @parma [in] bIncludeAtTops 是否包含置顶元素的高度值
    * @return 返回 nCount 个数据项的高度总和, 不包含表头的高度
    */
    int64_t GetElementsHeight(size_t nCount, bool bIncludeAtTops) const;

    /** 获取数据项的高度和宽度
    * @param [in] rcWidth 当前容器的宽度, 外部调用时，需要先剪去内边距
    * @param [in] nElementIndex 数据元素的索引号
    * @return 返回数据元素的高度和宽度
    */
    UiSize GetElementSize(int32_t rcWidth, size_t nElementIndex) const;

    /** 获取行宽
    */
    int32_t GetItemWidth() const;

    /** 获取行高(目前仅支持所有行的行高都相等的情况)
    */
    int32_t GetItemHeight() const;

    /** 获取表头控件的高度
    */
    int32_t GetHeaderHeight() const;

    /** 延迟加载展示数据(常规模式，行高相同、无隐藏项、无置顶项)
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    void LazyArrangeChildNormal(UiRect rc) const;

    /** 获取当前所有可见控件的数据元素索引
    * @param [in] rc 当前显示区域的矩形，不包含内边距
    * @param[out] collection 索引列表，范围是：[0, GetElementCount())
    */
    void GetDisplayElements(UiRect rc, std::vector<size_t>& collection, 
                            std::vector<size_t>* pAtTopItemIndexList) const;

private:
    /** 关联的ListBox接口
    */
    ListCtrlReportView* m_pDataView;

    /** 底部预留的空间，确保滚动到最底部的时候，最后一条数据容易看完整
    */
    int32_t m_nReserveHeight;

    /** 是否设置了底部预留空间
    */
    bool m_bReserveSet;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_
