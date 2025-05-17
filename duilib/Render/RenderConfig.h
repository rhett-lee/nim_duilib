#ifndef UI_RENDER_RENDER_CONFIG_H_
#define UI_RENDER_RENDER_CONFIG_H_

//定义这个宏，表示使用LLVM编译Skia
#define DUILIB_RENDER_SKIA_BY_LLVM  1

//Skia引擎需要依赖
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
    #pragma comment (lib, "opengl32.lib")
    #pragma comment (lib, "Imm32.lib")
    #pragma comment (lib, "comctl32.lib")
    #pragma comment (lib, "User32.lib")
    #pragma comment (lib, "shlwapi.lib")

    #ifdef DUILIB_BUILD_FOR_SDL
        //使用SDL库
        #pragma comment (lib, "../../../SDL3/lib/SDL3-static.lib")
        #pragma comment (lib, "Version.lib")
        #pragma comment (lib, "Winmm.lib")
        #pragma comment (lib, "Setupapi.lib")
    #endif
    
    #ifdef DUILIB_RENDER_SKIA_BY_LLVM
        //使用LLVM编译Skia
        #ifdef _DEBUG
            //Debug版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/llvm.x64.debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x64.debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x64.debug/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/llvm.x86.debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x86.debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x86.debug/skshaper.lib")
            #endif //_WIN64    
        #else
            //Release版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/llvm.x64.release/skia.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x64.release/svg.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x64.release/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/llvm.x86.release/skia.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x86.release/svg.lib")
                #pragma comment (lib, "../../../skia/out/llvm.x86.release/skshaper.lib")
            #endif //_WIN64    
        #endif //_DEBUG
    #else
        //使用Visual Studio 2022编译Skia
        #ifdef _DEBUG
            //Debug版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/msvc.x64.debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x64.debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x64.debug/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/msvc.x86.debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x86.debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x86.debug/skshaper.lib")
            #endif //_WIN64    
        #else
            //Release版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/msvc.x64.release/skia.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x64.release/svg.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x64.release/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/msvc.x86.release/skia.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x86.release/svg.lib")
                #pragma comment (lib, "../../../skia/out/msvc.x86.release/skshaper.lib")
            #endif //_WIN64    
        #endif //_DEBUG
    #endif //DUILIB_RENDER_SKIA_BY_LLVM
#else
    //Linux平台
    

#endif

#endif // UI_RENDER_RENDER_CONFIG_H_
