// Copyright 2016 The Chromium Embedded Framework Authors. Portions copyright
// 2013 The Chromium Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_OSR_IME_HANDLER_WIN_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_OSR_IME_HANDLER_WIN_H_
#pragma once

#include "duilib/duilib_defs.h"

#ifdef DUILIB_BUILD_FOR_CEF

#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100)
#include "include/internal/cef_types_wrappers.h"
#pragma warning(pop)

namespace client {

// 为承载离屏渲染浏览器的原生父窗口处理输入法（IME）相关逻辑。
// 本对象只在 CEF 的 UI 线程中访问。
class OsrImeHandlerWin {
 public:
  /** 构造函数
  * @param [in] hwnd 承载离屏浏览器的原生窗口句柄
  */
  explicit OsrImeHandlerWin(HWND hwnd);
  virtual ~OsrImeHandlerWin();

  // 获取当前是否处于输入法组合（组字）状态
  bool is_composing() const { return is_composing_; }

  // 从 Windows 获取当前输入语言并更新到内部状态
  void SetInputLanguage();

  // 按需创建输入法的光标窗口
  void CreateImeWindow();

  // 销毁输入法的光标窗口
  void DestroyImeWindow();

  // 清理与该 IMM32 管理对象关联的所有资源，并复位组合状态
  void CleanupComposition();

  // 复位组合状态，并取消正在进行的组合操作
  void ResetComposition();

  /** 获取当前组合操作的结果（如果存在组合操作）
  * @param [in] lparam 与输入法消息关联的 lParam 参数
  * @param [out] result 返回组合结果的文本
  * @return 返回 true 表示获取成功，否则返回 false
  */
  bool GetResult(LPARAM lparam, CefString& result);

  /** 获取当前组合操作的状态
  * 包括组合文本、下划线信息以及组合文本中的选中范围。
  * 说明：IMM32 不支持字符级选择。
  * @param [in] lparam 与输入法消息关联的 lParam 参数
  * @param [out] composition_text 返回组合文本
  * @param [out] underlines 返回组合文本的下划线信息
  * @param [out] composition_start 返回组合文本的起始位置
  * @return 返回 true 表示获取成功，否则返回 false
  */
  bool GetComposition(LPARAM lparam,
                      CefString& composition_text,
                      std::vector<CefCompositionUnderline>& underlines,
                      int& composition_start);

  // 启用指定窗口关联的输入法
  virtual void EnableIME();

  // 禁用指定窗口关联的输入法
  virtual void DisableIME();

  // 取消输入法正在进行的组合操作
  virtual void CancelIME();

  /** 更新指定窗口的输入法光标位置
  * @param [in] index 光标在组合文本中的索引
  */
  void UpdateCaretPosition(uint32_t index);

  /** 更新组合文本的范围
  * selection_range 为已被选中的字符范围；
  * character_bounds 为每个字符在视图设备坐标中的边界。
  * @param [in] selection_range 选中的字符范围
  * @param [in] character_bounds 每个字符的边界矩形
  */
  void ChangeCompositionRange(const CefRange& selection_range,
                              const std::vector<CefRect>& character_bounds);

  // 更新输入法窗口的位置
  void MoveImeWindow();

  //获取关联的窗口
  HWND GetHandlerHWND() const { return hwnd_; }

 private:
  /** 获取组合信息
  * @param [in] imm_context 输入法上下文
  * @param [in] lparam 与输入法消息关联的 lParam 参数
  * @param [out] composition_text 返回组合文本
  * @param [out] underlines 返回组合文本的下划线信息
  * @param [out] composition_start 返回组合文本的起始位置
  */
  void GetCompositionInfo(HIMC imm_context,
                          LPARAM lparam,
                          CefString& composition_text,
                          std::vector<CefCompositionUnderline>& underlines,
                          int& composition_start);

  /** 从输入法管理器（IMM）获取字符串
  * @param [in] imm_context 输入法上下文
  * @param [in] lparam 与输入法消息关联的 lParam 参数
  * @param [in] type 要获取的字符串类型
  * @param [out] result 返回获取到的字符串
  * @return 返回 true 表示获取成功，否则返回 false
  */
  bool GetString(HIMC imm_context, WPARAM lparam, int type, CefString& result);

  // 当前是否处于输入法组合（组字）状态
  bool is_composing_ = false;

  // 当前组合文本的字符范围及其边界矩形
  std::vector<CefRect> composition_bounds_;

  // 从 Windows 获取的当前输入语言 ID
  // 用于处理输入法中与语言相关的操作
  LANGID input_language_id_;

  // 当前输入上下文是否已创建系统光标（用于设置输入法候选窗口的位置）
  bool system_caret_ = false;

  // 从渲染进程获取的输入光标矩形
  CefRect ime_rect_;

  // 组合文本中的当前光标索引
  uint32_t cursor_index_;

  // 组合文本的取值范围（可用于确定组合边界中的偏移量）
  CefRange composition_range_;

  // 与本对象关联的窗口句柄
  HWND hwnd_;
};

}  // namespace client

#endif //DUILIB_BUILD_FOR_CEF

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_OSR_IME_HANDLER_WIN_H_
