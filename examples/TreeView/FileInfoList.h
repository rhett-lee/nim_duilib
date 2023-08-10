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

class FileInfoList: public ui::VirtualListBoxElement
{
public:
    FileInfoList();
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
    virtual size_t GetElementCount() override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) override;

public:
    
    /** 添加文件列表到原来的列表中
    */
    void AddFileList(const std::vector<FileInfo>& pathList);

    /** 用新的文件列表，替换原来的列表
    */
    void SetFileList(const std::vector<FileInfo>& pathList);

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
    //文件列表
    std::vector<FileInfo> m_pathList;
};

