---
name: nim-duilib-resource-pack
description: nim_duilib 资源打包与部署（ZIP打包、嵌入EXE、单文件发布），配置资源加载方式
---

# nim_duilib 资源打包与部署

## 三种资源加载模式

### 模式 1：本地文件夹（开发时推荐）
```cpp
ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
resourcePath += _T("resources\\");
ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
```
目录结构：
```
MyApp.exe
resources/
├── themes/default/
│   ├── global.xml
│   ├── public/...
│   └── my_app/...
├── fonts/...
└── lang/...
```

### 模式 2：ZIP 压缩包（发布时推荐）
```cpp
ui::ZipFileResParam resParam;
resParam.resourcePath = _T("resources\\");     // ZIP 内的相对路径
resParam.zipFilePath = ui::FilePathUtil::GetCurrentModuleDirectory();
resParam.zipFilePath += _T("resources.zip");   // ZIP 文件路径
resParam.zipPassword = _T("");                 // 可选密码
ui::GlobalManager::Instance().Startup(resParam);
```
发布文件：
```
MyApp.exe
resources.zip        # 包含 resources/ 目录结构
```

### 模式 3：嵌入 EXE 资源（单文件发布，仅 Windows）
```cpp
#include "resource.h"

ui::ResZipFileResParam resParam;
resParam.resourcePath = _T("resources\\");
resParam.hResModule = nullptr;                        // nullptr = 当前EXE
resParam.resourceName = MAKEINTRESOURCE(IDR_THEME);   // 资源ID
resParam.resourceType = _T("THEME");                  // 资源类型名
resParam.zipPassword = _T("");
ui::GlobalManager::Instance().Startup(resParam);
```

## 实现单 EXE 发布（模式 3 详细步骤）

### 步骤 1：创建 resource.h
```cpp
// resource.h
#ifndef RESOURCE_H_
#define RESOURCE_H_

#define IDR_THEME  101

#endif // RESOURCE_H_
```

### 步骤 2：创建 .rc 资源文件
```rc
// MyApp.rc
#include "resource.h"
IDR_THEME  THEME  "..\\..\\bin\\resources.zip"
```
**注意**：路径是相对于 .rc 文件的位置。

### 步骤 3：制作 resources.zip
使用 7-Zip 打包（推荐参数，确保 UTF-8 文件名）：
```bash
cd bin
7z a -tzip -mcu=on resources.zip resources/
```

**ZIP 要求：**
- 压缩算法：仅支持 Deflate（不支持 Deflate64）
- 文件名编码：UTF-8（7-Zip 用 `-mcu=on` 参数）
- 密码加密：仅支持 ZipCrypto（ZIP 传统加密）

### 步骤 4：在 MainThread 中切换加载模式
```cpp
void MainThread::OnInit()
{
#ifdef NDEBUG
    // Release：使用嵌入 EXE 的 ZIP 资源
    ui::ResZipFileResParam resParam;
    resParam.resourcePath = _T("resources\\");
    resParam.hResModule = nullptr;
    resParam.resourceName = MAKEINTRESOURCE(IDR_THEME);
    resParam.resourceType = _T("THEME");
    resParam.zipPassword = _T("");
    ui::GlobalManager::Instance().Startup(resParam);
#else
    // Debug：使用本地文件夹（方便修改和调试）
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
#endif

    // 创建窗口...
}
```

### 步骤 5：添加 .rc 到工程
- Visual Studio：右键项目 → 添加 → 现有项 → 选择 .rc 文件
- CMakeLists.txt：
```cmake
# Windows 平台添加资源文件
if(WIN32)
    target_sources(MyApp PRIVATE MyApp.rc)
endif()
```

## 打包清单：应该包含什么

### 必须打包的资源
```
resources/
├── themes/default/
│   ├── global.xml                  # 必须
│   ├── public/                     # 必须（全部 133 个文件）
│   │   ├── button/                 # 窗口按钮 SVG
│   │   ├── caption/                # 标题栏图标
│   │   ├── checkbox/               # 复选框图标
│   │   ├── combo/                  # 下拉框图标
│   │   ├── option/                 # 单选按钮图标
│   │   ├── scrollbar01/            # 滚动条资源
│   │   ├── scrollbar02/            # 滚动条资源
│   │   ├── shadow/                 # 窗口阴影
│   │   ├── slider/                 # 滑块资源
│   │   ├── tooltip/                # 提示框
│   │   ├── tree/                   # 树控件图标
│   │   ├── menu/                   # 菜单资源
│   │   ├── progress/               # 进度条
│   │   ├── animation/              # 加载动画 JSON
│   │   └── ...
│   └── my_app/                     # 你的应用 XML 和图片
│       ├── main_form.xml
│       └── ...
├── fonts/                          # 可选：自定义字体
└── lang/                           # 可选：多语言文件
```

### 严禁打包的内容
| 不要打包 | 原因 |
|---------|------|
| themes/default/basic/ | 示例程序目录 |
| themes/default/controls/ | 示例程序目录 |
| themes/default/layout/ | 示例程序目录 |
| themes/default/render/ | 示例程序目录 |
| themes/default/chat/ | 示例程序目录 |
| themes/default/cef/ | 示例程序目录 |
| themes/default/cef_browser/ | 示例程序目录 |
| themes/default/webview2/ | 示例程序目录 |
| themes/default/webview2_browser/ | 示例程序目录 |
| themes/default/list_box/ | 示例程序目录 |
| themes/default/list_ctrl/ | 示例程序目录 |
| themes/default/tree_view/ | 示例程序目录 |
| themes/default/rich_edit/ | 示例程序目录 |
| themes/default/color_picker/ | 示例程序目录 |
| themes/default/dpi_aware/ | 示例程序目录 |
| themes/default/move_control/ | 示例程序目录 |
| themes/default/threads/ | 示例程序目录 |
| themes/default/virtual_list_box/ | 示例程序目录 |
| themes/default/child_window/ | 示例程序目录 |
| themes/default/xml_preview/ | 示例程序目录 |
| themes/default/MultiLang/ | 示例程序目录 |
| bin/*.exe, bin/*.dll | 编译产物 |
| bin/bin.zip | 编译产物压缩包 |

**规则：只打包 global.xml + public/ + 你自己的应用目录 + fonts/(可选) + lang/(可选)**

## 跨平台注意事项

| 平台 | 支持的资源模式 |
|------|--------------|
| Windows | 本地文件 / ZIP文件 / 嵌入EXE(单文件) |
| Linux | 本地文件 / ZIP文件 |
| macOS | 本地文件 / ZIP文件 |
| FreeBSD | 本地文件 / ZIP文件 |

macOS/Linux 不支持嵌入EXE模式（无 Windows RC 资源机制），发布时使用 ZIP 文件模式。
