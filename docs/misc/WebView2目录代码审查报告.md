# WebView2 目录代码审查报告

## 一、审查范围

- 审查目录：[duilib/WebView2](file:///e:/develop/nim_duilib/duilib/WebView2)，共 10 个文件（4 个 `.cpp`、6 个 `.h`）：
  - COM 封装：[ComPtr.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComPtr.h)（COM 智能指针）、[ComCallback.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComCallback.h)（COM 回调对象，把 lambda/成员函数包装成 COM 接口）；
  - 环境参数：[WebView2EnvironmentOptions.h](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2EnvironmentOptions.h)/[.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2EnvironmentOptions.cpp)（`ICoreWebView2EnvironmentOptions` 及其扩展接口的实现）；
  - 全局配置：[WebView2Manager.h](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Manager.h)/[.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Manager.cpp)（进程级配置、默认缓存目录、注册控件创建回调）；
  - 控件层：[WebView2Control.h](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Control.h)/[.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Control.cpp)（对外接口、初始 URL 处理、窗口截图、控件属性）；
  - 实现层：[WebView2ControlImpl.h](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.h)/[.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp)（环境/控制器/WebView 的创建与清理、事件注册、脚本执行、消息收发、截图与站点图标）。
- 审查内容：空指针与 COM 生命周期、引用计数、回调线程与消息循环、窗口句柄与资源释放、整数与尺寸计算、成员初始化、逻辑条件、宽字符/UTF-8 转换。
- 审查原则：只修复可证实且风险低的缺陷，不做结构性重构；新增或修改的注释均使用简体中文。

## 二、总体结论

整体结构清晰：`ComPtr`/`ComCallback` 封装 COM 的生命周期与回调；`WebView2Manager` 保存进程级配置并负责控件注册；控件采用“延迟初始化”，首次需要时创建环境（Environment）→ 控制器（Controller）→ WebView，事件回调统一通过 `UiBind`/`ToWeakCallback` 转发到 UI 线程，避免跨线程操作界面。未发现内存越界、重复释放、明显空指针解引用等高危缺陷。

本轮共修复 7 处问题、修正 1 处注释错误，均为小范围修改：

1. `ComCallback` 创建回调对象时引用计数多 1，导致回调对象内存泄漏；
2. `MakeComPtr<T>()` 同样多一次 `AddRef`，导致对象泄漏；
3. `WebView2Control::Impl` 构造函数未初始化 `m_bScriptEnabled`/`m_bScriptEnabledSet`，且初始化列表顺序与声明顺序不一致；
4. `WebView2Manager::GetLanguage` 的静态缓存导致 `SetLanguage` 在首次调用后失效；
5. `GetUserAgent` 在未设置本地 UserAgent 时永远返回空，读不到运行时的实际值；
6. `AreDevToolsEnabled` 在控件未初始化时返回 `false`，与默认值及同类接口语义不一致；
7. 位图尺寸计算使用 `int` 相乘，存在整型溢出隐患；
8. 修正 `SetZoomControlEnabled` 的注释（“禁用”改为“启用”）。

## 三、已修复的问题

### 1. ComCallback 创建回调对象时引用计数多 1（内存泄漏）

- 位置：[ComCallback.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComCallback.h#L373-L381)
- 原实现：

```cpp
return ComPtr<TInterface>(new Impl(std::forward<TCallback>(callback)));
```

- 问题：`Impl` 构造时引用计数从 1 开始（符合 COM 对象刚创建的状态），而 `ComPtr(T*)` 构造函数会再执行一次 `AddRef`，引用计数变成 2。表达式结束后临时 `ComPtr` 析构（-1），此后 WebView2 释放回调对象（-1）后计数仍为 1，对象永远不会被 `delete`。
- 影响：所有通过 `ui::ComCallback(...)` 注册的事件回调（环境创建、控制器创建、初始化完成、导航事件、Web 消息、下载、焦点、快捷键、favicon、新窗口等）都会泄漏一个回调对象及其捕获的 `std::function`；长时间运行或反复创建控件时内存持续增长。
- 修复：直接写入 `ComPtr` 的内部指针，不再调用 `AddRef`：

```cpp
ComPtr<TInterface> spCallback;
*spCallback.GetAddressOf() = new Impl(std::forward<TCallback>(callback));
return spCallback;
```

### 2. MakeComPtr<T>() 同样多一次 AddRef

- 位置：[ComPtr.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComPtr.h#L340-L347)
- 原实现：`return ComPtr<T>(new T);`，与上一条原因相同，引用计数为 2，对象泄漏。
- 修复：改为直接写入内部指针。带参数的 `MakeComPtr<T>(U* p)` 保持原有语义（接管 + `AddRef`，调用方负责释放自己的引用），未做改动。

### 3. Impl 构造函数未初始化 m_bScriptEnabled/m_bScriptEnabledSet

- 位置：[WebView2ControlImpl.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp#L28-L50)
- 问题：构造函数初始化列表缺少 `m_bScriptEnabled` 与 `m_bScriptEnabledSet` 两项，控件初始化完成前调用 `IsScriptEnabled()`/`GetScriptEnabled()` 会返回未定义值（Debug 下为调试填充值）；同时 `m_lastError` 的初始化顺序与头文件声明顺序不一致（MSVC 的 C5038 警告）。
- 修复：补上 `m_bScriptEnabled(true)`、`m_bScriptEnabledSet(false)`（与 `WebView2Manager` 的默认值一致），并把 `m_lastError` 调整到声明顺序位置。

### 4. GetLanguage 的静态缓存导致 SetLanguage 失效

- 位置：[WebView2Manager.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Manager.cpp#L103-L121)
- 原实现：

```cpp
static DString language = m_language;   // 首次调用即固化
if (language.empty()) { ... 读取语言资源 ... }
return language;
```

- 问题：第一次调用 `GetLanguage()` 后，`static` 变量即被固化；之后再调用 `SetLanguage("en-US")` 完全无效，环境创建时使用的仍是最初的语言。对于“先查询、后配置”的用法（例如启动流程中先读取配置，再设置语言）会直接导致页面语言错误。
- 修复：显式设置的语言优先级最高，直接返回；只有未显式设置时才缓存“从语言资源推导出的默认语言”（仍保持一个进程只使用一种语言的约束）。

### 5. GetUserAgent 读不到运行时的实际值

- 位置：[WebView2ControlImpl.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp#L645-L662)
- 原实现：`if ((m_spWebView2 != nullptr) && !userAgent.empty()) { ...查询 ICoreWebView2Settings2... }`
- 问题：本地 `m_userAgent` 为空时（大多数情况）不会去查询运行时，直接返回空字符串；而 WebView2 本身有默认 UserAgent，业务代码无法获取实际值。
- 修复：只要 WebView 对象存在就查询运行时的实际 UserAgent，查询失败时回退到本地设置值。

### 6. AreDevToolsEnabled 在未初始化时返回 false

- 位置：[WebView2ControlImpl.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp#L1442-L1456)
- 问题：原实现只要 `m_spWebView2 == nullptr` 就返回 `false`，而默认配置是 `true`，且 `IsScriptEnabled`/`IsWebMessageEnabled` 等同组接口都采用“未初始化时返回本地设置值”的回退策略，语义不一致；也导致 `OpenDevToolsWindow()` 在初始化前永远返回失败。
- 修复：无 WebView 或读取设置失败时返回 `m_bAreDevToolsEnabled`。

### 7. 位图尺寸计算的整型溢出

- 位置：
  - [WebView2Control.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Control.cpp#L552)（窗口截图缓冲区分配）
  - [WebView2Control.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Control.cpp#L638)（截图缓冲区大小校验）
  - [WebView2ControlImpl.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp#L1549)（站点图标数据拷贝）
- 原实现：`width * height * 4`、`nHeight * nWidth * 4` 以 `int` 计算，`(int32_t)bitmap.size()` 做截断比较。
- 修复：先转换为 `size_t` 再相乘、直接比较 `size_t`，与 `CEFControl` 目录中同类实现保持一致。

### 8. 注释错误

- 位置：[WebView2Manager.h](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Manager.h#L93)
- `SetZoomControlEnabled` 的注释由“是否禁用缩放控制”改为“是否启用缩放控制”（与参数名及实际语义一致）。

## 四、已核实、无需修改的点

- [ComPtr.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComPtr.h)：`operator&`/`GetAddressOf` 先 `Reset` 再返回地址，符合 COM“输出参数”惯用法；`AsIID` 对空参数、空对象分别返回 `E_POINTER`、`E_NOINTERFACE`；移动构造后原对象置空。
- [ComCallback.h](file:///e:/develop/nim_duilib/duilib/WebView2/ComCallback.h)：`QueryInterface` 只识别 `IID_TInterface`（单接口回调对象），其余返回 `E_NOINTERFACE`；`Invoke` 参数通过 `std::tuple` 精确还原，0～6 个参数的限制合理。
- [WebView2EnvironmentOptions](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2EnvironmentOptions.cpp)：字符串输出使用 `CoTaskMemAlloc` + `wcscpy_s`，符合 WebView2 接口约定（调用方用 `CoTaskMemFree` 释放）；各扩展接口的 `QueryInterface` 分发正确；自定义协议注册时对每个元素 `AddRef`。
- [WebView2Manager.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2Manager.cpp)：`GetDefaultUserDataFolder` 使用 `webview2_cache/<AppName>` 的目录结构，路径拼接使用 `FilePath`，跨平台正确。
- [WebView2ControlImpl.cpp](file:///e:/develop/nim_duilib/duilib/WebView2/WebView2ControlImpl.cpp)：`CreateEnvironmentAsync → CreateControllerAsync → OnInitializationCompleted` 的分步初始化与失败重试逻辑正确；失败时保留已创建的环境对象，下一次可继续创建控制器。
- 事件注册统一使用 `add_XXX` + `EventRegistrationToken`，`Cleanup` 中先移除所有事件再 `Close()`/释放控制器、WebView、环境，顺序正确；所有跨线程回调都经 `UiBind` 回到 UI 线程。
- 站点图标的下载在后台线程完成解码，再用 `UiBind` 回到 UI 线程回调，线程边界清晰。
- 窗口截图基于 GDI `BitBlt`，得到的 32 位位图不设置 alpha 通道，与 `CEFControl` 目录的同类实现一致（属于既有约定），本轮未改动。
- 控件的 `CapturePreview` 在未初始化时返回 `E_FAIL`，`MakeImageSnapshot` 返回空指针，符合“未初始化时安全失败”的设计。

## 五、建议后续处理（本轮未修改）

1. `Navigate` 的协议补全规则：当前只要字符串中不含 `://` 就补 `https://`，会把 `about:blank`、`data:text/html,...` 之类的地址错误改写（而 `localhost:8080` 依赖当前规则）。建议后续增加“已包含协议前缀则原样保留”的判断，本轮未改以免改变现有输入行为。
2. `WebView2Manager::Initialize` 没有重复调用保护，每次调用都会向 `GlobalManager` 追加一个控件创建回调（目前没有移除接口）。建议后续增加“已初始化”标记。
3. `GetUserAgent`、`CapturePreview`、`DownloadFavIconImage` 等接口未在注释中声明线程约定，建议补充“仅限 UI 线程调用”。
4. `SetInitUrlIsLocalFile(true)` 时，若传入的是不存在的绝对路径，会被当作相对路径拼接资源目录，最终得到 `file:///相对路径`。建议后续区分绝对路径/相对路径的失败分支。
5. `SetUserAgent` 传入空字符串时被忽略，无法恢复 WebView2 的默认 UserAgent；建议后续支持重置。
6. 控件的 `SetAttribute` 依赖窗口上下文（`duilib` 在无窗口时断言），因此单元测试未覆盖属性名的解析分支，建议在集成测试中覆盖。

## 六、单元测试与验证结果

### 测试文件

新增 [tests/test_webview2.cpp](file:///e:/develop/nim_duilib/tests/test_webview2.cpp)，并在 [tests/CMakeLists.txt](file:///e:/develop/nim_duilib/tests/CMakeLists.txt) 中注册测试目标 `test_webview2`：

- 仅在 Windows 下构建（WebView2 是 Windows 专属能力）；
- 测试目标定义 `DUILIB_WEBVIEW2=1`，与 duilib 库的编译宏保持一致，否则看不到 WebView2 的类型定义；
- 不启动 WebView2 运行时、不创建窗口、不访问网络。

### 覆盖范围（共 246 项检查）

| 测试项 | 覆盖内容 |
| --- | --- |
| `ComPtr` | 空指针、接管新对象、`GetAddressOf` 释放旧对象、拷贝/移动构造、从原始指针构造、`Detach`、`Reset(T*)`、`Swap`、`AsIID`（成功/空参数/空对象）、`MakeComPtr` 引用计数为 1 |
| `ComCallback` | 0/1/2 参数回调、成员函数回调、`QueryInterface`（已知/未知接口）、`AddRef`/`Release` 配对；用 `std::weak_ptr` 验证回调对象销毁后捕获资源被释放（引用计数多 1 的回归测试） |
| `WebView2EnvironmentOptions` | `CreateInstance`、`IUnknown` 与 `ICoreWebView2EnvironmentOptions2～8` 的查询、字符串属性（`CoTaskMemFree` 释放）与默认值、布尔属性默认值与读写、自定义协议注册列表（默认空、空数组设置）、空参数校验、未知接口 |
| `WebView2Manager` | 单例、默认配置、全部配置项读写、语言的显式设置优先级与默认语言回退（`SetLanguage` 失效问题的回归测试）、默认网页缓存目录 |
| `WebView2Manager::Initialize` | 配置写入、控件创建回调注册（按控件名创建 `WebView2Control`）、未知控件名返回空 |
| `WebView2Control` | 控件类型、未初始化时的状态查询与默认配置（含 `m_bScriptEnabled` 未初始化的回归测试）、未初始化时 `NavigateBack/Forward/Refresh/Stop/CapturePreview/OpenDevToolsWindow/MakeImageSnapshot` 的安全失败、UserAgent 本地读写、缩放因子设置失败 |
| 初始 URL | 空值、普通 URL、`file://` URL、本地文件模式、反斜杠替换、存在的绝对路径转换为 `file:///` 形式、不存在的相对路径回退 |

### 构建与运行

- 构建：Debug x64、非 DLL 模式（按项目约定），目标 `duilib` 与 `test_webview2` 均编译通过。
- 运行：

```text
1/1 Test #39: webview2 .........................   Passed    0.13 sec
100% tests passed, 0 tests failed out of 1
```

- 回归：`cef_control` 测试同时运行通过。
- 未覆盖：真实 WebView2 运行时的环境创建、页面加载、DOM/脚本执行、事件回调（依赖 WebView2 Runtime、窗口句柄与 UI 消息循环，属于集成测试范畴）。
