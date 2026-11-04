#ifndef EXAMPLES_SIMPLE_FILE_VIEW_H_
#define EXAMPLES_SIMPLE_FILE_VIEW_H_

// duilib
#include "duilib/duilib.h"
#include <unordered_set>

/** 文件信息
*/
typedef ui::DirectoryTree::PathInfo PathInfo;

class MainForm;

/** 简单的文件视图：显示文件夹和文件列表
*/
class SimpleFileView : public ui::VirtualListBoxElement
{
public:
    SimpleFileView(MainForm* pMainForm, ui::VirtualListBox* pListBox);
    virtual ~SimpleFileView() override;

    /** 创建一个数据项
    * @param [in] pVirtualListBox 关联的虚表的接口
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement(ui::VirtualListBox* pVirtualListBox) override;

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
    * @param [in] currentPath 当前显示内容所在路径
    * @param [in] pathList 文件列表（调用后不再使用，内部会移动数据）
    * @param [in] selectedPath 需要选择的路径
    */
    void SetFileList(const ui::FilePath& currentPath, std::vector<PathInfo> pathList, const ui::FilePath& selectedPath);

    /** 获取当前显示内容所在路径和所选择的路径
    * @param [out] currentPath 当前显示内容所在路径
    * @param [out] selectedPath 当前视图中选择的路径
    */
    void GetCurrentPath(ui::FilePath& currentPath, ui::FilePath& selectedPath) const;

    /** 获取当前选择的所有路径
    */
    void GetSelectedPaths(std::vector<ui::FilePath>& paths) const;

    /** 获取当前选择的项目统计信息
    * @param [out] nSelectedCount 选中的项目数
    * @param [out] nSelectedFileSize 选中的文件总大小（不包含文件夹）
    */
    void GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const;

    /** 选择列表中的所有项目（本视图为单选模式时，调用无效）
    */
    void SelectAll();

private:
    /** 子项被双击时触发
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnDoubleClickItem(const ui::EventArgs& args);

    /** 文件列表的右键菜单
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnItemRClick(const ui::EventArgs& args);

    /** 判断图片项是否需要延迟加载缩略图（大图片的加载开销较大，延迟到界面空闲时按需加载）
    * @param [in] fileInfo 需要显示的路径信息
    * @return true表示延迟加载（填充阶段只显示占位图片）
    */
    bool IsThumbnailDeferred(const PathInfo& fileInfo) const;

    /** 启动缩略图延迟加载定时器
    */
    void StartThumbnailTimer();

    /** 停止缩略图延迟加载定时器
    */
    void StopThumbnailTimer();

    /** 缩略图延迟加载定时器触发：为当前可见的项目加载缩略图
    */
    void OnThumbnailTimer();

    /** 为指定的数据元素加载缩略图（该元素不可见时，不加载）
    * @param [in] nElementIndex 数据元素的索引ID
    */
    void LoadThumbnail(size_t nElementIndex);

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

    /** 文件列表的接口（右侧虚表显示的界面）
    */
    ui::ControlPtrT<ui::VirtualListBox> m_pListBox;

    /** 当前显示内容所在的路径
    */
    ui::FilePath m_currentPath;

    /** 文件列表
    */
    std::vector<PathInfo> m_pathList;

    /** 文件列表中元素的选择状态
    */
    std::vector<bool> m_pathSelectList;

    /** 出错的图片路径（包括加载图片失败或者图片数据解码失败）
    */
    std::unordered_set<DString> m_errorImagePathSet;

    /** 等待加载缩略图的数据元素索引
    */
    std::unordered_set<size_t> m_pendingThumbIndexs;

    /** 缩略图延迟加载定时器的ID（0表示未启动）
    */
    size_t m_nThumbTimerId;

    /** 缩略图延迟加载定时器的取消标志
    */
    ui::WeakCallbackFlag m_thumbTimerFlag;

    /** 缩略图延迟加载定时器连续没有加载到缩略图的次数（达到上限时停止定时器）
    */
    size_t m_nThumbIdleCount;
};

#endif //EXAMPLES_SIMPLE_FILE_VIEW_H_
