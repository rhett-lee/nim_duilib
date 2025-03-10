#include "WindowDropTarget.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/WindowBase.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui 
{

//TODO: 暂不支持其他平台的DropTarget功能
class WindowDropTargetImpl
{
public:
    explicit WindowDropTargetImpl(WindowDropTarget* /*pWindowDropTarget*/)
    {
    }

    ~WindowDropTargetImpl()
    {
    }
};

WindowDropTarget::WindowDropTarget():
    m_pWindow(nullptr),
    m_pDropTargetImpl(nullptr)
{
}

WindowDropTarget::~WindowDropTarget()
{
    ASSERT(m_pDropTargetImpl == nullptr);
    if (m_pDropTargetImpl != nullptr) {
        /*int32_t nRef = m_pDropTargetImpl->Release();
        ASSERT_UNUSED_VARIABLE(nRef == 0);
        m_pDropTargetImpl = nullptr;*/
    }
}

void WindowDropTarget::SetWindow(WindowBase* pWindow)
{
    m_pWindow = pWindow;
}

bool WindowDropTarget::RegisterDragDrop(ControlDropTarget* /*pDropTarget*/)
{
    return true;
}

bool WindowDropTarget::UnregisterDragDrop(ControlDropTarget* /*pDropTarget*/)
{
    return true;
}

void WindowDropTarget::Clear()
{
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
