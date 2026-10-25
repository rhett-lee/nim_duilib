# duilib 第三方库定制方式与升级说明

> 本文档由 **CM-21** 维护，用于记录 duilib 对第三方库的 CMake 级定制点，方便后续升级时重放改动，同时降低多库之间的命名/产物风格差异。

---

## 一、定制原则（当前约定）

| 维度 | 约定 |
| :--- | :--- |
| 产物输出目录 | 由 [`cmake/duilib_lib_output.cmake`](../../cmake/duilib_lib_output.cmake) 统一设置：Windows `<root>/lib/<arch>/<Config>/`，Unix `<root>/lib/<Config>/` |
| 产物命名 | 以 `duilib-` 为前缀（如 `duilib-zlib`、`duilib-png`、`duilib-webp`、`duilib-cximage`） |
| 库类型 | 优先静态库（静态库名前缀/后缀由 CMake 按平台自动补全） |
| Debug 后缀 | `_d` 已取消（CM-5 / D-2）；Debug 与 Release 同名、不同目录，exe 仍保留 `_d` |
| MSVC 运行库 | 统一通过 `CMAKE_POLICY_DEFAULT_CMP0091 NEW` + `CMAKE_MSVC_RUNTIME_LIBRARY` 按 `DUILIB_MD` 开关选择 MT/MTd 或 MD/MDd |

---

## 二、各库定制清单

### 2.1 zlib

**位置**：`duilib/third_party/zlib/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块（`CMAKE_POLICY_DEFAULT_CMP0091 NEW` + `CMAKE_MSVC_RUNTIME_LIBRARY`）
2. 文件头部插入产物名变量；`project()` 之后 include 输出目录脚本：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   set(DUILIB_ZLIB_OUTPUT_NAME "duilib-zlib")

   project(zlib ...)

   # 库输出目录（D-2：按平台/配置分目录）
   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")
   ```
3. 屏蔽自带测试/安装/共享库开关（对 duilib 场景不需要）：
   ```cmake
   set(ZLIB_BUILD_TESTING "OFF")
   set(ZLIB_BUILD_SHARED  "OFF")
   set(ZLIB_INSTALL       "OFF")
   ```
4. 注释掉原始 `zlib_static_suffix` / `CMAKE_DEBUG_POSTFIX` 逻辑（避免与上方 `CMAKE_MSVC_RUNTIME_LIBRARY` 冲突）
5. 在 `if(ZLIB_BUILD_STATIC)` 块内，将静态库目标改名为 `duilib-zlib`（与库文件名一致），
   并保留上游别名与导出名：
   ```cmake
   add_library(duilib-zlib STATIC ...)
   add_library(ZLIB::ZLIBSTATIC ALIAS duilib-zlib)
   set_target_properties(duilib-zlib PROPERTIES EXPORT_NAME ZLIBSTATIC OUTPUT_NAME ${DUILIB_ZLIB_OUTPUT_NAME})
   ```
   目标名与库名一致后，`DUILIB_LIBS` 可直接按目标名链接，CMake 会自动建立构建依赖。

**升级重放**：覆盖 `CMakeLists.txt` 后，把上述 5 处 `duilib` 标记块按行号重插（或对比 `diff` 自动合并）。

---

### 2.2 libpng

**位置**：`duilib/third_party/libpng/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. 文件中部插入 zlib 查找适配与输出目录脚本：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")
   set(ZLIB_ROOT "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/")
   set(DUILIB_ZLIB_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}duilib-zlib${CMAKE_STATIC_LIBRARY_SUFFIX}")
   # D-2 后为 <root>/lib[<arch>]/<Config>；$<CONFIG> 在链接时展开
   set(ZLIB_LIBRARY "${DUILIB_LIB_BASE}/$<CONFIG>/${DUILIB_ZLIB_LIB_NAME}")
   ```
3. 在 `if(PNG_STATIC)` 块中，将 `png_static` 目标改名为 `duilib-png`（与库文件名一致），
   `OUTPUT_NAME` 设为 `duilib-png`，并保留上游导出名 `png_static`：
   ```cmake
   add_library(duilib-png STATIC ...)
   set_target_properties(duilib-png PROPERTIES OUTPUT_NAME "duilib-png" EXPORT_NAME png_static)
   ```
4. 注释掉 `duilib-png` 自带的 `DEBUG_POSTFIX`（配置由目录表达，不再加 `_d`）
5. `PNG_LINK_LIBRARIES` 优先链接本工程的 zlib 目标，避免 `FindZLIB` 生成的
   `ZLIB::ZLIB` 导入目标把含 `$<CONFIG>` 的库路径带入依赖列表：
   ```cmake
   if(TARGET duilib-zlib)
     set(PNG_LINK_LIBRARIES duilib-zlib)
   else()
     set(PNG_LINK_LIBRARIES ZLIB::ZLIB)
   endif()
   ```
6. 根工程以 `SKIP_INSTALL_ALL` 内嵌构建 libpng（`duilib-png` 链接非导出目标 `duilib-zlib`，
   开启安装导出会导致生成失败）

**升级重放**：
- 按上述 6 处重插；注意 libpng 官方升级后 `PNG_STATIC` 判断逻辑可能变化，需确认 `png_static` 目标名是否改变。
- 若上游新引入的 `png_shared` / `png_framework` 产物名称与 duilib 无关，可保留原样；duilib 只链接 `duilib-png`（静态）。

---

### 2.3 libwebp

**位置**：`duilib/third_party/libwebp/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. `project()` 之后 include 输出目录脚本：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")
   ```
3. 将源码收集方式改为 `aux_source_directory(...)`（与原仓库一致，参见 CM-18 结论）
4. 显式定义静态库目标名：`add_library(duilib-webp STATIC ...)`

**升级重放**：`CMakeLists.txt` 整体体量小，可覆盖后按上述 4 处重插。

---

### 2.4 cximage

**位置**：`duilib/third_party/cximage/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. `project()` 之后 include 输出目录脚本（`cmake/duilib_lib_output.cmake`）
3. `add_library(duilib-cximage STATIC ...)`（直接自定义目标名）
4. 为 GNU/Clang 追加 `-frtti` 编译选项

**升级重放**：cximage 为手写 CMake，升级概率极低；如有上游更新，按上述 4 处手工合并即可。

---

### 2.5 libcef (wrapper)

**位置**：`duilib/third_party/libcef/libcef_{win|win_109|linux|macos}/CMakeLists.txt`

**改动点**：
1. `project()` 之后 include 输出目录脚本（与 duilib 库同一套规则）：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")
   ```

**当前差异（已定，D-5）**：
- 本库**不**改为 `duilib-*` 前缀，目标名保持 `libcef_dll_wrapper` / `libcef_dll_wrapper_109`（该决定已于 2026-10-21 确认，后续不再变更）。
- 链接侧在 `cmake/duilib_common.cmake` 中以 `DUILIB_CEF_WRAPPER_LIB_NAME` 引用它，因此 CMake 与 VS 两套体系都不需要改链接名。
- macOS/Linux 上 `PREFIX ""` 使产物为 `cef_dll_wrapper.a`，与 Windows 链接名
  `libcef_dll_wrapper` 不同。`libcef_dll/CMakeLists.txt` 在目标创建后补一个同名 ALIAS：
  ```cmake
  add_library(cef_dll_wrapper ALIAS ${CEF_TARGET})
  ```
  这样 `DUILIB_CEF_LIBS` 中的 `cef_dll_wrapper` 会解析为构建目标（绝对路径 + 构建依赖），
  避免并行构建时在 wrapper 生成前链接而报 `library 'cef_dll_wrapper' not found`。

**升级重放**：CEF wrapper 的 CMake 来自 CEF 官方二进制分发包，升级时通常是替换整个目录；只需在替换后补回上述 3 行即可。

---

## 三、全局索引（链接侧引用）

| 库 | duilib 使用的链接名 | 定义位置 |
| :--- | :--- | :--- |
| zlib | `duilib-zlib` | `add_library(duilib-zlib ...)` 目标名 |
| libpng | `duilib-png` | `add_library(duilib-png ...)` 目标名 |
| libwebp | `duilib-webp` | `add_library(duilib-webp ...)` 目标名 |
| cximage | `duilib-cximage` | `add_library(duilib-cximage ...)` 目标名 |
| libcef wrapper | Windows：`libcef_dll_wrapper` / `libcef_dll_wrapper_109`（即目标名）；<br>macOS/Linux：`cef_dll_wrapper`（ALIAS，产物为 `cef_dll_wrapper.a`） | `duilib_common.cmake` 变量 + `libcef_dll/CMakeLists.txt` 的 ALIAS |

---

## 四、CM-5 收敛结果（已完成）

- **命名统一（D-1，已完成）**：CMake 与 VS 两侧的 4 个库统一为
  `duilib-zlib` / `duilib-png` / `duilib-webp` / `duilib-cximage`；
  `msvc/PropertySheets/CommonSettings.props` 的 `DuilibThirdLibs`、属性表 README
  与 `build/pack.py` 白名单已同步。
- **Debug 后缀 `_d`（D-2，已完成）**：**不启用**。库产物统一放到
  `lib/<arch>/<Configuration>/`（Windows）或 `lib/<Configuration>/`（Unix），
  Debug/Release 同名不同目录；VS 侧已移除库的 `_d`，exe 仍保留 `_d`。
- **libcef wrapper 命名（D-5，已完成）**：**不纳入** `duilib-*` 前缀，名称保持
  `libcef_dll_wrapper` / `libcef_dll_wrapper_109`；`_d` 后缀按 D-2 一并取消，
  产物进入配置目录。本库的 duilib 定制仅保留"输出目录指向配置目录"一项。
- **打包侧（P4，已完成）**：`build/pack.py` 只认这一套布局，`--config` 选择配置，
  Unix `.a` 与 `lib` 前缀已处理；Skia / WebView2 / turbojpeg / SDL3 / CEF 链接库
  已纳入 SDK 包。
