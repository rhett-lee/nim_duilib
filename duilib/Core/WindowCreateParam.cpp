#include "WindowCreateParam.h"

namespace ui {

WindowCreateParam::WindowCreateParam():
    m_platformData(nullptr),
    m_className(_T("duilib_window")),
    m_dwClassStyle(kCS_VREDRAW | kCS_HREDRAW | kCS_DBLCLKS),
    m_dwStyle(kWS_OVERLAPPED | kWS_THICKFRAME),
    m_dwExStyle(0)
{
}

WindowCreateParam::WindowCreateParam(const DString& windowTitle):
    WindowCreateParam()
{
    m_windowTitle = windowTitle;
}

} // namespace ui
