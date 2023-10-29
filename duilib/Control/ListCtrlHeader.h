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

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

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

public:

    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

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

private:
    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_HEADER_H_
