#ifndef UI_CONTROL_LIST_CTRL_LIST_VIEW_H_
#define UI_CONTROL_LIST_CTRL_LIST_VIEW_H_

#include "duilib/Control/ListCtrlDefs.h"

namespace ui
{
/** 列表数据显示和布局功能(List类型)
*   基本结构：
*       <ListCtrlListView>
*           <ListCtrlListViewItem> <ListCtrlIcon/><ListCtrlLabel/> </ListCtrlListViewItem>
*           ...
*           <ListCtrlListViewItem> <ListCtrlIcon/><ListCtrlLabel/> </ListCtrlListViewItem>
*       </ListCtrlListView>
*   其中的ListCtrlIcon和ListCtrlLabel的属性，支持从配置文件读取
*/
class ListCtrl;
class ListCtrlData;
class ListCtrlListView : public ListCtrlView, public IListCtrlView
{
    typedef ListCtrlView BaseClass;
public:
    explicit ListCtrlListView(Window* pWindow);
    virtual ~ListCtrlListView() override;

    virtual DString GetType() const override { return _T("ListCtrlListView"); }
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void HandleEvent(const EventArgs& msg) override;

public:
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

public:
    /** 设置布局方向（默认情况下，List视图是纵向布局）
    * @param [in] bHorizontal true表示横向布局，false表示纵向布局
    */
    void SetHorizontalLayout(bool bHorizontal);

    /** 判断当前布局方向是否为横向布局
    */
    bool IsHorizontalLayout() const;

    /** 获取子项大小，该宽度和高度，是包含了控件的外边距和内边距的
    */
    UiSize GetItemSize() const;

    /** 设置子项大小
     * @param [in] szItem 子项大小数据，该宽度和高度，是包含了控件的外边距和内边距的
     */
    void SetItemSize(const UiSize& szItem);

    /** 设置是否使用单行文本
    */
    void SetTextSingleLine(bool bSingleLine);

    /** 获取是否使用单行文本
    */
    bool IsTextSingleLine(bool& bSingleLine) const;

    /** 监听List视图数据项UI元素填充事件（虚表）
     * @param [in] callback 要绑定的回调函数
     * 参数说明:
     *   wParam: 关联的UI容器子项索引号，有效范围：[0, GetItemCount())，如果值为Box::InvalidIndex，表示未关联任何子项
     *   lParam: 关联的UI容器子项索引号对应的数据项索引号，有效范围：[0, GetDataItemCount())，如果值为Box::InvalidIndex，表示无关联的数据项
     *   pEventData: 关联的UI容器子项的界面控件接口指针，类型为：ListCtrlListViewItem*指针
     */
    void AttachListViewItemFilled(const EventCallback& callback) { this->AttachEvent(kEventListViewItemFilled, callback); }

protected:
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

private:
    /** ListCtrl 控件接口
    */
    ListCtrl* m_pListCtrl;

    /** 数据接口
    */
    ListCtrlData* m_pData;

    /** 是否使用单行文本
    */
    bool m_bSingleLine;
    bool m_bSingleLineFlag;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_LIST_VIEW_H_
