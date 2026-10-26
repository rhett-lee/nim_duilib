// nim_duilib 发布 SDK 包的运行期用例。
//
// 判定方式：Runner 解析本程序的 stdout。必须看到 SDK_TEST_RESULT=ok，
// 且 SDK_TEST_FEATURES 与包元数据一致。
// 输出保持 ASCII，避免不同控制台代码页导致解析失败。

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
    std::printf("SDK_TEST_FEATURES sdl=%d cef=%d webview2=%d jpeg=%d dll=%d\n",
                Flag(DUILIB_SDL != 0),
                Flag(DUILIB_CEF != 0),
                Flag(DUILIB_WEBVIEW2 != 0),
                Flag(DUILIB_JPEG_TURBO != 0),
                isDll);

    try {
        // 与下游程序一致：资源目录位于可执行文件同级（由 nim_duilib_copy_runtime 复制）。
        const ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
        const std::string resourcePathUtf8 = resourcePath.NativePathA();
        std::printf("SDK_TEST_RESOURCE_ROOT=%s\n", resourcePathUtf8.c_str());
        if (!resourcePath.IsExistsDirectory()) {
            std::printf("SDK_TEST_RESULT=failed: resources directory not found\n");
            return 3;
        }

        // Startup 会初始化渲染工厂、字体、解码器与内部线程并加载主题资源；
        // 能走到 Shutdown 即证明包内静态库（或导入库 + DLL）完整可链接。
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
