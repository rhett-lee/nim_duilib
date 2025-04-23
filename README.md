# nim duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) 是一个界面库，包含了一整套桌面软件的开发部件，使用C++语言开发。nim_duilib是在[NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework)库的基础上进行了代码功能完善和扩展，但由于代码结构和资源XML文件的格式均有调整，所以不能直接兼容基于NIM_Duilib_Framework开发的程序，但代码和资源XML迁移的难度不大。您可以直接使用nim_duilib界面库来开发常用的桌面应用，以简化应用程序的UI开发工作。

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## 特色

 - 通用样式：支持以XML格式描述应用程序的窗口属性、布局属性、控件属性等，方便调整界面元素的位置和大小，较为灵活
 - 控件丰富：包含各种常见的窗口和控件，如图片控件、动画控件、按钮、文本框、列表控件、虚表控件、树控件、颜色选择控件、菜单等
 - 事件驱动：基于消息机制的事件处理，使得UI交互逻辑清晰
 - 皮肤支持：通过XML文件定义皮肤结构，可以轻松改变界面风格
 - 性能优异：界面资源的内存占有率低，使用Skia引擎绘制，后台绘制配置使用CPU绘制或者GPU绘制
 - 多种图片格式，支持的图片文件格式有：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
 - 动画图片，支持GIF动画文件、APNG动画文件、WebP动画文件
 - 使用Skia作为界面渲染引擎，性能较好，功能丰富，控件的功能扩展较容易
 - 支持DPI感知（Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2四种模式），支持独立设置DPI，支持高清DPI的适配
 - 多国语言支持，支持动态多种语言切换，易于实现国际化
 - CEF 控件支持（支持libcef 109 版本，以兼容Win7系统；支持libcef 133 版本，支持Win10及以上操作系统）
 - 支持SDL3：可使用SDL3作为窗口管理和输入输出等基本功能提供者，从而支持跨平台（目前已经适配了Linux平台）

## 目录
| 目录          | 说明 |
| :---          | :--- |
| duilib        | 项目的核心代码|
| docs          | 项目的说明文档|
| bin           | 各个示例程序输出目录，包含预设的皮肤和语言文件以及 CEF 依赖|
| manifest      | 应用程序清单文件（仅Windows平台）|
| licenses      | 引用的其他开源代码对应的licenses文件|
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
        <td rowspan="16">功能完善</td>
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
    <tr>
        <td rowspan="15">新增控件</td>
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
|directui  |                       | 借鉴界面库框架|[directui.LICENSE.txt](licenses/directui.LICENSE.txt)|
|duilib    |                       | 最早基于duilib开发|[duilib.LICENSE.txt](licenses/duilib.LICENSE.txt)|
|NIM_Duilib Framework|             | 基于NIM_Duilib_Framework开发|[NIM_Duilib_Framework.LICENSE.txt](licenses/NIM_Duilib_Framework.LICENSE.txt)|
|libcef    |third_party/libcef_win<br>third_party/libcef_win_109<br>third_party/libcef_linux| 用于加载CEF模块|[libcef.LICENSE.txt](licenses/libcef.LICENSE.txt)|
|udis86    |third_party/libudis86| 反汇编计算完整性指令最短长度|[udis86.LICENSE.txt](licenses/udis86.LICENSE.txt)|

备注：    
1. 本项目最早是基于duilib开发的，项目地址：[duilib](https://github.com/duilib/duilib)
2. 本项目是直接在NIM_Duilib_Framework项目的基础上继续开发的，项目地址：[NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)

## 获取代码和编译（Windows平台）
1. 设置工作目录：`D:\develop\skia\`    
2. 获取并编译Skia库（nim_duilib内部使用Skia作为界面绘制引擎，所以先要编译skia）：    
（1）注意事项：skia源码应该与nim_duilib源码位于相同的目录下，目录结构：`D:\develop\skia\`  
（2）获取skia代码的编译方法和修改的代码：`git clone https://github.com/rhett-lee/skia_compile`    
（3）编译skia源码：按照skia_compile目录中的[Windows下编译skia.md文档](https://github.com/rhett-lee/skia_compile/blob/main/Windows%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)中的方法，编译出skia相关的.lib文件    
　　　注意事项：skia源码编译的时候，应使用LLVM编译，程序运行比较流畅；如果使用VS编译，运行速度很慢，界面比较卡    
　　　检查方法：编译成功以后，在`skia/out`的子目录下，有生成`skia.lib`等lib文件    
3. 获取项目代码：`git clone https://github.com/rhett-lee/nim_duilib`
4. 编译nim_duilib：进入 `nim_duilib/examples` 目录，使用 Visual Studio 2022版本的 IDE 打开 `examples.sln`，选择编译选项为Debug|x64或者Release|x64，按下 F7 即可编译所有示例程序（编译完成的示例程序位于bin目录中）
5. 编译附件说明：    
（1）项目中工程的编译环境为Visual Studio 2022版本，如果使用其他版本的Visual Studio编译器，需要手动更新编译工程的属性。    
（2）项目中的工程默认配置是x64的，如果需要编译Win32的程序，在编译skia的时候，也需要启动32位的命令行（x86 Native Tools Command Prompt for VS 2022）   
（3）nim_duilib的代码兼容性默认是支持Win7以上系统，未支持Windows XP；Windows SDK的兼容性配置可在[duilib\duilib_config.h](duilib/duilib_config.h)文件中修改。     

## 获取代码和编译（Linux平台）
1. 设置工作目录：`/home/develop/`    
2. 获取并编译Skia库（nim_duilib内部使用Skia作为界面绘制引擎，所以先要编译skia）：    
（1）注意事项：skia源码应该与nim_duilib源码位于相同的目录下，目录结构：`/home/develop/skia/`  
（2）获取skia代码的编译方法和修改的代码：`git clone https://github.com/rhett-lee/skia_compile`    
（3）编译skia源码：按照skia_compile目录中的说明文档中的方法，编译出skia相关的.a文件    
　　　注意事项：skia源码编译的时候，应使用LLVM编译，程序运行比较流畅    
　　　检查方法：编译成功以后，在`skia/out`的子目录下，有生成`libskia.a`等.a文件    

| 操作系统平台            |参考文档    |  备注 |
| :---                    | :---       |:---   |
|OpenEuler                |[OpenEuler下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenEuler%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|     |
|OpenKylin（开放麒麟）    |[OpenKylin下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|     |
|UbuntuKylin（优麒麟）    |[UbuntuKylin下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/UbuntuKylin%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |     |
|中科方德                 |[中科方德下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/%E4%B8%AD%E7%A7%91%E6%96%B9%E5%BE%B7%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |     |
|统信UOS                  |[统信UOS下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/%E7%BB%9F%E4%BF%A1UOS%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)|python/gcc/g++/llvm/clang/clang++/gn等开发工具链，全部需要从源码编译安装|
|Ubuntu                   |[Ubuntu下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Ubuntu%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |    |
|Debian                   |[Debian下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Debian%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |     |
|Fedora                   |[Fedora下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/Fedora%E4%B8%8B%E7%BC%96%E8%AF%91skia.md)  |     |
|OpenSuse                 |[OpenSuse下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/OpenSuse%E4%B8%8B%E7%BC%96%E8%AF%91skia.md) |     |

3. 获取并编译SDL库（nim_duilib内部使用SDL作为Linux平台的界面绘制引擎，所以先要编译SDL）：  
（0）准备工作    

| 操作系统平台            |桌面类型        |需要安装的模块及安装命令 | 
| :---                    | :---           | :---                    |
|OpenEuler                |UKUI/DDE（X11） |`sudo dnf install libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel`| 
|OpenKylin（开放麒麟）    | Wayland        |`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|UbuntuKylin（优麒麟）    | X11            |`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|中科方德                 | X11            |`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|统信UOS                  | X11            |`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|Ubuntu                   |GNOME（Wayland）|`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Debian                   |GNOME（Wayland）|`sudo apt install libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Fedora                   |GNOME（Wayland）|`sudo dnf install libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel`|
|OpenSuse                 |KDE（X11）      |`sudo zypper install libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel`|

　（1）进入工作目录：`cd /home/develop/`    
　（2）获取源码：`git clone https://github.com/libsdl-org/SDL.git`   
　（3）使用cmake设置：`cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="/home/develop/SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF`    
　（4）进入SDL编译目录：`cd /home/develop/SDL.build`    
　（5）编译SDL：`make`    
　（6）安装SDL：`make install`    
　（7）SDL3的安装目录为：`/home/develop/SDL3/`，子目录里面包含了.a文件和.h文件等；    
　（8）编译完成后，`SDL`目录和`SDL.build`可以删除，仅保留`SDL3`目录即可。    
4. 获取项目代码并编译：    
　（1）回到工作目录：`cd /home/develop/`    
　（2）获取项目代码：`git clone https://github.com/rhett-lee/nim_duilib`    
　（3）进入代码目录：`cd /home/develop/nim_duilib/`    
　（4）设置编译脚本可执行：`chmod +x linux_build.sh`    
　（5）执行脚本编译：`./linux_build.sh`    
　（6）编译完成后，编译成功后，在libs目录生成了.a文件（libz.a、libpng.a、libwebp.a、libcximage.a、libduilib.a），在bin目录中生成了可执行文件。    
5. 备注：如果希望编译debug版本的.a文件，可在cmake的参数中追加：` -DCMAKE_BUILD_TYPE=Debug`    
6. 如果想自己编译最新版的gcc/g++（比如系统自带的gcc/g++版本较低时，由于不支持C++20，所以无法成功编译nim_duilib源码）    
　　可以参照如下流程编译（以gcc-14.2.0为例）：    
　（1）创建源码目录：`cd /home/develop; mkdir src; cd src`    
　（2）进入工作目录：`/home/develop/src`    
　（3）下载：`wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz`    
　（4）解压：`tar -xzf gcc-14.2.0.tar.gz`    
　（5）下载gcc依赖的第三方库：进入gcc-14.2.0目录，然后下载依赖的第三方库    
　　`cd /home/develop/src/gcc-14.2.0`    
　　`./contrib/download_prerequisites`    
　（6）配置：    
　　`mkdir -p /home/develop/src/gcc-14.2.0.build`    
　　`cd /home/develop/src/gcc-14.2.0.build`    
　　`../gcc-14.2.0/configure --prefix=/home/develop/install/gcc-14.2.0 --disable-multilib --enable-ld --enable-bootstrap`    
　（7）编译：`make -j 4` 多进程编译，编译参数可参考电脑实际有几个核心。    
　（8）安装：`make install`    
　（9）设置环境变量，以使新版gcc/g++可用：    
　　`export PATH=/home/develop/install/gcc-14.2.0/bin/:$PATH`    
　　`export LD_LIBRARY_PATH=/home/develop/install/gcc-14.2.0/lib64/:$LD_LIBRARY_PATH`    
　　`export C_INCLUDE_PATH=/home/develop/install/gcc-14.2.0/include/c++/14.2.0/:/home/develop/install/gcc-14.2.0/include/c++/14.2.0/x86_64-pc-linux-gnu/:$C_INCLUDE_PATH`    
　　`export CPLUS_INCLUDE_PATH=/home/develop/install/gcc-14.2.0/include/c++/14.2.0/:/home/develop/install/gcc-14.2.0/include/c++/14.2.0/x86_64-pc-linux-gnu/:$CPLUS_INCLUDE_PATH`    
　（10）进入项目目录并编译源码：  
　　`cd /home/develop/nim_duilib`    
　　`chmod +x linux_build.sh`    
　　`./linux_build.sh`    

## 开发计划
 - 跨平台（Windows/Linux系统）的窗口引擎（基于[SDL3.0](https://www.libsdl.org/)）不断测试与完善（X11和Wayland）
 - 动画功能的加强
 - 不断测试发现缺陷并修复，不断完善代码
 - 其他待补充

## 参考文档

 - [快速上手](docs/Getting-Started.md)
 - [参考文档](docs/Summary.md)
 - [示例程序](docs/Examples.md)
 - 跨平台（Windows/Linux系统）的窗口引擎（基于[SDL3.0](https://www.libsdl.org/)），相关的进展参见文档：[跨平台开发相关事项.md](跨平台开发相关事项.md)