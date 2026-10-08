English | [简体中文](README.md)

> Last synced: 2026-09-29

# nim_duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) is a cross-platform GUI library developed in C++, derived from the classic duilib GUI library with deep optimization and feature extensions. It supports the Windows/Linux/macOS/FreeBSD platforms. The supported Linux distributions include OpenEuler, OpenKylin, UbuntuKylin, UnionTech UOS, NeoKylin, Ubuntu, Fedora, Debian, and more, focusing on simplifying efficient desktop application development. Its design incorporates the DirectUI philosophy, describing the UI layout through XML to achieve separation of visuals and logic, significantly improving development flexibility and maintainability.

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## Core Technical Features

 - XML layout driven: XML files are used to define the UI structure, fully separating the UI layout from the business logic. Developers can quickly adjust control positions, sizes, and styles by modifying XML without changing the C++ core code, greatly improving development and iteration efficiency.
 - Rich control system: Built-in basic controls such as buttons, text boxes, list views, virtual list controls, tree controls, sliders, progress bars, menus, color pickers, property pages, and tab pages, while also supporting custom control development to meet diverse UI design needs.
 - Efficient rendering mechanism: Uses Skia as the rendering engine to achieve windowless drawing of controls, reducing system resource usage and improving UI refresh speed. Supports hardware-accelerated rendering (the backend drawing supports CPU or GPU rendering), ensuring smooth operation of complex interfaces.
 - Event-driven: Event handling based on the message mechanism makes the UI interaction logic clear, and supports configuring event response code in XML files.
 - Multiple image formats: Supports SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO image formats.
 - Animation format support: Supports GIF, APNG, WEBP, Lottie JSON, and PAG animation file formats.
 - Multilingual and internationalization: Supports dynamic switching between multiple languages, facilitating the development of globalized applications.
 - Dynamic skinning support: Defines the skin structure through XML files, making it easy to change the UI style, with support for dynamic skinning.
 - Window shadow support: Supports rounded-corner shadows and right-angle shadows for windows, with selectable shadow sizes that can be updated in real time; on Windows and macOS systems, system shadows are supported.
 - DPI awareness support: Provides four modes—Unaware, SystemAware, PerMonitorAware, and PerMonitorAware_V2—supports setting DPI independently and adapting to high-DPI displays (Windows platform only).
 - CEF control support: Supports libcef version 109 for compatibility with Windows 7; supports libcef version 142 for Windows 10 and above, as well as the Linux and macOS platforms.
 - WebView2 control support: Supports using the WebView2 control to display web pages; its interface is simply wrapped and easier to use (Windows platform only).
 - SDL3 support: SDL3 can be used as the provider of basic functions such as window management and input/output, thereby enabling cross-platform support (currently adapted for Windows/Linux/macOS/FreeBSD platforms).
 - Theme switching support: Light and dark themes are supported by default, accent color settings are supported, and auxiliary tools for designing themes are provided.

## Directory Structure
| Directory | Description |
| :--- | :--- |
| duilib        | Source code of the project |
| docs          | Documentation of the project, including feature descriptions and attribute lists for each control |
| bin           | Output directory for each example program, containing preset skins, language files, and CEF dependencies |
| licenses      | License files corresponding to other referenced open-source code |
| cmake         | Common settings depended on during CMake builds |
| build         | Build scripts and build projects for each platform (including VC build projects) |
| msvc          | Application manifest files and common VC project configuration for the Windows platform |
| examples      | Source code of the project's example programs, covering basic usage examples of all controls (example programs, see [docs/Examples.en.md](docs/Examples.en.md)) |
| duilib/third_party| Third-party libraries that the project code depends on; details are in subsequent documentation |

## Major Modifications Based on the NIM_Duilib_Framework Source Code
<table>
    <tr>
        <th>Category</th>
        <th>Modification</th>
    </tr>
    <tr>
        <td rowspan="12">Overall Improvements</td>
        <td align="left">1. Reorganized the code structure by functional modules, splitting large files into multiple smaller files by class, which helps understand the overall architecture of the library</td>
    </tr>
    <tr><td align="left">2. Reviewed the code's interface files and added comments and functional annotations for each interface, facilitating code reading and understanding</td></tr>
    <tr><td align="left">3. Optimized the configuration XML files, adjusted the attribute naming rules, allowed control width and height to be set as percentages, added some extensions to image attributes, and optimized the image loading process</td></tr>
    <tr><td align="left">4. Extended image resource support: added APNG/WEBP animation/Lottie JSON animation/PAG animation, added ICO support, and optimized the image loading engine and code implementation logic</td></tr>
    <tr><td align="left">5. Reimplemented the code for each layout and added common UI layout schemes, generally categorized as: float layout, horizontal layout, vertical layout, horizontal flow layout, vertical flow layout, grid layout, horizontal tile layout, vertical tile layout, virtual horizontal layout, virtual vertical layout, virtual horizontal tile layout, and virtual vertical tile layout, making the layout concept easier to understand and extend. See the documentation (docs/Box.en.md) for details</td></tr>
    <tr><td align="left">6. Replaced the XML file parsing engine with the pugixml parser, which offers better performance</td></tr>
    <tr><td align="left">7. Removed the dependency on the base library; message loop and thread communication related functions were reimplemented</td></tr>
    <tr><td align="left">8. Integrated the Skia engine as the default rendering engine</td></tr>
    <tr><td align="left">9. Added SDL3 support for cross-platform compatibility (already adapted for Windows, Linux, macOS, and FreeBSD platforms)</td></tr>
    <tr><td align="left">10. Moved the CEF component into the duilib project and upgraded the CEF version (supports libcef version 109 for compatibility with Windows 7; supports libcef version 142 for Windows 10 and above)</td></tr>
    <tr><td align="left">11. Redesigned the image management interface and loading process (Image directory), supporting multi-threaded image loading for better extensibility to other image formats</td></tr>
    <tr><td align="left">12. Redesigned the XML file structure of the global resource manager (global.xml), improved font management, and added font fallback support, thereby enabling Emoji font display; supports light and dark themes and accent colors</td></tr>
    <tr>
        <td rowspan="22">Feature Improvements</td>
        <td align="left">1. Added new attributes to the Window class: improved its functions, enhanced code fault tolerance for DPI adaptation and window messages, and adjusted the code structure</td>
    </tr>
    <tr><td align="left">2. Added new attributes to the Window class: use_system_caption, snap_layout_menu, sys_menu, sys_menu_rect, and icon, providing the option to use the operating system's default title bar; the custom-drawn title bar functions similarly to the system title bar</td></tr>
    <tr><td align="left">3. Introduced the WindowDropTarget helper class for the Window class, providing support for window-based drag-and-drop functionality</td></tr>
    <tr><td align="left">4. Reviewed the resource management related parts, making the management of fonts, colors, images, and other resources easier to understand</td></tr>
    <tr><td align="left">5. Optimized the ListBox control: subdivided into ListBox, VListBox, HListBox, VTileListBox, HTileListBox, VirtualListBox, VirtualVListBox, VirtualHListBox, VirtualVTileListBox, and VirtualHTileListBox variants, with more complete functionality and improved usability of the virtual-list-based ListBox</td></tr>
    <tr><td align="left">6. Optimized the controls associated with the Combo box (CheckCombo, FilterCombo), improving usability</td></tr>
    <tr><td align="left">7. Improved the editing functionality of the DateTime control</td></tr>
    <tr><td align="left">8. Optimized the functional code of the CMenuWnd class and reimplemented the menu according to the new structure, making the controls inside the menu fully compatible with the existing container/control system, easy to understand and maintain</td></tr>
    <tr><td align="left">9. Optimized and extended the functionality of the RichEdit control, adding many commonly used features</td></tr>
    <tr><td align="left">10. Optimized and extended the functionality of the TreeView control, adding many commonly used features and improving usability</td></tr>
    <tr><td align="left">11. Optimized the GlobalManager interface so that all resources are managed through this interface, easy to understand and maintain</td></tr>
    <tr><td align="left">12. Controls within containers in the same window can be configured via attributes to support drag-out and drag-in operations between different containers</td></tr>
    <tr><td align="left">13. Control background colors now support gradients, and a foreground color feature was added</td></tr>
    <tr><td align="left">14. Improved the multilingual feature to better support dynamic language switching, and provided the examples/MultiLang example program</td></tr>
    <tr><td align="left">15. Improved the DPI awareness feature, supporting the four modes Unaware, SystemAware, PerMonitorAware, and PerMonitorAware_V2, supporting independent DPI settings and high-DPI adaptation, and provided the examples/DpiAware example program</td></tr>
    <tr><td align="left">16. Removed the ui_components project; the CEF component code was reorganized and merged into the duilib project, and other contents were deleted</td></tr>
    <tr><td align="left">17. Optimized the window shadow feature; the window shadow uses an SVG image, added the shadow type attribute (shadow_type), and supports custom-drawn shadows and system shadows. See the `docs/Window.en.md` documentation for details</td></tr>
    <tr><td align="left">18. Added support for APNG/SVG/WEBP/ICO/LOTTIE/PAG image formats</td></tr>
    <tr><td align="left">19. Redesigned the control loading feature, using a Box container to display the loading function, configuring the loading interface (including animation images) through an XML file, and supporting interaction with animation images</td></tr>
    <tr><td align="left">20. Enhanced the Label text display control: added "justify" text alignment, added support for vertical text (text drawing direction from top to bottom, right to left), and added support for setting line spacing and character spacing</td></tr>
    <tr><td align="left">21. The Control control supports full-screen display (implemented by calling the newly added Window::SetFullscreenControl function); the CEF control and WebView2 control support switching the page to full screen via F11</td></tr>
    <tr><td align="left">22. Improved the details of control animation functions and introduced easing functions, supporting the configuration of control animation properties, such as setting the easing function type, total animation duration, and playback interval</td></tr>
    <tr>
        <td rowspan="33">New Controls / New Containers</td>
        <td align="left">1. GroupBox: grouping container</td>
    </tr>
    <tr><td align="left">2. HotKey: hotkey control</td></tr>
    <tr><td align="left">3. HyperLink: text with hyperlinks</td></tr>
    <tr><td align="left">4. IPAddress: IP address control</td></tr>
    <tr><td align="left">5. Line: line drawing control</td></tr>
    <tr><td align="left">6. RichText: formatted text (HTML-like format)</td></tr>
    <tr><td align="left">7. Split: splitter control/container</td></tr>
    <tr><td align="left">8. TabCtrl: multi-tab control (similar to browser tabs)</td></tr>
    <tr><td align="left">9. ListCtrl: list control (Report/Icon/List three views)</td></tr>
    <tr><td align="left">10. PropertyGrid: property grid control supporting text, number, checkbox, font, color, date, IP address, hotkey, file path, folder, and other properties</td></tr>
    <tr><td align="left">11. ColorPicker: color picker, a standalone window whose child controls can be used individually as color controls</td></tr>
    <tr><td align="left">12. ComboButton: button with a dropdown combo box</td></tr>
    <tr><td align="left">13. DirectoryTree: directory tree control for displaying directories in the file system</td></tr>
    <tr><td align="left">14. AddressBar: address bar control for displaying the path of the local file system</td></tr>
    <tr><td align="left">15. WebView2Control: encapsulates the basic functions of the WebView2 control</td></tr>
    <tr><td align="left">16. GridBox/GridScrollBox: controls based on grid layout</td></tr>
    <tr><td align="left">17. HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox: controls based on horizontal flow layout and vertical flow layout</td></tr>
    <tr><td align="left">18. MenuBar: menu bar control</td></tr>
    <tr><td align="left">19. IconControl/BitmapControl: used to display small icons and bitmap data based on memory</td></tr>
    <tr><td align="left">20. ChildWindow: child window control. On the Windows platform it is implemented as a native child window (with the WS_CHILD attribute); on other platforms it is an SDL popup window, not a native child window, since SDL does not support native child windows</td></tr>
    <tr><td align="left">21. ControlDragableT (template class, including the following four standard controls: ControlDragable/BoxDragable/HBoxDragable/VBoxDragable): supports reordering child controls within the same Box by dragging, and supports adjusting the container a control belongs to by dragging across different Boxes</td></tr>
    <tr><td align="left">22. ControlMovableT (template class, including the following four standard controls: ControlMovable/BoxMovable/HBoxMovable/VBoxMovable): supports adjusting the position of a control by mouse dragging, and also supports adjusting the position of the parent container by mouse dragging</td></tr>
    <tr><td align="left">23. ControlResizableT (template class, including the following four standard controls: ControlResizable/BoxResizable/HBoxResizable/VBoxResizable): supports adjusting the size of a control by mouse dragging, with functionality similar to resizing a window</td></tr>
    <tr><td align="left">24. XmlBox: a container that supports loading and previewing the XML files of the GUI library, which can be used to preview the display effects of controls defined in XML files</td></tr>
    <tr><td align="left">25. Pane/PanelHBox/PanelVBox: Titled panel container (PanelTemplate<Box>), supporting title bar styling, collapse/expand and accordion groups</td></tr>
    <tr><td align="left">26. MessageBoxWnd: a built-in owner-drawn skin modal message box.</td></tr>
    <tr><td align="left">27. ToastWnd: a built-in owner-drawn skin non-modal toast notification, with auto-dismiss, multi-toast stacking, hover-to-pause and click-to-dismiss.</td></tr>
    <tr><td align="left">28. SearchBox: Composite search box with left icon, edit box and clear button.</td></tr>
    <tr><td align="left">29. SpinBox: Numeric input box, derived from RichEdit, supports step/range/spin buttons.</td></tr>
    <tr><td align="left">30. Switch: Toggle switch control derived from CheckBox with sliding animation and color transition</td></tr>
    <tr><td align="left">31. Switch: Badge control for unread counts/red dots; shows "99+" when exceeding the limit, hidden automatically when count<=0</td></tr>
    <tr><td align="left">32. Flyout: a general-purpose popup (flying card) windowd. It is used to pop up arbitrary Box content around an anchor control (action panels, confirmation cards, rich-content tips, etc.).</td></tr>
    <tr><td align="left">33. Calendar: Calendar control </td></tr>
    <tr>
        <td rowspan="3">Performance Optimization</td>
        <td align="left">1. Optimized the memory usage of Control and its child controls, greatly reducing memory footprint when there are many UI elements</td>
    </tr>
    <tr><td align="left">2. Optimized the animation drawing process and merged timer trigger events to avoid UI lag when playing control animations or animation images</td></tr>
    <tr><td align="left">3. The virtual-list-based ListBox control and its associated controls: by optimizing the implementation mechanism, both usability and performance were significantly improved</td></tr>
    <tr>
        <td rowspan="15">Example Program Improvements</td>
        <td align="left">1. examples/ColorPicker: added a color picker example program</td>
    </tr>
    <tr><td align="left">2. examples/ListCtrl: added a list example program demonstrating the list's distinctive features</td></tr>
    <tr><td align="left">3. examples/render: added a rendering engine example program demonstrating most container, control, and resource management functions</td></tr>
    <tr><td align="left">4. examples/TreeView: added a tree control example program demonstrating various tree control functions</td></tr>
    <tr><td align="left">5. examples/RichEdit: added a rich text edit control example program demonstrating various rich text edit control functions</td></tr>
    <tr><td align="left">6. examples/MultiLang: demonstrates dynamic switching between multiple languages</td></tr>
    <tr><td align="left">7. examples/DpiAware: demonstrates the DPI awareness feature</td></tr>
    <tr><td align="left">8. examples/threads: demonstrates multi-threading functionality</td></tr>
    <tr><td align="left">9. examples/WebView2: demonstrates the WebView2 control</td></tr>
    <tr><td align="left">10. examples/WebView2Browser: demonstrates the WebView2 control (multi-tab)</td></tr>
    <tr><td align="left">11. examples/layout: demonstrates all layouts and containers</td></tr>
    <tr><td align="left">12. examples/ChildWindow: demonstrates the child window control</td></tr>
    <tr><td align="left">13. examples/XmlPreview: tests the XML file UI preview feature (testing the XmlBox container)</td></tr>
    <tr><td align="left">14. examples/ColorTheme: a preview and test program for color themes</td></tr>
    <tr><td align="left">15. Other example programs: most have undergone code compatibility modifications and optimizations, allowing the example programs to also be used as test programs</td></tr>
    <tr>
        <td rowspan="8">Documentation Improvements</td>
        <td align="left">1. Reorganized the README.md and the documentation in the docs subdirectory, making it easier for readers to understand the GUI library's functions and usage, and easier to get started</td>
    </tr>
    <tr><td align="left">2. The interfaces of each control are not organized into separate documents, because the purpose can be achieved by directly reading the comments in the interface files; currently the comments for each interface are fairly complete</td></tr>
    <tr><td align="left">3. Build documentation for each platform and the dependent build scripts</td></tr>
    <tr><td align="left">4. Reorganized the license files for the main project and dependent third-party source code, unified and managed in the licenses directory</td></tr>
</table>

## Description of Third-Party Libraries Used
| Name | Code Subdirectory | Purpose | License File | License Classification |
| :--- | :--- |:--- |:--- |:--- |
|apng |duilib/third_party/libpng | Supports APNG image format |zlib/libpng License|zlib/libpng License, a permissive open-source license|
|libpng |duilib/third_party/libpng | Supports PNG image format |[libpng.LICENSE.txt](licenses/libpng.LICENSE.txt)|Custom BSD-style permissive license|
|zlib |duilib/third_party/zlib | Supports PNG/APNG image format<br>Zip file decompression |[zlib.LICENSE.txt](licenses/zlib.LICENSE.txt)|zlib license, a permissive open-source license|
|cximage |duilib/third_party/cximage | Supports ICO image format |[cximage.LICENSE.txt](licenses/cximage.LICENSE.txt)|MIT-style license (non-standard MIT license)|
|giflib |duilib/third_party/giflib | Supports GIF image format |[giflib.LICENSE.txt](licenses/giflib.LICENSE.txt)|MIT License|
|libwebp |duilib/third_party/libwebp | Supports WebP image format |[libWebP.LICENSE.txt](licenses/libwebp.LICENSE.txt)|BSD 3-Clause License|
|stb_image |duilib/third_party/stb_image| Supports BMP image format<br>Image resizing |[stb_image.LICENSE.txt](licenses/stb_image.LICENSE.txt)|MIT License / Public Domain License|
|libjpeg-turbo |duilib/third_party/libjpeg-turbo| Supports JPEG image format |[libjpeg-turbo.LICENSE.md](licenses/libjpeg-turbo.LICENSE.md)|IJG License and modified BSD 3-Clause License|
|nanosvg |duilib/third_party/svg | Supports SVG image format |[nanosvg.LICENSE.txt](licenses/nanosvg.LICENSE.txt)|zlib License|
|pugixml |duilib/third_party/xml | Supports parsing of resource description XML |[pugixml.LICENSE.txt](licenses/pugixml.LICENSE.txt)|MIT License|
|ConvertUTF |duilib/third_party/convert_utf| Used for UTF-8/UTF-16 encoding conversion |[llvm.LICENSE.txt](licenses/llvm.LICENSE.txt)|Primarily Apache License Version 2.0,<br>supplemented by LLVM exception clauses,<br>legacy protocol for historical versions|
|skia |Project does not include skia source code | GUI library rendering engine<br>Supports SVG image format<br>Supports Lottie JSON animation|[skia.LICENSE.txt](licenses/skia.LICENSE.txt)|BSD 3-Clause License|
|SDL |Project does not include SDL source code | Cross-platform window management |[SDL.LICENSE.txt](licenses/SDL.LICENSE.txt)|zlib License|
|duilib | | NIM_Duilib_Framework<br>is developed based on duilib |[duilib.LICENSE.txt](licenses/duilib.LICENSE.txt)|BSD 2-Clause License|
|NIM_Duilib<br>Framework| | This project is developed based on<br>NIM_Duilib_Framework |[NIM_Duilib_Framework.LICENSE.txt](licenses/NIM_Duilib_Framework.LICENSE.txt)|MIT License|
|libcef |duilib/third_party/libcef | Used to load the CEF module|[libcef.LICENSE.txt](licenses/libcef.LICENSE.txt)|BSD 3-Clause License|
|udis86 |duilib/third_party/libudis86| Disassembly to calculate the minimum length of complete instructions |[udis86.LICENSE.txt](licenses/udis86.LICENSE.txt)|BSD 2-Clause License|
|WebView2 |duilib/third_party/<br>Microsoft.Web.WebView2| Supports the WebView2 control |[Microsoft.Web.WebView2.LICENSE.txt](licenses/Microsoft.Web.WebView2.LICENSE.txt)|BSD 3-Clause License|
|libpag |duilib/third_party/libpag | Supports PAG animation files<br>(this feature is disabled by default; see subsequent documentation) |[libpag.LICENSE.txt](licenses/libpag.LICENSE.txt)|Apache License Version 2.0 (main body)<br>The third-party components that libpag depends on have<br>many different licenses; see the directory:<br>`duilib/third_party/libpag/licenses`<br>for the files. If you mind libpag's license<br>(including the main body license / third-party component licenses),<br>you may leave libpag disabled.|

## UI Preview
The example programs written using this GUI library, demonstrating the display effects of each control, can be found in: [docs/Examples.en.md](docs/Examples.en.md) 

## About the PAG Animation File Format
* Currently the PAG animation file format is only supported on the Windows platform; other platforms are not yet supported
* The PAG animation file format support is disabled by default (because you need to compile libpag.lib and libpag.dll yourself and place them in the project for normal compilation and execution)
* How to enable the PAG animation format:    
(1) Open the [`msvc/PropertySheets/LibPagSettings.props`](msvc/PropertySheets/LibPagSettings.props) file with a text editor and change the `LibPagEnabled` variable's value to `1`    
(2) Compile the libpag library by referring to the following documentation: [`duilib/third_party/libpag/windows/libpag-build.md`](duilib/third_party/libpag/windows/libpag-build.md)     
* When building nim_duilib, you must use `build/duilib.sln` or `build/examples.sln` to build; other build methods are not supported.
* The main license of the libpag library is Apache License Version 2.0, and its dependent third-party components have many different licenses,<br>see the files in the directory: `duilib/third_party/libpag/licenses`.<br>If you mind libpag's license (including the main body license / third-party component licenses), you may leave libpag disabled.

## Programming Languages
- C/C++: The compiler needs to support C++20 (`main` branch)
- C/C++: The compiler needs to support C++17 (`develop-cpp17` branch; this branch is only for supporting the VS2017/VS2019 compilers on Windows; for other environments please use the main branch)

## Supported Operating Systems
- Windows: 7/10/11 and above
- Linux: OpenEuler, OpenKylin (openKylin), UbuntuKylin (Ubuntu Kylin), NeoKylin, UnionTech UOS, Ubuntu, Debian, Fedora, OpenSuse, etc.
- macOS: 12+
- FreeBSD

## Supported Compilers
- Visual Studio 2022/2026 (Windows)
- Visual Studio 2017/2019 (Windows; the compilers of these two versions are only supported by the `develop-cpp17` branch code, not by other branches; when using VS2017, the CEF module is not supported)
- LLVM (Windows)
- MinGW-W64: gcc/g++, clang/clang++ (Windows)
- gcc/g++ (Linux)
- clang/clang++ (Linux)
- clang/clang++ (macOS)
- clang/clang++ (FreeBSD)

## A. Build Process (Windows Platform)
### 1. Preparation: Install Required Software
1. Install python3 (the major version of python must be 3, and it needs to be added to the Path environment variable)    
(1) First install python3    
(2) Go to the directory where `python.exe` is located, make a copy of `python.exe` and rename it to `python3.exe`: ensure that `python3.exe` can be accessed from the command line arguments   
(3) Verify on the command line: `> python3.exe --version` can show the python version number     
2. Install Git For Windows: version 2.44 (other versions are also acceptable); git needs to be added to the Path environment variable, ensuring `git.exe` can be accessed from the command line arguments    
3. Install Visual Studio, and during installation pay attention to selecting the correct Windows SDK version    
   It is recommended to install the Windows 11 SDK, because the CEF module depends on the Windows 11 SDK; the Windows 10 SDK will cause CEF-related modules to fail to compile;    
   If you do not use the CEF feature, the Windows 10 SDK is also acceptable
4. Install LLVM: version 21.1.4 Win64 (other versions are also acceptable)    
(1) Installation directory: `C:\LLVM`    
(2) Note: If installed in another directory, the installation directory must not contain spaces, otherwise compilation will encounter problems.

### 2. Automatic Build Using Script (Recommended)
This script automatically completes the related source code download and build work.    
Select a working directory (note: the path must not contain spaces, otherwise the build script will fail), create a script `build.bat`, copy the prepared script below into it, and save the file.    
* For Visual Studio 2022/2026, the script file content is as follows:    
```
REM For Visual Studio 2022/2026
echo OFF
set retry_delay=10

:retry_clone_duilib
if not exist ".\nim_duilib\.git" (
    git clone https://github.com/rhett-lee/nim_duilib
) else (  
    git -C ./nim_duilib pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_duilib
)
if not exist ".\nim_duilib\.git" (
    echo clone duilib failed!
    exit /b 1
)
.\nim_duilib\build\build_duilib_all_in_one.bat
```
By default, the above script builds using the static runtime library (MT and MTd). If you need to use the dynamic runtime library (MD and MDd), append the `/MD` parameter to the last line of the above script, changing it to:    
`.\nim_duilib\build\build_duilib_all_in_one.bat /MD`    
Note 1: If nim_duilib is ultimately built as a DLL library, then the dynamic runtime library must be used.    
Note 2: If nim_duilib uses the dynamic runtime library, then the Skia library must also use the dynamic runtime library; for the build method, please refer to the documentation of the skia_compile repository.    
    
* For Visual Studio 2017/2019 (you need to use the develop-cpp17 branch code), the script file content is as follows:    
```
REM For Visual Studio 2017/2019
echo OFF
set retry_delay=10

:retry_clone_duilib
if not exist ".\nim_duilib\.git" (
    git clone https://github.com/rhett-lee/nim_duilib
) else (  
    git -C ./nim_duilib pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_duilib
)
if not exist ".\nim_duilib\.git" (
    echo clone duilib failed!
    exit /b 1
)

:retry_pull_duilib
git -C ./nim_duilib checkout develop-cpp17
git -C ./nim_duilib pull
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_pull_duilib
)
.\nim_duilib\build\build_duilib_all_in_one.bat
```
By default, the above script builds using the static runtime library (MT and MTd). If you need to use the dynamic runtime library (MD and MDd), append the `/MD` parameter to the last line of the above script, changing it to:    
`.\nim_duilib\build\build_duilib_all_in_one.bat /MD`    
Note 1: If nim_duilib is ultimately built as a DLL library, then the dynamic runtime library must be used.    
Note 2: If nim_duilib uses the dynamic runtime library, then the Skia library must also use the dynamic runtime library; for the build method, please refer to the documentation of the skia_compile repository.    
    
* After the script file is prepared, enter the command line console and run the script: 
```
.\build.bat
```
The compiled example programs are located in the bin directory.

### 3. Manual Build Process (Windows Platform)
1. Set the working directory: `D:\develop`    
2. Obtain the related code    
(1) `git clone https://github.com/rhett-lee/nim_duilib`      
(2) `git clone https://github.com/rhett-lee/skia_compile`    
(3) `git clone https://github.com/google/skia.git`  
3. Build the Skia source code    
(1) nim_duilib internally uses Skia as the UI rendering engine, so you need to build skia first; it is recommended to use LLVM for building, which runs smoothly    
(2) Follow the method in the [compile_skia_on_windows.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_windows.en.md) documentation in the skia_compile directory to build the skia-related .lib files      
4. If you are using Visual Studio 2017/2019, you need to use the develop-cpp17 branch code, and run the following command in the command line:    
   `git -C ./nim_duilib checkout develop-cpp17`
5. Build nim_duilib: enter the `build` directory, open `examples.sln` (if you are using Visual Studio 2017, you need to open `examples_vs2017.sln`), then you can build; the compiled example programs are located in the bin directory.
6. Notes on the CEF module:    
(1) The CEF module depends on the Windows 11 SDK; if a lower version SDK is used, there will be compilation errors.    
(2) The CEF module only supports Visual Studio 2019/2022/2026, and does not support Visual Studio 2017.    
(3) If the CEF module is not needed, it can be disabled by editing the `msvc\PropertySheets\CEFSettings.props` file and changing the value of `LibCefEnabled` to `0`.    
(4) After disabling the CEF module, you can use the `duilib_no_cef.sln` or `examples_no_cef.sln` project to build, thereby reducing the compilation of libCEF code.    
7. Notes on the WebView2 module:    
(1) If the WebView2 module is not needed, it can be disabled by editing the `msvc\PropertySheets\WebView2Settings.props` file and changing the value of `WebView2Enabled` to `0`.    
8. The nim_duilib library uses the static runtime library (/MT and /MTd) by default, and also supports the dynamic runtime library (/MD and /MDd). The switching method is as follows:    
(1) The runtime library used when building the Skia library must be the same as the one used by the nim_duilib library. For the Skia library's build method, please refer to the documentation of the skia_compile repository.    
(2) To switch the nim_duilib library to use the dynamic runtime library, run the following script:    
    `.\nim_duilib\msvc\PropertySheets\DuilibUseDynamicRuntime.bat`    
(3) To switch the nim_duilib library to use the static runtime library, run the following script:    
    `.\nim_duilib\msvc\PropertySheets\DuilibUseStaticRuntime.bat`    

## B. Build Process (Linux Platform)
### 1. Preparation: Install Required Software
For different operating system platforms, you can install the required software according to the following list.
| Operating System Platform | Desktop Type | Modules and Install Commands to Install (Required) | 
| :--- | :--- | :--- |
|OpenEuler |UKUI/DDE(X11) |`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel`| 
|OpenKylin(openKylin) | Wayland |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|UbuntuKylin(Ubuntu Kylin) | X11 |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-devlibxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|NeoKylin | X11 |`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|UnionTech UOS | X11 |`sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev`| 
|Ubuntu |GNOME(Wayland)|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip bzip2 libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Debian |GNOME(Wayland)|`sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev`| 
|Fedora |GNOME(Wayland)|`sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel`|
|OpenSuse |KDE(X11) |`sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel`|

### 2. Automatic Build Using Script (Recommended)
This script automatically completes the related source code download and build work.    
Select a working directory (note: the path must not contain spaces, otherwise the build script will fail), create a script `build.sh`, copy the prepared script below into it, and save the file.    
Then, in the console, add executable permission to the script file, and finally run the script: 
```
chmod +x build.sh
./build.sh
```

The script file content is as follows:    
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
The compiled example programs are located in the bin directory.    
Note: For the UOS system, you need to install the required development environment first, then install it; you can refer to the documentation: [compile_skia_on_uos.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_uos.en.md).

### 3. Manual Build Process (Linux Platform)
1. Set the working directory: `~/develop`    
2. Obtain the related code    
(1) `git clone https://github.com/rhett-lee/nim_duilib`      
(2) `git clone https://github.com/rhett-lee/skia_compile`    
(3) `git clone https://github.com/google/skia.git`  
(4) `git clone https://github.com/libsdl-org/SDL.git`    
3. Build the Skia library    

| Operating System Platform | Reference Documentation (Web Link) | Reference Documentation (Local File) |
| :--- | :--- |:--- |
|OpenEuler |[compile_skia_on_openeuler.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_openeuler.en.md)|[compile_skia_on_openeuler.en.md](../skia_compile/compile_skia_on_openeuler.en.md)|
|OpenKylin(openKylin) |[compile_skia_on_openkylin.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_openkylin.en.md)|[compile_skia_on_openkylin.en.md](../skia_compile/compile_skia_on_openkylin.en.md)|
|UbuntuKylin(Ubuntu Kylin) |[compile_skia_on_ubuntukylin.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_ubuntukylin.en.md)  |[compile_skia_on_ubuntukylin.en.md](../skia_compile/compile_skia_on_ubuntukylin.en.md)|
|NeoKylin |[compile_skia_on_neokylin.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_neokylin.en.md)  |[compile_skia_on_neokylin.en.md](../skia_compile/compile_skia_on_neokylin.en.md) |
|UnionTech UOS |[compile_skia_on_uos.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_uos.en.md)|[compile_skia_on_uos.en.md](../skia_compile/compile_skia_on_uos.en.md)|
|Ubuntu |[compile_skia_on_ubuntu.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_ubuntu.en.md) | [compile_skia_on_ubuntu.en.md](../skia_compile/compile_skia_on_ubuntu.en.md) |
|Debian |[compile_skia_on_debian.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_debian.en.md)  |[compile_skia_on_debian.en.md](../skia_compile/compile_skia_on_debian.en.md) |
|Fedora |[compile_skia_on_fedora.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_fedora.en.md)  |[compile_skia_on_fedora.en.md](../skia_compile/compile_skia_on_fedora.en.md)|
|OpenSuse |[compile_skia_on_opensuse.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_opensuse.en.md) | [compile_skia_on_opensuse.en.md](../skia_compile/compile_skia_on_opensuse.en.md) |

    Note: When compiling the skia source code, you should use LLVM for compilation, so that the program runs more smoothly.
4. Build the SDL library 
```
#!/bin/bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. Build nim_duilib
```
#!/bin/bash
cd ~/develop/nim_duilib/
chmod +x linux_build.sh
./linux_build.sh
```
After the build is complete, the executable file is generated in the bin directory.    
If you want to support CEF, you can refer to the relevant documentation [docs/CEF.en.md](docs/CEF.en.md).

## C. Build Process (macOS Platform)
### 1. Preparation: Install Required Software
After the system is installed, the work to be done is:    
#### Install Xcode Command Line Tools
```
xcode-select --install
```
Verify the installation:
```
clang++ --version
```
#### Install Homebrew
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
If it fails, you can look for other sources to install.    
Update Homebrew:    
```
brew update
```
#### Software Already Included in the System (No Need to Install)
`git make unzip python3`
#### Install cmake
```
brew install cmake
```
#### Install ninja
```
brew install ninja
```
#### Install gn (gn needs to be compiled from source)
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

### 2. Automatic Build Using Script (Recommended)
This script automatically completes the related source code download and build work.    
Select a working directory (note: the path must not contain spaces, otherwise the build script will fail), create a script `build.sh`, copy the prepared script below into it, and save the file.    
Then, in the console, add executable permission to the script file, and finally run the script: 
```
chmod +x build.sh
./build.sh
```

The script file content is as follows:    
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
The compiled example programs are located in the bin directory.    

### 3. Manual Build Process (macOS Platform)
1. Set the working directory: `~/develop`    
2. Obtain the related code    
(1) `git clone https://github.com/rhett-lee/nim_duilib`      
(2) `git clone https://github.com/rhett-lee/skia_compile`    
(3) `git clone https://github.com/google/skia.git`  
(4) `git clone https://github.com/libsdl-org/SDL.git`  
3. Build the Skia library    

| Operating System Platform | Reference Documentation (Web Link) | Reference Documentation (Local File) |
| :--- | :--- |:--- |
|macOS |[compile_skia_on_macos.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_macos.en.md) | [compile_skia_on_macos.en.md](../skia_compile/compile_skia_on_macos.en.md) |

    Note: When compiling the skia source code, you should use LLVM for compilation, so that the program runs more smoothly.
4. Build the SDL library 
```
#!/bin/bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. Build nim_duilib
```
#!/bin/bash
cd ~/develop/nim_duilib/
chmod +x macos_build.sh
./macos_build.sh
```
After the build is complete, the executable file is generated in the bin directory.    
If you want to support CEF, you can refer to the relevant documentation [docs/CEF.en.md](docs/CEF.en.md).

## D. Build Process (FreeBSD Platform)
### 1. Preparation: Install Required Software
```
sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2
```
### 2. Automatic Build Using Script (Recommended)
This script automatically completes the related source code download and build work.    
Select a working directory (note: the path must not contain spaces, otherwise the build script will fail), create a script `build.sh`, copy the prepared script below into it, and save the file.    
Then, in the console, add executable permission to the script file, and finally run the script: 
```
chmod +x build.sh
./build.sh
```

The script file content is as follows:    
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
The compiled example programs are located in the bin directory.

Note: The FreeBSD platform does not support CEF (Chromium Embedded Framework).
### 3. Manual Build Process (FreeBSD Platform)
1. Set the working directory: `~/develop`    
2. Obtain the related code    
(1) `git clone https://github.com/rhett-lee/nim_duilib`      
(2) `git clone https://github.com/rhett-lee/skia_compile`    
(3) `git clone https://github.com/google/skia.git`  
(4) `git clone https://github.com/libsdl-org/SDL.git`  
3. Build the Skia library    

| Operating System Platform | Reference Documentation (Web Link) | Reference Documentation (Local File) |
| :--- | :--- |:--- |
|FreeBSD |[compile_skia_on_freebsd.en.md](https://github.com/rhett-lee/skia_compile/blob/main/compile_skia_on_freebsd.en.md) | [compile_skia_on_freebsd.en.md](../skia_compile/compile_skia_on_freebsd.en.md) |

    Note: When compiling the skia source code, only LLVM compilation is supported.
4. Build the SDL library 
```
#!/usr/bin/env bash
cd ~/develop
cmake -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build ./SDL.build
cmake --install ./SDL.build
```
5. Build nim_duilib
```
#!/usr/bin/env bash
cd ~/develop/nim_duilib/
chmod +x ./build/freebsd_build.sh
./build/freebsd_build.sh
```
After the build is complete, the executable file is generated in the bin directory.    

## Development Plan
 - Continue to enrich the controls of the GUI library and improve its functionality
 - Continuous testing and improvement of the cross-platform (Windows/Linux/macOS/FreeBSD) window engine (based on [SDL3.0](https://www.libsdl.org/)) (currently relatively stable under X11/XWayland desktop environments, but has more issues under pure Wayland desktop environments)
 - Test the GUI library, discover and fix defects, and continuously improve the code

## AI-Assisted Development (Claude Code Integration)

nim_duilib provides AI-friendly documentation and skills that can be used with [Claude Code](https://docs.anthropic.com/en/docs/claude-code) to enable AI-assisted UI development.

### Feature Description
After registration, Claude Code can use the following nim_duilib-specific skills in **any project**:

| Command / Skill | Description |
| :--- | :--- |
| `/nim-init` | Initialize the nim_duilib development environment for the current project (copy the LLM reference documentation, update CLAUDE.md) |
| `/nim-duilib-create-window` | Create a new window (automatically generate C++ class + XML layout file) |
| `/nim-duilib-xml-layout` | Design XML UI layouts (templates such as forms, split panels, toolbars, card grids) |
| `/nim-duilib-add-control` | Add controls (XML snippets and C++ event binding code for 15+ controls) |
| `/nim-duilib-event-handler` | Event handling (XML inline events and C++ Attach binding) |
| `/nim-duilib-theme` | Theme customization (predefined colors, fonts, quick reference for 100+ common style classes) |
| `/nim-duilib-resource-pack` | Resource packaging and deployment (ZIP packaging, embedding into a single-file EXE release) |

### Quick Start

**Prerequisites:** [Claude Code](https://docs.anthropic.com/en/docs/claude-code) is installed

**Step 1: Register (One-Time Only)**

Run the following in the nim_duilib root directory:
```bash
# Windows (CMD / PowerShell)
.claude\register.bat

# Linux / macOS
bash .claude/register.sh
```
The registration script installs all skills to `~/.claude/skills/`, taking effect globally.

**Step 2: Use in Your Application Project**

Open Claude Code in any project that needs to use nim_duilib, and type:
```
/nim-init
```
The AI will automatically configure the LLM reference documentation and CLAUDE.md for the project. After that, you can directly instruct Claude to complete UI development using natural language, for example:
- "Create a settings window with a username input box and a save button"
- "Design a layout with a left navigation bar and a right content area"
- "Add a click event to this button"
- "Package the resources into a single EXE"

### Updating Skills
When the nim_duilib AI skill files (`.claude/skills/`) are updated, simply re-run the registration script:
```bash
cd nim_duilib
.claude\register.bat   # Windows
# bash .claude/register.sh  # Linux / macOS
```

### Unregister
To remove all global skills:
```bash
bash nim_duilib/.claude/unregister.sh
```

### File Structure
```
nim_duilib/.claude/
├── register.bat / register.ps1 / register.sh   # Global registration script
├── unregister.sh                                # Unregister script
├── docs/
│   └── nim-duilib-llm-reference.md              # Complete LLM reference manual
└── skills/                                      # AI skill definitions
    ├── nim-duilib-create-window.md
    ├── nim-duilib-xml-layout.md
    ├── nim-duilib-add-control.md
    ├── nim-duilib-event-handler.md
    ├── nim-duilib-theme.md
    └── nim-duilib-resource-pack.md
```

## Reference Documentation

 - [Quick Start](docs/Getting-Started.en.md)
 - [Example Programs](docs/Examples.en.md)
 - [Global Styles: Fonts, Colors, Images, and Other Resources](docs/Global.en.md)
 - [Window Styles](docs/Window.en.md)
 - [Container Styles](docs/Box.en.md)
 - [Control Styles](docs/Control.en.md)
 - [Menu Styles](docs/Menu.en.md)
 - [Modal Message Box (MessageBoxWnd)](docs/MessageBox.en.md)
 - [Non-Modal Toast Notification (ToastWnd)](docs/Toast.en.md)
 - [Control Events/Messages](docs/Events.en.md)
 - [Node Names of Each Control in XML Files](docs/XmlNode.en.md)
 - [Responding to Control Events Directly in XML Files](docs/XmlEvents.en.md)
 - [CEF Control](docs/CEF.en.md)
 - [Project Build Documentation and Scripts](build/build.md)
 - [Reference Documentation](docs/Summary.en.md)

## Related Links
1. The Skia build documentation repository, click to visit: [skia_compile](https://github.com/rhett-lee/skia_compile):    
2. This project is developed directly on top of the NIM_Duilib_Framework project. Project address: [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)
3. The NIM_Duilib_Framework project is developed based on duilib. Project address: [duilib](https://github.com/duilib/duilib)
