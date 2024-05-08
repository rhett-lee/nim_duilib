// basic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "basic_form.h"

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

	//初始化全局资源
	constexpr ui::ResourceType resType = ui::ResourceType::kLocalFiles;
	if (resType == ui::ResourceType::kLocalFiles) {
		//使用本地文件夹作为资源
		std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
		resourcePath += L"resources\\";
		ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
	}
	else if (resType == ui::ResourceType::kZipFile) {
		//使用本地zip压缩包作为资源（压缩包位于exe相同目录）	
		ui::ZipFileResParam resParam;
		resParam.resourcePath = L"resources\\";
		resParam.zipFilePath = nbase::win32::GetCurrentModuleDirectory();
		resParam.zipFilePath += L"resources.zip";
		resParam.zipPassword = "";
		ui::GlobalManager::Instance().Startup(resParam);
	}
	else if (resType == ui::ResourceType::kResZipFile) {
		//使用exe资源文件中的zip压缩包
		ui::ResZipFileResParam resParam;
		resParam.resourcePath = L"resources\\";
		resParam.hResModule = nullptr;
		resParam.resourceName = MAKEINTRESOURCE(IDR_THEME);
		resParam.resourceType = L"THEME";
		resParam.zipPassword = "";
		ui::GlobalManager::Instance().Startup(resParam);
	}
	else {
		return;
	}

	// 创建一个默认带有阴影的居中窗口
	BasicForm* window = new BasicForm();
	window->CreateWnd(NULL, BasicForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->SetIcon(IDI_BASIC);
	window->CenterWindow();
	window->ShowWindow();
	//window->Maximized();
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Instance().Shutdown();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();
}
