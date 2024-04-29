#pragma once

#include "ui_components/mini_blink_control/web_view_base.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Window.h"

class WkeMemoryDC;

namespace nim_comp {
class WkeWebView : public ui::Control,
                   public WebViewBase,
                   public ui::IUIMessageFilter {
 public:
  WkeWebView();
  ~WkeWebView() override;

  // override form ui::Control
  virtual void OnInit() override;
  void Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) override;
  void SetWindow(ui::Window* pManager) override;

  // override from ui::IUIMessageFilter
  LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override; // 处理窗体消息，转发到miniblink

  void OnPaint(wkeWebView webView, const void* buffer, const wkeRect* r, int width, int height);

  void SetWheelEnabled(bool enabled) { m_bWheelEnabled = enabled; }

 private:
  void OnCursorChange();
  bool SetCursorInfoTypeByCache();
  bool GetWebViewPos(ui::UiPoint& point);

  bool m_bWheelEnabled = true;
  std::unique_ptr<WkeMemoryDC> m_web_view_dc;
  int m_cursorInfoType = 0;
};

}
