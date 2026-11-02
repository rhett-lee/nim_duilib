#include "MainThread.h"
#include "WorkerThread.h"
#include "MainForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI),
    m_pMainForm(nullptr),
    m_bStopping(false),
    m_bExiting(false)
{
}

MainThread::~MainThread()
{
    //确保异步停止的辅助线程已经结束，避免 std::thread 析构时触发 std::terminate
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }
}

bool MainThread::OnInit()
{
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //输出一条日志
    PrintLog(_T("MainThread::OnInit"));

    // 创建一个默认带有阴影的居中窗口
    m_pMainForm = new MainForm(this);
    m_pMainForm->CreateWnd(nullptr, ui::WindowCreateParam(_T("threads"), true));
    m_pMainForm->PostQuitMsgWhenClosed(true);
    m_pMainForm->ShowWindow(ui::kSW_SHOW_NORMAL);

    StartThreads(kDefaultPoolThreadCount);
    return true;
}

void MainThread::OnCleanup()
{
    //退出过程中不再向界面线程投递异步回调
    m_bExiting.store(true, std::memory_order_release);

    //输出一条日志
    PrintLog(_T("MainThread::OnCleanup"));

    StopThreads(false);
    m_pMainForm = nullptr;

    //清理公共资源
    ui::GlobalManager::Instance().Shutdown();
}

MainThread::StartResult MainThread::StartThreads(int32_t nThreadCount)
{
    ui::GlobalManager::Instance().AssertUIThread();

    if (nThreadCount < 1) {
        nThreadCount = 1;
    }
    if (nThreadCount > kMaxPoolThreadCount) {
        nThreadCount = kMaxPoolThreadCount;
    }

    //线程池已经是该规模时，不重复创建线程（反复点击"启动"不应该引起线程重建）
    if ((int32_t)m_threadPools.size() == nThreadCount) {
        if (m_pMainForm != nullptr) {
            m_pMainForm->UpdateUI();
        }
        return StartResult::kAlreadyRunning;
    }

    //重建线程池：先静默停止已有线程，避免它们的OnCleanup日志与新线程的OnInit日志交错
    //这里必须同步停止：新线程要复用相同的线程标识符，必须等旧线程注销完成后才能创建
    StopThreads(false);

    //创建线程池（线程创建后，需要使用线程标识符来进行线程间通信：ui::kThreadUser + nThread）
    for (int32_t nThread = 0; nThread < nThreadCount; ++nThread) {
        std::shared_ptr<WorkerThread> pThread = std::make_shared<WorkerThread>(ui::kThreadUser + nThread);
        pThread->SetMainForm(m_pMainForm.get());
        pThread->Start();
        m_threadPools.push_back(pThread);
    }

    //更新界面状态
    if (m_pMainForm != nullptr) {
        m_pMainForm->UpdateUI();
    }
    return StartResult::kStarted;
}

void MainThread::StopThreads(bool bLogLifecycle, bool bAsync)
{
    ui::GlobalManager::Instance().AssertUIThread();

    //上一次异步停止如果还没结束，先等它结束（正常情况早已完成，几乎不会产生等待）
    if (m_stopThread.joinable()) {
        m_stopThread.join();
        m_bStopping.store(false, std::memory_order_release);
    }

    if (m_threadPools.empty()) {
        if (m_pMainForm != nullptr) {
            m_pMainForm->UpdateUI();
        }
        return;
    }

    if (!bAsync) {
        //同步停止：在工作线程上依次执行join（重建线程池、程序退出时使用）
        for (auto& pThread : m_threadPools) {
            if (pThread != nullptr) {
                pThread->SetLogLifecycle(bLogLifecycle);
                pThread->Stop();
            }
        }
        m_threadPools.clear();

        //更新界面状态
        if (m_pMainForm != nullptr) {
            m_pMainForm->UpdateUI();
        }
        return;
    }

    //异步停止：把线程池搬到后台，界面线程立即返回，避免等待工作线程当前任务结束时卡住界面
    m_bStopping.store(true, std::memory_order_release);
    auto pStoppingThreads = std::make_shared<std::vector<std::shared_ptr<WorkerThread>>>();
    pStoppingThreads->swap(m_threadPools);

    m_stopThread = std::thread([this, pStoppingThreads, bLogLifecycle]() {
        for (auto& pThread : *pStoppingThreads) {
            if (pThread != nullptr) {
                pThread->SetLogLifecycle(bLogLifecycle);
                pThread->Stop();
            }
        }
        pStoppingThreads->clear();

        //线程真正退出后，再回到界面线程更新状态
        if (!m_bExiting.load(std::memory_order_acquire)) {
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
                    UiBind(&MainThread::OnAsyncStopFinished, this));
        }
        });

    if (m_pMainForm != nullptr) {
        m_pMainForm->UpdateUI();
    }
}

bool MainThread::IsStopping() const
{
    return m_bStopping.load(std::memory_order_acquire);
}

void MainThread::OnAsyncStopFinished()
{
    m_bStopping.store(false, std::memory_order_release);
    if (m_pMainForm != nullptr) {
        m_pMainForm->OnThreadPoolStopped();
    }
}

int32_t MainThread::GetPoolThreadCount() const
{
    return (int32_t)m_threadPools.size();
}

void MainThread::GetThreadInfo(std::vector<ThreadInfo>& threadInfo) const
{
    threadInfo.clear();
    threadInfo.reserve(m_threadPools.size());
    for (const auto& pThread : m_threadPools) {
        if (pThread == nullptr) {
            continue;
        }
        ThreadInfo info;
        info.nThreadIdentifier = pThread->GetThreadIdentifier();
        info.sThreadName = pThread->GetThreadName();
        info.bRunning = pThread->IsRunning();
        info.nTaskCount = pThread->GetTaskCount();
        if (info.bRunning) {
            info.sSystemThreadId = ThreadIdToString(pThread->GetThreadId());
        }
        threadInfo.push_back(info);
    }
}

void MainThread::OnThreadTaskExecuted(int32_t nThreadIdentifier)
{
    for (const auto& pThread : m_threadPools) {
        if ((pThread != nullptr) && (pThread->GetThreadIdentifier() == nThreadIdentifier)) {
            pThread->IncreaseTaskCount();
            return;
        }
    }
}

void MainThread::PrintLog(const DString& log)
{
    //日志中带上当前线程的信息，便于区分类日志是哪个线程输出的
    //（不再输出"调用线程ID"：在主线程中执行时它与"线程ID"是同一个值，属于重复信息）
    DString logMsg = ui::StringUtil::Printf(_T("[%s:%s, %s:%s, %s:%d]: %s"),
        ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_2")).c_str(),
                                            ThreadIdToString(GetThreadId()).c_str(),
        ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_3")).c_str(),
                                            GetThreadName().c_str(),
        ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_4")).c_str(),
                                            GetThreadIdentifier(),
                                            log.c_str());
    if (m_pMainForm != nullptr) {
        m_pMainForm->PrintLog(logMsg);
    }
}
