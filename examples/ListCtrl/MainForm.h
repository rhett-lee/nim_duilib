#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 完成界面布局的初始化，各个控件的位置大小等布局信息完成初始化，供子类在界面启动后调整界面布局等操作
    */
    virtual void OnInitLayout() override;

    /** 语言切换事件（当前语言可通过GlobalManager获取）
    * @return 返回false表示不再触发窗口的kWindowLanguageChangedMsg事件, 否则触发该事件
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 填充数据
    */
    void InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId);

    /** 初始化本程序的测试功能相关UI事件
    */
    void InitListCtrlEvents(ui::ListCtrl* pListCtrl);

    /** 执行一些功能测试
    */
    void RunListCtrlTest();

    /** 控制该列
    */
    void OnColumnChanged(size_t nColumnId);

    /** 测试ListCtrl的loading功能
    */
    void TestListCtrlLoading(ui::ListCtrl* pListCtrl);

    /** 测试Loading功能
    */
    void OnTestLoadingProgress();

    /** 测试骨架屏（Skeleton）占位功能
    * @param bCardLayout true=卡片骨架屏；false=列表骨架屏
    */
    void OnTestLoadingSkeleton(bool bCardLayout);

    /** 停止骨架屏占位（超时或手动停止）
    */
    void OnStopLoadingSkeleton();

    /** 测试ListCtrl的事件回调接口
    */
    void TestListCtrlEvents(ui::ListCtrl* pListCtrl);

    /** 测试单格刷新功能（RefreshSubItem）
    */
    void TestRefreshSubItem(ui::ListCtrl* pListCtrl);

    /** 测试ListCtrl级别的列显隐接口（SetColumnVisible/IsColumnVisible）与列显隐变化事件
    */
    void TestColumnVisibleApi(ui::ListCtrl* pListCtrl);

    /** 初始化"压力测试"面板（Phase 0 性能基线）：绑定各按钮事件
    */
    void InitStressTest(ui::ListCtrl* pListCtrl);

    /** 压力测试：载入大数据量（10万行×10列），输出填充耗时
    */
    void StressTestLoad(ui::ListCtrl* pListCtrl);

    /** 压力测试：在列表中间插入若干行，输出耗时
    */
    void StressTestInsert(ui::ListCtrl* pListCtrl);

    /** 压力测试：删除若干行，输出耗时
    */
    void StressTestDelete(ui::ListCtrl* pListCtrl);

    /** 压力测试：整表排序，输出耗时
    */
    void StressTestSort(ui::ListCtrl* pListCtrl);

    /** 压力测试：滚动重绘，输出平均帧耗时
    */
    void StressTestRepaint(ui::ListCtrl* pListCtrl);

    /** 还原为初始演示数据（退出压测状态）
    */
    void StressTestRestore(ui::ListCtrl* pListCtrl);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetEventDisplayInfo(const ui::EventArgs& args);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetItemFilledEventDisplayInfo(const ui::EventArgs& args);

    /** 输出测试日志
    */
    void OutputDebugLog(const DString& logMsg);

private:
    //测试功能: 鼠标划入时显示子控件，鼠标移出时隐藏子控件
    void OnReportViewSubItemFilled(const ui::EventArgs& args);
    void OnReportViewSubItemMouseEnter(const ui::EventArgs& args);
    void OnReportViewSubItemMouseLeave(const ui::EventArgs& args);

private:
    /** loading状态的加载进度（模拟值，实际应用可用按情况计算进度）
    */
    float m_fLoadingPercent;

    /** 图标ID
    */
    uint32_t m_imageId;

    /** 演示单格刷新时使用的进度值（由应用层维护，模拟后台任务进度，不写入ListCtrl的数据模型）
    */
    std::vector<int32_t> m_progressValues;

    /** 压力测试数据是否已载入
    */
    bool m_bStressDataLoaded;
};

#endif //EXAMPLES_MAIN_FORM_H_
