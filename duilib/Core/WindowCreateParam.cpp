#include "WindowCreateParam.h"

namespace ui {

WindowCreateParam::WindowCreateParam():
    m_platformData(nullptr),
    m_className(_T("duilib_window")),
    m_dwClassStyle(kCS_VREDRAW | kCS_HREDRAW | kCS_DBLCLKS),
    m_dwStyle(0),
    m_dwExStyle(0),
    m_nX(kCW_USEDEFAULT),
    m_nY(0),
    m_nWidth(kCW_USEDEFAULT),
    m_nHeight(0)
{
}

WindowCreateParam::WindowCreateParam(const DString& windowTitle):
    WindowCreateParam()
{
    m_windowTitle = windowTitle;
}

} // namespace ui
