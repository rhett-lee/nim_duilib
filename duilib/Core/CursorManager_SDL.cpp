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
        bool nRet = SDL_SetCursor(iter->second);
        ASSERT_UNUSED_VARIABLE(nRet);
        return nRet;
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
    case CursorType::kCursorProgress:
        sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
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

    bool nRet = false;
    ASSERT(sdlCursor != nullptr);
    if (sdlCursor != nullptr) {
        nRet = SDL_SetCursor(sdlCursor);
        ASSERT_UNUSED_VARIABLE(nRet);
    }
    return nRet;
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
    uint32_t nFrameCount = 0;
    ImageDecoder imageDecoder;
    std::unique_ptr<ImageInfo> imageInfo = imageDecoder.LoadImageData(fileData, loadAttr, true, 100, pWindow->Dpi().GetScale(), true, nFrameCount);
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

#ifdef DUILIB_BUILD_FOR_WIN
    SDL_PixelFormat format = SDL_PIXELFORMAT_BGRA32;
#else
    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
#endif
    SDL_Surface* cursorSurface = SDL_CreateSurfaceFrom(pBitmap->GetWidth(), pBitmap->GetHeight(), format, pPixelBits, pBitmap->GetWidth() * sizeof(uint32_t));
    ASSERT(cursorSurface != nullptr);
    if (cursorSurface == nullptr) {
        return nullptr;
    }

    int hot_x = 0;
    int hot_y = 0;
    size_t nDot = imagePath.rfind(_T('.'));
    if ((nDot != DString::npos) && (fileData.size() > 16)) {
        DString ext = imagePath.substr(nDot);
        if (StringUtil::IsEqualNoCase(ext, _T(".cur"))) {
            //hot_x位于 0x0A，2个字节
            hot_x = (fileData[0x0B] << 8) | fileData[0x0A];
            //hot_y位于 0x0C，2个字节
            hot_y = (fileData[0x0D] << 8) | fileData[0x0C];
            if ((hot_x >= (int)pBitmap->GetWidth()) || (hot_x < 0)) {
                hot_x = 0;
            }
            if ((hot_y >= (int)pBitmap->GetHeight()) || (hot_y < 0)){
                hot_y = 0;
            }
            pWindow->Dpi().ScaleInt(hot_x);
            pWindow->Dpi().ScaleInt(hot_y);
        }
    }

    SDL_Cursor* sdlCursor = SDL_CreateColorCursor(cursorSurface, hot_x, hot_y);
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
    FilePath cursorFullPath = GlobalManager::Instance().GetExistsResFullPath(pWindow->GetResourcePath(), pWindow->GetXmlPath(), curImagePath);
    ASSERT(!cursorFullPath.IsEmpty());
    if (cursorFullPath.IsEmpty()) {
        return false;
    }

    SDL_Cursor* sdlCursor = nullptr;
    auto iter = m_impl->m_cursorMap.find(cursorFullPath);
    if (iter != m_impl->m_cursorMap.end()) {
        sdlCursor = iter->second;
    }
    else {
        //加载光标
        std::vector<uint8_t> fileData;
        if (GlobalManager::Instance().Zip().IsUseZip() && GlobalManager::Instance().Zip().IsZipResExist(cursorFullPath)) {
            //使用压缩包               
            bool bRet = GlobalManager::Instance().Zip().GetZipData(cursorFullPath, fileData);
            ASSERT_UNUSED_VARIABLE(bRet);
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
        bool nRet = SDL_SetCursor(sdlCursor);
        ASSERT_UNUSED_VARIABLE(nRet);
        return nRet;
    }
    return false;
}

bool CursorManager::ShowCursor(bool bShow)
{
    bool nRet = false;
    if (bShow) {
        nRet = SDL_ShowCursor();
    }
    else {
        nRet = SDL_HideCursor();
    }
    ASSERT(nRet);
    return nRet;
}

CursorID CursorManager::GetCursorID() const
{
    return (CursorID)SDL_GetCursor();
}

bool CursorManager::SetCursorByID(CursorID cursorId)
{
    bool nRet = SDL_SetCursor((SDL_Cursor*)cursorId);
    ASSERT(nRet);
    return nRet;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
