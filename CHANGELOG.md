# 更新日志

本项目的所有重要变更记录在此文件中，格式参考
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

版本号由 `duilib/duilib_version.h` 统一维护，格式为
`MAJOR.MINOR.PATCH.BUILD`，其中 `BUILD` 按 `MMDD` 书写且不补零。

## [Unreleased]

### 新增

- 根 CMake 工程与统一构建入口 `python3 build/build.py`：`build`（默认）、`deps`、`test`、`pack`、`doctor`、`clean`；
  构建树统一为 `out/<arch>/<Config>`。
- 依赖自动准备：版本清单 `cmake/third_party_versions.json` 锁定 Skia、SDL3 与 CEF，
  支持下载缓存、sha256 校验、断点续传与并发锁；已有产物可按 stamp 直接采纳。
- Skia 升级到 2026-10-30 的提交，补丁归档为
  `build/skia_patch/skia.2026-10-30.src.zip`。
- Skia 补丁归档随仓库分发：`build/skia_patch/`（清单 `skia.patchDir`）内的归档带有
  `SHA1.txt`，记录它对应的 Skia 提交；`build.py deps` / `build` 与 CMake 依赖脚本在
  使用前校验其与清单 `skia.default` 一致，不一致直接报错。不再克隆外部补丁仓库，
  也不需要联网；归档缺失时用
  `git checkout -- build/skia_patch` 恢复。
- 预检引导（全新环境）：`skia/bin/gn` 只由补丁归档提供，阶段 A 预检在归档尚未解压时
  不再中断配置，改为提示由 `build.py deps` 解压自带工具（归档缺失且确需重建时仍报错）。
- Ninja 自举（VS 未安装可选的 ninja 组件）：自动使用补丁归档自带的 `bin/ninja.exe`。
  `build.py` 依次在 PATH、VS 安装目录、已解压的 `skia/bin/ninja.exe`、补丁归档中查找，
  仍找不到时解压到 `out/.duilib-tools/bin/ninja.exe`；根工程在 `project()` 之前按同样
  顺序自举并写入 `CMAKE_MAKE_PROGRAM`，因此 `cmake --preset` / `-G Ninja` 不再要求
  系统装有 ninja。阶段 A 预检对 ninja 的处理与 gn 一致（归档存在时只提示，不再误报）。
- 补丁校验错误统一：`build.py` 与 CMake 依赖脚本共用同一组 `SHA1.txt` 文案；归档内
  缺少 `SHA1.txt` 时给出可读错误（此前 CMake 侧直接抛出 tar 错误）。
- 旧构建树迁移：配置期清理已废弃的 `DUILIB_SKIA_COMPILE_ROOT` /
  `DUILIB_SKIA_COMPILE_REPO` 缓存项，升级后 `CMakeCache.txt` 不再显示失效路径。
- 网络操作统一重试：`git clone/fetch`、CEF 下载共用版本清单 `retry` 段
  （`maxAttempts=0` 无限重试、`delaySeconds=20`）。
- 双语构建参考手册 `build/build-reference.md` / `.en.md`；`build/README.md` /
  `.en.md` 改为前置依赖与发布打包索引。
- `build/test_build.py`：构建入口与 i18n 的标准库单元测试。
- 合规：`THIRD-PARTY-NOTICES.md`，SDK 包内许可证目录统一为 `LICENSES/`。
- 合规：新增英文版第三方组件声明 `THIRD-PARTY-NOTICES.en.md`（根目录，与中文版成对）；
  `pack.py` 与 SDK 包测试改为同时打包、校验中英文两份声明。
- SDK 接入层（macOS App Bundle）：`nim_duilib_copy_runtime` 新增 `FRAMEWORKS`
  选项（目标须为 `MACOSX_BUNDLE`），把包内 `lib/libSDL3*.dylib` 复制到
  `<target>.app/Contents/Frameworks`，并把可执行文件的 rpath 设为
  `@executable_path/../Frameworks`（`BUILD_WITH_INSTALL_RPATH`，保证 bundle
  内的库优先，避免构建树的 SDK 绝对路径抢先命中）；`nim_duilibConfig.cmake`
  登记 SDL3 dylib 文件名，包内 README 与接入文档同步说明。
- SDK 接入层（macOS App Bundle 资源）：`nim_duilib_copy_runtime` 的 `RESOURCES`
  在 `MACOSX_BUNDLE` 目标上（未指定 `DESTINATION` 时）复制到
  `Contents/Resources/duilib/resources/`，与普通程序的 `<exe 目录>/resources/`
  同构，也是 `GlobalManager::GetResourceRootPath(true)` 定位的标准目录。
- 发布包测试新增 macOS App Bundle 消费者用例 C11：`build/sdk_test/bundle_main.cpp`
  与 `nim_duilib_sdk_bundle` 目标用 SDK 包生成 `sdk_bundle.app`，校验
  `Contents/Frameworks/libSDL3*.dylib`、`@rpath`/`LC_RPATH`、
  `Contents/Resources/duilib/resources`，并把 `.app` 复制到构建树外运行。
- 普通示例的 macOS App Bundle 支持（不修改 C++ 代码）：示例可设置
  `DUILIB_EXAMPLE_MACOS_BUNDLE=ON`，或用根选项
  `DUILIB_MACOS_BUNDLE_EXAMPLES=<目标名列表>` 指定；`examples/basic` 已默认开启。
  Bundle 把资源同步到 `Contents/Resources/duilib/resources`（与普通程序
  同构，资源清单按示例名自动推导），把 SDL3 动态库复制到 `Contents/Frameworks` 并设置
  `@executable_path/../Frameworks` rpath，自动清理切换模式后残留的另一种产物，
  并从 `duilib_version.h` 生成 Bundle 版本号。资源同步由每次构建都会运行的
  `<目标>_bundle_resources` 目标完成（`cmake/duilib_copy_bundle_resources.cmake`
  按内容比较、只复制变化的文件并清理清单外的旧文件），因此只改 XML/图片等资源、
  不改 C++ 源码时 Bundle 内容也会更新，不需要强制重新链接。
- macOS Bundle 资源按示例自动裁剪：资源清单由示例的目标名（`project()` 名称）
  推导，只复制示例自己的主题目录、默认主题公共资源、字体与语言文件，
  示例的 `CMakeLists.txt` 不需要配置；目标名与资源目录名不一致时（如
  `ChildWindow`→`child_window`）也可显式设置 `DUILIB_THEME_DIR_NAME`，显式值优先，
  写错目录会在配置阶段直接报错；`themes/default` 下其他示例的目录不再进包，
  Bundle 资源体积由约 4 MB 降到约 1.3 MB。需要额外资源时可用
  `DUILIB_MACOS_BUNDLE_RESOURCES` 覆盖默认清单。
- `GlobalManager::GetResourceRootPath(bool bMacOsAppBundle = true)` 在 macOS 下按
  参数选择资源根：`true`（默认）返回 `<App>.app/Contents/Resources/duilib/resources/`，
  `false` 返回可执行文件同目录的 `resources/`；非 Bundle 程序仍回退到后者。
  `GetResourceZipPath(bool)` 的 zip 模式对应 `.../duilib/resources.zip`。
- macOS App Bundle 参考文档 `build/macos-app-bundle.md` / `.en.md`，以及
  `Info.plist` 模板与可选的 `.icns` 图标配置。
- Windows MSVC DLL 模式：`python3 build/build.py build --dll` 把 duilib 编为
  `bin/duilib*.dll`（x86 为 `duilib32*.dll`，Debug 带 `_d`，导入库统一为
  `lib/<arch>/<Config>/duilib.lib`）；`--dll` 隐含 `--md`，自动准备 Skia
  `.md`/`.mdd`，同时同步 `msvc/PropertySheets/duilib_dll.flag`。Windows 上 SDL
  默认关闭，DLL 模式也不自动开启；关闭时 `deps` 不下载 SDL 源码、不构建 SDL3，
  需要时显式 `--sdl`。静态 ↔ DLL 模式切换由
  `out/<arch>/<Config>/.duilib-build-mode` 检测，自动清理构建树（含
  `libcef_dll_wrapper` 旧 `/MT` 目标文件）；DLL → 静态还会删除 `bin/duilib*.dll`。
- 发布包 DLL 支持：自动收集 `bin/duilib*.dll`，包名追加 `-dll`，元数据
  `IS_DLL=1`、`MSVC_RUNTIME=MD`；`pack.py` 通过 DLL 导出表确认 SDL 能力
  （DLL 以 SDL 构建时打包必须加 `--with-sdl`），并按 DLL 的实际导入决定是否
  随包 `bin/turbojpeg.dll`。
- Visual Studio 工程支持 Windows ARM64：`build/duilib.sln`、`duilib_no_cef.sln`、
  `examples.sln`、`examples_no_cef.sln` 及被引用的 30 个工程新增
  `Debug|ARM64` / `Release|ARM64`，可在 VS 配置管理器选择 ARM64，或用
  `msbuild ... /p:Configuration=Release /p:Platform=ARM64` 构建。
  `CommonSettings.props` 把 `ARM64` 映射为 `arm64`，与 CMake 产物共用
  `lib/arm64/<Config>`；manifest 使用 `duilib.arm64.manifest`；WebView2 指向
  `build/native/arm64`；SDL3 与 libpag 无 arm64 预编译库，ARM64 下强制关闭
  （libpag ARM64 未测、不跟进）。
  `libwebp.vcxproj` 补入 7 个 NEON 源，`SkGLWindowContext_Windows.cpp` 在 ARM64
  下 `ExcludedFromBuild`（无 WGL）；示例解决方案建议 `/m:2` 以避免 LTCG 链接的
  `C1060`。Release/Debug 均已实测：4 个解决方案全部编译通过（CEF 版各 23 个示例、
  `_no_cef` 版各 19 个示例），46 个示例 exe 为 PE `AA64`。
- 补齐 Windows ARM64 DLL 模式与 CEF 109 验证：`build.py --arch arm64 --dll` 与
  `--dll --config Debug` 生成 `bin/duilib.dll`（9.9 MB）/ `duilib_d.dll`（36 MB），
  导入库 4.15 / 4.17 MB；创建 `duilib_dll.flag` 后 VS `duilib.sln` 的 Release/Debug
  也产出 `duilib.dll`（17.3 MB）/ `duilib_d.dll`（27.3 MB），以上均为 PE `AA64`。
  CEF109 包 `nim_duilib-1.0.2026.1023-win-arm64-cef109-examples.zip`（1131 条目 /
  252.1 MB）通过 `--verify` 与 SDK 包测试（PASS=14 FAIL=0 SKIP=5）。
  Windows on ARM 真机运行验证无设备，标记未测、不跟进。
- 修正 VS 解决方案里 `libcef_dll_wrapper_109` 的冗余工程依赖：解决构建
  `examples.sln` 时该封装库被自动加入链接、而 `LibCefVersion109=0` 只提供
  CEF 142 的 `libcef.lib` 导致的 `LNK2001`（如 `cef_v8value_create_string`）。
  109 封装库仍随解决方案一起编译，`LibCefVersion109=1` 时由
  `CEFSettings.props` 显式链接。
- 新增双语 SDK 使用说明 `docs/Deployment.md` / `Deployment.en.md`（并登记到
  `docs/Summary*.md` 索引、随包收录）：包目录结构与包名含义、免安装解包与
  `SHA256SUMS` 校验、VS（`nim_duilib.targets`）与 CMake（`find_package` +
  `nim_duilib_copy_runtime`）接入、运行期文件清单（`resources/`、DLL、CEF 运行时
  目录、MSVC/MinGW 运行库）、升级步骤、迁移表（源码树 → SDK、静态 → DLL、
  CEF142 → CEF109、单配置 → Debug/Release、x64 → arm64）与 11 条常见问题；
  包内自动生成的 `MANIFEST.txt` 仍是权威清单。
- 新增控件与容器（相对 NIM_Duilib_Framework 的能力增量，属性与用法见 `docs/Control.md`）：
  - `GroupBox`：分组框容器，在左上角显示分组标题，边框圆角、线宽与颜色可配，适合把一组相关控件在视觉上归为一块（示例：`examples/ColorTheme`）
  - `HotKey`：热键输入控件，按下组合键即记录热键，支持与 `Ctrl + C` 形式的显示名互转，用于快捷键设置界面（示例：`examples/controls`）
  - `HyperLink`：超链接文字，可显示链接地址作为 ToolTip，点击时派发 `kEventLinkClick` 事件（示例：`examples/controls`）
  - `IPAddress`：IP 地址输入控件，由 4 段编辑框组成并自动跳转焦点，按 `192.168.0.1` 形式编辑整串地址；支持输入满 3 位自动跳段、`.` 跳段、`↑`/`↓` 按 0~255 增减当前段、`Ctrl+V` 整体粘贴完整地址，Backspace/Delete/方向键与普通文本编辑一致（仅在段首或段尾时跨段），并提供 `IsComplete`/`IsValid`/`ClearIPAddress` 与 `allow_empty` 属性（示例：`examples/render`）
  - `Line`：画线控件，横线/竖线可选，颜色、宽度与五种线型（solid/dash/dot/dash_dot/dash_dot_dot）可配（示例：`examples/render`）
  - `RichText`：富文本显示控件，支持 `<a href>`、`<b>`、`<i>`、`<s>`、`<u>`、`<bgcolor>`、`<font>`、`<br/>` 等类 HTML 标签（示例：`examples/controls`）
  - `Split`：分割条控件/容器，放在 HBox 中左右拖动、VBox 中上下拖动，改变相邻控件的宽高（示例：`examples/ColorTheme`）
  - `TabCtrl`：浏览器式多标签控件，支持图标、关闭按钮、固定标签、内置角标、拖动排序、水平/垂直标签栏与溢出滚动（示例：`examples/controls`）
  - `ListCtrl`：列表/报表控件，提供 Report/Icon/List 三种视图；Report 视图支持表头拖动换序、拖动或双击调整列宽、冻结列、单元格编辑器与列级排序（示例：`examples/ListCtrl`）
  - `PropertyGrid`：属性表控件，按分组展示属性名与属性值，值编辑器覆盖文本、数字、下拉框、字体、颜色、日期时间、IP 地址、热键与文件路径（示例：`examples/render`）
  - `ColorPicker`：拾色器窗口，含标准色、灰色标准色、自定义颜色与屏幕取色，可按背景色求最佳黑白对比前景色（示例：`examples/ColorPicker`）
  - `ComboButton`：带下拉组合框的按钮，左侧主按钮可配上下两行文字，右侧按钮点击弹出下拉组合框（示例：`examples/RichEdit`）
  - `DirectoryTree`：目录树控件，目录枚举在子线程执行避免界面卡死，支持隐藏/系统文件、按路径异步选中展开与异步刷新（示例：`examples/TreeView`）
  - `AddressBar`：地址栏控件，把本地路径拆成可点击的层级按钮，编辑框支持回车/Esc/失焦自动更新显示（示例：`examples/TreeView`）
  - `WebView2Control`：WebView2 浏览器控件，`url` 指定初始地址（可为本地文件），支持页面全屏、开发者工具与导航/后退/前进/刷新等回调（示例：`examples/WebView2`）
  - `GridBox/GridScrollBox`：网格布局容器，`rows`/`columns` 定义网格、子控件可用 `row_span`/`col_span` 合并单元格，GridScrollBox 额外带滚动条（示例：`examples/layout`）
  - `HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox`：流式布局容器，子控件沿水平/垂直方向排列、超出时自动换行，带 ScrollBox 变体还提供滚动条（示例：`examples/layout`）
  - `MenuBar`：菜单栏控件，管理顶级菜单并与对应 Menu 窗口 XML 联动，菜单文本支持多语言 ID（示例：`examples/render`）
  - `IconControl/BitmapControl`：内存位图显示控件：IconControl 直接显示 32 位像素数据，BitmapControl 支持目标/源矩形、等比适应或拉伸与透明度（示例：`examples/render`）
  - `ChildWindow`：子窗口控件，Windows 平台为系统原生子窗口（`WS_CHILD`），其他平台为 SDL 弹出式窗口（示例：`examples/ChildWindow`）
  - `ControlDragableT`：模板类，提供 ControlDragable/BoxDragable/HBoxDragable/VBoxDragable 四个标准控件，支持容器内拖动排序与跨容器拖动（示例：`examples/MoveControl`）
  - `ControlMovableT`：模板类，提供 ControlMovable/BoxMovable/HBoxMovable/VBoxMovable 四个标准控件，支持拖动控件与父容器位置（示例：`examples/MoveControl`）
  - `ControlResizableT`：模板类，提供 ControlResizable/BoxResizable/HBoxResizable/VBoxResizable 四个标准控件，支持拖动调整控件大小（示例：`examples/MoveControl`）
  - `XmlBox`：XML 预览容器，按 `xml_file_path` 加载界面 XML 并在容器内直接渲染，用于界面预览（示例：`examples/XmlPreview`）
  - `Pane/PanelHBox/PanelVBox`：带标题栏的面板容器，标题栏支持多语言与自定义样式，可折叠展开（含动画、手风琴分组）并派发折叠/展开事件（示例：`examples/panel`）
  - `MessageBoxWnd`：自绘皮肤的模态消息框，按位组合确定/取消/是/否/重试按钮，支持多种图标与多语言文本，返回值与 Win32 的 `IDOK`/`IDCANCEL` 等保持一致（示例：`examples/controls`）
  - `ToastWnd`：自绘皮肤的非模态通知窗口，四种图标与五种位置，默认 3000ms 自动关闭，带淡入淡出与上移动画，多条通知自动堆叠（示例：`examples/controls`）
  - `SearchBox`：搜索框组合控件，含搜索图标、编辑框与清除按钮，空态提示、`debounce_interval` 防抖、内置候选项自动完成与搜索历史（示例：`examples/controls`）
  - `SpinBox`：数字输入框，支持步长、范围、小数位、前后缀与千分位分组，可用步进按钮或方向键调整，失焦时按格式重排文本（示例：`examples/RichEdit`）
  - `Switch`：滑块开关，既可用语义色自绘也可配置 SVG 图片皮肤，支持切换动画、轨道内文字/图标与加载中态（示例：`examples/controls`）
  - `Badge`：角标控件，数字角标超过上限显示 `99+`、红点角标可显隐，背景走语义色自动适配深浅色主题，可悬浮在宿主控件角上（示例：`examples/controls`）
  - `Flyout`：通用浮层窗口，可在锚点周围的 12 个方位浮出任意 Box 内容，空间不足自动翻转，也是 CalendarFlyout、AutoComplete 等浮层的公共基座（示例：`examples/controls`）
  - `Calendar`：日历控件，支持单选与范围选择、每周起始日、最小/最大日期与业务禁用日期，提供月/年/十年三级视图与完整键盘导航（示例：`examples/ColorTheme`）
  - `NavigationView`：侧边栏导航容器，左侧导航窗格可在展开与图标条收起宽度之间切换，支持分组/分隔线、底部设置项并与 TabBox 内容页联动（示例：`examples/controls`）
  - `Chart`：轻量图表控件，纯自绘不依赖第三方图表库，支持折线/柱状/饼图三种形态、多系列绑定、增量更新、滚轮缩放拖拽平移与悬停 Tooltip（示例：`examples/controls`）
  - `Marquee`：文本跑马灯，文本超出显示区域时按四个方向无缝循环滚动，支持速度、帧间隔、悬停暂停与手动暂停（示例：`examples/controls`）
  - `Timeline`：垂直时间轴控件，纯自绘不创建子控件，节点支持 pending/inprogress/completed/failed 四态，内容由数据驱动并支持多语言（示例：`examples/controls`）
  - `TagInput`：标签输入框组合控件，内部以流式布局排列标签与编辑框，回车生成标签、点关闭按钮或在空编辑框按退格删除，标签过多自动换行（示例：`examples/controls`）
  - `AutoComplete`：自动完成输入框组合控件，输入达到阈值后在编辑框下方弹出候选浮层，支持前缀/包含匹配、大小写忽略、方向键选择与回车确认（示例：`examples/controls`）
  - `Mask`：遮罩/蒙版层容器，可按控件名在窗口内镂空高亮目标控件并让点击穿透，用于模态背景、新手引导与加载遮罩（示例：`examples/controls`）
  - `StatusBar`：状态栏控件，每个子控件即一个状态面板，相邻面板间自动绘制分隔线，右下角可选尺寸手柄并可参与窗口拖拽缩放（示例：`examples/controls`）
  - `ToolBar`：工具条控件，承载工具按钮与分隔线，可用宽度不足时自动收纳尾部控件并在右端显示"更多"按钮，宽度恢复后自动还原（示例：`examples/controls`）
  - `RangeSlider`：双滑块范围选择控件，在一条轨道上放置下限与上限两个滑块并高亮选中区间，支持最小间距、刻度吸附与点击跳值（示例：`examples/controls`）
  - `Stepper`：向导步骤条控件，节点显示序号或对勾并按当前索引派生状态，支持水平/垂直方向、点击跳转与禁用步骤（示例：`examples/controls`）
  - `Rating`：星级评分控件，支持只读展示、半星、悬停预览与点击/拖拽打分，星形外观由 SVG 图片驱动并随主题换色（示例：`examples/controls`）
  - `Carousel`：图片轮播控件，同一时刻显示一张幻灯片，支持无限循环、自动播放与悬停暂停、切换动画、页码指示点与左右切换按钮（示例：`examples/controls`）
  - `Pagination`：分页器控件，上一页/下一页与页码按钮按数据动态创建，页码超出可见数时以"…"收纳并始终保留首尾与当前页附近（示例：`examples/controls`）
  - `TreeGrid`：树表格控件，在同一控件内呈现可展开的层级行与多列表格，表头固定不滚动并与数据行逐列对齐，支持多选与父子三态级联勾选（示例：`examples/controls`）
  - `Dialog`：通用模态对话框，布局由类在运行时组装而不依赖额外 XML 文件，内容区可指定任意 XML 片段，按钮可标记默认按钮并在关闭前回调校验（示例：`examples/controls`）
  - `SystemNotification`：系统原生通知，SDL3 构建下经 SDL 系统通知 API 发送到系统通知中心，其他平台用 `Shell_NotifyIcon` 托盘气泡实现（示例：`examples/controls`）
  - `Breadcrumb`：面包屑导航控件，一行显示可点击的祖先路径与当前项，点击祖先项切换当前项并派发 `AttachCurrentChanged` 事件（示例：`examples/controls`）
  - `CascadingSelect`：级联选择器，支持"内嵌多列"与"下拉列表框"两种形态，点击某列某行后子列立即过滤为该节点子集并支持任意层级（示例：`examples/controls`）
  - `Popover`：气泡卡片，控件自身即锚点，按 click/hover/focus/manual 四种触发方式弹出带标题与正文的卡片，12 种方位越界自动翻转（示例：`examples/controls`）
  - `Popconfirm`：确认气泡卡片，继承 Popover，点击锚点弹出"图标 + 标题 + 描述 + 取消/确定"按钮行的卡片浮层（示例：`examples/controls`）
  - `Avatar`：头像控件，用圆形或圆角方形展示图片/图标/文字三类头像内容，支持尺寸预设、形状、内容优先级与语义色配色（示例：`examples/controls`）
  - `ChatMessageList`：聊天消息列表控件，消息区纯自绘：气泡左右分栏、宽度自适应换行、文字/图片头像、拖选复制、发送状态与点击重试，并支持图片消息、引用消息（双击引用块跳转高亮）与消息撤回（示例：`examples/chat`）
- 新增示例程序 `examples/chat`：模拟 IM 客户端的完整演示，含登录窗口（提示任意用户名密码即可登录、标题栏可切换主题与语言）与聊天主窗口（会话列表与未读角标、搜索过滤、图片头像、图片/引用/撤回消息、拖选复制、右键菜单），中英双语且随语言切换实时刷新。
- 物理动画（弹簧/惯性）：新增 `duilib/Animation/PhysicsAnimation.h`，内含 `SpringSimulation`（半隐式欧拉、固定小步长积分，用于有明确目标值的过渡）与 `InertiaSimulation`（指数摩擦解析解，用于只有初速度的滑行），并提供全局总开关 `PhysicsAnimation::SetEnabled()` 与默认参数接口。
  - `AnimationPlayer` 支持物理模式：新增 `SetPhysicsType()`、`SetSpringParameters()`、`SetInertiaParameters()`、`SetInertiaValueRange()`、`IsPhysicsEnabled()`，播放中修改结束值会从当前状态平滑过渡（不回跳）。
  - `Panel` 新增 `collapse_physics` 属性（折叠/展开使用弹簧动画），对应 `SetCollapsePhysicsEnabled()` / `SetCollapseSpringParameters()`；`collapse_anim` 默认值由 0 改为 200ms（可折叠面板默认带缓动动画）。
  - `ScrollBox` 及其子类新增 `inertia_scroll` / `inertia_friction` / `inertia_gain` 属性（滚轮结束后按惯性滑行），对应 `SetInertiaScrollEnabled()` / `SetInertiaScrollParameters()`；`inertia_scroll` 默认开启。
  - 关闭物理动画总开关后，以上功能自动回退为原有缓动动画/普通滚动（示例：`examples/controls` 的动画功能演示页）。
- 弹出层淡入动画：新增 `duilib/Animation/PopupAnimation.h`，通过分层窗口 Alpha 由 0 渐变到 255 实现整体淡入，默认开启（`PopupAnimation::SetEnabled(false)` 可关闭）；
  - 全局开关 `PopupAnimation::SetEnabled()`，并可配置时长（`SetTotalMillSeconds()`，默认 120ms）与缓动函数（默认 `EaseOutCubic`）。
  - 已接入 `Flyout`（`Popover`、`Popconfirm`、`CalendarFlyout` 自动继承）、`Menu` 及子菜单、`Combo` 下拉窗口（示例：`examples/controls` 的动画功能演示页）。
- `Carousel` 新增 `transition_easing` 属性（默认 `EaseLinear`，与原有线性切换一致），对应 `SetTransitionEasingFunctionType()` / `GetTransitionEasingFunctionType()`，可把切换动画改为 `EaseOutCubic` 等曲线。
- `GroupBox`（含 `GroupVBox`/`GroupHBox`）内置折叠功能：新增 `collapsible` 属性（默认 `false`，即默认不开启）与 `collapsed` 初始状态，
  开启后点击标题区域即可折叠/展开分组；新增 `collapse_icon_expanded` / `collapse_icon_collapsed` 配置展开/折叠状态的图标（支持 SVG，未配置时绘制默认箭头），
  对应 `SetCollapsible()` / `SetCollapsed()` / `ToggleCollapsed()` / `SetCollapseIconExpanded()` / `SetCollapseIconCollapsed()` 等接口；
  `examples/RichEdit` 左侧演示面板改为使用该内置功能，不再自行维护标题标记与子控件可见性。
- `GroupBox` 折叠/展开增加高度动画：新增 `collapse_anim` 属性（默认 200ms，`0` 表示不使用动画）与
  `SetCollapseAnimMillSeconds()` / `GetCollapseAnimMillSeconds()` 接口；折叠时缓入、展开时缓出，动画过程中子控件按当前高度裁剪绘制，
  支持动画中途反向切换；不修改自身高度模式与子控件可见性，展开后无副作用（示例：`examples/RichEdit`）。
- 默认主题新增可折叠分组样式类 `group_box_collapsible`（`global.xml`，可与其他 `group_box*` 样式类组合使用），
  折叠图标移至公共资源目录 `public/groupbox/`（新增 `PATH_GROUPBOX` 变量），其他示例程序可直接通过 `class` 复用。

### 变更

- 动画默认开启：`PopupAnimation`（弹出层淡入）、`Progress` 的 `animation_enabled`、`Panel` 的 `collapse_anim`（200ms）、`ScrollBox` 的 `inertia_scroll` 默认值统一改为开启，
  业务可按需在 XML 或 C++ 中关闭（`animation_enabled="false"`、`collapse_anim="0"`、`inertia_scroll="false"`、`PopupAnimation::SetEnabled(false)`），关闭后与原行为一致。
- `GroupBox` 视觉细节改进：新增 `text_gap` 属性（默认 4，DPI 自适应），标题文字两侧与边框线之间留出间距，不再紧贴字形；
  修正半透明线条色的计算（此前会同时把 RGB 压暗导致发灰），并让外圈柔光偏移随 DPI 缩放；补充 `GetLineColor()` 接口，清理 `SetAttribute` 中的冗余分支。
- CEF 默认版本升级到 156（`156.0.14+g4fa4ab5+chromium-156.0.8078.48`，2026-10-24），
  109 继续保留供 Windows 7 使用。`duilib/third_party/libcef/` 下的三个平台目录整体
  替换为 156 源码：156 发行包取消了 `lib/` 目录，Windows 导入库改从
  `Release/libcef.lib` 取得（`lib/{x64,Win32,arm64}`）；封装库随上游改用 C++20，
  Windows 上按要求关闭沙箱（避免 `CEF_API_VERSION_LAST` 与 wrapper 的 ABI 差异），
  wrapper 的 C++17 降级只保留给 109。版本清单、VS 工程文件列表、`deps` 指纹与脚本
  默认版本提示同步更新。
- CEF 156 移除了 `CefRegisterExtension`（`CEF_API_REMOVED(15400)`）：JS 桥改为在
  `OnContextCreated` 中向页面全局对象绑定 native 函数，再 `Eval` 注入等价的
  `NimCefWebInstance` 包装代码，Web 端 `call`/`register` 的调用方式不变；109 分支
  仍使用原有扩展注册。
- 依赖准备（CEF 运行时）的“采纳已有产物”规则收紧：Windows 无法从运行时读取版本
  号，改为每次按归档解压覆盖，Linux/macOS 仍在 `include/cef_version.h` 版本匹配时
  才采纳，避免旧版本运行时被写成新版本指纹。
- `build/` 控制台输出改为简易双语（2026-10-29，P3-3）：`build.py`、`pack.py`、
  `release.py`、`autotest.py`、`sdk_package_test.py` 的运行期消息与 `--help`
  文案统一由 `cmake/i18n/messages.json` 提供，按 `--lang` / `DUILIB_LANG` →
  `LC_ALL`/`LC_MESSAGES`/`LANG` → 系统区域 → `en` 判定；仅 `zh`/`zh_CN`/`zh_Hans`
  显示中文，`zh_TW`/`zh_HK`/`zh_Hant` 等繁体区域与所有非中文区域显示英文，与
  `cmake/duilib_i18n.cmake` 的判定保持一致。语言在 argparse 之前解析，因此
  `--help` 与参数错误也按所选语言输出；英文输出为纯 ASCII，非中文控制台不会乱码。
  五个入口均支持 `--lang {auto,zh_CN,en}`（`release.py` 的子命令后也可写），
  `sdk_package_test.py` 调用 `pack.py --verify` 时传递同一语言，保证负向用例的
  预期报错文本匹配。注释、docstring、`MANIFEST.txt`、`RELEASE-NOTES` 与包内中文
  内容保持原样。
- 统一入口一次只构建一个 `--config`（Debug/Release），需要两者时调用两次；
  `build/autotest.py` 的构建步骤改为调用 `build.py`。
- CEF 运行时按版本与架构输出到 `bin/libcef_win[/_109]/<arch>` 或 `bin/libcef_linux`，
  编译使用仓库跟踪的头文件与导入库。所有平台统一使用 `_minimal` 发行包（体积约为
  标准包的一半，含 `include/`、`Release/`、`Resources/`）。
- 依赖准备（CEF）：下载的归档按 CEF 官方发布的 `<下载地址>.sha1` 校验完整性；
  哈希文件与归档一起缓存、离线可复用。拿不到时回退版本清单中的 `sha256`，
  两者都没有时只告警并跳过校验。
- 命令行帮助去歧义：`--sdl`/`--no-sdl`、`--md`、`--vs-version` 在非 Windows 平台上
  标注为仅 Windows 有效；`--no-absolute-rpath` 在 Windows 上标注为仅 Unix。
  补充 `--deps-root`（Skia/SDL3/CEF 的安装位置）、`--jobs`、`--skia-ref`、
  `--sdl-ref` 等参数的用途说明。`--cef-version` 不再硬编码 142，接受 `109` 或
  `latest`（跟随版本清单默认值，清单升级后自动跟随）；`pack.py`、`release.py`、
  `autotest.py`、`sdk_package_test.py` 的同类参数说明同步完善。
- `doctor` 的依赖状态报告更完整：分别列出 Skia 源码与补丁归档、SDL 源码与前缀、
  CEF 运行时，缺失时说明由 `deps` 获取，`--offline` 下则提示无法获取；解释器名称
  按实际命令显示（Linux 上为 `python3`）。
- 依赖版本（SDL3）：清单默认 ref 由 `release-3.4.18` tag 改为 `main` 分支
  （该 tag 的源码当前编译报错）；`release-3.4.18` 仍保留在 `refs` 中，需要时可用
  `--sdl-ref release-3.4.18` 临时切换。
- FreeBSD 构建与打包验证扩展到 Debug/Release 全量（2026-10-29 FreeBSD 虚拟机）：
  `build.py` 的 Release/Debug 构建、`pack.py` 的 4 个包（Release/Debug × 默认 SDK 与
  `--with-examples`）、4 次 `sdk_package_test.py`（含两次 `--run-examples` 的 C10
  示例启动）与 `doctor`/`test`/`test_build`/`test_pack`/`test_sdk_package`/
  `test_autotest`/`autotest` 全部通过；`pack.py --with-cef` 按预期报错
  （FreeBSD 不支持 CEF）。测试矩阵与结论见 `build/test-matrix.md` §4。
- Windows 示例包的包内启动冒烟（C10）已覆盖 5 个包（2026-10-29）：x64 Release 的
  静态 CEF 包（`win-x64-cef-examples.zip`，1318 条目 / 328.2 MB）、SDL 包
  （`win-x64-sdl-cef-examples.zip`，1398 / 352.9 MB）、无 CEF 包
  （`win-x64-examples.zip`，764 / 121.8 MB），以及 x64 Debug/Release 的 DLL + SDL +
  CEF 包（`...-dll-debug.zip` 1398 / 219.6 MB、`...-dll.zip` 1398 / 210.1 MB）。
  各包 `pack.py --verify` SHA 全匹配，`sdk_package_test.py --scenarios C10
  --run-examples` 均为 `[PASS] C10 examples.smoke`（`bin/basic.exe` 启动 3 秒后仍在
  运行）；DLL 回归后已恢复静态构建并清理 `bin/duilib*.dll`。
- 新增 MSVC x86 + SDL3 出包组合（2026-10-29）：`build.py build --config Release
  --arch x86 --sdl` 按 x86 重建 `SDL3-static.lib`（786/786，Skia
  `llvm.x86.release`），`pack.py` 产出 `win-x86-sdl-examples.zip`
  （844 条目 / 128.2 MB，SDL3 79 项 + 21 个示例），`--verify` 844 条目 SHA 全匹配，
  `sdk_package_test.py --run-examples` 报 PASS=16 / FAIL=0 / SKIP=3（含
  `[PASS] C10 examples.smoke`）。打包矩阵 P0-1 的最后一个空格关闭；测试后已恢复
  x64 静态基线并重新构建 x64 `SDL3-static.lib`。
- 许可合规遗留项核对完成（2026-10-29，P3-6）：`docs/misc/License-Compliance.md`
  的 4 条遗留事项全部处理。新增组件登记流程写入 `Release-Checklist.md`
  （`pack.py --list` 对照、`licenses/`、`THIRD-PARTY-NOTICES.md`、对照表与更新记录）；
  确认 SDK 包固定 `DUILIB_LIB_PAG=0`、不收集 libpag 库，启用 libpag 时须随程序分发
  `duilib/third_party/libpag/licenses/**`（含 `tgfx.LICENSE.txt`）；`bin/resources`
  的 409 个文件按来源分层记录（首提交的 144 个上游资源、项目自制的控件/示例资源、
  Lottie/PAG 演示素材），并在 `THIRD-PARTY-NOTICES.md` 增补演示素材说明；
  CEF 官方 109/142 二进制包实测不含 `LICENSES.chromium.html`，估算其体积为 10 MB
  量级（zip 后 1–2 MB），决定默认不随包、需要时按同版本 Chrome/Chromium 手工补充。
- 发布流程 `release.py` 完成全流程演练（2026-10-29，P3-5）：`check` 的版本、工作区、
  tag 与中英 CHANGELOG 四项全部通过；`package --dry-run` 核对 3 个组合的命令与包名，
  实际产出 `nim_duilib-1.0.2026.1023-win-x64.zip`（548 文件 / 164.6 MB，zip 34.3 MB，
  包体检 548 条目 SHA 全匹配）；`finalize` 校验 `dist/1.0.2026.1023/` 下 9 个包
  （2.0 GB）全部通过，生成 `SHA256SUMS.txt` 与中英 `RELEASE-NOTES`；
  `clean --list` 正确列出版本目录且不删除文件；`check` 的脏工作区、已存在 tag、
  版本未严格递增、缺少 CHANGELOG、版本无条目 5 类失败路径，`finalize` 的目录缺失与
  坏压缩包，`clean` 的拒绝删除当前版本、缺少 `--yes`、目录不存在均按预期退出 1。
- 构建消息覆盖补全（2026-11-01）：CMake 侧的 `duilib_preflight_tools.cmake`、
  `duilib_cef.cmake`、`duilib_cef_macos.cmake`、`duilib_macos_bundle.cmake`、
  `duilib_deps_run.cmake` 与根 `CMakeLists.txt` 里剩余的英文提示全部改为消息目录
  （`cmake/i18n/messages.json`，新增 `preflight.*`、`deps.*`、`cmake.*`、`bundle.*`
  等键）；库工程 `duilib/CMakeLists.txt` 也接入消息目录（CEF 头文件目录、源码裁剪、
  CEF 开关与平台提示），其中文/英文输出随 `DUILIB_LANG` 切换。由于消息目录用
  `string(JSON)` 解析，该文件的 `cmake_minimum_required` 由 3.18 提为 3.19（根工程
  入口仍要求 3.24），见 `cmake/README.md` 的「构建消息目录」。
- RichText 功能扩展（可扩展项，第一批）：
  - 标签体系：新增 `<span color>`（局部着色别名）、`<p align>`、`<h1>`~`<h6>`（字号层级与加粗）、
    `<ul>/<ol>/<li>`（项目符号）、`<sup>/<sub>`（上标/下标）、`<hr color>`（水平分割线）、
    `<a title>`（自定义悬停提示，未指定时仍以 URL 作为提示）；
  - 排版能力：新增 `text_align="justify"`（两端对齐，段落的最后一行保持左对齐）、
    `letter_spacing`（字间距）、`text_indent`（首行缩进）、`paragraph_spacing`（段落间距）、
    `max_lines` 配合 `text_overflow="ellipsis|clip"`（限制行数并在行尾显示省略号或直接裁剪）；
  - 链接能力：新增 `visited_link_font_color`（点击后使用“已访问”颜色）、
    悬停/离开事件 `kEventLinkHover`/`kEventLinkLeave`、键盘 Tab/Shift+Tab 聚焦 + Enter 触发、
    右键“复制链接地址”；
  - 运行时接口：`AppendText`（追加富文本，可选择是否立即重绘）、`SetLinkColor`（设置链接各状态颜色）、
    `GetLinkCount`/`GetLinkUrl`/`GetLinkRect`（按索引查询链接数量、URL 与命中矩形）。
- RichText 功能扩展（可扩展项，第二批）：
  - 标签体系：新增 `<img src width height>` 内联图片标签，图片底部与文本基线对齐；
    `width`/`height` 为 96 DPI 下的像素值（省略时使用图片原始尺寸），
    图片高于文本时自动增大行高并保持行内基线一致，支持自动换行，图片位图随文本解析一次并缓存复用。
  - 交互能力：新增文本选择/复制，鼠标在非链接区域按下并拖拽可选择文本（支持跨行、
    自动换行后的多行、对齐与内联图片混排），按行绘制逐字对齐的半透明高亮背景；
    控件获得焦点后支持 `Ctrl+A` 全选、`Ctrl+C` 复制（换行统一输出为 "\r\n"）。
    新增属性 `selection_bg_color`（选中区域背景色）与 `enable_text_select`（是否允许选择，默认 true）；
    新增运行时接口 `HasSelection`/`GetSelectedText`/`CopySelectedText`/`SelectAll`/`ClearSelection`/
    `SetEnableTextSelect`/`IsEnableTextSelect`。
  - 链接无障碍信息：新增 `GetLinkTitle`（链接提示文本，未设置 title 时回退为 URL）、
    `GetHoveredLinkIndex`/`GetFocusedLinkIndex`（鼠标悬停/键盘聚焦的链接索引），
    便于在状态栏等位置展示当前链接；同时修正 RichText 位于滚动容器内时，
    链接的悬停判定、光标形态与鼠标选择未扣除滚动偏移导致命中位置偏移的问题。

### 移除

- 删除 `build/build.md`，构建说明合并到 `build/build-reference.md`。
- 删除旧的分平台构建脚本（`build_duilib_all_in_one.*`、`msvc_build.bat`、
  `linux_build.sh`、`macos_build.sh`、`freebsd_build.sh`、`msys2_build.sh` 等）：
  四个平台均已验证新入口后于 2026-10-29 移除，统一使用 `build/build.py`。

### 修复

- RichText 高优先级问题修复：
  - 内联标签之间的空格被丢弃（如 `Hello <b>World</b>` 显示为 `HelloWorld`、`</a> <a>` 之间的空格丢失）：
    解析时保留空白文本节点，跨节点的空白保留为一个空格，并去掉整段文本首尾的空白；
  - `text_align="center"` 不被识别（示例页的“横向居中”实际左对齐）：支持 `center` 作为 `hcenter` 的别名；
  - 多行文本的 `hcenter/right` 只平移整个文本块、未逐行对齐：绘制时按行宽计算偏移；
  - `<a>` 只取第一个子节点的文本，链接内无法嵌套格式：改为保留子节点并继承 `href`；
  - 未知标签（如 `<p>`、`<span>`、`<h1>`）会丢弃整段文本：改为忽略标签本身并保留子内容；
  - `trim_policy="none"` 仍会替换换行/TAB、`keep_one` 不压缩连续空白：修正为文档描述的语义；
  - `ToString()` 生成非法 XML（属性间缺少空格、未转义）：补齐分隔与转义，支持往返解析；
  - 运行期调用 `AppendTextSlice()` 后不刷新界面：追加后触发重新布局与绘制，并修正移动语义；
    该接口新增 `bRedraw` 参数，解析/批量追加时可传 `false` 避免重复重绘，完成后调用 `Redraw()` 统一刷新；
  - `replace_brace` 会把普通文本（如 `Hello {name}`）误当作标签导致文本消失：仅当替换后都是支持的标签时才解析，
    解析失败时按纯文本降级显示；
  - 鼠标从链接移到空白处时高亮不刷新：Hover 状态变化时统一触发重绘；
  - `link_font_underline` 只在 Hover/按下状态生效：常规状态也显示下划线。
- RichText 中优先级问题修复（性能与健壮性）：
  - 悬停/按下导致整篇重新排版：绘制缓存校验忽略文字颜色，绘制时按片段覆盖颜色；
  - 每帧重复堆分配（文本数据、链接字体）：复用绘制缓冲区，链接字体在解析时生成并共享；
  - 解析时为每个文本节点创建字体对象：无字体属性时共享父节点字体（写时复制）；
  - 同一尺寸下重复测量文本：缓存 `CalcDestRect` 的测量结果；
  - 鼠标按下/抬起时逐链接 `Invalidate`：状态变化时只刷新一次；
  - `<font size>` 未做有效性校验：限制到 1~512；
  - TAB 制表位按窗口绝对坐标计算：改为按行内偏移计算；
  - `word_wrap="false"` 时超长文本直接溢出控件：裁剪到控件区域并显示省略号；
  - 文本恰好占满行宽时多算一行（影响垂直居中）：无后续文本时不再换行；
  - 直接绘制与缓存绘制的背景色处理不一致：无背景色时均跳过绘制；
  - 绘制代码笔误（`glyphChars` 重复判断）与不可靠的行首判断：修正为 `glyphIDs` 与行内坐标判断；
  - `SetText()` 未清除 `text_id`：直接设置文本时清除文本 ID 与语言文件记录；
  - 链接点击事件的 URL 指针指向局部变量：改为直接使用成员中的 URL，避免回调中失效。
- ARM64 上 inline hook 复用了仅适用于 x86/x64 的指令编码（`E9 rel32` 跳转 + libudis86），
  编译期报 `C4311` 指针截断，运行期还会写入非法机器码：`DUILIB_ENABLE_INLINE_HOOK`
  改为只在 `_M_IX86` / `_M_X64` 下定义，ARM64 走原有 fallback 桩（接口保留，
  `Install()` / `Uninstall()` 返回 `false`），x86/x64 行为不变。
- MinGW（gcc 14）构建中自有代码的告警：`ColorConverter::OKLCHToARGB` 的 `r/g/b`
  未初始化即传入转换函数（`-Wmaybe-uninitialized`）、`WindowBuilder::ParseWindowCreateAttributes`
  的 `bIsLayeredWindowDefined` 仅在 SDL 分支使用（非 SDL 构建报
  `-Wunused-but-set-variable`）、`ListCtrl::ExportToCsvFile` 用 `push_back` 逐字节写
  BOM 触发 GCC 的 `-Wfree-nonheap-object` 误报；分别改为默认初始化、非 SDL 分支显式
  `UNUSED_VARIABLE`、以及用 BOM 常量构造 `std::vector`，重新编译后自有代码 0 告警。
- 同时安装多个 Visual Studio 时 `doctor` 的预检显示最新版本、与实际编译器不一致：
  预检此前固定使用 `vswhere -latest`；现在识别 `vcvars` 导出的 `VSINSTALLDIR` /
  `VisualStudioVersion` 并按该主版本过滤，`--vs-version 17` 时显示的是 VS 2022。
- 切换编译器（包括 VS 2022 ↔ VS 2026、MSVC ↔ MinGW）后 `lib/`、`build/build_temp`
  与 `bin/` 中仍残留旧工具集的产物（旧 `.lib` 会被新工具链接入）：`build.py` 现在
  在检测到切换时同时删除 `lib/`、`build/build_temp`、`bin/` 中的 exe/pdb/ilk/dll
  与 `.build-config` 标记（保留 `bin/resources*`、`libcef_win*`、缓存目录与
  `WebView2Loader.dll`），`deps` 在删除构建树之前也会完成清理；VS 工程的
  `duilib_crt_mode.txt` 改为记录 `<PlatformToolset>/<static|dll>`，切换 VS 版本后
  同样删除旧 `.obj/.pch`。
- Windows 多配置下 SDL3 的 Debug/Release 互相覆盖：SDL3 不设置 `DEBUG_POSTFIX`，
  Debug/Release 静态库同名，此前两次 install 到 `<deps-root>/SDL3` 同一前缀，
  后安装的 Release 覆盖 Debug；`/MT`、`/MD` 混链还会报 LNK2038。现在按
  “架构 × 配置 × 运行库”安装到独立变体前缀 `SDL3/<variant>`（MSVC 为
  `msvc.<arch>.<debug|release>[.mdd|.md]`，MinGW 复用 Skia 的 release 变体名、
  Debug/Release 共用一份），命名规则集中在 `cmake/duilib_sdl_variants.cmake`，
  配置期解析、依赖检查与准备、`build/pack.py`、`msvc/PropertySheets/SDLSettings.props`
  共用同一实现；安装 stamp 改记录变体名，多配置生成器下两个配置不再互相判为过期。
  变体缺失时仍回退单前缀并告警（兼容旧布局或显式传入的目录），`doctor` 会按配置
  报告实际使用的 SDL3 前缀。
- 打包在 `--md`（静态库 + 动态运行库）时选错 Skia/SDL3 变体：`pack.py` 此前只按
  `bin/` 是否存在 `duilib*.dll` 判断 `.md`/`.mdd`，`--msvc-runtime md` 未出 DLL
  时仍会打包 `/MT` 变体，下游链接会报 LNK2038。现在按运行库解析
  （`resolve_md_mode`：DLL 模式或 `--msvc-runtime md` 用 `.md`，Debug 为 `.mdd`），
  Skia 与 SDL3 目录规则一致，并补充对应单元测试。
- Visual Studio 多配置生成器下 SDL3 头文件目录未按配置切换：`duilib.vcxproj`
  的 Debug/Release 都写入了 Debug 变体的 `include` 目录，Release 目标看到的
  头文件目录与实际链接的库前缀不一致。现在头文件目录与库目录一样按配置解析
  （`$<$<CONFIG:Debug>:<debug 变体>/include>$<$<NOT:$<CONFIG:Debug>>:<release 变体>/include>`），
  规则由新增的 `duilib_sdl_include_dir()`（`cmake/duilib_sdl_variants.cmake`）
  统一给出，根工程（`duilib_root_options.cmake`）与示例子工程
  （`duilib_common.cmake`、`duilib_example.cmake`）共用；单配置、MinGW 共用
  变体与单前缀回退仍退化为普通目录，并补充对应单元测试。
- Review 修正（与变体方案配套）：
  1. 根工程 `duilib_root_options.cmake` 解析 SDL3 变体前缀时引用了
     `duilib_common.cmake` 才定义的 `DUILIB_MINGW`，MinGW 被当成 MSVC，
     include/前缀解析成 `msvc.<arch>.*`（MinGW 构建里实际写入了
     `-I.../SDL3/msvc.x64.release/include`）。根工程现已自行定义 `DUILIB_MINGW`，
     并补充静态检查用例；
  2. `DUILIB_DEPS_MODE=auto` 时 `duilib_configure_dependencies()` 里的
     `DUILIB_SDL_AUTOBUILD` 只是函数局部变量，子目录看不到；首次配置（变体尚未
     安装）会误报“未找到可用的 SDL3 安装前缀”。改为写到父作用域，并补充夹具用例；
  3. `build.py doctor --sdl` 以 `dll_mode=` 调用已改名的
     `pack._sdl_variant_name`，运行即 TypeError；改为 `md_mode=`，并加入调用点
     关键字与函数签名一致性检查；
  4. 非 Windows 的 SDL3 安装 stamp 不再追加空的变体分隔符，避免升级后无谓重编；
  5. `build.py doctor --sdl` 在 MSYS2 CLANG64 / clangarm64 下按 `gcc.*` 变体检查
     （CMake 侧 clang 走 `llvm.*`）；现在按 `CC` 推导 `llvm` / `gcc` 后再生成变体名
     （CLANG64 → `llvm.x64.release`，UCRT64 → `gcc.x64.release`，
     MINGW64 → `gcc.msvcrt.x64.release`）；
  6. MinGW 回退单前缀的告警照搬了 MSVC 文案（提示“Debug 目标会报 LNK2038”），
     MinGW 的 Debug/Release 共用一份静态库，不会发生 CRT 冲突；改为按
     `DUILIB_MINGW` 输出不同告警；
  7. 多配置生成器下 `DUILIB_SDL_SRC_ROOT_DIR` 由含 `$<CONFIG:...>` 的 include 目录
     推导，`get_filename_component` 会得到被表达式截断的无效路径；现在遇到生成器
     表达式时直接取已解析的变体前缀（该变量只作为示例子工程的兜底值）；
  8. `pack.py` 的 `_sdl_variant_name()`（MinGW 分支）调用 `_skia_candidate_names`
     时未透传 `windows`：显式 `windows=True` 的调用（`doctor`、测试）在非 Windows
     宿主上会退回非 Windows 的名字；已透传，测试也显式传入
     `windows=True` / `msys2_env=''`；
  9. `pack.py` / `doctor` 原先只按“规范变体名”解析 SDL3 前缀：打包侧的 MSYS2 判定
     只能依据 `sys.executable` / `MSYSTEM`（Git Bash 也会设置 `MSYSTEM`），与 CMake
     侧（编译器安装位置）可能不一致，导致找不到依赖脚本实际安装的变体目录。
     两者现在都按候选顺序逐个探测（首个为规范变体名，最后为 `<root>` 单前缀回退），
     并补充单元测试；
  10. MSYS2 判定在 `duilib_root_options.cmake` 与 `duilib_common.cmake` 各有一份
     实现；统一为 `duilib_skia_toolchain.cmake` 的 `duilib_detect_msys2()`，
     避免两处口径漂移；
  11. 打包侧的 MSYS2 判定与编译侧补齐同一口径：`pack.py` 的 `_msys2_env_name()`
     此前只看 `sys.executable` 与 `MSYSTEM`，从 cmd/PowerShell 打包 MSYS2 MINGW64
     产物时得到空环境名，候选里缺 `gcc.msvcrt.<arch>.release`，Skia 直接报
     “Skia lib dir not found”、`--with-sdl` 找不到 SDL3。现在依次尝试 `CC`/`CXX`、
     `PATH` 上的 gcc/clang、Python 解释器所在目录（判定方式与 `duilib_detect_msys2`
     相同：`<root>/<env>/bin` 且存在 `<root>/usr/bin/pacman.exe`），最后才回退
     `MSYSTEM`；仍无法确定时把 `gcc.msvcrt.<arch>.release` 作为候选**末尾兜底**，
     不影响独立 MinGW-w64 / UCRT64 的优先级；
  12. `duilib_crt_id()` 此前只看 `$ENV{MSYSTEM}`：Git Bash 同样设置
     `MSYSTEM=MINGW64`，同一个构建在不同 shell 里会得到不同 CRT 标识，进而误报
     `CRT mismatch`（FATAL）或无谓重编。现在有编译器路径时一律按
     `duilib_detect_msys2()` 的安装位置判定（MINGW64/MINGW32 → `<env>-msvcrt`，
     其余 → `<env>`，非 MSYS2 → `mingw-unknown`），仅在脱离配置直接调用时才回退
     `MSYSTEM`。升级后独立 MinGW-w64 若在 Git Bash 下配置过，旧 stamp 与
     `mingw-unknown` 不符，会要求用相同工具链重建一次依赖；
  13. `build.py doctor --sdl` 的报告口径：此前把“已安装但不是首个候选”的变体
     也报成 `[WARN] … missing; using …`。现在区分“变体目录命中”（`[OK]`）、
     “回退 `<deps-root>/SDL3` 单前缀”（`[WARN]`）与“尚未构建”（`[INFO]`）；
  14. `build.py pack` 不转发 `--deps-root`：用自定义依赖根编译后打包仍去读仓库同级
     的 `skia/`、`SDL3/`，可能打进旧产物或直接报找不到。`pack.py` 新增
     `--deps-root`（映射为 `<root>/skia`、`<root>/SDL3`，显式
     `--skia-lib-dir`/`--sdl-dir` 仍优先），`build.py pack` 总是传入解析后的依赖根；
  15. 同名参数的相对路径语义不一致：`build.py --deps-root` 按当前目录解析，
     `pack.py --deps-root` 却按仓库根解析，直接运行 `pack.py` 且工作目录不是仓库根
     时会指向另一个目录。`pack.py` 改为与 `build.py` 相同的“按当前目录解析”
     （`os.path.abspath(os.path.expanduser(...))`），并补充对应用例与帮助文案。
  文档同步：`CM-5` 的 I-5 与 D-4b 条目更新为“Debug/Release 各自变体前缀”，
  `msvc/PropertySheets/README.en.md` 的 SDLSettings 说明补齐 `SDL3RootDir`/`SDLVariant`，
  中英文属性表文档补上 `Imm32.lib`，`docs/misc/Root-CMake-Build-Plan.md` 的就绪判定
  改为按工具链区分（MinGW 不涉及 LNK2038），`cmake/README(.en).md` 的 Skia 输出目录
  列表改为真实变体名（`llvm.<arch>.<config>` 等）。
- Skia 变体目录名的规则统一到 `cmake/duilib_skia_variants.cmake`：配置期解析
  （`duilib_common.cmake`）、off 模式检查（`duilib_deps.cmake`）、依赖准备
  （`duilib_deps_run.cmake`）共用同一函数，不再各自拼目录名。`build/pack.py`
  的打包侧候选目录名也改为同一规则，并新增一致性用例逐一比对两侧
  （`build/test_cmake_scripts.py`），防止再次漂移。顺带修正 MinGW Debug 打包
  会漏掉 `.debug` 变体；`gcc.msvcrt` 只在 MSYS2 MINGW64 作为候选，不再把
  不兼容的 msvcrt 库打进 UCRT64/独立 MinGW 包。
- CRT 标识推导统一：新增 `cmake/duilib_crt.cmake` 的 `duilib_crt_id()`，
  `duilib_root_options.cmake`（写入依赖 stamp 的 `DUILIB_CRT_ID`）与
  `duilib_common.cmake`（读取依赖目录 `.duilib-crt` 后比对）共用同一实现，
  不再有两份可能分叉的判定。
- 多配置生成的 Debug 目标找不到 Skia：依赖准备此前只按 `DUILIB_CONFIG`
  （默认 Release）准备一份 Skia，而配置期会按 Debug/Release 逐一校验；
  现在多配置生成器下两份都准备（单配置仍只准备当前配置），并新增
  `deps.skia_multi_config` 提示。
- 构建测试不再依赖 PATH 上有 cmake：`build/build.py` 新增 `find_cmake()`
  （`DUILIB_CMAKE_EXECUTABLE` → PATH → Visual Studio 自带 CMake），配置/构建/
  `doctor`/测试共用；缺少 cmake 时相关用例不再被跳过。`build.py test` 除脚本级
  单元测试外，还通过 `build/test_cmake_scripts.py` 执行一次真实 configure 冒烟
  （prepare 模式，不依赖 Skia）。
- Debug 配置的依赖检查：`cmake/duilib_deps.cmake` 在 `DUILIB_DEPS_MODE=off` 时用
  `DUILIB_BUILD_TYPE` 判断 Skia 变体，但该变量由 `duilib_common.cmake` 定义、只在
  示例子工程里才被包含，根工程执行检查时仍为空，于是 Debug 被当成 Release：
  `skia/out` 下只有刚编译好的 `llvm.x64.debug` 时报
  `no usable Skia library directory ... Tried: llvm.x64.release`，Windows MSVC 的
  `build.py --config Debug` 因此无法完成配置。现在按 `CMAKE_BUILD_TYPE` 自行推导
  Debug/Release（与 `duilib_common.cmake`、`duilib_deps_run.cmake` 一致），并补上
  `--md` 对应的 `.debug.mdd` / `.release.md` 变体名；报错信息里的候选目录改为逐行列出。
- 依赖准备（Skia 补丁重新解压）：Windows 上补丁归档更新后需要整体重新解压时，
  归档内的 `bin/ninja.exe` 正被当前构建使用，解压会报
  `archive_write_header(): Can't unlink already-existing object` 并中断依赖准备。
  `cmake/duilib_deps_run.cmake` 现在把 `bin/` 下的构建工具从常规解压中排除：
  目标缺失时补上，内容与归档一致时原样保留，内容不同时先解到临时目录再替换
  （仍被占用时只告警并继续使用现有文件），源码文件照旧覆盖。
- MinGW-w64 GCC 16 编译 Skia：把工具链从 15.1.0（mingw-w64 v12）升到 16.2.0（mingw-w64 v14）
  后出现两处编译失败，补丁归档内的 Skia 源码已做兼容（GCC 15 与 MSVC 行为不变）：
  - 新版 `dwrite_3.h` 定义了 `NTDDI_WIN11_ZN`，同时提供的是早期版本的 `IDWritePaintReader`
    （接口带 `struct_size` 参数、`DWRITE_PAINT_ELEMENT` 布局也不同）。`SkScalerContext_win_dw.cpp`
    里按最终 Windows SDK 写的 COLRv1 代码因此被条件编译选中并报错（`MoveToFirstChild` /
    `GetGradientStop` 等参数不匹配）；现在 MinGW 下统一走原有的降级实现（与旧版 mingw-w64 一致）。
  - GCC 16 编译 `ml3` / `ml4` 变体（AVX2 / AVX512 的 lowp stage）时报
    `internal compiler error: in convert_move, at expr.cc:226`：由 16 字节内存加载经
    `__builtin_convertvector` 得到的 32 字节像素值在 MS x64 ABI 下按值传给下一个 stage 时，
    expand 阶段做了跨模式移动。`SkRasterPipeline_opts.h` 在 MinGW + GCC 16 的 AVX2/ML4 变体里
    让转换结果先物化到向量寄存器（空 `asm` 约束），编译与运行恢复正常。
- 依赖准备（Skia 升级）：本地 Skia 源码仍带着旧版本补丁在工作区里的修改时，
  `git checkout` 会因“本地修改将被检出操作覆盖”直接失败（依赖准备中断）。
  `cmake/duilib_deps_run.cmake` 在切换版本、重新解压补丁前先用 `git stash` 保存
  已跟踪文件的修改（不再 `reset --hard` / `clean -fd`），用户的手工修改与未跟踪
  脚本都不会被删除，需要时可用 `git stash list` / `git stash pop` 找回；补丁随后
  从归档重新应用。
- 构建消息语言（中文环境）：`cmake/duilib_i18n.cmake` 被根 CMakeLists、preflight、
  deps 等重复 include 时，文件开头的默认值 `DUILIB_I18N_LANG=en` 会覆盖已解析的
  `zh_CN`，导致 `project()` 之后的配置与依赖准备消息（含新增的补丁校验提示）全变成
  英文。现在默认值只在首次初始化时写入；复测 `[OK] 目标架构 x64`、`[OK] Skia 已就绪`
  等消息恢复中文。
- 构建（切换 Debug/Release）：示例程序都输出到共享的 `bin/`，目标文件却按配置
  分开存放；先编译 Debug 再编译 Release 时，`bin/` 中的文件比 Release 的目标
  文件新，Ninja 判定链接步骤已是最新（`ninja: no work to do`），`bin/` 一直是
  Debug 产物。现在 `build.py build` 在链接前根据 `bin/.build-config/<目标>`
  （兼容旧版 `bin/<目标>.build-config`）删除其他配置的示例产物及标记，强制当前
  配置重新链接；`--target X` 只清理该目标，`--no-examples` 跳过清理。
- macOS 示例 App Bundle（切换 Debug/Release 后启动崩溃）：`build.py` 的跨配置清理
  会整删 `bin/<目标>.app`，而 `Contents/Info.plist` 由 CMake 在 configure 阶段生成、
  链接阶段不重建；重新链接后 bundle 缺 `CFBundleIdentifier`，LaunchServices 解析不到
  当前进程的 bundle，SDL 初始化时 `+[UNUserNotificationCenter currentNotificationCenter]`
  断言抛异常，且断言位于 `dispatch_once` 内，`@try/@catch` 无法捕获。现在只删除
  `.app` 内 `Contents/MacOS` 下的产物，保留 `Info.plist`、`PkgInfo` 与资源；
  cef/CefBrowser 为同构 Bundle，同样受此修复保护。
- 发布包测试（macOS C10 示例启动）：`extract_package` 用 `open(..., "wb")` 解包，
  文件权限固定为 0644，Unix 上丢掉可执行位，`bin/basic.app/Contents/MacOS/basic`
  直接启动时报 `PermissionError: [Errno 13]`。现在按归档条目的权限位恢复（tar 用
  `member.mode`，zip 用 `external_attr >> 16`），Windows 上没有 POSIX 权限位则跳过。
- 发布包测试（Linux 磁盘空间）：编译器的临时文件原先落在系统临时目录，`/tmp`
  被写满（常见为 1–2 GiB 的 tmpfs）时 gcc 报 `error closing ...s: 设备上没有空间`，
  C01/C03 会被误判为「C++ 编译器不可用」「配置失败原因不正确」。现在把
  `TMPDIR`/`TEMP`/`TMP` 指到 `--out-dir/<NN>/work/tmp`，并在启动时按包大小估算
  所需空间、不足时打印 `[WARN]` 提示改用更大的 `--out-dir`。
- 发布包测试（macOS P04/P06 误报）：`sdk_package_test.py` 原先只在
  `bin/libcef_win*`、`bin/libcef_linux` 中查找 CEF 运行时，macOS 的 CEF 位于示例
  `.app/Contents/Frameworks/Chromium Embedded Framework.framework`，导致包名 `-cef`
  与实际能力不一致，且把随包的 CEF 头文件当成“未声明 CEF”。现在 macOS 的 `-cef`
  只表示 CEF 示例 Bundle 随包，`HAS_CEF` 按“SDK 是否提供 CEF 开发文件”判定。
- 打包（Windows arm64 CEF）：`pack.py` 原先对 Windows arm64 的 `--with-cef` 直接报错，
  实际上游 `cef_builds` 已提供 `windowsarm64` 预编译包（142 与 109 都有）。现在按
  `lib/arm64/libcef.lib` 收集导入库、按 `bin/libcef_win[_109]/arm64/` 收集运行时，
  `third_party_versions.json` 补上 109 的 `windows/arm64` 归档，`test_pack.py` 的
  「拒绝」用例改为「收集」用例；arm64 包内清单同步说明可用 `--with-cef` 出包。
- 构建（Windows arm64 工具链探测）：`build.py` 原先对 `vcvarsall.bat` 固定传 `arm64`，
  该参数要求 `Hostarm64` 原生工具集；x64 主机默认只装 `Hostx64\arm64` 交叉工具，
  于是已安装 ARM64 生成工具时仍误报 `Visual Studio C++ tools for arm64 are not installed`。
  现在按主机架构选择 `amd64_arm64` / `x86_arm64` / `arm64`，并新增 `_vcvars_platform`
  的 4 项单元测试；`build.py doctor --arch arm64` 可正确识别 Ninja 与 VS 工具链。
- 渲染（Windows arm64 构建失败）：Skia 上游在 `_M_ARM64` 下把 `SkWGL_win.cpp` 整体
  排除，`GrGLMakeNativeInterface()` 返回 nullptr，ARM64 没有 WGL 后端，链接时报
  `SkCreateWGLContext`、`SkWGLExtensions::*` 无法解析。`SkGLWindowContext_Windows.cpp`
  与 `Render_Skia_Windows` 现在在 ARM64 下不编译/不调用 GL 后端：`MakeGLForWin`
  直接返回 nullptr，Skia 渲染固定走 CPU，其他架构行为不变。
- 构建（Windows arm64 manifest）：`cmake/duilib_bin_windows.cmake` 原先只按 32/64 位
  选择清单，arm64 示例链接的是 `duilib.x64.manifest`（ARM64 进程无法启动）。现在按
  `DUILIB_TARGET_ARCH_RESOLVED` 优先选 `duilib.arm64.manifest`，`cmake/README*.md`
  的代码片段同步更新。
- SDK 接入（arm64 交叉编译）：Ninja 在 x64 主机交叉编译 arm64 时
  `CMAKE_SYSTEM_PROCESSOR` 仍是主机架构，`nim_duilibConfig.cmake` 把消费者工程判为
  x64 并拒绝 arm64 包。新增 `CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "ARM64"`
  回退分支后，arm64 包的下游配置/编译恢复正常。
- 发布包测试（交叉架构消费者）：x64 主机运行 arm64 消费者会报
  `OSError: [WinError 216]`，C01 被误判为 FAIL。`sdk_package_test.py` 新增
  `_can_run_arch()`（x64 主机可经 WOW64 运行 x86，反向不可），配置/编译/链接通过后
  按架构 SKIP 并注明「运行验证需 arm64 环境」，另加 5 项单元测试。
- 打包（macOS CEF 头文件）：`--with-cef` 此前在 macOS 也转发
  `include/duilib/duilib_cef.h`、`include/duilib/CEFControl/`，但这些头文件依赖
  未随包的 `third_party/libcef` 头文件，无法用于下游编译，且与元数据 `HAS_CEF=0`
  和发布文档矛盾。现在 macOS 不转发 CEF 头文件；包名中的 `-cef` 只在
  cef/CefBrowser 示例 App Bundle 确实进包时追加。
- 发布包测试（C07/C08/C09 耗时）：三个场景此前把整个解包目录重新压缩成 tgz /
  tar.bz2，Debug CEF 包可达数 GB，单场景会持续十几分钟且 Python 占满一个核
  （并非死循环）。现在只从真实包挑选最小头文件、最小静态库与运行资源生成代表性
  小归档，并重建 `SHA256SUMS`；判定逻辑不变，耗时与包体积无关，完整包仍由
  P01–P03、P07 覆盖。
- 依赖准备（SDL3）：本地 `<deps-root>/SDL` 源码停在别的分支/标签（例如 `main`）时，
  脚本此前直接用它编译，完成标记却只记录清单里的 ref 名称，导致用错版本编译的库被
  当成清单版本的产物复用。现在 `deps` 先按清单把工作区切到指定 tag/commit（本地
  已有该提交时不联网，`--offline` 同样可用），并在 `.duilib-sdl` 中记录源码提交；
  切换后旧产物自动重建（升级到本版本时会重建一次 SDL3）。
- 依赖准备（Skia 补丁，机制后续已调整）：本地补丁归档此前只要文件名存在就直接复用，
  外部仓库重复提交同名归档（旧版本）时，旧补丁会让旧的 Skia 产物一直不更新；旧实现
  在使用前核对归档与外部仓库的提交。现在补丁归档随仓库提交到 `build/skia_patch/`，
  以 `SHA1.txt` 与清单提交校验，见上文。
- 依赖准备（Skia 源码）：`--skia-ref` 选中的 tag/branch 此前不会检查本地工作区，
  现在与 commit ref 一样按清单对齐（branch 只要求当前就在该分支上，不追远端新提交）。
- 构建（Windows 链接依赖）：Skia 与 SDL3 的静态库此前只以库名（配合
  `link_directories`）传给 `target_link_libraries`，Ninja 不会把它们记为链接依赖，
  依赖重新编译后示例程序不会重新链接，包里可能留下用旧库生成的二进制。现在
  各平台统一按库文件绝对路径链接静态库（MSVC 的 `SDL3-static[d].lib`、MinGW 的
  `libSDL3.a`、各平台的 Skia `.a`/`.lib`）；共享库（Unix 的 SDL3 `.so`/`.dylib`）
  仍保留库名由 `-l` 解析，避免把构建机绝对路径写进 dylib 的 `install_name`/rpath。
- 构建（Windows 示例）：Debug 默认开启增量链接，并行链接多个示例时偶发
  `LNK1000: Internal error during IncrBuildImage`，旧 `.ilk` 损坏后还要手工删除；
  示例程序统一加 `/INCREMENTAL:NO`，链接耗时代价很小、构建结果稳定。
- 构建（Windows DLL / libjpeg-turbo）：DLL 模式改用动态 `turbojpeg.lib` 导入库，
  构建后把 `turbojpeg.dll` 复制到 `bin/`，不再链接按 `/MT` 编译的
  `turbojpeg-static.lib`，消除 `LNK4098`（`/MT` 与 `/MD` 目标文件冲突）。
- 构建（MSBuild 运行库切换）：`CommonSettings.props` 在每个工程 `IntDir` 记录
  `duilib_crt_mode.txt`，切换 `duilib_dll.flag` 后自动删除旧 `.obj/.pch`
  （含 CEF wrapper），修掉 VS 增量构建不跟踪编译选项、仍链接旧 `/MT` 目标文件的问题。
- 发布包测试（Windows 长路径 / x86 运行库）：解包时去掉归档顶层包目录、包工作
  目录改用短序号名，避免 CEF 头文件与 MSBuild `.tlog` 超过 `MAX_PATH`
  （`FileNotFoundError`、`MSB6003`）；MSVC `/MD` 包在运行消费者前把 VS 2022
  运行库复制到 exe 同级，避开系统 `SysWOW64` 旧运行库导致的 x86 崩溃。
- 发布包测试（DLL 包 JPEG 误判）：`infer_capabilities()` 只看 `lib/`，DLL 包
  实际只有运行期 `bin/turbojpeg.dll`，导致 P04 报“元数据 HAS_JPEG=1、实际文件
  为 0”，并连带 C02 消费者按错误能力配置失败；现在静态包看
  `lib/turbojpeg*.lib`、DLL 包看 `bin/turbojpeg.dll`，新增对应单元测试。
- 打包（Debug/Release 同名示例）：CMake 链接后在隐藏目录
  `bin/.build-config/<目标>` 记录构建配置（升级前写在 `bin/<目标>.build-config`，
  `pack.py` 仍兼容读取，重新链接时会自动删除旧文件）；打 Debug 包时按标记跳过
  Release 构建的示例，缺少标记时提示无法确认，不再笼统告警“可能混有 Release 版本”。
- 依赖准备（SDL3）：Debug 与 Release 生成的 `SDL3-static.lib` 同名且安装到同一个
  前缀，原先两个配置都构建、Release 最后安装，Debug 构建实际链接的是 Release 版
  静态库（`/MTd` 与 `/MT`、`/MDd` 与 `/MD` 混用）。现在单配置生成器（`build.py`
  默认的 Ninja）只构建并安装当前配置，完成标记也记录配置，切换 Debug/Release 时
  自动重建；多配置生成器（Visual Studio）仍需构建两份，会打印提示说明 Debug
  链接的是 Release 静态库。
- 构建（CMake 示例列表）：`DUILIB_EXAMPLES` 把推导结果写进缓存，关闭
  `DUILIB_ENABLE_CEF` 后再配置时仍沿用上次含 `cef`/`CefBrowser` 的旧列表，
  链接阶段报 `CefManager::GetInstance` 等符号无法解析。现在每次配置都重新推导
  平台默认列表（只有与默认值都不同、且非上次推导结果时才视为用户显式覆盖），
  根工程另加一道保护：`DUILIB_ENABLE_CEF=OFF` 时直接跳过 CEF 示例并提示。
- `build.py`：`DUILIB_ENABLE_CEF` 也是有状态的缓存项，用
  `--cmake-arg=-DDUILIB_ENABLE_CEF=OFF` 关掉一次后，后续不带该参数的构建仍沿用
  旧值。现在与其他开关一样每次显式传入（Windows MSVC、Linux、macOS 为 ON，
  MinGW/MSYS2 与 FreeBSD 为 OFF），用户仍可用 `--cmake-arg` 覆盖。
- 打包（SDK 接入）：`NimDuilibPackageRuntimeDlls` 之前把
  `bin/libcef_win[_109]/<arch>/` 下的 CEF 运行时 DLL 文件名摊平列入，下游用
  `nim_duilib_copy_runtime` 时会尝试复制不存在的 `bin/<name>` 而构建失败。
  现在元数据只登记 bin 根目录下的 DLL；`nim_duilibConfig.cmake` 也会按文件是否
  存在过滤，旧包同样可用。CEF 运行时仍按文档由下游整体复制对应子目录。
- 构建（macOS CEF 示例）：`cef.app`/`CefBrowser.app` 未把 SDL3 动态库放进
  `Contents/Frameworks`，也没有指向该目录的相对 rpath，只能靠构建机的 SDL3
  绝对路径运行。现在构建后由 `duilib_copy_sdl_bundle.cmake` 复制
  `libSDL3*.dylib`（保留版本链接），主程序 rpath 设为
  `@executable_path/../Frameworks`（Helper 为 `@executable_path/../../..`）；
  自备静态 SDL3 时自动跳过复制。
- 构建（macOS）：CEF 二进制分发包通常只带 Release 框架，Debug 构建复制
  `Debug/Chromium Embedded Framework.framework` 时会失败。现在配置期检测
  `Debug/` 与 `Release/`，Debug 框架缺失时自动回退到 Release 并打印提示；
  两者都不存在时在配置期直接报错，提示检查 `CEF_ROOT`。
- 构建工具：`autotest.py`、`sdk_package_test.py` 的默认输出目录改用系统临时目录
  （`tempfile.gettempdir()`），移除写死的 `C:\develop\temp`、`/tmp`，换机器无需改代码；
  `README`、`README.en.md`、`sdk-package-test.md` 中的相关示例同步更新。
- 依赖准备（Windows）：Skia 使用的 LLVM 根目录不再写死 `C:/LLVM`，改为按
  `-DDUILIB_LLVM_ROOT` → `LLVM_ROOT` 环境变量 → `C:/LLVM` → `Program Files/LLVM`
  → Visual Studio 自带 LLVM 的顺序探测。
- 构建（macOS）：示例程序链接时 ld64 报 “ignoring duplicate libraries”（`-lpthread`、
  `-lSDL3`、Skia 库等被重复列出，链接器本身会忽略重复项）。现在在链接器支持时添加
  `-Wl,-no_warn_duplicate_libraries`，消除该告警。
- 示例（controls）：`TabCtrlForm.h`/`TabCtrlForm.cpp` 缺少文件末尾换行、
  `TimelineForm.cpp` 存在只自增未使用的 `nIndex`，分别触发 `-Wnewline-eof` 与
  `-Wunused-but-set-variable`；已补齐换行并删除无用变量。
- 构建（Windows / CEF）：主工程使用 C++20，而 `libcef_dll_wrapper` 按上游固定使用
  C++17，编译命令行同时带 `/std:c++20` 与 `/std:c++17`，产生 D9025 警告。现在为
  wrapper 目标（含 109 变体）单独设置 `CXX_STANDARD 17`，不再覆盖全局标准。
- 依赖准备（UOS）：与旧脚本一致，统信 UOS 上固定使用 `gcc/g++`（Skia 与主工程一致），
  避免 clang++ 在该平台的链接错误。
- 依赖准备：Skia 补丁归档增加“已应用”校验与内容指纹（`skia/.duilib-patch`），补丁与
  基线提交未变化时不再重复解压，避免刷新源码 mtime 导致整棵 Skia 重编。
- 依赖准备（CEF）：缓存归档 sha256 不匹配时自动删除并重新下载；通过
  `DUILIB_CEF_ARCHIVE` 指定的本地归档仍直接报错，避免误删用户文件。
- 依赖准备：curl 下载增加 `--speed-limit 100 --speed-time 120`，连接后长时间低速
  传输会中断本次尝试并按统一参数重试，避免无限重试下永久挂起。
- `build.py`（MSYS2）：校验编译器目标为 mingw-w64（优先 clang、其次 gcc），否则给出
  明确错误，避免误用 MSYS 目标编译器产出非 Windows 程序。
- 依赖准备：Skia 源码仓库改为与旧脚本一致的 `https://github.com/google/skia.git`，
  原先的 `skia.googlesource.com` 在部分网络下无法访问；Skia/SDL 的克隆地址统一从
  版本清单读取，不再在脚本里硬编码。
- `build.py`：写在子命令之前的选项（如 `--config Debug build`）会被子命令默认值覆盖，
  现已在子命令侧抑制默认值。
- `build.py`：同一 `out/<arch>/<Config>` 切换编译器（MSVC ↔ MinGW）时复用旧 CMake
  缓存导致配置失败；现在检测到编译器变化会自动清空该构建目录。
- 依赖准备：Unix 上会误选 Skia 目录里的 Windows 版 `gn.exe`/`ninja.exe`；且回退到
  PATH 查找时结果变量已被置空，`find_program` 直接跳过搜索，最终报“找不到 gn”。
  现在只在 Windows 选用 `.exe`，Unix 使用本机工具并用独立临时变量查找 PATH。
- `build.py`：`vcvarsall` 输出的 `Path` 与继承的 `PATH` 并存，导致工具查找读到旧值。
- `build.py doctor`：预检以 `DUILIB_DEPS_MODE=off` 运行时整体跳过依赖工具检查，输出的
  环境摘要里没有 `gn`。现在 `gn` 始终检查，doctor 按 `prepare` 严格校验（缺失即失败）；
  `build --no-deps` 的 `off` 模式下缺失只告警，不阻断复用已有依赖的构建。
- 预检：非 Windows 平台新增编译器检查，要求 `gcc/g++` 或 `clang/clang++` 至少一组成对
  可用；有 `CC`/`CXX` 时按环境变量检查，否则先查 gcc、再查 clang。
- 依赖准备：Skia 的构建脚本固定调用 `python3`（不认 `python` / `py -3`），而预检此前
  接受 `python` 或 `py -3`，导致问题到编译 Skia 时才暴露。现在缺少 `python3` 时预检
  给出告警，并在实际编译 Skia 前报错终止。
- 文档与命令示例：macOS/Linux/FreeBSD 通常只有 `python3`，按旧文档用 `python` 调用
  `build/build.py` 会报 command not found；示例已统一为
  `python3 build/build.py`。
- 依赖准备（macOS）：SDL3 已成功编译安装后仍报 `SDL3 build failed`；就绪检查只认
  `libSDL3.so`，而 macOS 的安装产物是 `libSDL3.dylib`。现在 macOS 平台按 dylib 判断。
- CEF 示例（macOS）：各 Helper 子目标未应用 duilib 的 include/link 目录，编译时报
  `fatal error: 'duilib/duilib.h' file not found`；现在 Helper 目标与主目标使用相同的
  目录设置和链接依赖。
- 根 CMake 工程（macOS）：CEF wrapper（`libcef_dll_wrapper`）未随工程编译，而 CEF 示例
  链接时需要它；现在 macOS 也会构建 wrapper，产物输出到 `lib/<Config>`。
- `build.py build --arch arm64`：目标架构的 MSVC 工具集未安装时，vcvarsall 仍返回成功，
  CMake 只会报“C compiler identification is unknown”；现在初始化环境后检查 `cl`，
  缺失时直接提示 `Visual Studio C++ tools for arm64 are not installed` 并给出安装位置。
- `build.py clean`：删除构建树遇到文件被占用时抛出 Python 异常堆栈；现在输出
  带路径的明确错误，并提示关闭占用程序后重试。
- `build.py build --md`（MSVC）：Skia 目录候选没有区分 MSVC 运行库，`--md` 仍选到
  `/MT` 编译的 `llvm.<arch>.debug`，配置期报 CRT mismatch；现在 Debug 选
  `llvm.<arch>.debug.mdd`、Release 选 `llvm.<arch>.release.md`，`deps` 的候选命名同步修正。
- `--md`（MSVC）生成的运行库表达式错误：`MultiThreadedDLL$<$<CONFIG:Debug>:Debug>`
  在 Debug 展开为非法值 `MultiThreadedDLLDebug`，Generate 阶段直接失败；改为
  `MultiThreaded$<$<CONFIG:Debug>:Debug>DLL`（根工程、duilib、zlib、libpng、libwebp、
  cximage 与文档示例同步修正）。
- Skia 目录选择（MSYS2 CLANG64）：`llvm.<arch>.debug` 这类目录名与 MSVC 产物重名，
  但其中只有 `.lib`；现在 MinGW 只接受包含 `.a` 的候选目录，会按顺序回退到
  `mingw64-llvm.<arch>.release`，避免配置期误判后报“没有 .a 静态库”。
- CEF 示例（macOS/Linux）：wrapper 的链接名 `cef_dll_wrapper` 与目标名
  `libcef_dll_wrapper` 不一致，`target_link_libraries` 不会为它建立构建依赖，并行构建
  时 Helper 会在 wrapper 生成前链接，报 `ld: library 'cef_dll_wrapper' not found`；
  现在 wrapper 目标额外提供同名 ALIAS `cef_dll_wrapper`，链接侧按其引用即可获得绝对
  路径与构建依赖（链接名保持不变，不影响打包）。
- 根 CMake 工程：libpng/zlib 的目标名由 `png_static`/`zlibstatic` 改为 `duilib-png`/
  `duilib-zlib`，与库文件名一致。此前 `DUILIB_LIBS` 按库文件名链接时不会自动建立构建
  依赖，并行构建可能在库文件生成前就执行链接，报 `library 'duilib-png' not found`；
  改名后 CMake 会自动使用库文件的绝对路径并建立依赖。libpng 以内嵌方式构建时关闭
  安装导出（`SKIP_INSTALL_ALL`），避免其链接非导出目标 `duilib-zlib` 导致生成失败。
- Skia 编译工具链与主工程解耦：非 Windows 平台优先使用 `clang/clang++`（GCC 版 Skia
  运行性能明显较差），仅当 clang 不可用时回退 `gcc/g++`；构建产物目录相应为
  `out/llvm.<arch>.release` 或 `out/gcc.<arch>.release`。
- 修复首次构建 SDL3 成功后仍报 `SDL3 build failed`：完成标记（`.duilib-sdl`）原先在
  就绪检查之后写入，首次安装时校验必然失败；现在先写入再校验。
- Unix 示例程序改用相对 rpath（`$ORIGIN` / `@loader_path`，含 `../lib` 与
  `libcef_linux`），打包后优先加载包内 SDL3/CEF，不再复用构建机的绝对路径；
  打包脚本按可执行位识别 Unix 示例程序（原先只认 `.exe`）。
- 打包未包含 CEF 模块（未指定 `--with-cef`）时，自动跳过 `cef` 与 `CefBrowser`
  示例程序（Windows 为 `cef.exe`/`CefBrowser.exe`），避免包内留下必然启动失败的
  二进制；包内 README 同步说明。
- 打包（Linux）：CEF wrapper 的 Unix 产物被上游 CMake 去掉 `lib` 前缀
  （`PREFIX ""`），文件名为 `cef_dll_wrapper.a`，不在 `pack.py` 的收集白名单中，
  `--with-cef` 时报“缺少 libcef_dll_wrapper”。现在归一化时把无前缀名映射回
  白名单名，包内文件保持原名（下游 `nim_duilibConfig.cmake` 两种写法都接受）。
- 依赖准备（Unix）：SDL3 的完成标记包含目标配置，而 Unix 上 SDL3 固定按 Release
  构建共享库、与 duilib 的 Debug/Release 无关，切换配置会无谓地重新编译 SDL3；
  现在配置分量仅 Windows 保留。
- 依赖准备（CEF）：`_duilib_cef_tracked_dir` 原先不区分平台，恒返回
  `libcef_win[_109]`，Linux/macOS 的版本比对与迁移采纳错对 Windows 的跟踪头；
  现在按平台返回 `libcef_linux` / `libcef_macos` / `libcef_win[_109]`。
- 打包支持 macOS 的 CEF 示例 App Bundle（`bin/cef.app`、`bin/CefBrowser.app`）：
  `--with-examples` 时与其他平台一致由 `--with-cef` 联动控制——未指定则跳过并告警，
  指定后整个目录（含 `Contents/Frameworks` 内的 CEF Framework、Helper 与符号链接）
  按原结构收集。macOS 的 `--with-cef` 只控制示例，不收集 CEF 开发文件与运行时
  （Bundle 自带 Framework），原“直接报错”的行为移除。
- CEF 示例 App Bundle（macOS）：程序链接的是动态 SDL3（构建时 `SDL_SHARED=ON`），
  但 `duilib_cef_macos.cmake` 既没有把 `libSDL3*.dylib` 放进 `Contents/Frameworks`，
  也没有设置指向它的 rpath，能运行只是因为 rpath 中残留构建机的 SDL3 绝对路径，
  换目录/换机器即加载失败。现在构建后自动把 dylib 复制进 Bundle，并为主程序设置
  rpath `@executable_path/../Frameworks`、为 Helper 设置
  `@executable_path/../../..`（Helper 的 exe 在
  `Contents/Frameworks/<Helper>.app/Contents/MacOS`），打包时随 Bundle 一并收集。
- CEF 只由 `cef` 与 `CefBrowser` 示例链接：此前根工程开启 `DUILIB_ENABLE_CEF`
  时所有示例都链接 `libcef`（Linux 下 `basic` 等程序的 `ldd` 出现 `libcef.so`）。
  现在由示例级 `DUILIB_EXAMPLE_CEF` 控制，`DUILIB_ENABLE_CEF` 只决定库是否编译
  CEF 控件源码。

- CEF 109（Windows CMake）：根工程固定编译 `libcef_win`（最新版）的 wrapper，
  且 109 的 wrapper 目标名仍为 `libcef_dll_wrapper`，与链接方期望的
  `libcef_dll_wrapper_109` 不一致（会链接失败或误用最新版源码）；现在根工程按
  `DUILIB_CEF_VERSION` 选择源码目录，109 的 wrapper 目标名改为
  `libcef_dll_wrapper_109`。
- CEF 109（Windows CMake，Debug）：`USE_SANDBOX=ON` 时 wrapper 会定义
  `_HAS_ITERATOR_DEBUGGING=0`，与 Debug 主工程默认的 `_ITERATOR_DEBUG_LEVEL=2`
  冲突（LNK2038）；本项目不链接 `cef_sandbox.lib`，现在 109 配置期强制
  `USE_SANDBOX=OFF`。
- `build.py pack --sdl`：`--sdl` 只是编译开关，包内不会包含 SDL3（需要
  `--with-sdl`）；现在给出提示，避免误以为 SDL3 已随包。
- `build.py --dry-run`：`pack` 与 `test` 子命令此前忽略该选项、直接真实执行；
  现在与其他子命令一致，只打印将要执行的命令。
- `pack.py`：同一 `lib/<arch>/<Config>` 中同时存在 MSVC `.lib` 与 MinGW `.a`
  时，工具链判断会误选（切换工具链后旧库不会被自动清理）；现在打包直接报错，
  提示先清理该目录，避免打出错误的包。
- MinGW 示例链接：Debug 示例单个体积可达 200 MB 以上，并行链接多个示例时
  `ld` 峰值内存很高（8 GB 机器上易报 `out of memory`）；现在 GNU ld 追加
  `-Wl,--no-keep-memory` 降低峰值内存（内存仍紧张时可用 `--jobs 2`）；
  clang/CLANG64 使用的 lld 不识别该参数，保持原样。
- 参数校验：`--skia-ref` / `--sdl-ref` 传入清单中不存在的键时，不再笼统报
  “third_party_versions.json 内容无效”，而是指出具体键与修正方式；
  `--cef-archive` 指向不存在的文件时立即报错，不再等到 CMake 配置阶段。
- `pack.py --with-cef`：库目录缺少所选版本的 wrapper 时（常见于编译用了
  `--cef-version 109`、打包时漏加同一参数），错误信息会指出库目录中实际的
  CEF 版本，并提示打包时应使用的 `--cef-version`。
- `pack.py --with-examples`（Windows）：`bin/` 由多个架构共用，打包 x86 SDK 时
  会把残留的 x64 示例程序一并收进包；现在按 PE 头的 Machine 字段校验架构，
  不一致的 exe/dll 会被跳过并阻止出包（可清理 `bin/` 或加 `--allow-missing`）。
- `build.py`：`DUILIB_MD`、`DUILIB_BUILD_EXAMPLES`、`DUILIB_ENABLE_SDL`、
  `DUILIB_CEF_VERSION`、`DUILIB_SKIA_REF` 等有状态选项此前只在传入时设置，
  复用同一个 `out/<arch>/<Config>` 时旧取值会残留在 CMake 缓存中（例如先跑
  `--no-examples` 后，默认构建不再编译示例）；现在每次调用都显式传入本次解析
  后的取值，构建结果只由当前命令行决定。
- 依赖准备：SDL3 的构建目录按“架构-配置”共享，与工具链无关。切换工具链或运行库
  （MSVC ↔ MinGW、/MT ↔ /MD）后会复用旧 CMake 缓存并继续调用上次的编译器，
  例如 MinGW 下 `--sdl` 仍调用 `cl.exe` 并因缺少 MSVC 环境而失败。现在按
  `.duilib-sdl` stamp 判断，CRT 不一致时先清空 SDL3 构建目录再重新配置。
- `build.py --dry-run`：`build` 与 `clean` 在只打印命令、未实际执行的情况下仍输出
  “编译完成/已删除”；现在 dry-run 不再打印完成提示（与 `pack`/`test` 一致）。
- SDK 测试消费者工程（`build/sdk_test`）：`NIM_DUILIB_SDK_DIR` 支持省略与相对路径。
  解包后把 `sdk_test` 放进 SDK 根目录并执行
  `cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./` 时，原先的相对路径被按构建
  目录解析，`find_package(nim_duilib CONFIG)` 找不到
  `lib/cmake/nim_duilib/nim_duilibConfig.cmake`。现在以 `sdk_test` 自身、它的
  上一级/上两级、构建目录及其上一级/上两级为基准解析相对路径（取第一个命中），
  `sdk_test` 与解包目录并列时写 `-DNIM_DUILIB_SDK_DIR=./nim_duilib-<版本>-<平台>`
  也能直接工作；省略时按常见布局自动探测；找不到时报错并列出所有尝试过的目录。
- 单元测试（macOS）：`test_build.SkiaPatchTests.test_existing_repo_is_pulled` 用临时
  目录的原始路径做比较，而 `build.py` 的 deps root 会 `resolve()` 成真实路径；macOS 上
  `/var` 是 `/private/var` 的符号链接，两者不相等导致测试失败。断言改为按
  `Path.resolve()` 后的路径比较，Windows/Linux 结果不变。
- 链接（libjpeg-turbo 3.2.x + duilib-zlib）：libjpeg-turbo 3.2 起，上游预编译的
  `turbojpeg-static.lib` 内嵌 spng + zlib（用于 `tjLoadImage`/`tjSaveImage` 的 PNG
  支持），链接 `turbojpeg.c.obj` 时会一并拉入内置的 `zutil.c.obj`；其内置 zlib 虽
  已按 `Z_PREFIX` 改名，但 `zutil.c` 中的 `z_errmsg` 不在 `zconf.h` 的前缀列表内，
  与本工程的 duilib-zlib 重复定义，静态链接时报 `LNK2005`（3.1.x 的内置库不含 zlib，
  升级前不出现）。现在给 duilib-zlib 增加编译宏 `z_errmsg=duilib_z_errmsg`
  （内部符号，不在 `zlib.h` 公开 API 中，改名不影响功能），CMake
  （`duilib/third_party/zlib/CMakeLists.txt`）与 VS 工程
  （`duilib/third_party/zlib/win32/zlib.vcxproj`）两套构建同步修改；最小工程同时链接
  `duilib-zlib.lib` 与 `turbojpeg-static.lib` 验证通过。2026-10-29 又用 SDK 包测试的
  C01 消费者用例复验（重新构建 `lib/x64/Release/duilib-zlib.lib`——旧库早于修复提交，
  仍带原符号）：`PASS=15 FAIL=0 SKIP=4`；x86 与 arm64 的 `duilib-zlib.lib` 也已重新
  构建，三个架构的符号表均为 `duilib_z_errmsg`、无裸 `z_errmsg`。
- 构建消息（重复状态标记）：`build.py doctor` 缺 cmake 时输出成
  `[WARN]  [WARN] 未找到 cmake ...`（包装函数自己加前缀，文案又带了一次）；
  `deps.skia_patch_tool_in_use` 也自带 `[deps]` 标记，与 `[WARN]  ` 前缀重复。
  现在文案统一不带标记，并由新增单元测试
  `test_build.MessageCatalogTests.test_markers_not_duplicated` 静态扫描 CMake 与
  Python 调用点，发现“文案自带 `[OK]/[WARN]/[INFO]` 且由包装接口输出”即失败。
- 构建文档与实现对齐：`cmake/README.md` / `README.en.md` 的 in-tree 编译片段改为
  当前实现（`cmake.in_tree_build` + Hint），Q1 的平台报错改为
  `unsupported platform`（旧版为 `Unknown OS`）；`build/build-reference.md` 的
  macOS CEF 回退日志改为中文渲染并在括号内注明英文原文，`build-reference.en.md`
  保持英文；`build/sdk-package-test.md` 补充说明该用例工程的提示固定为中文。
  另外在 `cmake/README*.md` 的「构建消息目录」中列出有意不接入目录的位置
  （内部守卫、构建标记脚本、键值 dump、随包分发的 `packaging/cmake/*`），
  并在 `Packaging-Follow-Up-Items.md` 新增 P2-6 记录包内提示语言的决策。
- macOS App Bundle 的 zip 资源模式找不到压缩包：`GetResourceZipPath()` 由
  `GetResourceRootPath()` 的父目录派生，而 zip 模式下 Bundle 内没有 `resources`
  目录，根路径会回退到“可执行文件目录 + resources/”，父目录就成了 `bin/`，
  结果去 Bundle 外面找 `resources.zip`。现在 macOS 下先按
  `<App>.app/Contents/Resources/duilib/resources.zip` 定位，存在才采用，否则保留
  原有回退逻辑；非 Bundle 程序行为不变。
- 构建文档与实现继续对齐：`preflight.dev_packages_hint` 改指
  `build/linux-dependencies.md`（系统包清单见 `build/build-reference.md` 第 8 节）；
  `duilib_preflight_tools.cmake` 的 Linux 系统库注释由“可选，缺失只告警”改为
  实际行为（`pkg-config` 缺失只告警，库缺失一次性报错）并删除未使用的变量；
  `cmake/README*.md` 的 ninja 自举与阶段 B 预检描述改为实际查找顺序与校验项；
  `build/README(.en).md`、`build-reference(.en).md`、`linux-dependencies(.en).md`、
  根 `README(.en).md` 及 AI 指南统一把 Python 要求写为 3.8+（与 `build.py` 一致）。
- 新增 `build/test_docs.py`：`build.py test` 现在会校验维护范围内 Markdown 的相对
  链接（先剔除代码块与行内代码）以及 `docs/` 顶层文档的中英成对，历史目录
  `docs/misc/` 不参与；`docs/Summary(.en).md` 中指向已迁移头文件的链接同步修正。
- 测试文档与实现对齐：`build/sdk-package-test.md` 的 CEF 维度取值由 CEF 142 改为
  当前默认 156（`pack.py --cef-version` 仍提供 `latest`/`109`），前置表的 Python
  要求写为 3.8+，P02 的“必需文件”由 10 项改为 `check_layout()` 实际的 11 项，
  并补上遗漏的 `--lang` 参数说明、把 `--keep` 的删除目录写法改为实际使用的
  `<out>/<NN>`；`build/arm64-test-plan.md` 的现状表不再把
  CEF 109 出包记为“未测”（W-A07/W-P25 已通过），并注明 142 是出包当时清单的
  默认版本、当前默认已升至 156。
- 构建文档补全：`build/README(.en).md` 的包名规则补上 `-mingw-gcc|-mingw-llvm`
  与 `-dll`（与 `pack.py:build_package_name()`、`docs/Release(.en).md` 一致）；
  `cmake/README(.en).md` 目录树里 `duilib_bootstrap.cmake` 的说明改为实际查找顺序
  （VS 安装目录由 `build.py` 查找，脚本本身不含该来源），阶段 A 预检的工具清单
  补全 make / cc/cxx / gn / curl-wget。
- 只装 VS 2026（或同时装 VS 2022 与 VS 2026）时 Visual Studio 生成器名仍写死：
  `build.py` 在没有 Ninja 时固定回退 `Visual Studio 17 2022`，`sdk_package_test.py`
  也用同一字面量判断“本机没有 MinGW 编译器”，并在负向架构用例里显式传入该生成器名；
  只装 VS 2026 的机器会配置失败，MinGW 包也从 SKIP 变成 FAIL。新增
  `build.py:vs_generator_name()`，按安装路径
  （`...\Microsoft Visual Studio\{2017|2019|2022|18}\...`）映射
  `Visual Studio 15 2017` … `Visual Studio 18 2026`，无法识别时沿用 2022 的名字；
  `detect_toolchain()`、`build_block_reason()`（改用 `startswith("Visual Studio")`）
  与 `check_negative_arch()` 统一使用它，并新增 `test_build.VsGeneratorTests` 3 项、
  `test_sdk_package.BuildBlockReasonTest` 4 项；`docs/Release(.en).md`、
  `build/sdk-package-test.md`、`TRAE.md`/`WORKBUDDY.md` 里的 VS 生成器示例也注明
  生成器名随本机 VS 版本变化（VS 2022 为 `Visual Studio 17 2022`，VS 2026 为
  `Visual Studio 18 2026`）。
- 消息目录只有“引用了不存在的键”检查，反向的“死键”（`messages.json` 里留下但
  源码不再引用的条目）不会被发现：新增 `MessageCatalogTests.test_no_orphan_keys`，
  要求每个键名都作为字符串出现在 `cmake/**/*.cmake`、根与 `duilib/CMakeLists.txt`、
  `build/*.py`（不含测试）；键名经表格等数据传递（如 `SCENARIO_ORDER` 的
  `sdk.scn_p01`）同样算作引用。当前 706 个键全部有引用。
- 构建参考手册补漏：`build/build-reference(.en).md` 的“常用参数”表新增 pack 子命令的
  `--pack-arg`（原样追加到 `pack.py` 命令行），示例为 `--pack-arg=--list` 与
  `--pack-arg=--allow-missing`，与 `build.py` 的实现一致。
- 发布编排脚本 `build/release.py` 的包名与体检口径修正：`package` 预判包名时没有传
  `is_dll`，DLL 模式下 `--dry-run` 显示的包名缺 `-dll`；打包后的体检又只按这个预测
  文件名打开归档，而 macOS 的 `-cef` 需要真的收集到 CEF 示例 App Bundle、交叉出包时
  PE 架构不符的 DLL 也不会入包，预测名与实际包名可能不同，体检会以“包不存在”失败。
  新增 `_expected_runtime_dll()`（与 `pack.collect_duilib_runtime_dll()` 同规则：
  x86 为 `duilib32[_d].dll`、Debug 带 `_d`）把 DLL 模式传给
  `build_package_name(is_dll=...)`；体检改为检查“输出目录中本次新增的归档”，仍无
  新增时才报新消息 `rel.pack_output_missing`。新增 `build/test_release.py`（15 项，
  覆盖包名预判、归档筛选、`git status` 路径解析与 `dist/` 忽略规则、CHANGELOG 条目
  提取），并在 `build/README(.en).md` 文件表与 `docs/Release(.en).md` 发布流程中
  同步说明。
- SDK 接入文档补漏：`docs/Release(.en).md` 的 VS 属性表新增 `NimDuilibUseManifest`
  （默认 `1`，`0` 时不按工程 Platform 嵌入 `msvc/manifest/duilib.<arch>.manifest`）；
  `build/README(.en).md` 的 autotest 段落补充 `--smoke` 与 `--build-timeout`
  （冒烟只对 Release 预设执行、示例架构与主机不一致时跳过；构建超时默认 3600 秒）。
- `--lang` 说明补漏：`docs/Release(.en).md` 的 `pack.py` 参数表与
  `build/README(.en).md` 的 autotest 段落都未提到该选项（五个入口实际都支持）；
  现补充语言判定顺序（`--lang` / `DUILIB_LANG` → `LC_ALL`/`LC_MESSAGES`/`LANG` →
  系统区域 → `en`）、重复给出时以最后一个为准，以及 `release.py` 的子命令同样支持。
- 控制台语言在脚本间不一致：`build.py pack`、`release.py package` 与 `autotest.py`
  启动的 `pack.py` / `build.py` / 单元测试不继承已解析的 `--lang`，只能自己按环境变量
  与系统区域判定，中文系统上 `--lang en` 会中英混排；`release.py` 进程内复用的
  `pack.verify_package()` 也读 `pack` 模块自己的 `MESSAGES`。现由 `build.py pack`
  与 `release.py package` 显式转发 `--lang <已解析语言>`，`autotest.py` 通过
  `DUILIB_LANG` 传给每个子进程，`release.py` 在 `main()` 中同步 `pack.MESSAGES`。
  另外五个入口的 `_requested_lang()` 预扫描原先取第一个 `--lang`，而 argparse 以
  最后一个为准（`--lang en build --lang zh_CN` 时 `--help`、预览命令与实际解析结果
  不一致），现统一改为“最后一个生效”；新增 4 项用例
  （`test_build.PackCommandTests.test_language_is_forwarded_to_pack`、
  `test_release.PackageLanguageTest`、`test_autotest.ChildEnvironmentTest` 2 项）。
- MinGW 的 Skia 变体按配置各编一份：`deps` 对 MinGW 的 Debug/Release 使用同一组
  GN 参数（`is_debug=false`，没有 `/MTd` 之类的运行库切换），而候选列表又把
  `<前缀><gcc|llvm>.<arch>.debug` 排在前面，于是 Debug 先编一份、Release 再编一份
  （每种各约 6 分钟与一份额外磁盘占用），Debug 编出的那一份 Release 也认不出来。
  现在 MinGW 的 Skia 候选与配置无关，统一为 `<前缀><gcc|llvm>.<arch>.release`，
  配置期、`deps`、`pack.py` 与 SDL3 变体命名口径一致；已有的 `.<arch>.debug` 目录
  不再被自动选用（需要时用 `--skia-lib-dir` / `-DDUILIB_SKIA_LIB_SUBPATH` 显式指定）。
  `build/build-reference(.en).md`、`build/README(.en).md`、`cmake/README(.en).md`
  的说明与 `build/test_cmake_scripts.py` 的期望值同步更新。
- `deps --dry-run` 在构建树即将重建时仍按旧缓存展开计划：编译器切换（MSVC ↔ MinGW）
  或运行库/DLL 模式切换（`--md`/`--dll`）时，真正的 `deps` 会先清空构建树再重新配置，
  而 dry-run 仍会调用 `duilib_deps_plan`，打印出即将被丢弃的旧工具链/旧模式的
  `gn`、`ninja` 参数。现在 `cmd_deps()` 把 `reset_stale_build_dir()` 与
  `reset_stale_build_mode()` 的结果传给 `_print_deps_plan()`，构建树要重建时只打印
  新键 `deps.plan_reset` 与提示，不再进入计划目标（新增
  `test_build.DepsPlanPrintTests.test_reset_pending_skips_plan`）。
- `--dry-run` 的状态消息原来用完成态措辞（“正在清理”“已同步”），但 dry-run 并不删除/写入：
  `build.compiler_outputs`、`build.config_switch`、`build.dll_flag`、
  `build.toolchain_changed`、`build.mode_changed` 改为“将清理/将删除/将同步”，
  真实运行与 dry-run 都成立（中英同步）。
- `build/build-reference(.en).md` 的“`--dry-run` 本身不会配置”与实际不符：计划目标经
  `cmake --build` 执行，`CMakeLists.txt`/`cmake/*.cmake` 比构建系统新时 CMake 会先重跑
  一次配置（只更新 `out/` 构建目录，不做依赖准备、不联网、不编译）。
  文档改为区分“dry-run 不主动配置”与“CMake 自身因配置过期而重跑配置”。
- `docs/Release(.en).md` 的 VS 接入只写了“静态库全套或 DLL 导入库 + 系统库”，没有列出
  属性表按包元数据自动补齐的链接项：现在补上系统库清单（SDL 包再加
  `Version`/`Winmm`/`Setupapi`/`Hid`/`Mincore`/`Dinput8`，WebView2 包加 `WinInet`）、
  静态包的 Skia 与可选库及 `duilib*` 全套、DLL 包只链 `duilib.lib`，以及静态 + CEF 的
  `/DELAYLOAD:libcef.dll` 与静态 Debug 的 `/NODEFAULTLIB:"libcmt.lib"`。同时说明
  `NimDuilibSystemLibs` / `NimDuilibLinkLibs` 是属性表内部赋值，Import 前覆盖无效，
  额外库请用 `AdditionalDependencies` 追加。

## [1.0.2026.1023] - 2026-10-23

### 新增

- 发布打包脚本 `build/pack.py`：按 SDK 契约收集已构建产物，生成 zip/tgz、
  `MANIFEST.txt`、`SHA256SUMS` 与包内 `VERSION`。
- 发布流程编排脚本 `build/release.py`：提供 `check`、`package`、`finalize`、`clean`
  四个子命令，不执行 git 写操作。
- 包内接入层：VS 的 `msvc/nim_duilib.props`、`.targets` 与 x64/x86 manifest；
  CMake 的 `lib/cmake/nim_duilib/*.cmake`，提供 `nim_duilib::duilib`。
- 顶层使用文档随 SDK 打包，并按包内布局重写 Markdown 链接。
- `--with-cef` 同时打包 CEF 运行时：保持 `bin/libcef_win[_109]/<架构>/` 结构，
  含 `libcef.dll`、`*.pak`、`icudtl.dat`、`locales/` 等（Windows）。
- `MANIFEST.txt` 与发布文档标注 WebView2 依赖系统安装的 Microsoft Edge
  WebView2 Evergreen Runtime，SDK 仅提供 loader；补充动态/静态 loader 与
  常见问题说明。
- 发布文档与 `MANIFEST.txt` 增加 Windows 下的包校验命令
  （PowerShell `Get-FileHash`、`certutil -hashfile`），无需安装 `sha256sum`。
- 许可合规：补充 `licenses/expat.LICENSE.txt`（Skia 静态链接的 expat，MIT）
  与 `licenses/RobotoMono.LICENSE.txt`（资源字体，Apache-2.0），并新增
  `docs/misc/License-Compliance.md` 随包组件许可对照表。
- 发布相关文档：`docs/Release.md`、`docs/misc/Release-Checklist.md` 与打包工作记录。

### 变更

- 版本号统一到 `duilib/duilib_version.h`，`msvc/dllRes/version.h` 改为 RC 适配层。
- 资源目录调整为 `bin/resources/`，与示例程序同级。
- 包名追加特性后缀：`-sdl`、`-cef` / `-cef109`、`-examples`、`-debug`，
  同一版本的不同特性包可共存于 `dist/`。
- `docs/` 只打包顶层使用文档，`misc/`、`specs/` 等子目录不随包。
- 归档语义：tar 按源文件权限写入（保留可执行与 setuid 位），zip 用 `S_IFLNK`
  条目保存符号链接；Unix 包解包后 `.so` 链接链与示例程序的可执行权限得以保留，
  符号链接不再计入 `SHA256SUMS`。
- 支持可复现构建：设置 `SOURCE_DATE_EPOCH` 后统一归档条目、gzip 头与
  `MANIFEST.txt` 的打包时间；未设置时按源文件/产物 mtime 生成并排序条目，
  输入不变时连续两次打包摘要一致。

### 修复

- 修复 macOS 上根目录 `VERSION` 与标准头 `<version>` 冲突导致的编译失败。
- SDK 接入层（VS 属性表与 CMake Config）在 Debug 下补充
  `/NODEFAULTLIB:"libcmt.lib"`，消除下游链接的 LNK4098 告警。

### 移除

- 移除根目录 `VERSION.txt`，避免版本数据出现第二份来源。
