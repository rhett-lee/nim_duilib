#include "CefDragDrop_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/CEFControl/internal/Windows/osr_dragdrop_win.h"
#include <map>

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_app.h"
#pragma warning (pop)

namespace ui
{
class CefDragDrop::TImpl
{
public:

    // 各个DropTarget的弱引用，一个窗口对应一个DropTarget，这个DropTarget可以给多个BorwserHandler使用
    std::map<HWND, std::weak_ptr<client::DropTargetWin>> m_dropTargetMap;
};

CefDragDrop::CefDragDrop()
{
    m_pImpl = new TImpl;
}

CefDragDrop::~CefDragDrop()
{
    Clear();
}

CefDragDrop& CefDragDrop::GetInstance()
{
    static CefDragDrop self;
    return self;
}

void CefDragDrop::Clear()
{
    if (m_pImpl != nullptr) {
        ASSERT(m_pImpl->m_dropTargetMap.empty());
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

std::shared_ptr<client::DropTargetWin> CefDragDrop::GetDropTarget(HWND hwnd)
{
    if (m_pImpl == nullptr) {
        return nullptr;
    }
    // 查找是否存在这个弱引用
    std::map<HWND, std::weak_ptr<client::DropTargetWin>>& dropTargetMap = m_pImpl->m_dropTargetMap;
    auto it = dropTargetMap.find(hwnd);
    if (it == dropTargetMap.end()) {
        auto deleter = [this](client::DropTargetWin* src) {
                if ((m_pImpl != nullptr) && (src != nullptr)) {
                    auto it = m_pImpl->m_dropTargetMap.find(src->GetHWND());
                    if (it != m_pImpl->m_dropTargetMap.end()) {
                        ::RevokeDragDrop(src->GetHWND());

                        // 移除弱引用对象
                        m_pImpl->m_dropTargetMap.erase(it);
                    }
                    else {
                        ASSERT(false);
                    }

                    delete src;
                }  
            };

        // 如果不存在就新增一个
        client::DropTargetHandle handle(new client::DropTargetWin(hwnd), deleter);
        dropTargetMap[hwnd] = handle;

        HRESULT register_res = ::RegisterDragDrop(hwnd, handle.get());
        if (register_res == DRAGDROP_E_ALREADYREGISTERED) {
            ::RevokeDragDrop(hwnd);
            register_res = ::RegisterDragDrop(hwnd, handle.get());
        }
        ASSERT(register_res == S_OK);
        return handle;
    }
    else {
        // 如果存在就返回弱引用对应的强引用指针
        return it->second.lock();
    }
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
