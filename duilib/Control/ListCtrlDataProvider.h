#ifndef UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
#define UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
/** 用于存储的数据结构
*/
struct ListCtrlData
{
    UiString text;                  //文本内容
    uint16_t nTextFormat = 0;       //文本对齐方式等属性, 该属性仅应用于Header, 取值可参考：IRender.h中的DrawStringFormat，如果为-1，表示按默认配置的对齐方式
    int32_t nImageIndex = -1;       //图标资源索引号，在图片列表里面的下标值，如果为-1表示不显示图标
    UiColor textColor;              //文本颜色
    UiColor bkColor;                //背景颜色
    bool bShowCheckBox = true;      //是否显示CheckBox
    uint8_t nCheckBoxWidth = 0;     //CheckBox控件所占的宽度，仅当bShowCheckBox为true时有效
    bool bSelected = false;         //是否处于选择状态（ListBoxItem按整行选中）
    bool bChecked = false;          //是否处于勾选状态（CheckBox勾选状态）
    size_t nItemData = 0;           //用户自定义数据

    //TODO: 待实现功能列表
    //bool bVisible;                  //是否可见
    //bool bAlwaysAtTop;              //置顶显示
    //int32_t nBkImageIndex = -1;     //背景图片资源索引号
    //Item的文本可以编辑
    //选择：单选，多选，整行选中，提供接口
    //事件响应：点击，右键等
    //设置行高（最好支持每行的行高不同，Header的行高单独设置）
    //多视图的支持：Report，Icon等，类似与Windows资源管理器
    //数据类型的支持：比如整型，日期型，下拉表，字符串类型等
    //关联图片列表，图片列表需要单独实现
    //表格Margin的支持

};

/** 比较数据的附加信息
*/
struct ListCtrlCompareParam
{
    size_t nColumnIndex; //数据关联第几列，有效范围：[0, GetColumnCount())
    size_t nColumnId;    //数据关联列的ID
    void* pUserData;     //用户自定义数据，设置比较函数的时候一同传入
};

/** 存储数据的比较函数的原型, 实现升序的比较(a < b)
* @param [in] a 第一个比较数据
* @param [in] b 第二个比较数据
* @param [in] param 数据关联的参数
* @return 如果 (a < b)，返回true，否则返回false
*/
typedef std::function<bool(const ListCtrlData& a, const ListCtrlData& b, const ListCtrlCompareParam& param)>
    ListCtrlDataCompareFunc;

/** 列表项的数据管理器
*/
class ListCtrl;
struct ListCtrlDataItem;
class ListCtrlDataProvider : public ui::VirtualListBoxElement
{
public:
    //用于存储的数据结构
    typedef ListCtrlData Storage;
    typedef std::shared_ptr<Storage> StoragePtr;
    typedef std::vector<StoragePtr> StoragePtrList;
    typedef std::unordered_map<size_t, StoragePtrList> StorageMap;

public:
    ListCtrlDataProvider();

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateElement() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) override;

public:
    /** 设置表头接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 增加一列
    * @param [in] columnId 列的ID
    */
    bool AddColumn(size_t columnId);

    /** 删除一列
    * @param [in] columnId 列的ID
    */
    bool RemoveColumn(size_t columnId);

    /** 设置一列的勾选状态（Checked或者UnChecked）
    * @param [in] columnId 列的ID
    * @param [in] bChecked true表示选择，false表示取消选择
    */
    bool SetColumnCheck(size_t columnId, bool bChecked);

    /** 获取数据项总个数
    */
    size_t GetDataItemCount() const;

    /** 设置数据项总个数
    * @param [in] itemCount 数据项的总数，具体每个数据项的数据，通过回调的方式进行填充（内部为虚表实现）
    */
    bool SetDataItemCount(size_t itemCount);

    /** 在最后添加一个数据项
    * @param [in] dataItem 数据项的内容
    * @return 成功返回数据项的行索引号(rowIndex)，失败则返回Box::InvalidIndex
    */
    size_t AddDataItem(const ListCtrlDataItem& dataItem);

    /** 在指定行位置添加一个数据项
    * @param [in] itemIndex 数据项的索引号
    * @param [in] dataItem 数据项的内容
    */
    bool InsertDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem);

    /** 设置指定行的数据项
    * @param [in] itemIndex 数据项的索引号
    * @param [in] dataItem 数据项的内容
    */
    bool SetDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem);

    /** 删除指定行的数据项
    * @param [in] itemIndex 数据项的索引号
    */
    bool DeleteDataItem(size_t itemIndex);

    /** 删除所有行的数据项
    */
    bool DeleteAllDataItems();

    /** 设置数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号
    * @param [in] itemData 数据项关联的自定义数据
    */
    bool SetDataItemData(size_t itemIndex, size_t itemData);

    /** 获取数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号
    * @return 返回数据项关联的自定义数据
    */
    size_t GetDataItemData(size_t itemIndex) const;

    /** 设置指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] text 需要设置的文本内容
    */
    bool SetDataItemText(size_t itemIndex, size_t columnIndex, const std::wstring& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 数据项关联的文本内容
    */
    std::wstring GetDataItemText(size_t itemIndex, size_t columnIndex) const;

    /** 设置指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] textColor 需要设置的文本颜色
    */
    bool SetDataItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor);

    /** 获取指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] textColor 数据项关联的文本颜色
    */
    bool GetDataItemTextColor(size_t itemIndex, size_t columnIndex, UiColor& textColor) const;

    /** 设置指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bkColor 需要设置的背景颜色
    */
    bool SetDataItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor);

    /** 获取指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] bkColor 数据项关联的背景颜色
    */
    bool GetDataItemBkColor(size_t itemIndex, size_t columnIndex, UiColor& bkColor) const;

    /** 是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    */
    bool IsShowCheckBox(size_t itemIndex, size_t columnIndex) const;

    /** 设置是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bShowCheckBox true表示显示，false表示不显示
    */
    bool SetShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox);

    /** 设置CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] bSelected true表示勾选，false表示不勾选
    */
    bool SetCheckBoxSelect(size_t itemIndex, size_t columnIndex, bool bSelected);

    /** 获取CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [out] bSelected true表示勾选，false表示不勾选
    */
    bool GetCheckBoxSelect(size_t itemIndex, size_t columnIndex, bool& bSelected) const;

    /** 对数据排序
    * @param [in] columnId 列的ID
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t nColumnId, bool bSortedUp,
                       ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

    /** 设置外部自定义的排序函数, 替换默认的排序函数
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    void SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

private:
    /** 数据转换为存储数据结构
    */
    void DataItemToStorage(Storage& storage, const ListCtrlDataItem& item) const;

    /** 根据列序号查找列ID
    * @return 返回列ID，如果匹配不到，则返回Box::InvalidIndex
    */
    size_t GetColumnId(size_t nColumnIndex) const;

    /** 根据列ID查找列序号
    * @param [in] nColumnId 列ID
    * @return 返回列序号，如果匹配不到，则返回Box::InvalidIndex
    */
    size_t GetColumnIndex(size_t nColumnId) const;

    /** 判断一个数据项索引是否有效
    */
    bool IsValidDataItemIndex(size_t itemIndex) const;

    /** 判断一个列ID在数据存储中是否有效
    */
    bool IsValidDataColumnId(size_t nColumnId) const;

    /** 获取指定数据项的数据, 读取
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetDataItemStorage(size_t itemIndex, size_t columnIndex) const;

    /** 获取指定数据项的数据, 写入
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetDataItemStorageForWrite(size_t itemIndex, size_t columnIndex);

    /** 获取各个列的数据，用于UI展示
    * @param [in] nDataItemIndex 数据Item的下标，代表行
    * @param [in] columnIdList 列ID列表
    * @param [out] storageList 返回数据列表
    */
    bool GetDataItemStorageList(size_t nDataItemIndex,
                                std::vector<size_t>& columnIdList,
                                StoragePtrList& storageList) const;

    /** 某个数据项的Check勾选状态变化
    * @param [in] itemIndex 数据Item的下标，代表行
    * @param [in] nColumnId 列ID
    * @param [in] bChecked 是否勾选
    */
    void OnDataItemChecked(size_t itemIndex, size_t nColumnId, bool bChecked);

    /** 同步UI的Check状态
    */
    void UpdateControlCheckStatus(size_t nColumnId);

private:
    /** 排序数据
    */
    struct StorageData
    {
        size_t index;       //原来的数据索引号
        StoragePtr pStorage;
    };

    /** 对数据排序
    * @param [in] dataList 待排序的数据
    * @param [in] nColumnId 列的ID
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortStorageData(std::vector<StorageData>& dataList,
                         size_t nColumnId, bool bSortedUp,
                         ListCtrlDataCompareFunc pfnCompareFunc,
                         void* pUserData);

    /** 默认的数据比较函数
    * @param [in] a 第一个比较数据
    * @param [in] b 第二个比较数据
    * @return 如果 (a < b)，返回true，否则返回false
    */
    bool SortDataCompareFunc(const ListCtrlData& a, const ListCtrlData& b) const;

private:
    /** 表头控件
    */
    ListCtrl* m_pListCtrl;

    /** 数据，按列保存，每个列一个数组
    */
    StorageMap m_dataMap;

    /** 外部设置的排序函数
    */
    ListCtrlDataCompareFunc m_pfnCompareFunc;

    /** 外部设置的排序函数附加数据
    */
    void* m_pUserData;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
