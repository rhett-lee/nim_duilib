#include "CursorManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/FilePathUtil.h"
#include <map>

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui
{
class CursorManager::TImpl
{
public:
    TImpl();
    ~TImpl();

    /** 已经加载的光标资源句柄
    */
    std::map<FilePath, SDL_Cursor*> m_cursorMap;

    /** 系统光标
    */
    std::map<CursorType, SDL_Cursor*> m_systemCursorMap;
};

CursorManager::TImpl::TImpl()
{
}

CursorManager::TImpl::~TImpl()
{
    for (auto iter : m_cursorMap) {
        if (iter.second != nullptr) {
            SDL_DestroyCursor(iter.second);
        }
    }
    for (auto iter : m_systemCursorMap) {
        if (iter.second != nullptr) {
            SDL_DestroyCursor(iter.second);
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
    auto iter = m_impl->m_systemCursorMap.find(cursorType);
    if (iter != m_impl->m_systemCursorMap.end()) {
        ASSERT(iter->second != nullptr);
        int nRet = SDL_SetCursor(iter->second);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
        return nRet == 0;
    }
    
    SDL_Cursor* sdlCursor = nullptr;
    switch (cursorType) {
    case CursorType::kCursorArrow:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        break;
    case CursorType::kCursorIBeam:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
        break;
    case CursorType::kCursorHand:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
        break;
    case CursorType::kCursorWait:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
        break;
    case CursorType::kCursorCross:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        break;
    case CursorType::kCursorSizeWE:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
        break;
    case CursorType::kCursorSizeNS:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
        break;
    case CursorType::kCursorSizeNWSE:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
        break;
    case CursorType::kCursorSizeNESW:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
        break;
    case CursorType::kCursorSizeAll:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
        break;    
    case CursorType::kCursorNo:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
        break;    
    default:
        break;
    }

    if (sdlCursor == nullptr) {
        //默认光标
        sdlCursor = SDL_GetDefaultCursor();
    }
    else {
        m_impl->m_systemCursorMap[cursorType] = sdlCursor;
    }

    int nRet = -1;
    ASSERT(sdlCursor != nullptr);
    if (sdlCursor != nullptr) {
        nRet = SDL_SetCursor(sdlCursor);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
    }
    return nRet == 0;
}

bool CursorManager::SetImageCursor(const Window* pWindow, const FilePath& curImagePath)
{
    ASSERT(!curImagePath.IsEmpty());
    ASSERT(pWindow != nullptr);
    if ((pWindow == nullptr) || curImagePath.IsEmpty()) {
        return false;
    }

    //设置窗口图标
    const FilePath windowResFullPath = FilePathUtil::JoinFilePath(GlobalManager::Instance().GetResourcePath(), pWindow->GetResourcePath());
    FilePath cursorFullPath = FilePathUtil::JoinFilePath(windowResFullPath, curImagePath);
    cursorFullPath.NormalizeFilePath();
    if (GlobalManager::Instance().Zip().IsUseZip()) {
        //使用压缩包
        if (!GlobalManager::Instance().Zip().IsZipResExist(cursorFullPath)) {
            ASSERT(false);
            return false;
        }
    }
    else {
        //使用本地文件
        if (!cursorFullPath.IsExistsPath()) {
            ASSERT(false);
            return false;
        }
    }

    //TODO:

    return true;
}

bool CursorManager::ShowCursor(bool bShow)
{
    int nRet = -1;
    if (bShow) {
        nRet = SDL_ShowCursor();
    }
    else {
        nRet = SDL_HideCursor();
    }
    ASSERT(nRet == 0);
    return nRet == 0;
}

CursorID CursorManager::GetCursorID() const
{
    return (CursorID)SDL_GetCursor();
}

bool CursorManager::SetCursorByID(CursorID cursorId)
{
    int nRet = SDL_SetCursor((SDL_Cursor*)cursorId);
    ASSERT(nRet == 0);
    return nRet == 0;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
