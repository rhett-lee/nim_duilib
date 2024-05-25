// controls.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "controls_form.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//开启用鼠标模拟WM_POINTER消息
	//BOOL isOk = EnableMouseInPointer(TRUE);
	//BOOL isOk2 = IsMouseInPointerEnabled();

	MainThread().RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

	return 0;
}

void MiscThread::Init()
{
	nbase::ThreadManager::RegisterThread(thread_id_);
}

void MiscThread::Cleanup()
{
	nbase::ThreadManager::UnregisterThread();
}

void MainThread::Init()
{
	nbase::ThreadManager::RegisterThread(kThreadUI);

	// 启动杂事处理线程
	misc_thread_.reset(new MiscThread(kThreadGlobalMisc, "Global Misc Thread"));
	misc_thread_->Start();

	//初始化全局资源, 使用本地文件夹作为资源
	std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
	resourcePath += L"resources\\";
	ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

	// 创建一个默认带有阴影的居中窗口
	ControlForm* window = new ControlForm();
	window->CreateWnd(NULL, ControlForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->CenterWindow();
	window->ShowWindow();
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Instance().Shutdown();

	misc_thread_->Stop();
	misc_thread_.reset(nullptr);

	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();
}
