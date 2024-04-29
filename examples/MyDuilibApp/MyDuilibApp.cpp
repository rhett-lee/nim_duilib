// MyDuilibApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "MyDuilibApp.h"
#include "MainThread.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //创建主线程
    MainThread thread;

    //执行主线程消息循环
    thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

    //正常退出程序
    return 0;
}
