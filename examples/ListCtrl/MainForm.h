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

    /** 把所有行、所有列的单元格设置为可编辑/不可编辑
    */
    void SetAllColumnsEditable(ui::ListCtrl* pListCtrl, bool bEditable);

    /** 设置/取消演示列的"下拉候选编辑器"（kEditCombo）：勾选时可演示单元格下拉候选选择
    */
    void SetComboColumnEditor(ui::ListCtrl* pListCtrl, bool bEnabled);

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

    /** 停止单格刷新演示（RefreshSubItem）：取消定时器并解绑填充回调
    */
    void StopRefreshSubItem(ui::ListCtrl* pListCtrl);

    /** 初始化"填充回调与单格刷新"面板：填充回调开关、单格刷新演示开关
    */
    void InitFillRefreshTests(ui::ListCtrl* pListCtrl);

    /** 测试ListCtrl级别的列显隐接口（SetColumnVisible/IsColumnVisible）与列显隐变化事件
    */
    void TestColumnVisibleApi(ui::ListCtrl* pListCtrl);

    /** 初始化"单元格选择与勾选"面板：单元格选择 API、勾选 API、自动勾选、单元格选择事件
    */
    void InitCellSelectionTests(ui::ListCtrl* pListCtrl);

    /** 初始化"复制与导出"面板：复制为CSV、整表导出CSV、复制/导出开关
    */
    void InitCheckCopyTests(ui::ListCtrl* pListCtrl);

    /** 初始化"进度条与编辑器"面板：进度条外观、单格进度、逐列编辑器类型切换
    */
    void InitProgressEditorTests(ui::ListCtrl* pListCtrl);

    /** 初始化"事件与状态显示"面板：列布局事件、自定义排序、编辑状态事件
    */
    void InitEventStateTests(ui::ListCtrl* pListCtrl);

    /** 更新"最近列事件"显示文本
    */
    void UpdateColumnEventLabel(const DString& strEvent);

    /** 更新"当前格 / 选中区域"显示文本
    */
    void UpdateCellStateLabel(ui::ListCtrl* pListCtrl);

    /** 更新"选中行"显示文本（行选择模式）
    */
    void UpdateRowSelectionLabel(ui::ListCtrl* pListCtrl);

    /** 把进度条外观（文字位置 + 前景色/背景色）应用到单个单元格
    */
    void ApplyProgressAppearanceToSubItem(ui::ListCtrlSubItem* pSubItem);

    /** 把进度条外观应用到"进度列"的所有可见单元格
    */
    void ApplyProgressAppearance(ui::ListCtrl* pListCtrl);

    /** 初始化"压力测试"面板（Phase 0 性能基线）：绑定各按钮事件
    */
    void InitStressTest(ui::ListCtrl* pListCtrl);

    /** 压力测试：载入大数据量（10万行×10列），输出填充耗时
    */
    void StressTestLoad(ui::ListCtrl* pListCtrl);

    /** 压力测试：载入稀疏数据（10万行、仅少量单元格写入），用于验证分页存储的懒分配不膨胀
    */
    void StressTestLoadSparse(ui::ListCtrl* pListCtrl);

    /** 压力测试：读取并显示当前进程内存占用（工作集 / 峰值工作集 / 私有提交）
    */
    void StressTestMemory(ui::ListCtrl* pListCtrl);

    /** 压力测试：在列表中间插入若干行，输出耗时
    */
    void StressTestInsert(ui::ListCtrl* pListCtrl);

    /** 压力测试：删除若干行，输出耗时
    */
    void StressTestDelete(ui::ListCtrl* pListCtrl);

    /** 压力测试：批量删除500行（乱序索引，单趟压缩），输出耗时
    */
    void StressTestDeleteBatch(ui::ListCtrl* pListCtrl);

    /** 压力测试：压缩数据存储（CompactStorage），输出耗时与前后内存对比
    */
    void StressTestCompact(ui::ListCtrl* pListCtrl);

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
    void OnReportViewItemFilled(const ui::EventArgs& args);
    void OnReportViewSubItemFilled(ui::ListCtrlSubItem* pSubItem);
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

    /** "所有列可编辑"勾选框的状态：数据被重新载入后，按该状态重新应用单元格的可编辑属性
    */
    bool m_bAllColumnsEditable;

    /** "Combo列编辑器"勾选框的状态：数据被重新载入后，按该状态重新应用列编辑器类型
    */
    bool m_bComboEditor;

    /** "单元格填充回调（悬停按钮）"勾选框的状态：勾选后，整行/单格填充时会为单元格动态添加悬停按钮
    */
    bool m_bReportViewItemFilled;

    /** 单格刷新演示的定时器回调标志：关闭演示时 Cancel 掉，避免残留定时器
    */
    ui::WeakCallbackFlag m_refreshSubItemFlag;

    /** 进度条文字位置（"center"/"left"/"right"/"outside"）
    */
    DString m_progressTextPos;

    /** 进度条前景色（语义色名或 #AARRGGBB；空表示不覆盖默认值）
    */
    DString m_progressColor;

    /** 进度条背景色（语义色名或 #AARRGGBB；空表示不覆盖默认值）
    */
    DString m_progressBkColor;
};

#endif //EXAMPLES_MAIN_FORM_H_
