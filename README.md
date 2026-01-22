# nim duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) 是一款基于C++开发的跨平台界面库，源于经典的 duilib 界面库并进行了深度优化与功能扩展，支持Windows/Linux/macOS/FreeBSD平台，专注于简化桌面应用的高效开发。其设计融合了DirectUI理念，通过XML描述界面布局，实现视觉与逻辑的分离，显著提升开发灵活性与维护性。

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## 核心技术特性

 - XML布局驱动：采用 XML 文件定义界面结构，将界面布局与业务逻辑完全分离。开发者可通过修改 XML 快速调整控件位置、尺寸和样式，无需改动 C++ 核心代码，极大提升开发与迭代效率。
 - 丰富的控件体系：内置按钮、文本框、列表视图、虚表控件、树形控件、滑块、进度条、菜单、颜色选择、属性页、标签页等基础控件，同时支持自定义控件开发，满足多样化界面设计需求。
 - 高效的渲染机制：使用Skia作为渲染引擎，实现控件的无窗口绘制，减少系统资源占用，提升界面刷新速度。支持硬件加速渲染（后端绘制支持使用CPU绘制或者GPU绘制），确保复杂界面流畅运行。
 - 事件驱动：基于消息机制的事件处理，使得UI交互逻辑清晰，支持在XML文件中配置事件响应代码。
 - 多种图片格式：支持SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO图片格式。
 - 支持动画格式：支持GIF、APNG、WEBP、Lottie JSON、PAG动画文件格式。
 - 多语言与国际化：支持动态多种语言切换，便于开发全球化的应用程序。
 - 支持动态换肤：通过XML文件定义皮肤结构，可以轻松改变界面风格，支持动态换肤。
 - 支持窗口阴影：支持窗口的圆角阴影、直角阴影，并可选择阴影大小，可实时更新。
 - 支持DPI感知：有Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2四种模式，支持独立设置DPI，支持高清DPI的适配（仅限Windows平台）。
 - 支持CEF控件：支持libcef 109 版本，以兼容Windows 7系统；支持libcef 142 版本，支持Windows 10及以上操作系统，支持Linux和MacOS平台。
 - 支持WebView2控件：支持使用WebView2控件用于显示网页，其接口封装简单，更易于使用（仅支持Windows平台）。
 - 支持SDL3：可使用SDL3作为窗口管理和输入输出等基本功能提供者，从而支持跨平台（目前已经适配了Windows/Linux/MacOS/FreeBSD平台）。

## 目录结构
| 目录          | 说明 |
| :---          | :--- |
| duilib        | 项目的源代码|
| docs          | 项目的说明文档，包括各个控件的功能介绍与属性列表说明文档|
| bin           | 各个示例程序输出目录，包含预设的皮肤和语言文件以及 CEF 依赖|
| licenses      | 引用的其他开源代码对应的licenses文件|
| cmake         | cmake编译时依赖的公共设置|
| build         | 各个平台的编译脚本和编译工程（包括VC编译工程）|
| msvc          | Windows平台的应用程序清单文件和VC工程公共配置|
| examples      | 项目的示例程序源代码，涵盖所有控件的基本用法示例|
| duilib/third_party| 项目代码依赖的第三方库，详细内容见后续文档|

## 基于NIM_Duilib_Framework源码做的主要修改
<table>
    <tr>
        <th>分类</th>
        <th>修改内容</th>
    </tr>
    <tr>
        <td rowspan="11">整体改进</td>
        <td align="left">1. 调整了代码的组织结构，按照功能模块划分，大文件按类拆分为多个小文件，有利于理解整个库的体系结构</td>
    </tr>
    <tr><td align="left">2. 梳理了代码的接口文件，补充各个接口的注释和功能注释，有利于阅读和理解代码</td></tr>
    <tr><td align="left">3. 对配置XML文件进行优化，调整了属性命名规则，控件的宽度和高度可以按百分比设置，图片属性增加了部分扩展，优化了图片加载流程</td></tr>
    <tr><td align="left">4. 扩展了图片资源的支持：新增加APNG/WEBP动画/Lottie JSON动画/PAG动画，新增对ICO的支持，并优化了图片加载引擎及代码实现逻辑</td></tr>
    <tr><td align="left">5. 重新实现各个布局的代码，并新增了常见的界面布局方案，总体归类为：浮动布局、水平布局、垂直布局、水平流式布局、垂直流式布局、网格布局、水平瓦片布局、垂直瓦片布局、虚表水平布局、虚表垂直布局、虚表水平瓦片布局、虚表垂直瓦片布局，使布局的概念更易理解，易于扩展，详情可参考文档(docs/Box.md)</td></tr>
    <tr><td align="left">6. XML文件解析引擎替换为pugixml parser，性能更优</td></tr>
    <tr><td align="left">7. 移除对base库的依赖，消息循环和线程通信相关功能改为自己实现</td></tr>
    <tr><td align="left">8. 集成了Skia引擎，并作为默认的渲染引擎</td></tr>
    <tr><td align="left">9. 支持SDL3，支持跨平台（已经适配了Windows平台、Linux平台、MacOS平台、FreeBSD平台）</td></tr>
    <tr><td align="left">10. CEF组件放到duilib工程，并对CEF的版本进行了升级（支持libcef 109 版本，以兼容Win7系统；支持libcef 142 版本，支持Win10及以上操作系统）</td></tr>
    <tr><td align="left">11. 重新设计图片管理的接口和加载流程（Image目录），支持多线程加载图片，以更好的扩展其他图片格式支持</td></tr>
    <tr>
        <td rowspan="21">功能完善</td>
        <td align="left">1. 对窗口类（Window）增加了新的属性：的功能进行了完善，提高对DPI自适应、窗口消息的代码容错，代码结构做了调整</td>
    </tr>
    <tr><td align="left">2. 对窗口类（Window）增加了新的属性：use_system_caption，snap_layout_menu，sys_menu，sys_menu_rect, icon属性，提供使用操作系统默认标题栏的选项，自绘标题栏的功能与系统标题栏的功能相似</td></tr>
    <tr><td align="left">3. 对窗口（Window）引入WindowDropTarget辅助类，提供对基于窗口的拖放功能的支持</td></tr>
    <tr><td align="left">4. 对资源管理相关的部分做了梳理，使得字体、颜色、图片等资源管理更容易理解</td></tr>
    <tr><td align="left">5. 对ListBox控件进行了优化：细分为ListBox、VListBox、HListBox、VTileListBox、HTileListBox、VirtualListBox、VirtualVListBox、VirtualHListBox、VirtualVTileListBox、VirtualHTileListBox几种，功能更加完善，基于虚表的ListBox功能可用性提升</td></tr>
    <tr><td align="left">6. 对组合框（Combo）关联的控件（CheckCombo、FilterCombo）进行了优化，提升可用性</td></tr>
    <tr><td align="left">7. 对日期时间（DateTime）控件的编辑功能进行了完善</td></tr>
    <tr><td align="left">8. 对菜单（CMenuWnd）类的功能代码进行优化，按新的结构重新实现菜单，使得菜单内的控件完全兼容现有容器/控件体系，易于理解和维护</td></tr>
    <tr><td align="left">9. 对文本编辑控件（RichEdit）进行功能优化和扩展，丰富了较多常用功能</td></tr>
    <tr><td align="left">10. 对树控件（TreeView）进行功能优化和扩展，丰富了较多常用功能，提升可用性</td></tr>
    <tr><td align="left">11. 全局资源（GlobalManager）的接口进行了优化，使得所有资源全部通过此接口管理，易于理解和维护</td></tr>
    <tr><td align="left">12. 同一个窗口内的不同容器之间，容器内的控件可以通过属性设置支持拖出和拖入操作</td></tr>
    <tr><td align="left">13. 控件的背景色支持渐变色，新增前景色功能</td></tr>
    <tr><td align="left">14. 完善了多国语言的功能，能够更好的支持多语言动态切换，并提供示例程序examples/MultiLang</td></tr>
    <tr><td align="left">15. 完善了DPI感知功能，支持Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2四种模式，支持独立设置DPI，支持高清DPI的适配，提供了示例程序examples/DpiAware</td></tr>
    <tr><td align="left">16. 移除了ui_components工程，CEF组件代码重新梳理，继承到duilib工程中，其他内容删除</td></tr>
    <tr><td align="left">17. 优化窗口的阴影功能，窗口的阴影使用svg图片，增加了阴影类型属性（shadow_type），可选值为：<br> "default", 默认阴影 <br> "big", 大阴影，直角（适合普通窗口）<br> "big_round", 大阴影，圆角（适合普通窗口）<br> "small", 小阴影，直角（适合普通窗口）<br> "small_round", 小阴影，圆角（适合普通窗口）<br> "menu", 小阴影，直角（适合弹出式窗口，比如菜单等）<br> "menu_round", 小阴影，圆角（适合弹出式窗口，比如菜单等）<br> "none", 无阴影</td></tr>
    <tr><td align="left">18. 新增对APNG/SVG/WEBP/ICO/LOTTIE/PAG图片格式的支持</td></tr>
    <tr><td align="left">19. 重新设计控件的loading功能，使用Box容器展示loading功能，通过xml文件配置loading界面（包括动画图片），并支持与动画图片交互</td></tr>
    <tr><td align="left">20. Label文本显示控件的功能加强：对文本齐方式新增加"两端对齐"，新增对竖排文本的支持（文本绘制方向从上到下，从右到左），新增支持设置行间距和设置字间距</td></tr>
    <tr><td align="left">21. Control控件支持全屏显示（通过调用新增加的Window::SetFullscreenControl函数实现该功能），CEF控件和WebView2控件支持F11切换页面全屏</td></tr>
    <tr>
        <td rowspan="22">新增控件</td>
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
    <tr><td align="left">17. WebView2Control：封装了WebView2控件的基本功能</td></tr>
    <tr><td align="left">18. GridBox/GridScrollBox：基于网格布局的控件</td></tr>
    <tr><td align="left">19. HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox：基于水平流式布局和垂直流式布局的控件</td></tr>
    <tr><td align="left">20. MenuBar：菜单栏控件</td></tr>
    <tr><td align="left">21. IconControl/BitmapControl：用户显示基于内存的小图标和位图数据</td></tr>
    <tr><td align="left">22. ChildWindow：子窗口控件，Windows平台的实现为系统原生子窗口（带有WS_CHILD属性）；其他平台为SDL的弹出式窗口，非原生子窗口，SDL不支持原生子窗口</td></tr>
    <tr>
        <td rowspan="3">性能优化</td>
        <td align="left">1. 优化了Control及子控件的内存占用，在界面元素较多的时候，内存占有率有大幅降低</td>
    </tr>
    <tr><td align="left">2. 优化了动画绘制流程，合并定时器的触发事件，避免播放控件动画或者播放动画图片的过程中导致界面很卡的现象</td></tr>
    <tr><td align="left">3. 基于虚表的ListBox控件及关联控件：通过优化实现机制，使得可用性和性能有较大改善</td></tr>
    <tr>
        <td rowspan="13">示例程序完善</td>
        <td align="left">1. examples/ColorPicker: 新增加了拾色器示例程序</td>
    </tr>
    <tr><td align="left">2. examples/ListCtrl：新增加了列表的示例程序，演示列表的个性功能</td></tr>
    <tr><td align="left">3. examples/render：新增加了渲染引擎示例程序，演示了大部分容器、控件、资源管理的功能</td></tr>
    <tr><td align="left">4. examples/TreeView：新增加了树控件的示例程序，演示了树控件的各种功能</td></tr>
    <tr><td align="left">5. examples/RichEdit：新增加了富文本编辑控件的示例程序，演示了富文本编辑控件的各种功能</td></tr>
    <tr><td align="left">6. examples/MultiLang：提供多语言的动态切换功能演示</td></tr>
    <tr><td align="left">7. examples/DpiAware：提供了DPI感知功能的功能演示</td></tr>
    <tr><td align="left">8. examples/threads：提供了多线程的功能演示</td></tr>
    <tr><td align="left">9. examples/WebView2：提供了WebView2控件的功能演示</td></tr>
    <tr><td align="left">10. examples/WebView2Browser：提供了WebView2控件的功能演示（多标签）</td></tr>
    <tr><td align="left">11. examples/layout：所有布局和容器的功能演示</td></tr>
    <tr><td align="left">12. examples/ChildWindow：子窗口控件的功能演示</td></tr>
    <tr><td align="left">13. 其他的示例程序：大部分进行了代码兼容性修改和优化，使得示例程序也可以当作测试程序使用</td></tr>
    <tr>
        <td rowspan="8">完善文档</td>
        <td align="left">1. README.md和docs子目录的文档重新进行了梳理，使得阅读者更容易理解界面库的功能、用法，更易上手</td>
    </tr>
    <tr><td align="left">2. 各个控件的接口没有单独整理成文档，因为可以直接阅读接口文件中的注释来达到目的，目前各个接口的注释是比较完善的</td></tr>
    <tr><td align="left">3. 各个平台的编译文档和依赖的编译脚本</td></tr>
    <tr><td align="left">4. 重新整理主体项目、依赖第三方源码的授权文件，统一放在licenses目录中管理</td></tr>
</table>

## 使用的第三方库说明
| 名称     |代码子目录                 | 用途                                 | license文件        | 协议分类          |
| :---     | :---                      |:---                                  |:---               |:---               |
|apng      |duilib/third_party/libpng  | 支持APNG图片格式                     |zlib/libpng License|zlib/libpng 许可协议，宽松型开源协议|
|libpng    |duilib/third_party/libpng  | 支持PNG图片格式                      |[libpng.LICENSE.txt](licenses/libpng.LICENSE.txt)|自定义的 BSD 风格宽松协议|
|zlib      |duilib/third_party/zlib    | 支持PNG/APNG图片格式<br>Zip文件解压  |[zlib.LICENSE.txt](licenses/zlib.LICENSE.txt)|zlib许可协议，宽松型开源协议|
|cximage   |duilib/third_party/cximage | 支持ICO图片格式                      |[cximage.LICENSE.txt](licenses/cximage.LICENSE.txt)|MIT 风格协议（非标准 MIT 协议）|
|giflib    |duilib/third_party/giflib  | 支持GIF图片格式                      |[giflib.LICENSE.txt](licenses/giflib.LICENSE.txt)|MIT 许可协议|
|libwebp   |duilib/third_party/libwebp | 支持WebP图片格式                     |[libWebP.LICENSE.txt](licenses/libwebp.LICENSE.txt)|BSD 3条款许可协议|
|stb_image |duilib/third_party/stb_image| 支持BMP图片格式<br>调整图片大小     |[stb_image.LICENSE.txt](licenses/stb_image.LICENSE.txt)|MIT 许可协议/公有领域协议|
|libjpeg-turbo|duilib/third_party/libjpeg-turbo| 支持JPEG图片格式             |[libjpeg-turbo.LICENSE.md](licenses/libjpeg-turbo.LICENSE.md)|IJG 许可证和修改后的BSD 3条款许可协议|
|nanosvg   |duilib/third_party/svg      | 支持SVG图片格式                     |[nanosvg.LICENSE.txt](licenses/nanosvg.LICENSE.txt)|zlib 许可协议|
|pugixml   |duilib/third_party/xml      | 支持资源描述XML的解析               |[pugixml.LICENSE.txt](licenses/pugixml.LICENSE.txt)|MIT 许可协议|
|ConvertUTF|duilib/third_party/convert_utf| 用于UTF-8/UTF-16编码转换         |[llvm.LICENSE.txt](licenses/llvm.LICENSE.txt)|Apache License Version 2.0 为主，<br>LLVM 例外条款补充，<br>历史版本兼容 legacy 协议|
|skia      |项目未包含skia源码          | 界面库渲染引擎<br>支持SVG图片格式<br>支持Lottie JSON动画|[skia.LICENSE.txt](licenses/skia.LICENSE.txt)|BSD 3条款许可协议|
|SDL       |项目未包含SDL源码           | 跨平台窗口管理                     |[SDL.LICENSE.txt](licenses/SDL.LICENSE.txt)|zlib 许可协议|
|duilib    |                            | NIM_Duilib_Framework<br>是基于duilib开发  |[duilib.LICENSE.txt](licenses/duilib.LICENSE.txt)|BSD 2条款许可协议|
|NIM_Duilib<br>Framework|               | 本项目是基于<br>NIM_Duilib_Framework开发   |[NIM_Duilib_Framework.LICENSE.txt](licenses/NIM_Duilib_Framework.LICENSE.txt)|MIT 许可协议|
|libcef    |duilib/third_party/libcef   | 用于加载CEF模块|[libcef.LICENSE.txt](licenses/libcef.LICENSE.txt)|BSD 3条款许可协议|
|udis86    |duilib/third_party/libudis86| 反汇编计算完整性指令最短长度         |[udis86.LICENSE.txt](licenses/udis86.LICENSE.txt)|BSD 2条款许可协议|
|WebView2  |duilib/third_party/<br>Microsoft.Web.WebView2| 支持WebView2控件 |[Microsoft.Web.WebView2.LICENSE.txt](licenses/Microsoft.Web.WebView2.LICENSE.txt)|BSD 3条款许可协议|
|libpag    |duilib/third_party/libpag   | 支持PAG动画文件<br>（该功能默认未启用，详见后续文档） |[libpag.LICENSE.txt](licenses/libpag.LICENSE.txt)|Apache License Version 2.0(主体)<br>libpag依赖的第三方组件的<br>授权协议很多，详见目录:<br>`duilib/third_party/libpag/licenses`<br>中的文件。如果介意libpag的授权协议<br>（包括主体协议/第三方组件协议），<br>可以不启用libpag。|

## 界面效果预览
使用该界面库编写的示例程序，该文档可以见到各个控件的展示效果：[docs/Examples.md](docs/Examples.md) 

## 关于PAG动画文件格式
* 目前PAG动画文件格式仅在Window平台支持，其他平台暂未支持
* 支持PAG动画文件格式功能默认关闭（因为需要自己编译libpag.lib和libpag.dll，放进项目才能正常编译运行）
* 支持PAG动画格式的开启方法：    
（1）使用文本编译器打开[`msvc/PropertySheets/LibPagSettings.props`](msvc/PropertySheets/LibPagSettings.props)文件，将`LibPagEnabled`变量的支持修改为`1`    
（2）参照以下文档编译libpag库：[`duilib/third_party/libpag/windows/libpag-build.md`](duilib/third_party/libpag/windows/libpag-build.md)     
* 编译nim_duilib的时候，需要使用`build/duilib.sln`或者`build/examples.sln`编译，其他方式编译不支持。
* libpag库的主体授权协议为Apache License Version 2.0，其依赖的第三方组件的授权协议很多，<br>详见目录:`duilib/third_party/libpag/licenses`中的文件。<br>如果介意libpag的授权协议（包括主体协议/第三方组件协议），可以不启用libpag。

## 编程语言
- C/C++: 编译器需要支持C++20

## 支持的操作系统
- Windows：Windows 7.0 版本及以上
- Linux：OpenEuler、OpenKylin（开放麒麟）、UbuntuKylin（优麒麟）、中科方德、统信UOS、Ubuntu、Debian、Fedora、OpenSuse等
- macOS：12+
- FreeBSD

## 支持的编译器
- Visual Studio 2022（Windows）
- LLVM（Windows）
- MinGW-W64：gcc/g++、clang/clang++（Windows）
- gcc/g++（Linux）
- clang/clang++（Linux）
- clang/clang++（macOS）
- clang/clang++（FreeBSD）

## A. 编译过程（Windows平台）
### 一、准备工作：安装必备的软件
1. 安装python3（python的主版本需要是3，需要添加到Path环境变量）    
（1）首先安装python3    
（2）在Windows的设置里面，关闭python.exe和python3.exe的"应用执行别名"，否则编译skia的脚本执行有问题。Windows设置入口：设置 -> 应用 -> 高级应用设置 -> 应用执行别名    
（3）到python.exe所在目录中，复制一份python.exe，改名为python3.exe: 确保命令行参数中可以访问到python3.exe   
（4）在命令行验证：`> python3.exe --version` 可以查看python的版本号     
2. 安装Git For Windows: 2.44版本，git需要添加到Path环境变量，确保命令行参数中可以访问到git.exe    
3. 安装Visual Studio 2022社区版    
4. 安装LLVM：21.1.4 Win64 版本（其他版本也可以）    
（1）安装目录：`C:\LLVM`    
（2）注意事项：如果安装在其他目录，安装目录中不能有空格，否则编译会遇到问题。

### 二、使用脚本自动编译（推荐）
该脚本自动完成相关源码下载和编译工作。    
选定一个工作目录（注意事项：路径中不能包含空格，否则编译脚本会出错），创建一个脚本`build.bat`，将下面已经整理好脚本复制进去，保存文件。    
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

## B. 编译过程（Linux平台）
### 一、准备工作：安装必备的软件
不同的操作系统平台，可以按照以下列表安装必备的软件。
| 操作系统平台            |桌面类型        |需要安装的模块及安装命令（必选） | 
| :---                    | :---           | :---                    |
|OpenEuler                |UKUI/DDE（X11） |`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel`| 
|OpenKylin（开放麒麟）    | Wayland        |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|UbuntuKylin（优麒麟）    | X11            |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-devlibxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|中科方德                 | X11            |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|统信UOS                  | X11            |`sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|Ubuntu                   |GNOME（Wayland）|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip bzip2 libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Debian                   |GNOME（Wayland）|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Fedora                   |GNOME（Wayland）|`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel`|
|OpenSuse                 |KDE（X11）      |`sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel`|

### 二、使用脚本自动编译（推荐）
该脚本自动完成相关源码下载和编译工作。    
选定一个工作目录（注意事项：路径中不能包含空格，否则编译脚本会出错），创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
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
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
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

## C. 编译过程（macOS平台）
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

### 二、使用脚本自动编译（推荐）
该脚本自动完成相关源码下载和编译工作。    
选定一个工作目录（注意事项：路径中不能包含空格，否则编译脚本会出错），创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
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
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
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

## D. 编译过程（FreeBSD平台）
### 一、准备工作：安装必备的软件
```
sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2
```
### 二、使用脚本自动编译（推荐）
该脚本自动完成相关源码下载和编译工作。    
选定一个工作目录（注意事项：路径中不能包含空格，否则编译脚本会出错），创建一个脚本`build.sh`，将下面已经整理好脚本复制进去，保存文件。    
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
### 三、手动编译过程（FreeBSD平台）
1. 设置工作目录：`~/develop`    
2. 获取相关代码    
（1）`git clone https://github.com/rhett-lee/nim_duilib`      
（2）`git clone https://github.com/rhett-lee/skia_compile`    
（3）`git clone https://github.com/google/skia.git`  
（4）`git clone https://github.com/libsdl-org/SDL.git`  
3. 编译Skia库    

| 操作系统平台 |参考文档（网络链接） |  参考文档（本地文件） |
| :---         | :---                |:---                   |
|FreeBSD         |[FreeBSD下编译skia.md](https://github.com/rhett-lee/skia_compile/blob/main/FreeBSD下编译skia.md) | [FreeBSD下编译skia.md](../skia_compile/FreeBSD下编译skia.md) |

    注意事项：skia源码编译的时候，只支持使用LLVM编译。
4. 编译SDL库 
```
#!/usr/bin/env bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. 编译nim_duilib
```
#!/usr/bin/env bash
cd ~/develop/nim_duilib/
chmod +x ./build/freebsd_build.sh
./build/freebsd_build.sh
```
编译完成后，在bin目录中生成了可执行文件。    

## 开发计划
 - 跨平台（Windows/Linux/macOS/FreeBSD）的窗口引擎（基于[SDL3.0](https://www.libsdl.org/)）不断测试与完善（X11和Wayland）
 - 动画功能的加强
 - 不断测试发现缺陷并修复，不断完善代码
 - 其他待补充

## 参考文档

 - [快速上手](docs/Getting-Started.md)
 - [示例程序](docs/Examples.md)
 - [全局样式：字体、颜色、图片等资源](docs/Global.md)
 - [窗口的样式](docs/Window.md)
 - [容器的样式](docs/Box.md)
 - [控件的样式](docs/Control.md)
 - [菜单的样式](docs/Menu.md)
 - [控件的事件/消息](docs/Events.md)
 - [XML文件中各控件的节点名称](docs/XmlNode.md)
 - [XML文件中直接响应控件的事件](docs/XmlEvents.md)
 - [CEF控件](docs/CEF.md)
 - [项目编译相关文档和脚本](build/build.md)
 - [参考文档](docs/Summary.md)

## 相关链接
1. Skia的编译文档库，点击访问：[skia compile](https://github.com/rhett-lee/skia_compile) ：    
2. 本项目是直接在NIM_Duilib_Framework项目的基础上继续开发的，项目地址：[NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)
3. NIM_Duilib_Framework项目是基于duilib开发的，项目地址：[duilib](https://github.com/duilib/duilib)
