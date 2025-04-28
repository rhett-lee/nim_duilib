#ifndef EXAMPLES_COMPUTER_VIEW_H_
#define EXAMPLES_COMPUTER_VIEW_H_

// duilib
#include "duilib/duilib.h"
#include <map>

class MainForm;

/** "计算机"视图，显示磁盘列表
*/
class ComputerView: public ui::SupportWeakCallback
{
public:
    /** 构造函数
    * @param [in] pMainForm 关联窗口的接口
    * @param [in] pListCtrl 关联的列表控件接口
    */
    ComputerView(MainForm* pMainForm, ui::ListCtrl* pListCtrl);

    /** 析构函数
    */
    virtual ~ComputerView() override;

public:
    /** 显示"计算机"节点的内容
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    void ShowMyComputerContents(const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

private:
    /** 初始化
    */
    void Initialize();

    /** 初始化计算机视图的表头
    */
    void InitComputerViewHeader();

    /** 在"计算机"视图中双击
    */
    bool OnComuterViewDoubleClick(const ui::EventArgs& msg);    

    /** 图标被移除时，同步Image List中也要移除（不移除的话，会存在加载不到图标的情况）
    * @param [in] nIconId 图标ID（在IconManager中）
    */
    void OnRemoveIcon(uint32_t nIconId);

    /** 磁盘空间大小转换为显示字符串
    */
    DString FormatDiskSpace(uint64_t nSpace) const;

    /** 获取已用百分比显示字符串
    */
    DString FormatUsedPercent(uint64_t nTotalSpace, uint64_t nFreeSpace) const;

#if defined (DUILIB_BUILD_FOR_WIN)
    /** 初始化表头
    */
    void InitComputerViewHeader_Win();

    /** 显示"计算机"节点的内容
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    void ShowMyComputerContents_Win(ui::ImageListPtr pImageList, const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

#elif defined (DUILIB_BUILD_FOR_LINUX)
    /** 初始化表头
    */
    void InitComputerViewHeader_Linux();

    /** 显示"计算机"节点的内容
    * @param [in] diskInfoList 所有磁盘的信息列表
    */
    void ShowMyComputerContents_Linux(ui::ImageListPtr pImageList, const std::vector<ui::DirectoryTree::DiskInfo>& diskInfoList);

    /** 设备类型转换为字符串
    */
    DString GetDeviceTypeString(ui::DirectoryTree::DeviceType deviceType) const;

#endif

private:
    /** 计算机视图的表头
    */
    enum class ComputerViewColumn
    {
        kName,         //名称
        kType,         //类型
        kPartitionType,//分区类型
        kTotalSpace,   //总大小
        kFreeSpace,    //可用空间
        kUsedPercent,  //已用百分比
        kMountOn       //挂载点
    };

    /** 获取真实的列索引号
    */
    size_t GetColumnId(ComputerViewColumn nOriginIndex) const;

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

private:
    /** 计算机视图的数据
    */
    std::vector<ui::DirectoryTree::DiskInfo> m_diskInfoList;

    /** 计算机视图的列表接口
    */
    ui::ControlPtr<ui::ListCtrl> m_pComputerListCtrl;

    /** 计算机视图中每列的初始序号与列ID映射表（因为调整列顺序后，每列的序号发生变化，已经不能通过列序号添加数据）
    */
    std::map<ComputerViewColumn, size_t> m_columnIdMap;

    /** IconID 到 ImageID的映射表
    */
    std::map<uint32_t, int32_t> m_iconToImageMap;

    /** 图标删除回调函数的ID
    */
    uint32_t m_nRemoveIconCallbackId;
};

#endif //EXAMPLES_COMPUTER_VIEW_H_
