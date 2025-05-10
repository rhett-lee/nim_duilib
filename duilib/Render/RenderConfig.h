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
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Debug/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Debug/skshaper.lib")
            #endif //_WIN64    
        #else
            //Release版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Release/skia.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Release/svg.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x64.Release/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Release/skia.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Release/svg.lib")
                #pragma comment (lib, "../../../skia/out/LLVM.x86.Release/skshaper.lib")
            #endif //_WIN64    
        #endif //_DEBUG
    #else
        //使用Visual Studio 2022编译Skia
        #ifdef _DEBUG
            //Debug版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Debug/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Debug/skia.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Debug/svg.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Debug/skshaper.lib")
            #endif //_WIN64    
        #else
            //Release版本
            #ifdef  _WIN64
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Release/skia.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Release/svg.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x64.Release/skshaper.lib")
            #else
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Release/skia.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Release/svg.lib")
                #pragma comment (lib, "../../../skia/out/vs2022.x86.Release/skshaper.lib")
            #endif //_WIN64    
        #endif //_DEBUG
    #endif //DUILIB_RENDER_SKIA_BY_LLVM
#else
    //Linux平台
    

#endif

#endif // UI_RENDER_RENDER_CONFIG_H_
