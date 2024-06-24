#include "CursorManager.h"
#include <map>

#ifdef DUILIB_PLATFORM_WIN

namespace ui
{
class CursorManager::TImpl
{
public:
    TImpl();
    ~TImpl();

    /** 已经加载的光标资源句柄
    */
    std::map<DString, HCURSOR> m_cursorMap;
};

CursorManager::TImpl::TImpl()
{
}

CursorManager::TImpl::~TImpl()
{
    for (auto iter : m_cursorMap) {
        if (iter.second != nullptr) {
            ::DestroyCursor(iter.second);
        }
    }
}

CursorManager::CursorManager()
{
    m_impl = new TImpl;
}

CursorManager::~CursorManager()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

bool CursorManager::SetCursor(CursorType cursorType)
{
    bool bRet = true;
    switch (cursorType) {
    case kCursorArrow:
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        break;
    case kCursorHand:
        ::SetCursor(::LoadCursor(NULL, IDC_HAND));
        break;
    case kCursorHandIbeam:
        ::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
        break;
    case kCursorSizeWE:
        ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
        break;
    case kCursorSizeNS:
        ::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
        break;
    case kCursorWait:
        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool CursorManager::SetImageCursor(const DString& imagePath)
{
    ASSERT(!imagePath.empty());

    HCURSOR hCursor = nullptr;
    auto iter = m_impl->m_cursorMap.find(imagePath);
    if (iter != m_impl->m_cursorMap.end()) {
        hCursor = iter->second;
        ASSERT(hCursor != nullptr);
    }
    else {
        hCursor = (HCURSOR)::LoadImage(NULL, imagePath.c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        ASSERT(hCursor != nullptr);
        if (hCursor != nullptr) {
            m_impl->m_cursorMap[imagePath] = hCursor;
        }
    }
    if (hCursor != nullptr) {
        ::SetCursor(hCursor);
        return true;
    }
    return false;
}

bool CursorManager::ShowCursor(bool bShow)
{
    ::ShowCursor(bShow ? TRUE : FALSE);
    return true;
}

CursorID CursorManager::GetCursorID() const
{
    return (CursorID)::GetCursor();
}

bool CursorManager::SetCursorByID(CursorID cursorId)
{
    ::SetCursor((HCURSOR)cursorId);
    return true;
}

} // namespace ui

#endif // DUILIB_PLATFORM_WIN
