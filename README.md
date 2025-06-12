# nim duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) 是一个界面库，包含了一整套桌面软件的开发部件，使用C++语言开发。nim duilib是在NIM_Duilib_Framework库的基础上进行了代码功能完善和扩展，但由于代码结构和资源XML文件的格式均有调整，所以不能直接兼容基于NIM_Duilib_Framework开发的程序，但代码和资源XML迁移的难度不大。您可以直接使用nim_duilib界面库来开发常用的桌面应用，以简化应用程序的UI开发工作。

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## 特色

 - 通用样式：支持以XML格式描述应用程序的窗口属性、布局属性、控件属性等，方便调整界面元素的位置和大小，较为灵活
 - 控件丰富：包含各种常见的窗口和控件，如图片控件、动画控件、按钮、文本框、列表控件、虚表控件、树控件、颜色选择控件、菜单等
 - 事件驱动：基于消息机制的事件处理，使得UI交互逻辑清晰
 - 皮肤支持：通过XML文件定义皮肤结构，可以轻松改变界面风格
 - 性能优异：界面资源的内存占有率低，使用Skia引擎绘制，后台绘制配置使用CPU绘制或者GPU绘制
 - 多种图片格式：支持的图片文件格式有：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
 - 支持动画图片：支持GIF动画文件、APNG动画文件、WebP动画文件
 - 窗口阴影：支持窗口的圆角阴影、直角阴影，并可选择阴影大小，可实时更新
 - Skia引擎：使用Skia作为界面渲染引擎，性能较好，功能丰富，控件的功能扩展较容易
 - 支持DPI感知：有Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2四种模式，支持独立设置DPI，支持高清DPI的适配
 - 支持多国语言：支持动态多种语言切换，易于实现国际化
 - 支持CEF控件：支持libcef 109 版本，以兼容Win7系统；支持libcef 133 版本，支持Win10及以上操作系统
 - 支持SDL3：可使用SDL3作为窗口管理和输入输出等基本功能提供者，从而支持跨平台（目前已经适配了Linux平台）

## 目录结构
| 目录          | 说明 |
| :---          | :--- |
| duilib        | 项目的核心代码|
| docs          | 项目的说明文档|
| bin           | 各个示例程序输出目录，包含预设的皮肤和语言文件以及 CEF 依赖|
| licenses      | 引用的其他开源代码对应的licenses文件|
| cmake         | cmake编译时依赖的公共设置|
| build         | 各个平台的编译脚本和编译工程（包括VC编译工程）|
| msvc          | Windows平台的应用程序清单文件和VC工程公共配置|
| examples      | 项目的示例程序源代码|
| duilib/third_party| 项目代码依赖的第三方库，详细内容见后续文档|

## 基于NIM_Duilib_Framework源码做的主要修改
<table>
    <tr>
        <th>分类</th>
        <th>修改内容</th>
    </tr>
    <tr>
        <td rowspan="10">整体改进</td>
        <td align="left">1. 调整了代码的组织结构，按照功能模块划分，大文件按类拆分为多个小文件，有利于理解整个库的体系结构</td>
    </tr>
    <tr><td align="left">2. 梳理了代码的接口文件，补充各个接口的注释和功能注释，有利于阅读和理解代码</td></tr>
    <tr><td align="left">3. 对配置XML文件进行优化，调整了属性命名规则，控件的宽度和高度可以按百分比设置，图片属性增加了部分扩展，优化了图片加载流程</td></tr>
    <tr><td align="left">4. 扩展了图片资源的支持：新增加APNG/WEBP动画，新增对ICO的支持，并优化了图片加载引擎及代码实现逻辑</td></tr>
    <tr><td align="left">5. 对布局进行了梳理，归类为：自由布局（浮动布局）、水平布局、垂直布局、水平瓦片布局、垂直瓦片布局、虚表水平布局、虚表垂直布局、虚表水平瓦片布局、虚表垂直瓦片布局，使布局的概念更易理解，易于扩展</td></tr>
    <tr><td align="left">6. XML文件解析引擎替换为pugixml parser，性能更优</td></tr>
    <tr><td align="left">7. 移除对base库的依赖，消息循环和线程通信相关功能改为自己实现</td></tr>
    <tr><td align="left">8. 集成了Skia引擎，并作为默认的渲染引擎</td></tr>
    <tr><td align="left">9. 支持SDL3，支持跨平台（已经适配了Windows平台、Linux平台）</td></tr>
    <tr><td align="left">10. CEF组件放到duilib工程，并对CEF的版本进行了升级（支持libcef 109 版本，以兼容Win7系统；支持libcef 133 版本，支持Win10及以上操作系统）</td></tr>
    <tr>
        <td rowspan="17">功能完善</td>
        <td align="left">1. 对窗口类（Window）增加了新的属性：的功能进行了完善，提高对DPI自适应、窗口消息的代码容错，代码结构做了调整</td>
    </tr>
    <tr><td align="left">2. 对窗口类（Window）增加了新的属性：use_system_caption，snap_layout_menu，sys_menu，sys_menu_rect, icon属性，提供使用操作系统默认标题栏的选项，自绘标题栏的功能与系统标题栏的功能相似</td></tr>
    <tr><td align="left">3. 对窗口（Window）引入WindowDropTarget辅助类，提供对基于窗口的拖放功能的支持</td></tr>
    <tr><td align="left">4. 对资源管理相关的部分做了梳理，使得字体、颜色、图片等资源管理更容易理解</td></tr>
    <tr><td align="left">5. 对ListBox控件进行了优化：细分为ListBox、VListBox、HListBox、VTileListBox、HTileListBox、VirtualListBox、VirtualVListBox、VirtualHListBox、VirtualVTileListBox、VirtualHTileListBox几种，功能更加完善，基于虚表的ListBox功能可用性提升</td></tr>
    <tr><td align="left">6. 对组合框（Combo）关联的控件（CheckCombo、FilterCombo）进行了优化，提升可用性</td></tr>
    <tr><td align="left">7. 对日期时间（DateTime）控件的编辑功能进行了完善</td></tr>
    <tr><td align="left">8. 对菜单（CMenuWnd）类的功能代码进行优化，按新的结构重新实现菜单，使得菜单内的控件完全兼容现有容器/控件体系，易于理解和维护</td></tr>
    <tr><td align="left">9. 对富文本编辑控件（RichEdit）进行功能优化和扩展，丰富了较多常用功能</td></tr>
    <tr><td align="left">10. 对树控件（TreeView）进行功能优化和扩展，丰富了较多常用功能，提升可用性</td></tr>
    <tr><td align="left">11. 全局资源（GlobalManager）的接口进行了优化，使得所有资源全部通过此接口管理，易于理解和维护</td></tr>
    <tr><td align="left">12. 同一个窗口内的不同容器之间，容器内的控件可以通过属性设置支持拖出和拖入操作</td></tr>
    <tr><td align="left">13. 控件的背景色支持渐变色</td></tr>
    <tr><td align="left">14. 完善了多国语言的功能，能够更好的支持多语言动态切换，并提供示例程序examples/MultiLang</td></tr>
    <tr><td align="left">15. 完善了DPI感知功能，支持Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2四种模式，支持独立设置DPI，支持高清DPI的适配，提供了示例程序examples/DpiAware</td></tr>
    <tr><td align="left">16. 移除了ui_components工程，CEF组件代码重新梳理，继承到duilib工程中，其他内容删除</td></tr>
    <tr><td align="left">17. 优化窗口的阴影功能，窗口的阴影使用svg图片，增加了阴影类型属性（shadow_type），可选值为：<br> "default", 默认阴影 <br> "big", 大阴影，直角（适合普通窗口）<br> "big_round", 大阴影，圆角（适合普通窗口）<br> "small", 小阴影，直角（适合普通窗口）<br> "small_round", 小阴影，圆角（适合普通窗口）<br> "menu", 小阴影，直角（适合弹出式窗口，比如菜单等）<br> "menu_round", 小阴影，圆角（适合弹出式窗口，比如菜单等）<br> "none", 无阴影</td></tr>
    <tr>
        <td rowspan="16">新增控件</td>
        <td align="left">1. GroupBox：分组容器</td>
    </tr>
    <tr><td align="left">2. HotKey：热键控件</td></tr>
    <tr><td align="left">3. HyperLink：带有超级链接的文字</td></tr>
    <tr><td align="left">4. IPAddress：IP地址控件</td></tr>
    <tr><td align="left">5. Line：画线控件</td></tr>
    <tr><td align="left">6. RichText：格式化文本（类HTML格式）</td></tr>
    <tr><td align="left">7. Split: 分割条控件/容器</td></tr>
    <tr><td align="left">8. TabCtrl：多标签控件（类似浏览器的多标签）</td></tr>
    <tr><td align="left">9. ListCtrl：列表控件（Report/Icon/List三个视图）</td></tr>
    <tr><td align="left">10. PropertyGrid: 属性表控件，支持文本、数字、复选框、字体、颜色、日期、IP地址、热键、文件路径、文件夹等属性</td></tr>
    <tr><td align="left">11. ColorPicker：拾色器，独立窗口，其中的子控件，可以单独作为颜色控件来使用</td></tr>
    <tr><td align="left">12. ComboButton：带有下拉组合框的按钮</td></tr>
    <tr><td align="left">13. ShadowWnd：是WinImplBase的子类，使用附加阴影窗口实现的基类，实现了创建窗口并附加的功能，提供没有 kWS_EX_LAYERED 属性的窗口阴影</td></tr>
    <tr><td align="left">14. ControlDragableT：支持相同Box内的子控件通过拖动来调整顺序</td></tr>
    <tr><td align="left">15. DirectoryTree：目录树控件，用于显示文件系统中的目录</td></tr>
    <tr><td align="left">16. AddressBar：地址栏控件，用于显示本地文件系统的路径</td></tr>
    <tr>
        <td rowspan="3">性能优化</td>
        <td align="left">1. 优化了Control及子控件的内存占用，在界面元素较多的时候，内存占有率有大幅降低</td>
    </tr>
    <tr><td align="left">2. 优化了动画绘制流程，合并定时器的触发事件，避免播放控件动画或者播放动画图片的过程中导致界面很卡的现象</td></tr>
    <tr><td align="left">3. 基于虚表的ListBox控件及关联控件：通过优化实现机制，使得可用性和性能有较大改善</td></tr>
    <tr>
        <td rowspan="8">示例程序完善</td>
        <td align="left">1. examples/ColorPicker: 新增加了拾色器示例程序</td>
    </tr>
    <tr><td align="left">2. examples/ListCtrl：新增加了列表的示例程序，演示列表的个性功能</td></tr>
    <tr><td align="left">3. examples/render：新增加了渲染引擎示例程序，演示了大部分容器、控件、资源管理的功能</td></tr>
    <tr><td align="left">4. examples/TreeView：新增加了树控件的示例程序，演示了树控件的各种功能</td></tr>
    <tr><td align="left">5. examples/RichEdit：新增加了富文本编辑控件的示例程序，演示了富文本编辑控件的各种功能</td></tr>
    <tr><td align="left">6. examples/MultiLang：提供多语言的动态切换功能演示</td></tr>
    <tr><td align="left">7. examples/DpiAware：提供了DPI感知功能的功能演示</td></tr>
    <tr><td align="left">8. 其他的示例程序：大部分进行了代码兼容性修改和优化，使得示例程序也可以当作测试程序使用</td></tr>
    <tr>
        <td rowspan="8">完善文档</td>
        <td align="left">1. README.md和docs子目录的文档重新进行了梳理，使得阅读者更容易理解界面库的功能、用法，更易上手</td>
    </tr>
    <tr><td align="left">2. 各个控件的接口没有单独整理成文档，因为可以直接阅读接口文件中的注释来达到目的，目前各个接口的注释是比较完善的</td></tr>
    <tr><td align="left">3. 各个平台的编译文档和依赖的编译脚本</td></tr>
</table>

## 使用的第三方库说明
| 名称     |代码子目录| 用途 | license文件|
| :---     | :---     |:---  |:---        |
|apng      |third_party/libpng| 支持APNG图片格式|zlib/libpng License|
|libpng    |third_party/libpng| 支持PNG图片格式|[libpng.LICENSE.txt](licenses/libpng.LICENSE.txt)|
|zlib      |third_party/zlib| 支持PNG/APNG图片格式、Zip文件解压|[zlib.LICENSE.txt](licenses/zlib.LICENSE.txt)|
|cximage   |third_party/cximage| 支持GIF/ICO图片格式|[cximage.LICENSE.txt](licenses/cximage.LICENSE.txt)|
|libwebp   |third_party/libwebp| 支持WebP图片格式|[libWebP.LICENSE.txt](licenses/libwebp.LICENSE.txt)|
|stb_image |third_party/stb_image| 支持JPEG/BMP图片格式，调整图片大小|[stb_image.LICENSE.txt](licenses/stb_image.LICENSE.txt)|
|nanosvg   |third_party/svg| 支持SVG图片格式|[nanosvg.LICENSE.txt](licenses/nanosvg.LICENSE.txt)|
|pugixml   |third_party/xml| 支持资源描述XML的解析|[pugixml.LICENSE.txt](licenses/pugixml.LICENSE.txt)|
|ConvertUTF|third_party/convert_utf| 用于UTF-8/UTF-16编码的相互转换|[llvm.LICENSE.txt](licenses/llvm.LICENSE.txt)|
|skia      |项目未包含skia源码     | 界面库渲染引擎|[skia.LICENSE.txt](licenses/skia.LICENSE.txt)|
|SDL       |项目未包含SDL源码      | 跨平台窗口管理|[SDL.LICENSE.txt](licenses/SDL.LICENSE.txt)|
|duilib    |                       | 最早基于duilib开发|[duilib.LICENSE.txt](licenses/duilib.LICENSE.txt)|
|NIM_Duilib Framework|             | 基于NIM_Duilib_Framework开发|[NIM_Duilib_Framework.LICENSE.txt](licenses/NIM_Duilib_Framework.LICENSE.txt)|
|libcef    |third_party/libcef_win<br>third_party/libcef_win_109<br>third_party/libcef_linux| 用于加载CEF模块|[libcef.LICENSE.txt](licenses/libcef.LICENSE.txt)|
|udis86    |third_party/libudis86| 反汇编计算完整性指令最短长度|[udis86.LICENSE.txt](licenses/udis86.LICENSE.txt)|

## 界面效果预览
使用该界面库编写的示例程序，该文档可以见到各个控件的展示效果：[docs/Examples.md](docs/Examples.md) 

## 编程语言
- C/C++: 编译器需要支持C++20

## 支持的操作系统
- Windows：Windows 7.0 版本及以上
- Linux：OpenEuler、OpenKylin（开放麒麟）、UbuntuKylin（优麒麟）、中科方德、统信UOS、Ubuntu、Debian、Fedora、OpenSuse等

## 支持的编译器
- Visual Studio 2022（Windows）
- LLVM（Windows）
- MinGW-W64：gcc/g++、clang/clang++（Windows）
- gcc/g++（Linux）
- clang/clang++（Linux）
- clang/clang++（macOS）

## 获取代码和编译（Windows平台）
### 一、准备工作：安装必备的软件
1. 安装python3（python的主版本需要是3，需要添加到Path环境变量）    
（1）首先安装python3    
（2）在Windows的设置里面，关闭python.exe和python3.exe的"应用执行别名"，否则编译skia的脚本执行有问题。Windows设置入口：设置 -> 应用 -> 高级应用设置 -> 应用执行别名    
（3）到python.exe所在目录中，复制一份python.exe，改名为python3.exe: 确保命令行参数中可以访问到python3.exe   
（4）在命令行验证：`> python3.exe --version` 可以查看python的版本号     
2. 安装Git For Windows: 2.44版本，git需要添加到Path环境变量，确保命令行参数中可以访问到git.exe    
3. 安装Visual Studio 2022社区版    
4. 安装LLVM：20.1.0 Win64 版本    
（1）安装目录：`C:\LLVM`    
（2）注意事项：如果安装在其他目录，安装目录中不能有空格，否则编译会遇到问题。

### 二、一键编译（推荐）
选定一个工作目录，创建一个脚本`build.bat`，将下面已经整理好脚本复制进去，保存文件。    
脚本文件内容如下：    
```
echo OFF
set retry_delay=10

:retry_clone_duilib
if not exist ".\nim_duilib" (
    git clone https://github.com/rhett-lee/nim_duilib
) else (  
    git -C ./nim_duilib pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_duilib
)
.\nim_duilib\build\build_duilib_all_in_one.bat
```
进入命令行控制台，运行该脚本： 
```
.\build.bat
```
编译完成的示例程序位于bin目录中。

### 三、手动编译过程（Windows平台）
1. 设置工作目录：`D:\develop`    
2. 获取相关代码    
（1）`git clone https://github.com/rhett-lee/nim_duilib`      
（2）`git clone https://github.com/rhett-lee/skia_compile`    
（3）`git clone https://github.com/google/skia.git`  
2. 编译Skia源码    
（1）nim_duilib内部使用Skia作为界面绘制引擎，所以先要编译skia，优先用LLVM编译，运行流畅    
（2）按照skia_compile目录中的[Windows下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Windows%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)文档中的方法，编译出skia相关的.lib文件      
3. 编译nim_duilib：进入 `build` 目录，打开 `examples.sln`，可执行编译，编译完成的示例程序位于bin目录中。

## 获取代码和编译（Linux平台）
### 一、准备工作：安装必备的软件
不同的操作系统平台，可以按照以下列表安装必备的软件。
| 操作系统平台            |桌面类型        |需要安装的模块及安装命令（必选） | 
| :---                    | :---           | :---                    |
|OpenEuler                |UKUI/DDE（X11） |`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel`| 
|OpenKylin（开放麒麟）    | Wayland        |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|UbuntuKylin（优麒麟）    | X11            |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-devlibxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|中科方德                 | X11            |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|统信UOS                  | X11            |`sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|Ubuntu                   |GNOME（Wayland）|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Debian                   |GNOME（Wayland）|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Fedora                   |GNOME（Wayland）|`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel`|
|OpenSuse                 |KDE（X11）      |`sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel`|

### 二、一键编译（推荐）
选定一个工作目录，创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
然后在控制台，为脚本文件添加可执行权限，最后运行该脚本： 
```
chmod +x build.sh
./build.sh
```

脚本文件内容如下：    
```
#!/bin/bash

# Retry clone nim_duilib
while true; do
    if [ ! -d "./nim_duilib/.git" ]; then
        git clone https://github.com/rhett-lee/nim_duilib
    else
        git -C ./nim_duilib pull
    fi
    if [ $? -ne 0 ]; then
        sleep 10
        continue
    fi
    break
done

chmod +x ./nim_duilib/build/build_duilib_all_in_one.sh
./nim_duilib/build/build_duilib_all_in_one.sh
```
编译完成的示例程序位于bin目录中。    
备注：UOS系统，需要先安装所需的开发环境，然后再安装，可参考文档：[统信UOS下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/%E7%BB%9F%E4%BF%A1UOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)。

### 三、手动编译过程（Linux平台）
1. 设置工作目录：`~/develop`    
2. 获取相关代码    
（1）`git clone https://github.com/rhett-lee/nim_duilib`      
（2）`git clone https://github.com/rhett-lee/skia_compile`    
（3）`git clone https://github.com/google/skia.git`  
（4）`git clone https://github.com/libsdl-org/SDL.git`    
3. 编译Skia库    

| 操作系统平台            |参考文档（网络链接）    |  参考文档（本地文件） |
| :---                    | :---       |:---   |
|OpenEuler                |[OpenEuler下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenEuler%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|[OpenEuler下编译skia.md](../skia_compile/OpenEuler%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|
|OpenKylin（开放麒麟）    |[OpenKylin下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|[OpenKylin下编译skia.md](../skia_compile/OpenKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|
|UbuntuKylin（优麒麟）    |[UbuntuKylin下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/UbuntuKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |[UbuntuKylin下编译skia.md](../skia_compile/UbuntuKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|
|中科方德                 |[中科方德下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/%E4%B8%AD%E7%A7%91%E6%96%B9%E5%BE%B7%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |[中科方德下编译skia.md](../skia_compile/%E4%B8%AD%E7%A7%91%E6%96%B9%E5%BE%B7%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |
|统信UOS                  |[统信UOS下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/%E7%BB%9F%E4%BF%A1UOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|[统信UOS下编译skia.md](../skia_compile/%E7%BB%9F%E4%BF%A1UOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|
|Ubuntu                   |[Ubuntu下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Ubuntu%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) | [Ubuntu下编译skia.md](../skia_compile/Ubuntu%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |
|Debian                   |[Debian下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Debian%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |[Debian下编译skia.md](../skia_compile/Debian%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |
|Fedora                   |[Fedora下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Fedora%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |[Fedora下编译skia.md](../skia_compile/Fedora%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|
|OpenSuse                 |[OpenSuse下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenSuse%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) | [OpenSuse下编译skia.md](../skia_compile/OpenSuse%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |

    注意事项：skia源码编译的时候，应使用LLVM编译，程序运行比较流畅。
4. 编译SDL库 
```
#!/bin/bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. 编译nim_duilib
```
#!/bin/bash
cd ~/develop/nim_duilib/
chmod +x linux_build.sh
./linux_build.sh
```
编译完成后，在bin目录中生成了可执行文件。    
如果希望支持CEF，可以参考相关文档[docs/CEF.md](docs/CEF.md)。

## 获取代码和编译（macOS平台）
### 一、准备工作：安装必备的软件
安装完成系统后，需要做的工作：    
#### 安装Xcode命令行工具
```
xcode-select --install
```
验证安装：
```
clang++ --version
```
#### 安装Homebrew
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
如果失败，可以查找其他源来安装。    
更新Homebrew：    
```
brew update
```
#### 系统已经自带，无需安装的软件
`git make unzip python3`

#### 安装cmake
```
brew install cmake
```
#### 安装ninja
```
brew install ninja
```
#### 安装gn（需要从源码编译gn）
```
mkdir ~/develop
cd ~/develop
git clone https://github.com/timniederhausen/gn
cd gn
python3 build/gen.py
ninja -C out
sudo cp out/gn /usr/local/bin/
gn --version
```

### 二、一键编译（推荐）
选定一个工作目录，创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
然后在控制台，为脚本文件添加可执行权限，最后运行该脚本： 
```
chmod +x build.sh
./build.sh
```

脚本文件内容如下：    
```
#!/bin/bash

# Retry clone nim_duilib
while true; do
    if [ ! -d "./nim_duilib/.git" ]; then
        git clone https://github.com/rhett-lee/nim_duilib
    else
        git -C ./nim_duilib pull
    fi
    if [ $? -ne 0 ]; then
        sleep 10
        continue
    fi
    break
done

chmod +x ./nim_duilib/build/build_duilib_all_in_one.sh
./nim_duilib/build/build_duilib_all_in_one.sh
```
编译完成的示例程序位于bin目录中。    

### 三、手动编译过程（macOS平台）
1. 设置工作目录：`~/develop`    
2. 获取相关代码    
（1）`git clone https://github.com/rhett-lee/nim_duilib`      
（2）`git clone https://github.com/rhett-lee/skia_compile`    
（3）`git clone https://github.com/google/skia.git`  
（4）`git clone https://github.com/libsdl-org/SDL.git`  
3. 编译Skia库    

| 操作系统平台 |参考文档（网络链接） |  参考文档（本地文件） |
| :---         | :---                |:---                   |
|macOS         |[macOS下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/macOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) | [macOS下编译skia.md](../skia_compile/macOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |

    注意事项：skia源码编译的时候，应使用LLVM编译，程序运行比较流畅。
4. 编译SDL库 
```
#!/bin/bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. 编译nim_duilib
```
#!/bin/bash
cd ~/develop/nim_duilib/
chmod +x macos_build.sh
./macos_build.sh
```
编译完成后，在bin目录中生成了可执行文件。    
如果希望支持CEF，可以参考相关文档[docs/CEF.md](docs/CEF.md)。

## 获取代码和编译（FreeBSD平台）
### 一、准备工作：安装必备的软件
```
sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2
```
### 二、一键编译（推荐）
选定一个工作目录，创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
然后在控制台，为脚本文件添加可执行权限，最后运行该脚本： 
```
chmod +x build.sh
./build.sh
```

脚本文件内容如下：    
```
#!/usr/bin/env bash

# Retry clone nim_duilib
while true; do
    if [ ! -d "./nim_duilib/.git" ]; then
        git clone https://github.com/rhett-lee/nim_duilib
    else
        git -C ./nim_duilib pull
    fi
    if [ $? -ne 0 ]; then
        sleep 10
        continue
    fi
    break
done

chmod +x ./nim_duilib/build/build_duilib_all_in_one.sh
./nim_duilib/build/build_duilib_all_in_one.sh
```
编译完成的示例程序位于bin目录中。

​​注意：FreeBSD 平台不支持 CEF（Chromium Embedded Framework）。

## 开发计划
 - 跨平台（Windows/Linux/macOS）的窗口引擎（基于[SDL3.0](https://www.libsdl.org/)）不断测试与完善（X11和Wayland）
 - 动画功能的加强
 - 不断测试发现缺陷并修复，不断完善代码
 - 其他待补充

## 参考文档

 - [快速上手](docs/Getting-Started.md)
 - [参考文档](docs/Summary.md)
 - [示例程序](docs/Examples.md)

## 相关链接
1. Skia的编译文档库，点击访问：[skia compile](https://github.com/rhett-lee/skia_compile) ：    
2. 本项目最早是基于duilib开发的，项目地址：[duilib](https://github.com/duilib/duilib)
3. 本项目是直接在NIM_Duilib_Framework项目的基础上继续开发的，项目地址：[NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)
