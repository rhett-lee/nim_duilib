# libpag的编译方法
## Windows平台
### 一、源码下载(取主分支代码，目前Release的代码有bug，未成功调通流程）
```
# 当前目录: %PAG_ROOT%
git clone https://github.com/Tencent/libpag.git
# 当时的SHA-1: 2aadc0260cc0106e064e4071e00a0432a71d978c
```

### 二、开发环境构建（需要已安装Visual Studio 2022）：
1. 首先确保您已经安装了最新版本的 node.js，然后使用 npm 安装 depsync 命令行工具 ：    
   `npm install -g depsync`
2. 需要在源码根目录运行 depsync 命令来同步第三方依赖仓库：     
   `depsync`
3. 计划编译出libpag.dll，只编译Release版本，但这个DLL使用静态运行库（MT），编译前需要修改的文件有：     
（1）`libpag\third_party\vendor_tools\win.msvc.cmake`    
     修改内容：去掉DLL后缀    
（2）`libpag\third_party\tgfx\third_party\vendor_tools\win.msvc.cmake`    
     修改内容：去掉DLL后缀    
（3）`third_party\tgfx\third_party\skcms\CMakeLists.txt`    
     修改内容：cmake_minimum_required的版本号改为3.18：`cmake_minimum_required(VERSION 3.18)`    
（4）`third_party\tgfx\third_party\pathkit\CMakeLists.txt`    
     修改内容：cmake_minimum_required的版本号改为3.18：`cmake_minimum_required(VERSION 3.18)`    
（5）`third_party\tgfx\third_party\highway\CMakeLists.txt`    
     修改内容：cmake_minimum_required的版本号改为3.18：`cmake_minimum_required(VERSION 3.18)`    

### 三、使用cmake在命令行编译
同步接口头文件：    
将`libpag\include` 复制到目录 `duilib\third_party\libpag\windows\` 目录中，覆盖原有头文件，确保接口一致。    

### 四、使用cmake在命令行编译
1. x64编译    
（1）首先进入VS命令行：“x64 Native Tools Command Prompt for VS 2022”    
     这一步是必须的，否则使用cmake的时候，会报错    
（2）在VS命令行中运行以下命令完成编译：
```
# 当前目录: %PAG_ROOT%
cmake --fresh -G "Visual Studio 17 2022" -A x64 -S ./libpag -B ./build-libpag-x64 -DCMAKE_PROJECT_INCLUDE=./third_party/vendor_tools/win.msvc.cmake -DPAG_USE_QT=OFF -DPAG_USE_PNG_ENCODE=OFF -DPAG_USE_JPEG_ENCODE=OFF -DPAG_USE_WEBP_ENCODE=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.18 -DPAG_USE_HARFBUZZ=OFF -DPAG_USE_RTTR=OFF
cmake --build ./build-libpag-x64 --config Release
```
编译完成后:    
（3）将`build-libpag-x64\Release\libpag.lib` 复制到目录 `duilib\third_party\libpag\windows\lib-vc-x64`    
（4）将`build-libpag-x64\Release\libpag.dll` 复制到目录 `bin`    

2. Win32编译    
（1）首先进入VS命令行：“x86 Native Tools Command Prompt for VS 2022”    
     这一步是必须的，否则使用cmake的时候，会报错    
（2）在VS命令行中运行以下命令完成编译：
```
# 当前目录: %PAG_ROOT%
cmake --fresh -G "Visual Studio 17 2022" -A Win32 -S ./libpag -B ./build-libpag-Win32 -DCMAKE_PROJECT_INCLUDE=./third_party/vendor_tools/win.msvc.cmake -DPAG_USE_QT=OFF -DPAG_USE_PNG_ENCODE=OFF -DPAG_USE_JPEG_ENCODE=OFF -DPAG_USE_WEBP_ENCODE=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.18 -DPAG_USE_HARFBUZZ=OFF -DPAG_USE_RTTR=OFF
cmake --build ./build-libpag-Win32 --config Release
```
编译完成后:    
（3）将`build-libpag-Win32\Release\libpag.lib` 复制到目录 `duilib\third_party\libpag\windows\lib-vc-x86`    
（4）将`build-libpag-Win32\Release\libpag.dll` 复制到目录 `bin`     

3. 查看本地配置（供参考）
```
cmake -B ./build-libpag-x64 -LAH
cmake -B ./build-libpag-Win32 -LAH
```

4. 官方编译参考
```
https://pag.io/docs/en/sdk-desktop.html
```
