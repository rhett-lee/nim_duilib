#pragma once

// #include "VirtualListBox.h"
#include <map>

struct DownloadTask
{
	int nId; //唯一ID
	bool bSelected = false;//选择状态
	wchar_t* sName = nullptr;//名称
};

class Provider : public ui::VirtualListBoxElement
{
public:
	Provider();
	~Provider();

	virtual ui::Control* CreateElement() override;

	/** 填充指定数据项
	* @param[in] pControl 数据项控件指针
	* @param[in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
	*/
	virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

	/**
	* @brief 获取子项总数
	* @return 返回子项总数
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
	void SetTotal(int nTotal);
	void RemoveTask(size_t nIndex);
	void ChangeTaskName(size_t nIndex, const std::wstring& sName);

private:
	int m_nTotal;
	std::vector<DownloadTask> m_vTasks;
	bool m_bMultiSelect; //是否支持多选
	mutable nbase::NLock  lock_;
};

