#pragma once

#include "duilib/Box/TileBox.h"
#include "duilib/Box/ListBox.h"
#include "base/callback/callback.h"
#include <functional>

namespace ui {

typedef std::function<void(size_t nStartIndex, size_t nEndIndex)> DataChangedNotify;
typedef std::function<void()> CountChangedNotify;

class UILIB_API VirtualTileBoxElement : public virtual nbase::SupportWeakCallback
{
public:
    VirtualTileBoxElement();

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement() = 0;

    /** 填充指定数据项
    * @param[in] control 数据项控件指针
    * @param[in] index 索引
    */
    virtual void FillElement(ui::Control* control, size_t index) = 0;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() = 0;

    /** 注册事件通知回调
    * @param [in] dcNotify 数据内容变化通知接口
    * @param [in] ccNotify 数据项个数变化通知接口
    */
    void RegNotifys(const DataChangedNotify& dcNotify, const CountChangedNotify& ccNotify);

protected:

    /** 发送通知：数据内容发生变化
    * @param [in] nStartIndex 数据的开始下标
    * @param [in] nEndIndex 数据的结束下标
    */
    void EmitDataChanged(size_t nStartIndex, size_t nEndIndex);

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

class UILIB_API VirtualTileLayout : public TileLayout
{
public:
    VirtualTileLayout();

    /** 调整内部所有控件的位置信息
     * @param [in] items 控件列表
     * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
     * @return 返回排列后最终盒子的宽度和高度信息
     */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

    /** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
     * @param[in] items 子控件列表
     * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度）；
               包含items中子控件的外边距，包含items中子控件的内边距；
               包含Box控件本身的内边距；
               不包含Box控件本身的外边距；
               返回值中不包含拉伸类型的子控件大小。
     */
    virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable);
    
public:
    /** 获取数据项的高度
    * @param [in] nCount 数据项个数，如果为Box::InvalidIndex，则获取所有数据项的高度总和
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    * @return 返回 nCount 个数据项的高度总和
    */
    int64_t GetElementsHeight(UiRect rc, size_t nCount);

    /** 延迟加载展示数据
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    void LazyArrangeChild(UiRect rc);

    /** 获取需要展示的真实数据项最大个数（即有Control对象对应的真实数据项）
    * @param [in] rc 当前容器大小信息, 外部调用时，需要先剪去内边距
    */
    size_t AjustMaxItem(UiRect rc);

    /** 计算列数
    *@param [in] rcWidth 可用区域宽度
    *@return 计算得到的列数, 大于或等于1
    */
    int32_t CalcTileColumns(int32_t rcWidth) const;
};

/** 虚表实现的TileBox，支持大数据量，只支持纵向滚动条
*/
class UILIB_API VirtualTileBox : public ListBox
{
    friend class VirtualTileLayout;
public:
    explicit VirtualTileBox(Layout* pLayout = new VirtualTileLayout);

    /**
    * @brief 设置数据代理对象
    * @param[in] pProvider 开发者需要重写 VirtualListBoxElement 的接口来作为数据代理对象
    */
    void SetDataProvider(VirtualTileBoxElement* pProvider);

    /** 获取数据代理对象
    */
    VirtualTileBoxElement* GetDataProvider();

    /** 刷新列表
    */
    void Refresh();

    /** 获取当前所有可见控件的数据元素索引
    * @param[out] collection 索引列表，范围是：[0, GetElementCount())
    */
    void GetDisplayCollection(std::vector<size_t>& collection);

    /** 让控件在可见范围内
    * @param[in] iIndex 元素索引号，范围是：[0, GetElementCount())
    * @param[in] bToTop 是否在最上方
    */
    void EnsureVisible(size_t iIndex, bool bToTop);

protected:
    /// 重写父类接口，提供个性化功能
    virtual void SetScrollPos(UiSize64 szPos) override;
    virtual void SetPos(UiRect rc) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    //重载删除函数，禁止外部调用
    virtual bool RemoveItem(Control* pControl) override;
    virtual bool RemoveItemAt(size_t iIndex) override;
    virtual void RemoveAllItems() override;

    virtual void HandleEvent(const EventArgs& event) override;

protected:
    /** 键盘按下通知
    * @param[in] ch 按键
    */
    virtual void OnKeyDown(TCHAR ch);

    /** 键盘弹起通知
    * @param[in] ch 按键
    */
    virtual void OnKeyUp(TCHAR ch);

private:
    /** 是否含有数据代理对象接口
    */
    bool HasDataProvider() const;

    /** 创建一个子项
    * @return 返回创建后的子项指针
    */
    Control* CreateElement();

    /** 填充指定子项
    * @param[in] control 子项控件指针
    * @param[in] index 索引
    * @return 返回创建后的子项指针
    */
    void FillElement(Control* pControl, size_t iIndex);

    /** 获取元素总数
    * @return 返回元素总指数
    */
    size_t GetElementCount();

    /** 数据内容发生变化，在事件中需要重新加载展示数据
    */
    void OnModelDataChanged(size_t nStartIndex, size_t nEndIndex);

    /** 数据个数发生变化，在事件中需要重新加载展示数据
    */
    void OnModelCountChanged();

private:
    /** 判断是否要重新布局
    */
    bool NeedReArrange();

    /** 重新布局子项
    * @param[in] bForce 是否强制重新布局
    */
    void ReArrangeChild(bool bForce);

    /** 懒加载所需数据（只加载界面显示区域的那部分数据）
    */
    void LazyArrangeChild();

    /** 得到n个元素对应的高度和，
    * @param[in] nCount 要得到多少元素的高度，-1表示全部元素
    * @return 返回指定数量元素的高度和
    */
    int64_t CalcElementsHeight(size_t nCount);

    /** 获取布局接口
    */
    VirtualTileLayout* GetTileLayout();

    /** 获取数据项目的高度
    */
    int64_t GetRealElementHeight();

    /** 获取列数
    */
    size_t GetColumns();

    /** 判断某个元素是否在可见范围内
    * @param[in] iIndex 元素索引
    * @return 返回 true 表示可见，否则为不可见
    */
    bool IsElementDisplay(size_t iIndex);

    /** 得到可见范围内第一个元素的前一个元素索引
    * @return 返回上一个元素的索引
    */
    size_t GetTopElementIndex();

    /** 将数据元素的索引号转变为展示内容的索引号
    * @param [in] nElementIndex 数据元素的索引号
    * @return 如果nElementIndex是显示范围内的数据，返回对应的Item索引号
    *         如果nElementIndex不是显示范围内的数据，返回Box::InvalidIndex，表示无对应的Item索引号
    */
    size_t ElementIndexToItemIndex(size_t nElementIndex);

    /** 将展示内容的Item索引号，转换为数据元素的索引号
    */
    size_t ItemIndexToElementIndex(size_t nItemIndex);

private:
    /** 数据代理对象接口，提供展示数据
    */
    VirtualTileBoxElement* m_pDataProvider;
};

}
