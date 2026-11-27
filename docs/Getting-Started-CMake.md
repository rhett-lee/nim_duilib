简体中文 | [English](Getting-Started-CMake.en.md)

# 快速上手（CMake，Windows/Linux/macOS/FreeBSD）

本文是 [快速上手（Windows + Visual Studio）](Getting-Started.md) 的 CMake 版本，适用于 Windows、Linux、macOS、FreeBSD，介绍如何从源码编译 nim_duilib，并创建第一个应用。

两篇文档创建的程序和窗口代码相同，区别只在工程组织与编译方式。

## 适用范围与前置条件

- 支持平台：Windows（MSVC 或 MinGW）、Linux（x64/ARM64）、macOS（Intel/Apple Silicon）、FreeBSD。
- CMake 3.24 及以上，Python 3.8 及以上。
- 编译器：gcc 10+ 或 clang 12+；编译 Skia 需要 gcc 11+ 或 clang 12+。
- Linux 系统依赖的安装方法见 [linux-dependencies.md](../build/docs/linux-dependencies.md)。
- 中科方德、统信 UOS 自带的 CMake 版本过低（如 3.18.4），需要自行安装 CMake 3.24+。
- 需要打包为 macOS App Bundle 时，见 [macos-app-bundle.md](../build/docs/macos-app-bundle.md)。

## 获取代码并准备依赖

1. 获取项目代码

```bash
git clone https://github.com/rhett-lee/nim_duilib
cd nim_duilib
```

2. 准备依赖（Skia 绘制引擎、SDL3 等；补丁归档已随仓库提交，无需克隆外部仓库）

```bash
python3 build/build.py deps
```

- 不需要 CEF 时，建议加上 `--no-cef`，可以减少依赖和编译时间。
- Windows 上没有 `python3` 命令时，使用 `python` 或 `py -3`。
- 依赖会下载到 `nim_duilib` 同级的目录，可用 `--deps-root` 修改；离线环境可使用 `--offline`。

## 编译 duilib 库

```bash
python3 build/build.py build --config Release --no-examples --target duilib
```

- `--target duilib`：只构建 duilib 库目标（会先构建其依赖的第三方库），不编译测试和示例。
- 如果准备依赖时使用了 `--no-cef`，编译库时也要加上 `--no-cef`，保持配置一致。
- 库文件输出位置：Windows 为 `lib/<架构>/Release/duilib.lib`，Linux/macOS/FreeBSD 为 `lib/Release/libduilib.a`。
- Debug 配置使用 `--config Debug`，输出到对应的 Debug 目录（Windows 为 `lib/<架构>/Debug/`，Unix 为 `lib/Debug/`）。

如果需要先验证环境，可以不带 `--no-examples` 编译全部示例，示例程序会输出到仓库根目录的 `bin` 目录：

```bash
python3 build/build.py build --config Release
./bin/basic
```

注意：后面创建的应用必须链接由 CMake 构建出的库，不能链接 Visual Studio 解决方案生成的库（两者的编译宏不同，可能导致链接错误）。

## 创建入门工程

在仓库目录之外新建一个工程目录，例如 `~/my_duilib_app`，目录结构如下：

```text
my_duilib_app/
├── CMakeLists.txt
├── main.cpp
├── MainThread.h
├── MainThread.cpp
├── MainForm.h
├── MainForm.cpp
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.24)
project(MyDuilibApp CXX)

# nim_duilib 仓库根目录（按实际路径修改）
set(DUILIB_ROOT "/path/to/nim_duilib")

# 源码目录：duilib_bin.cmake 会自动收集 .cpp 并创建可执行目标
set(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include("${DUILIB_ROOT}/cmake/duilib_bin.cmake")
```

说明：

- `duilib_bin.cmake` 会按平台自动配置 C++20、包含目录、链接库和编译选项。
- 可执行程序默认输出到 `<DUILIB_ROOT>/bin`，那里的 `resources` 目录可以直接使用；无需再配置工作目录。
- 源码目录中的所有 `.cpp` 都会被自动加入工程，文件增删后需要重新执行 CMake 配置。

### main.cpp（跨平台入口）

```cpp
#include "MainThread.h"

#if defined(_WIN32)
    #if defined(__MINGW32__) || defined(__MINGW64__)
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR    lpCmdLine,
                     _In_ int       nCmdShow)
    #else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
    #endif
{
    UNUSED_VARIABLE(hInstance);
    UNUSED_VARIABLE(hPrevInstance);
    UNUSED_VARIABLE(lpCmdLine);
    UNUSED_VARIABLE(nCmdShow);

    //创建主线程
    MainThread thread;

    //执行主线程消息循环
    thread.RunMessageLoop();

    //正常退出程序
    return 0;
}
#else
    #if defined(__APPLE__)
        #include <CoreFoundation/CoreFoundation.h>
    #endif

int main(int argc, char** argv)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);

    #if defined(__APPLE__)
    // macOS：先让主运行循环完成一次初始化
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    #endif

    //创建主线程
    MainThread thread;

    //执行主线程消息循环
    thread.RunMessageLoop();

    //正常退出程序
    return 0;
}
#endif
```

### MainThread.h / MainThread.cpp

```cpp
//MainThread.h
#ifndef EXAMPLES_MAIN_THREAD_H_
#define EXAMPLES_MAIN_THREAD_H_

// duilib
#include "duilib/duilib.h"

/** 主线程
*/
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;

private:
    /** 运行前初始化，在进入消息循环前调用，如果初始化失败则不进入消息循环
    * @return 初始化成功返回true，初始化失败返回false
    */
    virtual bool OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;
};

#endif // EXAMPLES_MAIN_THREAD_H_
```

```cpp
//MainThread.cpp
#include "MainThread.h"
#include "MainForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

bool MainThread::OnInit()
{
    //初始化全局资源，使用本地文件夹作为资源（资源目录为：可执行文件目录/resources/）
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath))) {
        //资源加载失败（通常是 bin/resources 目录不存在），返回false后不会进入消息循环
        return false;
    }

    //创建并显示主窗口（居中显示，默认带有阴影）
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MyDuilibApp"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}
```

### MainForm.h / MainForm.cpp

```cpp
//MainForm.h
#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 应用程序的主窗口实现
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    */
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;
};

#endif //EXAMPLES_MAIN_FORM_H_
```

```cpp
//MainForm.cpp
#include "MainForm.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("my_duilib_app");
}

DString MainForm::GetSkinFile()
{
    return _T("MyDuilibForm.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    //窗口初始化完成，可以进行本Form的初始化

}
```

## 创建窗口描述 XML 文件

在 `bin/resources/themes/default` 目录下创建 `my_duilib_app` 文件夹，并新建 `MyDuilibForm.xml`，内容如下（编码格式为 UTF-8）：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="75%,75%" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false" snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_snap="true"
        size_box="4,4,4,4" icon="public/caption/logo.ico">
    <!-- 整个窗口中，所有控件都放在这个VBox容器中 -->
    <VBox bkcolor="bg_window_main">
        <!-- 标题栏区域 -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <!-- 标题栏：窗口左上角显示区域 -->
            <Control mouse_enabled="false"/>
            <!-- 标题栏：右侧窗口控制区域，窗口最小化、最大化、还原、关闭按钮 -->
            <HBox margin="0,0,0,0" valign="center" width="auto" height="36">
                <Button class="btn_wnd_fullscreen_11" height="32" width="40" name="btn_window_fullscreen" margin="0,2,0,2"/>
                <Button class="btn_wnd_min_11" height="32" width="40" name="btn_window_min" margin="0,2,0,2"/>
                <Box height="stretch" width="40" margin="0,2,0,2">
                    <Button class="btn_wnd_max_11" height="32" width="stretch" name="btn_window_max"/>
                    <Button class="btn_wnd_restore_11" height="32" width="stretch" name="btn_window_restore" visible="false"/>
                </Box>
                <Button class="btn_wnd_close_11" height="stretch" width="40" name="btn_window_close" margin="0,0,0,2"/>
            </HBox>
        </HBox>
        <!-- 标题栏区域结束 -->

        <!-- 工作区域，除了标题栏外的内容都放在这个大的Box区域 -->
        <Box bkcolor="bg_container">
            <VBox margin="0,0,0,0" valign="center" halign="center">
                <Label name="tooltip" text="这是一个简单的 nim_duilib 窗口，带有标题栏和常规按钮。" height="100%" width="100%" text_align="hcenter,vcenter"/>
            </VBox>
        </Box>
    </VBox>
</Window>
```

说明：窗口皮肤是在当前主题目录中查找的，如果后续增加了主题切换功能，需要把 `my_duilib_app` 文件夹复制到 `themes/color_light`、`themes/color_dark` 等主题目录。

## 配置、编译与运行

```bash
cd ~/my_duilib_app

# 配置（Release）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build --config Release --parallel

# 运行（程序输出在 nim_duilib 仓库的 bin 目录）
../nim_duilib/bin/MyDuilibApp
```

Windows 使用 Visual Studio 生成器时，`--config` 用于选择配置；Linux/macOS 使用单配置生成器时该参数会被忽略。

Debug 配置需要先编译 Debug 版库，再配置 CMake：

```bash
cd ~/nim_duilib
python3 build/build.py build --config Debug --no-examples --target duilib

cd ~/my_duilib_app
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug --parallel
```

Debug 与 Release 的可执行文件同名，后编译的配置会覆盖 `bin` 目录中的同名文件。

运行成功时，会显示一个带有标题栏、最小化/最大化/还原/关闭按钮、全屏按钮和一行文字提示的窗口。

如果窗口空白或创建失败，请检查：

- `bin/resources` 目录是否存在（仓库中已包含该目录）；
- `bin/resources/themes/default/my_duilib_app/MyDuilibForm.xml` 是否存在且为 UTF-8 编码；
- 可执行程序是否输出到了 `bin` 目录（资源按可执行文件所在目录查找）。

## 使用 SDK 的正式项目

上面的方式直接使用源码仓库中编译出的库，适合入门。正式项目推荐使用 SDK 包：

- 安装、升级与迁移说明见 [Deployment.md](Deployment.md)；
- 使用 `find_package(nim_duilib CONFIG REQUIRED)` 和 `nim_duilib_copy_runtime()` 可以把库、头文件和资源一起打包到应用目录。

## 常见问题

- **找不到 Skia / skia.lib**：先执行 `python3 build/build.py deps`，再执行 `python3 build/build.py build --config Release --no-examples --target duilib`。
- **链接时出现 jpeg/turbojpeg、libpag 等未定义符号**：应用链接的库不是由 CMake 构建的。请用 `build.py build` 重新编译库，不要使用 Visual Studio 解决方案生成的库。
- **Linux 提示 CMake 版本过低**：安装 CMake 3.24+；UOS/中科方德自带的 CMake 不满足要求。
- **macOS 生成的是普通可执行文件而不是 .app**：本文使用普通可执行文件；需要 App Bundle 时参考 [macos-app-bundle.md](../build/docs/macos-app-bundle.md)。
- **运行时找不到 SDL3 动态库**：在构建机上 CMake 会写入构建期 rpath；拷贝到其他机器运行时，需要设置 `LD_LIBRARY_PATH`（Linux）或 `DYLD_LIBRARY_PATH`（macOS），或随程序一起分发 SDL3 动态库。
- **需要 CEF**：准备依赖和编译库时都不要使用 `--no-cef`，并参考 [CEF.md](CEF.md)。
