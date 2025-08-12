#include "WindowDropTarget_SDL.h"
#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Core/NativeWindow_SDL.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/StringConvert.h"

namespace ui 
{

WindowDropTarget::WindowDropTarget(NativeWindow_SDL* pNativeWindow):
    m_pHoverDropTarget(nullptr),
    m_pNativeWindow(pNativeWindow)
{
}

WindowDropTarget::~WindowDropTarget()
{
}

void WindowDropTarget::OnDropBegin()
{
    m_dropPt.SetXY(0, 0);
    m_textList.clear();
    m_fileList.clear();
    m_fileSource.clear();
}

void WindowDropTarget::OnDropPosition(const UiPoint& pt)
{
    m_dropPt = pt;
    ControlPtrT<ControlDropTarget_SDL> pHoverDropTarget = GetControlDropTarget(UiPoint(pt.x, pt.y));
    if (pHoverDropTarget == nullptr) {
        if (m_pHoverDropTarget != nullptr) {
            m_pHoverDropTarget->OnDropLeave();
            m_pHoverDropTarget = nullptr;
        }
    }
    else if (pHoverDropTarget == m_pHoverDropTarget) {
        pHoverDropTarget->OnDropPosition(pt);
    }
    else {
        if (m_pHoverDropTarget != nullptr) {
            m_pHoverDropTarget->OnDropLeave();
        }
        m_pHoverDropTarget = pHoverDropTarget;
        int32_t hr = pHoverDropTarget->OnDropBegin(pt);
        if (hr == 0) {
            pHoverDropTarget->OnDropPosition(pt);
        }
        else {
            m_pHoverDropTarget = nullptr;
        }
    }
}

void WindowDropTarget::OnDropText(const DStringA& utf8Text)
{
    if (!utf8Text.empty()) {
        m_textList.push_back(StringConvert::UTF8ToT(utf8Text));
    }
}

void WindowDropTarget::OnDropFile(const DStringA& utf8Source, const DStringA& utf8File)
{
    m_fileSource = StringConvert::UTF8ToT(utf8Source);
    if (!utf8File.empty()) {
        m_fileList.push_back(StringConvert::UTF8ToT(utf8File));
    }
}

void WindowDropTarget::OnDropComplete()
{
    if (!m_fileList.empty()) {
        if (m_pHoverDropTarget != nullptr) {
            DString fileSource = m_fileSource;
            std::vector<DString> fileList = m_fileList;
            m_pHoverDropTarget->OnDropFiles(fileSource, fileList, m_dropPt);
            m_pHoverDropTarget = nullptr;
        }
    }
    else if (!m_textList.empty()) {
        if (m_pHoverDropTarget != nullptr) {
            std::vector<DString> textList = m_textList;
            m_pHoverDropTarget->OnDropTexts(textList, m_dropPt);
            m_pHoverDropTarget = nullptr;
        }
    }
    m_dropPt.SetXY(0, 0);
    m_textList.clear();
    m_fileList.clear();
    m_fileSource.clear();

    if (m_pHoverDropTarget != nullptr) {
        m_pHoverDropTarget->OnDropLeave();
        m_pHoverDropTarget = nullptr;
    }
}

ControlPtrT<ControlDropTarget_SDL> WindowDropTarget::GetControlDropTarget(const UiPoint& clientPt) const
{
    ControlPtr pNewHover = ControlPtr(m_pNativeWindow->FindControl(clientPt));
    if (pNewHover != nullptr) {
        return ControlPtrT<ControlDropTarget_SDL>(pNewHover->GetControlDropTarget_SDL());
    }
    return ControlPtrT<ControlDropTarget_SDL>();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
