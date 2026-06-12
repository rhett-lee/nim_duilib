#if defined(__APPLE__) || defined(macintosh) || defined(Macintosh)
#include <TargetConditionals.h>  // macOS 平台检测头文件
#if TARGET_OS_MAC               // 明确排除iOS等其他Apple平台

#include "duilib/duilib_config_macos.h"  // macOS专用配置头文件
#include "TestApplication.h"
#include <CoreFoundation/CoreFoundation.h>  // macOS核心服务头文件

// 定义macOS应用程序入口点
int main(int argc, char** argv) {
    // macOS特有的初始化（例如：激活多线程GCD）
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    
    // 创建并运行应用
    TestApplication app;
    app.Run();
    
    return 0;
}

#endif // TARGET_OS_MAC
#endif // __APPLE__ || macintosh || Macintosh
