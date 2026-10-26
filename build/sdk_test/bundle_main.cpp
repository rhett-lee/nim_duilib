// nim_duilib 发布 SDK 包的 macOS App Bundle 运行期用例。
//
// 与 main.cpp（资源放在可执行文件同级 resources/）不同，本用例验证 bundle 布局：
//   - 资源根为 <app>/Contents/Resources/duilib/；
//   - SDL3 动态库由 Contents/Frameworks 通过 @rpath 加载（能取到运行期版本号）。
// Runner 解析本程序的 stdout：必须看到 SDK_TEST_RESULT=ok；包内有 SDL 时还必须
// 看到正的 SDK_TEST_SDL_VERSION。输出保持 ASCII，避免控制台代码页影响解析。
//
// 该文件只在 macOS 的 sdk_test 工程中参与编译（见 CMakeLists.txt 的 APPLE 分支）。

#include "duilib/duilib.h"

#include <cstdio>
#include <exception>
#include <string>

// SDK 的 CMake 包会注入这些宏；直接使用 cmake 包之外的构建方式时按关闭处理。
#ifndef DUILIB_SDL
#define DUILIB_SDL 0
#endif
#ifndef DUILIB_CEF
#define DUILIB_CEF 0
#endif
#ifndef DUILIB_WEBVIEW2
#define DUILIB_WEBVIEW2 0
#endif
#ifndef DUILIB_JPEG_TURBO
#define DUILIB_JPEG_TURBO 0
#endif

#if DUILIB_SDL
// 本用例自己提供 main()，显式禁止 SDL 改写入口点。
#define SDL_MAIN_HANDLED 1
#include <SDL3/SDL.h>
#endif

namespace {

int Flag(bool value)
{
    return value ? 1 : 0;
}

} // namespace

int main()
{
#ifdef DUILIB_DLL
    const int isDll = 1;
#else
    const int isDll = 0;
#endif

    std::printf("SDK_TEST_BEGIN\n");
    std::printf("SDK_TEST_BUNDLE=1\n");
    std::printf("SDK_TEST_FEATURES sdl=%d cef=%d webview2=%d jpeg=%d dll=%d\n",
                Flag(DUILIB_SDL != 0),
                Flag(DUILIB_CEF != 0),
                Flag(DUILIB_WEBVIEW2 != 0),
                Flag(DUILIB_JPEG_TURBO != 0),
                isDll);

#if DUILIB_SDL
    // SDL_GetVersion 只读取动态库中的数据，不需要窗口、事件循环或桌面会话。
    // 能取到版本号说明 dyld 已通过 @rpath 从 <app>/Contents/Frameworks 加载 SDL3；
    // 若 dylib 缺失或 rpath 仍是构建机绝对路径，进程在进入 main 前就会退出。
    const int sdlVersion = SDL_GetVersion();
    std::printf("SDK_TEST_SDL_VERSION=%d\n", sdlVersion);
    if (sdlVersion <= 0) {
        std::printf("SDK_TEST_RESULT=failed: SDL3 runtime version unavailable\n");
        return 7;
    }
#endif

    try {
        // bundle 形态：优先使用 <app>/Contents/Resources/duilib/。
        // 该目录由 nim_duilib_copy_runtime(<target> RESOURCES) 复制。
        const ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(true);
        const std::string resourcePathUtf8 = resourcePath.NativePathA();
        std::printf("SDK_TEST_RESOURCE_ROOT=%s\n", resourcePathUtf8.c_str());
        if (!resourcePath.IsExistsDirectory()) {
            std::printf("SDK_TEST_RESULT=failed: bundle resources directory not found\n");
            return 3;
        }

        // 与下游 bundle 程序一致：Startup 初始化渲染工厂、字体、解码器与内部线程，
        // 并加载 Contents/Resources/duilib/ 下的主题资源。
        const ui::LocalFilesResParam resParam(resourcePath);
        if (!ui::GlobalManager::Instance().Startup(resParam)) {
            std::printf("SDK_TEST_RESULT=failed: Startup returned false\n");
            return 4;
        }
        ui::GlobalManager::Instance().Shutdown();
    } catch (const std::exception& e) {
        std::printf("SDK_TEST_RESULT=failed: exception: %s\n", e.what());
        return 5;
    } catch (...) {
        std::printf("SDK_TEST_RESULT=failed: unknown exception\n");
        return 6;
    }

    std::printf("SDK_TEST_RESULT=ok\n");
    return 0;
}
