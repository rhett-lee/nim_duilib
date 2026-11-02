#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"
#include "MainThread.h"
#include <atomic>
#include <chrono>
#include <vector>
#include <memory>

/** 定时器测试项：记录一个测试定时器在界面上的状态
*/
struct TimerTestItem
{
    /** 定时器ID，即 TimerManager::AddTimer 的返回值
    */
    size_t nTimerId = 0;

    /** 定时器触发间隔，单位：毫秒
    */
    int32_t nIntervalMs = 0;

    /** 定时器计划触发的次数，-1 表示一直重复
    */
    int32_t nTimes = -1;

    /** 已经触发的次数
    */
    int32_t nTickCount = 0;
};

/** 并发验证的一批任务：用于统计整批任务的实际并行耗时
*/
struct StressTestBatch
{
    /** 本批任务总数
    */
    int32_t nTotal = 0;

    /** 每个任务模拟的计算耗时(毫秒)
    */
    int32_t nWorkMs = 0;

    /** 已完成的任务个数（由各个工作线程累加）
    */
    std::atomic<int32_t> nFinished{ 0 };

    /** 批次是否已被中止（线程池在压测过程中被停止时置位）
    */
    std::atomic<bool> bAborted{ false };

    /** 整批任务的投递时刻，用于计算实测耗时
    */
    std::chrono::steady_clock::time_point startTime;
};

class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

    /** 线程状态表的行数上限，与 MainThread::kMaxPoolThreadCount 保持一致
    */
    static const int32_t kMaxThreadRows = MainThread::kMaxPoolThreadCount;

    /** 线程状态表中的一行
    */
    struct ThreadRow
    {
        /** 整行容器，用于设置选中时的背景色
        */
        ui::Control* pRow = nullptr;

        /** 线程标识符
        */
        ui::Label* pIdentifier = nullptr;

        /** 线程名称
        */
        ui::Label* pName = nullptr;

        /** 操作系统线程ID
        */
        ui::Label* pSystemThreadId = nullptr;

        /** 线程状态
        */
        ui::Label* pState = nullptr;

        /** 已执行任务次数
        */
        ui::Label* pTaskCount = nullptr;
    };

public:
    explicit MainForm(MainThread* pMainThread);
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

    /** 语言切换时调用，需要刷新动态生成的文本
    */
    virtual bool OnLanguageChanged() override;

public:
    /** 更新UI状态(可以在子线程中调用)
    */
    void UpdateUI();

    /** 输出日志(可以在子线程中调用)
    */
    void PrintLog(const DString& log);

    /** 线程池异步停止完成后的回调(在界面线程中执行)
    */
    void OnThreadPoolStopped();

private:
    /** 初始化线程状态表
    */
    void InitThreadTable();

    /** 初始化线程任务测试相关的控件
    */
    void InitThreadTaskControls();

    /** 初始化定时器测试相关的控件
    */
    void InitTimerControls();

    /** 刷新线程状态表(显示线程数量、运行状态、任务次数等，并保持选中行)
    */
    void RefreshThreadTable();

    /** 选中某个线程，作为任务测试的目标线程
    * @param [in] nThreadIdentifier 线程标识符
    */
    void SelectThread(int32_t nThreadIdentifier);

    /** 获取当前选中的目标线程标识符，没有选中时返回0
    */
    int32_t GetTargetThreadIdentifier() const;

    /** 在目标线程中投递一个任务
    * @param [in] nDelayMs 延迟时间(毫秒)，0 表示立即执行
    * @param [in] nRepeatTimes 重复次数，1 表示只执行一次
    * @param [in] sTaskType 任务类型描述(用于日志)
    * @param [in] nWorkMs 任务执行时模拟的耗时(毫秒)
    * @return 成功返回任务ID，失败返回0
    */
    size_t PostTaskToTargetThread(int32_t nDelayMs, int32_t nRepeatTimes,
                                  const DString& sTaskType, int32_t nWorkMs);

    /** 向指定线程投递一个任务
    * @param [in] nThreadIdentifier 目标线程的标识符
    * @param [in] nDelayMs 延迟时间(毫秒)，0 表示立即执行
    * @param [in] nRepeatTimes 重复次数，1 表示只执行一次
    * @param [in] sTaskType 任务类型描述(用于日志)
    * @param [in] nWorkMs 任务执行时模拟的耗时(毫秒)
    * @param [in] pBatch 并发验证的批次信息，不属于任何批次时传入 nullptr
    * @return 成功返回任务ID，失败返回0
    */
    size_t PostTaskToThread(int32_t nThreadIdentifier, int32_t nDelayMs, int32_t nRepeatTimes,
                            const DString& sTaskType, int32_t nWorkMs,
                            const std::shared_ptr<StressTestBatch>& pBatch = nullptr);

    /** 在子线程中执行的具体任务（由子线程调用）
    * @param [in] nTargetThreadId 投递任务时指定的目标线程标识符
    * @param [in] sTaskType 任务类型描述(用于日志)
    * @param [in] nWorkMs 任务执行时模拟的耗时(毫秒)
    * @param [in] pBatch 并发验证的批次信息，不属于任何批次时为 nullptr
    */
    void ExecuteTaskInThread(int32_t nTargetThreadId, const DString& sTaskType, int32_t nWorkMs,
                             const std::shared_ptr<StressTestBatch>& pBatch);

    /** 立即执行任务
    */
    void OnTaskNow();

    /** 延迟执行任务
    */
    void OnTaskDelayed();

    /** 重复执行任务
    */
    void OnTaskRepeated();

    /** 长耗时任务
    */
    void OnTaskLongRunning();

    /** 并发压测：向所有线程投递任务
    */
    void OnTaskConcurrent();

    /** 取消最近一次投递的任务
    */
    void OnTaskCancel();

    /** 添加一个测试定时器
    */
    void OnTimerAdd();

    /** 停止最近添加的定时器
    */
    void OnTimerRemoveLast();

    /** 停止全部测试定时器
    */
    void OnTimerRemoveAll();

    /** 定时器触发时的处理（在界面线程中执行）
    * @param [in] pItem 定时器的状态
    */
    void OnTimerTick(const std::shared_ptr<TimerTestItem>& pItem);

    /** 刷新定时器状态显示
    */
    void UpdateTimerState();

    /** 清空日志
    */
    void ClearLog();

    /** 更新运行时间
    */
    void UpdateRunningTime();

    /** 按语言ID取文本
    */
    static DString GetLangText(const DString& strId);

    /** 生成日志头的时间戳，格式为 "mm:ss.mmm"（相对窗口打开时刻）
    * 在产生日志的线程中调用，避免日志投递到界面线程后时序失真
    */
    DString MakeLogTimeStamp() const;

    /** 把一条日志追加到日志框(只能在界面线程中调用)
    * @param [in] strStamp 该日志产生时刻的时间戳
    * @param [in] log 日志内容
    */
    void AppendLog(const DString& strStamp, const DString& log);

    /** 日志条数超过上限时，丢弃前半部分，避免日志无限增长拖慢渲染
    */
    void TrimLogIfNeeded();

    /** 生成日志中使用的线程标签：形如 "WorkerThread-1(100)"
    * 名称便于直接看出是哪个线程，标识符便于与投递日志、界面表格核对
    * @param [in] nThreadIdentifier 线程标识符
    */
    static DString FormatThreadTag(int32_t nThreadIdentifier);

private:
    /** 日志显示控件
    */
    ui::RichEdit* m_pLogEdit;

    /** 运行时间显示
    */
    ui::Label* m_pRunningTimeLabel;

    /** 线程数设置
    */
    ui::RichEdit* m_pPoolSizeEdit;

    /** 启动/停止线程池按钮
    */
    ui::Button* m_pStartButton;
    ui::Button* m_pStopButton;

    /** 线程任务测试的按钮
    */
    ui::Button* m_pTaskNowButton;
    ui::Button* m_pTaskDelayedButton;
    ui::Button* m_pTaskRepeatedButton;
    ui::Button* m_pTaskLongButton;
    ui::Button* m_pTaskConcurrentButton;
    ui::Button* m_pTaskCancelButton;

    /** 目标线程提示
    */
    ui::Label* m_pTargetThreadLabel;

    /** 定时器测试的输入与按钮
    */
    ui::RichEdit* m_pTimerIntervalEdit;
    ui::RichEdit* m_pTimerTimesEdit;
    ui::Button* m_pTimerAddButton;
    ui::Button* m_pTimerRemoveButton;
    ui::Button* m_pTimerClearButton;
    ui::Label* m_pTimerStateLabel;

    /** 开始时间
    */
    std::chrono::steady_clock::time_point m_startTime;

    /** 日志序号
    */
    int32_t m_nLogLineNumber;

    /** 日志框中当前已有的行数(用于判断是否需要裁剪)
    */
    int32_t m_nLogLineCount;

    /** 当前正在进行的并发验证批次，没有进行中的批次时为空
    */
    std::shared_ptr<StressTestBatch> m_pStressBatch;

    /** 线程管理接口
    */
    MainThread* m_pMainThread;

    /** 线程状态表的行
    */
    ThreadRow m_threadRows[kMaxThreadRows];

    /** 当前选中的目标线程标识符，0 表示未选中
    */
    int32_t m_nTargetThreadIdentifier;

    /** 已投递的任务ID，用于取消任务
    */
    std::vector<size_t> m_taskIds;

    /** 测试定时器的列表
    */
    std::vector<std::shared_ptr<TimerTestItem>> m_timers;
};

#endif //EXAMPLES_MAIN_FORM_H_
