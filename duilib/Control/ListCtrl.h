#ifndef UI_CONTROL_LIST_CTRL_H_
#define UI_CONTROL_LIST_CTRL_H_

#include "duilib/Control/ListCtrlDefs.h"
#include "duilib/Control/ListCtrlHeader.h"
#include "duilib/Control/ListCtrlHeaderItem.h"
#include "duilib/Control/ListCtrlItem.h"
#include "duilib/Control/ListCtrlSubItem.h"
#include "duilib/Control/ListCtrlData.h"
#include "duilib/Core/ImageList.h"

namespace ui
{
class RichEdit;
class DateTime;

class ListCtrlData;
class ListCtrlReportView;
class ListCtrlIconView;
class ListCtrlListView;
class ListCtrlHeader;
class ListCtrlHeaderItem;
class ListCtrlItem;
class ListCtrlSubItem;
class ListCtrlIconViewItem;
class ListCtrlListViewItem;
class ListCtrlComboFlyout;

/** ListCtrl控件
*/
class DUILIB_API ListCtrl: public VBox
{
    typedef VBox BaseClass;
    friend class ListCtrlData;          //列表数据管理容器
    friend class ListCtrlReportView;    //列表数据显示UI控件
    friend class ListCtrlIconView;      //列表数据显示UI控件
    friend class ListCtrlListView;      //列表数据显示UI控件
    friend class ListCtrlHeader;
    friend class ListCtrlHeaderItem;
    friend class ListCtrlItem;
    friend class ListCtrlSubItem;
    friend class ListCtrlIconViewItem;
    friend class ListCtrlListViewItem;
public:
    explicit ListCtrl(Window* pWindow);
    virtual ~ListCtrl() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void HandleEvent(const EventArgs& msg) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置表格类型（默认为Report类型）
    */
    void SetListCtrlType(ListCtrlType type);

    /** 获取表格类型
    */
    ListCtrlType GetListCtrlType() const;

    /** 设置图片列表，每个视图有一个独立的图片列表，也可以共享同一个图片列表
    * @param [in] type 视图类型
    * @param [in] spImageList 图片资源接口, 智能指针
    */
    void SetImageList(ListCtrlType type, const ImageListPtr& spImageList);

    /** 获取图片列表，可以添加图片资源，用于在列表中显示图标资源
    */
    ImageListPtr GetImageList(ListCtrlType type) const;

public:
    /** 获取列的个数
    */
    size_t GetColumnCount() const;

    /** 在指定位置添加一列
    * @param [in] columnIndex 在第几列以后插入该列，如果是-1，表示在最后追加一列
    * @param [in] columnInfo 列的基本属性
    * @return 返回这一列的表头控件接口，使用ListCtrlHeaderItem::GetColumnId()函数可以获取列ID
    */
    ListCtrlHeaderItem* InsertColumn(int32_t columnIndex, const ListCtrlColumn& columnInfo);

    /** 获取列的ID
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @return 返回该列索引序号对应列的ID，即使用ListCtrlHeaderItem::GetColumnId()函数获取的列ID
    */
    size_t GetColumnId(size_t columnIndex) const;

    /** 判断一个列的ID是否有效
    * @param [in] columnId 列的ID值
    */
    bool IsValidColumnId(size_t columnId) const;

    /** 获取列的索引序号
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    * @return 列的序号：[0, GetColumnCount())，代表第几列
    */
    size_t GetColumnIndex(size_t columnId) const;

    /** 获取列表头的控件接口
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    */
    ListCtrlHeaderItem* GetColumn(size_t columnIndex) const;
    ListCtrlHeaderItem* GetColumnById(size_t columnId) const;

    /** 删除一列
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    */
    bool DeleteColumn(size_t columnIndex);
    bool DeleteColumnById(size_t columnId);

    /** 删除所有列
    */
    void DeleteAllColumns();

    /** 获取列宽度
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    */
    int32_t GetColumnWidth(size_t columnIndex) const;
    int32_t GetColumnWidthById(size_t columnId) const;

    /** 调整列的宽度(根据该列内容的实际宽度自适应)
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    * @param [in] nWidth 列宽值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
    */
    bool SetColumnWidth(size_t columnIndex, int32_t nWidth, bool bNeedDpiScale);
    bool SetColumnWidthById(size_t columnId, int32_t nWidth, bool bNeedDpiScale);

    /** 自动调整列的宽度(根据该列内容的实际宽度自适应)
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    */
    bool SetColumnWidthAuto(size_t columnIndex);
    bool SetColumnWidthAutoById(size_t columnId);

    /** 显示/隐藏一列
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    * @param [in] bColumnVisible true表示显示该列，false表示隐藏该列
    * @return 返回true表示设置成功，false表示参数无效
    */
    bool SetColumnVisible(size_t columnIndex, bool bColumnVisible);
    bool SetColumnVisibleById(size_t columnId, bool bColumnVisible);

    /** 判断一列是否显示
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    * @return 返回true表示该列显示，返回false表示该列隐藏（或者参数无效）
    */
    bool IsColumnVisible(size_t columnIndex) const;
    bool IsColumnVisibleById(size_t columnId) const;

    /** 按设置的宽度值调整所有列的宽度
    * @param [in] columnWidthList 容器的下标对应列表列的索引序号，容器的值代表各个列的期望宽度
    *             列的宽度设置支持支持三种类型：固定数值、自动适应、拉伸。
    *             固定数值时：与上面的SetColumnWidth函数功能相同
    *             自动适应时：与上面的SetColumnWidthAuto函数功能相同
    *             拉伸时，按以下规则操作：
    *             （1）按照列表视图总宽度，所有拉伸类型的列，按指定的百分比进行分配所有可分配宽度（可分配宽度=列表视图总宽度-固定宽度-自动宽度）
    *             （2）如果可分配宽度不足（即为0时），则保持相关列的原宽度，不做任何操作
    *             （3）举例1：自定义比例，假定共计三列，columnWidthList里面有三个值：UiFixedInt::MakeStretch(30),UiFixedInt::MakeStretch(30),UiFixedInt::MakeStretch(40)
    *                        表示三个列的宽度分别占列表视图总宽度的30%、30%、40%
    *             （4）举例2：各列的宽度平均分配，可以直接设置拉伸值，不需要计算具体比例：columnWidthList里面有三个值：UiFixedInt::MakeStretch(),UiFixedInt::MakeStretch(),UiFixedInt::MakeStretch()
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应（仅当列宽度设置为固定数值时使用）
    */
    bool SetColumnWidth(const std::vector<UiFixedInt>& columnWidthList, bool bNeedDpiScale);
    
    /** 设置是否支持列表头拖动改变列的顺序
    */
    void SetEnableHeaderDragOrder(bool bEnable);

    /** 是否支持列表头拖动改变列的顺序
    */
    bool IsEnableHeaderDragOrder() const;

    /** 设置冻结列的个数（固定列表左侧的前N列，不随横向滚动而移动）
    * @param [in] nCount 冻结的列数，0表示不冻结
    */
    void SetFrozenColumnCount(size_t nCount);

    /** 获取冻结列的个数
    */
    size_t GetFrozenColumnCount() const;

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

    /** 设置某列数据的排序方式（通过列序号）
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值
    * @param [in] nSortFlag 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    */
    void SetColumnSortFlag(size_t columnIndex, uint8_t nSortFlag);
    void SetColumnSortFlagById(size_t columnId, uint8_t nSortFlag);

    /** 获取某列数据的排序方式（通过列序号）
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] columnId 列的ID值
    * return 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    */
    uint8_t GetColumnSortFlag(size_t columnIndex);
    uint8_t GetColumnSortFlagById(size_t columnId);

    /** 获取表头控件接口, 在控件初始化以后才有值
    */
    ListCtrlHeader* GetHeaderCtrl() const;

public:
    /** 获取数据项总个数（数据行数）
    */
    size_t GetDataItemCount() const;

    /** 设置数据项总个数(对应行数)
    * @param [in] itemCount 数据项的总数
    */
    bool SetDataItemCount(size_t itemCount);

    /** 在最后添加一个数据项(行数+1), 数据关联到第一列（列序号为0）
    * @param [in] dataItem 数据项的内容
    * @return 成功数据项的索引号，有效范围：[0, GetDataItemCount()); 失败则返回Box::InvalidIndex
    */
    size_t AddDataItem(const ListCtrlSubItemData& dataItem);

    /** 在指定行位置添加一个数据项(行数+1)
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] dataItem 数据项的内容
    */
    bool InsertDataItem(size_t itemIndex, const ListCtrlSubItemData& dataItem);

    /** 删除指定行的数据项(行数-1)
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    bool DeleteDataItem(size_t itemIndex);

    /** 删除所有行的数据项(行数为0)
    */
    bool DeleteAllDataItems();

    /** 批量删除指定行的数据项(行数-K)
    * @param [in] itemIndexs 需要删除的数据项索引号列表，有效范围：[0, GetDataItemCount())；
    *                        重复索引与越界索引会被忽略
    * @return 返回实际删除的行数
    */
    size_t DeleteDataItems(const std::vector<size_t>& itemIndexs);

    /** 压缩数据存储：整理行身份与存储页布局，回收已删除行占用的内存
        说明：显式的整表维护操作，仅在无填充/绘制进行时调用；调用后内部已发出的单元格原始指针失效
    */
    void CompactStorage();

    /** 设置数据项的默认高度(行高)
    * @param [in] nItemHeight 高度值
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetDataItemHeight(int32_t nItemHeight, bool bNeedDpiScale);

    /** 获取数据项的默认高度(行高)
    */
    int32_t GetDataItemHeight() const;

public:
    /** 设置数据项的行属性数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    */
    bool SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData);

    /** 获取数据项的行属性数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] itemData 关联的数据
    */
    bool GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const;

    /** 设置数据项的可见性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bVisible 是否可见
    */
    bool SetDataItemVisible(size_t itemIndex, bool bVisible);

    /** 获取数据项的可见性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的可见性
    */
    bool IsDataItemVisible(size_t itemIndex) const;

    /** 设置数据项的选择属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bSelected 是否选择状态
    */
    bool SetDataItemSelected(size_t itemIndex, bool bSelected);

    /** 获取数据项的选择属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的选择状态
    */
    bool IsDataItemSelected(size_t itemIndex) const;

    /** 设置行首的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    */
    bool SetDataItemImageId(size_t itemIndex, int32_t imageId);

    /** 获取行首的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    */
    int32_t GetDataItemImageId(size_t itemIndex) const;

    /** 设置数据项的置顶状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nAlwaysAtTop 置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    */
    bool SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop);

    /** 获取数据项的置顶状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的置顶状态，-1表示不置顶, 0 或者 正数表示置顶，数值越大优先级越高，优先显示在最上面
    */
    int8_t GetDataItemAlwaysAtTop(size_t itemIndex) const;

    /** 设置数据项的行高
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nItemHeight 行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    bool SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale);

    /** 获取数据项的行高
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的行高, -1表示使用ListCtrl设置的默认行高，其他值表示本行的设置行高
    */
    int32_t GetDataItemHeight(size_t itemIndex) const;

    /** 设置数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] userData 数据项关联的自定义数据
    */
    bool SetDataItemUserData(size_t itemIndex, size_t userData);

    /** 获取数据项的自定义数据
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的自定义数据
    */
    size_t GetDataItemUserData(size_t itemIndex) const;

public:
    /** 设置指定<行,列>的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] subItemData 指定数据项的内容，列序号在dataItem.nColumnIndex中指定
    */
    bool SetSubItemData(size_t itemIndex, size_t columnIndex, const ListCtrlSubItemData& subItemData);
    bool SetSubItemDataById(size_t itemIndex, size_t columnId, const ListCtrlSubItemData& subItemData);

    /** 获取指定<行,列>的数据项
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [out] subItemData 指定数据项的内容
    */
    bool GetSubItemData(size_t itemIndex, size_t columnIndex, ListCtrlSubItemData& subItemData) const;
    bool GetSubItemDataById(size_t itemIndex, size_t columnId, ListCtrlSubItemData& subItemData) const;

public:
    /** 设置指定数据项的文本
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] text 需要设置的文本内容
    */
    bool SetSubItemText(size_t itemIndex, size_t columnIndex, const DString& text);
    bool SetSubItemTextById(size_t itemIndex, size_t columnId, const DString& text);

    /** 获取指定数据项的文本
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的文本内容
    */
    DString GetSubItemText(size_t itemIndex, size_t columnIndex) const;
    DString GetSubItemTextById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的排序分组（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] nSortGroup 需要设置的排序分组的整型数据
    */
    bool SetSubItemSortGroup(size_t itemIndex, size_t columnIndex, int32_t nSortGroup);
    bool SetSubItemSortGroupById(size_t itemIndex, size_t columnId, int32_t nSortGroup);

    /** 获取指定数据项的排序分组（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 排序分组的整型数据
    */
    int32_t GetSubItemGroup(size_t itemIndex, size_t columnIndex) const;
    int32_t GetSubItemGroupById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] userDataN 需要设置的数据项关联的整型数据
    */
    bool SetSubItemUserDataN(size_t itemIndex, size_t columnIndex, uint64_t userDataN);
    bool SetSubItemUserDataNById(size_t itemIndex, size_t columnId, uint64_t userDataN);

    /** 获取指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的整型数据
    */
    uint64_t GetSubItemUserDataN(size_t itemIndex, size_t columnIndex) const;
    uint64_t GetSubItemUserDataNById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的关联用户数据（整型）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] userDataS 需要设置的数据项关联的字符串数据
    */
    bool SetSubItemUserDataS(size_t itemIndex, size_t columnIndex, const DString& userDataS);
    bool SetSubItemUserDataSById(size_t itemIndex, size_t columnId, const DString& userDataS);

    /** 获取指定数据项的关联用户数据（字符串）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的字符串数据
    */
    DString GetSubItemUserDataS(size_t itemIndex, size_t columnIndex) const;
    DString GetSubItemUserDataSById(size_t itemIndex, size_t columnId) const;

public:
    /** 设置指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] textColor 需要设置的文本颜色
    */
    bool SetSubItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor);
    bool SetSubItemTextColorById(size_t itemIndex, size_t columnId, const UiColor& textColor);

    /** 获取指定数据项的文本颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    UiColor GetSubItemTextColor(size_t itemIndex, size_t columnIndex) const;
    UiColor GetSubItemTextColorById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的文本属性（文本对齐方式等）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] nTextFormat 需要设置的文本属性, 参见枚举类型：IRender.h中的enum DrawStringFormat
    */
    bool SetSubItemTextFormat(size_t itemIndex, size_t columnIndex, int32_t nTextFormat);
    bool SetSubItemTextFormatById(size_t itemIndex, size_t columnId, int32_t nTextFormat);

    /** 获取指定数据项的文本属性（文本对齐方式等）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 数据项关联的文本属性, 参见枚举类型：IRender.h中的enum DrawStringFormat
    */
    int32_t GetSubItemTextFormat(size_t itemIndex, size_t columnIndex) const;
    int32_t GetSubItemTextFormatById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的文本是否为单行显示（仅Report视图有效）
        说明：Report视图为虚表，单元格控件在滚动时会被复用，因此该属性存储在数据模型中，
              由框架在填充单元格数据时应用到复用的控件上；设置为多行(false)后，文本会按单元格
              宽度自动换行，此时行高需由调用方通过 SetDataItemHeight 调整，否则超出部分会被裁剪
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bSingleLine true表示单行显示（超出时按省略号裁剪），false表示多行显示（自动换行）
    */
    bool SetSubItemSingleLine(size_t itemIndex, size_t columnIndex, bool bSingleLine);
    bool SetSubItemSingleLineById(size_t itemIndex, size_t columnId, bool bSingleLine);

    /** 获取指定数据项的文本是否为单行显示（仅Report视图有效）
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return true表示单行显示，false表示多行显示（自动换行）
    */
    bool IsSubItemSingleLine(size_t itemIndex, size_t columnIndex) const;
    bool IsSubItemSingleLineById(size_t itemIndex, size_t columnId) const;

    /** 设置指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bkColor 需要设置的背景颜色
    */
    bool SetSubItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor);
    bool SetSubItemBkColorById(size_t itemIndex, size_t columnId, const UiColor& bkColor);

    /** 获取指定数据项的背景颜色
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    UiColor GetSubItemBkColor(size_t itemIndex, size_t columnIndex) const;
    UiColor GetSubItemBkColorById(size_t itemIndex, size_t columnId) const;

    /** 是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemShowCheckBox(size_t itemIndex, size_t columnIndex) const;
    bool IsSubItemShowCheckBoxById(size_t itemIndex, size_t columnId) const;

    /** 设置是否显示CheckBox
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bShowCheckBox true表示显示，false表示不显示
    */
    bool SetSubItemShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox);
    bool SetSubItemShowCheckBoxById(size_t itemIndex, size_t columnId, bool bShowCheckBox);

    /** 设置CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bChecked true表示勾选，false表示不勾选
    */
    bool SetSubItemCheck(size_t itemIndex, size_t columnIndex, bool bChecked);
    bool SetSubItemCheckById(size_t itemIndex, size_t columnId, bool bChecked);

    /** 获取CheckBox的勾选状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemChecked(size_t itemIndex, size_t columnIndex) const;
    bool IsSubItemCheckedById(size_t itemIndex, size_t columnId) const;

    /** 设置该列的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] imageId 图标资源Id，如果为-1表示行首不显示图标, 该ID由ImageList生成
    */
    bool SetSubItemImageId(size_t itemIndex, size_t columnIndex, int32_t imageId);
    bool SetSubItemImageIdById(size_t itemIndex, size_t columnId, int32_t imageId);

    /** 获取该列的图标
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    int32_t GetSubItemImageId(size_t itemIndex, size_t columnIndex) const;
    int32_t GetSubItemImageIdById(size_t itemIndex, size_t columnId) const;

    /** 设置该列的文本是否可编辑
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bEditable true表示可编辑，false表示不可编辑
    */
    bool SetSubItemEditable(size_t itemIndex, size_t columnIndex, bool bEditable);
    bool SetSubItemEditableById(size_t itemIndex, size_t columnId, bool bEditable);

    /** 获取该列的文本是否可编辑
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemEditable(size_t itemIndex, size_t columnIndex) const;
    bool IsSubItemEditableById(size_t itemIndex, size_t columnId) const;

    /** 设置该列以进度条方式显示
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] nProgress 进度值，有效范围[0, 100]，-1表示不显示进度条（按普通文本显示）
    */
    bool SetSubItemProgress(size_t itemIndex, size_t columnIndex, int32_t nProgress);
    bool SetSubItemProgressById(size_t itemIndex, size_t columnId, int32_t nProgress);

    /** 获取该列的进度值
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @return 返回进度值，有效范围[0, 100]，-1表示不显示进度条（按普通文本显示）
    */
    int32_t GetSubItemProgress(size_t itemIndex, size_t columnIndex) const;
    int32_t GetSubItemProgressById(size_t itemIndex, size_t columnId) const;

    /** 设置该列的文本是否按富文本（类HTML格式）渲染
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bRichText true表示按富文本渲染，false表示按普通文本渲染
    */
    bool SetSubItemRichText(size_t itemIndex, size_t columnIndex, bool bRichText);
    bool SetSubItemRichTextById(size_t itemIndex, size_t columnId, bool bRichText);

    /** 获取该列的文本是否按富文本（类HTML格式）渲染
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    */
    bool IsSubItemRichText(size_t itemIndex, size_t columnIndex) const;
    bool IsSubItemRichTextById(size_t itemIndex, size_t columnId) const;

public:
    /** 对数据排序
    * @param [in] columnIndex 列的索引号，有效范围：[0, GetColumnCount())
    * @param [in] columnId 列的ID
    * @param [in] bSortedUp true表示升序，false表示降序
    * @param [in] nSortFlag 排序方法标志位，参见 ListCtrlSubItemSortFlag 的枚举值
    * @param [in] pfnCompareFunc 自定义的比较函数，如果为nullptr, 则使用默认的比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    bool SortDataItems(size_t columnIndex, bool bSortedUp, uint8_t nSortFlag = ListCtrlSubItemSortFlag::kDefault,
                       ListCtrlDataCompareFunc pfnCompareFunc = nullptr,
                       void* pUserData = nullptr);
    bool SortDataItemsById(size_t columnId, bool bSortedUp, uint8_t nSortFlag = ListCtrlSubItemSortFlag::kDefault,
                           ListCtrlDataCompareFunc pfnCompareFunc = nullptr,
                           void* pUserData = nullptr);

    /** 设置外部自定义的排序函数, 替换默认的排序函数
    * @param [in] pfnCompareFunc 数据比较函数
    * @param [in] pUserData 用户自定义数据，调用比较函数的时候，通过参数传回给比较函数
    */
    void SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData);

public:
    /** 是否支持多选
    */
    bool IsMultiSelect() const;

    /** 设置是否支持多选
    */
    void SetMultiSelect(bool bMultiSelect);

    /** 批量设置选择数据项（行选择专用）
        说明：仅在行选择模式（kRow）下生效；单元格选择模式（kCell）下对行选择不做任何修改（no-op）
    * @param [in] selectedIndexs 需要设置选择的数据项索引号，有效范围：[0, GetDataItemCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    */
    void SetSelectedDataItems(const std::vector<size_t>& selectedIndexs, bool bClearOthers);

    /** 获取选择的元素列表（行选择专用）
        说明：仅在行选择模式（kRow）下有结果；单元格选择模式（kCell）下返回空列表
    * @param [in] itemIndexs 返回当前选择的数据项索引号，有效范围：[0, GetDataItemCount())
    */
    void GetSelectedDataItems(std::vector<size_t>& itemIndexs) const;

    /** 选择全部（按当前选择模式分派：行选择模式全选行，单元格选择模式全选单元格）
    */
    void SetSelectAll();

    /** 取消所有选择（按当前选择模式分派：行选择模式清空行选择，单元格选择模式清空单元格选择）
    */
    void SetSelectNone();

public:
    /** 获取选择模式（行选择模式 / 单元格选择模式）
    */
    ListCtrlSelectionMode GetSelectionMode() const;

    /** 设置选择模式（默认是行选择模式）
        说明：切换到单元格选择模式时，会自动清除当前的行选择；切回行选择模式时，会自动清除单元格选择
    * @param [in] mode 选择模式
    */
    void SetSelectionMode(ListCtrlSelectionMode mode);

    /** 获取当前单元格（单元格选择模式下有效，无效时返回的ListCtrlCellId::IsValid()为false）
    */
    ListCtrlCellId GetCurrentCell() const;

    /** 设置当前单元格（单元格选择模式下有效；不能用于行选择模式）
    * @param [in] nRowIndex 行的显示索引号，有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID值
    * @return 返回是否设置成功（行索引或列ID无效，或者当前不是单元格选择模式时返回false）
    */
    bool SetCurrentCell(size_t nRowIndex, size_t nColumnId);

    /** 设置单元格的选择区域（锚点 + 焦点，构成规范化矩形）
    * @param [in] anchorCell 锚点单元格
    * @param [in] focusCell 焦点单元格（为无效值时，仅设置锚点单元格，不改变选中集合）
    * @param [in] bAdditive true=向已有选择追加该矩形区域（不清除已有）；false=先清除已有单元格选择再设置
    * @return 返回是否设置成功（锚点无效或当前不是单元格选择模式时返回false）
    */
    bool SetCellSelection(const ListCtrlCellId& anchorCell, const ListCtrlCellId& focusCell, bool bAdditive);

    /** 清除单元格的选择区域
        仅在确实存在选择状态（有选中单元格或当前格）时才刷新界面并派发 kEventCellSelectionChanged 事件
    */
    void ClearCellSelection();

    /** 获取单元格的选择区域（锚点 + 焦点，未做规范化）
    */
    void GetCellSelection(ListCtrlCellId& anchorCell, ListCtrlCellId& focusCell) const;

    /** 批量设置选择的单元格（单元格选择等价 API，对应行选择的 SetSelectedDataItems）
        说明：仅在单元格选择模式（kCell）下生效；行选择模式（kRow）下不做任何修改（no-op）
    * @param [in] cellList 需要设置为选中状态的单元格列表
    * @param [in] bClearOthers 如果为true，表示先清除其它已选中的单元格，只保留本次设置的
    */
    void SetSelectedCells(const std::vector<ListCtrlCellId>& cellList, bool bClearOthers);

    /** 获取选择的单元格列表（规范化区域内展开的单元格，按行优先顺序）
        说明：跳过被合并覆盖的单元格（当前尚未实现单元格合并，后续合并功能落地后生效）
    * @param [in] cellList 返回选择的单元格列表
    * @return 返回选择的单元格个数
    */
    size_t GetSelectedCells(std::vector<ListCtrlCellId>& cellList) const;

    /** 判断一个单元格是否在选择的区域内
    * @param [in] nRowIndex 行的显示索引号，有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID值
    */
    bool IsCellSelected(size_t nRowIndex, size_t nColumnId) const;

    /** 判断一个单元格是否为当前单元格
    * @param [in] nRowIndex 行的显示索引号，有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID值
    */
    bool IsCellCurrent(size_t nRowIndex, size_t nColumnId) const;

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

    /** 刷新界面，保持数据与显示同步
    * @param [in] bSync true表示同步刷新，立即重绘当前视图; false表示异步刷新，刷新动作为异步重绘，标记为脏区域，交给系统重绘界面
    */
    void Refresh(bool bSync = false);

    /** 刷新指定数据项
    * @param [in] dataItemIndexs 需要刷新的数据项索引号, 有效范围：[0, GetDataItemCount())
    */
    void RefreshDataItems(const std::vector<size_t>& dataItemIndexs);

    /** 刷新指定单元格（单格刷新，仅Report视图有效；用于进度、状态类列的局部更新，避免整行刷新）
    * @param [in] dataItemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnIndex 列索引序号, 有效范围：[0, GetColumnCount())
    * @return 返回true表示更新成功（该单元格当前在可见范围内并已完成刷新）；
    *         如果返回false，表示参数无效，或者该单元格当前不在可见范围内（数据本身已更新，滚动到可见范围后会显示最新数据）
    */
    bool RefreshSubItem(size_t dataItemIndex, size_t columnIndex);

    /** 刷新指定单元格（单格刷新，仅Report视图有效；用于进度、状态类列的局部更新，避免整行刷新）
    * @param [in] dataItemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColumnId()函数获取
    * @return 返回true表示更新成功（该单元格当前在可见范围内并已完成刷新）；
    *         如果返回false，表示参数无效，或者该单元格当前不在可见范围内（数据本身已更新，滚动到可见范围后会显示最新数据）
    */
    bool RefreshSubItemById(size_t dataItemIndex, size_t columnId);

    /** 是否允许刷新界面
    * @param [in] bEnable 如果为true，允许通过Refresh系列函数刷新界面; 如果为false，则禁止通过Refresh系列函数刷新界面，
                          并且数据变化的时候也不会自动刷新界面
    * @return 返回旧的IsEnableRefresh()状态
    */
    bool SetEnableRefresh(bool bEnable);

    /** 判断是否允许刷新界面
    */
    bool IsEnableRefresh() const;

public:
    /** 获取列表数据展示接口(Report视图)
    */
    ListCtrlReportView* GetReportView() const;

    /** 获取列表数据展示接口(Icon视图)
    */
    ListCtrlIconView* GetIconView() const;

    /** 获取列表数据展示接口(List视图)
    */
    ListCtrlListView* GetListView() const;

public:
    /** 获取第一个数据项的界面控件接口（Report视图）
    * @return 返回界面控件接口
              注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlItem* GetFirstDisplayItem() const;

    /** 获取下一个数据项的界面控件接口（Report视图）
    * @param [in] pItem 当前界面控件接口
    * @return 返回当前界面控件的下一个界面控件接口，如果当前已经是最后一个，返回nullptr
    *         注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlItem* GetNextDisplayItem(ListCtrlItem* pItem) const;

public:
    /** 获取第一个数据项的界面控件接口（Icon视图）
    * @return 返回界面控件接口
              注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlIconViewItem* GetFirstDisplayIconItem() const;

    /** 获取下一个数据项的界面控件接口（Icon视图）
    * @param [in] pItem 当前界面控件接口
    * @return 返回当前界面控件的下一个界面控件接口，如果当前已经是最后一个，返回nullptr
    *         注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlIconViewItem* GetNextDisplayIconItem(ListCtrlIconViewItem* pItem) const;

public:
    /** 获取第一个数据项的界面控件接口（Icon视图）
    * @return 返回界面控件接口
              注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlListViewItem* GetFirstDisplayListItem() const;

    /** 获取下一个数据项的界面控件接口（Icon视图）
    * @param [in] pItem 当前界面控件接口
    * @return 返回当前界面控件的下一个界面控件接口，如果当前已经是最后一个，返回nullptr
    *         注意事项：界面刷新后，界面控件与数据元素之间的关系会动态变化
    */
    ListCtrlListViewItem* GetNextDisplayListItem(ListCtrlListViewItem* pItem) const;

public:
    /** 设置是否支持子项编辑
    */
    void SetEnableItemEdit(bool bEnableItemEdit);

    /** 判断是否支持子项编辑
    */
    bool IsEnableItemEdit() const;

    /** 设置是否支持复制选中内容到剪贴板（Ctrl+C，TSV 格式；默认 true）
    */
    void SetEnableCopy(bool bEnableCopy);

    /** 判断是否支持复制选中内容到剪贴板
    */
    bool IsEnableCopy() const;

    /** 设置是否支持 CSV 导出（Ctrl+Shift+C 复制为 CSV，及 ExportToCsvFile；默认 true）
    */
    void SetEnableCsvExport(bool bEnableCsvExport);

    /** 判断是否支持 CSV 导出
    */
    bool IsEnableCsvExport() const;

    /** 复制当前选中内容到剪贴板
    * @param [in] bCsvFormat true 表示 CSV 格式（受 enable_csv_export 开关控制），
    *                       false 表示 TSV 格式（受 enable_copy 开关控制）
    * @return 返回 true 表示复制成功；开关关闭、无选中内容、剪贴板写入失败时返回 false
    */
    bool CopySelectionToClipboard(bool bCsvFormat = false);

    /** 导出为 CSV 文件（UTF-8 BOM 编码，便于 Excel 直接打开）
    * @param [in] strFilePath CSV 文件路径（由应用提供，绝对路径）
    * @param [in] bSelectedOnly true 表示仅导出当前选中内容，false 表示导出整表
    * @return 返回 true 表示导出成功；enable_csv_export 开关关闭、无数据、写文件失败时返回 false
    */
    bool ExportToCsvFile(const DString& strFilePath, bool bSelectedOnly = true);

public:
    /** @name 单元格编辑（kCell 模式；仅 Report 视图）
    * @{ */

    /** 按列的序号设置编辑器类型（作用于该列的所有单元格）
    * @param [in] columnIndex 列的序号，有效范围：[0, GetColumnCount())
    * @param [in] editorType 编辑器类型（文本/数值/日期）
    */
    void SetColumnEditorType(size_t columnIndex, ListCtrlEditorType editorType);

    /** 按列的ID设置编辑器类型（推荐：列ID稳定，列序可变）
    * @param [in] columnId 列的ID
    * @param [in] editorType 编辑器类型
    */
    void SetColumnEditorTypeById(size_t columnId, ListCtrlEditorType editorType);

    /** 获取某列序号的编辑器类型（未设置时为 kEditText）
    */
    ListCtrlEditorType GetColumnEditorType(size_t columnIndex) const;

    /** 获取某列ID的编辑器类型（未设置时为 kEditText）
    */
    ListCtrlEditorType GetColumnEditorTypeById(size_t columnId) const;

    /** 按列的序号设置候选值列表（kEditCombo 编辑器使用）
    * @param [in] columnIndex 列的序号，有效范围：[0, GetColumnCount())
    * @param [in] candidates 候选值列表（纯文本，按传入顺序显示）；传入空列表表示清空候选
    */
    void SetColumnCandidates(size_t columnIndex, const std::vector<DString>& candidates);

    /** 按列的ID设置候选值列表（推荐：列ID稳定，列序可变）
    * @param [in] columnId 列的ID
    * @param [in] candidates 候选值列表（纯文本，按传入顺序显示）；传入空列表表示清空候选
    */
    void SetColumnCandidatesById(size_t columnId, const std::vector<DString>& candidates);

    /** 获取某列序号的候选值列表（未设置时返回空列表）
    */
    std::vector<DString> GetColumnCandidates(size_t columnIndex) const;

    /** 获取某列ID的候选值列表（未设置时返回空列表）
    */
    std::vector<DString> GetColumnCandidatesById(size_t columnId) const;

    /** 进入指定单元格的编辑状态（等同于对当前格按 F2）
    * 前置条件：enable_item_edit 为 true、kCell 模式、Report 视图、该单元格"可编辑"且所在行可选；
    *           行不在可视区时先滚动到可视区
    * @param [in] nRowIndex 行的显示索引号，有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID
    * @return 返回 true 表示已进入编辑状态（或已在编辑中）
    */
    bool EnterCellEdit(size_t nRowIndex, size_t nColumnId);

    /** 对"当前单元格"进入编辑状态（F2 的实现入口）
    * @return 返回 true 表示已进入编辑状态
    */
    bool EnterCurrentCellEdit();

    /** 判断当前是否处于单元格编辑状态（kCell 模式）
    */
    bool IsCellEditing() const;

    /** 获取当前正在编辑的单元格
    * @param [out] cell 返回单元格定位（无编辑态时 nRowIndex / nColumnId 为 Box::InvalidIndex）
    * @return 返回是否处于单元格编辑状态
    */
    bool GetEditingCell(ListCtrlCellId& cell) const;

    /** @} */

public:
    /** 获取编辑框控件的接口(只有执行编辑操作以后，才有值)
    */
    RichEdit* GetRichEdit() const;

public:
    /** @name CheckBox（行级）关联函数
    * @{ */

    /** 设置是否自动勾选选择的数据项(作用于Header与每行)
    */
    void SetAutoCheckSelect(bool bAutoCheckSelect);

    /** 获取是否自动勾选选择的数据项
    */
    bool IsAutoCheckSelect() const;

    /** 设置是否在表头最左侧显示CheckBox
    * @param [in] bShow true表示显示CheckBox，false表示不显示
    */
    bool SetHeaderShowCheckBox(bool bShow);

    /** 判断是否在每行行首显示CheckBox
    */
    bool IsHeaderShowCheckBox() const;

    /** 设置是否在每行行首显示CheckBox
    * @param [in] bShow true表示显示CheckBox，false表示不显示
    */
    void SetDataItemShowCheckBox(bool bShow);

    /** 判断是否在每行行首显示CheckBox
    */
    bool IsDataItemShowCheckBox() const;

    /** 设置数据项的勾选属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] bCheck 是否勾选状态
    */
    bool SetDataItemCheck(size_t itemIndex, bool bCheck);

    /** 获取数据项的勾选属性
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @return 返回数据项关联的勾选状态
    */
    bool IsDataItemCheck(size_t itemIndex) const;

    /** 批量设置勾选数据项（行首的CheckBox打勾的数据）
    * @param [in] itemIndexs 需要设置勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    */
    void SetCheckedDataItems(const std::vector<size_t>& itemIndexs, bool bClearOthers);

    /** 获取勾选的元素列表（行首的CheckBox打勾的数据）
    * @param [in] itemIndexs 返回当前勾选的数据项索引号，有效范围：[0, GetDataItemCount())
    */
    void GetCheckedDataItems(std::vector<size_t>& itemIndexs) const;

    /** @} */

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
    * @param [in] fLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(float fLineWidth, bool bNeedDpiScale);
    float GetColumnGridLineWidth() const;

    /** 纵向网格线的颜色
    * @param [in] color 纵向网格线的颜色
    */
    void SetColumnGridLineColor(const DString& color);
    DString GetColumnGridLineColor() const;

    /** 是否支持双击Header的分割条自动调整列宽
    */
    void SetEnableColumnWidthAuto(bool bEnable);
    bool IsEnableColumnWidthAuto() const;

    /** 获取当前排序列的ID和排序方式
    * @param [out] nSortColumnId 排序列的ID
    * @param [out] bSortUp 当前排序是否为升序排列，true表示升序，false表示降序
    * @return true表示有排序列，false表示无排序列
    */
    bool GetSortColumn(size_t& nSortColumnId, bool& bSortUp) const;

public:
    /** 监听选择子项的事件
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachSelect(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSelect, callback, callbackID); }

    /** 监听选择项发生变化的事件
     * @param[in] callback 选择子项时的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachSelChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSelChanged, callback, callbackID); }

    /** 监听双击事件
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachDoubleClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventMouseDoubleClick, callback, callbackID); }

    /** 绑定鼠标点击处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventClick, callback, callbackID); }

    /** 绑定鼠标右键点击处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachRClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventRClick, callback, callbackID); }

    /** 绑定鼠标进入ListCtrlItem/ListCtrlListViewItem/ListCtrlIconViewItem控件的处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachItemMouseEnter(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventItemMouseEnter, callback, callbackID); }

    /** 绑定鼠标离开ListCtrlItem/ListCtrlListViewItem/ListCtrlIconViewItem控件的处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachItemMouseLeave(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventItemMouseLeave, callback, callbackID); }

    /** 绑定鼠标进入ListCtrlSubItem控件的处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   listCtrlType: 列表类型，有效的取值应为: ListCtrlType::Report
     *   pEventData: 选择子项的界面控件接口指针，类型为：ListCtrlSubItem*指针
     */
    void AttachSubItemMouseEnter(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSubItemMouseEnter, callback, callbackID); }

    /** 绑定鼠标离开ListCtrlSubItem控件的处理函数
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   listCtrlType: 列表类型，有效的取值应为: ListCtrlType::Report
     *   pEventData: 选择子项的界面控件接口指针，类型为：ListCtrlSubItem*指针
     */
    void AttachSubItemMouseLeave(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSubItemMouseLeave, callback, callbackID); }

    /** 监听回车事件
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachReturn(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventReturn, callback, callbackID); }

    /** 监听键盘按下事件
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachKeyDown(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventKeyDown, callback, callbackID); }

    /** 监听键盘弹起事件
     * @param [in] callback 事件处理的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项     
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *                 如果listCtrlType为其他值，表示该事件未关联子项数据
     *   pEventData: 选择子项的界面控件接口指针，可能的类型为：ListCtrlItem*指针, ListCtrlIconViewItem*指针, ListCtrlListViewItem*指针
     *               可将listCtrlType的值转换为ListCtrlType，然后根据枚举值来判断具体应转换为哪种类型
     */
    void AttachKeyUp(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventKeyUp, callback, callbackID); }

    /** 监听开始编辑事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 是接口指针：ListCtrlEditParam*，可以通过设置bCancelled取消操作
     */
    void AttachEnterEdit(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventEnterEdit, callback, callbackID); }

    /** 监听结束编辑事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 是接口指针：ListCtrlEditParam*，可以通过设置bCancelled取消操作
     */
    void AttachLeaveEdit(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventLeaveEdit, callback, callbackID); }

    /** 监听数据项总个数变化事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 是新的个数(size_t)
     *   lParam: 是旧的个数(size_t)
     */
    void AttachDataItemCountChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventDataItemCountChanged, callback, callbackID); }

    /** 监听Report视图数据项UI元素填充事件（虚表）
     *  触发场景：整行填充数据时触发一次；调用RefreshSubItem单格刷新时也会触发一次
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlItem*指针，数据按行填充
     *   pExtraData: 关联的单元格指针，类型为：ListCtrlSubItem*指针；为nullptr表示填充了本行的所有单元格；非nullptr表示只填充了该单元格（单格刷新）
     */
    void AttachReportViewItemFilled(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventReportViewItemFilled, callback, callbackID); }

    /** 监听List视图数据项UI元素填充事件（虚表）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlListViewItem*指针
     */
    void AttachListViewItemFilled(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventListViewItemFilled, callback, callbackID); }

    /** 监听Icon视图数据项UI元素填充事件（虚表）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlIconViewItem*指针
     */
    void AttachIconViewItemFilled(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventIconViewItemFilled, callback, callbackID); }

    /** 监听列表类型变化事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 当前列表视图类型，参见listCtrlType的枚举值
     *   lParam: 原来的列表视图类型，参见listCtrlType的枚举值
     *   pEventData: 当前列表视图的指针：ListCtrlReportView*, ListCtrlIconView*, ListCtrlListView*
     */
    void AttachViewTypeChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventViewTypeChanged, callback, callbackID); }

    /** 监听列表位置变化事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *   pEventData: 当前列表视图的指针：ListCtrlReportView*, ListCtrlIconView*, ListCtrlListView*
     */
    void AttachViewPosChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventViewPosChanged, callback, callbackID); }

    /** 监听列表大小变化事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   listCtrlType: 列表类型，有效的取值为: ListCtrlType::Report, ListCtrlType::Icon, ListCtrlType::List
     *   pEventData: 当前列表视图的指针：ListCtrlReportView*, ListCtrlIconView*, ListCtrlListView*
     */
    void AttachViewSizeChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventViewSizeChanged, callback, callbackID); }

    /** 监听列宽变化事件（拖动列表头调整列宽，或者调用SetColumnWidth等接口时触发）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 列ID(size_t)
     *   lParam: 关联的另一个列ID(size_t)，如果是拖动调整列宽，可能是相邻列；如果为Box::InvalidIndex表示无关联列
     */
    void AttachColumnWidthChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventColumnWidthChanged, callback, callbackID); }

    /** 监听列顺序变化事件（拖动列表头调整列的顺序时触发）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam/lParam: 未使用，可通过GetColumnId等接口获取最新的列顺序
     */
    void AttachColumnOrderChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventColumnOrderChanged, callback, callbackID); }

    /** 监听列显示/隐藏状态变化事件
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 列ID(size_t)
     *   lParam: 新状态(1表示显示，0表示隐藏)
     */
    void AttachColumnVisibleChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventColumnVisibleChanged, callback, callbackID); }

    /** 监听列排序变化事件（点击列表头排序，或者调用SortDataItems等接口时触发）
     * @param [in] callback 要绑定的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明:
     *   wParam: 列ID(size_t)
     *   lParam: 新状态(1表示升序，0表示降序)
     */
    void AttachColumnSorted(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventColumnSorted, callback, callbackID); }

    /** 绑定当前单元格发生变化的事件（单元格选择模式）
    * @param [in] callback 要绑定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明:
    *   wParam: 行的显示索引(size_t)
    *   lParam: 列ID(size_t)
    */
    void AttachCurrentCellChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventCurrentCellChanged, callback, callbackID); }

    /** 绑定单元格选择区域发生变化的事件（单元格选择模式）
    * @param [in] callback 要绑定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明:
    *   wParam: 选中区域个数(size_t)，0表示当前无选择
    *   lParam: 0（保留）
    *   pEventData: 指向 std::vector<ui::ListCtrlCellRange>*（选中的区域列表，可能多项；无选择时为nullptr）
    */
    void AttachCellSelectionChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventCellSelectionChanged, callback, callbackID); }

protected:
    /** 控件初始化
    */
    virtual void OnInit() override;

    /** 初始化Report视图
    */
    void InitReportView();

    /** 初始化Icon视图
    */
    void InitIconView();

    /** 初始化List视图
    */
    void InitListView();

    /** 设置ListCtrlHeader的属性Class
    */
    void SetHeaderClass(const DString& className);

    /** ListCtrlHeaderItem的属性Class
    */
    void SetHeaderItemClass(const DString& className);
    DString GetHeaderItemClass() const;

    /** ListCtrlHeader/SplitBox的属性Class
    */
    void SetHeaderSplitBoxClass(const DString& className);
    DString GetHeaderSplitBoxClass() const;

    /** ListCtrlHeader/SplitBox/Control的属性Class
    */
    void SetHeaderSplitControlClass(const DString& className);
    DString GetHeaderSplitControlClass() const;

    /** CheckBox的Class属性(应用于Header和ListCtrl数据)
    */
    void SetCheckBoxClass(const DString& className);
    DString GetCheckBoxClass() const;

    /** 数据Report视图中的ListBox的Class属性
    */
    void SetReportViewClass(const DString& className);
    DString GetReportViewClass() const;

    /** ListCtrlItem的Class属性
    */
    void SetDataItemClass(const DString& className);
    DString GetDataItemClass() const;

    /** ListCtrlItem/ListCtrlSubItem的Class属性
    */
    void SetDataSubItemClass(const DString& className);
    DString GetDataSubItemClass() const;

    /** 数据Icon视图中的ListBox的Class属性
    */
    void SetIconViewClass(const DString& className);
    DString GetIconViewClass() const;

    /** 数据Icon视图中的ListBox的子项Class属性
    */
    void SetIconViewItemClass(const DString& className);
    DString GetIconViewItemClass() const;

    /** 数据Icon视图中的ListBox的子项中图片的Class属性
    */
    void SetIconViewItemImageClass(const DString& className);
    DString GetIconViewItemImageClass() const;

    /** 数据Icon视图中的ListBox的子项中Label的Class属性
    */
    void SetIconViewItemLabelClass(const DString& className);
    DString GetIconViewItemLabelClass() const;

    /** 数据List视图中的ListBox的Class属性
    */
    void SetListViewClass(const DString& className);
    DString GetListViewClass() const;

    /** 数据List视图中的ListBox的子项Class属性
    */
    void SetListViewItemClass(const DString& className);
    DString GetListViewItemClass() const;

    /** 数据List视图中的ListBox的子项的图片的Class属性
    */
    void SetListViewItemImageClass(const DString& className);
    DString GetListViewItemImageClass() const;

    /** 数据List视图中的ListBox的子项的Label的Class属性
    */
    void SetListViewItemLabelClass(const DString& className);
    DString GetListViewItemLabelClass() const;

    /** 编辑框的Class属性
    */
    void SetRichEditClass(const DString& richEditClass);
    DString GetRichEditClass() const;

    /** 日期编辑框的Class属性（kEditDate 编辑器使用的 DateTime 控件）
    */
    void SetDateTimeClass(const DString& dateTimeClass);
    DString GetDateTimeClass() const;

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

    /** 表头的CheckBox勾选操作(列级)
    * @param [in] nColumnId 列的ID
    * @param [in] bChecked true表示勾选（Checked状态），false表示取消勾选（UnChecked状态）
    */
    void OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked);

    /** 表头的CheckBox勾选操作(行级)
    * @param [in] bChecked true表示勾选（Checked状态），false表示取消勾选（UnChecked状态）
    */
    void OnHeaderCheckStateChanged(bool bChecked);

    /** 表头列的显示属性发生变化
    * @param [in] nColumnId 列的ID
    */
    void OnHeaderColumnVisibleChanged(size_t nColumnId);

    /** 列表头列的分割条双击事件
    * @param [in] pHeaderItem 列表头控件接口
    */
    void OnHeaderColumnSplitDoubleClick(ListCtrlHeaderItem* pHeaderItem);

    /** 同步UI的Check状态(列级别的CheckBox)
    * @param [in] nColumnId 列ID, 如果为Box::InvalidIndex表示更新所有列
    */
    void UpdateHeaderColumnCheckBox(size_t nColumnId);

    /** 同步UI的Check状态(行级别的CheckBox)
    */
    void UpdateHeaderCheckBox();

private:
    /** 列表事件，转发给应用层
    */
    void OnListCtrlViewEvent(ListCtrlType listCtrlType, const EventArgs& args);

    /** 进入编辑状态
    * @param [in] itemIndex 数据项的索引号, 有效范围：[0, GetDataItemCount())
    * @param [in] nColumnId 列的ID
    * @param [in] pItem 列表项的接口
    * @param [in] pSubItem 列表项子项的接口
    */
    void OnItemEnterEditMode(size_t itemIndex, size_t nColumnId,
                             IListBoxItem* pItem, ListCtrlLabel* pSubItem);

    /** 进入编辑状态的定时器取消机制
    */
    WeakCallbackFlag m_editModeFlag;

    /** 从定时器进入编辑状态
    */
    void OnItemEditMode(ListCtrlEditParam editParam);

    /** 校验编辑状态的输入参数，是否有效
    */
    bool IsValidItemEditParam(const ListCtrlEditParam& editParam) const;

    /** 校验是否满足可编辑状态
    */
    bool IsValidItemEditState(const ListCtrlEditParam& editParam) const;

    /** 更新RichEdit控件的位置和大小
    */
    void UpdateRichEditSize(ListCtrlLabel* pSubItem);

    /** 更新DateTime控件的位置和大小（kEditDate 编辑器）
    */
    void UpdateDateTimeSize(ListCtrlLabel* pSubItem);

    /** 子项编辑事件
    * @return 返回 true 表示已写回数据（用户未取消、且输入通过编辑器类型校验）
    */
    bool OnItemEdited(const ListCtrlEditParam& editParam, const DString& newItemText);

    /** 获取指定显示索引号对应的数据行控件（Report 视图；仅"当前可视"的行，滚动复用后不可缓存）
    * @param [in] nItemIndex 行的显示索引号
    * @return 返回数据行控件（不可视时返回nullptr）
    */
    ListCtrlItem* GetDisplayItemByIndex(size_t nItemIndex) const;

    /** 判断编辑器输入的文本是否符合列声明的编辑器类型（空串一律视为有效，表示清空单元格）
    */
    static bool IsValidCellEditText(const DString& strNewText, ListCtrlEditorType editorType);

    /** 进入下拉候选编辑状态（kEditCombo 编辑器）：在单元格下方弹出候选浮层
    * @param [in] editParam 编辑参数（进入编辑时创建）
    * @param [in] pSubItem 命中单元格的文本控件（作为浮层锚点）
    */
    void EnterComboEditMode(const ListCtrlEditParam& editParam, ListCtrlLabel* pSubItem);

    /** 进入日期编辑状态（kEditDate 编辑器）：在单元格上浮出 DateTime 控件，聚焦后弹出月日历
    * @param [in] editParam 编辑参数（进入编辑时创建）
    * @param [in] pSubItem 命中单元格的文本控件
    */
    void EnterDateEditMode(const ListCtrlEditParam& editParam, ListCtrlLabel* pSubItem);

    /** 下拉候选编辑：候选项被选中（鼠标点选/键盘回车）后提交
    * @param [in] nIndex 候选项序号
    * @return 返回 true 表示已提交写回
    */
    bool OnComboCandidateSelected(size_t nIndex);

    /** 关闭下拉候选浮层（未打开时为空操作）
    */
    void CloseComboFlyout();

    /** 判断下拉候选浮层是否处于打开状态
    */
    bool IsComboEditOpened() const;

    /** 提交成功后，把当前格下移一行（kCell 模式；已在最后一行则不移动）
    */
    void MoveCurrentCellToNextRow();

    /** 视图接收到鼠标消息
    */
    void OnViewMouseEvents(const EventArgs& msg);

    /** 视图接收到鼠标消息
    */
    void OnViewKeyboardEvents(const EventArgs& msg);

    /** 结束编辑状态
    */
    void LeaveEditMode();

    /** 清除所有编辑状态的事件监听
    */
    void ClearEditEvents();

    /** 设置表格类型（默认为Report类型）
    * @param [in] type 视图类型
    * @param [in] bInit true表示初始化阶段调用
    */
    void DoSetListCtrlType(ListCtrlType type, bool bInit);

    /** 单元格选择模式：收集当前选中的单元格区域列表（可能多项，用于事件上报）
        归并规则：同一显示行内按"列索引连续"合并为横向区段，纵向相邻行的相同区段再合并为矩形
    * @param [out] rangeList 返回选中区域列表（无选择时为空）
    */
    void GetSelectedCellRanges(std::vector<ListCtrlCellRange>& rangeList) const;

    /** 生成选中内容（或整表）的文本
    * @param [in] bCsvFormat true 表示 CSV 格式（符合 RFC4180），false 表示 TSV 格式（制表符分隔）
    * @param [in] bSelectedOnly true 表示仅导出当前选中内容，false 表示导出整表（仅 CSV 导出路径使用）
    * @return 返回生成的文本（无内容时返回空串）
    */
    DString BuildSelectionText(bool bCsvFormat, bool bSelectedOnly) const;

    /** 转义单个字段的内容
    * @param [in] strText 原始字段文本
    * @param [in] bCsvFormat true 表示 CSV 格式（按需加双引号，并翻倍内部双引号）；
    *                         false 表示 TSV 格式（把制表符/回车/换行替换为空格）
    * @return 返回转义后的文本
    */
    static DString EscapeText(const DString& strText, bool bCsvFormat);

    /** 单元格选择模式：派发"当前单元格发生变化"事件
    */
    void NotifyCurrentCellChanged(const ListCtrlCellId& cell);

    /** 单元格选择模式：派发"选择区域发生变化"事件
    */
    void NotifyCellSelectionChanged();

    /** 单元格选择模式：刷新视图显示（重绘单元格高亮与当前格焦点框）
    */
    void RefreshCellSelectionView();

    /** 单元格选择模式：更新鼠标悬停的单元格（用于绘制单元格悬停高亮）
    * @param [in] nRowIndex 行的显示索引号（未命中单元格时为 Box::InvalidIndex）
    * @param [in] nColumnId 列的ID（未命中单元格时为 Box::InvalidIndex）
    */
    void UpdateHoverCell(size_t nRowIndex, size_t nColumnId);

    /** 单元格选择模式：清除鼠标悬停的单元格（鼠标移出控件时调用，避免悬停高亮残留）
    */
    void ClearHoverCell();

    /** 单元格选择模式：判断指定单元格是否为鼠标悬停的单元格
    * @param [in] nRowIndex 行的显示索引号
    * @param [in] nColumnId 列的ID
    */
    bool IsCellHovered(size_t nRowIndex, size_t nColumnId) const;

    /** 单元格选择模式：处理来自 ListCtrlItem 的鼠标事件（设置/扩展单元格选择）
    * @param [in] msg 鼠标事件消息
    * @param [in] nRowIndex 行的显示索引号
    * @param [in] nColumnId 列的ID（未命中单元格时为 Box::InvalidIndex）
    * @return 返回是否已处理（true表示已按单元格选择处理，调用方不应再做行选择）
    */
    bool OnCellMouseEvent(const EventArgs& msg, size_t nRowIndex, size_t nColumnId);

    /** 单元格选择模式：鼠标按下时开始一次拖动框选（左键/右键共用）
        普通单击选中单元格（清除其它）；Shift 扩展到点击格；Ctrl 切换选中；Ctrl+Shift 追加扩展；
        未命中单元格（空白区域）则清空单元格选择；结束后置位拖动框选状态（框选矩形驱动后续选择）
    * @param [in] msg 鼠标按下事件消息
    * @param [in] nRowIndex 行的显示索引号（未命中单元格时为 Box::InvalidIndex）
    * @param [in] nColumnId 列的ID（未命中单元格时为 Box::InvalidIndex）
    */
    void BeginCellDrag(const EventArgs& msg, size_t nRowIndex, size_t nColumnId);

    /** 单元格选择模式：处理键盘事件（方向键移动当前格、Shift扩展区域、Ctrl+A全选等）
    * @param [in] msg 键盘事件消息
    * @return 返回是否已处理
    */
    bool OnCellKeyDown(const EventArgs& msg);

    /** 处理下拉候选浮层的键盘事件（行选择模式与单元格选择模式通用）
    *  浮层处于打开状态时：上/下方向键移动候选项、回车提交、Esc取消（均消费按键）
    * @param [in] msg 键盘事件消息
    * @return 返回是否已处理（浮层未打开时返回false，按键继续按常规流程处理）
    */
    bool OnComboKeyDown(const EventArgs& msg);

private:
    /** 表头控件
    */
    ListCtrlHeader* m_pHeaderCtrl;

    /** 列表数据展示(Report视图)
    */
    ListCtrlReportView* m_pReportView;

    /** 列表数据展示(Icon视图)
    */
    ListCtrlIconView* m_pIconView;

    /** 列表数据展示(List视图)
    */
    ListCtrlListView* m_pListView;

    /** 列表数据管理
    */
    ListCtrlData* m_pData;

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

    /** ListCtrlItem的Class属性
    */
    UiString m_dataItemClass;

    /** ListCtrlItem/ListCtrlSubItem的Class属性
    */
    UiString m_dataSubItemClass;

    /** ListBox的Report视图Class属性
    */
    UiString m_reportViewClass;

    /** ListBox的Icon视图Class属性(Icon视图)
    */
    UiString m_iconViewClass;

    /** ListBox的Icon视图子项Class属性(Icon视图)
    */
    UiString m_iconViewItemClass;

    /** ListBox的Icon视图图标子项Class属性(Icon视图)
    */
    UiString m_iconViewItemImageClass;

    /** ListBox的Icon视图文本子项Class属性(Icon视图)
    */
    UiString m_iconViewItemLabelClass;

    /** ListBox的List视图Class属性(List视图)
    */
    UiString m_listViewClass;

    /** ListBox的List视图Class属性(List视图)
    */
    UiString m_listViewItemClass;

    /** ListBox的List视图图标子项Class属性(List视图)
    */
    UiString m_listViewItemImageClass;

    /** ListBox的List视图文本子项Class属性(List视图)
    */
    UiString m_listViewItemLabelClass;

    /** ListCtrl的编辑框Class属性
    */
    UiString m_listCtrlRichEditClass;

    /** ListCtrl的日期编辑框Class属性（kEditDate 编辑器）
    */
    UiString m_listCtrlDateTimeClass;

    /** 富文本编辑框类中声明的字符白名单（limit_chars）
    *  说明：编辑框控件会被复用，数值编辑器进入编辑时会临时改写白名单；
    *  为避免抹掉控件类上声明的白名单配置，首次创建编辑框时记录该值，切换回非数值编辑器时恢复。
    */
    DString m_richEditLimitChars;

    /** 图片列表
    */
    ImageListPtr m_imageList[3];

    /** 编辑框控件
    */
    RichEdit* m_pRichEdit;

    /** 日期编辑框控件（kEditDate 编辑器）
    */
    DateTime* m_pDateTime;

    /** 表头的高度
    */
    int32_t m_nHeaderHeight;

    /** 数据项的高度
    */
    int32_t m_nItemHeight;

    /** 当前排序列的ID
    */
    size_t m_nSortedColumnId;

    /** 每列的排序方式标志位映射表(列ID -> 排序方式)
    */
    std::map<size_t, uint8_t> m_columnSortFlagMap;

    /** 当前排序是否为升序排列
    */
    bool m_bSortedUp;

    /** 表格类型（默认为Report类型）
    */
    ListCtrlType m_listCtrlType;

    /** 是否支持拖动改变列的顺序
    */
    bool m_bEnableHeaderDragOrder;

    /** 冻结列的个数（固定列表左侧的前N列，不随横向滚动而移动）
    */
    size_t m_nFrozenColumnCount;

    /** 是否显示表头控件
    */
    bool m_bShowHeaderCtrl;

    /** 是否允许刷新界面
    */
    bool m_bEnableRefresh;

    /** 是否支持多选(默认是单选)
    */
    bool m_bMultiSelect;

    /** 选择模式（默认是行选择模式）
    */
    ListCtrlSelectionMode m_selectionMode;

    /** 单元格选择模式下是否正在通过鼠标拖动扩展选择区域
    */
    bool m_bCellDragSelecting;

    /** 单元格选择模式下本次拖动是否为"追加"模式（Ctrl+拖动：追加矩形区域，不清除已有选择）
    */
    bool m_bCellDragAdditive;

    /** 单元格悬停状态的内部标识：按行身份(RowId)存储，与显示顺序解耦
        （排序/增删行后，悬停高亮仍跟随原数据行，配合列ID定位唯一单元格）
    */
    struct HoverCellId
    {
        ListCtrlData::RowId nRowId;     //行身份（无效时为 ListCtrlData::kInvalidRowId）
        size_t nColumnId;               //列ID（无效时为 Box::InvalidIndex）

        HoverCellId() : nRowId(ListCtrlData::kInvalidRowId), nColumnId(Box::InvalidIndex) {}

        /** 判断是否为有效的单元格
        */
        bool IsValid() const { return (nRowId != ListCtrlData::kInvalidRowId) && (nColumnId != Box::InvalidIndex); }

        bool operator==(const HoverCellId& r) const { return (nRowId == r.nRowId) && (nColumnId == r.nColumnId); }
        bool operator!=(const HoverCellId& r) const { return !(*this == r); }
    };

    /** 单元格选择模式下鼠标悬停的单元格（用于绘制单元格悬停高亮）
    */
    HoverCellId m_hoverCell;

    /** 是否支持双击Header的分割条自动调整列宽
    */
    bool m_bEnableColumnWidthAuto;

    /** 是否自动勾选选择的数据项（与Windows下ListCtrl的LVS_EX_AUTOCHECKSELECT属性相似）
    */
    bool m_bAutoCheckSelect;

    /** 是否显示Header的CheckBox（行级）
    */
    bool m_bHeaderShowCheckBox;

    /** 是否显示数据项的CheckBox（行级）
    */
    bool m_bDataItemShowCheckBox;

    /** 是否支持复制选中内容到剪贴板（默认 true）
    */
    bool m_bEnableCopy;

    /** 是否支持 CSV 导出（默认 true）
    */
    bool m_bEnableCsvExport;

    /** 是否支持子项编辑
    */
    bool m_bEnableItemEdit;

    /** 列ID -> 编辑器类型（按列ID保存，列序变化不影响）
    */
    std::map<size_t, ListCtrlEditorType> m_columnEditorTypes;

    /** 正在编辑的单元格（仅 kCell 模式成立；非编辑态时为无效值）
    */
    ListCtrlCellId m_editingCell;

    /** 列ID -> 候选值列表（按列ID保存，供 kEditCombo 编辑器使用）
    */
    std::map<size_t, std::vector<DString>> m_columnCandidates;

    /** 当前候选浮层（kEditCombo 编辑器；关闭后由框架自动销毁，指针同时置空）
    */
    ListCtrlComboFlyout* m_pComboFlyout;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_H_
