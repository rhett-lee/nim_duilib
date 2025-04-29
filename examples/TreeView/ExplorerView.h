#ifndef EXAMPLES_EXPLORER_VIEW_H_
#define EXAMPLES_EXPLORER_VIEW_H_

// duilib
#include "duilib/duilib.h"

class MainForm;

/** 文件信息
*/
typedef ui::DirectoryTree::PathInfo PathInfo;

/** 文件浏览器视图：显示文件夹和文件列表
*/
class ExplorerView : public ui::SupportWeakCallback
{
public:
    /** 构造函数
    * @param [in] pMainForm 关联窗口的接口
    * @param [in] pListCtrl 关联的列表控件接口
    */
    ExplorerView(MainForm* pMainForm, ui::ListCtrl* pListCtrl);

    /** 析构函数
    */
    virtual ~ExplorerView() override;

public:
    /** 用新的文件列表，替换原来的列表
    */
    void SetFileList(const std::vector<PathInfo>& pathList);

private:
    /** 初始化
    */
    void Initialize();

    /** 初始化计算机视图的表头
    */
    void InitViewHeader();

    /** 在"计算机"视图中双击
    */
    bool OnExplorerViewDoubleClick(const ui::EventArgs& msg);

    /** 图标被移除时，同步Image List中也要移除（不移除的话，会存在加载不到图标的情况）
    * @param [in] nIconId 图标ID（在IconManager中）
    */
    void OnRemoveIcon(uint32_t nIconId);

    /** 文件大小转换为显示字符串
    */
    DString FormatFileSize(bool bFolder, uint64_t nFileSize) const;

    /** 文件修改时间转换为显示字符串
    */
    DString FormatFileTime(const ui::FileTime& fileTime) const;

private:
    /** 视图的表头
    */
    enum class ExplorerViewColumn
    {
        kName,           //文件名称
        kModifyDateTime, //修改日期
        kType,           //文件类型
        kSize            //文件大小
    };

    /** 获取真实的列索引号
    */
    size_t GetColumnId(ExplorerViewColumn nOriginIndex) const;

    /** 列表项排序分组
    */
    enum SortGroup
    {
        kFolder = 0, //文件夹
        kFile   = 1, //文件
    };

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

    /** 视图的列表接口
    */
    ui::ControlPtr<ui::ListCtrl> m_pListCtrl;

    /** 计算机视图中每列的初始序号与列ID映射表（因为调整列顺序后，每列的序号发生变化，已经不能通过列序号添加数据）
    */
    std::map<ExplorerViewColumn, size_t> m_columnIdMap;

    /** IconID 到 ImageID的映射表
    */
    std::map<uint32_t, int32_t> m_iconToImageMap;

    /** 图标删除回调函数的ID
    */
    uint32_t m_nRemoveIconCallbackId;

    /** 路径列表（包含文件夹和文件）
    */
    std::vector<PathInfo> m_pathInfoList;
};

#endif // EXAMPLES_EXPLORER_VIEW_H_
