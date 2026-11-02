#include "WorkerThread.h"
#include "MainForm.h"

DString WorkerThread::MakeThreadName(int32_t nThreadIdentifier)
{
    //线程标识符从 ui::kThreadUser 开始递增，转为从 1 开始的序号，便于在界面上区分各个线程
    int32_t nIndex = nThreadIdentifier - ui::kThreadUser + 1;
    if (nIndex < 1) {
        nIndex = 1;
    }
    //名称中带上线程池内的序号，便于在多线程测试时区分各个线程
    return ui::StringUtil::Printf(_T("WorkerThread-%d"), nIndex);
}

WorkerThread::WorkerThread(int32_t nThreadIdentifier)
    : FrameworkThread(MakeThreadName(nThreadIdentifier), nThreadIdentifier),
    m_pMainForm(nullptr),
    m_nLocalThreadIdentifier(nThreadIdentifier),
    m_bLogLifecycle(true),
    m_nTaskCount(0)
{
}

WorkerThread::~WorkerThread()
{
}

bool WorkerThread::OnInit()
{
    //输出日志
    if (m_bLogLifecycle.load(std::memory_order_relaxed)) {
        PrintLog(_T("WorkerThread::OnInit"));
    }
    return true;
}

void WorkerThread::OnCleanup()
{
    //输出日志
    if (m_bLogLifecycle.load(std::memory_order_relaxed)) {
        PrintLog(_T("WorkerThread::OnCleanup"));
    }
}

void WorkerThread::SetLogLifecycle(bool bLog)
{
    m_bLogLifecycle.store(bLog, std::memory_order_relaxed);
}

void WorkerThread::SetMainForm(MainForm* pMainForm)
{
    m_pMainForm = pMainForm;
}

void WorkerThread::IncreaseTaskCount()
{
    ++m_nTaskCount;
}

int32_t WorkerThread::GetTaskCount() const
{
    return m_nTaskCount.load(std::memory_order_relaxed);
}

void WorkerThread::PrintLog(const DString& log)
{
    //日志中带上线程信息，便于区分日志是哪个线程输出的
    //（不再输出"调用线程ID"：在本线程中执行时它与"线程ID"是同一个值，属于重复信息）
    DString logMsg = ui::StringUtil::Printf(_T("[%s:%s, %s:%s, %s:%d]: %s"),
    ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_2")).c_str(),
                                        ThreadIdToString(GetThreadId()).c_str(),
    ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_3")).c_str(),
                                        GetThreadName().c_str(),
    ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_4")).c_str(),
                                        m_nLocalThreadIdentifier,
                                        log.c_str());
    if (m_pMainForm != nullptr) {
        m_pMainForm->PrintLog(logMsg);
    }
}
