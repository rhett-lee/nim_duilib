#include "CursorManager.h"

namespace ui
{
CursorManager::CursorManager()
{
}

CursorManager::~CursorManager()
{
    for (auto iter : m_cursorMap) {
        if (iter.second != nullptr) {
            ::DestroyCursor(iter.second);
        }
    }
}

bool CursorManager::SetCursor(CursorType cursorType)
{
#ifdef DUILIB_PLATFORM_WIN
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
#else
    ASSERT(false);
    return false;
#endif
}

bool CursorManager::SetImageCursor(const DString& imagePath)
{
#ifdef DUILIB_PLATFORM_WIN
    ASSERT(!imagePath.empty());

    HCURSOR hCursor = nullptr;
    auto iter = m_cursorMap.find(imagePath);
    if (iter != m_cursorMap.end()) {
        hCursor = iter->second;
        ASSERT(hCursor != nullptr);
    }
    else {
        hCursor = (HCURSOR)::LoadImage(NULL, imagePath.c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        ASSERT(hCursor != nullptr);
        if (hCursor != nullptr) {
            m_cursorMap[imagePath] = hCursor;
        }
    }
    if (hCursor != nullptr) {
        ::SetCursor(hCursor);
        return true;
    }
    return false;
#else
    ASSERT(false);
    return false;
#endif
}

bool CursorManager::ShowCursor(bool bShow)
{
#ifdef DUILIB_PLATFORM_WIN
    ::ShowCursor(bShow ? TRUE : FALSE);
    return true;
#else
    ASSERT(false);
    return false;
#endif
}

CursorID CursorManager::GetCursorID() const
{
#ifdef DUILIB_PLATFORM_WIN
    return (CursorID)::GetCursor();
#else
    ASSERT(false);
    return 0;
#endif
}

bool CursorManager::SetCursorByID(CursorID cursorId)
{
#ifdef DUILIB_PLATFORM_WIN
    ::SetCursor((HCURSOR)cursorId);
    return true;
#else
    ASSERT(false);
    return 0;
#endif
}

} // namespace ui
