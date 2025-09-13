#include "MainThread.h"
#include "WorkerThread.h"
#include "MainForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI),
    m_pMainForm(nullptr)
{
}

MainThread::~MainThread()
{
}

void MainThread::OnInit()
{
    //输出一条日志
    PrintLog(_T("MainThread::OnInit"));

    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建一个默认带有阴影的居中窗口
    m_pMainForm = new MainForm(this);
    m_pMainForm->CreateWnd(nullptr, ui::WindowCreateParam(_T("threads"), true));
    m_pMainForm->PostQuitMsgWhenClosed(true);
    m_pMainForm->ShowWindow(ui::kSW_SHOW_NORMAL);

    StartThreads();
}

void MainThread::OnCleanup()
{
    //输出一条日志
    PrintLog(_T("MainThread::OnCleanup"));

    StopThreads();
    m_pMainForm = nullptr;

    //清理公共资源
    ui::GlobalManager::Instance().Shutdown();
}

void MainThread::StartThreads()
{
    ui::GlobalManager::Instance().AssertUIThread();
    //创建线程池（线程创建后，需要使用线程标识符来进行线程间通信：ui::kThreadUser + nThread）
    const size_t nMaxThreads = 6;
    for (size_t nThread = 0; nThread < nMaxThreads; ++nThread) {
        std::shared_ptr<WorkerThread> pThread = std::make_shared<WorkerThread>(ui::kThreadUser + (int32_t)nThread);
        pThread->SetMainForm(m_pMainForm.get());
        pThread->Start();
        m_threadPools.push_back(pThread);
    }

    //更新界面状态
    if (m_pMainForm != nullptr) {
        m_pMainForm->UpdateUI();
    }
}

void MainThread::StopThreads()
{
    ui::GlobalManager::Instance().AssertUIThread();
    for (auto pThread : m_threadPools) {
        if (pThread != nullptr) {
            pThread->Stop();
        }
    }
    m_threadPools.clear();

    //更新界面状态
    if (m_pMainForm != nullptr) {
        m_pMainForm->UpdateUI();
    }
}

int32_t MainThread::GetPoolThreadCount() const
{
    return (int32_t)m_threadPools.size();
}

void MainThread::PrintLog(const DString& log)
{
    DString logMsg = ui::StringUtil::Printf(_T("[调用线程ID：%s][线程ID：%s, 线程名称: %s, 线程标识符：%d]: %s"),
                                            ThreadIdToString(std::this_thread::get_id()).c_str(),
                                            ThreadIdToString(GetThreadId()).c_str(),
                                            GetThreadName().c_str(),
                                            GetThreadIdentifier(),
                                            log.c_str());
    if (m_pMainForm != nullptr) {        
        m_pMainForm->PrintLog(logMsg);
    }
}
