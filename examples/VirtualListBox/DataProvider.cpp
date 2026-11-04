#include "DataProvider.h"
#include "Item.h"

DownloadTask::DownloadTask(int nIdValue, const DString& taskName) :
    nId(nIdValue)
{
    SetName(taskName);
}

DownloadTask::~DownloadTask()
{
    delete[] sName;
}

DownloadTask::DownloadTask(DownloadTask&& other) noexcept :
    nId(other.nId),
    bSelected(other.bSelected),
    sName(other.sName)
{
    other.nId = 0;
    other.bSelected = false;
    other.sName = nullptr;
}

DownloadTask& DownloadTask::operator=(DownloadTask&& other) noexcept
{
    if (this != &other) {
        delete[] sName;
        nId = other.nId;
        bSelected = other.bSelected;
        sName = other.sName;

        other.nId = 0;
        other.bSelected = false;
        other.sName = nullptr;
    }
    return *this;
}

void DownloadTask::SetName(const DString& taskName)
{
    delete[] sName;
    sName = nullptr;
    sName = new DString::value_type[taskName.size() + 1];
    ui::StringUtil::StringCopy(sName, taskName.size() + 1, taskName.c_str());
}

DataProvider::DataProvider():
    m_nTotal(0),
    m_nNextId(0),
    m_bMultiSelect(true)
{
    m_pfnDataChangedNotify = nullptr;
}

DataProvider::~DataProvider()
= default;

ui::Control* DataProvider::CreateElement(ui::VirtualListBox* pVirtualListBox)
{
    ASSERT(pVirtualListBox != nullptr);
    if (pVirtualListBox == nullptr) {
        return nullptr;
    }
    ASSERT(pVirtualListBox->GetWindow() != nullptr);
    if (pVirtualListBox->GetWindow() == nullptr) {
        return nullptr;
    }
    Item* item = new Item(pVirtualListBox->GetWindow());
    ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("virtual_list_box/item.xml")));
    return item;
}

bool DataProvider::FillElement(ui::Control* pControl, size_t nElementIndex)
{
    std::lock_guard<std::mutex> guard(m_lock);
    Item* pItem = dynamic_cast<Item*>(pControl);
    ASSERT(pItem != nullptr);
    ASSERT(nElementIndex < m_vTasks.size());
    if ((pItem == nullptr) || (nElementIndex >= m_vTasks.size())) {
        return false;
    }
    const DownloadTask& task = m_vTasks[nElementIndex];
    DString img = _T("icon.png");
    DString title = ui::StringUtil::Printf(_T("%s [%02d]"), task.sName, task.nId);
    //进度由任务ID计算，保证滚动或刷新后保持不变
    int32_t nProgress = (int32_t)((task.nId * 37) % 101);
    pItem->InitSubControls(img, title, nElementIndex, nProgress);
    return true;
}

size_t DataProvider::GetElementCount() const
{
    std::lock_guard<std::mutex> guard(m_lock);
    return m_vTasks.size();
}

void DataProvider::SetElementSelected(size_t nElementIndex, bool bSelected)
{
    std::lock_guard<std::mutex> guard(m_lock);
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

bool DataProvider::IsElementSelected(size_t nElementIndex) const
{
    std::lock_guard<std::mutex> guard(m_lock);
    bool bSelected = false;
    if (nElementIndex < m_vTasks.size()) {
        bSelected = m_vTasks[nElementIndex].bSelected;
    }
    return bSelected;
}

void DataProvider::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    std::lock_guard<std::mutex> guard(m_lock);
    size_t nCount = m_vTasks.size();
    for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
        if (m_vTasks[nElementIndex].bSelected) {
            selectedIndexs.push_back(nElementIndex);
        }
    }
}

bool DataProvider::IsMultiSelect() const
{
    std::lock_guard<std::mutex> guard(m_lock);
    return m_bMultiSelect;
}

void DataProvider::SetMultiSelect(bool bMultiSelect)
{
    std::lock_guard<std::mutex> guard(m_lock);
    m_bMultiSelect = bMultiSelect;
}

void DataProvider::SetTotal(int nTotal)
{
    if (nTotal <= 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> guard(m_lock);
        if ((nTotal == m_nTotal) && !m_vTasks.empty()) {
            return;
        }
        m_nTotal = nTotal;
    }
    Refresh();
}

void DataProvider::Refresh()
{
    int nTotal = 0;
    {
        std::lock_guard<std::mutex> guard(m_lock);
        if (m_nTotal < 1) {
            m_nTotal = 1;
        }
        nTotal = m_nTotal;
    }

    DString name = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_VIRTUALLISTBOX_TASK_NAME"));
    std::vector<DownloadTask> tasks;
    tasks.reserve((size_t)nTotal);
    for (int i = 0; i < nTotal; ++i) {
        tasks.emplace_back(i, name);
    }

    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_vTasks.swap(tasks);
        m_nNextId = nTotal;
    }

    // 通知TileBox数据总数变动
    EmitCountChanged();
    NotifyDataChanged();
}

void DataProvider::AddTask(const DString& sName)
{
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_vTasks.emplace_back(m_nNextId, sName);
        ++m_nNextId;
        m_nTotal = (int)m_vTasks.size();
    }

    // 通知TileBox数据总数变动
    EmitCountChanged();
    NotifyDataChanged();
}

void DataProvider::RemoveTask(size_t nIndex)
{
    bool bUpdated = false;
    {
        std::lock_guard<std::mutex> guard(m_lock);
        if (nIndex < m_vTasks.size()) {
            m_vTasks.erase(m_vTasks.begin() + nIndex);
            m_nTotal = (int)m_vTasks.size();
            bUpdated = true;
        }
    }

    if (bUpdated) {
        // 通知TileBox数据总数变动
        EmitCountChanged();
        NotifyDataChanged();
    }
}

void DataProvider::ChangeTaskName(size_t nIndex, const DString& sName)
{
    bool bUpdated = false;
    {
        std::lock_guard<std::mutex> guard(m_lock);
        if (nIndex < m_vTasks.size()) {
            m_vTasks[nIndex].SetName(sName);
            bUpdated = true;
        }
    }

    // 发送数据变动通知
    if (bUpdated) {
        EmitDataChanged(nIndex, nIndex);
    }
}

void DataProvider::SetDataChangedNotify(const std::function<void()>& notify)
{
    m_pfnDataChangedNotify = notify;
}

void DataProvider::ClearDataChangedNotify()
{
    m_pfnDataChangedNotify = nullptr;
}

void DataProvider::NotifyDataChanged()
{
    if (m_pfnDataChangedNotify) {
        m_pfnDataChangedNotify();
    }
}
