#ifndef UI_CONTROL_LIST_CTRL_H_
#define UI_CONTROL_LIST_CTRL_H_

#pragma once

#include "duilib/Control/ListCtrlHeader.h"
#include "duilib/Control/ListCtrlHeaderItem.h"
#include "duilib/Control/ListCtrlDataProvider.h"
#include "duilib/Box/ListBoxItem.h"

namespace ui
{

/** 列表数据项UI控件（行）
*/
class ListCtrlItem;

/** 列表数据显示UI控件
*/
class ListCtrlDataView;

/** 列表数据管理容器
*/
class ListCtrlDataProvider;

/** 列的基本信息（用于添加列）
*/
struct ListCtrlColumn
{
    std::wstring text;              //表头的文本
    int32_t nColumnWidth = 100;     //列宽，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMin = 0;    //列宽最小值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nColumnWidthMax = 0;    //列宽最大值，0表示用默认设置，如果bNeedDpiScale为true，则执行DPI自适应处理
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    bool bSortable = true;          //是否支持排序
    bool bResizeable = true;        //是否支持通过拖动调整列宽
    bool bShowCheckBox = true;      //是否显示CheckBox（支持在表头和数据列显示CheckBox）
    int32_t nCheckBoxWidth = 24;    //CheckBox控件所占的宽度，用于设置文本偏移量，避免显示重叠，仅当bShowCheckBox为true时有效, 如果bNeedDpiScale为true，则执行DPI自适应处理
    bool bNeedDpiScale = true;      //是否对数值做DPI自适应
};

/** 列表数据项的基本信息（用于添加数据）
*/
struct ListCtrlDataItem
{
    size_t nColumnIndex = 0;        //【必填】第几列，有效范围：[0, GetColumnCount())
    std::wstring text;              //文本内容
    int32_t nTextFormat = -1;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageIndex = -1;       //图标资源索引号，在图片列表里面的下标值，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox
    int32_t nCheckBoxWidth = 24;    //CheckBox控件所占的宽度，仅当bShowCheckBox为true时有效, 如果bNeedDpiScale为true，则执行DPI自适应处理
    bool bNeedDpiScale = true;      //是否对数值做DPI自适应
};

/** ListCtrl控件
*/
class ListCtrl: public VBox
{
    friend class ListCtrlHeader;
    friend class ListCtrlDataProvider;
    friend class ListCtrlDataView;
public:
	ListCtrl();
	virtual ~ListCtrl();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
    /** 在指定位置添加一列
    * @param [in] columnIndex 在第几列以后插入该列，如果是-1，表示在最后追加一列
    * @param [in] columnInfo 列的基本属性
    * @return 返回这一列的表头控件接口
    */
    ListCtrlHeaderItem* InsertColumn(int32_t columnIndex, const ListCtrlColumn& columnInfo);

    /** 获取列的个数
    */
    size_t GetColumnCount() const;

    /** 获取列宽度
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    int32_t GetColumnWidth(size_t columnIndex) const;

    /** 获取列表头的控件接口
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    ListCtrlHeaderItem* GetColumn(size_t columnIndex) const;

    /** 获取列表头的控件接口
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColomnId()函数获取
    */
    ListCtrlHeaderItem* GetColumnById(size_t columnId) const;

    /** 获取列的索引序号
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColomnId()函数获取
    * @return 列的序号：[0, GetColumnCount())，代表第几列
    */
    size_t GetColumnIndex(size_t columnId) const;

    /** 获取列的ID
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    size_t GetColumnId(size_t columnIndex) const;

    /** 删除一列
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    bool DeleteColumn(size_t columnIndex);

    /** 获取表头控件接口, 在控件初始化以后才有值
    */
    ListCtrlHeader* GetListCtrlHeader() const;

    /** 设置是否支持列表头拖动改变列的顺序
    */
    void SetEnableHeaderDragOrder(bool bEnable);

    /** 是否支持列表头拖动改变列的顺序
    */
    bool IsEnableHeaderDragOrder() const;

    /** 设置是否显示表头控件
    */
    void SetHeaderVisible(bool bVisible);

    /** 当前是否显示表头控件
    */
    bool IsHeaderVisible() const;

    /** 设置表头控件的高度
    * @param[in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetHeaderHeight(int32_t nHeaderHeight, bool bNeedDpiScale);

    /** 获取表头控件的高度
    */
    int32_t GetHeaderHeight() const;

public:
    /** 设置数据项的高度
    * @param[in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetDataItemHeight(int32_t nItemHeight, bool bNeedDpiScale);

    /** 获取数据项的高度
    */
    int32_t GetDataItemHeight() const;

    /** 获取数据项总个数
    */
    size_t GetDataItemCount() const;

    /** 设置数据项总个数(对应行数)
    * @param [in] itemCount 数据项的总数，具体每个数据项的数据，通过回调的方式进行填充（内部为虚表实现）
    */
    bool SetDataItemCount(size_t itemCount);

    /** 在最后添加一个数据项
    * @param [in] dataItem 数据项的内容
    * @return 成功返回数据项的行索引号(rowIndex)，失败则返回Box::InvalidIndex
    */
    size_t AddDataItem(const ListCtrlDataItem& dataItem);

    /** 在指定行位置添加一个数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] dataItem 数据项的内容
    */
    bool InsertDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem);

    /** 设置指定行的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] dataItem 数据项的内容
    */
    bool SetDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem);

    /** 删除指定行的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool DeleteDataItem(size_t itemIndex);

    /** 删除所有行的数据项
    */
    bool DeleteAllDataItems();

    /** 设置数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 数据项关联的自定义数据
    */
    bool SetDataItemData(size_t itemIndex, size_t itemData);

    /** 获取数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的自定义数据
    */
    size_t GetDataItemData(size_t itemIndex) const;

    /** 设置指定数据项的文本
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] text 需要设置的文本内容
    */
    bool SetDataItemText(size_t itemIndex, size_t columnIndex, const std::wstring& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 数据项关联的文本内容
    */
    std::wstring GetDataItemText(size_t itemIndex, size_t columnIndex) const;

    /** 设置指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] textColor 需要设置的文本颜色
    */
    bool SetDataItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor);

    /** 获取指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] textColor 数据项关联的文本颜色
    */
    bool GetDataItemTextColor(size_t itemIndex, size_t columnIndex, UiColor& textColor) const;

    /** 设置指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bkColor 需要设置的背景颜色
    */
    bool SetDataItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor);

    /** 获取指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] bkColor 数据项关联的背景颜色
    */
    bool GetDataItemBkColor(size_t itemIndex, size_t columnIndex, UiColor& bkColor) const;

    /** 对数据排序
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 自定义的比较函数，如果为nullptr, 则使用默认的比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t columnIndex, bool bSortedUp, 
                       ListCtrlDataCompareFunc pfnCompareFunc = nullptr,
                       void* pUserData = nullptr);

    /** 设置外部自定义的排序函数, 替换默认的排序函数
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    void SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

public:
    /** 获取当前显示的数据项列表，顺序是从上到下
    * @param [in] itemIndexList 当前显示的数据项索引号列表, 每一元素的的有效范围：[0, GetDataItemCount())
    */
    void GetDisplayDataItems(std::vector<size_t>& itemIndexList) const;

    /** 得到可见范围内第一个数据项的索引号
    * @return 返回数据项的索引号， 有效范围：[0, GetDataItemCount())
    */
    size_t GetTopDataItem() const;

    /** 判断一个数据项是否可见
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool IsDataItemDisplay(size_t itemIndex) const;

    /** 确保数据索引项可见
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bToTop 是否确保在最上方
    */
    bool EnsureDataItemVisible(size_t itemIndex, bool bToTop);

protected:
    /** 控件初始化
    */
    virtual void DoInit() override;

    /** 设置ListCtrlHeader的属性Class
    */
    void SetHeaderClass(const std::wstring& className);

    /** ListCtrlHeaderItem的属性Class
    */
    void SetHeaderItemClass(const std::wstring& className);
    std::wstring GetHeaderItemClass() const;

    /** ListCtrlHeader/SplitBox的属性Class
    */
    void SetHeaderSplitBoxClass(const std::wstring& className);
    std::wstring GetHeaderSplitBoxClass() const;

    /** ListCtrlHeader/SplitBox/Control的属性Class
    */
    void SetHeaderSplitControlClass(const std::wstring& className);
    std::wstring GetHeaderSplitControlClass() const;

    /** CheckBox的Class属性(应用于Header和ListCtrl数据)
    */
    void SetCheckBoxClass(const std::wstring& className);
    std::wstring GetCheckBoxClass() const;

    /** 数据视图中的ListBox的Class属性
    */
    void SetDataViewClass(const std::wstring& className);
    std::wstring GetDataViewClass() const;

    /** ListCtrlItem的Class属性
    */
    void SetDataItemClass(const std::wstring& className);
    std::wstring GetDataItemClass() const;

    /** ListCtrlItem/LabelBox的Class属性
    */
    void SetDataItemLabelClass(const std::wstring& className);
    std::wstring GetDataItemLabelClass() const;

    /** 横向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetRowGridLineWidth() const;

    /** 纵向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetColumnGridLineWidth() const;

protected:
    /** 增加一列
    * @param [in] nColumnId 列的ID
    */
    void OnHeaderColumnAdded(size_t nColumnId);

    /** 删除一列
    * @param [in] nColumnId 列的ID
    */
    void OnHeaderColumnRemoved(size_t nColumnId);

    /** 调整列的宽度（拖动列宽调整，每次调整两个列的宽度）
    * @param [in] nColumnId1 第一列的ID
    * @param [in] nColumnId2 第二列的ID
    */
    void OnColumnWidthChanged(size_t nColumnId1, size_t nColumnId2);

    /** 对列排序
    * @param [in] nColumnId 列的ID
    * @param [in] bSortedUp 如果为true表示升序，如果为false表示降序
    */
    void OnColumnSorted(size_t nColumnId, bool bSortedUp);

    /** 通过拖动列表头，调整了列的顺序
    */
    void OnHeaderColumnOrderChanged();

    /** 表头的CheckBox勾选操作
    * @param [in] nColumnId 列的ID
    * @param [in] bChecked true表示勾选（Checked状态），false表示取消勾选（UnChecked状态）
    */
    void OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked);

    /** 表头列的显示属性发生变化
    */
    void OnHeaderColumnVisibleChanged();

    /** 同步UI的Check状态
    */
    void UpdateControlCheckStatus(size_t nColumnId);

private:
	/** 初始化标志
	*/
	bool m_bInited;

	/** 表头控件
	*/
	ListCtrlHeader* m_pHeaderCtrl;

	/** 列表数据展示
	*/
    ListCtrlDataView* m_pDataView;

	/** 列表数据管理
	*/
    ListCtrlDataProvider* m_pDataProvider;

    /** ListCtrlHeader的属性Class
    */
    UiString m_headerClass;

    /** ListCtrlHeaderItem的属性Class
    */
    UiString m_headerItemClass;

    /** ListCtrlHeader/SplitBox的属性Class
    */
    UiString m_headerSplitBoxClass;

    /** ListCtrlHeader/SplitBox/Control的属性Class
    */
    UiString m_headerSplitControlClass;

    /** CheckBox的Class
    */
    UiString m_checkBoxClass;

    /** 是否支持拖动改变列的顺序
    */
    bool m_bEnableHeaderDragOrder;

    /** ListCtrlItem的Class属性
    */
    UiString m_dataItemClass;

    /** ListCtrlItem/LabelBox的Class属性
    */
    UiString m_dataItemLabelClass;

    /** 当前是否可以更新Header的Check状态
    */
    bool m_bCanUpdateHeaderCheckStatus;

    /** 横向网格线的宽度
    */
    int32_t m_nRowGridLineWidth;

    /** 纵向网格线的宽度
    */
    int32_t m_nColumnGridLineWidth;

    /** ListBox的Class属性
    */
    UiString m_dataViewClass;

    /** 表头的高度
    */
    int32_t m_nHeaderHeight;

    /** 是否显示表头控件
    */
    bool m_bShowHeaderCtrl;

    /** 数据项的高度
    */
    int32_t m_nItemHeight;
};

/** ListCtrl子项控件
*/
class ListCtrlItem : public ListBoxItemH
{
public:
    /** 获取控件类型
    */
    virtual std::wstring GetType() const override { return L"ListCtrlItem"; }

};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_H_
