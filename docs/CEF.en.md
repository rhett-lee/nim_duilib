English | [简体中文](CEF.md)

> Last synced: 2026-09-29

# CEF Control (CefControl)    

nim_duilib's CEF control (CefControl) is an integrated wrapper around libcef, encapsulating the functionality related to the CEF component as a control of duilib, so that the web browsing function can be integrated with the GUI library. libcef (Chromium Embedded Framework, CEF for short) is an open-source framework that allows developers to embed the Chromium (the open-source foundation of the Google Chrome browser) browser functionality into their applications. Through libcef, developers can integrate web rendering, JavaScript execution, and HTML5 support into their own applications without requiring the user to install a browser separately.

## I. Core Functions and Architecture of libcef
### Function Definition
libcef.dll/libcef.so is the core dynamic-link library (DLL) of CEF, providing the browser kernel functionality of Chromium, including:
* Web rendering: supports modern web technologies such as HTML5 and CSS3
* JavaScript execution: enables interaction with web page scripts
* Network communication: handles HTTP requests, responses, and resource loading
* Multi-process architecture: manages rendering, plug-ins, and other modules through independent processes to improve stability
### System Architecture
* libcef.dll/libcef.so sits between the application and the Chromium engine, and implements functional interaction through interface calls. For example, when the application loads a web page through libcef.dll/libcef.so, Chromium's network module and rendering module are responsible for content acquisition and display respectively.
* nim_duilib encapsulates the basic functions of the libcef.dll/libcef.so module through the CEF control (CefControl), integrating the web page and the UI interface into one.

## II. Classes Related to the CEF Control (CefControl)
| Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- |
| Interface Class | [duilib/duilib_cef.h](../duilib/duilib_cef.h) | The interface class of the CEF control; applications need to include this header file `#include "duilib/duilib_cef.h"` |
| CefManager | [duilib/CEFControl/CefManager.h](../duilib/CEFControl/CefManager.h) | CEF control manager, responsible for the initialization and deinitialization of the CEF module |
| CefControl | [duilib/CEFControl/CefControl.h](../duilib/CEFControl/CefControl.h) | CEF control interface, providing basic web browsing functions and event reception |
| CefControlEvent | [duilib/CEFControl/CefControlEvent.h](../duilib/CEFControl/CefControlEvent.h) | Event reception interface for web browsing related to the CEF control |
| CefControlNative | [duilib/CEFControl/CefControlNative.h](../duilib/CEFControl/CefControlNative.h) | Encapsulation of the window mode of the CEF control |
| CefControlOffScreen | [duilib/CEFControl/CefControlOffScreen.h](../duilib/CEFControl/CefControlOffScreen.h) | Encapsulation of the off-screen rendering mode of the CEF control |

## III. Example Programs Related to the CEF Control (CefControl)
| Example Program | Description |
| :---     | :--- |
| examples\cef           | A simple usage example of the CEF control; this program uses the window mode of the CEF control |
| examples\CefBrowser    | A multi-tab browser usage example of the CEF control; this program uses the off-screen rendering mode of the CEF control |


### In the following documents, assume the source root directory of nim_duilib is the variable: `${NIM_DUILIB_ROOT}` .    
## IV. Usage Notes for the CEF Control (CefControl) (Windows Platform)

### 1. Download of the Latest Version Binary Files and Resource Files of libcef (You Need to Download Them Yourself)
Place the binary files and resource files of libcef (libcef.dll, etc.) in the following directories:    
The x64 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win\x64` directory,    
The Win32 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win\Win32` directory    
* Download link for the 64-bit version: [11/11/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_windows64.tar.bz2)
* Download link for the 32-bit version: [11/11/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_windows32.tar.bz2)    
  After downloading the archive, extract it.    
  For the 64-bit version, copy the files in the Release directory and the files in the Resources directory to the `${NIM_DUILIB_ROOT}\bin\libcef_win\x64` directory,    
  For the 32-bit version, copy the files in the Release directory and the files in the Resources directory to the `${NIM_DUILIB_ROOT}\bin\libcef_win\Win32` directory.

### 2. Download of the 109 Version Binary Files and Resource Files of libcef (You Need to Download Them Yourself)
Place the binary files and resource files of libcef 109 (libcef.dll, etc.) in the following directories:    
The x64 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64` directory,    
The Win32 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32` directory    
* Download link for the 64-bit version: [02/03/2023 - 109.1.18+gf1c41e4+chromium-109.0.5414.120 / Chromium 109.0.5414.120](https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows64.tar.bz2)
* Download link for the 32-bit version: [01/27/2023 - 109.1.18+gf1c41e4+chromium-109.0.5414.120 / Chromium 109.0.5414.120](https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows32.tar.bz2)    
  After downloading the archive, extract it.    
  For the 64-bit version, copy the files in the Release directory and the files in the Resources directory to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64` directory,    
  For the 32-bit version, copy the files in the Release directory and the files in the Resources directory to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32` directory.    

### 3. Organization Structure of libcef Binary Files and Resource Files
The binary files and resource files of libcef need to be placed in the designated directory under bin so that libcef.dll can be loaded normally.    
For example, for the latest version of CEF, the 32-bit version files need to be placed in the `${NIM_DUILIB_ROOT}\bin\libcef_win\Win32` directory, and the 64-bit version files need to be placed in the `${NIM_DUILIB_ROOT}\bin\libcef_win\x64` directory.    
The basic organization structure of the libcef binary files and resource files is (taking the 64-bit version as an example):
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
locales (directory, containing language packs such as zh-CN.pak, en-US.pak, etc.)
```

### 4. Using libcef Version 109 (For VC Projects That Use the `CEFSettings.props` Property Sheet)
Open the `${NIM_DUILIB_ROOT}\msvc\PropertySheets\CEFSettings.props` file with VS, change the LibCefVersion109 property value to `true`, and recompile the code.    
You can see the effect through the `${NIM_DUILIB_ROOT}\examples\cef` and `${NIM_DUILIB_ROOT}\examples\CefBrowser` projects (visit a website that can display the UA, and show the UA to confirm).

### 5. Using the Latest Version of libcef (For VC Projects That Use the `CEFSettings.props` Property Sheet)
Open the `${NIM_DUILIB_ROOT}\msvc\PropertySheets\CEFSettings.props` file with VS, change the LibCefVersion109 property value to `false`, and recompile the code.    
You can see the effect through the `${NIM_DUILIB_ROOT}\examples\cef` and `${NIM_DUILIB_ROOT}\examples\CefBrowser` projects (visit a website that can display the UA, and show the UA to confirm).

### 6. How to Manually Set libcef Related Properties in Your Own Project (VC Projects That Do Not Use the `CEFSettings.props` Property Sheet)
#### (1) Support for the New Version of libcef
Newer versions of libcef (higher than version 109) have more complete functionality. They support operating systems of Windows 10 and above (Win10/Win11, etc.), and do not support operating systems lower than Win10 such as Win7.    
The basic usage steps are as follows (all directories only write the subdirectories relative to the nim_duilib root directory; the actual settings can be flexibly adjusted according to your own project organization structure):    
1. Modify the header file include path of the VC project: add `duilib\third_party\libcef\libcef_win` (entry: VS project properties -> C/C++ -> General -> Additional Include Directories)    
2. Modify the library file include path of the VC project: add `duilib\third_party\libcef\libcef_win\lib\$(Platform)` (entry: VS project properties -> Linker -> General -> Additional Library Directories)    
3. Modify the VC project to include the following library files (entry: VS project properties -> Linker -> Input -> Additional Dependencies):    
* For the Debug version, add: `libcef.lib;libcef_dll_wrapper_d.lib`    
* For the Release version, add: `libcef.lib;libcef_dll_wrapper.lib`  
4. In the VC project, set libcef.dll to be delay-loaded, and add `libcef.dll` (entry: VS project properties -> Linker -> Input -> Delay Loaded Dlls)    
5. Place the binary files and resource files of libcef (libcef.dll, etc.) in the following directories:    
* The x64 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win\x64` directory
* The Win32 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win\Win32` directory

#### (2) Support for libcef Version 109
libcef version 109 supports operating systems of Windows 7 and above (Win7/Win10/Win11, etc.), and does not support low-version operating systems such as Windows XP.    
The basic usage steps are as follows (all directories only write the subdirectories relative to the nim_duilib root directory; the actual settings can be flexibly adjusted according to your own project organization structure):    
1. Modify the header file include path of the VC project: add `duilib\third_party\libcef\libcef_win_109` (entry: VS project properties -> C/C++ -> General -> Additional Include Directories)    
2. Modify the library file include path of the VC project: add `duilib\third_party\libcef\libcef_win_109\lib\$(Platform)` (entry: VS project properties -> Linker -> General -> Additional Library Directories)    
3. Modify the VC project to include the following library files (entry: VS project properties -> Linker -> Input -> Additional Dependencies):    
* For the Debug version, add: `libcef.lib;libcef_dll_wrapper_109_d.lib`    
* For the Release version, add: `libcef.lib;libcef_dll_wrapper_109.lib`  
4. In the VC project, set libcef.dll to be delay-loaded, and add `libcef.dll` (entry: VS project properties -> Linker -> Input -> Delay Loaded Dlls)    
5. Place the binary files and resource files of libcef (libcef.dll, etc.) in the following directories:    
* The x64 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\x64` directory
* The Win32 version goes to the `${NIM_DUILIB_ROOT}\bin\libcef_win_109\Win32` directory

## V. Usage Notes for the CEF Control (CefControl) (Linux Platform)
The basic usage steps are as follows (all directories only write the subdirectories relative to the nim_duilib root directory ${NIM_DUILIB_ROOT}; the actual settings can be flexibly adjusted according to your own project organization structure):    
### 1. Download of the Latest Version Binary Files and Resource Files of libcef (You Need to Download Them Yourself)
* Download link for the x64 version:  [11/11/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_linux64.tar.bz2)    
* Download link for the ARM64 version: [11/11/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_linuxarm64.tar.bz2)    
  After downloading the archive, extract it.    
  Then copy the files in the Release directory and the files in the Resources directory to the `${NIM_DUILIB_ROOT}/bin/libcef_linux/` directory (the libcef_linux folder needs to be created).    
### 2. Organization Structure of libcef Binary Files and Resource Files
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
locales (directory, containing language packs such as zh-CN.pak, en-US.pak, etc.)
```
### 3. Content to Be Added to the Program's Makefile or CMakeLists.txt
* In the header file include path, add `duilib/third_party/libcef/libcef_linux`    
* In the library file include path, add `bin/libcef_linux` (this directory contains the dynamic library files of libcef: libcef.so, etc.)    
* Set the link dependencies, add ` libcef.so cef_dll_wrapper X11`    
* Place the binary files and resource files of libcef (libcef.so, etc.) in the following directory `bin/libcef_linux`.    

## VI. Usage Notes for the CEF Control (CefControl) (macOS Platform)
The basic usage steps are as follows (all directories only write the subdirectories relative to the nim_duilib root directory ${NIM_DUILIB_ROOT}; the actual settings can be flexibly adjusted according to your own project organization structure):    
### 1. Download of the Latest Version Binary Files and Resource Files of libcef (You Need to Download Them Yourself)
* Download link for the x64 version:  [11/12/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_macosx64.tar.bz2)    
* Download link for the ARM64 version: [11/12/2025 - 142.0.10+g29548e2+chromium-142.0.7444.135 / Chromium 142.0.7444.135](https://cef-builds.spotifycdn.com/cef_binary_142.0.10%2Bg29548e2%2Bchromium-142.0.7444.135_macosarm64.tar.bz2)    
  After downloading the archive, extract it.    
  Then copy the entire contents of the directory to the `${NIM_DUILIB_ROOT}/../cef_binary` directory (the cef_binary folder needs to be created, and is at the same level as the nim_duilib directory).    
### 2. Content List Inside cef_binary
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
After compilation, the cmake script will copy the dependent files to the target `*.app/Contents/Frameworks/Chromium Embedded Framework.framework/` path.    
