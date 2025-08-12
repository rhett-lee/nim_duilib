#include "ControlDropTargetImpl_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "duilib/Core/Control.h"
#include "duilib/Core/ControlDropTargetUtils.h"

namespace ui 
{
ControlDropTargetImpl_SDL::ControlDropTargetImpl_SDL(Control* pControl) :
    m_pControl(pControl)
{
}

ControlDropTargetImpl_SDL::~ControlDropTargetImpl_SDL()
{
}

int32_t ControlDropTargetImpl_SDL::OnDropBegin(const UiPoint& pt)
{
    if (m_pControl != nullptr) {
        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropEnter;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeSDL;
        msg.lParam = 0;
        msg.ptMouse = pt;
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
    }
    return S_OK;
}

void ControlDropTargetImpl_SDL::OnDropPosition(const UiPoint& pt)
{
    if (m_pControl != nullptr) {
        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropOver;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeSDL;
        msg.lParam = 0;
        msg.ptMouse = pt;
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
    }
}

void ControlDropTargetImpl_SDL::OnDropTexts(const std::vector<DString>& textList, const UiPoint& pt)
{
    if (m_pControl != nullptr) {
        ControlDropData_SDL data;
        data.m_bTextData = true;
        data.m_textList = textList;

        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropData;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeSDL;
        msg.lParam = (LPARAM)&data;
        msg.ptMouse = pt;
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
    }
}

void ControlDropTargetImpl_SDL::OnDropFiles(const DString& source, const std::vector<DString>& fileList, const UiPoint& pt)
{
    if (m_pControl != nullptr) {
        if (!fileList.empty()) {
            //当前执行的是拖放文件操作
            if (!m_pControl->IsEnableDropFile()) {
                //不支持文件拖放
                return;
            }
            //支持文件拖放操作，判断是否满足过滤条件
            DString fileTypes = m_pControl->GetDropFileTypes();
            if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, fileList)) {
                //文件类型不满足过滤条件
                return;
            }
        }

        ControlDropData_SDL data;
        data.m_bTextData = false;
        data.m_source = source;
        data.m_fileList = fileList;

        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropData;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeSDL;
        msg.lParam = (LPARAM)&data;
        msg.ptMouse = pt;
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
    }
}

void ControlDropTargetImpl_SDL::OnDropLeave()
{
    if (m_pControl != nullptr) {
        m_pControl->SendEvent(EventType::kEventDropLeave);
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
