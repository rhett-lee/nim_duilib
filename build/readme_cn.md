# nim_duilib MSVC 工程与编译脚本说明

本文档详细介绍 `nim_duilib\build` 目录中 Visual Studio 解决方案文件（*.sln）和编译脚本文件（*.bat/*.sh）的用途及使用方法，供维护人员参考。

## 目录文件概览

```
build/
├── 解决方案文件（Visual Studio）
│   ├── duilib.sln                   # 基础解决方案（含lib库，不含示例程序）
│   ├── duilib_no_cef.sln            # 基础解决方案（不含CEF）
│   ├── examples.sln                 # 完整解决方案（含lib库和示例程序）
│   └── examples_no_cef.sln          # 完整解决方案（不含CEF）
│
├── 编译脚本（Windows/Batch）
│   ├── detect_vs_version.bat         # 自动检测VS版本
│   ├── msvc_build.bat                # CMake + MSVC 编译脚本
│   ├── build_duilib_all_in_one.bat   # 一键编译脚本（MSVC + LLVM）
│   ├── gcc-mingw-w64_build.bat       # CMake + MinGW-w64 gcc 编译脚本
│   ├── llvm-mingw-w64_build.bat      # CMake + MinGW-w64 clang 编译脚本
│   └── build_duilib_all_in_one_mingw-w64.bat  # 一键编译脚本（MinGW-w64）
│
├── 编译脚本（Linux/macOS/其他平台）
│   ├── build_duilib_all_in_one.sh   # Linux/macOS/FreeBSD 一键编译脚本
│   ├── linux_build.sh               # Linux 系统编译脚本
│   ├── msys2_build.sh               # MSYS2 系统编译脚本
│   ├── macos_build.sh               # macOS 系统编译脚本
│   └── freebsd_build.sh             # FreeBSD 系统编译脚本
│
└── build.md                         # 项目编译相关文档
```

---

## 解决方案文件详解

### 1. duilib.sln

**功能：** Visual Studio 基础解决方案文件，包含 duilib 核心库工程。

**包含内容：**
- duilib 核心库工程
- 第三方依赖库工程（zlib、libpng、cximage、libwebp）

**特点：**
- 不包含示例程序
- 包含 CEF 和 WebView2 支持
- 需要预先下载并编译 Skia

**适用场景：**
- 仅使用 duilib 核心库进行开发
- 不需要参考示例程序

---

### 2. duilib_no_cef.sln

**功能：** 不包含 CEF 模块的基础解决方案。

**与 duilib.sln 的区别：**
- 不支持 CEF（libCEF）模块
- WebView2 支持仍可用

---

### 3. examples.sln

**功能：** 完整的示例程序解决方案，包含 duilib 库和所有示例程序。

**包含内容：**
- duilib 核心库
- 第三方依赖库
- 完整示例程序集：
  - `basic` - 基础示例
  - `controls` - 控件示例
  - `ColorPicker` - 颜色选择器
  - `DpiAware` - DPI 感知示例
  - `chat` - 聊天界面示例
  - `layout` - 布局示例
  - `ListBox` - 列表框示例
  - `ListCtrl` - 列表控件示例
  - `MoveControl` - 移动控件示例
  - `MultiLang` - 多语言示例
  - `render` - 渲染示例
  - `RichEdit` - 富文本编辑器示例
  - `VirtualListBox` - 虚拟列表框示例
  - `threads` - 多线程示例
  - `TreeView` - 树形视图示例
  - `cef` - CEF 浏览器示例
  - `CefBrowser` - CEF 浏览器完整示例
  - `WebView2` - WebView2 示例
  - `WebView2Browser` - WebView2 浏览器完整示例
  - `ChildWindow` - 子窗口示例
  - `XmlPreview` - XML 预览示例

**注意：** CEF 模块需要手动下载配置，参见 [docs/CEF.md](../docs/CEF.md)。

---

### 4. examples_no_cef.sln

**功能：** 完整的示例程序解决方案，不包含 CEF 相关示例。

**与 examples.sln 的区别：**
- 不包含 `cef` 和 `CefBrowser` 示例程序
- 其他所有示例程序和功能保持一致

---

## 编译脚本详解

### 1. detect_vs_version.bat（VS 版本检测）

**功能：** 自动检测系统中安装的 Visual Studio 版本。

**检测优先级：**
1. **优先使用 vswhere.exe**：通过 VS 安装工具获取最新版本
2. **目录遍历备选**：按版本从高到低遍历常见安装目录

**检测的 VS 版本：**
- VS2026 (v145)
- VS2022 (v143)
- VS2019 (v142)
- VS2017 (v141)

**输出变量：**
| 变量名 | 说明 |
|--------|------|
| `VS_VERSION` | VS 版本标识（如 vs2022） |
| `VS_PATH` | VS 安装路径 |
| `VS_MAJOR_VERSION` | 主版本号（如 17） |

**使用方式：**
```batch
call detect_vs_version.bat
echo Detected: %VS_VERSION% (%VS_MAJOR_VERSION%)
```

---

### 2. msvc_build.bat（CMake + MSVC 编译）

**功能：** 使用 CMake 和 MSVC 编译器在命令行环境下编译项目。

**前置条件：**
- Visual Studio 2022 (17.0) 或更高版本
- CMake 3.21+（如使用 VS2026 则需 CMake 4.2+）
- 预先下载并编译 Skia

**使用方法：**
```batch
cd nim_duilib\build
msvc_build.bat
```

**编译内容：**
1. 第三方库：zlib、libpng、cximage、libwebp、libcef
2. duilib 核心库
3. 所有示例程序

**Skia 路径要求：**
- 期望 Skia 编译输出在 `..\..\skia\out\llvm.x64.release`

---

### 3. build_duilib_all_in_one.bat（一键编译 - MSVC）

**功能：** 全自动编译脚本，自动下载所有依赖项（包括 Skia）并完成编译。

**前置条件：**
- git.exe
- python3.exe
- LLVM Clang 编译器（位于 `C:\LLVM\bin`）
- Visual Studio 2022 (17.0) 或更高版本

**使用方法：**
```batch
cd nim_duilib\build
build_duilib_all_in_one.bat        # 静态运行库（/MT）
build_duilib_all_in_one.bat /MD    # 动态运行库（/MD）
```

**运行流程：**
1. 检测必要软件（git、python3、clang）
2. 检测 VS 版本
3. 克隆 nim_duilib 仓库（如不存在）
4. 克隆 skia_compile 工具仓库
5. 克隆 skia 仓库
6. 应用 Skia 补丁
7. 编译 Skia（Debug/Release x64/x86）
8. 设置 duilib 运行库模式
9. 使用 devenv 编译整个解决方案

**编译输出目录：**
| 平台 | 输出目录 |
|------|----------|
| x64 Release | `..\bin\` |
| x64 Debug | `..\bin\` |
| x86 Release | `..\bin\` |
| x86 Debug | `..\bin\` |

**Skia 编译选项：**
```
llvm.x64.debug
llvm.x64.release
llvm.x86.debug
llvm.x86.release
```

---

### 4. gcc-mingw-w64_build.bat（CMake + MinGW gcc）

**功能：** 使用 CMake 和 MinGW-w64 GCC 编译器编译项目。

**前置条件：**
- MinGW-w64 GCC/G++ 编译器
- CMake 3.21+

**使用方法：**
```batch
cd nim_duilib\build
gcc-mingw-w64_build.bat        # 不启用 SDL
gcc-mingw-w64_build.bat -sdl   # 启用 SDL
```

**特点：**
- 不支持 CEF 模块
- 支持 WebView2
- 支持 SDL 可选功能

---

### 5. llvm-mingw-w64_build.bat（CMake + MinGW clang）

**功能：** 使用 CMake 和 MinGW-w64 Clang/LLVM 编译器编译项目。

**前置条件：**
- MinGW-w64 Clang/LLVM 编译器
- CMake 3.21+

**使用方法：**
```batch
cd nim_duilib\build
llvm-mingw-w64_build.bat        # 不启用 SDL
llvm-mingw-w64_build.bat -sdl   # 启用 SDL
```

**特点：**
- 使用 Clang 编译器，性能更好
- 不支持 CEF 模块
- 支持 WebView2
- 支持 SDL 可选功能

---

### 6. build_duilib_all_in_one_mingw-w64.bat（一键编译 - MinGW）

**功能：** 全自动编译脚本，使用 MinGW-w64 编译器自动下载依赖并编译。

**前置条件：**
- git.exe
- python3.exe
- cmake.exe
- GCC/G++ 或 Clang/Clang++ 编译器（二选一）

**使用方法：**
```batch
cd nim_duilib\build
build_duilib_all_in_one_mingw-w64.bat      # 不启用 SDL
build_duilib_all_in_one_mingw-w64.bat -sdl # 启用 SDL
```

**运行流程：**
1. 检测必要软件（git、python3、cmake、gcc/clang）
2. 克隆 nim_duilib 仓库
3. 克隆 skia_compile 工具仓库
4. 克隆 skia 仓库
5. 应用 Skia 补丁
6. 编译 Skia（根据编译器选择 gcc 或 clang 版本）
7. 可选：克隆并编译 SDL3
8. 调用对应的 MinGW 编译脚本

---

## 编译配置对比

| 特性 | MSVC | MinGW GCC | MinGW Clang |
|------|------|-----------|-------------|
| 编译工具 | devenv.exe | mingw32-make.exe | mingw32-make.exe |
| CEF 支持 | ✅ 支持 | ❌ 不支持 | ❌ 不支持 |
| WebView2 支持 | ✅ 支持 | ✅ 支持 | ✅ 支持 |
| SDL 支持 | ✅ 可选 | ✅ 可选 | ✅ 可选 |
| 静态运行库 | ✅ /MT | ✅ | ✅ |
| 动态运行库 | ✅ /MD | ✅ | ✅ |
| 一键编译脚本 | ✅ | ❌ | ❌ |

---

## 快速开始

### 方式一：使用 Visual Studio IDE

1. **仅编译库**：
   ```
   双击打开 duilib.sln 或 duilib_no_cef.sln
   选择配置（Debug/Release）和平台（x64/Win32）
   生成 → 重新生成解决方案
   ```

2. **编译完整示例**：
   ```
   双击打开 examples.sln 或 examples_no_cef.sln
   选择配置和平台
   生成 → 重新生成解决方案
   ```

### 方式二：使用一键编译脚本（推荐新手）

```batch
cd nim_duilib\build
build_duilib_all_in_one.bat
```

脚本会自动处理：
- 下载所有依赖
- 编译 Skia
- 编译 duilib 和所有示例

### 方式三：使用 CMake 命令行编译

```batch
cd nim_duilib\build
msvc_build.bat
```

---

## 编译输出说明

### 输出目录结构

```
nim_duilib/
├── bin/                         # 编译输出目录
│   ├── duilib.dll               # duilib DLL文件
│   ├── basic.exe                # 示例程序
│   ├── controls.exe
│   └── ...
│
├── lib/                         # 静态库目录
│   ├── x64/
│   │   ├── duilib.lib
│   │   └── ...
│   └── Win32/
│       └── ...
│
├── build_temp/                   # 编译临时目录（可安全删除）
│   ├── msvc/
│   └── ...
│
└── .vs/                          # VS 缓存目录（可定期清理）
```

### 编译目标文件说明

| 文件类型 | 路径 | 说明 |
|----------|------|------|
| 动态库 | `bin\duilib.dll` | duilib 动态链接库 |
| 静态库 | `lib\x64\duilib.lib` | duilib 静态库 |
| 可执行程序 | `bin\*.exe` | 示例程序 |
| 资源文件 | `bin\resources\` | UI 资源文件 |

---

## 常见问题排查

### Q1：编译报 "Visual Studio 2022 or newer is required"

**原因：** 检测到系统安装的 VS 版本低于 2022。

**解决方案：**
1. 安装 Visual Studio 2022 或更高版本
2. 确保使用 `detect_vs_version.bat` 正确检测到新版本

---

### Q2：编译报 "Skia not found"

**原因：** Skia 未编译或路径不正确。

**解决方案：**
1. 使用 `build_duilib_all_in_one.bat` 自动下载并编译 Skia
2. 或手动编译 Skia，确保输出在 `..\..\skia\out\llvm.x64.release`

---

### Q3：编译报 "clang.exe not found"

**原因：** LLVM Clang 未安装或不在 PATH 中。

**解决方案：**
1. 安装 LLVM Clang（推荐版本 16+）
2. 确保 `C:\LLVM\bin` 在系统 PATH 中
3. 或修改 `build_duilib_all_in_one.bat` 中的 LLVM 路径

---

### Q4：编译报 "CMake version is too old"

**原因：** CMake 版本低于要求。

**解决方案：**
- MSVC 编译需要 CMake 3.21+（VS2026 需要 4.2+）
- 下载最新 CMake：https://cmake.org/download/

---

### Q5：CEF 示例程序无法运行

**原因：** CEF 运行时文件未正确配置。

**解决方案：**
1. 手动下载 libCEF 并配置，参见 [docs/CEF.md](../docs/CEF.md)
2. 将 libcef.dll 和相关资源复制到 `bin\libcef_win` 或 `bin\libcef_win_109`

---

## 维护注意事项

### 1. 清理编译缓存

编译过程中产生的临时文件可以安全清理：

```batch
# 清理 VS 缓存
rmdir /s /q nim_duilib\build\.vs

# 清理编译临时文件
rmdir /s /q nim_duilib\build\build_temp

# 清理 CEF 运行时缓存
rmdir /s /q nim_duilib\cef_cache

# 清理 WebView2 缓存
rmdir /s /q nim_duilib\webview2_cache
```

### 2. 切换运行库模式

**静态运行库（/MT）：**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseStaticRuntime.bat
```

**动态运行库（/MD）：**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseDynamicRuntime.bat
```

**重要：** 切换后必须关闭 VS 并重新打开解决方案。

### 3. 添加新的示例程序

1. 在 `examples\` 目录下创建新的工程
2. 编辑示例的 `.vcxproj` 文件，添加属性表引用：
   ```xml
   <Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettings.props" />
   ```
3. 将新工程添加到 `examples.sln` 和 `examples_no_cef.sln`

### 4. Skia 编译版本要求

duilib 依赖特定版本的 Skia，编译输出目录必须为：
```
skia/out/
├── llvm.x64.debug/
├── llvm.x64.release/
├── llvm.x86.debug/
├── llvm.x86.release/
├── mingw64-gcc.x64.release/   # MinGW GCC 编译
├── mingw64-llvm.x64.release/  # MinGW Clang 编译
└── ...
```

### 5. 支持的编译器版本

| 编译器 | 最低版本 | 推荐版本 |
|--------|----------|----------|
| Visual Studio | VS2022 (17.0) | VS2022 / VS2026 |
| LLVM Clang    |      | 16.0+ |
| MinGW GCC     |      | 13.2+ |
| MinGW Clang   |      | 16.0+ |

备注：VS2017/VS2019在develop-cpp17分支代码中支持，但主分支不支持。

---

## 版本兼容性矩阵

| 功能特性 | MSVC + VS2022/2026 | MinGW GCC | MinGW Clang | Linux GCC/Clang | macOS Clang |
|----------|-------------------|-----------|-------------|-----------------|-------------|
| 核心库 | ✅ | ✅ | ✅ | ✅ | ✅ |
| Skia 渲染 | ✅ | ✅ | ✅ | ✅ | ✅ |
| CEF 浏览器 | ✅ | ❌ | ❌ | ✅ | ✅ |
| WebView2 | ✅ | ✅ | ✅ | ❌ | ❌ |
| SDL 输入 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 静态运行库 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 动态运行库 | ✅ | ❌ | ❌ | ❌ | ❌ |
| C++20 标准 | ✅ (VS2022+) | ✅ | ✅ | ✅ | ✅ |
| C++17 标准 | ✅ (VS2017/VS2019) | ✅ | ✅ | ✅ | ✅ |

备注：VS2017/VS2019在develop-cpp17分支代码中支持，但主分支不支持。
