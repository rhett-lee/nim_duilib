#include "duilib/Utils/TrayIcon.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FileUtil.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>

namespace ui
{

/** 托盘图标的SDL实现
*/
class TrayIconImpl : public TrayIcon
{
public:
    TrayIconImpl();
    virtual ~TrayIconImpl() override;

    /** 初始化托盘图标
    * @param [in] pWindow 关联的窗口指针
    * @param [in] iconFilePath 图标文件路径（支持ico格式）
    * @param [in] tooltip 托盘提示文本
    * @return 初始化成功返回true，失败返回false
    */
    bool Initialize(const Window* pWindow, const DString& iconFilePath, const DString& tooltip);

public:
    virtual bool SetIcon(const Window* pWindow, const DString& iconFilePath) override;
    virtual bool SetTooltip(const DString& tooltip) override;
    virtual bool ShowBalloon(const DString& title, const DString& content, uint32_t timeoutMs = 3000) override;
    virtual bool Hide() override;
    virtual bool Show() override;
    virtual bool IsTrayVisible() const override;
    virtual bool Remove() override;
    virtual void* GetTrayHandle() const override;

private:
    struct TraySurfaceData
    {
        /** 当前图标表面
        */
        SDL_Surface* m_pIconSurface = nullptr;

        /** 当前图标表面关联的数据
        */
        std::shared_ptr<IBitmap> m_pBitmap;

        /** 清理资源
        */
        void Clear()
        {
            if (m_pIconSurface != nullptr) {
                SDL_DestroySurface(m_pIconSurface);
                m_pIconSurface = nullptr;
            }
            m_pBitmap.reset();
        }
    };

    /** 从文件加载图标为SDL_Surface
    * @param [in] pWindow 关联的窗口
    * @param [in] iconFilePath 图标文件路径
    */
    TraySurfaceData LoadSDLSurfaceFromFile(const Window* pWindow, const DString& iconFilePath);

    /** 从文件数据加载图标为SDL_Surface
    * @param [in] fileData 文件数据
    * @param [in] iconFilePath 文件路径
    */
    TraySurfaceData LoadSDLSurfaceFromFileData(const std::vector<uint8_t>& fileData, const DString& iconFilePath);

    /** 鼠标点击回调事件(左键，右键，中键点击)
    */
    static bool SDLCALL OnSDLTrayLeftClickCallback(void* userdata, SDL_Tray* tray);
    static bool SDLCALL OnSDLTrayRightClickCallback(void* userdata, SDL_Tray* tray);
    static bool SDLCALL OnSDLTrayMiddleClickCallback(void* userdata, SDL_Tray* tray);

    /** 鼠标点击回调事件(左键，右键，中键点击)
    */
    bool OnTrayLeftClickCallback(SDL_Tray* tray);
    bool OnTrayRightClickCallback(SDL_Tray* tray);
    bool OnTrayMiddleClickCallback(SDL_Tray* tray);

    /** 获取当前鼠标位置(屏幕坐标)
    */
    void GetMousePosition(int32_t& x, int32_t& y);

private:
    /** SDL托盘对象指针
    */
    SDL_Tray* m_sdlTray;

    /** 当前图标表面
    */
    TraySurfaceData m_iconSurface;

    /** 提示文本
    */
    DString m_tooltip;

    /** 是否隐藏
    */
    bool m_bHidden;

    /** 关联的窗口指针
    */
    const Window* m_pWindow;
};

TrayIconImpl::TrayIconImpl() :
    m_sdlTray(nullptr),
    m_bHidden(false),
    m_pWindow(nullptr)
{
}

TrayIconImpl::~TrayIconImpl()
{
    Remove();
}

bool TrayIconImpl::Initialize(const Window* pWindow, const DString& iconFilePath, const DString& tooltip)
{
    m_pWindow = pWindow;
    m_tooltip = tooltip;
    m_bHidden = false;

    // 加载图标
    m_iconSurface = LoadSDLSurfaceFromFile(pWindow, iconFilePath);

    // 创建SDL托盘图标
    return Show();
}

TrayIconImpl::TraySurfaceData TrayIconImpl::LoadSDLSurfaceFromFile(const Window* pWindow, const DString& iconFilePath)
{
    if (iconFilePath.empty()) {
        return TraySurfaceData();
    }

    FilePath windowResPath;
    FilePath windowXmlPath;
    if (pWindow != nullptr) {
        windowResPath = pWindow->GetResourcePath();
        windowXmlPath = pWindow->GetXmlPath();
    }
    FilePath iconFullPath = GlobalManager::Instance().GetExistsResFullPath(windowResPath, windowXmlPath, FilePath(iconFilePath));
    ASSERT(!iconFullPath.IsEmpty());
    if (iconFullPath.IsEmpty()) {
        return TraySurfaceData();
    }

    std::vector<uint8_t> fileData;
    if (GlobalManager::Instance().Zip().IsUseZip() &&
        GlobalManager::Instance().Zip().IsZipResExist(iconFullPath)) {
        //使用压缩包        
        GlobalManager::Instance().Zip().GetZipData(iconFullPath, fileData);
    }
    else {
        //使用本地文件
        ASSERT(iconFullPath.IsExistsFile());
        if (iconFullPath.IsExistsFile()) {
            FileUtil::ReadFileData(iconFullPath, fileData);
        }
    }
    return LoadSDLSurfaceFromFileData(fileData, iconFullPath.ToString());
}

TrayIconImpl::TraySurfaceData TrayIconImpl::LoadSDLSurfaceFromFileData(const std::vector<uint8_t>& fileData, const DString& iconFilePath)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return TraySurfaceData();
    }

    ImageDecoderFactory& imageDecoders = GlobalManager::Instance().ImageDecoders();
    float fImageSizeScale = GlobalManager::Instance().Dpi().GetDisplayScale();
    ImageDecodeParam decodeParam;
    decodeParam.m_imageFilePath = iconFilePath;
    decodeParam.m_fImageSizeScale = fImageSizeScale;
    decodeParam.m_pFileData = std::make_shared<std::vector<uint8_t>>(fileData);
    decodeParam.m_rcMaxDestRectSize.cx = (int32_t)std::round(32 * fImageSizeScale);
    decodeParam.m_rcMaxDestRectSize.cy = decodeParam.m_rcMaxDestRectSize.cx;
    std::shared_ptr<IBitmap> pBitmap = imageDecoders.DecodeImageData(decodeParam);
    if (pBitmap == nullptr) {
        return TraySurfaceData();
    }
    uint32_t nWidth = pBitmap->GetWidth();
    uint32_t nHeight = pBitmap->GetHeight();
    if ((nWidth < 1) || (nHeight < 1)) {
        return TraySurfaceData();
    }

    void* pPixelBits = pBitmap->LockPixelBits();
    ASSERT(pPixelBits != nullptr);
    if (pPixelBits == nullptr) {
        return TraySurfaceData();
    }

#ifdef DUILIB_BUILD_FOR_WIN
    SDL_PixelFormat format = SDL_PIXELFORMAT_BGRA32;
#else
    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
#endif
    SDL_Surface* iconSurface = SDL_CreateSurfaceFrom(pBitmap->GetWidth(), pBitmap->GetHeight(), format, pPixelBits, pBitmap->GetWidth() * sizeof(uint32_t));
    ASSERT(iconSurface != nullptr);
    TraySurfaceData surfaceData;
    surfaceData.m_pIconSurface = iconSurface;
    surfaceData.m_pBitmap = pBitmap;
    return surfaceData;
}

bool TrayIconImpl::SetIcon(const Window* pWindow, const DString& iconFilePath)
{
    if (m_sdlTray == nullptr) {
        return false;
    }

    // 释放旧图标
    m_iconSurface.Clear();

    // 加载新图标
    m_iconSurface = LoadSDLSurfaceFromFile(pWindow, iconFilePath);

    // 设置托盘图标
    SDL_SetTrayIcon(m_sdlTray, m_iconSurface.m_pIconSurface);
    return true;
}

bool TrayIconImpl::SetTooltip(const DString& tooltip)
{
    if (m_sdlTray == nullptr) {
        return false;
    }

    m_tooltip = tooltip;
    std::string tooltipUTF8 = StringConvert::TToUTF8(tooltip);
    SDL_SetTrayTooltip(m_sdlTray, tooltipUTF8.c_str());
    return true;
}

bool TrayIconImpl::ShowBalloon(const DString& title, const DString& content, uint32_t timeoutMs)
{
    // SDL3的托盘API目前不直接支持气泡提示
    // 可以考虑使用SDL_ShowSimpleMessageBox或其他方式实现
    UNUSED_VARIABLE(title);
    UNUSED_VARIABLE(content);
    UNUSED_VARIABLE(timeoutMs);
    return false;
}

bool TrayIconImpl::Hide()
{
    // SDL的托盘API没有隐藏功能，只能销毁后重建
    // 注意：销毁托盘会同时销毁菜单和菜单项
    if (m_sdlTray != nullptr) {
        SDL_DestroyTray(m_sdlTray);
        m_sdlTray = nullptr;
    }
    m_bHidden = true;
    return true;
}

bool TrayIconImpl::Show()
{
    // 重建托盘图标
    if (m_sdlTray == nullptr) {
        std::string tooltipUTF8 = StringConvert::TToUTF8(m_tooltip);

        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetPointerProperty(props, SDL_PROP_TRAY_CREATE_ICON_POINTER, m_iconSurface.m_pIconSurface);
        SDL_SetStringProperty(props, SDL_PROP_TRAY_CREATE_TOOLTIP_STRING, tooltipUTF8.c_str());
        SDL_SetPointerProperty(props, SDL_PROP_TRAY_CREATE_USERDATA_POINTER, this);
        SDL_SetPointerProperty(props, SDL_PROP_TRAY_CREATE_LEFTCLICK_CALLBACK_POINTER, (void*)&TrayIconImpl::OnSDLTrayLeftClickCallback);
        SDL_SetPointerProperty(props, SDL_PROP_TRAY_CREATE_RIGHTCLICK_CALLBACK_POINTER, (void*)&TrayIconImpl::OnSDLTrayRightClickCallback);
        SDL_SetPointerProperty(props, SDL_PROP_TRAY_CREATE_MIDDLECLICK_CALLBACK_POINTER, (void*)&TrayIconImpl::OnSDLTrayMiddleClickCallback);
        m_sdlTray = SDL_CreateTrayWithProperties(props);
        SDL_DestroyProperties(props);
    }
    m_bHidden = false;
    return m_sdlTray != nullptr;
}

bool TrayIconImpl::IsTrayVisible() const
{
    return (m_sdlTray != nullptr) && !m_bHidden;
}

bool TrayIconImpl::Remove()
{
    if (m_sdlTray != nullptr) {
        SDL_DestroyTray(m_sdlTray);
        m_sdlTray = nullptr;
    }
    m_iconSurface.Clear();
    m_bHidden = true;
    return true;
}

void* TrayIconImpl::GetTrayHandle() const
{
    return (void*)m_sdlTray;
}

void TrayIconImpl::GetMousePosition(int32_t& x, int32_t& y)
{
    float fx = 0.0f;
    float fy = 0.0f;
    SDL_GetGlobalMouseState(&fx, &fy);
    x = static_cast<int32_t>(fx);
    y = static_cast<int32_t>(fy);
}

bool SDLCALL TrayIconImpl::OnSDLTrayLeftClickCallback(void* userdata, SDL_Tray* tray)
{
    TrayIconImpl* pThis = (TrayIconImpl*)userdata;
    if (pThis != nullptr) {
        return pThis->OnTrayLeftClickCallback(tray);
    }
    return false;
}

bool SDLCALL TrayIconImpl::OnSDLTrayRightClickCallback(void* userdata, SDL_Tray* tray)
{
    TrayIconImpl* pThis = (TrayIconImpl*)userdata;
    if (pThis != nullptr) {
        return pThis->OnTrayRightClickCallback(tray);
    }
    return false;
}

bool SDLCALL TrayIconImpl::OnSDLTrayMiddleClickCallback(void* userdata, SDL_Tray* tray)
{
    TrayIconImpl* pThis = (TrayIconImpl*)userdata;
    if (pThis != nullptr) {
        return pThis->OnTrayMiddleClickCallback(tray);
    }
    return false;
}

bool TrayIconImpl::OnTrayLeftClickCallback(SDL_Tray* tray)
{
    ASSERT(tray == m_sdlTray);
    if (tray == m_sdlTray) {
        int32_t x = 0;
        int32_t y = 0;
        GetMousePosition(x, y);
        NotifyMessage(TrayIconMessageType::kLeftClick, x, y);
    }
    return false;//返回false，不显示SDL内置菜单
}

bool TrayIconImpl::OnTrayRightClickCallback(SDL_Tray* tray)
{
    ASSERT(tray == m_sdlTray);
    if (tray == m_sdlTray) {
        int32_t x = 0;
        int32_t y = 0;
        GetMousePosition(x, y);
        NotifyMessage(TrayIconMessageType::kRightClick, x, y);
    }
    return false;//返回false，不显示SDL内置菜单
}

bool TrayIconImpl::OnTrayMiddleClickCallback(SDL_Tray* tray)
{
    ASSERT(tray == m_sdlTray);
    if (tray == m_sdlTray) {
        int32_t x = 0;
        int32_t y = 0;
        GetMousePosition(x, y);
        NotifyMessage(TrayIconMessageType::kMiddleClick, x, y);
    }
    return false;//返回false，不显示SDL内置菜单
}

// TrayIcon 基类的Create函数，SDL平台实现
std::unique_ptr<TrayIcon> TrayIcon::Create(const Window* pWindow, const DString& iconFilePath, const DString& tooltip)
{
    std::unique_ptr<TrayIconImpl> pTrayIcon = std::make_unique<TrayIconImpl>();
    if (pTrayIcon->Initialize(pWindow, iconFilePath, tooltip)) {
        return pTrayIcon;
    }
    return nullptr;
}

} //namespace ui

#endif // DUILIB_BUILD_FOR_SDL
