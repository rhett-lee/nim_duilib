---
name: nim-duilib-cef-upgrade
description: CEF（libcef）依赖升级与排错：版本清单、vendored 源码替换、本地补丁、wrapper 构建、CEF API 移除适配、deps 运行时校验
trigger: 当用户要求升级 CEF/libcef 版本、deps 运行时版本不对、wrapper 编译或链接失败、CefRegisterExtension 等 API 找不到时触发
---

# nim_duilib CEF 版本升级

完整流程、实测记录与坑表见仓库内 Skill：`.trae/skills/nim-duilib-cef-upgrade/SKILL.md`
（与 `.workbuddy/skills/nim-duilib-cef-upgrade/` 同源）；JS 桥探针：
`.trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py`。以下为可直接执行的要点。

## 关键位置

| 位置 | 作用 |
|------|------|
| `cmake/third_party_versions.json` | `cef.default` + `cef.versions.<版本>.archives."<平台>/<arch>"` 的 `file`/`sha256`；平台键 7 个（`windows/x86|x64|arm64`、`linux/x64|arm64`、`macos/x64|arm64`，109 仅 windows 三项） |
| `cmake/duilib_cef.cmake` | 下载 `_minimal` 包 → 校验（优先官方 `.sha1`，回退清单 sha256）→ Windows 解压到 `bin/libcef_win[_109]/<arch>`，Linux `bin/libcef_linux`，macOS `DUILIB_CEF_ROOT`；写指纹 `.duilib-cef` = `版本\|arch\|平台` |
| 根 `CMakeLists.txt` | 按 `DUILIB_CEF_VERSION` 选 vendored 目录并编译 `libcef_dll_wrapper`；Windows `USE_SANDBOX=OFF`；仅 109 降级 C++17 |
| `duilib/third_party/libcef/libcef_{win,linux,macos}` | 跟踪的 `include/`、`libcef_dll/`、`cmake/`、`CMakeLists.txt`、`LICENSE.txt`；Windows 另有 `lib/<arch>/libcef.lib`、`libcef_dll_wrapper.vcxproj(.filters)`（`libcef_win_109` 为遗留目录，升级时不动） |
| `duilib/CEFControl/internal/CefClientAppRender.cpp` | JS 桥与 `CEF_VERSION_MAJOR` 分支 |

## 升级步骤

1. **登记版本**：从 `https://cef-builds.spotifycdn.com/index.json` 取版本/日期；清单 URL 中 `+` 写作 `%2B`；
   修改清单（`cef.default` 指向新版本、新增 156 式的新条目、删除旧默认版本、**保留 109**）。
2. **替换 vendored 源码（三平台）**：`include/`、`libcef_dll/`、`cmake/`、`CMakeLists.txt`、`LICENSE.txt`；
   Windows 另换 `lib/{x64,Win32,arm64}/libcef.lib` 与 vcxproj 文件列表。
   **156 起发行包无 `lib/`**，导入库取 `Release/libcef.lib`。
3. **重打 5 处本地补丁**（整体覆盖后极易丢失）：
   - 三平台 `CMakeLists.txt`：`find_package(CEF REQUIRED)` 后插入 `DUILIB_SRC_ROOT_DIR` +
     `include(.../cmake/duilib_lib_output.cmake)`；
   - `libcef_win/cmake/cef_variables.cmake`：按 `DUILIB_MD` 强制 `/MD`，否则 `/MT`；
   - `libcef_linux|macos/libcef_dll/CMakeLists.txt`：`add_library(cef_dll_wrapper ALIAS ${CEF_TARGET})`；
   - `libcef_win/include/cef_config.h`：`CEF_V8_ENABLE_SANDBOX` 仅 `#if defined(_WIN64)`；
   - `libcef_linux/cmake/cef_variables.cmake`：主机处理器判断补 `aarch64`。
   自查：`git grep -n duilib -- duilib/third_party/libcef`。
4. **更新 vcxproj/.filters**：用 `cmake -G "Visual Studio 17 2022"` 配置新包生成上游工程，
   对齐 `ClCompile`/`ClInclude` 集合（156：cc 213、h 476），保留 `CEFSettings.props` 与 `CEF_USE_BOOTSTRAP`。
5. **构建适配**：Windows 全版本 `USE_SANDBOX OFF`（109 迭代器调试冲突；156 沙箱会定义
   `CEF_API_VERSION_LAST` 与主工程实验性 API 不一致）；wrapper 156 起用 C++20，仅 109 保留 C++17。
6. **API 移除适配**：先 `grep -rn "CEF_API_REMOVED\|CEF_API_ADDED" <新包>/include/`，再在
   `duilib/CEFControl/` 查被移除符号。156 移除 `CefRegisterExtension`（`removed=15400`）：
   在 `OnContextCreated` 用 `global->SetValue("__nimCefNativeXxx", CefV8Value::CreateFunction(...), DONTENUM)`
   绑定 native 函数，再 `context->Eval(...)` 注入等价的 `NimCefWebInstance.call/register`；
   `#if CEF_VERSION_MAJOR <= 109` 保留原实现。
7. **修 deps 采纳规则**：Windows 读不到运行时版本，必须按归档重新解压覆盖；仅 Linux/macOS 在
   `include/cef_version.h` 匹配时才采纳，避免旧运行时被写成新指纹。
8. **脚本/文档同步**：`build/pack.py`、`build/sdk_package_test.py` 的默认版本从清单读取；
   `test_build.py`/`test_pack.py` 断言跟随清单；README/docs/CHANGELOG/test-matrix 更新默认版本；
   历史记录不改。

## 验证

```bash
python3 build/build.py deps --offline        # 确认 libcef.dll 版本/大小与 .duilib-cef 指纹
python3 build/build.py build --config Release --no-deps
python3 build/build.py build --config Debug --no-deps
python3 build/build.py test
python3 build/build.py doctor                # cef default / cef runtime 应为新版本
git diff --check
```

- Debug 链接报 Skia `LNK2019` 时先 `deps --config Debug`（Debug Skia 变体落后，与 CEF 无关）。
- 构建前结束 `cef.exe`/`CefBrowser.exe` 残留进程，否则占用 bin 产物导致链接失败。
- JS 桥端到端（156 起必做）：

  ```bash
  Start-Process .\bin\cef.exe -ArgumentList '--remote-debugging-port=9335'
  python .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 inspect
  python .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 invoke
  # 用 EnumWindows 找到新弹窗并 PostMessage WM_CLOSE(0x0010)，然后：
  python .trae/skills/nim-duilib-cef-upgrade/scripts/cef_bridge_probe.py --port 9335 read
  ```

  `inspect` 输出 `NimCefWebInstance` 为 `object`、`__nimCefNativeCall` 为 `function`；
  `read` 输出 `[false,{"message":"Success."}]`。

## 高发问题

| 现象 | 处理 |
|------|------|
| 仍加载旧 DLL、指纹却是新版本 | deps 旧“采纳”逻辑；删运行时目录重跑（Windows 一律重新解压） |
| `C3861: CefRegisterExtension 找不到标识符` | 按第 6 步绑定 native 函数 + `Eval` |
| LNK2038 / LNK2005 | `CEF_RUNTIME_LIBRARY_FLAG` 与主工程 CRT 不一致，检查 `DUILIB_MD` 补丁 |
| D9025 选项覆盖 | 156 起不要再给 wrapper 设 C++17 |
| 构建 exit=1 且无编译错误 | 残留示例进程占用 bin 产物 |
| 命令行判断误报 | PowerShell `Select-Object -First N` 会提前终止原生命令管道 |
| CMake 变量判断失效 | 未定义变量需写 `if(NOT "${VAR}" STREQUAL "")` |

## 新版本预检

读新包 `include/cef_version.h`、`include/cef_api_hash.h`（`CEF_API_VERSION_MIN/LAST`）；
diff `include/`、`libcef_dll/` 文件集合；检查 `<pkg>/cmake/cef_variables.cmake` 的 C++ 标准、
`USE_SANDBOX` 分支、是否仍引用 `cef_sandbox.lib`；确认包内是否还有 `lib/`、`Release/` 是否新增 DLL。
