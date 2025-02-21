#include "cef_manager.h"
#include "ui_components/cef_control/app/client_app.h"
#include "ui_components/cef_control/handler/browser_handler.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#pragma warning (pop)

//TODO: 
#pragma comment (lib, "libcef.lib")

namespace nim_comp
{
///////////////////////////////////////////////////////////////////////////////////

// 发现一个非常奇葩的bug，离屏渲染+多线程消息循环模式下，在浏览器对象上右击弹出菜单，是无法正常关闭的
// 翻cef源码后发现菜单是用TrackPopupMenu函数创建的，在MSDN资料上查看后发现调用TrackPopupMenu前
// 需要给其父窗口调用SetForegroundWindow。但是在cef源码中没有调用
// 最终翻cef源码后得到的解决方法是在cef的UI线程创建一个窗口，这个窗体的父窗口必须是在主程序UI线程创建的
// 这样操作之后就不会出现菜单无法关闭的bug了，虽然不知道为什么但是bug解决了
void FixContextMenuBug(HWND hwnd)
{
    ::CreateWindowW(L"Static", L"", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
    ::PostMessage(hwnd, WM_CLOSE, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////
CefManager::CefManager()
{
    browser_count_ = 0;
    is_enable_offset_render_ = true;
}

CefManager::~CefManager()
{
    ASSERT(map_drag_target_reference_.empty());
}

CefManager* CefManager::GetInstance()
{
    static CefManager self;
    return &self;
}

void CefManager::AddCefDllToPath()
{
#if !defined(SUPPORT_CEF)
    return;
#endif

    TCHAR path_envirom[4096] = { 0 };
    GetEnvironmentVariable(_T("path"), path_envirom, 4096);
    
    ui::FilePath cef_path = ui::FilePathUtil::GetCurrentModuleDirectory();

#ifdef _WIN64
    cef_path += _T("libcef\\x64");
#else
    cef_path += _T("libcef\\Win32");
#endif

    if (!cef_path.IsExistsDirectory()) {
        ::MessageBoxW(NULL, L"请解压Cef.rar压缩包", L"提示", MB_OK);
        exit(0);
    }
    DString new_envirom(cef_path.NativePath());
    new_envirom.append(_T(";")).append(path_envirom);
    SetEnvironmentVariable(_T("path"), new_envirom.c_str());
}

// Cef的初始化接口，同时备注了使用各个版本的Cef时遇到的各种坑
// Cef1916版本较稳定，各个功能使用正常，但是某些在debug模式网页打开时会出中断警告（但并不是错误），可能是因为对新html标准支持不够，但是在release模式下正常使用
// Cef2357版本无法使用，当程序处理重定向信息并且重新加载页面后，渲染进程会崩掉
// Cef2526、2623版本对各种新页面都支持，唯一的坑就是debug模式在多线程消息循环开启下，程序退出时会中断，但是release模式正常。
//        (PS:如果开发者不使用负责Cef功能的开发，可以切换到release模式的cef dll文件，这样即使在deubg下也不会报错，修改AddCefDllToPath代码可以切换到release目录)
bool CefManager::Initialize(const DString& app_data_dir, CefSettings &settings, bool is_enable_offset_render /*= true*/)
{
#if !defined(SUPPORT_CEF)
    return true;
#endif
    is_enable_offset_render_ = is_enable_offset_render;

    CefMainArgs main_args(GetModuleHandle(NULL));
    CefRefPtr<ClientApp> app(new ClientApp);
    
    // 如果是在子进程中调用，会堵塞直到子进程退出，并且exit_code返回大于等于0
    // 如果在Browser进程中调用，则立即返回-1
    int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
    if (exit_code >= 0)
        return false;

    GetCefSetting(app_data_dir, settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), NULL);
    
    if (is_enable_offset_render_)
    {
        HWND hwnd = ::CreateWindowW(L"Static", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
        CefPostTask(TID_UI, base::BindOnce(&FixContextMenuBug, hwnd));
    }

    return bRet;
}

void CefManager::UnInitialize()
{
#if !defined(SUPPORT_CEF)
    return;
#endif
    CefShutdown();
}

bool CefManager::IsEnableOffsetRender() const
{
    return is_enable_offset_render_;
}

void CefManager::AddBrowserCount()
{
    browser_count_++;
}

void CefManager::SubBrowserCount()
{
    browser_count_--;
    ASSERT(browser_count_ >= 0);
}

int CefManager::GetBrowserCount()
{
    return browser_count_;
}

void CefManager::PostQuitMessage(int nExitCode)
{
#if !defined(SUPPORT_CEF)
    ::PostQuitMessage(nExitCode);
    return;
#endif

    // 当我们需要结束进程时，千万不要直接调用::PostQuitMessage，这是可能还有浏览器对象没有销毁
    // 应该等所有浏览器对象都销毁后再调用::PostQuitMessage
    if (browser_count_ == 0)
    {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [nExitCode]()
         {
            ::PostQuitMessage(nExitCode);
        });
    }
    else
    {
        auto cb = [nExitCode]()
        {
            CefManager::GetInstance()->PostQuitMessage(nExitCode);
        };

        ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, cb, 500);
    }
}

client::DropTargetHandle CefManager::GetDropTarget(HWND hwnd)
{
    // 查找是否存在这个弱引用
    auto it = map_drag_target_reference_.find(hwnd);
    if (it == map_drag_target_reference_.end()) {
        auto deleter = [this](client::DropTargetWin *src) {
            auto it = map_drag_target_reference_.find(src->GetHWND());
            if (it != map_drag_target_reference_.end()) {
                RevokeDragDrop(src->GetHWND());

                // 移除弱引用对象
                map_drag_target_reference_.erase(it);
            }
            else {
                ASSERT(false);
            }

            delete src;
        };
        
        // 如果不存在就新增一个
        client::DropTargetHandle handle(new client::DropTargetWin(hwnd), deleter);
        map_drag_target_reference_[hwnd] = handle;

        HRESULT register_res = ::RegisterDragDrop(hwnd, handle.get());
        (void)register_res;
        ASSERT((register_res == S_OK) || (register_res == DRAGDROP_E_ALREADYREGISTERED));

        return handle;
    }
    else {
        // 如果存在就返回弱引用对应的强引用指针
        return it->second.lock();
    }
}

void CefManager::GetCefSetting(const DString& app_data_dir, CefSettings &settings)
{
    if (!ui::FilePath(app_data_dir).IsExistsDirectory()) {
        ui::FilePathUtil::CreateDirectories(app_data_dir);
    }
    settings.no_sandbox = true;

    // 设置localstorage，不要在路径末尾加"\\"，否则运行时会报错
    CefString(&settings.cache_path) = app_data_dir + _T("CefLocalStorage");

    // 设置debug log文件位置
    CefString(&settings.log_file) = app_data_dir + _T("cef.log");

    // cef2623、2526版本debug模式:在使用multi_threaded_message_loop时退出程序会触发中断
    // 加入disable-extensions参数可以修复这个问题，但是会导致一些页面打开时报错
    // 开启Cef多线程消息循环，兼容nbase库消息循环
    settings.multi_threaded_message_loop = true;

    // 开启离屏渲染
    settings.windowless_rendering_enabled = is_enable_offset_render_;
}

}
