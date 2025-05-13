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

/** ListCtrl控件
*/
class ListCtrl: public VBox
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

    /** 批量设置选择数据项
    * @param [in] selectedIndexs 需要设置选择的数据项索引号，有效范围：[0, GetDataItemCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    */
    void SetSelectedDataItems(const std::vector<size_t>& selectedIndexs, bool bClearOthers);

    /** 获取选择的元素列表
    * @param [in] itemIndexs 返回当前选择的数据项索引号，有效范围：[0, GetDataItemCount())
    */
    void GetSelectedDataItems(std::vector<size_t>& itemIndexs) const;

    /** 选择全部
    */
    void SetSelectAll();

    /** 取消所有选择
    */
    void SetSelectNone();

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
    */
    void Refresh();

    /** 刷新指定数据项
    * @param [in] dataItemIndexs 需要刷新的数据项索引号, 有效范围：[0, GetDataItemCount())
    */
    void RefreshDataItems(const std::vector<size_t>& dataItemIndexs);

    /** 是否允许刷新界面
    * @param [in] bEnable 如果为true，允许通过Refresh()函数刷新界面; 如果为false，则允许通过Refresh()函数刷新界面
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
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetRowGridLineWidth() const;

    /** 横向网格线的颜色
    * @param [in] color 横向网格线的颜色
    */
    void SetRowGridLineColor(const DString& color);
    DString GetRowGridLineColor() const;

    /** 纵向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
    int32_t GetColumnGridLineWidth() const;

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
     * @param[in] callback 选择子项时的回调函数
     * 参数说明:
     *   wParam: 选择子项的界面控件接口指针，类型为：ListCtrlItem*
     *   lParam: 选择的数据项索引号，有效范围：[0, GetDataItemCount())
     */
    void AttachSelect(const EventCallback& callback) { AttachEvent(kEventSelect, callback); }

    /** 监听选择项发生变化的事件
     * @param[in] callback 选择子项时的回调函数
     */
    void AttachSelChange(const EventCallback& callback) { AttachEvent(kEventSelChange, callback); }

    /** 监听双击事件
     * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
     * 参数说明:
     *   wParam: 点击的子项的界面控件接口指针，类型为：ListCtrlItem*，如果未点击子项，则为0
     *   lParam: 关联的数据项索引号，有效范围：[0, GetDataItemCount()), 仅当wParam不为0时有效
     */
    void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

    /** 绑定鼠标点击处理函数
     * @param[in] callback 要绑定的回调函数
     * 参数说明:
     *   wParam: 点击的子项的界面控件接口指针，类型为：ListCtrlItem*，如果未点击子项，则为0
     *     lParam: 关联的数据项索引号，有效范围：[0, GetDataItemCount()), 仅当wParam不为0时有效
     */
    void AttachClick(const EventCallback& callback) { AttachEvent(kEventClick, callback); }

    /** 绑定鼠标右键点击处理函数
     * @param[in] callback 要绑定的回调函数
     *  参数说明:
     *   wParam: 点击的子项的界面控件接口指针，类型为：ListCtrlItem*，如果未点击子项，则为0
     *   lParam: 关联的数据项索引号，有效范围：[0, GetDataItemCount()), 仅当wParam不为0时有效
     */
    void AttachRClick(const EventCallback& callback) { AttachEvent(kEventRClick, callback); }

    /** 监听回车事件
     * @param[in] callback 收到回车时的回调函数
     * 参数说明:
     *   wParam: 关联的子项的界面控件接口指针，类型为：ListCtrlItem*，如果未点击子项，则为0
     *   lParam: 关联的数据项索引号，有效范围：[0, GetDataItemCount())
     */
    void AttachReturn(const EventCallback& callback) { this->AttachEvent(kEventReturn, callback); }

    /** 监听开始编辑事件
     * @param[in] callback 事件回调函数
     * 参数说明:
     *   wParam: 是接口指针：ListCtrlEditParam*，可以通过设置bCancelled取消操作
     */
    void AttachEnterEdit(const EventCallback& callback) { this->AttachEvent(kEventEnterEdit, callback); }

    /** 监听结束编辑事件
     * @param[in] callback 事件回调函数
     * 参数说明:
     *   wParam: 是接口指针：ListCtrlEditParam*，可以通过设置bCancelled取消操作
     */
    void AttachLeaveEdit(const EventCallback& callback) { this->AttachEvent(kEventLeaveEdit, callback); }

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
    */
    void OnHeaderColumnVisibleChanged();

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

    /** 子项编辑事件
    */
    void OnItemEdited(const ListCtrlEditParam& editParam, const DString& newItemText);

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

    /** 图片列表
    */
    ImageListPtr m_imageList[3];

    /** 编辑框控件
    */
    RichEdit* m_pRichEdit;

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

    /** 是否显示表头控件
    */
    bool m_bShowHeaderCtrl;

    /** 是否允许刷新界面
    */
    bool m_bEnableRefresh;

    /** 是否支持多选(默认是单选)
    */
    bool m_bMultiSelect;

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

    /** 是否支持子项编辑
    */
    bool m_bEnableItemEdit;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_H_
