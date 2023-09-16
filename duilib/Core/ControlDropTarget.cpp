#include "ControlDropTarget.h"
#include "duilib/Core/Control.h"

namespace ui 
{

ControlDropTarget::ControlDropTarget():
    m_pControl(nullptr)
{
}

ControlDropTarget::~ControlDropTarget()
{
    if (m_pControl != nullptr) {
        m_pControl = nullptr;
    }
}

void ControlDropTarget::SetControl(Control* pControl)
{
    m_pControl = pControl;
}

Control* ControlDropTarget::GetControl() const
{
    return m_pControl;
}

bool ControlDropTarget::ContainsPt(const UiPoint& screenPt) const
{
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return false;
    }
    UiPoint clientPt = screenPt;
    m_pControl->ScreenToClient(clientPt);
    if (m_pControl->GetPos().ContainsPt(clientPt)) {
        return true;
    }
    return false;
}

int32_t ControlDropTarget::DragEnter(void* /*pDataObj*/, uint32_t /*grfKeyState*/, const UiPoint& /*pt*/, uint32_t* /*pdwEffect*/)
{
    //默认禁止拖放
    return S_FALSE;
}

int32_t ControlDropTarget::DragOver(uint32_t /*grfKeyState*/, const UiPoint& /*pt*/, uint32_t* /*pdwEffect*/)
{
    //默认禁止拖放
    return S_FALSE;
}

int32_t ControlDropTarget::DragLeave(void)
{
    //默认禁止拖放
    return S_FALSE;
}

int32_t ControlDropTarget::Drop(void* /*pDataObj*/, uint32_t /*grfKeyState*/, const UiPoint& /*pt*/, uint32_t* /*pdwEffect*/)
{
    //默认禁止拖放
    return S_FALSE;
}

} // namespace ui

