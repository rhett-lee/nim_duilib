# duilib 第三方库定制方式与升级说明

> 本文档由 **CM-21** 维护，用于记录 duilib 对第三方库的 CMake 级定制点，方便后续升级时重放改动，同时降低多库之间的命名/产物风格差异。

---

## 一、定制原则（当前约定）

| 维度 | 约定 |
| :--- | :--- |
| 产物输出目录 | 统一指向仓库根目录 `<root>/lib/` |
| 产物命名 | 以 `duilib-` 为前缀（如 `duilib-zlib`、`duilib-png`、`duilib-webp`、`duilib-cximage`） |
| 库类型 | 优先静态库（静态库名前缀/后缀由 CMake 按平台自动补全） |
| Debug 后缀 | `_d` 后缀受 **CM-5** 阻塞，目前暂未统一启用；升级时请关注该专项 |
| MSVC 运行库 | 统一通过 `CMAKE_POLICY_DEFAULT_CMP0091 NEW` + `CMAKE_MSVC_RUNTIME_LIBRARY` 按 `DUILIB_MD` 开关选择 MT/MTd 或 MD/MDd |

---

## 二、各库定制清单

### 2.1 zlib

**位置**：`duilib/third_party/zlib/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块（`CMAKE_POLICY_DEFAULT_CMP0091 NEW` + `CMAKE_MSVC_RUNTIME_LIBRARY`）
2. 文件头部插入产物输出目录 + 产物名变量：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${DUILIB_SRC_ROOT_DIR}/lib/")
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${DUILIB_SRC_ROOT_DIR}/lib/")
   set(DUILIB_ZLIB_OUTPUT_NAME "duilib-zlib")
   ```
3. 屏蔽自带测试/安装/共享库开关（对 duilib 场景不需要）：
   ```cmake
   set(ZLIB_BUILD_TESTING "OFF")
   set(ZLIB_BUILD_SHARED  "OFF")
   set(ZLIB_INSTALL       "OFF")
   ```
4. 注释掉原始 `zlib_static_suffix` / `CMAKE_DEBUG_POSTFIX` 逻辑（避免与上方 `CMAKE_MSVC_RUNTIME_LIBRARY` 冲突）
5. 在 `if(ZLIB_BUILD_STATIC)` 块内，为 `zlibstatic` 目标追加属性：
   ```cmake
   set_target_properties(zlibstatic PROPERTIES EXPORT_NAME ZLIBSTATIC OUTPUT_NAME ${DUILIB_ZLIB_OUTPUT_NAME})
   ```

**升级重放**：覆盖 `CMakeLists.txt` 后，把上述 5 处 `duilib` 标记块按行号重插（或对比 `diff` 自动合并）。

---

### 2.2 libpng

**位置**：`duilib/third_party/libpng/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. 文件中部插入 zlib 查找适配：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   set(ZLIB_ROOT "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/")
   set(DUILIB_ZLIB_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}duilib-zlib${CMAKE_STATIC_LIBRARY_SUFFIX}")
   set(ZLIB_LIBRARY "${DUILIB_SRC_ROOT_DIR}/lib/${CMAKE_CFG_INTDIR}/${DUILIB_ZLIB_LIB_NAME}")
   ```
3. 在 `if(PNG_STATIC)` 块尾部，将 `png_static` 的 `OUTPUT_NAME` 改为 `duilib-png`
4. 在目标生成区之前设置全局 `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`

**升级重放**：
- 按上述 4 处重插；注意 libpng 官方升级后 `PNG_STATIC` 判断逻辑可能变化，需确认 `png_static` 目标名是否改变。
- 若上游新引入的 `png_shared` / `png_framework` 产物名称与 duilib 无关，可保留原样；duilib 只链接 `duilib-png`（静态）。

---

### 2.3 libwebp

**位置**：`duilib/third_party/libwebp/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. 文件头部插入产物输出目录：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/lib/")
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/lib/")
   ```
3. 将源码收集方式改为 `aux_source_directory(...)`（与原仓库一致，参见 CM-18 结论）
4. 显式定义静态库目标名：`add_library(duilib-webp STATIC ...)`

**升级重放**：`CMakeLists.txt` 整体体量小，可覆盖后按上述 4 处重插。

---

### 2.4 cximage

**位置**：`duilib/third_party/cximage/CMakeLists.txt`

**改动点**：
1. 文件头部插入 `DUILIB_MD` 运行库选择块
2. 文件头部插入产物输出目录
3. `add_library(duilib-cximage STATIC ...)`（直接自定义目标名）
4. 为 GNU/Clang 追加 `-frtti` 编译选项

**升级重放**：cximage 为手写 CMake，升级概率极低；如有上游更新，按上述 4 处手工合并即可。

---

### 2.5 libcef (wrapper)

**位置**：`duilib/third_party/libcef/libcef_{win|win_109|linux|macos}/CMakeLists.txt`

**改动点**：
1. 仅追加产物输出目录：
   ```cmake
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/lib/")
   ```

**当前差异（已定，D-5）**：
- 本库**不**改为 `duilib-*` 前缀，目标名保持 `libcef_dll_wrapper` / `libcef_dll_wrapper_109`（该决定已于 2026-10-21 确认，后续不再变更）。
- 链接侧在 `cmake/duilib_common.cmake` 中以 `DUILIB_CEF_WRAPPER_LIB_NAME` 引用它，因此 CMake 与 VS 两套体系都不需要改链接名。

**升级重放**：CEF wrapper 的 CMake 来自 CEF 官方二进制分发包，升级时通常是替换整个目录；只需在替换后补回上述 3 行即可。

---

## 三、全局索引（链接侧引用）

| 库 | duilib 使用的链接名 | 定义位置 |
| :--- | :--- | :--- |
| zlib | `duilib-zlib` | `zlib` 目标属性 `OUTPUT_NAME` |
| libpng | `duilib-png` | `png_static` 目标属性 `OUTPUT_NAME` |
| libwebp | `duilib-webp` | `add_library(duilib-webp ...)` 目标名 |
| cximage | `duilib-cximage` | `add_library(duilib-cximage ...)` 目标名 |
| libcef wrapper | `libcef_dll_wrapper` / `libcef_dll_wrapper_109` | `duilib_common.cmake` 变量 |

---

## 四、待收敛项（依赖 CM-5）

- **命名统一方向（已定，2026-10-21，D-1）**：统一为 `duilib-*`。CMake 侧已满足；**待实施**的是 VS 侧 4 个库的 `TargetName` 改为 `duilib-zlib` / `duilib-png` / `duilib-webp` / `duilib-cximage`（**不再加 `_d`，见下条**），并同步 `msvc/PropertySheets/CommonSettings.props` 的 `DuilibThirdLibs`、属性表 README、`build/pack.py`。详细改动清单见 [CM-5-Build-Artifacts-Inventory.md](../../docs/misc/CM-5-Build-Artifacts-Inventory.md) 5.2 节。
- **Debug 后缀 `_d`（已定，2026-10-21，D-2）**：**不启用**。库产物统一放到 `lib/<arch>/<Configuration>/`（Windows）或 `lib/<Configuration>/`（Unix），Debug/Release 同名不同目录；VS 侧移除 `TargetName` 的 `_d`，exe 仍保留 `_d`。
- **libcef wrapper 命名（已定，2026-10-21，D-5）**：**不纳入** `duilib-*` 前缀，名称保持 `libcef_dll_wrapper` / `libcef_dll_wrapper_109`；`_d` 后缀按 D-2 一并取消，产物进入配置目录。本库的 duilib 定制仅保留"输出目录指向配置目录"一项，链接侧名称无需任何改动。
