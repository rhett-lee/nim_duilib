#pragma once

// #include "VirtualTileBox.h"
#include <map>

struct DownloadTask
{
	int nId;
	std::wstring sName;
};

class Provider : public ui::VirtualTileBoxElement
{
public:
	Provider();
	~Provider();

	virtual ui::Control* CreateElement() override;

	/**
	* @brief 填充指定子项
	* @param[in] control 子项控件指针
	* @param[in] index 索引
	* @return 返回创建后的子项指针
	*/
	virtual void FillElement(ui::Control *control, size_t index) override;

	/**
	* @brief 获取子项总数
	* @return 返回子项总数
	*/
	virtual size_t GetElementCount() override;

public:
	void SetTotal(int nTotal);
	void RemoveTask(size_t nIndex);
	void ChangeTaskName(size_t nIndex, const std::wstring& sName);

private:
	int m_nTotal;
	std::vector<DownloadTask> m_vTasks;
	nbase::NLock  lock_;
};

