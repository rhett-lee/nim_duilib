#include "MainForm.h"
#include "MainThread.h"

MainForm::MainForm(MainThread* pMainThread):
    m_pMainThread(pMainThread),
    m_pLogEdit(nullptr),
    m_pRunningTimeLabel(nullptr),
    m_nLogLineNumber(0)
{
}

MainForm::~MainForm()
{
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

    ui::Button* pButtonStart = dynamic_cast<ui::Button*>(FindControl(_T("start_threads")));
    ui::Button* pButtonStop = dynamic_cast<ui::Button*>(FindControl(_T("stop_threads")));

    if (pButtonStart != nullptr) {
        pButtonStart->SetEnabled(false);
        pButtonStart->AttachClick([this, pButtonStart, pButtonStop](const ui::EventArgs&) {
            //启动子线程
            if (m_pMainThread != nullptr) {
                m_pMainThread->StartThreads();
            }
            pButtonStart->SetEnabled(false);
            if (pButtonStop != nullptr) {
                pButtonStop->SetEnabled(true);
            }
            return true;
            });
    }

    if (pButtonStop != nullptr) {
        pButtonStop->AttachClick([this, pButtonStart, pButtonStop](const ui::EventArgs&) {
            //停止子线程
            if (m_pMainThread != nullptr) {
                m_pMainThread->StopThreads();
            }
            pButtonStop->SetEnabled(false);
            if (pButtonStart != nullptr) {
                pButtonStart->SetEnabled(true);
            }
            return true;
            });
    }

    ui::Button* pRunTaskButton = dynamic_cast<ui::Button*>(FindControl(_T("run_task_in_threads")));
    if (pRunTaskButton != nullptr) {
        pRunTaskButton->AttachClick([this](const ui::EventArgs&) {
            //在子线程中执行任务
            int32_t nThreadIdentifier = 1;
            ui::RichEdit* pThreadIdentifier = dynamic_cast<ui::RichEdit*>(FindControl(_T("threads_identifier")));
            if (pThreadIdentifier != nullptr) {
                //从界面获取子线程标识符
                nThreadIdentifier = (int32_t)pThreadIdentifier->GetTextNumber();
            }
            bool bRet = RunTaskInThread(nThreadIdentifier);
            ASSERT(bRet);
            return true;
            });
    }

    //启动定时器，定时更新界面上的运行时间(每秒更新一次)
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI, UiBind(&MainForm::UpdateRunningTime, this), 1000);
}

bool MainForm::RunTaskInThread(int32_t nThreadIdentifier)
{
    //在子线程中执行任务
    bool bRet = false;
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        return bRet;
    }
    //在子线程中执行ExecuteTaskInThread函数（函数也是可以带参数的）
    bRet = ui::GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, UiBind(&MainForm::ExecuteTaskInThread, this));

    //在子线程中执行一个匿名函数(演示功能)
    if (bRet) {
        auto task = [this]() {
                ExecuteTaskInThread();
            };
        bRet = ui::GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, task);
    }
    return bRet;
}

void MainForm::ExecuteTaskInThread()
{
    ASSERT(!ui::GlobalManager::Instance().IsInUIThread());

    //执行具体的计算任务，此处只是显示一条日志（也是通过线程间通信，让主线程更新日志数据到界面）
    DString log = ui::StringUtil::Printf(_T("[执行线程ID:%05d]: MainForm::ExecuteTaskInThread 在子线程中执行"), std::this_thread::get_id());
    PrintLog(log);
}

void MainForm::UpdateRunningTime()
{
    ASSERT(ui::GlobalManager::Instance().IsInUIThread());
    if (m_pRunningTimeLabel != nullptr) {
        //界面显示时间个数：时:分:秒
        auto thisTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_startTime);
        int32_t seconds = (int32_t)thisTime.count();
        DString msg = ui::StringUtil::Printf(_T("%02d:%02d:%02d"), seconds / 60 / 60, seconds / 60, seconds % 60);
        m_pRunningTimeLabel->SetText(msg);
    }
}

void MainForm::UpdateUI()
{
    if (m_pMainThread == nullptr) {
        return;
    }
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        //当前在子线程中执行，将函数执行发送到主线程中执行(使用线程间通信的方式实现，通过UiBind能够保证this指针失效的情况下，不出现非法访问)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&MainForm::UpdateUI, this));
    }
    else {
        //设置线程数等参数
        ui::RichEdit* pThreadIdentifier = dynamic_cast<ui::RichEdit*>(FindControl(_T("threads_identifier")));
        ui::Button* pRunTaskButton = dynamic_cast<ui::Button*>(FindControl(_T("run_task_in_threads")));
        if (pThreadIdentifier != nullptr) {
            if (m_pMainThread->GetPoolThreadCount() > 0) {
                pThreadIdentifier->SetMinNumber(1);
                pThreadIdentifier->SetMaxNumber(m_pMainThread->GetPoolThreadCount() + 2);
                pThreadIdentifier->SetText(_T("1"));
                if (pRunTaskButton != nullptr) {
                    pRunTaskButton->SetEnabled(true);
                }
            }
            else {
                pThreadIdentifier->SetMinNumber(0);
                pThreadIdentifier->SetMaxNumber(0);
                pThreadIdentifier->SetText(_T("0"));
                if (pRunTaskButton != nullptr) {
                    pRunTaskButton->SetEnabled(false);
                }
            }
        }
    }
}

void MainForm::PrintLog(const DString& log)
{
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        //当前在子线程中执行，将函数执行发送到主线程中执行(使用线程间通信的方式实现，通过UiBind能够保证this指针失效的情况下，不出现非法访问)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&MainForm::PrintLog, this, log));
    }
    else {
        //当前在主线程(UI线程)中执行：将信息显示在界面上
        if (m_pLogEdit != nullptr) {
            DString line = ui::StringUtil::Printf(_T("%04d: "), ++m_nLogLineNumber);
            line += log;
            line += _T("\n");
            m_pLogEdit->AppendText(line);
        }
    }
}
