# nim duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) 是一个界面库，包含了一整套桌面软件的开发部件，使用C++语言开发。nim_duilib是在[NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework)库的基础上进行了代码功能完善和扩展，但由于代码结构和资源XML文件的格式均有调整，所以不能直接兼容基于NIM_Duilib_Framework开发的程序，但代码和资源XML迁移的难度不大。您可以直接使用nim_duilib界面库来开发常用的桌面应用，以简化应用程序的UI开发工作。

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## 特色

 - 核心特性包括：
    - 通用样式：支持以XML格式描述应用程序的窗口属性、布局属性、控件属性等，方便调整界面元素的位置和大小，较为灵活
    - 控件丰富：包含各种常见的窗口和控件，如图片控件、动画控件、按钮、文本框、列表控件、虚表控件、树控件、颜色选择控件、菜单等
    - 事件驱动：基于消息机制的事件处理，使得UI交互逻辑清晰
    - 皮肤支持：通过XML文件定义皮肤结构，可以轻松改变界面风格
    - 性能优异：界面资源的内存占有率低，默认使用Skia引擎绘制，绘制性能较高
 - 多种图片格式，支持的图片文件格式有：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
 - 动画图片，支持GIF动画文件、APNG动画文件、WebP动画文件
 - 抽象渲染接口，支持Skia渲染引擎（也可以通过接口扩展为其他渲染引擎提供支持）
 - DPI 自适应，支持高清DPI的适配
 - 触控设备支持（Surface、Wacom）
 - 多国语言支持，易于实现国际化
 - CEF 控件支持

## 目录

├─`base` Google base 基础类库  
├─`duilib` nim_duilib 的核心代码，依赖 base    
├─`ui_components` 基于 duilib 封装的常用组件库如 `msgbox`、`toast`、`cef_control` 等  
├─`examples` 各类示例程序源代码  
├─`docs` 说明文档  
├─`libs` 静态库编译后的输出目录，包含预设的一些 CEF 组件静态库  
├─`bin` 各个示例程序输出目录，包含预设的皮肤和语言文件以及 CEF 依赖  
├─`build` Windows下的程序配置及manifest文件等    
├─`licenses` 引用的其他开源代码对应的licenses    
├─`duilib/third_party` nim_duilib 核心代码依赖的第三方库    
　　├─ apng 用于对APNG图片格式的支持     
　　├─ libpng 用于对PNG图片格式的支持     
　　├─ cximage 用于对GIF/ICO图片格式的支持      
　　├─ libwebp 用于对WebP图片格式的支持     
　　├─ stb_image 用于对JPEG/BMP图片格式的支持     
　　├─ svg 用于对SVG图片格式的支持     
　　├─ unzip 用于对zip格式的资源包的支持     
　　├─ zlib 用于对PNG/APNG图片格式的支持     
　　├─ xml 用于对资源描述XML的解析支持     

## 开始

1. 获取项目代码

```bash
git clone https://github.com/rhett-lee/nim_duilib
```

2. 获取skia代码的编译方法和修改的代码（nim_duilib默认使用skia作为绘制引擎，所以先要编译skia）

```bash
git clone https://github.com/rhett-lee/skia_compile
```

3. 编译skia源码：按照skia_compile目录中的[Windows下编译skia.md文档](../skia_compile/Windows下编译skia.md)中的方法，编译出skia相关的lib文件    
   注意事项：skia源码应该与nim_duilib源码位于相同的目录下。    
   注意事项：skia源码编译的时候，应使用LLVM编译，程序运行比较流畅；如果使用VS编译，运行速度很满，界面比较卡。    
   检查方法：编译成功以后，在skia/out的子目录下，有生成skia.lib等lib文件

4. 编译nim_duilib：进入 `nim_duilib/examples` 目录，使用 Visual Studio 2022版本的 IDE 打开 `examples.sln`，选择编译选项为Debug|x64或者Release|x64，按下 F7 即可编译所有示例程序（编译完成的示例程序位于bin目录中）。

5. 编译附件说明：    
（1）项目中工程的编译环境为Visual Studio 2022版本，如果使用其他版本的Visual Studio编译器，需要手动更新编译工程的属性。    
（2）项目中的工程默认配置是x64的，如果需要编译Win32的程序，在编译skia的时候，也需要启动32位的命令行（x86 Native Tools Command Prompt for VS 2022）   
（3）nim_duilib的代码兼容性默认是支持Win7以上系统，未支持Windows XP；Windows SDK的兼容性配置可在[duilib\duilib_config.h](duilib/duilib_config.h)文件中修改    
（4）渲染引擎的支持：nim_duilib内部支持Skia绘制引擎和Gdiplus绘制引擎，默认是Skia引擎，不建议选择Gdiplus引擎，因为Gdiplus引擎下有些控件的功能不完善；  
（5）如何配置切换渲染引擎：可修改[duilib\Core\GlobalManager.cpp](duilib/Core/GlobalManager.cpp)文件顶部的源码来切换渲染引擎。

## 文档

 - [快速上手](docs/Getting-Started.md)
 - [参考文档](docs/Summary.md)
 - [示例程序](docs/Examples.md)
 
## 基于NIM_Duilib_Framework源码做的主要修改
1. 调整了代码的组织结构，按照功能模块划分，大文件按类拆分为多个小文件，有利于理解整个库的体系结构；
2. 梳理了代码的接口文件，补充各个接口的注释和功能注释，有利于阅读和理解代码；
3. 对配置XML文件进行优化，调整了属性命名规则，控件的宽度和高度可以按百分比设置，图片属性增加了部分扩展，优化了图片加载流程；
4. 集成了Skia引擎，并作为默认的渲染引擎；
5. 扩展了图片资源的支持：新增加APNG/WEBP动画，新增对ICO的支持，并优化了图片加载引擎及代码实现逻辑；
6. XML文件解析引擎替换为pugixml parser，性能更优；
7. 对基本数据类型进行了梳理，对控件大小、估算大小、内边距、外边距、颜色、字体等做成基本类型，易于理解和维护；
8. 对布局进行了梳理，归类为：自由布局（浮动布局）、水平布局、垂直布局、水平瓦片布局、垂直瓦片布局、虚表水平布局、虚表垂直布局、虚表水平瓦片布局、虚表垂直瓦片布局，使布局的概念更易理解，易于扩展；
9. 新增加了一些功能控件：
 - GroupBox：分组容器
 - HotKey：热键控件
 - HyperLink：带有超级链接的文字
 - IPAddress：IP地址控件
 - Line：画线控件
 - RichText：格式化文本（类HTML格式）
 - Split: 分割条控件/容器
 - TabCtrl：多标签控件（类似浏览器的多标签）
 - ListCtrl：列表控件（Report/Icon/List三个视图）
 - PropertyGrid: 属性表控件，支持文本、数字、复选框、字体、颜色、日期、IP地址、热键、文件路径、文件夹等属性
 - ColorPicker：拾色器，独立窗口，其中的子控件，可以单独作为颜色控件来使用
 - ComboButton：带有下拉组合框的按钮
 - 新增加了ShadowWnd类：是WinImplBase的子类，使用附加阴影窗口实现的基类，实现了创建窗口并附加的功能，提供没有 WS_EX_LAYERED 属性的窗口阴影
10. 完善了原有控件的一些功能：
 - 对窗口类（Window）增加了新的属性：的功能进行了完善，提高对DPI自适应、窗口消息的代码容错，代码结构做了调整
 - 对窗口类（Window）增加了新的属性：use_system_caption，提供使用操作系统默认标题栏的选项
 - 对窗口（Window）引入WindowDropTarget辅助类，提供对基于窗口的拖放功能的支持
 - 对资源管理相关的部分做了梳理，使得字体、颜色、图片等资源管理更容易理解
 - 对ListBox控件进行了优化：细分为ListBox、VListBox、HListBox、VTileListBox、HTileListBox、VirtualListBox、VirtualVListBox、VirtualHListBox、VirtualVTileListBox、VirtualHTileListBox几种，功能更加完善，基于虚表的ListBox功能可用性提升；
 - 对组合框（Combo）关联的控件（CheckCombo、FilterCombo）进行了优化，提升可用性
 - 对日期时间（DateTime）控件的编辑功能进行了完善
 - 对菜单（CMenuWnd）类的功能代码进行优化，按新的结构重新实现菜单，使得菜单内的控件完全兼容现有容器/控件体系，易于理解和维护
 - 对富文本编辑控件（RichEdit）进行功能优化和扩展，丰富了较多常用功能
 - 对树控件（TreeView）进行功能优化和扩展，丰富了较多常用功能，提升可用性
 - 全局资源（GlobalManager）的接口进行了优化，使得所有资源全部通过此接口管理，易于理解和维护
 - 同一个窗口内的不同容器之间，容器内的控件可以通过属性设置支持拖出和拖入操作
11. 性能优化：
 - 优化了Control及子控件的内存占用，在界面元素较多的时候，内存占有率有大幅降低；
 - 优化了动画绘制流程，合并定时器的触发事件，避免播放控件动画或者播放动画图片的过程中导致界面很卡的现象
 - 基于虚表的ListBox控件及关联控件：通过优化实现机制，使得可用性和性能有较大改善
12. 示例程序完善：
 - examples/ColorPicker: 新增加了拾色器示例程序
 - examples/ListCtrl：新增加了列表的示例程序，演示列表的个性功能
 - examples/render：新增加了渲染引擎示例程序，演示了大部分容器、控件、资源管理的功能
 - examples/TreeView：新增加了树控件的示例程序，演示了树控件的各种功能
 - examples/RichEdit：新增加了富文本编辑控件的示例程序，演示了富文本编辑控件的各种功能
 - 其他的示例程序：大部分进行了代码兼容性修改和优化，使得示例程序也可以当作测试程序使用；
13. 文档的完善：
- README.md和docs子目录的文档重新进行了梳理，使得阅读者更容易理解界面库的功能、用法，更易上手
- 各个控件的接口没有单独整理成文档，因为可以直接阅读接口文件中的注释来达到目的，目前各个接口的注释是比较完善的

## 开发计划
 - 窗口的封装优化：支持跨平台的窗口引擎（SDL/GTK等，进行跨平台尝试）
 - 支持DPI感知：支持Per-Monitor (V2) DPI 感知，提高对多显示器模式下的用户体验；
 - 不断测试发现缺陷并修复，不断完善代码
 - 其他待补充

