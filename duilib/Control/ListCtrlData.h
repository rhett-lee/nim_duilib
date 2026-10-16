#ifndef UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
#define UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_

#include "duilib/Box/VirtualListBox.h"
#include "duilib/Control/ListCtrlDefs.h"
#include <unordered_map>
#include <array>
#include <memory>

namespace ui
{
/** 列表项的数据管理器
*/
class ListCtrl;
class ListCtrlReportView;
struct ListCtrlSubItemData;
class DUILIB_API ListCtrlData : public ui::VirtualListBoxElement
{
    friend class ListCtrlReportView; //列表数据显示UI控件(Report视图)：需要访问单元格的数据存储，以支持单格刷新
    friend class ListCtrl;           //ListCtrl控件：需要按显示索引换算行身份(RowId)，用于单元格悬停状态与显示顺序解耦
public:
    //用于存储的数据结构
    typedef ListCtrlSubItemData2 Storage;
    typedef std::vector<ListCtrlItemData> RowDataList;      //行属性池：按行身份(RowId)索引

    /** 行身份：行的唯一标识，单调递增、删除后不复用
        用于把"行的身份"与"行的物理位置"解耦：增删改只重排显示顺序表，数据本体不搬移
    */
    typedef uint32_t RowId;

    /** 无效的行身份
    */
    static constexpr RowId kInvalidRowId = static_cast<RowId>(0xFFFFFFFF);

    /** 每页的单元格数量（分页连续存储）
        页索引 = RowId / kPageCells，页内偏移 = RowId % kPageCells
    */
    static constexpr size_t kPageCells = 64;

    /** 存储页：页内单元格连续存放，附带"已写入"标记
        优势：页内数组连续（绘制/排序扫描缓存友好，无shared_ptr指针追逐）；
              页为懒分配（某页首次写入时才创建），稀疏/纯虚表场景内存不膨胀
    */
    struct StoragePage
    {
        std::array<Storage, kPageCells> cells;  //页内单元格数组（连续）
        uint8_t usedMask[kPageCells] = {};      //每个单元格是否已写入（1表示已写入）
    };

    /** 每列的存储页列表（按页索引存放；页为懒分配，列表中允许出现空指针）
    */
    typedef std::vector<std::unique_ptr<StoragePage>> StoragePageList;
    typedef std::unordered_map<size_t, StoragePageList> StorageMap;

public:
    ListCtrlData();

    /** 创建一个数据项
    * @param [in] pVirtualListBox 关联的虚表的接口
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement(ui::VirtualListBox* pVirtualListBox) override;

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

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] dpiManager DPI缩放管理器
    */
    void ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale);

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

    /** 批量删除指定行的数据项, 并刷新界面显示
        内部实现：先按行身份去重并标记墓碑，再对显示顺序表做单趟压缩，避免逐行删除的 O(K×N) 搬移
    * @param [in] displayIndexs 需要删除的数据项显示索引号列表，有效范围：[0, GetDataItemCount())；
    *                            重复索引会被忽略，越界索引会被忽略
    * @return 返回实际删除的行数
    */
    size_t DeleteDataItems(const std::vector<size_t>& displayIndexs);

    /** 压缩数据存储：整理行身份与存储页布局，回收已删除行占用的内存
        说明：这是一次显式的整表维护操作（重编号 + 全量压缩），仅在无填充/绘制进行时调用；
              调用后所有已发出的内部单元格指针（原始指针）均失效
    */
    void CompactStorage();

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
    * @param [out] bChanged 返回数据是否变化
    */
    bool SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool& bChanged);

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

public:
    /** 设置指定数据项的文本，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @param [in] text 需要设置的文本内容
    */
    bool SetSubItemText(size_t itemIndex, size_t columnId, const DString& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号
    * @param [in] columnId 列的ID
    * @return 数据项关联的文本内容
    */
    DString GetSubItemText(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的排序分组（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] nSortGroup 需要设置的排序分组的整型数据
    */
    bool SetSubItemSortGroup(size_t itemIndex, size_t columnId, int32_t nSortGroup);

    /** 获取指定数据项的排序分组（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 排序分组的整型数据
    */
    int32_t GetSubItemSortGroup(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] userDataN 需要设置的数据项关联的整型数据
    */
    bool SetSubItemUserDataN(size_t itemIndex, size_t columnId, uint64_t userDataN);

    /** 获取指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的整型数据
    */
    uint64_t GetSubItemUserDataN(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] userDataS 需要设置的数据项关联的字符串数据
    */
    bool SetSubItemUserDataS(size_t itemIndex, size_t columnId, const DString& userDataS);

    /** 获取指定数据项的关联用户数据（字符串）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的字符串数据
    */
    DString GetSubItemUserDataS(size_t itemIndex, size_t columnId) const;

public:
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

    /** 设置指定数据项的文本是否为单行显示，并刷新界面显示
        说明：设置为多行(false)后，文本超出单元格宽度时会自动换行；
              行高需由调用方通过 SetDataItemHeight 调整，否则超出部分会被裁剪
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] bSingleLine true表示单行显示（超出时按省略号裁剪），false表示多行显示（自动换行）
    */
    bool SetSubItemSingleLine(size_t itemIndex, size_t columnId, bool bSingleLine);

    /** 获取指定数据项的文本是否为单行显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return true表示单行显示，false表示多行显示（自动换行）
    */
    bool IsSubItemSingleLine(size_t itemIndex, size_t columnId) const;

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

    /** 设置该列以进度条方式显示，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] nProgress 进度值，有效范围[0, 100]，-1表示不显示进度条（按普通文本显示）
    */
    bool SetSubItemProgress(size_t itemIndex, size_t columnId, int32_t nProgress);

    /** 获取该列的进度值
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 返回进度值，有效范围[0, 100]，-1表示不显示进度条（按普通文本显示）
    */
    int32_t GetSubItemProgress(size_t itemIndex, size_t columnId) const;

    /** 设置该列的文本是否按富文本（类HTML格式）渲染，并刷新界面显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @param [in] bRichText true表示按富文本渲染，false表示按普通文本渲染
    */
    bool SetSubItemRichText(size_t itemIndex, size_t columnId, bool bRichText);

    /** 获取该列的文本是否按富文本（类HTML格式）渲染
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemRichText(size_t itemIndex, size_t columnId) const;

    /** 对数据排序，并刷新界面显示
    * @param [in] columnId 列的ID
    * @param [in] nColumnIndex 列的序号
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] nSortFlag 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t nColumnId, size_t nColumnIndex,
                       bool bSortedUp, uint8_t nSortFlag,
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
    * @return 如果失败或者该单元格无数据则返回nullptr
    */
    const Storage* GetSubItemStorage(size_t itemIndex, size_t nColumnId) const;

    /** 获取指定数据项的数据, 写入（必要时分配存储页）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID
    * @return 如果失败则返回nullptr
    */
    Storage* GetSubItemStorageForWrite(size_t itemIndex, size_t nColumnId);

    /** 按行身份读取某列的单元格（只读）；该单元格未写入时返回nullptr
    */
    static const Storage* GetStorageAt(const StoragePageList& pageList, RowId rowId);

    /** 按行身份获取某列可写的单元格；必要时分配存储页并标记为已写入
    */
    static Storage* GetStorageAtForWrite(StoragePageList& pageList, RowId rowId);

    /** 将某列某行身份的单元格置为墓碑（清空数据并标记为未写入）；不分配存储页，
        并在该页已全部为墓碑时释放整页（页回收）
    */
    static void ResetStorageAt(StoragePageList& pageList, RowId rowId);

    /** 获取各个列的数据，用于UI展示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [out] subItemList 返回改行所有列的数据列表
    */
    bool GetSubItemStorageList(size_t itemIndex, std::vector<ListCtrlSubItemData2Pair>& subItemList) const;

    /** 显示索引 → 行身份；索引无效时返回 kInvalidRowId
    */
    RowId GetRowIdByDisplayIndex(size_t itemIndex) const;

    /** 行身份 → 显示索引；未找到时返回 Box::InvalidIndex
    */
    size_t GetDisplayIndexByRowId(RowId rowId) const;

    /** 分配一个新的行身份，并让各列数据池与行属性池增长到该位置
    * @return 返回新分配的行身份
    */
    RowId AllocRowId();

    /** 释放一个行身份关联的数据（列池置墓碑、行属性重置），并修正计数与单选状态
    */
    void FreeRowId(RowId rowId);

    /** 单元格选择：查找某列的选中位图（不存在时返回nullptr）
    */
    const std::vector<uint8_t>* FindCellBitmap(size_t nColumnId) const;

    /** 单元格选择：查找（必要时创建）某列的选中位图，并保证容量可容纳 rowId
    */
    std::vector<uint8_t>& GetCellBitmapForWrite(size_t nColumnId, RowId rowId);

    /** 单元格选择：按行身份设置单个单元格的选中状态
    */
    void SetCellSelectedByRowId(RowId rowId, size_t nColumnId, bool bSelected);

    /** 单元格选择：按行身份判断单个单元格是否被选中
    */
    bool IsCellSelectedByRowId(RowId rowId, size_t nColumnId) const;

    /** 检查内部数据的一致性（仅在Debug编译下进行断言检查）
    */
    void AssertDataConsistency() const;

public:
    /** 获取指定显示位置的数据项行属性（按显示顺序访问）
    * @param [in] itemIndex 数据项的显示索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回行属性数据的指针，索引无效时返回nullptr；该指针在下次数据变更前有效
    */
    const ListCtrlItemData* GetDisplayItemData(size_t itemIndex) const;

    /** 是否为标准模式（行高都为默认行高，无隐藏行，无置顶行）
    */
    bool IsNormalMode() const;

public:
    /** 获取当前单元格（单元格选择模式下的当前格，未设置时返回无效值）
        返回的行索引为当前的显示索引号（内部按行身份存储，排序/增删后自动跟随数据行）
    */
    ListCtrlCellId GetCurrentCell() const;

    /** 获取锚点单元格（区域选择的起点，未设置时返回无效值）
    */
    ListCtrlCellId GetAnchorCell() const;

    /** 设置当前单元格（仅更新当前格，不改变选中集合）
    * @param [in] cell 单元格标识（行显示索引 + 列ID）
    * @return 返回是否设置成功（行索引或列ID无效时返回false，不做任何修改）
    */
    bool SetCurrentCell(const ListCtrlCellId& cell);

    /** 设置锚点单元格（仅更新锚点，不改变选中集合）
    * @param [in] cell 单元格标识（行显示索引 + 列ID）
    * @return 返回是否设置成功（行索引或列ID无效时返回false，不做任何修改）
    */
    bool SetAnchorCell(const ListCtrlCellId& cell);

    /** 获取单元格的选择区域（锚点 + 当前格，未做规范化；用于查询与事件上报）
    */
    void GetCellSelection(ListCtrlCellId& anchorCell, ListCtrlCellId& focusCell) const;

    /** 判断单个单元格是否被选中（O(1)，按行身份判定，与显示顺序解耦）
    * @param [in] nRowIndex 行的显示索引号
    * @param [in] nColumnId 列ID
    */
    bool IsCellSelected(size_t nRowIndex, size_t nColumnId) const;

    /** 设置单个单元格的选中状态（不清除其它已选中的单元格）
    * @return 返回选中状态是否发生变化（行无效/行不可选/状态未变化时返回 false）
    */
    bool SetCellSelected(size_t nRowIndex, size_t nColumnId, bool bSelected);

    /** 对"显示行区间 × 指定列集合"做区域选择
    * @param [in] nRowBegin/nRowEnd 显示索引区间（已规范化，begin<=end）；无效值表示不改变行
    * @param [in] columnIdList 参与选择的列ID列表（顺序无关）
    * @param [in] bAdditive true=追加到已有选择；false=先清除全部单元格选择再设置
    */
    void SetCellSelectionRect(size_t nRowBegin, size_t nRowEnd,
                              const std::vector<size_t>& columnIdList, bool bAdditive);

    /** 全选所有单元格
    * @param [in] columnIdList 参与选择的列ID列表
    */
    void SelectAllCells(const std::vector<size_t>& columnIdList);

    /** 清除所有单元格选择（选中集合、锚点、当前格一并清空）
    * @return 清除前是否存在实际的选择状态（有选中单元格或存在当前格）；
    *         返回 false 表示本来就没有选择，调用方无需刷新界面或派发选择变化事件
    */
    bool ClearCellSelection();

private:
    /** 排序数据
    */
    struct StorageData
    {
        size_t index;             //原来的数据索引号
        const Storage* pStorage;  //该行在排序列的单元格（可能为nullptr）
    };

    /** 对数据排序
    * @param [in] dataList 待排序的数据
    * @param [in] nColumnId 列的ID
    * @param [in] nColumnIndex 列的序号
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] nSortFlag 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortStorageData(std::vector<StorageData>& dataList, size_t nColumnId, size_t nColumnIndex,
                         bool bSortedUp, uint8_t nSortFlag,
                         ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

    /** 默认的数据比较函数
    * @param [in] a 第一个比较数据
    * @param [in] b 第二个比较数据
    * @param [in] nSortFlag 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    * @return 如果 (a < b)，返回true，否则返回false
    */
    bool SortDataCompareFunc(const ListCtrlSubItemData2& a, const ListCtrlSubItemData2& b, uint8_t nSortFlag) const;

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

    /** 数据，按列保存，每列一份分页连续存储
        按页索引存放（页 = RowId / kPageCells），页内按行身份偏移索引；页为懒分配，
        删除某行时仅把该单元格标记为未写入（墓碑），数据本体不搬移；
        整页全部变为墓碑时释放该页（页回收），长时间高频增删后可调用 CompactStorage 做整体压缩
    */
    StorageMap m_dataMap;

    /** 行的属性数据，按行身份(RowId)索引（删除某行时该槽位重置为默认值墓碑）
    */
    RowDataList m_rowDataList;

    /** 显示顺序表：m_displayOrder[i] 表示当前第i个显示位置对应的行身份
        是唯一随增/删/改/排序变动的结构（每个元素仅4字节），数据本体不随之搬移
    */
    std::vector<RowId> m_displayOrder;

    /** 下一个可分配的行身份（单调递增，删除后不复用）
    */
    RowId m_nextRowId;

    /** 填充元素时复用的缓冲区，避免每次填充行数据都重新分配临时容器
        （仅在 FillElement 同步调用期间使用，不跨调用持有状态）
    */
    std::vector<ListCtrlSubItemData2Pair> m_subItemListBuffer;

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

    /** 单选的时候，选中的元素行身份（无效时为 kInvalidRowId）
    */
    RowId m_nSelectedRowId;

    /** 当前默认的文本属性
    */
    int32_t m_nDefaultTextStyle;

    /** 当前默认的行高
    */
    int32_t m_nDefaultItemHeight;

    /** 单元格身份：行身份(RowId) + 列ID
        与显示顺序解耦，行被排序/增删后仍指向同一数据行
    */
    struct CellIdentity
    {
        RowId nRowId = kInvalidRowId;
        size_t nColumnId = Box::InvalidIndex;
        bool IsValid() const { return (nRowId != kInvalidRowId) && (nColumnId != Box::InvalidIndex); }
    };

    /** 单元格选择模式下的锚点单元格（拖拽/区域选择的起点）
    */
    CellIdentity m_anchorCell;

    /** 单元格选择模式下的当前单元格（区域焦点）
    */
    CellIdentity m_currentCell;

    /** 单元格选中集合：每列一张位图（key=列ID，value=按行身份RowId索引的位图，1表示选中）
        独立于分页懒分配的单元格存储：全选/选中未实化行不会强制分配单元格实体，
        内存为"列数 × 行数 / 8 字节"；按行身份索引，排序/增删后选择跟随原数据行
    */
    std::unordered_map<size_t, std::vector<uint8_t>> m_cellSelected;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_DATA_PROVIDER_H_
