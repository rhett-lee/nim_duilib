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
    *   说明：这里按文件路径做增量更新（复用已有的列表项，只增、删差异项），
    *         不再每次删除全部列表项后重新添加；切换目录等情况需要重建列表时，
    *         也是复用已有的列表项（只重写每行的内容），以降低刷新目录或搜索过滤时的开销
    * @param [in] currentPath 当前显示内容所在路径
    * @param [in] pathList 文件和文件夹列表（调用后不再使用，内部会移动数据）
    * @param [in] selectedPath 需要选择的路径
    * @param [in] searchKeywords 当前的搜索关键字（小写，空列表表示未启用搜索过滤），用于高亮显示命中的文件名
    */
    void SetFileList(const ui::FilePath& currentPath, std::vector<PathInfo> pathList, const ui::FilePath& selectedPath,
                     const std::vector<DString>& searchKeywords = {});

    /** 获取当前显示内容所在路径和所选择的路径
    * @param [out] currentPath 当前显示内容所在路径
    * @param [out] selectedPath 当前视图中选择的路径
    */
    void GetCurrentPath(ui::FilePath& currentPath, ui::FilePath& selectedPath) const;

    /** 获取视图的列表接口
    */
    ui::ListCtrl* GetListCtrl() const;

    /** 获取当前选择的文件或目录路径（无选择时返回空）
    */
    ui::FilePath GetSelectedPath() const;

    /** 获取当前选择的所有文件或目录路径
    */
    void GetSelectedPaths(std::vector<ui::FilePath>& paths) const;

    /** 获取当前选择的项目统计信息
    * @param [out] nSelectedCount 选中的项目数
    * @param [out] nSelectedFileSize 选中的文件总大小（不包含文件夹）
    */
    void GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const;

    /** 选择列表中的所有项目
    */
    void SelectAll();

    /** 文件大小转换为显示字符串
    */
    static DString FormatFileSize(bool bFolder, uint64_t nFileSize);

public:
    /** 视图的表头
    */
    enum class ExplorerViewColumn
    {
        kName,           //文件名称
        kModifyDateTime, //修改日期
        kType,           //文件类型
        kSize            //文件大小
    };

    /** 获取排序列
    * @param [out] viewColumn 当前排序的列
    * @param [out] bSortUp true表示升序，false表示降序
    */
    bool GetSortColumnInfo(ExplorerViewColumn& viewColumn, bool& bSortUp) const;

    /** 排序
    */
    void SortByColumn(ExplorerViewColumn viewColumn, bool bSortUp);

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

    /** 文件列表的右键菜单
    */
    bool OnExplorerViewRClick(const ui::EventArgs& msg);

    /** 图标被移除时，同步Image List中也要移除（不移除的话，会存在加载不到图标的情况）
    * @param [in] nIconId 图标ID（在IconManager中）
    */
    void OnRemoveIcon(uint32_t nIconId);

    /** 文件修改时间转换为显示字符串
    */
    DString FormatFileTime(const ui::FileTime& fileTime) const;

private:
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

    /** 应用列表项的视觉效果：斑马纹背景、搜索命中高亮
    *   说明：仅在"详细信息"视图生效，避免影响图标视图和列表视图的纯文本显示
    * @param [in] searchKeywords 当前的搜索关键字（小写，空列表表示未启用搜索过滤）
    */
    void ApplyRowStyle(const std::vector<DString>& searchKeywords);

    /** 获取斑马纹的背景颜色（由主题中的悬停颜色派生，自动适配深色/浅色主题）
    */
    ui::UiColor GetStripeColor() const;

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

    /** 视图的列表接口
    */
    ui::ControlPtrT<ui::ListCtrl> m_pListCtrl;

    /** 计算机视图中每列的初始序号与列ID映射表（因为调整列顺序后，每列的序号发生变化，已经不能通过列序号添加数据）
    */
    std::map<ExplorerViewColumn, size_t> m_columnIdMap;

    /** IconID 到 ImageID的映射表
    */
    std::map<uint32_t, int32_t> m_iconToImageMap;

    /** 图标删除回调函数的ID
    */
    uint32_t m_nRemoveIconCallbackId;

    /** 当前显示内容所在的路径
    */
    ui::FilePath m_currentPath;

    /** 路径列表（包含文件夹和文件）
    */
    std::vector<PathInfo> m_pathInfoList;
};

#endif // EXAMPLES_EXPLORER_VIEW_H_
