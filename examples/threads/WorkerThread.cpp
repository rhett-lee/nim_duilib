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
