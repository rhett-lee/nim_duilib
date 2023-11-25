#ifndef UI_BOX_VIRTUAL_LISTBOX_H_
#define UI_BOX_VIRTUAL_LISTBOX_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include "duilib/Box/VirtualHLayout.h"
#include "duilib/Box/VirtualVLayout.h"
#include "duilib/Box/VirtualHTileLayout.h"
#include "duilib/Box/VirtualVTileLayout.h"
#include "base/callback/callback.h"
#include <functional>

namespace ui {

typedef std::function<void(size_t nStartIndex, size_t nEndIndex)> DataChangedNotify;
typedef std::function<void()> CountChangedNotify;

class UILIB_API VirtualListBoxElement : public virtual nbase::SupportWeakCallback
{
public:
    VirtualListBoxElement();

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement() = 0;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) = 0;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() const = 0;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) = 0;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) const = 0;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    virtual void GetSelectedElements(std::vector<size_t>& selectedIndexs) const = 0;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const = 0;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) = 0;

public:
    /** 注册事件通知回调
    * @param [in] dcNotify 数据内容变化通知接口
    * @param [in] ccNotify 数据项个数变化通知接口
    */
    void RegNotifys(const DataChangedNotify& dcNotify, const CountChangedNotify& ccNotify);

protected:

    /** 发送通知：数据内容发生变化
    * @param [in] nStartElementIndex 数据的开始下标
    * @param [in] nEndElementIndex 数据的结束下标
    */
    void EmitDataChanged(size_t nStartElementIndex, size_t nEndElementIndex);

    /** 发送通知：数据项个数发生变化
    */
    void EmitCountChanged();

private:

    /** 数据内容发生变化的响应函数
    */
    DataChangedNotify m_pfnDataChangedNotify;

    /** 数据个数发生变化的响应函数
    */
    CountChangedNotify m_pfnCountChangedNotify;
};

/** 虚表实现的ListBox，支持大数据量，只支持纵向滚动条
*/
class UILIB_API VirtualListBox : public ListBox
{
    friend class VirtualHLayout;
    friend class VirtualVLayout;    
    friend class VirtualHTileLayout;
    friend class VirtualVTileLayout;
public:
    explicit VirtualListBox(Layout* pLayout);

    /** 设置数据代理对象
    * @param[in] pProvider 开发者需要重写 VirtualListBoxElement 的接口来作为数据代理对象
    */
    virtual void SetDataProvider(VirtualListBoxElement* pProvider);

    /** 获取数据代理对象
    */
    VirtualListBoxElement* GetDataProvider() const;

    /** 是否含有数据代理对象接口
    */
    bool HasDataProvider() const;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const override;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) override;

public:
    /** 获取数据元素总数
    * @return 返回数据元素总个数
    */
    size_t GetElementCount() const;

    /** 获取当前选择的数据元素索引号(仅单选时有效)
    @return 返回选择的数据元素索引号，范围：[0, GetElementCount())
    */
    size_t GetCurSelElement() const;

    /** 设置选择状态, 同时按需更新界面显示
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    void SetElementSelected(size_t nElementIndex, bool bSelected);

    /** 批量设置选择元素, 同时按需更新界面显示
    * @param [in] selectedIndexs 需要设置选择的元素列表，有效范围：[0, GetElementCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    * @return 如果有数据变化返回true，否则返回false
    */
    bool SetSelectedElements(const std::vector<size_t>& selectedIndexs, bool bClearOthers);

    /** 批量设置选择元素, 不更新界面显示
    * @param [in] selectedIndexs 需要设置选择的元素列表，有效范围：[0, GetElementCount())
    * @param [in] bClearOthers 如果为true，表示对其他已选择的进行清除选择，只保留本次设置的为选择项
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetSelectedElements(const std::vector<size_t>& selectedIndexs, 
                             bool bClearOthers,
                             std::vector<size_t>& refreshIndexs);

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    bool IsElementSelected(size_t nElementIndex) const;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    void GetSelectedElements(std::vector<size_t>& selectedIndexs) const;

    /** 选择全部, 同时按需更新界面显示
    * @return 如果有数据变化返回true，否则返回false
    */
    bool SetSelectAll();

    /** 取消所有选择, 同时按需更新界面显示
    * @return 如果有数据变化返回true，否则返回false
    */
    bool SetSelectNone();

    /** 取消所有选择, 不更新界面显示，可以由外部刷新界面显示
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetSelectNone(std::vector<size_t>& refreshIndexs);

    /** 取消所有选择(但排除部分元素), 不更新界面显示，可以由外部刷新界面显示
    * @param [in] excludeIndexs 需要排除的元素索引号，这部分元素的选择状态保持原状
    * @param [out] refreshIndexs 返回需要刷新显示的元素索引号
    */
    void SetSelectNoneExclude(const std::vector<size_t>& excludeIndexs, 
                              std::vector<size_t>& refreshIndexs);

    /** 获取当前所有可见控件的数据元素索引
    * @param [out] collection 索引列表，有效范围：[0, GetElementCount())
    */
    void GetDisplayElements(std::vector<size_t>& collection) const;

    /** 获取一个元素对应显示控件的索引号
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    * @return 返回该元素对应UI控件的索引号，有效范围：[0, GetItemCount())
    */
    size_t GetDisplayItemIndex(size_t nElementIndex) const;

    /** 获取一个显示控件关联的元素索引号
    * @param [in] nItemIndex UI控件的索引号，有效范围：[0, GetItemCount())
    * @return 该UI控件关联的数据元素索引号，有效范围：[0, GetElementCount())
    */
    size_t GetDisplayItemElementIndex(size_t nItemIndex) const;

    /** 让控件在可见范围内
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    * @param [in] bToTop 是否在最上方
    */
    void EnsureVisible(size_t nElementIndex, bool bToTop);

    /** 刷新指定范围的数据，保持数据与界面显示同步，数据范围: [nStartElementIndex, nEndElementIndex]
    * @param [in] nStartElementIndex 数据的开始下标
    * @param [in] nEndElementIndex 数据的结束下标
    */
    void RefreshElements(size_t nStartElementIndex, size_t nEndElementIndex);

    /** 刷新指定的数据，保持数据与界面显示同步
    * @param [elementIndexs] 列表中为元素索引号，有效范围：[0, GetElementCount())
    */
    void RefreshElements(const std::vector<size_t>& elementIndexs);

    /** 刷新列表
    */
    virtual void Refresh();

    /** 确保矩形区域可见
	* @param [in] rcItem 可见区域的矩形范围
	* @param [in] vVisibleType 垂直方向可见的附加标志
	* @param [in] hVisibleType 水平方向可见的附加标志
	*/
	virtual void EnsureVisible(const UiRect& rcItem,
							   ListBoxVerVisible vVisibleType,
							   ListBoxHorVisible hVisibleType) override;

    /** 确保子项可见
	* @param [in] iIndex 子项索引，范围是：[0, GetItemCount())
	* @param [in] vVisibleType 垂直方向可见的附加标志
	* @param [in] hVisibleType 水平方向可见的附加标志
	* @return 如果是虚表实现，返回该元素对应的新的控件索引号，范围是：[0, GetItemCount())
	*/
	virtual size_t EnsureVisible(size_t iIndex,
							     ListBoxVerVisible vVisibleType = ListBoxVerVisible::kVisible,
							     ListBoxHorVisible hVisibleType = ListBoxHorVisible::kVisible) override;

public:
    /** 监听选择子项的事件
    * @param[in] callback 选择子项时的回调函数
    * 参数说明:
    *   wParam: 当前新选择的子项ID，有效范围：[0, GetItemCount())
    *	lParam: 原来旧选择的子项ID，有效范围：[0, GetItemCount())，可能为无效值Box::InvalidIndex
    *   可以通过 GetDisplayItemElementIndex 函数得到关联的数据元素索引号
    */
    void AttachSelect(const EventCallback& callback) { AttachEvent(kEventSelect, callback); }

    /** 监听双击事件
     * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
     *  参数说明:
     *    wParam: 双击的子项ID，有效范围：[0, GetItemCount())，如果值Box::InvalidIndex，表示未双击任何子项
     *    lParam: 关联的子项ID对应的数据元素索引号，有效范围：[0, GetElementCount())，如果值Box::InvalidIndex，表示无关联的数据元素
     */
    void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

    /** 绑定鼠标点击处理函数
    * @param[in] callback 要绑定的回调函数
    *  参数说明:
    *    wParam: 点击的子项ID，有效范围：[0, GetItemCount())，如果值Box::InvalidIndex，表示未双击任何子项
    *    lParam: 关联的子项ID对应的数据元素索引号，有效范围：[0, GetElementCount())，如果值Box::InvalidIndex，表示无关联的数据元素
    */
    void AttachClick(const EventCallback& callback) { AttachEvent(kEventClick, callback); }

    /** 绑定鼠标右键点击处理函数
    * @param[in] callback 要绑定的回调函数
    *  参数说明:
    *    wParam: 点击的子项ID，有效范围：[0, GetItemCount())，如果值Box::InvalidIndex，表示未双击任何子项
    *    lParam: 关联的子项ID对应的数据元素索引号，有效范围：[0, GetElementCount())，如果值Box::InvalidIndex，表示无关联的数据元素
    */
    void AttachRClick(const EventCallback& callback) { AttachEvent(kEventRClick, callback); }

    /** 监听回车事件
     * @param[in] callback 收到回车时的回调函数
     *  参数说明:
     *    wParam: 关联的子项ID，有效范围：[0, GetItemCount())，如果值Box::InvalidIndex，表示未双击任何子项
     *    lParam: 关联的子项ID对应的数据元素索引号，有效范围：[0, GetElementCount())，如果值Box::InvalidIndex，表示无关联的数据元素
     */
    void AttachReturn(const EventCallback& callback) { this->AttachEvent(kEventReturn, callback); }

public:
    /// 重写父类接口，提供个性化功能
    virtual void SetScrollPos(UiSize64 szPos) override;
    virtual void SetPos(UiRect rc) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void SendEvent(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0, TCHAR tChar = 0,
                           const UiPoint& mousePos = UiPoint()) override;
    virtual void SendEvent(const EventArgs& event) override;

protected:
    //重载删除函数，禁止外部调用
    virtual bool RemoveItem(Control* pControl) override;
    virtual bool RemoveItemAt(size_t iIndex) override;
    virtual void RemoveAllItems() override;

    /** 即将查找指定的元素（为虚表提供加载数据的机会）
	* @param [in] nCurSel 当前选择的显示控件索引号
	* @param [in] mode  查找的模式
	* @param [in] nCount 查找的控件的第几个数据
	* @param [out] nDestItemIndex 返回加载后的目标控件索引号，有效范围: [0, GetItemCount())
	* @return 返回true表示有数据加载行为，false表示无任何动作
	*/
	virtual bool OnFindSelectable(size_t nCurSel, SelectableMode mode, 
								  size_t nCount, size_t& nDestItemIndex) override;

    /** 查询本条数据前面还有几条数据（支持虚表数据）
    * @param [in] nCurSel 当前选择的显示控件索引号
    */
    virtual size_t GetItemCountBefore(size_t nCurSel) override;

    /** 查询本条数据后面还有几条数据（支持虚表数据）
    * @param [in] nCurSel 当前选择的显示控件索引号
    */
    virtual size_t GetItemCountAfter(size_t nCurSel) override;

    /** 判断一个数据元素是否为可选择项
	* @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
	*/
    virtual bool IsSelectableElement(size_t nElementIndex) const;

    /** 获取下一个可选择的数据元素
    * @param [in] nElementIndex 元素索引号，有效范围：[0, GetElementCount())
    * @param [in] bForward true表示向前查找，false表示向后查找
    */
    virtual size_t FindSelectableElement(size_t nElementIndex, bool bForward) const;

    /** 对子项排序
     * @param [in] pfnCompare 自定义排序函数
     * @param [in] pCompareContext 传递给比较函数的用户自定义数据
     */
    virtual bool SortItems(PFNCompareFunc pfnCompare, void* pCompareContext) override;

    /** 子项的选择状态变化事件，用于状态同步
    * @param [in] iIndex 子项目的ID，范围是：[0, GetItemCount())
    * @param [in] pListBoxItem 关联的列表项接口
    */
    virtual void OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem) override;

protected:

    /** 设置虚表布局接口
    */
    void SetVirtualLayout(VirtualLayout* pVirtualLayout);

    /** 获取虚表布局接口
    */
    VirtualLayout* GetVirtualLayout() const;

public:
    struct RefreshData
    {
        size_t nItemIndex;      //界面控件的索引号
        Control* pControl;      //界面控件的接口
        size_t nElementIndex;   //数据元素的索引号
    };
    typedef std::vector<RefreshData> RefreshDataList;

    /** 执行了界面刷新操作, 界面的UI控件个数可能会发生变化
    */
    virtual void OnRefreshElements(const RefreshDataList& /*refreshDataList*/) {}

protected:
    /** 创建一个子项
    * @return 返回创建后的子项指针
    */
    Control* CreateElement();

    /** 填充指定数据项
    * @param[in] pControl 数据项控件指针
    * @param[in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    void FillElement(Control* pControl, size_t nElementIndex);

    /** 重新布局子项
    * @param[in] bForce 是否强制重新布局
    */
    void ReArrangeChild(bool bForce);

    /** 数据内容发生变化，在事件中需要重新加载展示数据
    */
    void OnModelDataChanged(size_t nStartElementIndex, size_t nEndElementIndex);

    /** 数据个数发生变化，在事件中需要重新加载展示数据
    */
    void OnModelCountChanged();

    /** 是否允许从界面状态同步到存储状态
    */
    bool IsEnableUpdateProvider() const;

    /** 发送事件的函数
    * @param [in] args 事件内容
    * @param [in] bFromItem true表示来自子控件，false表示来自自身
    */
    void VSendEvent(const EventArgs& args, bool bFromItem);

private:
    /** 数据代理对象接口，提供展示数据
    */
    VirtualListBoxElement* m_pDataProvider;

    /** 虚表布局接口
    */
    VirtualLayout* m_pVirtualLayout;

    /** 是否允许从界面状态同步到存储状态
    */
    bool m_bEnableUpdateProvider;
};

/** 横向布局的虚表ListBox
*/
class UILIB_API VirtualHListBox : public VirtualListBox
{
public:
    VirtualHListBox() :
        VirtualListBox(new VirtualHLayout)
    {
        VirtualLayout* pVirtualLayout = dynamic_cast<VirtualHLayout*>(GetLayout());
        SetVirtualLayout(pVirtualLayout);
    }

    virtual std::wstring GetType() const override { return DUI_CTR_VIRTUAL_HLISTBOX; }
};

/** 纵向布局的虚表ListBox
*/
class UILIB_API VirtualVListBox : public VirtualListBox
{
public:
    VirtualVListBox() :
        VirtualListBox(new VirtualVLayout)
    {
        VirtualLayout* pVirtualLayout = dynamic_cast<VirtualVLayout*>(GetLayout());
        SetVirtualLayout(pVirtualLayout);
    }

    virtual std::wstring GetType() const override { return DUI_CTR_VIRTUAL_VLISTBOX; }
};

/** 瓦片布局的虚表ListBox(横向布局)
*/
class UILIB_API VirtualHTileListBox : public VirtualListBox
{
public:
    VirtualHTileListBox() :
        VirtualListBox(new VirtualHTileLayout)
    {
        VirtualLayout* pVirtualLayout = dynamic_cast<VirtualHTileLayout*>(GetLayout());
        SetVirtualLayout(pVirtualLayout);
    }

    virtual std::wstring GetType() const override { return DUI_CTR_VIRTUAL_HTILE_LISTBOX; }
};

/** 瓦片布局的虚表ListBox(纵向布局)
*/
class UILIB_API VirtualVTileListBox : public VirtualListBox
{
public:
    VirtualVTileListBox() :
        VirtualListBox(new VirtualVTileLayout)
    {
        VirtualLayout* pVirtualLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
        SetVirtualLayout(pVirtualLayout);
    }

    virtual std::wstring GetType() const override { return DUI_CTR_VIRTUAL_VTILE_LISTBOX; }
};

}

#endif //UI_BOX_VIRTUAL_LISTBOX_H_
