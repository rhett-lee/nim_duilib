// basic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "move_control.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 创建主线程
	MainThread thread;

	// 执行主线程循环
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

	return 0;
}

void MainThread::Init()
{
	nbase::ThreadManager::RegisterThread(kThreadUI);

	//初始化全局资源, 使用本地文件夹作为资源
	std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
	resourcePath += L"resources\\";
	ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
	
	MoveControlForm* window = new MoveControlForm(L"move_control", L"move_control", L"main.xml");
	window->CreateWnd(NULL, L"move_control", UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->CenterWindow();
	window->ShowWindow();
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Instance().Shutdown();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();
}
