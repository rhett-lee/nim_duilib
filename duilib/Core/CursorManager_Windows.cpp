#include "CursorManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/FilePathUtil.h"
#include <map>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Utils/ApiWrapper_Windows.h"

namespace ui
{
class CursorManager::TImpl
{
public:
    TImpl();
    ~TImpl();

    /** 已经加载的光标资源句柄
    */
    std::map<FilePath, HCURSOR> m_cursorMap;
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
    case CursorType::kCursorArrow:
        ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
        break;
    case CursorType::kCursorIBeam:
        ::SetCursor(::LoadCursor(nullptr, IDC_IBEAM));
        break;
    case CursorType::kCursorHand:
        ::SetCursor(::LoadCursor(nullptr, IDC_HAND));
        break;
    case CursorType::kCursorWait:
        ::SetCursor(::LoadCursor(nullptr, IDC_WAIT));
        break;
    case CursorType::kCursorCross:
        ::SetCursor(::LoadCursor(nullptr, IDC_CROSS));
        break;
    case CursorType::kCursorSizeWE:
        ::SetCursor(::LoadCursor(nullptr, IDC_SIZEWE));
        break;
    case CursorType::kCursorSizeNS:
        ::SetCursor(::LoadCursor(nullptr, IDC_SIZENS));
        break;
    case CursorType::kCursorSizeNWSE:
        ::SetCursor(::LoadCursor(nullptr, IDC_SIZENWSE));
        break;
    case CursorType::kCursorSizeNESW:
        ::SetCursor(::LoadCursor(nullptr, IDC_SIZENESW));
        break;
    case CursorType::kCursorSizeAll:
        ::SetCursor(::LoadCursor(nullptr, IDC_SIZEALL));
        break;    
    case CursorType::kCursorNo:
        ::SetCursor(::LoadCursor(nullptr, IDC_NO));
        break;
    case CursorType::kCursorProgress:
        ::SetCursor(::LoadCursor(nullptr, IDC_APPSTARTING));
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

/** 从内存数据中加载光标
*/
static HCURSOR LoadCursorFromData(const Window* pWindow, std::vector<uint8_t>& fileData)
{
    if (fileData.empty() || (pWindow == nullptr)) {
        return nullptr;
    }
    const size_t fileDataSize = fileData.size();
    
    //Little Endian Only
    int16_t test = 1;
    bool bLittleEndianHost = (*((char*)&test) == 1);
    ASSERT_UNUSED_VARIABLE(bLittleEndianHost);

    bool bValidIcoFile = false;
    fileData.resize(fileData.size() + 1024); //填充空白
    typedef struct tagIconDir {
        uint16_t idReserved;
        uint16_t idType;
        uint16_t idCount;
    } ICONHEADER;
    typedef struct tagIconDirectoryEntry {
        uint8_t  bWidth;
        uint8_t  bHeight;
        uint8_t  bColorCount;
        uint8_t  bReserved;
        uint16_t  wPlanes;
        uint16_t  wBitCount;
        uint32_t dwBytesInRes;
        uint32_t dwImageOffset;
    } ICONDIRENTRY;

    ICONHEADER* icon_header = (ICONHEADER*)fileData.data();
    if ((icon_header->idReserved == 0) && (icon_header->idType == 2)) {
        bValidIcoFile = true;
        for (int32_t c = 0; c < icon_header->idCount; ++c) {
            size_t nDataOffset = sizeof(ICONHEADER) + sizeof(ICONDIRENTRY) * c;
            if (nDataOffset >= fileData.size()) {
                bValidIcoFile = false;
                break;
            }
            ICONDIRENTRY* pIconDir = (ICONDIRENTRY*)((uint8_t*)fileData.data() + nDataOffset);
            if (pIconDir->dwImageOffset >= fileDataSize) {
                bValidIcoFile = false;
                break;
            }
            else if ((pIconDir->dwImageOffset + pIconDir->dwBytesInRes) > fileDataSize) {
                bValidIcoFile = false;
                break;
            }
        }
    }
    ASSERT(bValidIcoFile);
    if (!bValidIcoFile) {
        return nullptr;
    }

    BOOL fIcon = TRUE; //按逻辑应该设置为FALSE, 但不知为何设置为FALSE时是失败的
    HICON hIcon = nullptr;
    uint32_t uDpi = pWindow->Dpi().GetDPI();
    int32_t cxCursor = GetSystemMetricsForDpiWrapper(SM_CXCURSOR, uDpi);
    int32_t cyCursor = GetSystemMetricsForDpiWrapper(SM_CYCURSOR, uDpi);
    int32_t offset = ::LookupIconIdFromDirectoryEx((PBYTE)fileData.data(), fIcon, cxCursor, cyCursor, LR_DEFAULTCOLOR);
    if (offset > 0) {
        hIcon = ::CreateIconFromResourceEx((PBYTE)fileData.data() + offset, (DWORD)fileDataSize - (DWORD)offset, fIcon, 0x00030000, cxCursor, cyCursor, LR_DEFAULTCOLOR);
    }
    ASSERT(hIcon != nullptr);
    return (HCURSOR)hIcon;
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

    HCURSOR hCursor = nullptr;
    auto iter = m_impl->m_cursorMap.find(cursorFullPath);
    if (iter != m_impl->m_cursorMap.end()) {
        hCursor = iter->second;
    }
    else {
        //加载光标
        if (GlobalManager::Instance().Zip().IsUseZip() && GlobalManager::Instance().Zip().IsZipResExist(cursorFullPath)) {
            //使用压缩包
            std::vector<uint8_t> fileData;
            GlobalManager::Instance().Zip().GetZipData(cursorFullPath, fileData);
            ASSERT(!fileData.empty());
            if (!fileData.empty()) {
                //从内存中加载光标
                hCursor = LoadCursorFromData(pWindow, fileData);
                ASSERT(hCursor != nullptr);
                if (hCursor != nullptr) {
                    m_impl->m_cursorMap[cursorFullPath] = hCursor;
                }
            }
        }
        else {
            //使用本地文件
            hCursor = (HCURSOR)::LoadImage(nullptr, cursorFullPath.NativePath().c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
            ASSERT(hCursor != nullptr);
            if (hCursor != nullptr) {
                m_impl->m_cursorMap[cursorFullPath] = hCursor;
            }
        }
    }
    ASSERT(hCursor != nullptr);
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

#endif // DUILIB_BUILD_FOR_WIN
