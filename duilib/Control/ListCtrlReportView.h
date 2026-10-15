#ifndef UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_
#define UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_

#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
//包含类：ListCtrlReportView / ListCtrlReportLayout

/** 列表数据显示和布局功能(Report类型)
 *  ReportView视图的数据基本结构:
 *  <ListCtrlReportView>
 *      <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/> </ListCtrlItem>
 *      ...
 *      <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/> </ListCtrlItem>
 *  </ListCtrlReportView>
 */
class ListCtrl;
class ListCtrlData;
class ListCtrlSubItem;
class ListCtrlItem;
struct ListCtrlItemData;
class DUILIB_API ListCtrlReportView : public ListCtrlView, public IListCtrlView
{
    typedef ListCtrlView BaseClass;
    friend class ListCtrlReportLayout;
public:
    explicit ListCtrlReportView(Window* pWindow);
    virtual ~ListCtrlReportView() override;

    virtual DString GetType() const override { return _T("ListCtrlReportView"); }
    virtual void HandleEvent(const EventArgs& msg) override;

    /** 鼠标事件(来源于子控件的消息转发)：单元格选择模式下拦截，用于实现单元格的选择与区域扩展
    * @param [in] msg 鼠标事件的内容
    * @return 返回true表示已按单元格选择处理，不再触发行选择
    */
    virtual bool OnListBoxItemMouseEvent(const EventArgs& msg) override;

    /** 设置ListCtrl控件接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 设置数据代理对象
    * @param[in] pProvider 数据代理对象
    */
    virtual void SetDataProvider(VirtualListBoxElement* pProvider) override;

    /** 刷新界面，保持数据与显示同步
    * @param [in] bSync true表示同步刷新，立即重绘当前视图; false表示异步刷新，刷新动作为异步重绘，标记为脏区域，交给系统重绘界面
    */
    virtual void Refresh(bool bSync = false) override;

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

    /** 监听Report视图数据项UI元素填充事件（虚表，填充某行数据）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlItem*指针，数据按行填充
     */
    void AttachReportViewItemFilled(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventReportViewItemFilled, callback, callbackID); }

    /** 监听Report视图数据项UI元素填充事件（虚表，填充某行某列数据）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlSubItem*指针，代表某行某列数据的UI元素控件
     *               通过该接口，可以获取数据项索引号、哪一列的数据
     */
    void AttachReportViewSubItemFilled(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventReportViewSubItemFilled, callback, callbackID); }

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
    * @param [in] fLineWidth 网格线的宽度，如果为0表示不显示横向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetRowGridLineWidth(float fLineWidth, bool bNeedDpiScale);
    float GetRowGridLineWidth() const;

    /** 横向网格线的颜色
    * @param [in] color 横向网格线的颜色
    */
    void SetRowGridLineColor(const DString& color);
    DString GetRowGridLineColor() const;

    /** 纵向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(float fLineWidth, bool bNeedDpiScale);
    float GetColumnGridLineWidth() const;

    /** 纵向网格线的颜色
    * @param [in] color 纵向网格线的颜色
    */
    void SetColumnGridLineColor(const DString& color);
    DString GetColumnGridLineColor() const;

    /** 调整列的宽度: 数组中第1个值是列的序号，第2个值列宽度
    */
    void AdjustSubItemWidth(const std::map<size_t, int32_t>& subItemWidths);

    /** 刷新指定单元格的数据显示（单格刷新）
    * @param [in] nElementIndex 数据元素的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID
    * @return 返回true表示更新成功，返回false表示参数无效，或者该单元格当前不在可见范围内
    */
    bool RefreshSubItemData(size_t nElementIndex, size_t nColumnId);

protected:
    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    /** 查找子控件
    */
    virtual Control* FindControl(FINDCONTROLPROC Proc, void* pProcData, uint32_t uFlags,
                                 const UiPoint& ptMouse = UiPoint(),
                                 const UiPoint& scrollPos = UiPoint()) override;

    /** 获取当前滚动条偏移量（重写 ScrollBox::GetScrollOffset）
    *  冻结列命中遍历期间（m_bFrozenHitTestPass 为 true 时）横向分量按0计：
    *  冻结列固定显示在视口左侧、不随横向滚动移动，其坐标空间横向偏移为0，
    *  使子控件的 GetScrollOffsetInScrollBox() 与冻结列的显示位置口径一致
    * @return 返回滚动条偏移量
    */
    virtual UiSize GetScrollOffset() const override;

    /** 获取某个子控件（含子孙控件）所属坐标空间的滚动偏移量（重写 ScrollBox::GetScrollOffsetForChild）
    *  冻结列区域内的子控件（数据行的单元格、表头的单元格及其子孙控件）固定显示在视口左侧、
    *  不随横向滚动移动，其横向偏移量按0计；其余子控件（如整行的 ListCtrlItem）保持默认口径
    * @param [in] pChild 待换算坐标的子控件（可能是子孙控件）
    * @return 返回该子控件所在坐标空间对应的滚动偏移量
    */
    virtual UiSize GetScrollOffsetForChild(const PlaceHolder* pChild) const override;

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
    virtual int32_t GetMaxDataItemWidth(const std::vector<const ListCtrlSubItemData2*>& subItemList) override;

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

    /** 当前是否允许绘制"行选中"背景色（行选中背景色为行选择模式专属视觉）
        单元格选择模式下返回false：这样无论行选择状态由哪个入口（鼠标/键盘/API）改动，
        界面都不会显示出"行被选中"的视觉表现，从而在单元格选择模式下看不出行可选状态
    */
    virtual bool CanPaintSelectedColors(bool bHasStateImages) const override;

    /** 在视图空白处点击了鼠标左键
    * @return 如果有选择变化返回true，否则返回false
    */
    virtual bool OnLButtonClickedBlank() override;

    /** 在视图空白处点击了鼠标右键
    * @return 如果有选择变化返回true，否则返回false
    */
    virtual bool OnRButtonClickedBlank() override;

    /** 某个数据项的Check勾选状态变化(列级)
    * @param [in] nElementIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列ID
    * @param [in] bChecked 是否勾选
    */
    void OnSubItemColumnChecked(size_t nElementIndex, size_t nColumnId, bool bChecked);

    /** 判断某个子控件（含子孙控件）是否属于"冻结列"区域
    *  判定方式：向上查找该控件所属的单元格（数据行的单元格为 ListCtrlSubItem，表头单元格为 ListCtrlHeaderItem），
    *  再用单元格的列ID换取列索引号，与冻结列个数比较；整行的 ListCtrlItem 等无归属单元格的控件返回false
    * @param [in] pChild 待判断的子控件（可能是子孙控件）
    * @return 返回true表示该控件属于冻结列区域，否则返回false
    */
    bool IsChildInFrozenColumn(const PlaceHolder* pChild) const;

private:
    /** 绘制网格线
    */
    void PaintGridLines(IRender* pRender);

    /** 单元格选择模式下的鼠标框选：把框选矩形内的单元格置为选中
    *  说明：框选矩形由 ListBoxHelper 传入（视图客户坐标，与行模式同一口径）；
    *        Ctrl+拖动（m_bCellDragAdditive）为追加语义，只增不减
    * @param [in] left 框选矩形的X坐标left值
    * @param [in] right 框选矩形的X坐标right值
    * @param [in] top 框选矩形的Y坐标top值
    * @param [in] bottom 框选矩形的Y坐标bottom值
    * @return 如果有选择变化返回true，否则返回false
    */
    bool OnFrameSelectionForCells(int64_t left, int64_t right, int64_t top, int64_t bottom);

    /** 命中测试：将鼠标位置（窗口客户坐标）映射到单元格（行显示索引 + 列ID）
    * @param [in] ptMouse 鼠标位置（窗口客户坐标）
    * @param [out] nRowIndex 命中单元格所在行的显示索引；未命中时为 Box::InvalidIndex
    * @param [out] nColumnId 命中单元格的列ID；未命中时为 Box::InvalidIndex
    * @param [out] ppItem 命中单元格所在的数据项（ListCtrlItem）指针；未命中时为 nullptr
    * @return 返回是否命中某个单元格
    */
    bool HitTestCell(const UiPoint& ptMouse, size_t& nRowIndex, size_t& nColumnId, ListCtrlItem** ppItem = nullptr) const;

    /** 把鼠标位置换算为"子项控件坐标"（与 ListCtrlSubItem::GetRect() 同一坐标系）
    *  横向按冻结列口径换算：鼠标落在冻结区内时，冻结列不随横向滚动移动，横向滚动量按 0 计
    *  （与绘制趟、FindControl 的冻结列处理保持一致，避免横向滚动后命中的列与视觉错位）
    * @param [in] ptMouse 鼠标位置（窗口客户坐标）
    * @param [in] scrollOffsetInBox 滚动框内的滚动偏移量（与 ListCtrlItem::GetSubItem 取值一致）
    * @return 返回换算后的坐标
    */
    UiPoint MouseToCellSpace(const UiPoint& ptMouse, const UiPoint& scrollOffsetInBox) const;

    /** 判断鼠标点是否落在"置顶行"（不可选行）范围内
    *  置顶行固定显示在顶部，会遮挡其下方滚动的普通行；鼠标落在其上时不应穿透选择下方的单元格
    * @param [in] ptMouse 鼠标位置（窗口客户坐标）
    * @return 返回是否落在置顶行范围内
    */
    bool IsPointOnAtTopItem(const UiPoint& ptMouse) const;

    /** 命中"置顶行"（不可选行）所在的列表项
    * @param [in] ptMouse 鼠标位置（窗口客户坐标）
    * @return 返回鼠标点命中的置顶行列表项；未命中时为 nullptr
    */
    ListCtrlItem* HitTestTopItem(const UiPoint& ptMouse) const;

    /** 判断鼠标点（窗口客户坐标）是否落在"列表项自身的CheckBox图标"上
    *  列表项指数据行（ListCtrlItem）与表头（ListCtrlHeader），其CheckBox由控件自身绘制；
    *  用于单元格选择模式下判断是否需要放行鼠标事件，让Item自行完成勾选/取消勾选
    * @param [in] pSender 鼠标事件所属的控件（即 EventArgs::GetSender()）
    * @param [in] ptMouse 鼠标位置（窗口客户坐标）
    * @return 返回true表示鼠标点落在列表项自身的CheckBox图标上
    */
    bool IsMouseOnItemCheckBox(const Control* pSender, const UiPoint& ptMouse) const;

    /** 计算"表头与置顶行"所占的矩形区域（这些行固定显示在顶部，会遮挡其下方滚动的普通行）
    * @param [out] rcTopFixed 返回表头与置顶行所占的矩形区域
    * @return 是否存在表头或置顶行
    */
    bool GetTopFixedItemsRect(UiRect& rcTopFixed) const;

    /** 把某个单元格的数据填充到UI控件（FillDataItem与RefreshSubItemData共用，保证填充逻辑一致）
    * @param [in] pSubItem 目标子项控件
    * @param [in] pStorage 单元格的数据，可以为空
    * @param [in] defaultSubItem 默认属性子项控件
    * @param [in] nColumnId 列的ID
    * @param [in] nElementIndex 数据元素的索引号
    */
    void FillSubItemData(ListCtrlSubItem* pSubItem,
                         const ListCtrlSubItemData2* pStorage,
                         const ListCtrlSubItem& defaultSubItem,
                         size_t nColumnId,
                         size_t nElementIndex);

    /** 列布局表：描述各列的 X 区间（内容坐标，以视口左边为原点）
    *  由"首个可见数据行的子项控件几何"推导；无可见数据行时回退到表头几何。
    *  这是"某一列 X 区间"的唯一来源：冻结区宽度、以及后续的单元格选择 / 合并 /
    *  多级表头等能力均由此推导，禁止在绘制、命中、网格线各处自行累加列宽。
    *  说明：本表按"可见顺序"记录列，坐标为内容坐标（不扣滚动偏移）。
    */
    struct ListCtrlColumnLayout
    {
        /** 单个列的位置信息 */
        struct Column
        {
            /** 列的ID */
            size_t nColumnId = Box::InvalidIndex;

            /** 列的序号（0起，按可见顺序） */
            size_t nColumnIndex = 0;

            /** 列左边界的X坐标（内容坐标，相对视口左边） */
            int32_t nXBegin = 0;

            /** 列右边界的X坐标（内容坐标，相对视口左边） */
            int32_t nXEnd = 0;

            /** 列的宽度 */
            int32_t GetWidth() const { return nXEnd - nXBegin; }
        };

        /** 各列的信息（按列序排列） */
        std::vector<Column> m_columns;

        /** 获取列数 */
        size_t GetColumnCount() const { return m_columns.size(); }

        /** 获取前N列的总宽度（即这一组列的右边界X坐标，相对视口左边）
        * @param [in] nColumnCount 列数；如果超出实际列数，则按实际列数计算（与历史行为一致）
        * @return 返回宽度，如果无法计算则返回0
        */
        int32_t GetColumnsWidth(size_t nColumnCount) const
        {
            if (m_columns.empty() || (nColumnCount == 0)) {
                return 0;
            }
            const size_t nIndex = (nColumnCount < m_columns.size()) ? (nColumnCount - 1) : (m_columns.size() - 1);
            const int32_t nWidth = m_columns[nIndex].nXEnd;
            return (nWidth > 0) ? nWidth : 0;
        }
    };

    /** 单元格几何层：绘制 / 命中测试 / 网格线 / 冻结区 / 列X区间统一由此推导
    *  内容坐标：子控件 GetRect() 所在坐标系，随滚动偏移移动
    *  屏幕坐标：最终绘制 / 鼠标位置所在坐标系（不含 RenderOffset）
    *  冻结列不随横向滚动移动，所以其内容坐标与屏幕坐标同值
    *  （本结构由 A1 的 FrozenTransform 推广而来，冻结列是它的一个特例）
    */
    struct ListCtrlCellGeometry
    {
        /** 是否存在冻结列 */
        bool bHasFrozen = false;

        /** 冻结的列数 */
        size_t nFrozenCount = 0;

        /** 冻结区宽度（内容坐标与屏幕坐标同值） */
        int32_t nFrozenWidth = 0;

        /** 视口矩形（屏幕坐标，不包含内边距） */
        UiRect rcViewport;

        /** 当前滚动偏移 */
        UiSize scrollPos;

        /** 列布局表（各列X区间的唯一来源） */
        ListCtrlColumnLayout columnLayout;

        /** 冻结区的右边界（屏幕X坐标） */
        int32_t FrozenRight() const { return rcViewport.left + nFrozenWidth; }

        /** 屏幕X坐标是否落在冻结区内 */
        bool IsFrozenX(int32_t screenX) const { return bHasFrozen && (screenX < FrozenRight()); }

        /** 网格线：由"已扣除滚动偏移的内容X"推导屏幕X（冻结列不随横向滚动移动） */
        int32_t GridLineScreenX(int32_t x, bool bFrozenColumn) const
        {
            return bFrozenColumn ? (x + scrollPos.cx) : x;
        }

        /** 绘制趟：推导裁剪区（屏幕）+ 绘制区（屏幕）+ 窗口原点偏移
        * @param [in] bFrozenPass true表示绘制冻结列趟，false表示绘制可滚动区域趟
        * @param [out] rcClip 裁剪矩形（屏幕坐标）
        * @param [out] rcPaint 绘制矩形（屏幕坐标）
        * @param [out] ptOffset 窗口原点偏移
        */
        void GetPaintPass(bool bFrozenPass, UiRect& rcClip, UiRect& rcPaint, UiPoint& ptOffset) const
        {
            rcClip = rcViewport;
            rcPaint = rcViewport;
            if (bHasFrozen) {
                if (bFrozenPass) {
                    //冻结列趟：只保留视口左侧的冻结区
                    rcClip.right = FrozenRight();
                    rcPaint.right = FrozenRight();
                }
                else {
                    //可滚动区域趟：裁剪掉冻结区，绘制区左边界对齐到冻结区右侧（随横向滚动移动）
                    rcClip.left = FrozenRight();
                    rcPaint.Offset(scrollPos.cx, scrollPos.cy);
                    rcPaint.left = FrozenRight() + scrollPos.cx;
                }
            }
            else {
                rcPaint.Offset(scrollPos.cx, scrollPos.cy);
            }
            //冻结列不随横向滚动移动，所以横向的窗口原点偏移量为0
            ptOffset = bFrozenPass ? UiPoint(0, scrollPos.cy) : UiPoint(scrollPos.cx, scrollPos.cy);
        }
    };

    /** 构建单元格几何层（一次推导，绘制 / 命中测试 / 网格线复用）
    */
    ListCtrlCellGeometry BuildCellGeometry() const;

    /** 构建列布局表：由首个可见数据行的子项控件几何推导各列X区间；无可见数据行时回退到表头几何
    * @param [in] nViewLeft 视口左边的X坐标（内容坐标的原点）
    * @return 返回构建出的列布局表
    */
    ListCtrlColumnLayout BuildColumnLayout(int32_t nViewLeft) const;

    /** 绘制单元格选择的高亮（选中区域半透明填充 + 当前格焦点框）
    * @param [in] pRender 绘制接口
    * @param [in] cellGeometry 单元格几何层
    * @param [in] bFrozenPass 是否绘制冻结列趟
    */
    void PaintCellSelection(IRender* pRender, const ListCtrlCellGeometry& cellGeometry, bool bFrozenPass);

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
    float m_fRowGridLineWidth;

    /** 横向网格线的颜色
    */
    UiString m_rowGridLineColor;

    /** 纵向网格线的宽度
    */
    float m_fColumnGridLineWidth;

    /** 纵向网格线的颜色
    */
    UiString m_columnGridLineColor;

    /** 冻结列命中遍历标记：为true时，本控件对外的横向滚动量按0计（冻结列不随横向滚动移动）
    */
    bool m_bFrozenHitTestPass;
};

/** 列表数据显示控件的布局管理接口
*/
class DUILIB_API ListCtrlReportLayout : public Layout, public VirtualLayout
{
    typedef Layout BaseClass;
public:
    ListCtrlReportLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::ListCtrlReportLayout; }

    /** 按布局策略调整内部所有子控件的位置和大小
     * @param [in] items 子控件列表
     * @param [in] rc 当前容器位置与大小信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回排列后最终布局的宽度和高度信息，包含Box容器的内边距，但不包含Box容器本身的外边距(当容器支持滚动条时使用该返回值)
     */
    virtual UiSize64 ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly = false) override;

    /** 根据内部子控件大小估算容器布局大小（用于评估宽度或者高度为"auto"类型的控件大小，拉伸类型的子控件不计入大小估算）
     * @param [in] items 子控件列表
     * @param [in] szAvailable 容器的可用宽度和高度，包含分配给该容器的内边距，但不包含分配给容器的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度），包含Box容器本身的内边距，但不包含Box容器本身的外边距；
     */
    virtual UiSize64 EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable) override;

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

    /** 底部预留的空间，确保滚动到最底部的时候，最后一条数据容易看完整
    */
    int32_t GetReserveHeight() const;

private:
    /** 关联的ListBox接口
    */
    ListCtrlReportView* m_pDataView;

    /** 是否设置了底部预留空间
    */
    bool m_bReserveSet;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_REPORT_VIEW_H_
