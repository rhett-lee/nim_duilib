# FilterCombo 下拉展开时无法输入中文（SDL 后端）Spec（C-8）

## Why
**现象**（用户报告，Windows + SDL 后端，`SDLEnabled=1`，`SDL: VideoDriver:"windows", RenderName:"direct3d11"`）：`controls_d.exe` 中 `FilterCombo` 控件，**下拉列表展开时无法输入中文**（拼音候选框不出现 / 无法上屏），**仅能输入英文/ASCII**；**下拉列表收起后中文输入恢复正常**。

**性质**：这不是 FilterCombo 特有逻辑缺陷，而是**「SDL 后端下创建新顶层窗口会破坏已激活的输入法上下文」**这一通用缺陷，在下拉弹窗（一个独立 SDL 顶层窗口）上被暴露出来。凡是走「新建弹窗」路径的控件（Combo 下拉、Tooltip、Toast、Menu、颜色选择器弹窗等）在 SDL 后端下都会踩到同一问题。

**关联文档**：`docs/misc/Development-Plan.md` 的 **C-2**（IME 组合输入在非 Windows 平台显著退化，已完成）建立了 duilib 侧「文本输入开关 / 组合态」状态机；本项是同一链路上的另一个平台缺陷（Windows + SDL，即 IMM32 后端）。

## 根因（已逐条对照源码验证）
链路全在 SDL3 内部（`C:\develop\SDL`），**duilib 侧无法自愈**：

1. SDL3 的 Windows 输入法实现（[SDL_windowskeyboard.c](file:///c:/develop/SDL/src/video/windows/SDL_windowskeyboard.c)）是**设备全局**的：
   - `ime_hwnd_main` 在首次 `IME_Init` 时被固定为**主窗口** HWND（L402）；
   - `IME_SetWindow`（L709-718，唯一会把 `ime_hwnd_current` 切到别的窗口的地方）**只在 `IME_Init` 内部被调用**（L417）；
   - 而 `IME_Init`（L386-392）在 `ime_initialized == true` 时**直接 return**。
   → 结论：`ime_hwnd_current` 从始至终等于**主窗口** HWND，后续任何窗口的 `IME_Init` 都不会把它切走。
2. `WIN_CreateWindow` 在创建**任意**新窗口后调用 `WIN_StopTextInput(_this, window)`（[SDL_windowswindow.c#L740-L743](file:///c:/develop/SDL/src/video/windows/SDL_windowswindow.c#L740-L743)），注意传入的是**新窗口**。
3. `WIN_StopTextInput`（[SDL_windowskeyboard.c#L297-L309](file:///c:/develop/SDL/src/video/windows/SDL_windowskeyboard.c#L297-L309)）→ `IME_Init`（第 1 点：直接 return）→ `IME_Disable(videodata, hwnd)`；而 `IME_Disable`（[L451-L463](file:///c:/develop/SDL/src/video/windows/SDL_windowskeyboard.c#L451-L463)）判断的是 `ime_hwnd_current == ime_hwnd_main`（成立）→ 执行 `ImmAssociateContext(ime_hwnd_current /* = 主窗口 */, NULL)`。
   → **创建 Combo 下拉弹窗（新的 SDL 顶层窗口）时，主窗口的输入法上下文被解除关联（HIMC 被摘下）。**
4. 该路径**绕过了** `SDL_StopTextInput`，因此主窗口的 `window->text_input_active` **仍为 `true`**：
   - SDL 仍正常派发 `SDL_EVENT_TEXT_INPUT` → **英文/ASCII 可输入 ✓**；
   - 组合输入依赖的 HIMC / WM_IME_* 已失效 → **中文不可输入 ✓**。
   与用户观察到的「英文可、中文不可」完全一致。
5. **duilib 侧无法自愈**：
   - `NativeWindow_SDL::SetImeOpenStatus(true)`（[NativeWindow_SDL.cpp#L2813-L2817](file:///c:/develop/nim_duilib/duilib/Core/NativeWindow_SDL.cpp#L2813-L2817)）在 `SDL_TextInputActive(m_sdlWindow)` 为 `true` 时**直接跳过**；
   - `SDL_StartTextInput` 自身在 `window->text_input_active == true` 时也不会调用后端 `StartTextInput`（[SDL_video.c#L5871-L5879](file:///c:/develop/SDL/src/video/SDL_video.c#L5871-L5879)）；
   - 而唯一会重新关联 HIMC 的 `IME_Enable`（[L433-L449](file:///c:/develop/SDL/src/video/windows/SDL_windowskeyboard.c#L433-L449)，`ImmAssociateContext(hwnd, ime_himc)`）只能经后端 `WIN_StartTextInput` 到达。
   → 弹窗存在期间**没有任何 API 能恢复**（`SDL_TextInputActive` 恒为真，start 是空操作）。
6. **为什么收起下拉后中文会恢复**：收起时焦点链路会产生一次真实的 stop→start 往返：
   - `RichEdit` 重新获得控件焦点 → `Combo::OnEditSetFocus`（[Combo.cpp#L1066-L1071](file:///c:/develop/nim_duilib/duilib/Control/Combo.cpp#L1066-L1071)）把 RichEdit 的焦点事件转成 **Combo 自身**的 `kEventSetFocus`；
   - `Control::SendEventMsg` → `FireAllEvents`（回调）→ `HandleEvent`（[Control.cpp#L2739-L2743](file:///c:/develop/nim_duilib/duilib/Core/Control.cpp#L2739-L2743)）→ `Combo` 未重写 `OnSetFocus`，落到 `Control::OnSetFocus`（[Control.cpp#L3201-L3212](file:///c:/develop/nim_duilib/duilib/Core/Control.cpp#L3201-L3212)，`#if defined (DUILIB_BUILD_FOR_WIN)` 分支）→ `SetImeOpenStatus(false)` → `SDL_StopTextInput` → 本窗口 `text_input_active` 变 `false`；
   - 紧接着 `RichEdit2::OnSetFocus`（[RichEdit2.cpp#L3430-L3438](file:///c:/develop/nim_duilib/duilib/Control/RichEdit2.cpp#L3430-L3438)）→ `SetImeOpenStatus(true)` → 此时 `!SDL_TextInputActive` 成立 → 真正 `SDL_StartTextInput` → `IME_Enable` → **重新 `ImmAssociateContext`**。
   - 而**下拉展开期间焦点不变**，不存在这次往返 → IME 一直处于被解除关联的状态。

## What Changes
### 修复（选定方案 A：duilib 侧修复，无需重建 SDL）
在 `NativeWindow_SDL::CreateWindowAndRender`（[NativeWindow_SDL.cpp#L852-L873](file:///c:/develop/nim_duilib/duilib/Core/NativeWindow_SDL.cpp#L852-L873)，`CreateSdlWindow` 的唯一上层调用点）中：

1. 在调用 `CreateSdlWindow` **之前**记录当前键盘焦点窗口（`SDL_GetKeyboardFocus()`）；
2. 新窗口创建成功后（此时 SDL 已完成第 2/3 步的破坏动作），若「原焦点窗口」仍存活、不是新窗口本身、且 `SDL_TextInputActive(原焦点窗口)` 为真，则对**原焦点窗口**强制做一次 **Stop → Start 往返**：
   `SDL_StopTextInput(pPrevFocusWnd); SDL_StartTextInput(pPrevFocusWnd);`

原理：Stop 让 `text_input_active` 变 `false` 并走一次 `IME_Disable`，随后的 Start 因此**真正抵达 `WIN_StartTextInput` → `IME_Enable` → `ImmAssociateContext(ime_hwnd_current, ime_himc)`**，把主窗口的 HIMC 重新关联回去。该往返与 duilib 既有的「焦点切换时 IME 关-开」语义一致（`NativeWindow_Windows.h` 已声明「关闭再打开以后，能够保持原输入法状态」），不会丢失用户的输入法中/英模式。

```cpp
bool NativeWindow_SDL::CreateWindowAndRender(NativeWindow_SDL* pParentWindow, const WindowCreateAttributes& createAttributes)
{
    //创建新窗口前，记录当前拥有键盘焦点的窗口：SDL 的 Windows 后端在创建任意新窗口时
    //会调用 WIN_StopTextInput(新窗口)，而 IME_Init 在已初始化时直接返回，导致
    //IME_Disable 解除的是【当前主窗口】的输入法上下文关联（ImmAssociateContext(hwnd, NULL)），
    //但主窗口的 text_input_active 仍为 true，表现为"英文可输入、中文不可输入"，
    //且 SetImeOpenStatus(true) 会因 SDL_TextInputActive() 为 true 而直接返回、无法恢复。
    SDL_Window* pPrevFocusWnd = SDL_GetKeyboardFocus();

    //创建SDL窗口
    m_sdlWindow = CreateSdlWindow(pParentWindow, createAttributes);
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }

#if defined (DUILIB_BUILD_FOR_WIN)
    //创建新窗口后，对创建前的键盘焦点窗口做一次 Stop/Start 往返，
    //使其输入法上下文被重新关联（此问题仅 Windows 平台的 IMM32 实现存在）。
    if ((pPrevFocusWnd != nullptr) && (pPrevFocusWnd != m_sdlWindow) && SDL_TextInputActive(pPrevFocusWnd)) {
        SDL_StopTextInput(pPrevFocusWnd);
        SDL_StartTextInput(pPrevFocusWnd);
    }
#endif

    //创建SDL渲染接口
    ...
}
```

### 备选方案 B（记录，不默认实施）
上游修复应落在 SDL：`WIN_CreateWindow` 创建新窗口时不应影响**当前**输入法窗口，例如让 `IME_Disable` 用传入的 `hwnd` 与 `ime_hwnd_current` 比较（而非 `ime_hwnd_current == ime_hwnd_main`），或在 `SDL_WINDOW_NOT_FOCUSABLE` / popup 窗口上跳过该 `WIN_StopTextInput` 调用。
**不默认实施**：需重建 SDL（当前构建流程仅覆盖 nim_duilib），且用户对 SDL 的使用方式尚未确认；仅作为上游建议记录在案。

## ADDED Requirements

### Requirement: SDL 后端下创建新窗口不得破坏已激活的文本输入 / 输入法
在 Windows + SDL 后端下，创建新的顶层窗口（弹窗、下拉列表等）SHALL NOT 使此前处于文本输入激活状态的窗口丧失输入法（IME）能力；该窗口的英文/ASCII 输入行为 SHALL 保持不变。

#### Scenario: FilterCombo 下拉展开时中文可输入
- **WHEN** 焦点在 `FilterCombo` 的编辑框内、输入法处于中文状态，用户展开下拉列表
- **THEN** 下拉展开期间仍可正常输入中文（拼音组合、候选框、上屏均正常），与未展开时一致。

#### Scenario: 下拉展开期间持续输入
- **WHEN** 下拉已展开，用户继续逐字输入（每次文本变化触发 `FilterCombo::OnEditTextChanged` → `ShowComboList()`）
- **THEN** 因弹窗已存在不会重复创建窗口，中文输入持续可用，无中断。

#### Scenario: 下拉收起后中文仍可用
- **WHEN** 用户收起下拉列表
- **THEN** 中文输入保持可用（现有「焦点切换 stop→start」路径继续生效）。

#### Scenario: 英文/ASCII 行为不变
- **WHEN** 下拉展开或收起
- **THEN** 英文/ASCII 输入始终正常，无重复字符、无丢字。

#### Scenario: 非 Windows SDL 平台行为不变
- **WHEN** 在 Linux / macOS / FreeBSD 的 SDL 后端下创建弹窗
- **THEN** 不执行 Stop/Start 往返（该分支以 `DUILIB_BUILD_FOR_WIN` 保护），文本输入行为与改动前一致。

#### Scenario: 非 SDL（原生 Windows）后端行为不变
- **WHEN** `SDLEnabled=0` 编译运行
- **THEN** 走 `NativeWindow_Windows.cpp`，本项改动不参与编译，行为不变。

## Impact
- Affected specs：`docs/misc/Development-Plan.md` C-2（同链路背景）、本项 C-8。
- Affected code：`duilib/Core/NativeWindow_SDL.cpp` 单文件单函数。
- **非 BREAKING**：无公开 API 变化、无资源/主题变化、无 XML 变化。
- **不影响**：原生 Windows 后端、Linux/macOS/FreeBSD 路径（`#if defined (DUILIB_BUILD_FOR_WIN)` 保护）、组合态状态机（C-2 成果）。

## 改动文件清单
1. `duilib/Core/NativeWindow_SDL.cpp`：`NativeWindow_SDL::CreateWindowAndRender`（L852-L873）——新增「创建前记录焦点窗口」+ Windows 下 Stop/Start 往返（含中文注释说明根因）。

## 验证方式
- **编译**：`msbuild c:\develop\nim_duilib\build\examples.sln /p:Configuration=Debug /p:Platform=x64`（增量，要求 0 error / 0 warning）。**只验 Debug x64，不编译 Release**。
- **运行时（人工手动核对，用户执行）**：`controls_d.exe` →
  1. 点击 `FilterCombo` 编辑框（不下拉）输入中文：正常（改动前行为，作对照）；
  2. 展开下拉列表后输入中文：**候选框出现且可上屏**（本项修复点）；
  3. 下拉展开状态下连续输入多个中文字：字符全部上屏，无丢失；
  4. 收起下拉后继续输入中文：仍正常；
  5. 全程英文输入正常。
- **回归**：`Combo`（`kCombo_DropList`，会激活弹窗）、`RichEdit`/`RichEdit2`、Tooltip/Toast/Menu 等弹窗打开与关闭无异常，无焦点闪烁、无重复字符。

## 非目标（Out of Scope）
- **不修改 SDL 源码**（方案 B 仅记录）。
- **不重构 duilib 的 IME 开关策略**（如「非文本控件获焦不关 IME」等方案不在本项内；C-2 建立的组合态状态机保持不变）。
- **不新增/调整主题、语言、示例资源**。
- **不处理 `kCombo_DropList`（不可编辑）的输入法语义**（该模式无编辑框，不涉及中文输入）。
- **不做自动化测试**（无 GUI 输入法自动化方案），运行时验证由人工执行。
