English | [简体中文](README.md)

> Last synced: 2026-09-29

# nim_duilib MSVC Property Configuration Guide

This document details the purpose and usage of the VS property configuration files (*.props) and script files (*.bat) in the `nim_duilib\msvc\PropertySheets` directory, for reference by maintainers.

## Directory File Overview

```
PropertySheets/
├── CommonSettings.props           # Base configuration (required)
├── DuilibSettings.props          # duilib library's own configuration
├── BinCommonSettings.props       # Executable program public configuration
├── BinCommonSettingsCEF.props     # Executable program configuration with CEF support
├── BinCommonSettingsWebView2.props # Executable program configuration with WebView2 support
├── BinOutSettings.props          # Output directory configuration
├── BinManifestSettings.props     # Manifest configuration
├── SDLSettings.props             # SDL3 support configuration
├── SkiaSettings.props            # Skia graphics library configuration (required)
├── JpegTurboSettings.props        # libjpeg-turbo configuration
├── LibPagSettings.props          # libpag configuration
├── CEFSettings.props             # libCEF configuration
├── WebView2Settings.props         # WebView2 configuration
├── DuilibUseDynamicRuntime.bat    # Switch to dynamic runtime mode
└── DuilibUseStaticRuntime.bat     # Switch to static runtime mode
```

## Property Configuration Files Explained

### 1. CommonSettings.props (base configuration)

**Function:** Base configuration shared by all projects, including basic compiler and linker settings.

**Main content:**

- **Platform toolset selection:** Automatically select the corresponding toolset based on the VS version
  - VS2022 (17.0) → v143
  - VS2019 (16.0) → v142
  - VS2017 (15.0) → v141

- **SDK version configuration:** Windows 10.0 target platform

- **C++ standard configuration:**
  - VS2022/VS2026: Use C++20 standard
  - VS2019/VS2017: Use C++17 standard

- **Runtime library configuration:** Automatically switch between MT/MTd or MD/MDd mode by detecting the `duilib_dll.flag` file

- **Base library list:**
  ```xml
  duilib.lib / duilib_d.lib          # Main library
  zlib.lib / zlib_d.lib              # Compression library
  cximage.lib / cximage_d.lib        # Image processing library
  libpng.lib / libpng_d.lib          # PNG image library
  libwebp.lib / libwebp_d.lib        # WebP image library
  ```

**Important variables:**
- `EnableDuilibDll`: Whether to use DLL mode (1=DLL, 0=static library)
- `DuilibSystemLibs`: System dependency library list
- `DuilibThirdLibs`: Third-party dependency library list
- `DuilibMainLib`: duilib main library name

---

### 2. DuilibSettings.props (duilib library's own configuration)

**Function:** Property configuration specifically for building the duilib library itself.

**Import relationships:**
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

**Main features:**

- **Project type switching:** Decide whether to build as DLL or static library based on the `EnableDuilibDll` variable
  ```xml
  <ConfigurationType>DynamicLibrary</ConfigurationType>  <!-- DLL mode -->
  <ConfigurationType>StaticLibrary</ConfigurationType>   <!-- Static library mode -->
  ```

- **Output configuration:** In DLL mode output to the `bin\` directory; in static library mode output to the `lib\$(Platform)\` directory

- **Feature module macro definitions:**
  - `DUILIB_SDL`: SDL support switch
  - `DUILIB_WEBVIEW2`: WebView2 support switch
  - `DUILIB_CEF`: CEF support switch
  - `DUILIB_JPEG_TURBO`: libjpeg-turbo support switch
  - `DUILIB_LIB_PAG`: libpag support switch

---

### 3. BinCommonSettings.props (executable program public configuration)

**Function:** Common configuration for all executable programs (exe).

**Import relationships:**
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

**Main features:**

- **Include directory configuration:**
  ```xml
  $(SolutionDir)\..;$(ProjectDir)
  ```

- **Library file path configuration:** Select different library paths based on whether DLL mode is enabled

- **Preprocessor definitions:**
  - `DUILIB_SDL`
  - `DUILIB_JPEG_TURBO`
  - `DUILIB_LIB_PAG`

---

### 4. BinCommonSettingsCEF.props (CEF executable program configuration)

**Function:** Configuration for executable programs that need libCEF support.

**Import relationships:**
```
BinCommonSettingsCEF.props
└── BinCommonSettings.props (contains the complete public configuration)
    └── CEFSettings.props
```

**Special configuration:**
- **Delay-load DLL:** `libcef.dll` uses the delay-load mechanism
- **Preprocessor definition:** `DUILIB_CEF=$(LibCefEnabled)`

---

### 5. BinCommonSettingsWebView2.props (WebView2 executable program configuration)

**Function:** Configuration for executable programs that need WebView2 support.

**Import relationships:**
```
BinCommonSettingsWebView2.props
└── BinCommonSettings.props (contains the complete public configuration)
    └── WebView2Settings.props
```

**Special configuration:**
- **Preprocessor definition:** `DUILIB_WEBVIEW2=$(WebView2Enabled)`

---

### 6. BinOutSettings.props (output directory configuration)

**Function:** Unified management of the output directory and intermediate directory of executable programs.

**Configuration rules:**

| Config/Platform | Output directory | Intermediate directory | Target name |
|-----------|----------|----------|----------|
| Release x64 | `..\bin\` | `build_temp\x64\$(ProjectName)\Release\` | `$(ProjectName)` |
| Debug x64 | `..\bin\` | `build_temp\x64\$(ProjectName)\Debug\` | `$(ProjectName)_d` |
| Release Win32 | `..\bin\` | `build_temp\x86\$(ProjectName)\Release\` | `$(ProjectName)32` |
| Debug Win32 | `..\bin\` | `build_temp\x86\$(ProjectName)\Debug\` | `$(ProjectName)32_d` |

---

### 7. SDLSettings.props (SDL3 support configuration)

**Function:** Configure support options for the SDL3 graphics/input library.

**Configuration items:**
- `SDLEnabled`: SDL support switch (0=disabled, 1=enabled)
- `SDLIncludeDir`: SDL include directory (`$(SolutionDir)\..\..\SDL3\include`)
- `SDLLibDir`: SDL lib directory (selected based on platform and configuration)
- `SDLLibs`: SDL dependency library list (`SDL3-static.lib;Version.lib;Winmm.lib;Setupapi.lib`)

**Macro definition:** `DUILIB_SDL=$(SDLEnabled)`

---

### 8. SkiaSettings.props (Skia graphics library configuration)

**Function:** Configure support options for the Skia vector graphics rendering engine.

**Configuration items:**
- `SkiaPreprocessorDefinitions`: Skia preprocessor definitions (`SK_GANESH;SK_GL;SK_RELEASE`)
- `SkiaIncludeDir`: Skia include directory (`$(SolutionDir)\..\..\skia`)
- `SkiaLibDir`: Skia lib directory (release/debug selected based on platform and configuration)
- `SkiaLibs`: Skia dependency library list (`skia.lib;svg.lib;skshaper.lib;skottie.lib;sksg.lib;jsonreader.lib`)

**Note:** Skia is a required component; all configurations depend on Skia.

---

### 9. JpegTurboSettings.props (libjpeg-turbo configuration)

**Function:** Configure support options for the libjpeg-turbo JPEG image codec library.

**Configuration items:**
- `JpegTurboEnabled`: libjpeg-turbo support switch (0=disabled, 1=enabled)
- `JpegTurboIncludeDir`: libjpeg-turbo include directory
- `JpegTurboLibDir`: libjpeg-turbo lib directory
- `JpegTurboLibs`: Dependency library list (`turbojpeg-static.lib`)

**Macro definition:** `DUILIB_JPEG_TURBO=$(JpegTurboEnabled)`

---

### 10. LibPagSettings.props (libpag configuration)

**Function:** Configure support options for the libpag animated image codec library.

**Configuration items:**
- `LibPagEnabled`: libpag support switch (0=disabled, 1=enabled)
- `LibPagIncludeDir`: libpag include directory
- `LibPagLibDir`: libpag lib directory
- `LibPagLibs`: Dependency library list (`libpag.lib`)

**Macro definition:** `DUILIB_LIB_PAG=$(LibPagEnabled)`

---

### 11. CEFSettings.props (libCEF configuration)

**Function:** Configure support options for the libCEF (Chromium Embedded Framework).

**Configuration items:**
- `LibCefEnabled`: libCEF support switch (0=disabled, 1=enabled)
- `LibCefVersion109`: Version selection (0=latest, 1=109)
  - Version 109: Supports Win7 and above
  - Latest version: Supports Win10 and above only
- `LibCefSrcDir`: libCEF source subdirectory
- `LibCefDllWrapperName`: CEF wrapper library name
- `LibCefDllName`: CEF DLL name (`libcef.dll`)
- `LibCefIncludeDir`: libCEF include directory
- `LibCefLibDir`: libCEF lib directory
- `LibCefLibs`: Dependency library list

**Macro definition:** `DUILIB_CEF=$(LibCefEnabled)`

---

### 12. WebView2Settings.props (WebView2 configuration)

**Function:** Configure support options for the Microsoft WebView2 control.

**Configuration items:**
- `WebView2Enabled`: WebView2 support switch (0=disabled, 1=enabled)
- `WebView2LibDir`: WebView2 lib directory
- `WebView2Libs`: Dependency library list (`WebView2LoaderStatic.lib`)

**Macro definition:** `DUILIB_WEBVIEW2=$(WebView2Enabled)`

---

## Script File Description

### 1. DuilibUseDynamicRuntime.bat (switch to dynamic runtime mode)

**Function:** Switch the duilib project to dynamic runtime mode (MD/MDd).

**Operation steps:**
1. Create the `duilib_dll.flag` file
2. Prompt the user to close and reopen the VS project

**Scope of effect:**
- The `EnableDuilibDll` variable in `CommonSettings.props` becomes 1
- All projects are compiled with `/MD` (Release) or `/MDd` (Debug)
- duilib itself is compiled as a DLL

---

### 2. DuilibUseStaticRuntime.bat (switch to static runtime mode)

**Function:** Switch the duilib project to static runtime mode (MT/MTd).

**Operation steps:**
1. Delete the `duilib_dll.flag` file
2. Prompt the user to close and reopen the VS project

**Scope of effect:**
- The `EnableDuilibDll` variable in `CommonSettings.props` becomes 0
- All projects are compiled with `/MT` (Release) or `/MTd` (Debug)
- duilib itself is compiled as a static library

---

## Usage Guide

### How to reference property configuration in a project

#### Building the duilib library itself
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\DuilibSettings.props" />
```

#### Building executable programs (basic version)
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettings.props" />
```

#### Building executable programs (with CEF support)
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettingsCEF.props" />
```

#### Building executable programs (with WebView2 support)
```xml
<Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettingsWebView2.props" />
```

---

### How to switch static/dynamic runtime

1. **Switch to dynamic runtime mode:**
   ```batch
   cd nim_duilib\msvc\PropertySheets
   DuilibUseDynamicRuntime.bat
   ```

2. **Switch to static runtime mode:**
   ```batch
   cd nim_duilib\msvc\PropertySheets
   DuilibUseStaticRuntime.bat
   ```

3. **Reload the project:**
   - Close Visual Studio
   - Reopen the solution file (*.sln)
   - Wait for the properties to finish reloading

---

### How to enable/disable optional features

Edit the corresponding configuration file and modify the switch variable:

| Feature module | Configuration file | Variable name | Optional values |
|----------|----------|--------|--------|
| SDL support | SDLSettings.props | `SDLEnabled` | 0 (disabled), 1 (enabled) |
| libjpeg-turbo | JpegTurboSettings.props | `JpegTurboEnabled` | 0 (disabled), 1 (enabled) |
| libpag | LibPagSettings.props | `LibPagEnabled` | 0 (disabled), 1 (enabled) |
| libCEF | CEFSettings.props | `LibCefEnabled` | 0 (disabled), 1 (enabled) |
| libCEF version | CEFSettings.props | `LibCefVersion109` | 0 (latest), 1 (109) |
| WebView2 | WebView2Settings.props | `WebView2Enabled` | 0 (disabled), 1 (enabled) |

---

## Configuration Dependency Graph

```
┌─────────────────────────────────────────────────────────────┐
│                    DuilibSettings.props                      │
│                   (duilib library's own config)               │
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
│                 (executable program public config)           │
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
│ (CEF program)            │    │ (WebView2 program)              │
├─────────────────────────┤    ├─────────────────────────────┤
│ BinCommonSettings.props  │    │  BinCommonSettings.props    │
│ └─ CEFSettings.props      │    │  └─ WebView2Settings.props   │
└─────────────────────────┘    └─────────────────────────────┘
```

---

## Maintenance Notes

### 1. Configuration files must be reloaded after modification

After modifying any `.props` file, you must **close Visual Studio and reopen the solution**, otherwise the changes may not take effect.

### 2. Runtime library mode switching

- After switching the static/dynamic runtime mode, **the entire solution must be rebuilt**
- It is recommended to clean all intermediate files (build_temp directory) and output files (bin, lib directories) before rebuilding

### 3. Adding a new dependency library

If you need to add a new third-party library dependency, it is recommended to:

1. Create a standalone `XXXSettings.props` configuration file in the `PropertySheets` directory
2. Add `Import` statements in `DuilibSettings.props` and `BinCommonSettings.props`
3. Configure the corresponding Include directory, Lib directory, library file list, and other variables
4. Add the corresponding macro definitions to control the switches

### 4. Path variable conventions

- `$(SolutionDir)`: Points to the directory where the `.sln` file is located
- `$(ProjectDir)`: Points to the current project directory
- All third-party library paths should use relative paths, following the form `$(SolutionDir)\..\`

### 5. Debug/Release configuration differences

- In Debug mode, the library name automatically gets a `_d` suffix
- In Debug mode, the `libcmt.lib` default library is disabled
- Skia, SDL, JpegTurbo, and other libraries use different output directories in Debug and Release

---

## Version Compatibility

| VS version | Toolset | C++ standard | Support status |
|---------|--------|----------|----------|
| VS2026 | v145 | C++20 | Main branch supported |
| VS2022 | v143 | C++20 | Main branch supported |
| VS2019 | v142 | C++17 | develop-cpp17 branch supported |
| VS2017 | v141 | C++17 | develop-cpp17 branch supported |

---

## Common Issues / Troubleshooting

### Q: Modified the .props file but the configuration did not take effect
**A:** You must close VS and reopen the solution so that MSBuild reloads the property files.

### Q: Build reports that xxx.lib cannot be found
**A:** Check whether the path configuration in the corresponding `XXXSettings.props` file is correct, and ensure the library file exists in the specified directory.

### Q: Want to enable an optional feature but don't know how
**A:** Edit the corresponding `XXXSettings.props` file and set the switch variable to 1.

### Q: Build fails after switching between different VS versions
**A:** Make sure to use the corresponding branch (main branch or develop-cpp17 branch); different branches support different VS versions.
