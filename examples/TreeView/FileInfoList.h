#pragma once

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

/** 文件信息
*/
struct FileInfo
{
    //文件名, 不含路径
    std::wstring m_fileName;
    //文件完整路径名
    std::wstring m_filePath;
    //是否为文件夹
    bool m_isFolder = false;
    //关联的图标(资源的生命周期该类内部管理)
    HICON m_hIcon = nullptr;
};

class MainForm;
class FileInfoList: public ui::VirtualListBoxElement
{
public:
    explicit FileInfoList(MainForm* pMainForm);
    virtual ~FileInfoList();

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

    /** 获取数据项总数
    * @return 返回数据项总数
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
    /** 用新的文件列表，替换原来的列表
    */
    void SetFileList(ui::TreeNode* pTreeNode, const std::vector<FileInfo>& pathList);

    /** 清空文件列表
    */
    void ClearFileList();

private:
    /** 子项被双击时触发
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnDoubleClickItem(const ui::EventArgs& args);

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

    /** 关联的树节点
    */
    ui::TreeNode* m_pTreeNode;

    //文件列表
    std::vector<FileInfo> m_pathList;
};

