#include "CursorManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/ImageInfo.h"
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

/** 从内存数据中加载光标
*/
static SDL_Cursor* LoadCursorFromData(const Window* pWindow, std::vector<uint8_t>& fileData, const DString& imagePath)
{
    if (fileData.empty() || (pWindow == nullptr)) {
        return nullptr;
    }
    
    ImageLoadAttribute loadAttr = ImageLoadAttribute(DString(), DString(), false, false, 0);
    loadAttr.SetImageFullPath(imagePath);
    ImageDecoder imageDecoder;
    std::unique_ptr<ImageInfo> imageInfo = imageDecoder.LoadImageData(fileData, loadAttr, true, 100, pWindow->Dpi());
    ASSERT(imageInfo != nullptr);
    if (imageInfo == nullptr) {
        return nullptr;
    }

    IBitmap* pBitmap = imageInfo->GetBitmap(0);
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return nullptr;
    }

    void* pPixelBits = pBitmap->LockPixelBits();
    ASSERT(pPixelBits != nullptr);
    if (pPixelBits == nullptr) {
        return nullptr;
    }

    SDL_Surface* cursorSurface = SDL_CreateSurfaceFrom(pBitmap->GetWidth(), pBitmap->GetHeight(), SDL_PIXELFORMAT_BGRA32, pPixelBits, pBitmap->GetWidth() * sizeof(uint32_t));
    ASSERT(cursorSurface != nullptr);
    if (cursorSurface == nullptr) {
        return nullptr;
    }
    SDL_Cursor* sdlCursor = SDL_CreateColorCursor(cursorSurface, 0, 0);
    SDL_DestroySurface(cursorSurface);
    cursorSurface = nullptr;

    ASSERT(sdlCursor != nullptr);
    return sdlCursor;
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

    SDL_Cursor* sdlCursor = nullptr;
    auto iter = m_impl->m_cursorMap.find(cursorFullPath);
    if (iter != m_impl->m_cursorMap.end()) {
        sdlCursor = iter->second;
    }
    else {
        //加载光标
        std::vector<uint8_t> fileData;
        if (GlobalManager::Instance().Zip().IsUseZip()) {
            //使用压缩包
            if (GlobalManager::Instance().Zip().IsZipResExist(cursorFullPath)) {                
                bool bRet = GlobalManager::Instance().Zip().GetZipData(cursorFullPath, fileData);
                ASSERT_UNUSED_VARIABLE(bRet);                
            }
            else {
                ASSERT(false);
            }
        }
        else {
            //使用本地文件
            bool bRet = FileUtil::ReadFileData(cursorFullPath, fileData);
            ASSERT_UNUSED_VARIABLE(bRet);
        }
        ASSERT(!fileData.empty());
        if (!fileData.empty()) {
            //从内存中加载光标
            sdlCursor = LoadCursorFromData(pWindow, fileData, curImagePath.ToString());
            ASSERT(sdlCursor != nullptr);
            if (sdlCursor != nullptr) {
                m_impl->m_cursorMap[cursorFullPath] = sdlCursor;
            }
        }
    }
    ASSERT(sdlCursor != nullptr);
    if (sdlCursor != nullptr) {
        int nRet = SDL_SetCursor(sdlCursor);
        ASSERT_UNUSED_VARIABLE(nRet == 0);
        return nRet == 0;
    }
    return false;
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
