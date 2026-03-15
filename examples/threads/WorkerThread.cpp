#include "WorkerThread.h"
#include "MainForm.h"

WorkerThread::WorkerThread(int32_t nThreadIdentifier)
    : FrameworkThread(_T("WorkerThread"), nThreadIdentifier),
    m_pMainForm(nullptr)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::OnInit()
{
    //输出日志
    PrintLog(_T("WorkerThread::OnInit"));
}

void WorkerThread::OnCleanup()
{
    //输出日志
    PrintLog(_T("WorkerThread::OnCleanup"));
}

void WorkerThread::SetMainForm(MainForm* pMainForm)
{
    m_pMainForm = pMainForm;
}

void WorkerThread::PrintLog(const DString& log)
{
    //_T("[调用线程ID：%s][线程ID：%s, 线程名称: %s, 线程标识符：%d]: %s
    DString logMsg = ui::StringUtil::Printf(_T("[%s:%s][%s:%s, %s:%s, %s:%d]: %s"),
    ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_THREADS_EXECUTE_LOG_02_1")).c_str(),
                                        ThreadIdToString(std::this_thread::get_id()).c_str(),
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
