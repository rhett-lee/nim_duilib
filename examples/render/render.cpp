// render.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "render.h"
#include "render_form.h"

enum ThreadId
{
	kThreadUI
};

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

	//开启DPI自适应功能
	bool bAdaptDpi = true;

	// 获取资源路径，初始化全局参数
	std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();

	// Debug 模式下使用本地文件夹作为资源
	// 默认皮肤使用 resources\\themes\\default
	// 默认语言使用 resources\\lang\\zh_CN
	// 如需修改请指定 Startup 最后两个参数
	ui::GlobalManager::Instance().Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), bAdaptDpi);

	// 创建一个默认带有阴影的居中窗口
	RenderForm* window = new RenderForm();
	window->CreateWnd(NULL, RenderForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->CenterWindow();
	window->ShowWindow();
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Instance().Shutdown();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();
}
