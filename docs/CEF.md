# CEF控件（CefControl）    
nim_duilib的CEF控件（CefControl）是对libcef的集成封装，将CEF组件相关的功能封装成为duilib的一个控件，使得网页浏览功能能够与界面库整合在一起使用。libcef（Chromium Embedded Framework，简称CEF）‌是一个开源框架，允许开发者在其应用程序中嵌入Chromium（Google Chrome浏览器的开源基础）浏览器功能。通过libcef，开发者可以将网页渲染、JavaScript执行及HTML5支持等功能集成到自己的应用中，而无需用户单独安装浏览器‌。  

## 一、libcef的核心功能与架构
### 功能定义
libcef.dll/libcef.so是CEF的核心动态链接库，提供Chromium的浏览器内核功能，包括：    
* 网页渲染‌：支持HTML5、CSS3等现代网页技术‌
* JavaScript执行‌：实现与网页脚本的交互‌
* 网络通信‌：处理HTTP请求、响应及资源加载‌
* 多进程架构‌：通过独立进程管理渲染、插件等模块，提升稳定性‌
### 系统架构
* libcef.dll/libcef.so 位于应用程序与Chromium引擎之间，通过接口调用实现功能交互。例如，应用程序通过libcef.dll/libcef.so加载网页时，Chromium的网络模块和渲染模块分别负责内容获取与显示‌。
* nim_duilib通过CEF控件（CefControl）封装了libcef.dll/libcef.so模块的基本功能，将网页与UI界面集成为一体。

## 二、CEF控件（CefControl）相关的类
| 类名称 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| 接口类 | [duilib/duilib_cef.h](../duilib/duilib_cef.h) | CEF控件的接口类，应用程序需要包含这个头文件`#include "duilib/duilib_cef.h"` |
| CefManager | [duilib/CEFControl/CefManager.h](../duilib/CEFControl/CefManager.h) | CEF控件管理器，负责CEF模块的初始化和反初始化相关工作 |
| CefControl | [duilib/CEFControl/CefControl.h](../duilib/CEFControl/CefControl.h) | CEF控件接口，提供网页浏览相关的基本功能及事件的接受等功能 |
| CefControlEvent | [duilib/CEFControl/CefControlEvent.h](../duilib/CEFControl/CefControlEvent.h) | CEF控件的网页浏览相关事件接收接口 |
| CefControlNative | [duilib/CEFControl/CefControlNative.h](../duilib/CEFControl/CefControlNative.h) | CEF控件窗口模式的封装 |
| CefControlOffScreen | [duilib/CEFControl/CefControlOffScreen.h](../duilib/CEFControl/CefControlOffScreen.h) | CEF控件离屏渲染模式的封装 |

## 三、CEF控件（CefControl）相关的示例程序
| 示例程序 | 说明 |
| :---     | :--- |
| examples\cef           | CEF控件的简单使用示例，该程序使用了CEF控件窗口模式 |
| examples\CefBrowser    | CEF控件的多标签浏览器使用示例， 该程序使用了CEF控件离屏渲染模式|


### 后续文档中，假设nim_duilib的源码根目录为变量：`${NIM_DUILIB_ROOT}` 。   
## 四、CEF控件（CefControl）的使用说明(Windows平台)

### 1. 使用libcef 109版本（适用于使用`CEFSettings.props`属性文件的VC工程)
使用VS打开`${NIM_DUILIB_ROOT}\msvc\PropertySheets\CEFSettings.props`文件，将LibCefVersion109属性值改为`true`，重新编译代码即可。    
通过`${NIM_DUILIB_ROOT}\examples\cef`和`${NIM_DUILIB_ROOT}\examples\CefBrowser`工程可以看到效果（访问一下可以查看UA的网站，显示一下UA可确认）。

### 2. 使用libcef 最新版本（适用于使用`CEFSettings.props`属性文件的VC工程)
使用VS打开`${NIM_DUILIB_ROOT}\msvc\PropertySheets\CEFSettings.props`文件，将LibCefVersion109属性值改为`false`，重新编译代码即可。    
通过`${NIM_DUILIB_ROOT}\examples\cef`和`${NIM_DUILIB_ROOT}\examples\CefBrowser`工程可以看到效果（访问一下可以查看UA的网站，显示一下UA可确认）。

### 3. libcef 137版本二进制文件和资源文件的下载（需要自己下载）
将libcef的二进制文件和资源文件（libcef.dll等）放在以下目录中：    
x64版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win\x64`目录，    
Win32版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win\Win32`目录    
* 64位版本的下载链接：[06/04/2025 - 137.0.10+g7e14fe1+chromium-137.0.7151.69 / Chromium 137.0.7151.69](https://cef-builds.spotifycdn.com/cef_binary_137.0.10%2Bg7e14fe1%2Bchromium-137.0.7151.69_windows64.tar.bz2)
* 32位版本的下载链接：[06/04/2025 - 137.0.10+g7e14fe1+chromium-137.0.7151.69 / Chromium 137.0.7151.69](https://cef-builds.spotifycdn.com/cef_binary_137.0.10%2Bg7e14fe1%2Bchromium-137.0.7151.69_windows32.tar.bz2)    
  下载压缩包后，解压。    
  对于64位版本，将Release目录里的文件和Resources目录里面的文件复制到`${NIM_DUILIB_ROOT}\bin\libcef_win\x64`目录即可，    
  对于32位版本，将Release目录里的文件和Resources目录里面的文件复制到`${NIM_DUILIB_ROOT}\bin\libcef_win\Win32`目录即可。

### 4. libcef 109版本二进制文件和资源文件的下载（需要自己下载）
将libcef 109版本的二进制文件和资源文件（libcef.dll等）放在以下目录中：    
x64版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64`目录，    
Win32版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32`目录    
* 64位版本的下载链接：[02/03/2023 - 109.1.18+gf1c41e4+chromium-109.0.5414.120 / Chromium 109.0.5414.120](https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows64.tar.bz2)
* 32位版本的下载链接：[01/27/2023 - 109.1.18+gf1c41e4+chromium-109.0.5414.120 / Chromium 109.0.5414.120](https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows32.tar.bz2)    
  下载压缩包后，解压。    
  对于64位版本，将Release目录里的文件和Resources目录里面的文件复制到`${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64`目录即可，    
  对于32位版本，将Release目录里的文件和Resources目录里面的文件复制到`${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32`目录即可。    

### 5. libcef二进制文件和资源文件的组织结构说明
libcef二进制文件和资源文件需要放在bin下的指定目录，libcef.dll才能被正常加载。    
比如最新版本CEF，32位版本文件需放在`${NIM_DUILIB_ROOT}\bin\libcef_win\Win32`目录，64位版本文件需放在`${NIM_DUILIB_ROOT}\bin\libcef_win\x64`目录。    
libcef二进制文件和资源文件的基本组织结构为（以64位版本为例）：
```
chrome_elf.dll
d3dcompiler_47.dll
dxcompiler.dll
dxil.dll
libcef.dll
libEGL.dll
libGLESv2.dll
vulkan-1.dll
vk_swiftshader.dll
v8_context_snapshot.bin
vk_swiftshader_icd.json
chrome_100_percent.pak
chrome_200_percent.pak
icudtl.dat
resources.pak
locales（目录，里面包含zh-CN.pak、en-US.pak等语言包）
```
### 6. 如何在自己项目的工程中手工设置libcef相关的属性（未使用`CEFSettings.props`属性文件的VC工程）
#### （1）libcef 新版的支持
libcef 的较新的版本（高于109版本），功能更完善。支持Win10及以上版本的操作系统（Win10/Win11等），不支持Win7等低于Win10的操作系统。    
使用的基本步骤如下（所有目录只写了相对nim_duilib根目录的子目录，实际设置根据自己的项目组织结构可灵活调整）：    
1. 修改VC工程的头文件包含路径：添加`duilib\third_party\libcef\libcef_win`（入口：VS工程属性 -> C/C++ -> General -> Additional Include Directories）    
2. 修改VC工程的库文件包含路径：添加`duilib\third_party\libcef\libcef_win\lib\$(Platform)`（入口：VS工程属性 -> Linker -> General -> Additional Library Directories）    
3. 修改VC工程的库文件包含一下库文件（入口：VS工程属性 -> Linker -> Input -> Additional Dependencies）：    
* Debug版本添加：`libcef.lib;libcef_dll_wrapper_d.lib`    
* Release版本添加：`libcef.lib;libcef_dll_wrapper.lib`  
4. 修改VC工程中，设置libcef.dll为延迟加载，添加`libcef.dll`（入口：VS工程属性 -> Linker -> Input -> Delay Loaded Dlls）    
5. 将libcef的二进制文件和资源文件（libcef.dll等）放在以下目录中：    
* x64版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win\x64`目录
* Win32版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win\Win32`目录

#### （2）libcef 109版本的支持
libcef 109版本支持Win7及以上版本的操作系统（Win7/Win10/Win11等），不支持Windows XP等低版本的操作系统。    
使用的基本步骤如下（所有目录只写了相对nim_duilib根目录的子目录，实际设置根据自己的项目组织结构可灵活调整）：    
1. 修改VC工程的头文件包含路径：添加`duilib\third_party\libcef\libcef_win_109`（入口：VS工程属性 -> C/C++ -> General -> Additional Include Directories）    
2. 修改VC工程的库文件包含路径：添加`duilib\third_party\libcef\libcef_win_109\lib\$(Platform)`（入口：VS工程属性 -> Linker -> General -> Additional Library Directories）    
3. 修改VC工程的库文件包含一下库文件（入口：VS工程属性 -> Linker -> Input -> Additional Dependencies）：    
* Debug版本添加：`libcef.lib;libcef_dll_wrapper_109_d.lib`    
* Release版本添加：`libcef.lib;libcef_dll_wrapper_109.lib`  
4. 修改VC工程中，设置libcef.dll为延迟加载，添加`libcef.dll`（入口：VS工程属性 -> Linker -> Input -> Delay Loaded Dlls）    
5. 将libcef的二进制文件和资源文件（libcef.dll等）放在以下目录中：    
* x64版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64`目录
* Win32版本的放在`${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32`目录

## 五、CEF控件（CefControl）的使用说明（Linux平台）
使用的基本步骤如下（所有目录只写了相对nim_duilib根目录${NIM_DUILIB_ROOT}的子目录，实际设置根据自己的项目组织结构可灵活调整）：    
### 1. libcef 137版本二进制文件和资源文件的下载（需要自己下载）
* x64位版本的下载链接：[06/04/2025 - 137.0.10+g7e14fe1+chromium-137.0.7151.69 / Chromium 137.0.7151.69](https://cef-builds.spotifycdn.com/cef_binary_137.0.10%2Bg7e14fe1%2Bchromium-137.0.7151.69_linux64.tar.bz2)    
* ARM64位版本的下载链接：[06/04/2025 - 137.0.10+g7e14fe1+chromium-137.0.7151.69 / Chromium 137.0.7151.69](https://cef-builds.spotifycdn.com/cef_binary_137.0.10%2Bg7e14fe1%2Bchromium-137.0.7151.69_linuxarm64.tar.bz2)    
  下载压缩包后，解压。    
  然后将Release目录里的文件和Resources目录里面的文件复制到`${NIM_DUILIB_ROOT}/bin/libcef_linux/`目录即可（libcef_linux这个文件夹需要新建）。    
### 2. libcef二进制文件和资源文件的组织结构说明
```
chrome-sandbox
libcef.so
libEGL.so
libGLESv2.so
libvk_swiftshader.so
libvulkan.so.1
v8_context_snapshot.bin
vk_swiftshader_icd.json
chrome_100_percent.pak
chrome_200_percent.pak
icudtl.dat
resources.pak
locales（目录，里面包含zh-CN.pak、en-US.pak等语言包）
```
### 3. 程序的Makefile或者CMakeLists.txt里面需要添加的内容
* 头文件包含路径中，添加`duilib/third_party/libcef/libcef_linux`    
* 库文件包含路径中，添加`bin/libcef_linux`（这个目录包含了libcef的动态库文件：libcef.so等）    
* 设置链接依赖的库，添加` libcef.so cef_dll_wrapper X11`    
* 将libcef的二进制文件和资源文件（libcef.so等）放在以下目录`bin/libcef_linux`中。    

## 六、CEF控件（CefControl）的使用说明（macOS平台）
使用的基本步骤如下（所有目录只写了相对nim_duilib根目录${NIM_DUILIB_ROOT}的子目录，实际设置根据自己的项目组织结构可灵活调整）：    
### 1. libcef 137版本二进制文件和资源文件的下载（需要自己下载）
* x64位版本的下载链接：[05/31/2025 - 137.0.8+gaaaa0bf+chromium-137.0.7151.56 / Chromium 137.0.7151.56](https://cef-builds.spotifycdn.com/cef_binary_137.0.8%2Bgaaaa0bf%2Bchromium-137.0.7151.56_macosx64.tar.bz2)    
* ARM64位版本的下载链接：[06/05/2025 - 137.0.10+g7e14fe1+chromium-137.0.7151.69 / Chromium 137.0.7151.69](https://cef-builds.spotifycdn.com/cef_binary_137.0.10%2Bg7e14fe1%2Bchromium-137.0.7151.69_macosarm64.tar.bz2)    
  下载压缩包后，解压。    
  然后将目录里的内容完整复制到`${NIM_DUILIB_ROOT}/../cef_binary`目录即可（cef_binary这个文件夹需要新建，与nim_duilib目录是平级目录）。    
### 2. cef_binary里面内容列表
```
BUILD.bazel
LICENSE.txt
README.md
bazel
cef_paths2.gypi
CMakeLists.txt
Doxyfile
MODULE.bazel
README.txt
WORKSPACE
cef_paths.gypi
cmake
libcef_dll
include
Debug
Release
tests
```  
编译后，cmake脚本会将依赖的文件复制到目标`*.app/Contents/Frameworks/Chromium Embedded Framework.framework/`路径中。    