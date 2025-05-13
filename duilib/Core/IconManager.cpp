#include "IconManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/Image.h"

#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
#pragma warning (push)
#pragma warning (disable: 4505)
    #include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

namespace ui 
{

IconManager::IconManager():
    m_nNextID(0),
    m_nNextCallbackID(0),
    m_prefix(_T("icon:"))
{
}

IconManager::~IconManager()
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    m_iconMap.clear();
    m_imageStringMap.clear();
}

DString IconManager::GetIconString(uint32_t id) const
{
    if (id == 0) {
        return DString();
    }
    DString str = m_prefix;
    str += StringUtil::UInt32ToString(id);
    return str;
}

bool IconManager::IsIconString(const DString& str) const
{
    return str.find(m_prefix) == 0;
}

uint32_t IconManager::GetIconID(const DString& str) const
{
    uint32_t id = 0;
    size_t pos = str.find(m_prefix);
    ASSERT(pos == 0);
    if (pos == 0) {
        DString idStr = str.substr(pos + m_prefix.size());
        id = StringUtil::StringToUInt32(idStr.c_str(), nullptr, 10);
    }
    return id;
}

UiSize IconManager::GetIconSize(const DString& str) const
{
    UiSize iconSize;
    uint32_t id = GetIconID(str);

    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    auto iter = m_iconMap.find(id);
    if (iter != m_iconMap.end()) {
        iconSize.cx = iter->second.m_nBitmapWidth;
        iconSize.cy = iter->second.m_nBitmapHeight;
    }
    return iconSize;
}

bool IconManager::LoadIconData(const DString& str,
                               const Window* pWindow,
                               const ImageLoadAttribute& loadAtrribute,
                               bool bEnableDpiScale,
                               std::vector<uint8_t>& bitmapData,
                               uint32_t& bitmapWidth,
                               uint32_t& bitmapHeight,
                               bool& bDpiScaled) const
{
    bitmapData.clear();
    bitmapWidth = 0;
    bitmapHeight = 0;
    bDpiScaled = false;

    uint32_t id = GetIconID(str);
    IconBitmapData iconBitmapData;
    if (!GetIconBitmapData(id, iconBitmapData)) {
        ASSERT(!IsImageString(id));
        return false;
    }
    if ((iconBitmapData.m_nBitmapHeight < 1) || (iconBitmapData.m_nBitmapWidth < 1) ||
        iconBitmapData.m_bitmapData.empty() || ((int32_t)iconBitmapData.m_bitmapData.size() != iconBitmapData.m_nBitmapHeight * iconBitmapData.m_nBitmapWidth * 4)){
        ASSERT(0);
        return false;
    }

    int32_t nWidth = iconBitmapData.m_nBitmapWidth;
    int32_t nHeight = iconBitmapData.m_nBitmapHeight;

    uint32_t iconWidth = (uint32_t)nWidth;
    uint32_t iconHeight = (uint32_t)nHeight;
    //此处：如果只设置了宽度或者高度，那么会按锁定纵横比的方式对整个图片进行缩放
    if (!loadAtrribute.CalcImageLoadSize(iconWidth, iconHeight)) {
        iconWidth = (uint32_t)nWidth;
        iconHeight = (uint32_t)nHeight;
    }

    //加载图片时，按需对图片大小进行DPI自适应
    bool bNeedDpiScale = bEnableDpiScale;
    if (loadAtrribute.HasSrcDpiScale()) {
        //如果配置文件中有设置scaledpi属性，则以配置文件中的设置为准
        bNeedDpiScale = loadAtrribute.NeedDpiScale();
    }
    if (bNeedDpiScale && (pWindow != nullptr)) {
        uint32_t dpiScale = pWindow->Dpi().GetScale();
        ASSERT(dpiScale > 0);
        if (dpiScale != 0) {
            float scaleRatio = (float)dpiScale / 100.0f;
            iconWidth = static_cast<uint32_t>(iconWidth * scaleRatio);
            iconHeight = static_cast<uint32_t>(iconHeight * scaleRatio);
            bDpiScaled = true;
        }
    }

    if ((iconWidth > 0) && (iconHeight > 0)) {
        nWidth = (int32_t)iconWidth;
        nHeight = (int32_t)iconHeight;
    }

    bool bRet = true;
    if ((nWidth != iconBitmapData.m_nBitmapWidth) || (nHeight != iconBitmapData.m_nBitmapHeight)) {
        //需要进行Resize
        std::vector<uint8_t> resizedBitmapData;
        resizedBitmapData.resize((size_t)nWidth * nHeight * 4);
        const unsigned char* input_pixels = iconBitmapData.m_bitmapData.data();
        int input_w = iconBitmapData.m_nBitmapWidth;
        int input_h = iconBitmapData.m_nBitmapHeight;
        int input_stride_in_bytes = 0;
        unsigned char* output_pixels = resizedBitmapData.data();
        int output_w = nWidth;
        int output_h = nHeight;
        int output_stride_in_bytes = 0;
        stbir_pixel_layout num_channels = STBIR_RGBA;
        unsigned char* result = stbir_resize_uint8_linear(input_pixels, input_w, input_h, input_stride_in_bytes,
                                                          output_pixels, output_w, output_h, output_stride_in_bytes,
                                                          num_channels);
        if (result != nullptr) {
            bitmapData.swap(resizedBitmapData);
        }
        else {
            //失败了
            bRet = false;
        }
    }
    else {
        bitmapData = iconBitmapData.m_bitmapData;
    }

    if (bRet) {
        bitmapWidth = nWidth;
        bitmapHeight = nHeight;
        ASSERT(bitmapData.size() == bitmapHeight * bitmapWidth * 4);
    }
    return bRet;
}

bool IconManager::GetIconBitmapData(uint32_t id, IconBitmapData& bitmapData) const
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    auto iter = m_iconMap.find(id);
    if (iter != m_iconMap.end()) {
        bitmapData = iter->second;
        return true;
    }
    return false;
}

uint32_t IconManager::AddIcon(const uint8_t* pBitmapData, int32_t nBitmapDataSize, int32_t nBitmapWidth, int32_t nBitmapHeight)
{
    if ((pBitmapData == nullptr) || (nBitmapDataSize < 1) || (nBitmapWidth < 1) || (nBitmapHeight < 1)) {
        return 0;
    }
    ASSERT(nBitmapDataSize == nBitmapHeight * nBitmapWidth * 4);
    if (nBitmapDataSize != nBitmapHeight * nBitmapWidth * 4) {
        return 0;
    }
    IconBitmapData bitmapData;
    bitmapData.m_bitmapData.resize(nBitmapHeight * nBitmapWidth * 4);
    memcpy(bitmapData.m_bitmapData.data(), pBitmapData, (size_t)nBitmapDataSize);
    bitmapData.m_nBitmapHeight = nBitmapHeight;
    bitmapData.m_nBitmapWidth = nBitmapWidth;
    return AddIconBitmapData(bitmapData);
}

uint32_t IconManager::AddIconBitmapData(IconBitmapData& bitmapData)
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    uint32_t nIconID = ++m_nNextID;
    IconBitmapData& data = m_iconMap[nIconID];
    data.m_nBitmapWidth = bitmapData.m_nBitmapWidth;
    data.m_nBitmapHeight = bitmapData.m_nBitmapHeight;
    data.m_bitmapData.swap(bitmapData.m_bitmapData);
    return nIconID;
}

uint32_t IconManager::AddIcon(const DString& imageString)
{
    ASSERT(!imageString.empty());
    if (imageString.empty()) {
        return 0;
    }
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    uint32_t nIconID = ++m_nNextID;
    m_imageStringMap[nIconID] = imageString;
    return nIconID;
}

bool IconManager::IsImageString(uint32_t id) const
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    return m_imageStringMap.find(id) != m_imageStringMap.end();
}

DString IconManager::GetImageString(uint32_t id) const
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    auto iter = m_imageStringMap.find(id);
    if (iter != m_imageStringMap.end()) {
        return iter->second.c_str();
    }
    return DString();
}

void IconManager::RemoveIcon(uint32_t id)
{
    bool bDeleted = false;
    std::vector<RemoveIconEvent> callbackList;
    {
        std::lock_guard<std::mutex> threadGuard(m_iconMutex);
        auto iconIter = m_iconMap.find(id);
        if (iconIter != m_iconMap.end()) {
            m_iconMap.erase(iconIter);
            bDeleted = true;
        }
        auto iterImage = m_imageStringMap.find(id);
        if (iterImage != m_imageStringMap.end()) {
            m_imageStringMap.erase(iterImage);
            bDeleted = true;
        }
        if (bDeleted) {
            for (auto iter : m_callbackMap) {
                callbackList.push_back(iter.second);
            }
        }
    }
    if (!callbackList.empty()) {
        for (RemoveIconEvent callback : callbackList) {
            if (callback) {
                callback(id);
            }
        }
    }    
}

uint32_t IconManager::AttachRemoveIconEvent(RemoveIconEvent callback)
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    uint32_t callbackId = 0;
    if (callback != nullptr) {
        callbackId = ++m_nNextCallbackID;
        m_callbackMap[callbackId] = callback;
    }
    return callbackId;
}

void IconManager::DetachRemoveIconEvent(uint32_t callbackID)
{
    std::lock_guard<std::mutex> threadGuard(m_iconMutex);
    auto iter = m_callbackMap.find(callbackID);
    if (iter != m_callbackMap.end()) {
        m_callbackMap.erase(callbackID);
    }
}

#ifdef DUILIB_BUILD_FOR_WIN

/** 图标资源的位图句柄自动释放
*/
struct ScopedICONINFO :
    public ICONINFO {
    ScopedICONINFO() {
        hbmColor = nullptr;
        hbmMask = nullptr;
    }
    ~ScopedICONINFO() {
        if (hbmColor) {
            ::DeleteObject(hbmColor);
        }
        if (hbmMask) {
            ::DeleteObject(hbmMask);
        }
    }
};

/** 将HICON转换为位图
*/
static bool IconToBitmap(HICON hIcon, IconBitmapData& bitmapData)
{
    bitmapData.m_bitmapData.clear();
    bitmapData.m_nBitmapWidth = 0;
    bitmapData.m_nBitmapHeight = 0;

    if (hIcon == nullptr) {
        return false;
    }

    ScopedICONINFO iconInfo;
    if (!::GetIconInfo(hIcon, &iconInfo)) {
#ifdef _DEBUG
        DWORD dwLastError = ::GetLastError();
        (void)dwLastError;
        ASSERT(!"GetIconInfo failed!");
#endif
        return false;
    }

    if (!iconInfo.fIcon) {
        return false;
    }

    int32_t nWidth = 0;
    int32_t nHeight = 0;
    if (iconInfo.hbmColor != nullptr) {
        BITMAP bmp = { 0 };
        ::GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);
        nWidth = bmp.bmWidth;
        nHeight = bmp.bmHeight;
    }
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return false;
    }

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = nWidth;
    bi.biHeight = -nHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    HWND hWnd = nullptr;
    HDC hdc = ::GetDC(hWnd);
    ASSERT(hdc != nullptr);
    uint32_t* bits = nullptr;
    HBITMAP dib = ::CreateDIBSection(hdc,
                                     reinterpret_cast<BITMAPINFO*>(&bi),
                                     DIB_RGB_COLORS,
                                     reinterpret_cast<void**>(&bits),
                                     nullptr,
                                     0);
    ASSERT(dib != nullptr);
    if (dib == nullptr)    {
        ::ReleaseDC(hWnd, hdc);
        return false;
    }
    if (bits == nullptr) {
        ::ReleaseDC(hWnd, hdc);
        ::DeleteObject(dib);
        return false;
    }
    HDC dibDC = ::CreateCompatibleDC(hdc);
    ::ReleaseDC(hWnd, hdc);
    hdc = nullptr;

    HGDIOBJ oldObj = ::SelectObject(dibDC, dib);

    const size_t num_pixels = (size_t)nWidth * nHeight;
    memset(bits, 0, num_pixels * 4);
    ::DrawIconEx(dibDC, 0, 0, hIcon, nWidth, nHeight, 0, nullptr, DI_MASK);

    bool* opaque = (new bool[num_pixels]);
    for (size_t i = 0; i < num_pixels; ++i)    {
        opaque[i] = !bits[i];
    }

    memset(bits, 0, num_pixels * 4);
    ::DrawIconEx(dibDC, 0, 0, hIcon, nWidth, nHeight, 0, nullptr, DI_NORMAL);

    bitmapData.m_bitmapData.resize(num_pixels * 4, 0);
    ::memcpy(bitmapData.m_bitmapData.data(), (void*)(bits), num_pixels * 4);

    bool bitmap_has_alpha_channel = false;
    const uint32_t* pixels = static_cast<const uint32_t*>((const void*)bitmapData.m_bitmapData.data());
    for (const uint32_t* end = pixels + num_pixels; pixels != end; ++pixels) {
        if ((*pixels & 0xff000000) != 0) {
            bitmap_has_alpha_channel = true;
        }
    }
    if (!bitmap_has_alpha_channel) {
        uint32_t* p = static_cast<uint32_t*>((void*)bitmapData.m_bitmapData.data());
        for (size_t i = 0; i < num_pixels; ++p, ++i) {
            if (opaque[i]) {
                *p |= 0xff000000;
            }
            else {
                *p &= 0x00000000;
            }
        }
    }

    ::SelectObject(dibDC, oldObj);
    ::DeleteObject(dib);
    ::DeleteDC(dibDC);
    delete[] opaque;

    bitmapData.m_nBitmapWidth = nWidth;
    bitmapData.m_nBitmapHeight = nHeight;
    ASSERT((int32_t)bitmapData.m_bitmapData.size() == bitmapData.m_nBitmapHeight * bitmapData.m_nBitmapWidth * 4);
    return true;
}

uint32_t IconManager::AddIcon(HICON hIcon)
{
    if (hIcon == nullptr) {
        return 0;
    }
    IconBitmapData bitmapData;
    if (!IconToBitmap(hIcon, bitmapData)) {
        ASSERT(0);
        return 0;
    }
    return AddIconBitmapData(bitmapData);
}

#endif //DUILIB_BUILD_FOR_WIN

}

