#ifndef UI_CONTROL_LIST_CTRL_HEADER_H_
#define UI_CONTROL_LIST_CTRL_HEADER_H_

#pragma once

#include "duilib/Box/ListBoxItem.h"

namespace ui
{

/** ListCtrl的表头控件
*/
class ListCtrl;
class ListCtrlHeaderItem;
struct ListCtrlColumn;
class ListCtrlHeader : public ListBoxItemH
{
    friend class ListCtrlHeaderItem;
public:
    ListCtrlHeader();
    virtual ~ListCtrlHeader();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 设置属性
    */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

    /** 判断控件类型是否为可选择的
     * @return 默认返回false
     */
    virtual bool IsSelectableType() const override;

public:
    /** 在指定位置添加一列
    *  表头控件的基本结构如下：
    * 
    *   <ListCtrlHeader>
    *       <ListCtrlHeaderItem> CheckBox[可选] </ListCtrlHeaderItem>
    *       <SplitBox> <Control/> </SplitBox>
    *       ..
    *       <ListCtrlHeaderItem> CheckBox[可选] </ListCtrlHeaderItem>
    *       <SplitBox> <Control/> </SplitBox>
    *   </ListCtrlHeader>
    * 
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

    /** 调整列的宽度(根据该列内容的实际宽度自适应)
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @param [in] nWidth 列宽值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
    */
    bool SetColumnWidth(size_t columnIndex, int32_t nWidth, bool bNeedDpiScale);

    /** 获取列表头的控件接口
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    ListCtrlHeaderItem* GetColumn(size_t columnIndex) const;

    /** 获取列表头的控件接口
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColomnId()函数获取
    */
    ListCtrlHeaderItem* GetColumnById(size_t columnId) const;

    /** 获取列宽度和列索引序号
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColomnId()函数获取
    * @param [out] columnIndex 列的序号：[0, GetColumnCount())，代表第几列
    * @param [out] nColumnWidth 列的宽度值
    */
    bool GetColumnInfo(size_t columnId, size_t& columnIndex, int32_t& nColumnWidth) const;

    /** 获取列的索引序号
    * @param [in] columnId 列的ID值，通过ListCtrlHeaderItem::GetColomnId()函数获取
    * @return 列的序号：[0, GetColumnCount())，代表第几列
    */
    size_t GetColumnIndex(size_t columnId) const;

    /** 获取列的索引序号
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    * @return 列的ID，如果匹配不到，则返回Box::InvalidIndex
    */
    size_t GetColumnId(size_t columnIndex) const;

    /** 删除一列
    * @param [in] columnIndex 列索引序号：[0, GetColumnCount())
    */
    bool DeleteColumn(size_t columnIndex);

    /** 设置是否在Header显示CheckBox
    * @param [in] bShow true表示在表头显示CheckBox，false表示不显示
    */
    bool SetShowCheckBox(bool bShow);

    /** 判断是否显示了CheckBox
    */
    bool IsShowCheckBox() const;

    /** 禁止/允许CheckBox勾选项变化事件
    * @return 返回原来的状态，用于还原
    */
    bool SetEnableCheckChangeEvent(bool bEnable);

    /** 设置左侧的Padding值，用于与数据行对齐显示
    */
    void SetPaddingLeftValue(int32_t nPaddingLeft);

    /** 设置文字与图标之间的间隔（像素）
    */
    void SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale);

    /** 获取文字与图标之间的间隔（像素）
    */
    int32_t GetIconSpacing() const;

public:

    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 获取关联的ListCtrl接口
    */
    ListCtrl* GetListCtrl() const;

public:
    /** 列表头鼠标右键点击处理函数
    * @param[in] callback 要绑定的回调函数, 
    *   参数说明：wParam如果不为0，其值是：ListCtrlHeaderItem*
    *           可以通过 ListCtrlHeaderItem* pHeaderItem = (ListCtrlHeaderItem*)wParam; 获取到点击在哪列了
    *           如果wParam如果为0，说明是点击到了ListCtrlHeader自身空白处，没有点击到任何列
    */
    void AttachRClick(const EventCallback& callback) { AttachEvent(kEventRClick, callback); }

protected:
    /** 是否支持列表头拖动改变列的顺序
    */
    bool IsEnableHeaderDragOrder() const;

    /** 拖动列表头改变列宽的事件响应函数
    * @param [in] pLeftHeaderItem 左侧的列表头控件接口
    * @param [in] pRightHeaderItem 右侧的列表头控件接口
    */
    void OnHeaderColumnResized(Control* pLeftHeaderItem, Control* pRightHeaderItem);

    /** 点击列表头触发排序的事件响应函数
    * @param [in] pHeaderItem 列表头控件接口
    */
    void OnHeaderColumnSorted(ListCtrlHeaderItem* pHeaderItem);

    /** 通过拖动列表头，调整了列的顺序
    */
    void OnHeaderColumnOrderChanged();

    /** CheckBox的勾选项操作
    * @param [in] pHeaderItem 列表头控件接口
    * @param [in] bChecked true表示勾选（Checked状态），false表示取消勾选（UnChecked状态）
    */
    void OnHeaderColumnCheckStateChanged(ListCtrlHeaderItem* pHeaderItem, bool bChecked);

    /** 表头列的显示属性发生变化
    */
    void OnHeaderColumnVisibleChanged();

    /** 列表头列的分割条双击事件
    * @param [in] pHeaderItem 列表头控件接口
    */
    void OnHeaderColumnSplitDoubleClick(ListCtrlHeaderItem* pHeaderItem);

    /** 是否支持勾选模式（目前是TreeView/ListCtrl在使用这个模式）
        勾选模式是指：
        （1）只有点击在CheckBox图片上的时候，勾选框图片才是选择状态（非勾选模式下，是点击在控件矩形内就选择）
        （2）勾选状态和选择状态分离，是两个不同的状态
    */
    virtual bool SupportCheckedMode() const override;

    /** 勾选状态变化事件(m_bChecked变量发生变化)
    */
    virtual void OnPrivateSetChecked() override;

    /** 获取CheckBox的图片宽度
    */
    int32_t GetCheckBoxImageWidth();

    /** 更新Padding，确保文字与图标不重叠
    */
    void UpdatePaddingLeft();

private:
    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl;

    /** 设置的CheckBox宽度值
    */
    int32_t m_nCheckBoxPadding;

    /** 左侧的Padding值，用于与数据行对齐显示
    */
    int32_t m_nPaddingLeftValue;

    /** 是否允许CheckBox勾选项变化事件
    */
    bool m_bEnableCheckChangeEvent;

    /** 文字与图标之间的间隔(图标显示在文字的右侧或者左侧时)
    */
    int32_t m_nIconSpacing;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_HEADER_H_
