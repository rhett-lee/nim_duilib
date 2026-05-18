#import "SDL_MacOS.h"

#if defined(__APPLE__) && defined(__MACH__)

#import <SDL3/SDL.h>
#import <Cocoa/Cocoa.h>

namespace ui
{

void* GetSDLWindowContentView(SDL_Window* sdlWindow) 
{
    if(sdlWindow == nullptr) {
        return nullptr;
    }
    SDL_PropertiesID propID = ::SDL_GetWindowProperties(sdlWindow);
    NSWindow* pNSWindow = (NSWindow*)::SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
    NSView* pNSView = nullptr;
    if (pNSWindow != nullptr) {
        pNSView = [pNSWindow contentView] ;
    }
    return (void*)pNSView;
}

bool SetFocus_MacOS(void* /*pNSWindow*/)
{
    //无需实现：如果用代码实现激活主窗口，则CEF在退出时有错误，导致进程无法正常退出
    return false;
}

/** NSWindow 阴影与圆角模式
 支持：动态切换、反复调用、无边框窗口、正常窗口
 保证：阴影与圆角不会互斥、不会残留、不会显示异常
 解决：所有 'NSWindow may not respond to...' 编译警告
 @param pNSWindow NSWindow* 指针（由 C++ 传入 void*）
 @param nativeShadowType 阴影/圆角模式
 @return 设置成功返回 YES，失败返回 NO
 */
bool ModifyNsWindowShadowType(void* pNSWindow, NativeWindowShadowType nativeShadowType)
{
    if (!pNSWindow) {
        return false;
    }

    // 1. 安全转换并验证类型
    NSWindow* window = (__bridge NSWindow*)pNSWindow;
    if (!window || ![window isKindOfClass:[NSWindow class]]) {
        return false;
    }

    // 2. 确保在主线程执行
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            ModifyNsWindowShadowType(pNSWindow, nativeShadowType);
        });
        return true;
    }

    // 3. 获取 contentView 并验证
    NSView* contentView = window.contentView;
    if (!contentView) {
        // 如果没有内容视图，无法设置图层相关的阴影/圆角，但可以设置窗口级阴影
        if (nativeShadowType == NativeWindowShadowType::kShadowSystemDisabled) {
            [window setHasShadow:NO];
        } else {
            [window setHasShadow:YES];
        }
        [window invalidateShadow];
        return true;
    }

    // 4. 将 SDL 创建的 borderless 窗口转换为文档窗口：
    //    替换 mask → 获得系统圆角和深度阴影
    //    FullSizeContentView + titlebarAppearsTransparent → 隐藏标题栏
    //    全屏退出后通过 toggle NSWindowStyleMaskTitled 强制 WindowServer 重新复合以恢复系统圆角
    if (nativeShadowType != NativeWindowShadowType::kShadowSystemDisabled) {
        window.titlebarAppearsTransparent = YES;
        window.titleVisibility = NSWindowTitleHidden;
        NSWindowStyleMask requiredMask = NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView
                                       | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
        if ((window.styleMask & requiredMask) != requiredMask) {
            [window setStyleMask:requiredMask];

            [[window standardWindowButton:NSWindowCloseButton] setHidden:YES];
            [[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
            [[window standardWindowButton:NSWindowZoomButton] setHidden:YES];
        }
    }

    // 5. 处理图层逻辑
    switch (nativeShadowType) {
        case NativeWindowShadowType::kShadowSystemDisabled:
        {
            [window setHasShadow:NO];
            // 禁用阴影时，通常也移除自定义圆角以恢复系统默认或直角状态
            if (contentView.wantsLayer) {
                CALayer* layer = contentView.layer;
                if (layer) {
                    layer.cornerRadius = 0;
                    layer.masksToBounds = NO; // 可选：根据需求决定是否裁剪
                }
            }
            break;
        }

        case NativeWindowShadowType::kShadowSystemDefault:
        case NativeWindowShadowType::kShadowSystemRound:
        {
            [window setHasShadow:YES];
            // 不设置自定义 cornerRadius，由系统圆角统一处理窗口外观
            window.backgroundColor = [NSColor clearColor];
            break;
        }

        case NativeWindowShadowType::kShadowSystemSmallRound:
        {
            [window setHasShadow:YES];

            contentView.wantsLayer = YES;
            CALayer* layer = contentView.layer;
            if (layer) {
                layer.cornerRadius = 5.0;
                layer.masksToBounds = YES;
            }

            window.backgroundColor = [NSColor clearColor];
            break;
        }

        case NativeWindowShadowType::kShadowSystemDoNotRound:
        {
            [window setHasShadow:YES];

            // 移除圆角
            if (contentView.wantsLayer) {
                CALayer* layer = contentView.layer;
                if (layer) {
                    layer.cornerRadius = 0;
                    // masksToBounds 保持原样或设为 NO，取决于是否有子视图溢出需求
                }
            }
            break;
        }
    }

    // 5. 强制刷新阴影
    [window invalidateShadow];    
    return true;
}

void RestoreWindowShadowAfterFullscreen(void* pNSWindow, NativeWindowShadowType nativeShadowType)
{
    if (!pNSWindow) {
        return;
    }
    // 延迟到下一个 runloop 再设置，确保 SDL 退出全屏的动作完全结束
    dispatch_async(dispatch_get_main_queue(), ^{
        ModifyNsWindowShadowType(pNSWindow, nativeShadowType);
    });
}

}

#endif
