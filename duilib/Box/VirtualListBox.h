// Copyright 2017, NetEase (Hangzhou) Network Co., Ltd. All rights reserved.
//
// zqw
// 2015/7/22
//
// virtual ListBox class
#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/ListBox.h"

namespace ui
{

/**
 * @brief 虚拟列表接口类
 * 提供开发者对子项数据管理个性化定制
 */
class VirtualListBoxElement
{
public:
	/**@brief 创建一个子项
	 * @return 返回创建后的子项指针
	 */
	virtual Control* CreateElement() = 0;

	/**@brief 填充指定子项
	 * @param[in] control 子项控件指针
	 * @param[in] index 索引
	 * @return 返回创建后的子项指针
	 */
	virtual void FillElement(Control* control, size_t index) = 0;

	/**@brief 获取子项总数
	 * @return 返回子项总数
	 */
	virtual size_t GetElementCount() = 0;
};

/**
 * @brief 虚拟列表
 */
class UILIB_API VirtualVLayout : public VLayout
{
protected:
	virtual ui::UiSize64 ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc);
};

class UILIB_API VirtualHLayout : public HLayout
{
protected:
	virtual ui::UiSize64 ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc);
};

/** 虚表实现的ListBox，垂直布局
*/
class UILIB_API VirtualListBox : public ListBox
{
	friend class VirtualVLayout;
	friend class VirtualHLayout;
public:
	explicit VirtualListBox(ui::Layout* pLayout = new VirtualVLayout);

	virtual std::wstring GetType() const override;

	/**@brief 设置数据代理对象
	 * @param[in] pProvider 开发者需要重写 VirtualListBoxElement 的接口来作为数据代理对象
	 */
	virtual void SetDataProvider(VirtualListBoxElement* pProvider);

	/**@brief 设置子项高度
	 * @param[in] nHeight 高度值
	 */
	virtual void SetElementHeight(size_t nHeight);

	/**@brief 初始化子项
	 * @param[in] nMaxItemCount 初始化数量，默认 50
	 */
	virtual void InitElement(size_t nMaxItemCount = 50);

	/**@brief 设置列表的纵横方向
	 * @param[in] direction 方向
	 */
	enum class ListDirection
	{
		kListHorizontal = 0, //横向
		kListVertical   = 1  //纵向
	};
	void SetDirection(ListDirection direction);

	/**@brief 刷新列表
	 */
	virtual void Refresh();

	/**@brief 删除所有子项
	 */
	virtual void RemoveAllItems() override;

	/**@brief 设置是否强制重新布局
	 * @param[in] bForce 设置为 true 为强制，否则为不强制
	 */
	void SetForceArrange(bool bForce);

	/**@brief 获取当前所有可见控件的索引
	 * @param[out] collection 索引列表
	 */
    void GetDisplayCollection(std::vector<size_t>& collection);

	/**@brief 让控件在可见范围内
	 * @param[in] iIndex 控件索引
	 * @param[in] bToTop 是否在最上方
	 */
    void EnsureVisible(size_t iIndex, bool bToTop = false);

protected:
	/// 重写父类接口，提供个性化功能
	virtual void SetScrollPos(ui::UiSize64 szPos) override;
	virtual void HandleEvent(const ui::EventArgs& event) override;
	virtual void SetPos(UiRect rc) override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/**@brief 重新布局子项
	 * @param[in] bForce 是否强制重新布局
	 */
	void ReArrangeChild(bool bForce);

	/**@brief 添加一个子项
	 * @param[in] iIndex 要插入的位置
	 */
	void AddElement(size_t iIndex);

	/**@brief 移除一个子项
	 * @param[in] iIndex 要移除的子项索引
	 */
	void RemoveElement(size_t iIndex);

	/**@brief 键盘按下通知
	 * @param[in] ch 按键
	 */
	virtual void OnKeyDown(TCHAR ch) { (void)ch; }

	/**
	 * @brief 键盘弹起通知
	 * @param[in] ch 按键
	 * @return 无
	 */
	virtual void OnKeyUp(TCHAR ch) { (void)ch; }

private:
	enum ScrollDirection
	{
		kScrollUp = -1,
		kScrollDown = 1
	};

	/**@brief 创建一个子项
	 */
	Control* CreateElement();

	/**@brief 填充指定子项
	 * @param[in] control 子项控件指针
	 * @param[in] index 索引
	 */
	void FillElement(Control *pControl, size_t iIndex);

	/**@brief 获取元素总数
	 */
	size_t GetElementCount();

	/**@brief 使用默认布局
	 */
	bool UseDefaultLayout();

	/**@brief 得到n个元素对应的高度和，
	 * @param[in] nCount 要得到多少元素的高度，-1表示全部元素
	 * @return 返回指定数量元素的高度和
	 */
    int64_t CalcElementsHeight(size_t nCount);

	/**@brief 得到可见范围内第一个元素的前一个元素索引
	 * @param[out] bottom 返回上一个元素的 bottom 值
	 * @return 返回上一个元素的索引
	 */
	size_t GetTopElementIndex(int64_t* bottom);

	/**@brief 判断某个元素是否在可见范围内
	 * @param[in] iIndex 元素索引
	 * @return 返回 true 表示可见，否则为不可见
	 */
    bool IsElementDisplay(size_t iIndex);

    /**@brief 判断是否要重新布局
	 * @param[out] direction 向上滚动还是向下滚动
	 * @return true 为需要重新布局，否则为 false
     */
	bool NeedReArrange(ScrollDirection& direction);

private:

	//数据提供者接口
	VirtualListBoxElement* m_pDataProvider;

	// 每个项的在列表方向的长度(横向为长度，纵向为高度)
	size_t m_nElementHeight;

	// 列表真实控件数量上限  
	size_t m_nMaxItemCount;

	//滚动条位置
	int64_t m_nOldScrollPos;

	//重新排列布局标志
	bool m_bArrangedOnce;

	// 强制布局标记
	bool m_bForceArrange;

	//列表的纵横方向
	ListDirection m_eDirection = ListDirection::kListVertical;
};

}