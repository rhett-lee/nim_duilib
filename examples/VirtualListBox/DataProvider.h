#ifndef EXAMPLES_DATA_PROVIDER_H_
#define EXAMPLES_DATA_PROVIDER_H_

// duilib
#include "duilib/duilib.h"

#include <functional>
#include <mutex>
#include <vector>

struct DownloadTask
{
public:
    DownloadTask() = default;
    explicit DownloadTask(int nIdValue, const DString& taskName);
    ~DownloadTask();

    // 名称使用裸指针存储，避免大数据量时每个元素都带一个 DString 的开销
    DownloadTask(const DownloadTask&) = delete;
    DownloadTask& operator=(const DownloadTask&) = delete;
    DownloadTask(DownloadTask&& other) noexcept;
    DownloadTask& operator=(DownloadTask&& other) noexcept;

    /** 设置任务名称（内部会释放旧的名称内存）
    */
    void SetName(const DString& taskName);

public:
    int nId = 0; //唯一ID
    bool bSelected = false;//选择状态
    DString::value_type* sName = nullptr;//名称
};

class DataProvider : public ui::VirtualListBoxElement
{
public:
    DataProvider();
    virtual ~DataProvider() override;

    /** 创建一个数据项
    * @param [in] pVirtualListBox 关联的虚表的接口
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement(ui::VirtualListBox* pVirtualListBox) override;

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
    /** 设置任务总数（变化时会刷新数据）
    */
    void SetTotal(int nTotal);

    /** 重建全部任务数据
    */
    void Refresh();

    /** 新增一个任务
    */
    void AddTask(const DString& sName);

    /** 删除指定任务
    */
    void RemoveTask(size_t nIndex);

    /** 修改指定任务的名称
    */
    void ChangeTaskName(size_t nIndex, const DString& sName);

    /** 设置数据数量变化后的通知函数
    */
    void SetDataChangedNotify(const std::function<void()>& notify);

    /** 清除数据数量变化后的通知函数
    */
    void ClearDataChangedNotify();

private:
    /** 触发数据数量变化后的通知
    */
    void NotifyDataChanged();

private:
    int m_nTotal;
    int m_nNextId;
    std::function<void()> m_pfnDataChangedNotify;
    std::vector<DownloadTask> m_vTasks;
    bool m_bMultiSelect; //是否支持多选
    mutable std::mutex m_lock;
};

#endif //EXAMPLES_DATA_PROVIDER_H_
