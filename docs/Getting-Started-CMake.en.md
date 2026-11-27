English | [简体中文](Getting-Started-CMake.md)

> Last synced: 2026-11-27

# Quick Start (CMake, Windows/Linux/macOS/FreeBSD)

This is the CMake version of [Quick Start (Windows + Visual Studio)](Getting-Started.en.md), applicable to Windows, Linux, macOS, and FreeBSD. It shows how to build nim_duilib from source and create your first application.

Both documents create the same application and window code; only the project layout and build tooling differ.

## Scope and prerequisites

- Supported platforms: Windows (MSVC or MinGW), Linux (x64/ARM64), macOS (Intel/Apple Silicon), and FreeBSD.
- CMake 3.24 or later, Python 3.8 or later.
- Compilers: gcc 10+ or clang 12+; building Skia requires gcc 11+ or clang 12+.
- Linux system dependencies: see [linux-dependencies.en.md](../build/docs/linux-dependencies.en.md).
- The CMake shipped with NFSChina and UnionTech UOS is too old (for example 3.18.4); install CMake 3.24+ manually.
- To package a macOS App Bundle, see [macos-app-bundle.en.md](../build/docs/macos-app-bundle.en.md).

## Get the code and prepare dependencies

1. Get the project code

```bash
git clone https://github.com/rhett-lee/nim_duilib
cd nim_duilib
```

2. Prepare dependencies (the Skia rendering engine, SDL3, etc.; the patch archive ships with the repository, so no external repository is cloned)

```bash
python3 build/build.py deps
```

- If CEF is not needed, add `--no-cef` to reduce dependencies and build time.
- On Windows without the `python3` command, use `python` or `py -3`.
- Dependencies are downloaded next to `nim_duilib`; use `--deps-root` to change the location, and `--offline` for offline environments.

## Build the duilib library

```bash
python3 build/build.py build --config Release --no-examples --target duilib
```

- `--target duilib` builds only the duilib library target (including its third-party dependencies), without tests or examples.
- If you used `--no-cef` when preparing dependencies, add `--no-cef` when building the library as well to keep the configuration consistent.
- Library output: `lib/<arch>/Release/duilib.lib` on Windows, and `lib/Release/libduilib.a` on Linux/macOS/FreeBSD.
- For Debug, use `--config Debug`; the output goes to the corresponding Debug directory (`lib/<arch>/Debug/` on Windows, `lib/Debug/` on Unix).

To verify the environment first, omit `--no-examples` to build all examples; example executables are written to the `bin` directory at the repository root:

```bash
python3 build/build.py build --config Release
./bin/basic
```

Note: the application created below must link against a library built by CMake, not the library produced by the Visual Studio solution (their compile definitions differ, which can cause link errors).

## Create a basic project

Create a project directory outside the repository, for example `~/my_duilib_app`, with the following layout:

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

# nim_duilib repository root (change to your actual path)
set(DUILIB_ROOT "/path/to/nim_duilib")

# Source directory: duilib_bin.cmake collects .cpp files and creates the executable target
set(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include("${DUILIB_ROOT}/cmake/duilib_bin.cmake")
```

Notes:

- `duilib_bin.cmake` configures C++20, include directories, link libraries, and compiler options for the current platform.
- The executable is written to `<DUILIB_ROOT>/bin` by default, where the `resources` directory is ready to use; no working-directory configuration is needed.
- All `.cpp` files in the source directory are added automatically; re-run CMake configuration after adding or removing files.

### main.cpp (cross-platform entry point)

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

    //Create the main thread
    MainThread thread;

    //Run the main thread message loop
    thread.RunMessageLoop();

    //Exit the program normally
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
    // macOS: let the main run loop initialize once
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    #endif

    //Create the main thread
    MainThread thread;

    //Run the main thread message loop
    thread.RunMessageLoop();

    //Exit the program normally
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

/** Main thread
*/
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;

private:
    /** Initialization before running, called before entering the message loop; if initialization fails, the message loop is not entered
    * @return returns true if initialization succeeds, false if it fails
    */
    virtual bool OnInit() override;

    /** Cleanup on exit, called after exiting the message loop
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
    //Initialize global resources, using a local folder as the resource
    //(the resource directory is: <executable directory>/resources/)
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath))) {
        //Resource loading failed (usually the bin/resources directory is missing);
        //returning false means the message loop is not entered
        return false;
    }

    //Create and show the main window (centered, with a default shadow)
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

/** Implementation of the application's main window
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /**  Called when the window is created, implemented by the subclass to obtain the window skin directory
    * @return the subclass must implement and return the window skin directory
    */
    virtual DString GetSkinFolder() override;

    /**  Called when the window is created, implemented by the subclass to obtain the window skin XML description file
    * @return the subclass must implement and return the window skin XML description file
    */
    virtual DString GetSkinFile() override;

    /** Called after the window is created, for the subclass to do some initialization work
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
    //Window initialization is complete; you can now perform initialization for this Form

}
```

## Create the window description XML file

Create the `my_duilib_app` folder under `bin/resources/themes/default` and add a `MyDuilibForm.xml` file with the following content (encoded as UTF-8):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="75%,75%" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false" snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_snap="true"
        size_box="4,4,4,4" icon="public/caption/logo.ico">
    <!-- All controls in the entire window are placed in this VBox container -->
    <VBox bkcolor="bg_window_main">
        <!-- Title bar area -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <!-- Title bar: display area at the top-left of the window -->
            <Control mouse_enabled="false"/>
            <!-- Title bar: window control area on the right, with window minimize, maximize, restore and close buttons -->
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
        <!-- End of title bar area -->

        <!-- Working area: everything other than the title bar is placed in this large Box area -->
        <Box bkcolor="bg_container">
            <VBox margin="0,0,0,0" valign="center" halign="center">
                <Label name="tooltip" text="This is a simple nim_duilib window, with a title bar and regular buttons." height="100%" width="100%" text_align="hcenter,vcenter"/>
            </VBox>
        </Box>
    </VBox>
</Window>
```

Note: the window skin is looked up in the currently active theme directory. If you add theme switching later, copy the `my_duilib_app` folder into `themes/color_light`, `themes/color_dark`, and other theme directories.

## Configure, build, and run

```bash
cd ~/my_duilib_app

# Configure (Release)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release --parallel

# Run (the executable is written to the nim_duilib repository's bin directory)
../nim_duilib/bin/MyDuilibApp
```

With the Visual Studio generator on Windows, `--config` selects the configuration; with single-config generators on Linux/macOS, the option is ignored.

For a Debug build, first build the Debug library, then configure CMake:

```bash
cd ~/nim_duilib
python3 build/build.py build --config Debug --no-examples --target duilib

cd ~/my_duilib_app
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug --parallel
```

Debug and Release use the same executable name, so the configuration built later overwrites the existing file in `bin`.

On success, a window with a title bar, minimize/maximize/restore/close buttons, a fullscreen button, and a line of text is displayed.

If the window is blank or fails to be created, check:

- whether the `bin/resources` directory exists (it is included in the repository);
- whether `bin/resources/themes/default/my_duilib_app/MyDuilibForm.xml` exists and is encoded as UTF-8;
- whether the executable was written to the `bin` directory (resources are resolved relative to the executable directory).

## Production projects using the SDK

The steps above link against the library built from the source repository and are intended for getting started. For production, use the SDK package:

- Installation, upgrade, and migration: see [Deployment.en.md](Deployment.en.md);
- `find_package(nim_duilib CONFIG REQUIRED)` plus `nim_duilib_copy_runtime()` packages the library, headers, and resources into the application directory.

## FAQ

- **Skia / skia.lib not found**: run `python3 build/build.py deps`, then `python3 build/build.py build --config Release --no-examples --target duilib`.
- **Undefined symbols for jpeg/turbojpeg, libpag, etc.**: the linked library was not built by CMake. Rebuild the library with `build.py build` instead of using the Visual Studio solution output.
- **CMake version too old on Linux**: install CMake 3.24+; the CMake shipped with UOS/NFSChina is too old.
- **macOS produces a plain executable instead of a .app**: this guide uses a plain executable; for an App Bundle, see [macos-app-bundle.en.md](../build/docs/macos-app-bundle.en.md).
- **SDL3 shared library not found at run time**: CMake records a build-tree rpath on the build machine; when copying the program elsewhere, set `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS), or ship the SDL3 shared library with the program.
- **CEF required**: do not use `--no-cef` when preparing dependencies or building the library, and see [CEF.en.md](CEF.en.md).
