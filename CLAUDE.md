# nim_duilib - 跨平台 C++ UI 库

## 项目概述
nim_duilib 是基于 Skia 渲染引擎的跨平台 C++ UI 框架，采用 XML 描述界面布局 + C++ 控制逻辑的开发模式。
- **支持平台**: Windows (7/10/11+), Linux, macOS (12+), FreeBSD
- **渲染引擎**: Skia (CPU/OpenGL)
- **构建工具**: CMake + Visual Studio / GCC / Clang
- **C++ 标准**: C++17+

## 项目结构
```
nim_duilib/
├── duilib/              # 核心库源码
│   ├── Core/            # 窗口、控件基类、事件、管理器
│   ├── Control/         # UI控件（Button, Label, RichEdit, TreeView...）
│   ├── Box/             # 容器控件（VBox, HBox, ListBox, TabBox...）
│   ├── Layout/          # 布局引擎（HLayout, VLayout, GridLayout...）
│   ├── Animation/       # 动画系统
│   ├── Image/           # 图片处理（PNG/SVG/GIF/WEBP/APNG/Lottie/PAG）
│   ├── Render/          # 渲染接口
│   ├── RenderSkia/      # Skia渲染实现
│   ├── Utils/           # 工具类（WindowImplBase, FilePath...）
│   ├── CEFControl/      # CEF浏览器集成
│   └── WebView2/        # WebView2控件
├── examples/            # 21个示例程序
├── docs/                # 完整文档
├── bin/resources/       # 主题资源（XML布局、图片、字体）
├── build/               # 构建脚本和解决方案
└── cmake/               # CMake配置
```

## 开发模式（XML + C++）

### XML 布局文件
- 位置: `bin/resources/themes/default/<skin_folder>/<skin_file>.xml`
- 全局资源: `bin/resources/themes/default/global.xml`（字体、颜色、通用样式）
- 编码: UTF-8

### C++ 代码三件套
每个窗口通常需要三个文件:
1. **MainThread** - 继承 `ui::FrameworkThread`，负责初始化和创建窗口
2. **MainForm.h/cpp** - 继承 `ui::WindowImplBase`，实现窗口逻辑
3. **XML布局文件** - 描述界面结构和样式

### 关键代码模式

**初始化全局资源:**
```cpp
ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
resourcePath += _T("resources\\");
ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
```

**创建窗口:**
```cpp
MainForm* window = new MainForm();
window->CreateWnd(nullptr, ui::WindowCreateParam(_T("WindowTitle"), true));
window->PostQuitMsgWhenClosed(true);
window->ShowWindow(ui::kSW_SHOW_NORMAL);
```

**查找控件:**
```cpp
ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_name")));
```

**事件绑定:**
```cpp
btn->AttachClick([this](const ui::EventArgs& args) {
    // 处理点击事件
    return true;
});
```

## 文档参考
- 完整文档: `docs/Summary.md`（文档索引）
- 控件属性: `docs/Control.md`
- 容器/布局: `docs/Box.md`
- 全局资源: `docs/Global.md`
- 窗口属性: `docs/Window.md`
- 事件系统: `docs/Events.md`
- XML事件: `docs/XmlEvents.md`
- XML节点名: `docs/XmlNode.md`
- LLM详细参考: `.claude/docs/nim-duilib-llm-reference.md`

## 编码规范
- 字符串使用 `DString` 类型，字面量用 `_T("...")` 宏包裹
- 控件查找用 `FindControl(_T("name"))`，需 `dynamic_cast` 到具体类型
- 事件回调返回 `true` 表示已处理
- XML属性值中内嵌引号用单引号`'`或花括号`{}`代替双引号
- 控件类支持模板变体: `Label`(Control基)、`LabelBox`(Box基)、`LabelHBox`(HBox基)、`LabelVBox`(VBox基)
- 窗口析构由框架管理，使用 `new` 创建，不需要手动 `delete`

## 构建
- Windows: 打开 `build/examples.sln`，选择 Debug|x64 或 Release|x64
- 跨平台: `build/build_duilib_all_in_one.sh` 或 `build/build_duilib_all_in_one.bat`
- 依赖: 需要先编译 Skia（参考 `build/build.md`）
