// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_EVENTS_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_EVENTS_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_CEF

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_render_handler.h"
#pragma warning (pop)

namespace client {

/** 离屏渲染模式下的拖动事件处理接口
*   用于把系统的拖放事件转发给 CEF 浏览器，由浏览器处理网页内的拖放逻辑。
*/
class OsrDragEvents {
 public:
  /** 拖动进入浏览器区域
  * @param [in] drag_data 拖动的数据
  * @param [in] ev 鼠标事件（坐标为视图坐标）
  * @param [in] effect 允许的拖动操作类型
  * @return 返回实际执行的拖动操作类型
  */
  virtual CefBrowserHost::DragOperationsMask OnDragEnter(
      CefRefPtr<CefDragData> drag_data,
      CefMouseEvent ev,
      CefBrowserHost::DragOperationsMask effect) = 0;

  /** 拖动在浏览器区域内移动
  * @param [in] ev 鼠标事件（坐标为视图坐标）
  * @param [in] effect 允许的拖动操作类型
  * @return 返回实际执行的拖动操作类型
  */
  virtual CefBrowserHost::DragOperationsMask OnDragOver(
      CefMouseEvent ev,
      CefBrowserHost::DragOperationsMask effect) = 0;

  /** 拖动离开浏览器区域
  */
  virtual void OnDragLeave() = 0;

  /** 在浏览器区域内放下拖动的数据
  * @param [in] ev 鼠标事件（坐标为视图坐标）
  * @param [in] effect 允许的拖动操作类型
  * @return 返回实际执行的拖动操作类型
  */
  virtual CefBrowserHost::DragOperationsMask OnDrop(
      CefMouseEvent ev,
      CefBrowserHost::DragOperationsMask effect) = 0;

 protected:
  virtual ~OsrDragEvents() = default;
};

}  // namespace client

#endif //DUILIB_BUILD_FOR_CEF
#endif  // CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_EVENTS_H_
