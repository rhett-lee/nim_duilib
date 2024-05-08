// popover.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "popover_form.h"

enum ThreadId
{
  kThreadUI
};

class MainThread : public nbase::FrameworkThread
{
public:
  MainThread() : nbase::FrameworkThread("MainThread") {}
  virtual ~MainThread() {}

private:

  virtual void Init() override
  {
	nbase::ThreadManager::RegisterThread(kThreadUI);

    //初始化全局资源, 使用本地文件夹作为资源
    std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
    resourcePath += L"resources\\";
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

	// 创建一个默认带有阴影的居中窗口
	PopoverForm* window = new PopoverForm();
	window->CreateWnd(NULL, PopoverForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->CenterWindow();
	window->ShowWindow();
  }

  /**
  * 虚函数，主线程退出时，做一些清理工作
  * @return void	无返回值
  */
  virtual void Cleanup() override
  {
    ui::GlobalManager::Instance().Shutdown();
    SetThreadWasQuitProperly(true);
    nbase::ThreadManager::UnregisterThread();
  }
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