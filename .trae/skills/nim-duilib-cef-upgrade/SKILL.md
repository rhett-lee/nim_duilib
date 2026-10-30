---
name: nim-duilib-cef-upgrade
description: nim_duilib 的 CEF（libcef）依赖升级手册。用于把仓库跟踪的头文件/封装库/导入库与运行时从一个 CEF 版本升到另一个版本（如 142 → 156），或排查 deps 运行时版本错配、wrapper 编译/链接失败、CEF API 被移除等问题。
---

# CEF 版本升级（nim_duilib）

本手册来自 142 → 156 的实战升级（2026-10，Windows x64 验证通过），按顺序执行即可复用。

## 先摸清架构（升级前必读）

| 位置 | 作用 |
|------|------|
| `cmake/third_party_versions.json` | 版本清单：`cef.default` + `cef.versions.<版本>.archives."<平台>/<arch>"` 的 `file` 与 `sha256`。平台键固定 7 个：`windows/x86`、`windows/x64`、`windows/arm64`、`linux/x64`、`linux/arm64`、`macos/x64`、`macos/arm64`（109 只有 windows 三项） |
| `cmake/duilib_cef.cmake` | 脚本模式准备运行时：按清单下载 `_minimal` 包 → 官方 `.sha1` 优先、缺失时回退清单 sha256 → 解压 → Windows 复制 `Release/` + `Resources/` 到 `bin/libcef_win[_109]/<arch>`（x86 目录名是 `Win32`），Linux 到 `bin/libcef_linux`，macOS 到 `DUILIB_CEF_ROOT`；写入指纹 `.duilib-cef`，内容为 `版本\|arch\|平台` |
| `cmake/duilib_common.cmake` | `DUILIB_CEF_LIB_PATH`、`DUILIB_CEF_LIBS`（`libcef` + `libcef_dll_wrapper`） |
| 根 `CMakeLists.txt`（约 144 行起） | 按 `DUILIB_CEF_VERSION` 选 `libcef_win`/`libcef_win_109`/`libcef_linux`/`libcef_macos` 子目录编译 wrapper；Windows 强制 `USE_SANDBOX=OFF`；仅对 `libcef_dll_wrapper_109` 降级 C++17 |
| `duilib/third_party/libcef/libcef_{win,linux,macos}` | 仓库跟踪的 CEF 源码：`include/`、`libcef_dll/`、`cmake/`、`CMakeLists.txt`、`LICENSE.txt`；Windows 另有 `lib/<arch>/libcef.lib` 与 `libcef_dll_wrapper.vcxproj(.filters)`。`libcef_win_109` 是遗留目录，升级新版本时不动 |
| `duilib/CEFControl/` | 业务代码，按 `#if CEF_VERSION_MAJOR` 分支；`internal/CefClientAppRender.cpp` 是 JS 桥入口 |

常用命令：`python3 build/build.py deps`（准备运行时）、`build.py build --no-deps`、`build.py test`、`build.py doctor`；`--cef-version 109|latest`，`latest` 跟随清单默认值。

## 升级步骤

### 1. 取包、登记清单

1. 从 `https://cef-builds.spotifycdn.com/index.json` 取目标版本：确认 `cef_version`、`chromium_version`、各平台 `files[]` 的 `name` 与 `last_modified`（文档里的日期以此为准，不要用二进制文件时间推测）。
2. 下载 7 个 `_minimal` 包（清单只登记 minimal，项目约定）。若要替换源码，再下载同版本的**完整包**（`windows64.tar.bz2` 等）。
3. 计算实际文件的 sha256 写入清单；URL 里的 `+` 必须写成 `%2B`（清单里的文件名是 URL 编码后的形式）。
4. 改 `cmake/third_party_versions.json`：`cef.default` 指向新版本键；新增 `versions."<新版本>"`；删除旧默认版本键（**保留 109**）。JSON 必须能被 `json.load` 解析，且 `default` 存在于 `versions`。

### 2. 替换仓库跟踪的源码（三平台）

对 `libcef_win`、`libcef_linux`、`libcef_macos` 分别替换：`include/`、`libcef_dll/`、`cmake/`、`CMakeLists.txt`、`LICENSE.txt`。

- Windows 还要替换：`lib/{x64,Win32,arm64}/libcef.lib`、`libcef_dll_wrapper.vcxproj` 与 `.filters` 的文件列表。
- **156 起发行包取消了 `lib/` 目录**：导入库在 `Release/libcef.lib`（windows64/32/arm64 各自的包里取），不要再找 `lib/libcef.lib`。
- 先用 `git status` 记录旧目录里被删/新增的头文件，升级后核对（156 新增 `cef_is_complete.h`、`cef_is_instantiation.h`、`cef_to_address.h`、`cef_always_false.h`、`cef_callback_tags.h`、`cef_component_updater.h`、`cef_component_updater_capi.h`、`cef_types_component.h` 与 4 组 wrapper `.h/.cc`；删除了 `cef_auto_reset.h`、`cef_callback_list.h`、`cef_cancelable_callback.h`、`cef_tuple.h`、`cef_raw_scoped_refptr_mismatch_checker.h`、`cef_scoped_policy.h`）。
- 文本文件统一 CRLF（仓库 `core.autocrlf=true`，工作区应为 CRLF）。

### 3. 重新打上 5 处仓库本地补丁（最容易漏）

整体覆盖后必须逐项检查，用 `git grep -n duilib -- duilib/third_party/libcef` 辅助定位：

1. **三个平台的 `CMakeLists.txt`**：`find_package(CEF REQUIRED)` 之后插入
   ```cmake
   # duilib
   get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")
   # duilib end
   ```
   保证 wrapper 产物输出到 `lib/<arch>/<Config>`，与 duilib 库同规则。
2. **`libcef_win/cmake/cef_variables.cmake`**：设置 `CEF_RUNTIME_LIBRARY_FLAG` 之后，按 `DUILIB_MD` 强制 `/MD`，否则 `/MT`（否则与主工程 CRT 不一致，链接报 LNK2038 / LNK2005）。
3. **`libcef_linux|libcef_macos/libcef_dll/CMakeLists.txt`**：`SET_LIBRARY_TARGET_PROPERTIES(${CEF_TARGET})` 之后加
   ```cmake
   add_library(cef_dll_wrapper ALIAS ${CEF_TARGET})
   ```
   否则链接方按 `cef_dll_wrapper` 找不到目标（macOS 静态包前缀不同时产物名会不一致）。
4. **`libcef_win/include/cef_config.h`**：`CEF_V8_ENABLE_SANDBOX` 用 `#if defined(_WIN64)` 包裹，避免 32 位构建问题。
5. **`libcef_linux/cmake/cef_variables.cmake`**：`CMAKE_HOST_SYSTEM_PROCESSOR` 判断补回 `"aarch64"`（上游曾缺失，导致 Linux arm64 识别成 x86_64）。

### 4. 更新 VS 工程文件列表（Windows）

`libcef_dll_wrapper.vcxproj` 是手工维护的文件清单，必须与上游一致：

1. 在临时目录用 `cmake -G "Visual Studio 17 2022"` 配置该版本的完整包，得到上游生成的 `libcef_dll_wrapper.vcxproj`。
2. 取出其中的 `ClCompile` / `ClInclude` 相对路径集合，与仓库 vcxproj 的集合做差集，增删到两边一致（156：cc 213、h 476）。
3. 同步 `.vcxproj.filters`（按目录分组）。
4. **保留仓库特有的接入**：`CEFSettings.props` / `CommonSettings.props` 的 Import 与 `CEF_USE_BOOTSTRAP` 定义，不要被上游工程整体覆盖掉。

### 5. 根工程构建适配

- Windows 一律 `set(USE_SANDBOX OFF CACHE BOOL "..." FORCE)`：
  - 109：`USE_SANDBOX=ON` 会给 Debug wrapper 追加 `_HAS_ITERATOR_DEBUGGING=0`，与 Debug 主工程冲突（LNK2038）。
  - 156：`USE_SANDBOX=ON` 会定义 `CEF_API_VERSION=CEF_API_VERSION_LAST`（15600），而主工程使用实验性 API（默认 `999999`），wrapper 与主工程必须同 API 版本，否则 ABI 不一致。
  - 156 发行包也不再引用 `cef_sandbox.lib`，本项目本来就不链接它。
- wrapper C++ 标准：156 起上游要求 C++20，删除所有降级；只保留 `libcef_dll_wrapper_109` 的 `CXX_STANDARD 17`。
- 判据：配置/编译日志里没有 D9025（选项覆盖）与 LNK2038；wrapper 库文件名与链接方期望一致。

### 6. 代码适配：CEF API 移除

1. 升级前先扫一遍变更：
   - `grep -rn "CEF_API_REMOVED\|CEF_API_ADDED" <新包>/include/`
   - 对比新旧 `include/` 的函数签名与 `cef_api_hash.h` 的 `CEF_API_VERSION_LAST` / `MIN`。
2. 在 `duilib/CEFControl/` 里搜被移除符号。156 移除了 `CefRegisterExtension`（`removed=15400`，见 `include/cef_v8.h`），原代码编译报 C3861。
3. 替代写法（保持 Web 端 API 不变）：`OnContextCreated` 里向页面全局对象绑定 native 函数，再 `Eval` 注入包装代码；**109 分支保留原实现**，不要删除：

   ```cpp
   #if CEF_VERSION_MAJOR > 109
   CefRefPtr<CefV8Value> global = context->GetGlobal();
   CefRefPtr<CefJSHandler> handler = new CefJSHandler();
   handler->AttachJSBridge(m_renderJsBridge);
   global->SetValue("__nimCefNativeCall", CefV8Value::CreateFunction("call", handler),
                    V8_PROPERTY_ATTRIBUTE_DONTENUM);
   global->SetValue("__nimCefNativeRegister", CefV8Value::CreateFunction("register", handler),
                    V8_PROPERTY_ATTRIBUTE_DONTENUM);
   // Eval 注入 NimCefWebInstance.call / register，内部调用上面的 native 函数
   #else
   // 原 CefRegisterExtension("v8/extern", extensionCode, handler)
   #endif
   ```

4. `OnContextCreated` 每个 frame 都会执行，注入代码必须可重复执行（覆盖同名对象即可）；`CefJSHandler::Execute` 以 `name == "call"/"register"` 分发。

### 7. 收紧 deps 的“采纳已有运行时”规则

升级后最隐蔽的坑：Windows 运行时目录只有二进制，读不到版本，旧 `libcef.dll` 会被**误写成新版本指纹**（142 运行时被标成 `156|x64|windows` 就是这样发生的）。

- 规则：只有 Linux/macOS 在 `bin/libcef_linux`（或 `DUILIB_CEF_ROOT`）的 `include/cef_version.h` 与清单版本前缀匹配时才“采纳”；Windows 一律按归档重新解压覆盖。
- 判据：删掉 `bin/libcef_win/x64` 后 `python3 build/build.py deps --offline`，检查 `libcef.dll` 的 `FileVersionInfo`、文件大小与 `.duilib-cef` 内容一致。
- 换版本后 `bin/libcef_win/{Win32,arm64}`、`bin/libcef_linux` 里的旧运行时也要刷新，否则出包会静默带上旧 DLL（可先删目录再让 deps 重建）。

### 8. 脚本与文档同步（禁止硬编码版本号）

- `build/pack.py`、`build/sdk_package_test.py`：默认版本提示从 `cmake/third_party_versions.json` 的 `cef.default` 读取；`test_build.py`、`test_pack.py` 的断言跟随清单。
- `cmake/duilib_root_options.cmake` 的帮助文本、`build/build.py` 的注释不要写死版本。
- 文档：`README(.en).md`、`docs/CEF(.en).md`（下载链接与日期）、`docs/Deployment(.en).md`、`docs/Differences(.en).md`、`build/build-reference(.en).md`、`build/README(.en).md`、`build/arm64-test-plan.md`、`duilib/third_party/README.md`（归档来源表）、`CHANGELOG(.en).md`，`build/test-matrix.md` 更新“当前基线”并追加当天的小节。
- **历史记录不改**：`docs/misc/*`、`build/sdk-package-test.md` 的旧结果、test-matrix 的历史行。

### 9. 验证清单（按顺序）

```bash
python3 build/build.py deps --offline          # 运行时：确认版本、指纹、DLL 版本
python3 build/build.py build --config Release --no-deps
python3 build/build.py build --config Debug --no-deps
python3 build/build.py test                    # 236 用例（skipped=2）基线
python3 build/build.py doctor                  # cef default / cef runtime 行必须为新版本
git diff --check                               # 空白错误；同时检查 CRLF 一致性
```

补充要点：

- **Debug 前先看 Skia**：Debug 与 Release 用不同的 Skia 产物（如 `E:\develop\skia\out\llvm.x64.debug`）。若 Skia 源码/清单刚变过，Debug 链接会报 `SkPathBuilder::addPolygon`、`SkDashPathEffect::Make`、`SkCanvas::drawGlyphs` 之类 `LNK2019`——先 `python3 build/build.py deps --config Debug --offline` 重编该变体，与 CEF 无关。
- **构建前结束示例进程**：`CefBrowser.exe` 之类的残留进程会占用 `bin/*.exe`，导致链接阶段删除旧文件失败（表现为构建 exit=1 但无编译错误）。
- 冒烟：`Start-Process bin\cef.exe`/`CefBrowser.exe`，8–10 秒后仍存活即可，随后 `Stop-Process`。
- JS 桥端到端（156 及以后必做）：见下一节。

### 10. JS 桥端到端探针（可选但强烈建议）

`CefRegisterExtension` 被移除后，仅“能编译”不代表桥可用。用 DevTools 协议实测：

```powershell
# 终端 A：带远程调试端口启动示例
Start-Process .\bin\cef.exe -ArgumentList '--remote-debugging-port=9335'
# 终端 B（等页面加载完成约 8 秒）
python3 .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 inspect
python3 .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 invoke
python3 .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 read
```

判据：`inspect` 输出 `NimCefWebInstance` 为 `object`、`__nimCefNativeCall` 为 `function`；`invoke` 后弹出一个标题含 `C++ receives the message sent by JavaScript.` 的模态框（用 `EnumWindows` 找该进程新增的可见窗口），向其发送 `WM_CLOSE`（`0x0010`）；`read` 输出 `[false,{"message":"Success."}]`（`window.__r`）。

原理与坑：

- `Runtime.evaluate` 在 renderer 进程执行，即使 UI 线程被模态框阻塞也能继续求值，所以调用后仍可读回调结果。
- PowerShell 5 没有现成 WebSocket 客户端；脚本用裸 socket 实现，**客户端帧必须加掩码（mask）**，否则服务端直接断开（握手 101 成功但收不到任何响应）。
- 必须处理 ping/pong 与分片长度（126/127 扩展），并按 `id` 匹配响应。

## 常见问题与教训

| 现象 | 原因 / 处理 |
|------|------|
| 升级后仍加载旧 DLL，指纹却是新版本 | deps 旧“迁移采纳”逻辑；按第 7 节收紧规则，删除运行时目录重跑 |
| `C3861: CefRegisterExtension 找不到标识符` | 156 移除该 API；按第 6 节绑定 native 函数 + Eval |
| wrapper 链接报 LNK2038 / LNK2005 | `CEF_RUNTIME_LIBRARY_FLAG` 与主工程 CRT 不一致；检查第 3 节补丁 2 与 `DUILIB_MD` |
| 日志出现 D9025 | wrapper 同时出现 C++20 与 C++17 选项；156 起不要再降级 |
| Debug 链接一堆 Skia `LNK2019` | Debug Skia 变体落后于源码；`deps --config Debug` |
| 构建 exit=1 但无编译错误 | 旧示例进程占用 bin 产物；先结束进程再链接 |
| 命令行管道判断失败 | PowerShell `Select-Object -First N` 会提前终止原生命令，导致误判；改全量日志 + `Select-String` / `Tee-Object` |
| CMake 变量判断失效 | `if(NOT VAR STREQUAL "")` 在未定义时按字面量比较；必须写 `if(NOT "${VAR}" STREQUAL "")` |
| 解压命令报错 | CMake 4.x 的 `cmake -E tar` 不支持 `--current-directory`；用 `Push-Location` 后再执行 |
| 文档日期不对 | 以 `index.json` 的 `last_modified` 为准，别用 DLL 时间戳 |
| 补丁丢失 | 整体覆盖 vendored 目录后忘记重打；用 `git grep -n duilib -- duilib/third_party/libcef` 自查 |

## 新版本预检（升级前 10 分钟）

1. 解压完整包，读 `include/cef_version.h` 的 `CEF_VERSION` / `CEF_VERSION_MAJOR` 与 `include/cef_api_hash.h` 的 `CEF_API_VERSION_MIN/LAST`。
2. `diff` 新包与仓库跟踪目录的 `include/`、`libcef_dll/` 文件集合（新增/删除类文件）。
3. 检查 `<pkg>/cmake/cef_variables.cmake`：wrapper 的 C++ 标准、`USE_SANDBOX` 分支、是否还存在 `cef_sandbox.lib`。
4. 检查 `<pkg>` 根目录布局：有无 `lib/`（决定导入库来源）、`Release/` 里是否新增 DLL（如 `bootstrap.exe`、`dxcompiler.dll`，出包规则是否要跟着改）。
5. 在 `include/` 里 `grep CEF_API_REMOVED`，对照 `duilib/CEFControl/` 与示例代码里用到的符号，先列出改造点再动手。
