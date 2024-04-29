#ifndef UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
#define UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"
#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表项的数据管理器
*/
class ListCtrl;
struct ListCtrlSubItemData;
class ListCtrlData : public ui::VirtualListBoxElement
{
public:
    //用于存储的数据结构
    typedef ListCtrlSubItemData2 Storage;
    typedef std::shared_ptr<Storage> StoragePtr;
    typedef std::vector<StoragePtr> StoragePtrList;
    typedef std::unordered_map<size_t, StoragePtrList> StorageMap;
    typedef std::vector<ListCtrlItemData> RowDataList;

public:
    ListCtrlData();

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
    virtual size_t GetElementCount() const override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) const override;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    virtual void GetSelectedElements(std::vector<size_t>& selectedIndexs) const override;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const override;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) override;

public:
    /** 设置视图接口
    */
    void SetListView(IListCtrlView* pListView);

    /** 设置是否自动勾选选择的数据项(作用于Header与每行)
    */
    void SetAutoCheckSelect(bool bAutoCheckSelect);

    /** 获取是否自动勾选选择的数据项
    */
    bool IsAutoCheckSelect() const;

    /** 设置默认的文本属性
    */
    void SetDefaultTextStyle(int32_t nTextStyle);

    /** 设置默认的行高
    */
    void SetDefaultItemHeight(int32_t nItemHeight);

public:
    /** 增加一列, 并刷新界面显示
    * @param [in] columnId 列的ID
    */
    bool AddColumn(size_t columnId);

    /** 删除一列, 并刷新界面显示
    * @param [in] columnId 列的ID
    */
    bool RemoveColumn(size_t columnId);

    /** 获取某列的宽度最大值
    * @return 返回该列宽度的最大值，返回的是DPI自适应后的值； 如果失败返回-1
    */
    int32_t GetMaxColumnWidth(size_t columnId) const;

    /** 设置一列的勾选状态（Checked或者UnChecked）
    * @param [in] columnId 列的ID
    * @param [in] bChecked true表示选择，false表示取消选择
    * @param [in] bRefresh 是否刷新界面显示
    */
    bool SetColumnCheck(size_t columnId, bool bChecked, bool bRefresh);

    /** 获取数据项总个数
    */
    size_t GetDataItemCount() const;

    /** 设置数据项总个数, 并刷新界面显示
    * @param [in] itemCount 数据项的总数，具体每个数据项的数据，通过回调的方式进行填充（内部为虚表实现）
    */
    bool SetDataItemCount(size_t itemCount);

    /** 在最后添加一个数据项, 并刷新界面显示
    * @param [in] columnId 列的ID
    * @param [in] dataItem 数据项的内容
    * @return 成功返回数据项的行索引号，失败则返回Box::InvalidIndex
    */
    size_t AddDataItem(size_t columnId, const ListCtrlSubItemData& dataItem);

    /** 在指定行位置添加一个数据项, 并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [in] dataItem 数据项的内容
    */
    bool InsertDataItem(size_t itemIndex, size_t columnId, const ListCtrlSubItemData& dataItem);

    /** 删除指定行的数据项, 并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    */
    bool DeleteDataItem(size_t itemIndex);

    /** 删除所有行的数据项, 并刷新界面显示
    * @return 如果有数据被删除返回true，否则返回false
    */
    bool DeleteAllDataItems();

    /** 设置数据项的行属性数据, 并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    * @param [out] bChanged 返回数据是否变化
    * @param [out] bCheckChanged 返回bCheck标志是否变化
    */
    bool SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData, 
                         bool& bChanged, bool& bCheckChanged);

    /** 获取数据项的行属性数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    */
    bool GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const;

    /** 设置数据项的可见性, 并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bVisible 是否可见
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemVisible(size_t itemIndex, bool bVisible, bool& bChanged);

    /** 获取数据项的可见性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的可见性
    */
    bool IsDataItemVisible(size_t itemIndex) const;

    /** 设置数据项的选择属性, 并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bSelected 是否选择状态
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemSelected(size_t itemIndex, bool bSelected, bool& bChanged);

    /** 获取数据项的选择属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的选择状态
    */
    bool IsDataItemSelected(size_t itemIndex) const;

    /** 设置行首的图标, 不刷新界面，由外部负责调用界面刷新
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemImageId(size_t itemIndex, int32_t imageId, bool& bChanged);

    /** 获取行首的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    int32_t GetDataItemImageId(size_t itemIndex) const;

    /** 获取选择状态(bSelect)
    * @param [out] bSelected 是否选择
    * @param [out] bPartSelected 是否部分选择
    */
    void GetDataItemsSelectStatus(bool& bSelected, bool& bPartSelected) const;

    /** 设置数据项的勾选属性（每行前面的CheckBox）, 不刷新界面，由外部负责调用界面刷新
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bChecked 是否勾选状态
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemChecked(size_t itemIndex, bool bChecked, bool& bChanged);

    /** 获取数据项的选择属性（每行前面的CheckBox）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的勾选状态
    */
    bool IsDataItemChecked(size_t itemIndex) const;

    /** 设置所有行的勾选状态（Checked或者UnChecked）, 并刷新界面显示
    * @param [in] bChecked true表示勾选，false表示取消勾选
    */
    bool SetAllDataItemsCheck(bool bChecked);

    /** 批量设置勾选数据项（行首的CheckBox打勾的数据）, 不刷新界面，由外部负责调用界面刷新
    * @param [in] itemIndexs 需要设置勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetCheckedDataItems(const std::vector<size_t>& itemIndexs,
                             bool bClearOthers,
                             std::vector<size_t>& refreshIndexs);

    /** 获取勾选的元素列表（行首的CheckBox打勾的数据）
    * @param [in] itemIndexs 返回当前勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    */
    void GetCheckedDataItems(std::vector<size_t>& itemIndexs) const;

    /** 获取勾选状态(bChecked)
    * @param [out] bChecked 是否勾选
    * @param [out] bPartChecked 是否部分勾选
    */
    void GetDataItemsCheckStatus(bool& bChecked, bool& bPartChecked) const;

    /** 设置数据项的置顶状态, 不刷新界面，由外部负责调用界面刷新
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nAlwaysAtTop 置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop, bool& bChanged);

    /** 获取数据项的置顶状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    */
    int8_t GetDataItemAlwaysAtTop(size_t itemIndex) const;

    /** 设置数据项的行高, 不刷新界面，由外部负责调用界面刷新
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nItemHeight 行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale, bool& bChanged);

    /** 获取数据项的行高
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    */
    int32_t GetDataItemHeight(size_t itemIndex) const;

    /** 设置数据项的自定义数据, 不刷新界面
    * @param [in] itemIndex 数据项的索引号
    * @param [in] userData 数据项关联的自定义数据
    */
    bool SetDataItemUserData(size_t itemIndex, size_t userData);

    /** 获取数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号
    * @return 返回数据项关联的自定义数据
    */
    size_t GetDataItemUserData(size_t itemIndex) const;

public:
    /** 设置指定<行,列>的数据项，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] subItemData 指定数据项的内容，列序号在dataItem.nColumnIndex中指定
    * @param [out] bCheckChanged bChecked状态是否变化
    */
    bool SetSubItemData(size_t itemIndex, size_t columnId,
                        const ListCtrlSubItemData& subItemData, bool& bCheckChanged);

    /** 获取指定<行,列>的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [out] subItemData 指定数据项的内容
    */
    bool GetSubItemData(size_t itemIndex, size_t columnId, ListCtrlSubItemData& subItemData) const;

    /** 设置指定数据项的文本，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [in] text 需要设置的文本内容
    */
    bool SetSubItemText(size_t itemIndex, size_t columnId, const std::wstring& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @return 数据项关联的文本内容
    */
    std::wstring GetSubItemText(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的文本颜色，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [in] textColor 需要设置的文本颜色
    */
    bool SetSubItemTextColor(size_t itemIndex, size_t columnId, const UiColor& textColor);

    /** 获取指定数据项的文本颜色，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [out] textColor 数据项关联的文本颜色
    */
    bool GetSubItemTextColor(size_t itemIndex, size_t columnId, UiColor& textColor) const;

    /** 设置指定数据项的文本属性（文本对齐方式等），并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] nTextFormat 需要设置的文本属性
    */
    bool SetSubItemTextFormat(size_t itemIndex, size_t columnId, int32_t nTextFormat);

    /** 获取指定数据项的文本属性（文本对齐方式等）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的文本属性
    */
    int32_t GetSubItemTextFormat(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的背景颜色，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [in] bkColor 需要设置的背景颜色
    */
    bool SetSubItemBkColor(size_t itemIndex, size_t columnId, const UiColor& bkColor);

    /** 获取指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [out] bkColor 数据项关联的背景颜色
    */
    bool GetSubItemBkColor(size_t itemIndex, size_t columnId, UiColor& bkColor) const;

    /** 是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemShowCheckBox(size_t itemIndex, size_t columnId) const;

    /** 设置是否显示CheckBox，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] bShowCheckBox true表示显示，false表示不显示
    */
    bool SetSubItemShowCheckBox(size_t itemIndex, size_t columnId, bool bShowCheckBox);

    /** 设置CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] bChecked true表示勾选，false表示不勾选
    * @param [in] bRefresh true表示刷新该元素的界面显示，false表示不需要刷新界面
    */
    bool SetSubItemCheck(size_t itemIndex, size_t columnId, bool bChecked, bool bRefresh);

    /** 获取CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [out] bChecked true表示勾选，false表示不勾选
    */
    bool GetSubItemCheck(size_t itemIndex, size_t columnId, bool& bChecked) const;

    /** 获取某一列的勾选状态(bChecked)
    * @param [in] columnId 列的ID
    * @param [out] bChecked 是否选择
    * @param [out] bPartChecked 是否部分选择
    */
    void GetColumnCheckStatus(size_t columnId, bool& bChecked, bool& bPartChecked) const;

    /** 设置该列的图标，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    */
    bool SetSubItemImageId(size_t itemIndex, size_t columnId, int32_t imageId);

    /** 获取该列的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    */
    int32_t GetSubItemImageId(size_t itemIndex, size_t columnId) const;

    /** 设置该列的文本是否可编辑
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] bEditable true表示可编辑，false表示不可编辑
    */
    bool SetSubItemEditable(size_t itemIndex, size_t columnId, bool bEditable);

    /** 获取该列的文本是否可编辑
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemEditable(size_t itemIndex, size_t columnId) const;

    /** 对数据排序，并刷新界面显示
    * @param [in] columnId 列的ID
    * @param [in] nColumnIndex 列的序号
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t nColumnId, size_t nColumnIndex, bool bSortedUp,
                       ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

    /** 设置外部自定义的排序函数, 替换默认的排序函数
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    void SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

public:
    /** 批量设置选择元素, 不更新界面显示
    * @param [in] selectedIndexs 需要设置选择的元素列表，有效范围：[0, GetElementCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetSelectedElements(const std::vector<size_t>& selectedIndexs,
                             bool bClearOthers,
                             std::vector<size_t>& refreshIndexs);

    /** 选择全部(排除不可见元素、置顶元素), 不更新界面显示
    * @return 如果有数据变化返回true，否则返回false
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    bool SelectAll(std::vector<size_t>& refreshIndexs);

    /** 取消所有选择, 不更新界面显示，可以由外部刷新界面显示
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SelectNone(std::vector<size_t>& refreshIndexs);

    /** 取消所有选择(但排除部分元素), 不更新界面显示，可以由外部刷新界面显示
    * @param [in] excludeIndexs 需要排除的元素索引号，这部分元素的选择状态保持原状
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SelectNoneExclude(const std::vector<size_t>& excludeIndexs,
                           std::vector<size_t>& refreshIndexs);

    /** 判断一个行数据是否可选择
    */
    bool IsSelectableRowData(const ListCtrlItemData& rowData) const;

    /** 判断一个数据元素是否为可选择项
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    */
    bool IsSelectableElement(size_t nElementIndex) const;

private:
    /** 数据转换为存储数据结构
    */
    void SubItemToStorage(const ListCtrlSubItemData& item, Storage& storage) const;

    /** 存储数据转换为结构数据
    */
    void StorageToSubItem(const Storage& storage, ListCtrlSubItemData& item) const;

    /** 判断一个数据项索引是否有效
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool IsValidDataItemIndex(size_t itemIndex) const;

    /** 判断一个列ID在数据存储中是否有效
    */
    bool IsValidDataColumnId(size_t nColumnId) const;

    /** 获取指定数据项的数据, 读取
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetSubItemStorage(size_t itemIndex, size_t nColumnId) const;

    /** 获取指定数据项的数据, 写入
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 如果失败则返回nullptr
    */
    StoragePtr GetSubItemStorageForWrite(size_t itemIndex, size_t nColumnId);

    /** 获取各个列的数据，用于UI展示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [out] subItemList 返回改行所有列的数据列表
    */
    bool GetSubItemStorageList(size_t itemIndex, std::vector<ListCtrlSubItemData2Pair>& subItemList) const;

public:
    /** 获取行属性数据
    */
    const RowDataList& GetItemDataList() const;

    /** 是否为标准模式（行高都为默认行高，无隐藏行，无置顶行）
    */
    bool IsNormalMode() const;

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
    * @param [in] nColumnIndex 列的序号
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortStorageData(std::vector<StorageData>& dataList,
                         size_t nColumnId, size_t nColumnIndex, bool bSortedUp,
                         ListCtrlDataCompareFunc pfnCompareFunc,
                         void* pUserData);

    /** 默认的数据比较函数
    * @param [in] a 第一个比较数据
    * @param [in] b 第二个比较数据
    * @return 如果 (a < b)，返回true，否则返回false
    */
    bool SortDataCompareFunc(const ListCtrlSubItemData2& a, const ListCtrlSubItemData2& b) const;

    /** 更新个性化数据（隐藏行、行高、置顶等）
    */
    void UpdateNormalMode();

private:
    /** 视图控件接口
    */
    IListCtrlView* m_pListView;

    /** 是否自动勾选选择的数据项
    */
    bool m_bAutoCheckSelect;

    /** 数据，按列保存，每个列一个数组
    */
    StorageMap m_dataMap;

    /** 行的属性数据
    */
    RowDataList m_rowDataList;

    /** 外部设置的排序函数
    */
    ListCtrlDataCompareFunc m_pfnCompareFunc;

    /** 外部设置的排序函数附加数据
    */
    void* m_pUserData;

    /** 隐藏行的个数
    */
    int32_t m_hideRowCount;

    /** 非默认行高行的个数
    */
    int32_t m_heightRowCount;

    /** 置顶行的个数
    */
    int32_t m_atTopRowCount;

    /** 是否支持多选
    */
    bool m_bMultiSelect;

    /** 单选的时候，选择的元素索引号
    */
    size_t m_nSelectedIndex;

    /** 当前默认的文本属性
    */
    int32_t m_nDefaultTextStyle;

    /** 当前默认的行高
    */
    int32_t m_nDefaultItemHeight;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
