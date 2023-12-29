#include "wke_web_view.h"
#include "wke_memory_dc.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/Macros.h"

namespace {

void OnPaintCallback(wkeWebView webView, void* param, const void* buffer, const wkeRect* r, int width, int height) {
  nim_comp::WkeWebView* web_view = (nim_comp::WkeWebView*)param;
  web_view->OnPaint(webView, buffer, r, width, height);
}

}  // namespace

namespace nim_comp {

WkeWebView::WkeWebView() {}

WkeWebView::~WkeWebView() {
    GetWindow()->RemoveMessageFilter(this);

  if (m_wke_web_view) {
    wkeDestroyWebView(m_wke_web_view);
  }
}

void WkeWebView::OnInit() {
    if (IsInited()) {
        return;
    }
    __super::OnInit();
  m_web_view_dc = std::make_unique<WkeMemoryDC>();

  m_wke_web_view = CreateWebView();

  InitWebViewBase();

  wkeOnPaintBitUpdated(m_wke_web_view, OnPaintCallback, this);

  GetWindow()->AddMessageFilter(this);

  AttachResize(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
    resize(GetWidth(), GetHeight());
    return true;
  }));

  AttachKillFocus(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
    killFocus();
    return true;
  }));
}

void WkeWebView::Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) {
  __super::Paint(pRender, rcPaint);

  if (!m_web_view_dc || !m_web_view_dc->IsValid()) {
    return;
  }
  std::unique_ptr<ui::IBitmap> bitmap;
  ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
  ASSERT(pRenderFactory != nullptr);
  if (pRenderFactory != nullptr) {
      bitmap.reset(pRenderFactory->CreateBitmap());
  }
  ASSERT(bitmap != nullptr);
  if (bitmap == nullptr) {
      return;
  }

  if (!bitmap->Init(m_web_view_dc->GetWidth(), m_web_view_dc->GetHeight(), true, m_web_view_dc->GetBits())) {
      return;
  }
  ui::UiRect rect = GetRect();
  pRender->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), bitmap.get(), 0, 0, ui::RopMode::kSrcCopy);
}

void WkeWebView::SetWindow(ui::Window* pManager) {
  if (!m_wke_web_view)
  {
    __super::SetWindow(pManager);
    return;
  }

  if (GetWindow()) {
      GetWindow()->RemoveMessageFilter(this);
    __super::SetWindow(pManager);
    if (pManager != nullptr) {
        pManager->AddMessageFilter(this);
    }    
  }
}

LRESULT WkeWebView::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
  if (!IsVisible() || !IsEnabled() || !m_wke_web_view)
  {
    bHandled = false;
    return 0;
  }

  HWND hWnd = GetWindow()->GetHWND();

  switch (uMsg) {

  case WM_SETCURSOR:
    if (SetCursorInfoTypeByCache()) {
      bHandled = true;
    }
    break;

  case WM_KEYDOWN:
  {
    if (!IsFocused()) {
      break;
    }

    uint32_t virtualKeyCode = static_cast<uint32_t>(wParam);
    uint32_t flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
      flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
      flags |= WKE_EXTENDED;

    bHandled = wkeFireKeyDownEvent(m_wke_web_view, virtualKeyCode, flags, false);
  }
  break;

  case WM_KEYUP:
  {
    if (!IsFocused()) {
      break;
    }

    uint32_t virtualKeyCode = static_cast<uint32_t>(wParam);
    uint32_t flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
      flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
      flags |= WKE_EXTENDED;

    bHandled = wkeFireKeyUpEvent(m_wke_web_view, virtualKeyCode, flags, false);
  }
  break;

  case WM_CHAR:
  {
    if (!IsFocused()) {
      break;
    }

    uint32_t charCode = static_cast<uint32_t>(wParam);
    uint32_t flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
      flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
      flags |= WKE_EXTENDED;

    bHandled = wkeFireKeyPressEvent(m_wke_web_view, charCode, flags, false);
  }
  break;

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONDBLCLK: //
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MOUSEMOVE:
  {
    ui::UiPoint pOriginPt;
    pOriginPt.x = GET_X_LPARAM(lParam);
    pOriginPt.y = GET_Y_LPARAM(lParam);
    if (pOriginPt.x == -1 || pOriginPt.y == -1) {
      break;
    }

    ui::UiPoint pWebPt = pOriginPt;

    if (!GetWebViewPos(pWebPt)) {
      break;
    }

    OnCursorChange();

    if (uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN) {
      ::SetFocus(hWnd);
      ::SetCapture(hWnd);
      SetFocus();
      setFocus();
    }
    else if (uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_RBUTTONUP) {
      ::ReleaseCapture();
    }

    unsigned int flags = 0;

    if (wParam & MK_CONTROL)
      flags |= WKE_CONTROL;
    if (wParam & MK_SHIFT)
      flags |= WKE_SHIFT;

    if (wParam & MK_LBUTTON)
      flags |= WKE_LBUTTON;
    if (wParam & MK_MBUTTON)
      flags |= WKE_MBUTTON;
    if (wParam & MK_RBUTTON)
      flags |= WKE_RBUTTON;

    bHandled = wkeFireMouseEvent(m_wke_web_view, uMsg, pWebPt.x, pWebPt.y, flags);

    ui::Control* control = GetWindow()->FindControl(pOriginPt);
    if (bHandled && control != this && control->IsMouseEnabled()) {
      bHandled = false;
    }
  }
  break;

  case WM_CONTEXTMENU:
  {
    ui::UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (pt.x == -1 || pt.y == -1) {
      break;
    }
    GetWindow()->ScreenToClient(pt);

    if (!GetWebViewPos(pt)) {
      break;
    }

    unsigned int flags = 0;

    if (wParam & MK_CONTROL)
      flags |= WKE_CONTROL;
    if (wParam & MK_SHIFT)
      flags |= WKE_SHIFT;

    if (wParam & MK_LBUTTON)
      flags |= WKE_LBUTTON;
    if (wParam & MK_MBUTTON)
      flags |= WKE_MBUTTON;
    if (wParam & MK_RBUTTON)
      flags |= WKE_RBUTTON;

    bHandled = wkeFireContextMenuEvent(m_wke_web_view, pt.x, pt.y, flags);
  }
  break;

  case WM_MOUSEWHEEL:
  {
    if (!m_bWheelEnabled) {
      break;
    }

    ui::UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (pt.x == -1 || pt.y == -1) {
      break;
    }

    GetWindow()->ScreenToClient(pt);

    if (!GetWebViewPos(pt)) {
      break;  
    }

    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    unsigned int flags = 0;

    if (wParam & MK_CONTROL)
      flags |= WKE_CONTROL;
    if (wParam & MK_SHIFT)
      flags |= WKE_SHIFT;

    if (wParam & MK_LBUTTON)
      flags |= WKE_LBUTTON;
    if (wParam & MK_MBUTTON)
      flags |= WKE_MBUTTON;
    if (wParam & MK_RBUTTON)
      flags |= WKE_RBUTTON;

    //flags = wParam;

    bHandled = wkeFireMouseWheelEvent(m_wke_web_view, pt.x, pt.y, delta, flags);
  }
  break;

  case WM_IME_STARTCOMPOSITION:
  {
    wkeRect caret = wkeGetCaretRect(m_wke_web_view);

    CANDIDATEFORM form;
    form.dwIndex = 0;
    form.dwStyle = CFS_EXCLUDE;
    form.ptCurrentPos.x = caret.x;
    form.ptCurrentPos.y = caret.y + caret.h;
    form.rcArea.top = caret.y;
    form.rcArea.bottom = caret.y + caret.h;
    form.rcArea.left = caret.x;
    form.rcArea.right = caret.x + caret.w;

    HIMC hIMC = ImmGetContext(hWnd);
    ImmSetCandidateWindow(hIMC, &form);
    ImmReleaseContext(hWnd, hIMC);

  }
  break;

  default:
    break;
  }

  return 0;
}

void WkeWebView::OnPaint(wkeWebView /*webView*/, const void* buffer, const wkeRect* /*r*/, int width, int height) {
  if (!buffer) {
    return;
  }
  if (m_web_view_dc->GetWidth() != width || m_web_view_dc->GetHeight() != height)
    m_web_view_dc->Init(GetWindow()->GetPaintDC(), width, height, false);

  LPBYTE pDst = (LPBYTE)m_web_view_dc->GetBits();
  if (pDst)
    memcpy(pDst, (char*)buffer, height * width * 4);

  Invalidate();
}

void WkeWebView::OnCursorChange() {
  if (m_wke_web_view) {
    m_cursorInfoType = wkeGetCursorInfoType(m_wke_web_view);
  }
}

bool WkeWebView::SetCursorInfoTypeByCache() {
  ui::UiPoint pt;
  GetWindow()->GetCursorPos(pt);
  GetWindow()->ScreenToClient(pt);
  if (!GetWebViewPos(pt))
    return false;

  HCURSOR hCur = NULL;
  switch (m_cursorInfoType) {
  case WkeCursorInfoPointer:
    hCur = ::LoadCursor(NULL, IDC_ARROW);
    break;
  case WkeCursorInfoIBeam:
    hCur = ::LoadCursor(NULL, IDC_IBEAM);
    break;
  case WkeCursorInfoHand:
    hCur = ::LoadCursor(NULL, IDC_HAND);
    break;
  case WkeCursorInfoWait:
    hCur = ::LoadCursor(NULL, IDC_WAIT);
    break;
  case WkeCursorInfoHelp:
    hCur = ::LoadCursor(NULL, IDC_HELP);
    break;
  case WkeCursorInfoEastResize:
    hCur = ::LoadCursor(NULL, IDC_SIZEWE);
    break;
  case WkeCursorInfoNorthResize:
    hCur = ::LoadCursor(NULL, IDC_SIZENS);
    break;
  case WkeCursorInfoSouthWestResize:
  case WkeCursorInfoNorthEastResize:
    hCur = ::LoadCursor(NULL, IDC_SIZENESW);
    break;
  case WkeCursorInfoSouthResize:
  case WkeCursorInfoNorthSouthResize:
    hCur = ::LoadCursor(NULL, IDC_SIZENS);
    break;
  case WkeCursorInfoNorthWestResize:
  case WkeCursorInfoSouthEastResize:
    hCur = ::LoadCursor(NULL, IDC_SIZENWSE);
    break;
  case WkeCursorInfoWestResize:
  case WkeCursorInfoEastWestResize:
    hCur = ::LoadCursor(NULL, IDC_SIZEWE);
    break;
  case WkeCursorInfoNorthEastSouthWestResize:
  case WkeCursorInfoNorthWestSouthEastResize:
    hCur = ::LoadCursor(NULL, IDC_SIZEALL);
    break;
  default:
    hCur = ::LoadCursor(NULL, IDC_ARROW);
    break;
  }

  if (hCur) {
    ::SetCursor(hCur);
    return true;
  }

  return false;
}

bool WkeWebView::GetWebViewPos(ui::UiPoint& point) {
  point.Offset(GetScrollOffsetInScrollBox());
  if (!GetRect().ContainsPt(point))
    return false;

  point.x = point.x - GetRect().left;
  point.y = point.y - GetRect().top;
  return true;
}

}  // namespace nim_comp
