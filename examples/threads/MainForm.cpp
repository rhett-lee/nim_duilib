#include "MainForm.h"
#include "MainThread.h"
#include "WorkerThread.h"
#include <thread>

namespace
{
    /** 延迟任务的延迟时间(毫秒)
    */
    const int32_t kDelayedTaskMs = 500;

    /** 重复执行任务的间隔时间(毫秒)
    */
    const int32_t kRepeatedTaskIntervalMs = 200;

    /** 重复执行任务的次数
    */
    const int32_t kRepeatedTaskTimes = 5;

    /** 长耗时任务模拟的耗时(毫秒)
    */
    const int32_t kLongTaskWorkMs = 300;

    /** 并发验证时，每个线程投递的任务个数
    */
    const int32_t kConcurrentTasksPerThread = 5;

    /** 并发验证时，每个任务模拟的计算耗时(毫秒)
    */
    const int32_t kConcurrentTaskWorkMs = 30;

    /** 日志框中保留的最大行数，超过后丢弃前半部分
    */
    const int32_t kMaxLogLines = 2000;

    /** 界面状态的刷新周期(毫秒)
    */
    const int32_t kRefreshIntervalMs = 250;
}

MainForm::MainForm(MainThread* pMainThread):
    m_pLogEdit(nullptr),
    m_pRunningTimeLabel(nullptr),
    m_pPoolSizeEdit(nullptr),
    m_pStartButton(nullptr),
    m_pStopButton(nullptr),
    m_pTaskNowButton(nullptr),
    m_pTaskDelayedButton(nullptr),
    m_pTaskRepeatedButton(nullptr),
    m_pTaskLongButton(nullptr),
    m_pTaskConcurrentButton(nullptr),
    m_pTaskCancelButton(nullptr),
    m_pTargetThreadLabel(nullptr),
    m_pTimerIntervalEdit(nullptr),
    m_pTimerTimesEdit(nullptr),
    m_pTimerAddButton(nullptr),
    m_pTimerRemoveButton(nullptr),
    m_pTimerClearButton(nullptr),
    m_pTimerStateLabel(nullptr),
    m_startTime(std::chrono::steady_clock::now()),
    m_nLogLineNumber(0),
    m_nLogLineCount(0),
    m_pMainThread(pMainThread),
    m_nTargetThreadIdentifier(0)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetLangText(const DString& strId)
{
    ui::LangManager& lang = ui::GlobalManager::Instance().Lang();
    if (lang.HasStringByID(strId)) {
        return lang.GetStringByID(strId);
    }
    //语言文件中缺少该ID时直接返回ID本身：避免界面上出现空白，也便于发现漏配的文案
    ASSERT(!"MainForm::GetLangText: string ID not found in language file!");
    return strId;
}

DString MainForm::FormatThreadTag(int32_t nThreadIdentifier)
{
    if (nThreadIdentifier == ui::kThreadUI) {
        return GetLangText(_T("STRID_THREADS_THREAD_UI"));
    }
    if (nThreadIdentifier <= 0) {
        return _T("-");
    }
    //只有线程池中的工作线程才有名称，其他标识符(比如界面线程)直接显示标识符
    if ((nThreadIdentifier >= ui::kThreadUser) && (nThreadIdentifier < ui::kThreadUser + kMaxThreadRows)) {
        return ui::StringUtil::Printf(_T("%s(%d)"),
                WorkerThread::MakeThreadName(nThreadIdentifier).c_str(), nThreadIdentifier);
    }
    return ui::StringUtil::Printf(_T("%d"), nThreadIdentifier);
}

DString MainForm::GetSkinFolder()
{
    return _T("threads");
}

DString MainForm::GetSkinFile()
{
    return _T("threads.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    m_pLogEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("log_view")));
    m_pRunningTimeLabel = dynamic_cast<ui::Label*>(FindControl(_T("running_time")));
    m_startTime = std::chrono::steady_clock::now();

    InitThreadTable();
    InitThreadTaskControls();
    InitTimerControls();

    ui::Button* pClearLogButton = dynamic_cast<ui::Button*>(FindControl(_T("clear_log")));
    if (pClearLogButton != nullptr) {
        pClearLogButton->AttachClick([this](const ui::EventArgs&) {
            ClearLog();
            return true;
        });
    }

    //首次进入时，按线程池的实际状态初始化界面
    UpdateUI();

    //启动定时器，定时更新界面状态(运行时间每秒刷新，线程状态表按更短的周期刷新)
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI, UiBind(&MainForm::UpdateRunningTime, this), 1000);
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI, UiBind(&MainForm::RefreshThreadTable, this), kRefreshIntervalMs);
}

void MainForm::InitThreadTable()
{
    for (int32_t nRow = 0; nRow < kMaxThreadRows; ++nRow) {
        ThreadRow& row = m_threadRows[nRow];
        DString rowName = ui::StringUtil::Printf(_T("thread_row_%d"), nRow);
        row.pRow = FindControl(rowName);
        row.pIdentifier = dynamic_cast<ui::Label*>(FindControl(ui::StringUtil::Printf(_T("thread_tid_%d"), nRow)));
        row.pName = dynamic_cast<ui::Label*>(FindControl(ui::StringUtil::Printf(_T("thread_name_%d"), nRow)));
        row.pSystemThreadId = dynamic_cast<ui::Label*>(FindControl(ui::StringUtil::Printf(_T("thread_ostid_%d"), nRow)));
        row.pState = dynamic_cast<ui::Label*>(FindControl(ui::StringUtil::Printf(_T("thread_state_%d"), nRow)));
        row.pTaskCount = dynamic_cast<ui::Label*>(FindControl(ui::StringUtil::Printf(_T("thread_tasks_%d"), nRow)));
        if (row.pRow != nullptr) {
            //点击某一行时，将该线程设置为任务测试的目标线程
            row.pRow->AttachButtonUp([this, nRow](const ui::EventArgs&) {
                int32_t nThreadIdentifier = ui::kThreadUser + nRow;
                if (m_pMainThread != nullptr) {
                    std::vector<MainThread::ThreadInfo> threadInfo;
                    m_pMainThread->GetThreadInfo(threadInfo);
                    if (nRow >= (int32_t)threadInfo.size()) {
                        return true;
                    }
                    nThreadIdentifier = threadInfo[nRow].nThreadIdentifier;
                }
                SelectThread(nThreadIdentifier);
                return true;
            });
        }
    }
}

void MainForm::InitThreadTaskControls()
{
    m_pPoolSizeEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("pool_size")));
    m_pStartButton = dynamic_cast<ui::Button*>(FindControl(_T("start_threads")));
    m_pStopButton = dynamic_cast<ui::Button*>(FindControl(_T("stop_threads")));
    m_pTargetThreadLabel = dynamic_cast<ui::Label*>(FindControl(_T("target_thread")));

    if (m_pPoolSizeEdit != nullptr) {
        m_pPoolSizeEdit->SetMinNumber(1);
        m_pPoolSizeEdit->SetMaxNumber(kMaxThreadRows);
        m_pPoolSizeEdit->SetText(ui::StringUtil::Printf(_T("%d"), MainThread::kDefaultPoolThreadCount));
    }

    if (m_pStartButton != nullptr) {
        m_pStartButton->AttachClick([this](const ui::EventArgs&) {
            if (m_pMainThread == nullptr) {
                return true;
            }
            int32_t nThreadCount = MainThread::kDefaultPoolThreadCount;
            if (m_pPoolSizeEdit != nullptr) {
                nThreadCount = (int32_t)m_pPoolSizeEdit->GetTextNumber();
            }
            if (nThreadCount < 1) {
                nThreadCount = 1;
            }
            if (nThreadCount > kMaxThreadRows) {
                nThreadCount = kMaxThreadRows;
            }

            const int32_t nOldCount = m_pMainThread->GetPoolThreadCount();
            MainThread::StartResult result = m_pMainThread->StartThreads(nThreadCount);
            if (result == MainThread::StartResult::kAlreadyRunning) {
                //线程池已经是该规模，不做重建，只提示一下，避免日志里刷出成对的生命周期记录
                PrintLog(ui::StringUtil::Printf(_T("%s: %d"),
                        GetLangText(_T("STRID_THREADS_LOG_POOL_RUNNING")).c_str(), nThreadCount));
            }
            else if (nOldCount > 0) {
                PrintLog(ui::StringUtil::Printf(_T("%s: %d -> %d"),
                        GetLangText(_T("STRID_THREADS_LOG_POOL_RESTART")).c_str(), nOldCount, nThreadCount));
            }
            else {
                PrintLog(ui::StringUtil::Printf(_T("%s: %d"),
                        GetLangText(_T("STRID_THREADS_LOG_POOL_STARTED")).c_str(), nThreadCount));
            }
            return true;
        });
    }

    if (m_pStopButton != nullptr) {
        m_pStopButton->AttachClick([this](const ui::EventArgs&) {
            if (m_pMainThread == nullptr) {
                return true;
            }
            const int32_t nThreadCount = m_pMainThread->GetPoolThreadCount();

            //中止进行中的并发验证：未执行的任务会随线程退出被丢弃，整批任务不可能再集齐
            if (m_pStressBatch != nullptr) {
                const int32_t nFinishedCount = m_pStressBatch->nFinished.load(std::memory_order_acquire);
                if (nFinishedCount < m_pStressBatch->nTotal) {
                    m_pStressBatch->bAborted.store(true, std::memory_order_release);
                    PrintLog(ui::StringUtil::Printf(_T("%s: %d/%d"),
                            GetLangText(_T("STRID_THREADS_LOG_CONCURRENT_ABORT")).c_str(),
                            nFinishedCount, m_pStressBatch->nTotal));
                }
                m_pStressBatch.reset();
            }

            //异步停止：界面线程不等待工作线程的当前任务执行完，避免界面卡顿
            PrintLog(ui::StringUtil::Printf(_T("%s: %d"),
                    GetLangText(_T("STRID_THREADS_LOG_POOL_STOPPING")).c_str(), nThreadCount));
            m_pMainThread->StopThreads(true, true);
            return true;
        });
    }

    m_pTaskNowButton = dynamic_cast<ui::Button*>(FindControl(_T("task_now")));
    if (m_pTaskNowButton != nullptr) {
        m_pTaskNowButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskNow();
            return true;
        });
    }

    m_pTaskDelayedButton = dynamic_cast<ui::Button*>(FindControl(_T("task_delayed")));
    if (m_pTaskDelayedButton != nullptr) {
        m_pTaskDelayedButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskDelayed();
            return true;
        });
    }

    m_pTaskRepeatedButton = dynamic_cast<ui::Button*>(FindControl(_T("task_repeated")));
    if (m_pTaskRepeatedButton != nullptr) {
        m_pTaskRepeatedButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskRepeated();
            return true;
        });
    }

    m_pTaskLongButton = dynamic_cast<ui::Button*>(FindControl(_T("task_long")));
    if (m_pTaskLongButton != nullptr) {
        m_pTaskLongButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskLongRunning();
            return true;
        });
    }

    m_pTaskConcurrentButton = dynamic_cast<ui::Button*>(FindControl(_T("task_concurrent")));
    if (m_pTaskConcurrentButton != nullptr) {
        m_pTaskConcurrentButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskConcurrent();
            return true;
        });
    }

    m_pTaskCancelButton = dynamic_cast<ui::Button*>(FindControl(_T("task_cancel")));
    if (m_pTaskCancelButton != nullptr) {
        m_pTaskCancelButton->AttachClick([this](const ui::EventArgs&) {
            OnTaskCancel();
            return true;
        });
    }
}

void MainForm::InitTimerControls()
{
    m_pTimerIntervalEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("timer_interval")));
    m_pTimerTimesEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("timer_times")));
    m_pTimerAddButton = dynamic_cast<ui::Button*>(FindControl(_T("timer_add")));
    m_pTimerRemoveButton = dynamic_cast<ui::Button*>(FindControl(_T("timer_remove")));
    m_pTimerClearButton = dynamic_cast<ui::Button*>(FindControl(_T("timer_clear")));
    m_pTimerStateLabel = dynamic_cast<ui::Label*>(FindControl(_T("timer_state")));

    if (m_pTimerIntervalEdit != nullptr) {
        m_pTimerIntervalEdit->SetMinNumber(50);
        m_pTimerIntervalEdit->SetMaxNumber(60000);
        m_pTimerIntervalEdit->SetText(_T("1000"));
    }
    if (m_pTimerTimesEdit != nullptr) {
        //次数为 0 表示一直重复（TimerManager 中 -1 表示无限次）
        m_pTimerTimesEdit->SetMinNumber(0);
        m_pTimerTimesEdit->SetMaxNumber(1000);
        m_pTimerTimesEdit->SetText(_T("5"));
    }

    if (m_pTimerAddButton != nullptr) {
        m_pTimerAddButton->AttachClick([this](const ui::EventArgs&) {
            OnTimerAdd();
            return true;
        });
    }
    if (m_pTimerRemoveButton != nullptr) {
        m_pTimerRemoveButton->AttachClick([this](const ui::EventArgs&) {
            OnTimerRemoveLast();
            return true;
        });
    }
    if (m_pTimerClearButton != nullptr) {
        m_pTimerClearButton->AttachClick([this](const ui::EventArgs&) {
            OnTimerRemoveAll();
            return true;
        });
    }
    UpdateTimerState();
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    UpdateUI();
    RefreshThreadTable();
    UpdateTimerState();
    return bRet;
}

void MainForm::UpdateUI()
{
    if (m_pMainThread == nullptr) {
        return;
    }
    int32_t nThreadCount = m_pMainThread->GetPoolThreadCount();
    if (m_pPoolSizeEdit != nullptr) {
        if (nThreadCount > 0) {
            m_pPoolSizeEdit->SetText(ui::StringUtil::Printf(_T("%d"), nThreadCount));
        }
    }
    //异步停止的过程中，禁止重复的启动/停止操作
    const bool bStopping = m_pMainThread->IsStopping();
    if (m_pStartButton != nullptr) {
        m_pStartButton->SetEnabled(!bStopping);
    }
    if (m_pStopButton != nullptr) {
        m_pStopButton->SetEnabled((nThreadCount > 0) && !bStopping);
    }

    //线程池为空时，线程任务测试没有意义，禁用相关按钮
    const bool bTaskEnabled = (nThreadCount > 0);
    ui::Button* taskButtons[] = { m_pTaskNowButton, m_pTaskDelayedButton, m_pTaskRepeatedButton,
                                  m_pTaskLongButton, m_pTaskConcurrentButton, m_pTaskCancelButton };
    for (ui::Button* pButton : taskButtons) {
        if (pButton != nullptr) {
            pButton->SetEnabled(bTaskEnabled);
        }
    }
    RefreshThreadTable();
}

void MainForm::OnThreadPoolStopped()
{
    //工作线程已经真正退出（异步停止完成），这里刷新界面并给出结论
    UpdateUI();
    PrintLog(GetLangText(_T("STRID_THREADS_LOG_POOL_STOPPED")));
}

void MainForm::RefreshThreadTable()
{
    if (m_pMainThread == nullptr) {
        return;
    }
    std::vector<MainThread::ThreadInfo> threadInfo;
    m_pMainThread->GetThreadInfo(threadInfo);

    //选中的线程已经不存在时，自动选中第一个线程
    bool bTargetValid = false;
    for (const MainThread::ThreadInfo& info : threadInfo) {
        if (info.nThreadIdentifier == m_nTargetThreadIdentifier) {
            bTargetValid = true;
            break;
        }
    }
    if (!bTargetValid) {
        m_nTargetThreadIdentifier = threadInfo.empty() ? 0 : threadInfo[0].nThreadIdentifier;
    }

    const DString strRunning = GetLangText(_T("STRID_THREADS_STATE_RUNNING"));
    const DString strStopped = GetLangText(_T("STRID_THREADS_STATE_STOPPED"));
    for (int32_t nRow = 0; nRow < kMaxThreadRows; ++nRow) {
        ThreadRow& row = m_threadRows[nRow];
        if (row.pRow == nullptr) {
            continue;
        }
        const bool bHasThread = nRow < (int32_t)threadInfo.size();
        row.pRow->SetVisible(bHasThread);
        if (!bHasThread) {
            continue;
        }
        const MainThread::ThreadInfo& info = threadInfo[nRow];
        if (row.pIdentifier != nullptr) {
            row.pIdentifier->SetText(ui::StringUtil::Printf(_T("%d"), info.nThreadIdentifier));
        }
        if (row.pName != nullptr) {
            row.pName->SetText(info.sThreadName);
        }
        if (row.pSystemThreadId != nullptr) {
            row.pSystemThreadId->SetText(info.sSystemThreadId);
        }
        if (row.pState != nullptr) {
            row.pState->SetText(info.bRunning ? strRunning : strStopped);
        }
        if (row.pTaskCount != nullptr) {
            row.pTaskCount->SetText(ui::StringUtil::Printf(_T("%d"), info.nTaskCount));
        }
        const bool bSelected = (info.nThreadIdentifier == m_nTargetThreadIdentifier);
        row.pRow->SetBkColor(bSelected ? _T("bg_list_item_selected") : _T(""));
    }

    if (m_pTargetThreadLabel != nullptr) {
        if (m_nTargetThreadIdentifier > 0) {
            m_pTargetThreadLabel->SetText(ui::StringUtil::Printf(_T("%s: %s"),
                    GetLangText(_T("STRID_THREADS_LOG_TARGET")).c_str(),
                    FormatThreadTag(m_nTargetThreadIdentifier).c_str()));
        }
        else {
            m_pTargetThreadLabel->SetText(GetLangText(_T("STRID_THREADS_LOG_NO_TARGET")));
        }
    }
}

void MainForm::SelectThread(int32_t nThreadIdentifier)
{
    m_nTargetThreadIdentifier = nThreadIdentifier;
    RefreshThreadTable();
}

int32_t MainForm::GetTargetThreadIdentifier() const
{
    return m_nTargetThreadIdentifier;
}

size_t MainForm::PostTaskToTargetThread(int32_t nDelayMs, int32_t nRepeatTimes,
                                        const DString& sTaskType, int32_t nWorkMs)
{
    return PostTaskToThread(GetTargetThreadIdentifier(), nDelayMs, nRepeatTimes, sTaskType, nWorkMs);
}

size_t MainForm::PostTaskToThread(int32_t nThreadIdentifier, int32_t nDelayMs, int32_t nRepeatTimes,
                                  const DString& sTaskType, int32_t nWorkMs,
                                  const std::shared_ptr<StressTestBatch>& pBatch)
{
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        ASSERT(false);
        return 0;
    }
    if (nThreadIdentifier <= 0) {
        PrintLog(GetLangText(_T("STRID_THREADS_LOG_NO_TARGET")));
        return 0;
    }

    size_t nTaskId = 0;
    auto task = UiBind(&MainForm::ExecuteTaskInThread, this, nThreadIdentifier, sTaskType, nWorkMs, pBatch);
    if (nRepeatTimes > 1) {
        nTaskId = ui::GlobalManager::Instance().Thread().PostRepeatedTask(nThreadIdentifier, task,
                                kRepeatedTaskIntervalMs, nRepeatTimes);
    }
    else if (nDelayMs > 0) {
        nTaskId = ui::GlobalManager::Instance().Thread().PostDelayedTask(nThreadIdentifier, task, nDelayMs);
    }
    else {
        nTaskId = ui::GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, task);
    }
    if (nTaskId != 0) {
        m_taskIds.push_back(nTaskId);
    }
    return nTaskId;
}

void MainForm::ExecuteTaskInThread(int32_t nTargetThreadId, const DString& sTaskType, int32_t nWorkMs,
                                   const std::shared_ptr<StressTestBatch>& pBatch)
{
    ASSERT(!ui::GlobalManager::Instance().IsInUIThread());

    const int32_t nThreadIdentifier = ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier();
    DString systemThreadId = ui::FrameworkThread::ThreadIdToString(std::this_thread::get_id());

    //统计该线程已执行的任务次数（在子线程中累加，界面线程按固定周期读取显示）
    if (m_pMainThread != nullptr) {
        m_pMainThread->OnThreadTaskExecuted(nThreadIdentifier);
    }

    if (nWorkMs > 0) {
        //模拟一个耗时的计算任务：此时界面线程不会被阻塞
        std::this_thread::sleep_for(std::chrono::milliseconds(nWorkMs));
    }

    //执行线程带上线程名称，便于直接看出任务在哪个线程中执行；
    //目标线程只显示标识符（它与执行线程通常是同一个值，写全名会让日志过长而折行）
    DString log = ui::StringUtil::Printf(_T("[%s][%s:%d][%s:%s][%s:%s]"),
        GetLangText(sTaskType).c_str(),
        GetLangText(_T("STRID_THREADS_LOG_TARGET")).c_str(), nTargetThreadId,
        GetLangText(_T("STRID_THREADS_LOG_ACTUAL")).c_str(), FormatThreadTag(nThreadIdentifier).c_str(),
        GetLangText(_T("STRID_THREADS_EXECUTE_LOG_01_1")).c_str(), systemThreadId.c_str());
    PrintLog(log);

    //并发验证：统计整批任务的完成情况（最后一个任务结束时，整批任务的实测耗时才有意义）
    if (pBatch != nullptr) {
        const int32_t nFinished = ++pBatch->nFinished;
        if ((nFinished == pBatch->nTotal) && !pBatch->bAborted.load(std::memory_order_acquire)) {
            const int64_t nElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - pBatch->startTime).count();
            const int32_t nSerialMs = pBatch->nTotal * pBatch->nWorkMs;
            double fSpeedup = 0.0;
            if (nElapsedMs > 0) {
                //加速比：串行基线与实测耗时的比值，明显大于 1 才说明任务真正并行执行
                fSpeedup = (double)nSerialMs / (double)nElapsedMs;
            }
            PrintLog(ui::StringUtil::Printf(GetLangText(_T("STRID_THREADS_LOG_CONCURRENT_DONE")).c_str(),
                    pBatch->nTotal, (int32_t)nElapsedMs, nSerialMs, fSpeedup));
        }
    }
}

void MainForm::OnTaskNow()
{
    size_t nTaskId = PostTaskToTargetThread(0, 1, _T("STRID_THREADS_TASK_NOW"), 0);
    if (nTaskId != 0) {
        PrintLog(ui::StringUtil::Printf(_T("[%s][%s:%d]"),
                GetLangText(_T("STRID_THREADS_LOG_POSTED")).c_str(),
                GetLangText(_T("STRID_THREADS_LOG_TASK_ID")).c_str(), (int32_t)nTaskId));
    }
}

void MainForm::OnTaskDelayed()
{
    size_t nTaskId = PostTaskToTargetThread(kDelayedTaskMs, 1, _T("STRID_THREADS_TASK_DELAYED"), 0);
    if (nTaskId != 0) {
        PrintLog(ui::StringUtil::Printf(_T("[%s %dms][%s:%d]"),
                GetLangText(_T("STRID_THREADS_LOG_POSTED")).c_str(), kDelayedTaskMs,
                GetLangText(_T("STRID_THREADS_LOG_TASK_ID")).c_str(), (int32_t)nTaskId));
    }
}

void MainForm::OnTaskRepeated()
{
    size_t nTaskId = PostTaskToTargetThread(0, kRepeatedTaskTimes, _T("STRID_THREADS_TASK_REPEATED"), 0);
    if (nTaskId != 0) {
        PrintLog(ui::StringUtil::Printf(_T("[%s %d*%dms][%s:%d]"),
                GetLangText(_T("STRID_THREADS_LOG_POSTED")).c_str(), kRepeatedTaskTimes, kRepeatedTaskIntervalMs,
                GetLangText(_T("STRID_THREADS_LOG_TASK_ID")).c_str(), (int32_t)nTaskId));
    }
}

void MainForm::OnTaskLongRunning()
{
    size_t nTaskId = PostTaskToTargetThread(0, 1, _T("STRID_THREADS_TASK_LONG"), kLongTaskWorkMs);
    if (nTaskId != 0) {
        PrintLog(ui::StringUtil::Printf(_T("[%s %dms][%s:%d]"),
                GetLangText(_T("STRID_THREADS_LOG_POSTED")).c_str(), kLongTaskWorkMs,
                GetLangText(_T("STRID_THREADS_LOG_TASK_ID")).c_str(), (int32_t)nTaskId));
    }
}

void MainForm::OnTaskConcurrent()
{
    if (m_pMainThread == nullptr) {
        return;
    }
    std::vector<MainThread::ThreadInfo> threadInfo;
    m_pMainThread->GetThreadInfo(threadInfo);
    if (threadInfo.empty()) {
        PrintLog(GetLangText(_T("STRID_THREADS_LOG_NO_TARGET")));
        return;
    }

    const int32_t nThreadCount = (int32_t)threadInfo.size();
    const int32_t nTotalTasks = nThreadCount * kConcurrentTasksPerThread;

    //先说明测试目的与预期：只看日志时也能知道这个按钮在验证什么
    PrintLog(ui::StringUtil::Printf(GetLangText(_T("STRID_THREADS_LOG_CONCURRENT_PLAN")).c_str(),
            nThreadCount, kConcurrentTasksPerThread, kConcurrentTaskWorkMs,
            nTotalTasks * kConcurrentTaskWorkMs));

    auto pBatch = std::make_shared<StressTestBatch>();
    pBatch->nTotal = nTotalTasks;
    pBatch->nWorkMs = kConcurrentTaskWorkMs;
    pBatch->startTime = std::chrono::steady_clock::now();
    m_pStressBatch = pBatch;

    //向所有工作线程各投递若干个任务：任务由各自的线程并行执行，而不是在界面线程排队
    int32_t nPosted = 0;
    for (const MainThread::ThreadInfo& info : threadInfo) {
        for (int32_t nIndex = 0; nIndex < kConcurrentTasksPerThread; ++nIndex) {
            size_t nTaskId = PostTaskToThread(info.nThreadIdentifier, 0, 1,
                    _T("STRID_THREADS_TASK_CONCURRENT"), kConcurrentTaskWorkMs, pBatch);
            if (nTaskId != 0) {
                ++nPosted;
            }
        }
    }
    PrintLog(ui::StringUtil::Printf(_T("[%s: %d x %d = %d]"),
            GetLangText(_T("STRID_THREADS_LOG_CONCURRENT")).c_str(),
            nThreadCount, kConcurrentTasksPerThread, nPosted));
    RefreshThreadTable();
}

void MainForm::OnTaskCancel()
{
    //从后往前找，取消最近一次投递且尚未完成的任务
    while (!m_taskIds.empty()) {
        size_t nTaskId = m_taskIds.back();
        m_taskIds.pop_back();
        if (nTaskId == 0) {
            continue;
        }
        if (ui::GlobalManager::Instance().Thread().CancelTask(nTaskId)) {
            PrintLog(ui::StringUtil::Printf(_T("[%s][%s:%d]"),
                    GetLangText(_T("STRID_THREADS_LOG_CANCELED")).c_str(),
                    GetLangText(_T("STRID_THREADS_LOG_TASK_ID")).c_str(), (int32_t)nTaskId));
            return;
        }
    }
    PrintLog(GetLangText(_T("STRID_THREADS_LOG_NO_TASK")));
}

void MainForm::OnTimerAdd()
{
    int32_t nIntervalMs = 1000;
    int32_t nTimes = -1;
    if (m_pTimerIntervalEdit != nullptr) {
        nIntervalMs = (int32_t)m_pTimerIntervalEdit->GetTextNumber();
    }
    if (m_pTimerTimesEdit != nullptr) {
        nTimes = (int32_t)m_pTimerTimesEdit->GetTextNumber();
    }
    if (nTimes <= 0) {
        //界面上的 0 表示无限次重复
        nTimes = -1;
    }
    if (nIntervalMs < 1) {
        nIntervalMs = 1;
    }

    auto pItem = std::make_shared<TimerTestItem>();
    pItem->nIntervalMs = nIntervalMs;
    pItem->nTimes = nTimes;

    //定时器回调在界面线程中执行，可以直接操作界面
    size_t nTimerId = ui::GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), [this, pItem]() {
        OnTimerTick(pItem);
    }, (uint32_t)nIntervalMs, nTimes);

    if (nTimerId == 0) {
        PrintLog(GetLangText(_T("STRID_THREADS_LOG_TIMER_FAILED")));
        return;
    }
    pItem->nTimerId = nTimerId;
    m_timers.push_back(pItem);

    PrintLog(ui::StringUtil::Printf(_T("[%s][%s:%d][%s:%dms][%s:%d]"),
            GetLangText(_T("STRID_THREADS_LOG_TIMER_ADDED")).c_str(),
            GetLangText(_T("STRID_THREADS_LOG_TIMER_ID")).c_str(), (int32_t)nTimerId,
            GetLangText(_T("STRID_THREADS_TIMER_INTERVAL_SHORT")).c_str(), nIntervalMs,
            GetLangText(_T("STRID_THREADS_TIMER_TIMES_SHORT")).c_str(), nTimes));
    UpdateTimerState();
}

void MainForm::OnTimerTick(const std::shared_ptr<TimerTestItem>& pItem)
{
    if (pItem == nullptr) {
        return;
    }
    ++pItem->nTickCount;
    //带上执行线程：定时器的回调在界面线程中执行，这解释了为什么回调里可以直接操作界面控件
    const int32_t nTimerTickThread = ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier();
    PrintLog(ui::StringUtil::Printf(_T("[%s][%s:%d][%s:%d][%s:%s]"),
            GetLangText(_T("STRID_THREADS_LOG_TIMER_TICK")).c_str(),
            GetLangText(_T("STRID_THREADS_LOG_TIMER_ID")).c_str(), (int32_t)pItem->nTimerId,
            GetLangText(_T("STRID_THREADS_LOG_TIMER_TICK_COUNT")).c_str(), pItem->nTickCount,
            GetLangText(_T("STRID_THREADS_LOG_ACTUAL")).c_str(), FormatThreadTag(nTimerTickThread).c_str()));

    //达到计划次数后，定时器已经停止，从活动列表中移除
    if ((pItem->nTimes > 0) && (pItem->nTickCount >= pItem->nTimes)) {
        for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
            if (iter->get() == pItem.get()) {
                m_timers.erase(iter);
                break;
            }
        }
        UpdateTimerState();
    }
}

void MainForm::OnTimerRemoveLast()
{
    if (m_timers.empty()) {
        PrintLog(GetLangText(_T("STRID_THREADS_LOG_NO_TIMER")));
        return;
    }
    std::shared_ptr<TimerTestItem> pItem = m_timers.back();
    m_timers.pop_back();
    if ((pItem != nullptr) && (pItem->nTimerId != 0)) {
        ui::GlobalManager::Instance().Timer().RemoveTimer(pItem->nTimerId);
        PrintLog(ui::StringUtil::Printf(_T("[%s][%s:%d]"),
                GetLangText(_T("STRID_THREADS_LOG_TIMER_REMOVED")).c_str(),
                GetLangText(_T("STRID_THREADS_LOG_TIMER_ID")).c_str(), (int32_t)pItem->nTimerId));
    }
    UpdateTimerState();
}

void MainForm::OnTimerRemoveAll()
{
    std::vector<std::shared_ptr<TimerTestItem>> timers;
    timers.swap(m_timers);
    for (const auto& pItem : timers) {
        if ((pItem != nullptr) && (pItem->nTimerId != 0)) {
            ui::GlobalManager::Instance().Timer().RemoveTimer(pItem->nTimerId);
        }
    }
    PrintLog(ui::StringUtil::Printf(_T("[%s: %d]"),
            GetLangText(_T("STRID_THREADS_LOG_TIMER_REMOVED_ALL")).c_str(), (int32_t)timers.size()));
    UpdateTimerState();
}

void MainForm::UpdateTimerState()
{
    if (m_pTimerStateLabel == nullptr) {
        return;
    }
    DString strIds;
    for (const auto& pItem : m_timers) {
        if (pItem == nullptr) {
            continue;
        }
        if (!strIds.empty()) {
            strIds += _T(", ");
        }
        strIds += ui::StringUtil::Printf(_T("%d"), (int32_t)pItem->nTimerId);
    }
    DString text = ui::StringUtil::Printf(_T("%s: %d"),
            GetLangText(_T("STRID_THREADS_TIMER_ACTIVE")).c_str(), (int32_t)m_timers.size());
    if (!strIds.empty()) {
        text += ui::StringUtil::Printf(_T(" (%s: %s)"),
                GetLangText(_T("STRID_THREADS_LOG_TIMER_ID")).c_str(), strIds.c_str());
    }
    m_pTimerStateLabel->SetText(text);
}

void MainForm::UpdateRunningTime()
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    if (m_pRunningTimeLabel != nullptr) {
        //界面显示时间个数：时:分:秒
        auto thisTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_startTime);
        int32_t seconds = (int32_t)thisTime.count();
        //注意：分钟要对 60 取模，否则运行超过 1 小时后会显示成 01:62:05 这样的结果
        DString msg = ui::StringUtil::Printf(_T("%02d:%02d:%02d"), seconds / 3600, (seconds / 60) % 60, seconds % 60);
        m_pRunningTimeLabel->SetText(msg);
    }
}

void MainForm::ClearLog()
{
    if (m_pLogEdit != nullptr) {
        m_pLogEdit->SetText(_T(""));
    }
    m_nLogLineNumber = 0;
    m_nLogLineCount = 0;
}

void MainForm::PrintLog(const DString& log)
{
    //时间戳在产生日志的线程中获取：若等日志投递到界面线程后再取，多线程场景下的时序会被排队延迟扭曲
    const DString strStamp = MakeLogTimeStamp();
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        //当前在子线程中执行，将函数执行发送到主线程中执行(使用线程间通信的方式实现，通过UiBind能够保证this指针失效的情况下，不出现非法访问)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&MainForm::AppendLog, this, strStamp, log));
    }
    else {
        //当前在主线程(UI线程)中执行：将信息显示在界面上
        AppendLog(strStamp, log);
    }
}

DString MainForm::MakeLogTimeStamp() const
{
    const int64_t nElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_startTime).count();
    const int32_t nTotalSeconds = (int32_t)(nElapsedMs / 1000);
    return ui::StringUtil::Printf(_T("%02d:%02d.%03d"),
            nTotalSeconds / 60, nTotalSeconds % 60, (int32_t)(nElapsedMs % 1000));
}

void MainForm::AppendLog(const DString& strStamp, const DString& log)
{
    if (m_pLogEdit == nullptr) {
        return;
    }
    DString line = ui::StringUtil::Printf(_T("%04d %s  "), ++m_nLogLineNumber, strStamp.c_str());
    line += log;
    line += _T("\n");
    m_pLogEdit->AppendText(line);
    ++m_nLogLineCount;
    TrimLogIfNeeded();
}

void MainForm::TrimLogIfNeeded()
{
    if ((m_pLogEdit == nullptr) || (m_nLogLineCount <= kMaxLogLines)) {
        return;
    }

    //丢弃前半部分日志，只保留最近的一部分：避免长时间运行时日志无限增长，拖慢文本框的渲染
    const DString strText = m_pLogEdit->GetText();
    const DString::value_type chNewLine = static_cast<DString::value_type>(_T('\n'));

    int32_t nTotalLines = 0;
    for (DString::value_type ch : strText) {
        if (ch == chNewLine) {
            ++nTotalLines;
        }
    }

    const int32_t nKeepLines = kMaxLogLines / 2;
    const int32_t nDropLines = nTotalLines - nKeepLines;
    if (nDropLines <= 0) {
        m_nLogLineCount = nTotalLines;
        return;
    }

    //定位到需要丢弃的最后一行之后
    size_t nPos = 0;
    int32_t nDropped = 0;
    while (nDropped < nDropLines) {
        const size_t nFound = strText.find(chNewLine, nPos);
        if (nFound == DString::npos) {
            nPos = strText.size();
            break;
        }
        nPos = nFound + 1;
        ++nDropped;
    }

    m_pLogEdit->SetText(strText.substr(nPos));
    m_nLogLineCount = nTotalLines - nDropped;

    //裁剪后把光标和视图拉回末尾，保持"始终看最新日志"的习惯
    const int32_t nLength = m_pLogEdit->GetTextLength();
    m_pLogEdit->SetSel(nLength, nLength);
    m_pLogEdit->EndDown();
}
