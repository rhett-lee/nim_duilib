#include "MainThread.h"
#include "ChatForm.h"
#include "LoginForm.h"
#include "duilib/Core/GlobalManager.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

bool MainThread::OnInit()
{
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //先显示登录窗口；登录成功后切换到聊天主界面（不退出程序）
    m_pLoginForm = new LoginForm();
    m_pLoginForm->SetLoginCallback([this](const DString& strUser) {
        ShowChatWindow(strUser);
        });
    ui::WindowCreateParam loginParam(_T("chat_login"), true);
    loginParam.m_bCenterWindow = true;
    m_pLoginForm->CreateWnd(nullptr, loginParam);
    m_pLoginForm->ShowWindow(ui::kSW_SHOW_NORMAL);
    //未登录时关闭登录窗口即退出程序
    m_pLoginForm->PostQuitMsgWhenClosed(true);
    return true;
}

void MainThread::ShowChatWindow(const DString& strUser)
{
    if (m_pChatForm != nullptr) {
        return;
    }

    //登录成功：关闭登录窗口，但此时不退出程序（退出标志改为 false），
    //退出改由聊天主界面的关闭按钮触发。
    if (m_pLoginForm != nullptr) {
        LoginForm* pLoginForm = m_pLoginForm;
        m_pLoginForm = nullptr;
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [pLoginForm]() {
            if (pLoginForm != nullptr) {
                pLoginForm->PostQuitMsgWhenClosed(false);
                pLoginForm->CloseWnd();
            }
            });
    }

    ChatForm* pChatForm = new ChatForm();
    pChatForm->SetLoginUser(strUser);
    ui::WindowCreateParam chatParam(_T("chat_main"), true);
    chatParam.m_bCenterWindow = true;
    pChatForm->CreateWnd(nullptr, chatParam);
    pChatForm->ShowWindow(ui::kSW_SHOW_NORMAL);
    //聊天主界面关闭时退出程序
    pChatForm->PostQuitMsgWhenClosed(true);
    m_pChatForm = pChatForm;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}
