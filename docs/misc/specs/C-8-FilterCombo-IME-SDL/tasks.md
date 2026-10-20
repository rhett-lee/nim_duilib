# FilterCombo 下拉展开时无法输入中文（SDL 后端）Tasks（C-8）

- [x] T1 修复 `NativeWindow_SDL::CreateWindowAndRender`（唯一改动点）
  - [x] T1.1 在调用 `CreateSdlWindow` 前记录键盘焦点窗口：`SDL_Window* pPrevFocusWnd = SDL_GetKeyboardFocus();`
  - [x] T1.2 在 `m_sdlWindow` 创建成功（非空校验通过）后，以 `#if defined (DUILIB_BUILD_FOR_WIN)` 保护插入 Stop/Start 往返：
    条件 `(pPrevFocusWnd != nullptr) && (pPrevFocusWnd != m_sdlWindow) && SDL_TextInputActive(pPrevFocusWnd)`，
    动作 `SDL_StopTextInput(pPrevFocusWnd); SDL_StartTextInput(pPrevFocusWnd);`
  - [x] T1.3 补齐中文注释，说明根因（SDL 的 `WIN_CreateWindow → WIN_StopTextInput(新窗口)` 经 `IME_Disable` 解除主窗口 HIMC 关联，且 `text_input_active` 不变导致 duilib 侧 `SetImeOpenStatus(true)` 无法自愈）
- [x] T2 编译验证
  - [x] T2.1 `msbuild c:\develop\nim_duilib\build\examples.sln /p:Configuration=Debug /p:Platform=x64` 增量编译通过；`duilib` 无 error / 无 warning（构建日志中出现的 C4996 均来自第三方 `libpng`，属既有告警，与本次改动无关）
  - [x] T2.2 确认 `NativeWindow_SDL.obj` 已重新编译（`build/build_temp/x64/duilib/Debug/NativeWindow_SDL.obj` = 2026-10-20 16:07:50），`bin/controls_d.exe` 重新链接（16:13:36），改动确实进入产物
- [ ] T3 运行时验证（**人工手动执行**，助手不代为运行 GUI）
  - [ ] T3.1 对照项：不下拉时输入中文正常
  - [ ] T3.2 修复点：展开下拉列表后输入中文可组合、候选框正常、可上屏
  - [ ] T3.3 展开状态下连续输入多个中文字，无丢字
  - [ ] T3.4 收起下拉后中文仍正常
  - [ ] T3.5 全程英文/ASCII 输入正常，无重复字符
- [ ] T4 回归核对
  - [ ] T4.1 `Combo`（`kCombo_DropList`，会激活弹窗）打开/关闭正常
  - [ ] T4.2 `RichEdit`/`RichEdit2` 输入与组合态显示正常（C-2 成果不回退）
  - [ ] T4.3 Tooltip / Toast / Menu 等弹窗打开/关闭无异常、无焦点闪烁
  - [ ] T4.4 `SDLEnabled=0`（原生 Windows 后端）编译路径不受影响（本项改动在 `#if defined (DUILIB_BUILD_FOR_WIN)` 内的 SDL 文件，原生后端不参与）

# Task Dependencies
- T2 依赖 T1
- T3 依赖 T2
- T4 依赖 T3

# 备注
- 代码提交由用户在验证通过后手动执行，助手不代为提交。
- 只验证 Debug x64 编译，Release 由用户自行处理。
- **T3 / T4 状态**：需在 GUI 中实际操作中文输入法，助手无法代跑，**由用户人工验证**；T1 / T2（代码 + 编译）已完成。
