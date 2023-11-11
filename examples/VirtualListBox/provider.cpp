#include "stdafx.h"
#include "provider.h"
#include "item.h"


int g_index = 1;

Provider::Provider():
	m_nTotal(0),
	m_bMultiSelect(true)
{
}


Provider::~Provider()
= default;

ui::Control* Provider::CreateElement()
{
	Item* item = new Item;
	ui::GlobalManager::Instance().FillBoxWithCache(item, L"virtual_list_box/item.xml");
	return item;
}

bool Provider::FillElement(ui::Control* pControl, size_t nElementIndex)
{
	nbase::NAutoLock auto_lock(&lock_);
	Item* pItem = dynamic_cast<Item*>(pControl);
	ASSERT(pItem != nullptr);
	ASSERT(nElementIndex < m_vTasks.size());
	if ((pItem == nullptr) || (nElementIndex >= m_vTasks.size())) {
		return false;
	}
	const DownloadTask& task = m_vTasks[nElementIndex];
	std::wstring img = L"icon.png";
	std::wstring title = nbase::StringPrintf(L"%s [%02d]", task.sName, task.nId);
	pItem->InitSubControls(img, title, nElementIndex);
	return true;
}

size_t Provider::GetElementCount() const
{
	// 加锁
	nbase::NAutoLock auto_lock(&lock_);
	return m_vTasks.size();
}

void Provider::SetElementSelected(size_t nElementIndex, bool bSelected)
{
	nbase::NAutoLock auto_lock(&lock_);
	const size_t nCount = m_vTasks.size();
	if (nElementIndex < nCount) {
		m_vTasks[nElementIndex].bSelected = bSelected;
		if (bSelected && !m_bMultiSelect) {
			for (size_t index = 0; index < nCount; ++index) {
				if ((index != nElementIndex) && m_vTasks[index].bSelected) {
					m_vTasks[index].bSelected = false;
				}
			}
		}
	}
}

bool Provider::IsElementSelected(size_t nElementIndex) const
{
	nbase::NAutoLock auto_lock(&lock_);
	bool bSelected = false;
	if (nElementIndex < m_vTasks.size()) {
		bSelected = m_vTasks[nElementIndex].bSelected;
	}
	return bSelected;
}

void Provider::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
	selectedIndexs.clear();
	nbase::NAutoLock auto_lock(&lock_);
	size_t nCount = m_vTasks.size();
	for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
		if (m_vTasks[nElementIndex].bSelected) {
			selectedIndexs.push_back(nElementIndex);
		}
	}
}

bool Provider::IsMultiSelect() const
{
	return m_bMultiSelect;
}

void Provider::SetMultiSelect(bool bMultiSelect)
{
	m_bMultiSelect = bMultiSelect;
}

void Provider::SetTotal(int nTotal)
{
	if (nTotal == m_nTotal) return;
	if (nTotal <= 0) return;
	m_nTotal = nTotal;

	// 加锁
	lock_.Lock();
	for (auto task : m_vTasks) {
		delete [] task.sName;
	}
	m_vTasks.clear();
	std::wstring name = L"任务名称";
	m_vTasks.reserve(nTotal);
	for (auto i=0; i < nTotal; i++)
	{
		DownloadTask task;
		task.nId = i;
		//不适用std::wstring，因为它占用的内存很多，当数据量达到千万级别以上时，占的内存太多
		task.sName = new wchar_t[name.size() + 1];
		wcscpy_s(task.sName, name.size() + 1, name.c_str());
		m_vTasks.emplace_back(std::move(task));
	}
	lock_.Unlock();

	// 通知TileBox数据总数变动
	EmitCountChanged();
}

void Provider::RemoveTask(size_t nIndex)
{	
	lock_.Lock();
	bool bUpdated = false;
	if (nIndex < m_vTasks.size()) {
		auto iter = m_vTasks.begin() + nIndex;
		m_vTasks.erase(iter);
		bUpdated = true;
	}
	lock_.Unlock();

	if (bUpdated) {
		// 通知TileBox数据总数变动
		EmitCountChanged();
	}	
}

void Provider::ChangeTaskName(size_t nIndex, const std::wstring& sName)
{
	lock_.Lock();
	bool bUpdated = false;
	if (nIndex < m_vTasks.size()) {
		delete m_vTasks[nIndex].sName;
		m_vTasks[nIndex].sName = new wchar_t[sName.size() + 1];
		wcscpy_s(m_vTasks[nIndex].sName, sName.size() + 1, sName.c_str());
		bUpdated = true;
	}
	lock_.Unlock();

	// 发送数据变动通知
	if (bUpdated) {
		EmitDataChanged(nIndex, nIndex);
	}	
}
