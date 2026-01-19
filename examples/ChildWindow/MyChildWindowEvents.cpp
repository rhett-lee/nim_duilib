#include "MyChildWindowEvents.h"
#include "ChildWindowPaint.h"
#include "ChildWindowPaintScheduler.h"

//绘制的FPS计算工具(计算每秒可以绘制多少帧)
class FPSCounter
{
public:
    FPSCounter() : m_frameCount(0), m_currentFPS(0.0f)
    {
        m_lastTime = std::chrono::high_resolution_clock::now();
    }

    void Update()
    {
        m_frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastTime);

        if (duration.count() >= 1000) {
            m_currentFPS = m_frameCount / (duration.count() / 1000.0f);
            m_frameCount = 0;
            m_lastTime = currentTime;
        }
    }

    float GetFPS() const
    {
        return m_currentFPS;
    }

private:
    size_t m_frameCount;//绘制了多少帧
    std::chrono::high_resolution_clock::time_point m_lastTime;
    float m_currentFPS; //当前FPS
};

MyChildWindowEvents::MyChildWindowEvents(ui::ChildWindow* pChildWindow,
                                         size_t nChildWindowIndex,
                                         ChildWindowPaintScheduler* pPaintScheduler) :
    m_pChildWindow(pChildWindow),
    m_pPaintScheduler(pPaintScheduler)
{
    m_childWindowPaint = std::make_unique<ChildWindowPaint>(pChildWindow);
    m_fps = std::make_unique<FPSCounter>();
    m_lastPaintTime = std::chrono::high_resolution_clock::now();
    m_pFpsLabel = dynamic_cast<ui::Label*>(pChildWindow->FindSubControl(_T("label_fps")));
    m_bPaintFps = true;

    //更新子窗口名称
    ui::Label* pChildWindowName = dynamic_cast<ui::Label*>(pChildWindow->FindSubControl(_T("child_window_name")));
    if (pChildWindowName != nullptr) {
        DString childWindowName = ui::StringUtil::Printf(_T("ChildWindow%d"), (int32_t)nChildWindowIndex);
        pChildWindowName->SetText(childWindowName);
    }

    //动态绘制
    ui::CheckBox* pStartFpsPaint = dynamic_cast<ui::CheckBox*>(pChildWindow->FindSubControl(_T("fps_paint")));
    if (pStartFpsPaint != nullptr) {
        pStartFpsPaint->AttachSelect([this](const ui::EventArgs&) {
            m_bPaintFps = true;
            if (m_pChildWindow != nullptr) {
                m_pChildWindow->InvalidateChildWindow();
                m_pChildWindow->Invalidate();
            }
            return true;
            });
        pStartFpsPaint->AttachUnSelect([this](const ui::EventArgs&) {
            m_bPaintFps = false;
            if (m_pChildWindow != nullptr) {
                m_pChildWindow->InvalidateChildWindow();
                m_pChildWindow->Invalidate();
            }
            return true;
            });
    }

    //全屏显示
    ui::CheckBox* pFullscreen = dynamic_cast<ui::CheckBox*>(pChildWindow->FindSubControl(_T("child_fullscreen")));
    if (pFullscreen != nullptr) {
        pFullscreen->AttachSelect([this](const ui::EventArgs&) {
            if (m_pChildWindow != nullptr) {
                ui::Window* pWindow = m_pChildWindow->GetWindow();
                if (pWindow != nullptr) {
                    pWindow->SetFullscreenControl(m_pChildWindow.get());
                }
            }
            return true;
            });
        pFullscreen->AttachUnSelect([this](const ui::EventArgs&) {
            if (m_pChildWindow != nullptr) {
                ui::Window* pWindow = m_pChildWindow->GetWindow();
                if ((pWindow != nullptr) && (pWindow->GetFullscreenControl() == m_pChildWindow.get())) {
                    pWindow->ExitControlFullscreen();
                }
            }
            return true;
            });
    }

    //退出全屏事件
    if (m_pChildWindow != nullptr) {
        ui::Window* pWindow = m_pChildWindow->GetWindow();
        if (pWindow != nullptr) {
            pWindow->AttachWindowExitFullscreenMsg([this](const ui::EventArgs&) {
                if (m_pChildWindow != nullptr) {
                    ui::CheckBox* pFullscreen = dynamic_cast<ui::CheckBox*>(m_pChildWindow->FindSubControl(_T("child_fullscreen")));
                    if (pFullscreen != nullptr) {
                        pFullscreen->SetSelected(false);
                    }
                }
                return true;
                });
        }
    }
}

MyChildWindowEvents::~MyChildWindowEvents()
{
}

ui::ChildWindow* MyChildWindowEvents::GetChildWindow() const
{
    return m_pChildWindow.get();
}

bool MyChildWindowEvents::IsPaintFps() const
{
    return m_bPaintFps;
}

LRESULT MyChildWindowEvents::OnPaintMsg(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    bHandled = true;

    //绘制子窗口（用于演示功能）
    m_childWindowPaint->PaintChildWindow(rcPaint, nativeMsg, m_bPaintFps);

    if (m_bPaintFps) {
        //更新FPS计数器
        m_fps->Update();

        //更新UI绘制计时器
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastPaintTime);
        if (duration.count() >= 500) {
            //每秒更新2次
            m_lastPaintTime = currentTime;
            DString fps = ui::StringUtil::Printf(_T("%d"), (int32_t)m_fps->GetFPS());
            if (m_pFpsLabel != nullptr) {
                m_pFpsLabel->SetText(fps);
            }
        }
    }
    else {
        if (m_pFpsLabel != nullptr) {
            const DString empty = _T("0");
            if (empty != m_pFpsLabel->GetText()) {
                //清空
                m_pFpsLabel->SetText(empty);
                if (m_pChildWindow != nullptr) {
                    m_pChildWindow->InvalidateChildWindow();
                }
            }
        }
    }
    if (m_pPaintScheduler != nullptr) {
        //持续绘制子窗口
        m_pPaintScheduler->PaintNextChildWindow(m_pChildWindow.get());
    }
    return 0;
}
