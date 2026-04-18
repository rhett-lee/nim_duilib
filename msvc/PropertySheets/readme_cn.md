# nim_duilib MSVC 属性配置说明

本文档详细介绍 `nim_duilib\msvc\PropertySheets` 目录中 VS 属性配置文件（*.props）和脚本文件（*.bat）的用途及使用方法，供维护人员参考。

## 目录文件概览

```
PropertySheets/
├── CommonSettings.props           # 基础配置（必选）
├── DuilibSettings.props           # duilib 库自身配置
├── BinCommonSettings.props        # 可执行程序公共配置
├── BinCommonSettingsCEF.props     # CEF 支持的可执行程序配置
├── BinCommonSettingsWebView2.props # WebView2 支持的可执行程序配置
├── BinOutSettings.props           # 输出目录配置
├── BinManifestSettings.props      # Manifest 清单配置
├── SDLSettings.props              # SDL3 支持配置
├── SkiaSettings.props             # Skia 图形库配置（必选）
├── JpegTurboSettings.props        # libjpeg-turbo 配置
├── LibPagSettings.props           # libpag 配置
├── CEFSettings.props              # libCEF 配置
├── WebView2Settings.props         # WebView2 配置
├── DuilibUseDynamicRuntime.bat    # 切换到动态运行库模式
└── DuilibUseStaticRuntime.bat     # 切换到静态运行库模式
```

## 属性配置文件详解

### 1. CommonSettings.props（基础配置）

**功能：** 所有工程共享的基础配置，包含编译器和链接器的基本设置。

**主要内容：**

- **平台工具集选择**：根据 VS 版本自动选择对应的工具集
  - VS2022 (17.0) → v143
  - VS2019 (16.0) → v142
  - VS2017 (15.0) → v141

- **SDK 版本配置**：Windows 10.0 目标平台

- **C++ 标准配置**：
  - VS2022/VS2026：使用 C++20 标准
  - VS2019/VS2017：使用 C++17 标准

- **运行库配置**：通过检测 `duilib_dll.flag` 文件自动切换 MT/MTd 或 MD/MDd 模式

- **基础库列表**：
  ```xml
  duilib.lib / duilib_d.lib          # 主库
  zlib.lib / zlib_d.lib              # 压缩库
  cximage.lib / cximage_d.lib        # 图像处理库
  libpng.lib / libpng_d.lib          # PNG 图像库
  libwebp.lib / libwebp_d.lib        # WebP 图像库
  ```

**重要变量：**
- `EnableDuilibDll`：是否使用 DLL 模式（1=DLL，0=静态库）
- `DuilibSystemLibs`：系统依赖库列表
- `DuilibThirdLibs`：第三方依赖库列表
- `DuilibMainLib`：duilib 主库名称

---

### 2. DuilibSettings.props（duilib 库自身配置）

**功能：** 专门用于编译 duilib 库本身的属性配置。

**导入关系：**
```
DuilibSettings.props
├── CommonSettings.props
├── SkiaSettings.props
├── SDLSettings.props
├── CEFSettings.props
├── WebView2Settings.props
├── JpegTurboSettings.props
└── LibPagSettings.props
```

**主要功能：**

- **工程类型切换**：根据 `EnableDuilibDll` 变量决定编译为 DLL 还是静态库
  ```xml
  <ConfigurationType>DynamicLibrary</ConfigurationType>  <!-- DLL模式 -->
  <ConfigurationType>StaticLibrary</ConfigurationType>   <!-- 静态库模式 -->
  ```

- **输出配置**：DLL 模式下输出到 `bin\` 目录，静态库模式下输出到 `lib\$(Platform)\` 目录

- **功能模块宏定义**：
  - `DUILIB_SDL`：SDL 支持开关
  - `DUILIB_WEBVIEW2`：WebView2 支持开关
  - `DUILIB_CEF`：CEF 支持开关
  - `DUILIB_JPEG_TURBO`：libjpeg-turbo 支持开关
  - `DUILIB_LIB_PAG`：libpag 支持开关

---

### 3. BinCommonSettings.props（可执行程序公共配置）

**功能：** 所有可执行程序（exe）的公共配置。

**导入关系：**
```
BinCommonSettings.props
├── CommonSettings.props
├── BinManifestSettings.props
├── BinOutSettings.props
├── SDLSettings.props
├── SkiaSettings.props
├── JpegTurboSettings.props
└── LibPagSettings.props
```

**主要功能：**

- **Include 目录配置**：
  ```xml
  $(SolutionDir)\..;$(ProjectDir)
  ```

- **库文件路径配置**：根据是否启用 DLL 模式选择不同的库路径

- **预处理器定义**：
  - `DUILIB_SDL`
  - `DUILIB_JPEG_TURBO`
  - `DUILIB_LIB_PAG`

---

### 4. BinCommonSettingsCEF.props（CEF 可执行程序配置）

**功能：** 需要支持 libCEF 的可执行程序的配置。

**导入关系：**
```
BinCommonSettingsCEF.props
└── BinCommonSettings.props（包含完整的公共配置）
    └── CEFSettings.props
```

**特殊配置：**
- **延迟加载 DLL**：`libcef.dll` 使用延迟加载机制
- **预处理器定义**：`DUILIB_CEF=$(LibCefEnabled)`

---

### 5. BinCommonSettingsWebView2.props（WebView2 可执行程序配置）

**功能：** 需要支持 WebView2 的可执行程序的配置。

**导入关系：**
```
BinCommonSettingsWebView2.props
└── BinCommonSettings.props（包含完整的公共配置）
    └── WebView2Settings.props
```

**特殊配置：**
- **预处理器定义**：`DUILIB_WEBVIEW2=$(WebView2Enabled)`

---

### 6. BinOutSettings.props（输出目录配置）

**功能：** 统一管理可执行程序的输出目录和中间目录。

**配置规则：**

| 配置/平台 | 输出目录 | 中间目录 | 目标名称 |
|-----------|----------|----------|----------|
| Release x64 | `..\bin\` | `build_temp\x64\$(ProjectName)\Release\` | `$(ProjectName)` |
| Debug x64 | `..\bin\` | `build_temp\x64\$(ProjectName)\Debug\` | `$(ProjectName)_d` |
| Release Win32 | `..\bin\` | `build_temp\x86\$(ProjectName)\Release\` | `$(ProjectName)32` |
| Debug Win32 | `..\bin\` | `build_temp\x86\$(ProjectName)\Debug\` | `$(ProjectName)32_d` |

---

### 7. SDLSettings.props（SDL3 支持配置）

**功能：** 配置 SDL3 图形/输入库的支持选项。

**配置项：**
- `SDLEnabled`：SDL 支持开关（0=禁用，1=启用）
- `SDLIncludeDir`：SDL include 目录（`$(SolutionDir)\..\..\SDL3\include`）
- `SDLLibDir`：SDL lib 目录（根据平台和配置选择）
- `SDLLibs`：SDL 依赖库列表（`SDL3-static.lib;Version.lib;Winmm.lib;Setupapi.lib`）

**宏定义：** `DUILIB_SDL=$(SDLEnabled)`

---

### 8. SkiaSettings.props（Skia 图形库配置）

**功能：** 配置 Skia 矢量图形渲染引擎的支持选项。

**配置项：**
- `SkiaPreprocessorDefinitions`：Skia 预处理器定义（`SK_GANESH;SK_GL;SK_RELEASE`）
- `SkiaIncludeDir`：Skia include 目录（`$(SolutionDir)\..\..\skia`）
- `SkiaLibDir`：Skia lib 目录（根据平台和配置选择 release/debug）
- `SkiaLibs`：Skia 依赖库列表（`skia.lib;svg.lib;skshaper.lib;skottie.lib;sksg.lib;jsonreader.lib`）

**注意：** Skia 是必选组件，所有配置都依赖 Skia。

---

### 9. JpegTurboSettings.props（libjpeg-turbo 配置）

**功能：** 配置 libjpeg-turbo JPEG 图像编解码库的支持选项。

**配置项：**
- `JpegTurboEnabled`：libjpeg-turbo 支持开关（0=禁用，1=启用）
- `JpegTurboIncludeDir`：libjpeg-turbo include 目录
- `JpegTurboLibDir`：libjpeg-turbo lib 目录
- `JpegTurboLibs`：依赖库列表（`turbojpeg-static.lib`）

**宏定义：** `DUILIB_JPEG_TURBO=$(JpegTurboEnabled)`

---

### 10. LibPagSettings.props（libpag 配置）

**功能：** 配置 libpag 动图编解码库的支持选项。

**配置项：**
- `LibPagEnabled`：libpag 支持开关（0=禁用，1=启用）
- `LibPagIncludeDir`：libpag include 目录
- `LibPagLibDir`：libpag lib 目录
- `LibPagLibs`：依赖库列表（`libpag.lib`）

**宏定义：** `DUILIB_LIB_PAG=$(LibPagEnabled)`

---

### 11. CEFSettings.props（libCEF 配置）

**功能：** 配置 libCEF Chromium Embedded Framework 的支持选项。

**配置项：**
- `LibCefEnabled`：libCEF 支持开关（0=禁用，1=启用）
- `LibCefVersion109`：版本选择（0=最新版本，1=109 版本）
  - 109 版本：支持 Win7 及以上系统
  - 最新版本：仅支持 Win10 及以上系统
- `LibCefSrcDir`：libCEF 源码子目录
- `LibCefDllWrapperName`：CEF 封装库名称
- `LibCefDllName`：CEF DLL 名称（`libcef.dll`）
- `LibCefIncludeDir`：libCEF include 目录
- `LibCefLibDir`：libCEF lib 目录
- `LibCefLibs`：依赖库列表

**宏定义：** `DUILIB_CEF=$(LibCefEnabled)`

---

### 12. WebView2Settings.props（WebView2 配置）

**功能：** 配置 Microsoft WebView2 控件的支持选项。

**配置项：**
- `WebView2Enabled`：WebView2 支持开关（0=禁用，1=启用）
- `WebView2LibDir`：WebView2 lib 目录
- `WebView2Libs`：依赖库列表（`WebView2LoaderStatic.lib`）

**宏定义：** `DUILIB_WEBVIEW2=$(WebView2Enabled)`

---

## 脚本文件说明

### 1. DuilibUseDynamicRuntime.bat（切换到动态运行库模式）

**功能：** 将 duilib 工程切换到动态运行库模式（MD/MDd）。

**操作步骤：**
1. 创建 `duilib_dll.flag` 文件
2. 提示用户关闭并重新打开 VS 工程

**影响范围：**
- `CommonSettings.props` 中 `EnableDuilibDll` 变量变为 1
- 所有工程使用 `/MD`（Release）或 `/MDd`（Debug）编译
- duilib 自身编译为 DLL 形式

---

### 2. DuilibUseStaticRuntime.bat（切换到静态运行库模式）

**功能：** 将 duilib 工程切换到静态运行库模式（MT/MTd）。

**操作步骤：**
1. 删除 `duilib_dll.flag` 文件
2. 提示用户关闭并重新打开 VS 工程

**影响范围：**
- `CommonSettings.props` 中 `EnableDuilibDll` 变量变为 0
- 所有工程使用 `/MT`（Release）或 `/MTd`（Debug）编译
- duilib 自身编译为静态库形式

---

## 使用指南

### 如何在工程中引用属性配置

#### 编译 duilib 库本身
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\DuilibSettings.props" />
```

#### 编译可执行程序（基础版本）
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettings.props" />
```

#### 编译可执行程序（支持 CEF）
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettingsCEF.props" />
```

#### 编译可执行程序（支持 WebView2）
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettingsWebView2.props" />
```

---

### 如何切换静态/动态运行库

1. **切换到动态运行库模式**：
   ```batch
   cd nim_duilib\msvc\PropertySheets
   DuilibUseDynamicRuntime.bat
   ```

2. **切换到静态运行库模式**：
   ```batch
   cd nim_duilib\msvc\PropertySheets
   DuilibUseStaticRuntime.bat
   ```

3. **重新加载工程**：
   - 关闭 Visual Studio
   - 重新打开解决方案文件（*.sln）
   - 等待属性重新加载完成

---

### 如何启用/禁用可选功能

编辑对应的配置文件，修改开关变量：

| 功能模块 | 配置文件 | 变量名 | 可选值 |
|----------|----------|--------|--------|
| SDL 支持 | SDLSettings.props | `SDLEnabled` | 0（禁用），1（启用） |
| libjpeg-turbo | JpegTurboSettings.props | `JpegTurboEnabled` | 0（禁用），1（启用） |
| libpag | LibPagSettings.props | `LibPagEnabled` | 0（禁用），1（启用） |
| libCEF | CEFSettings.props | `LibCefEnabled` | 0（禁用），1（启用） |
| libCEF 版本 | CEFSettings.props | `LibCefVersion109` | 0（最新），1（109） |
| WebView2 | WebView2Settings.props | `WebView2Enabled` | 0（禁用），1（启用） |

---

## 配置依赖关系图

```
┌─────────────────────────────────────────────────────────────┐
│                    DuilibSettings.props                      │
│                   (duilib 库自身配置)                         │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐   │
│  │   Common     │    │    Skia      │    │     SDL      │   │
│  │  Settings    │◄───│  Settings    │    │  Settings    │   │
│  └──────┬───────┘    └──────────────┘    └──────────────┘   │
│         │                     │                              │
│         │              ┌──────┴───────┐                     │
│         │              │  JpegTurbo   │                     │
│         │              │  Settings    │                     │
│         │              └──────────────┘                     │
│         │                                                   │
│         │              ┌──────────────┐                     │
│         │              │    LibPag    │                     │
│         │              │  Settings    │                     │
│         │              └──────────────┘                     │
│         │                                                   │
│         │              ┌──────────────┐                     │
│         │              │     CEF      │                     │
│         │              │  Settings    │                     │
│         │              └──────────────┘                     │
│         │                                                   │
│         │              ┌──────────────┐                     │
│         │              │   WebView2   │                     │
│         │              │  Settings    │                     │
│         │              └──────────────┘                     │
└─────────┼───────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────┐
│                  BinCommonSettings.props                     │
│                 (可执行程序公共配置)                          │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐   │
│  │   Common     │    │    BinOut    │    │   BinMani-   │   │
│  │  Settings    │    │  Settings    │    │   fest       │   │
│  └──────────────┘    └──────────────┘    └──────────────┘   │
│                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐   │
│  │     SDL      │    │    Skia      │    │  JpegTurbo   │   │
│  │  Settings    │    │  Settings    │    │  Settings    │   │
│  └──────────────┘    └──────────────┘    └──────────────┘   │
│                                                              │
│  ┌──────────────┐                                           │
│  │    LibPag    │                                           │
│  │  Settings    │                                           │
│  └──────────────┘                                           │
└─────────────────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────┐    ┌─────────────────────────────┐
│BinCommonSettingsCEF.props│    │BinCommonSettingsWebView2.props│
│ (CEF支持的可执行程序)    │    │ (WebView2支持的可执行程序)   │
├─────────────────────────┤    ├─────────────────────────────┤
│ BinCommonSettings.props  │    │  BinCommonSettings.props    │
│ └─ CEFSettings.props      │    │  └─ WebView2Settings.props   │
└─────────────────────────┘    └─────────────────────────────┘
```

---

## 维护注意事项

### 1. 配置文件修改后必须重新加载

修改任何 `.props` 文件后，必须**关闭 Visual Studio 并重新打开解决方案**，否则修改可能不会生效。

### 2. 运行库模式切换

- 切换静态/动态运行库模式后，**必须重新编译整个解决方案**
- 建议清理所有中间文件（build_temp 目录）和输出文件（bin、lib 目录）后再重新编译

### 3. 添加新的依赖库

如果需要添加新的第三方库依赖，建议：

1. 在 `PropertySheets` 目录下创建独立的 `XXXSettings.props` 配置文件
2. 在 `DuilibSettings.props` 和 `BinCommonSettings.props` 中添加 `Import` 语句
3. 配置对应的 Include 目录、Lib 目录、库文件列表等变量
4. 添加相应的宏定义控制开关

### 4. 路径变量约定

- `$(SolutionDir)`：指向 `.sln` 文件所在目录
- `$(ProjectDir)`：指向当前工程目录
- 所有第三方库路径应使用相对路径，参考 `$(SolutionDir)\..\` 的形式

### 5. Debug/Release 配置差异

- Debug 模式下，库名称会自动添加 `_d` 后缀
- Debug 模式下会禁用 `libcmt.lib` 默认库
- Skia、SDL、JpegTurbo 等库在 Debug 和 Release 下使用不同的输出目录

---

## 版本兼容性

| VS 版本 | 工具集 | C++ 标准 | 支持情况 |
|---------|--------|----------|----------|
| VS2026 | v145 | C++20 | 主分支支持 |
| VS2022 | v143 | C++20 | 主分支支持 |
| VS2019 | v142 | C++17 | develop-cpp17 分支支持 |
| VS2017 | v141 | C++17 | develop-cpp17 分支支持 |

---

## 常见问题排查

### Q: 修改了 .props 文件但配置没有生效
**A:** 必须关闭 VS 并重新打开解决方案，让 MSBuild 重新加载属性文件。

### Q: 编译时报找不到 xxx.lib
**A:** 检查对应的 `XXXSettings.props` 文件中的路径配置是否正确，确保库文件存在于指定目录。

### Q: 想要启用某个可选功能但不知道如何操作
**A:** 编辑对应的 `XXXSettings.props` 文件，将开关变量设置为 1 即可。

### Q: 在不同 VS 版本间切换后编译失败
**A:** 确保使用对应分支（主分支或 develop-cpp17 分支），不同分支支持的 VS 版本不同。
