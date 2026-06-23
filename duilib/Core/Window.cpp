#include "Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/FullscreenBox.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ToolTip.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/WindowRoot.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{
Window::Window() :
    m_pFocus(nullptr),
    m_pEventHover(nullptr),
    m_pEventToolTip(nullptr),
    m_pEventClick(nullptr),
    m_pEventKey(nullptr),
    m_bFirstLayout(false),
    m_bInitLayout(false),
    m_bIsArranged(false),
    m_bPostQuitMsgWhenClosed(false),
    m_renderBackendType(RenderBackendType::kRaster_BackendType),
    m_bWindowAttributesApplied(false),
    m_bCheckSetWindowFocus(false),
    m_bWindowShadowInited(false)
{
    m_toolTip = std::make_unique<ToolTip>();
    m_windowRoot = std::make_unique<WindowRoot>(this);
    m_windowRoot->SetControlFinder(&m_controlFinder);
}

Window::~Window()
{
    ASSERT(!IsWindow());
    ClearWindow();
    m_windowRoot.reset();
    m_windowBuilder.reset();
    m_pColorManager.reset();
}

void Window::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("use_system_caption")) {
        //是否使用操作系统默认的标题栏
        SetUseSystemCaption(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("shadow_attached")) {
        //是否开启阴影
        SetShadowAttached(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("shadow_type")) {
        //设置窗口的阴影类型
        ShadowType nShadowType = ShadowType::kShadowDefault;
        if (Shadow::GetShadowType(strValue, nShadowType)) {
            SetShadowType(nShadowType);
        }
    }    
    else if (strName == _T("layered_window")) {
        //是否为分层窗口
        SetLayeredWindow(StringUtil::IsValueTrue(strValue), true);
    }
    else if ((strName == _T("alpha")) || (strName == _T("layered_window_alpha"))) {
        //分层窗口的透明度, 该值在UpdateLayeredWindow函数中作为参数使用
        SetLayeredWindowAlpha(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("opacity")) || (strName == _T("layered_window_opacity"))) {
        //分层窗口的透明度, 该值在SetLayeredWindowAttributes函数中作为参数使用
        SetLayeredWindowOpacity(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("drag_drop")) {
        //是否允许拖放操作
        SetEnableDragDrop(StringUtil::IsValueTrue(strValue));
    }
}

void Window::SetEnableDragDrop(bool bEnable)
{
    NativeWnd()->SetEnableDragDrop(bEnable);
}

bool Window::IsEnableDragDrop() const
{
    return NativeWnd()->IsEnableDragDrop();
}

void Window::SetClass(const DString& strClass)
{
    if (strClass.empty()) {
        return;
    }
    std::list<DString> splitList = StringUtil::Split(strClass, _T(" "));
    for (auto it = splitList.begin(); it != splitList.end(); it++) {
        DString pDefaultAttributes = GlobalManager::Instance().GetClassAttributes((*it));
        if (pDefaultAttributes.empty()) {
            pDefaultAttributes = GetClassAttributes(*it);
        }

        ASSERT(!pDefaultAttributes.empty());
        if (!pDefaultAttributes.empty()) {
            ApplyAttributeList(pDefaultAttributes);
        }
    }
}

void Window::ApplyAttributeList(const DString& strList)
{
    //属性列表，先解析，然后再应用
    if (strList.empty()) {
        return;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(strList, attributeList);
    for (const auto& attribute : attributeList) {
        SetAttribute(attribute.first, attribute.second);
    }
}

Window* Window::GetParentWindow() const
{
    WindowBase* pWindowBase = WindowBase::GetParentWindow();
    if (pWindowBase != nullptr) {
        return dynamic_cast<Window*>(pWindowBase);
    }
    else {
        return nullptr;
    }
}

bool Window::SetRenderBackendType(RenderBackendType backendType)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_renderBackendType = backendType;
#else
    backendType = RenderBackendType::kRaster_BackendType;
    m_renderBackendType = backendType;
#endif
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    bool bRet = false;
    if (m_render == nullptr) {
        //首次调用时，初始化
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            m_render.reset(pRenderFactory->CreateRender(GetRenderDpi(), GetWindowHandle(), m_renderBackendType));
            bRet = (m_render != nullptr);
        }
    }
    else {
        ASSERT(m_render->GetRenderBackendType() == backendType);
        bRet = (m_render->GetRenderBackendType() == backendType);
    }
    ASSERT(bRet);
    return bRet;
}

RenderBackendType Window::GetRenderBackendType() const
{
    RenderBackendType backendType = m_renderBackendType;
    if (m_render != nullptr) {
        backendType = m_render->GetRenderBackendType();
    }
    return backendType;
}

bool Window::SetWindowIcon(const DString& iconFilePath)
{
    if (iconFilePath.empty()) {
        return false;
    }
    bool bRet = false;
    FilePath iconFullPath = GlobalManager::Instance().GetExistsResFullPath(GetResourcePath(), GetXmlPath(), FilePath(iconFilePath));
    ASSERT(!iconFullPath.IsEmpty());
    if (iconFullPath.IsEmpty()) {
        return false;
    }
    if (GlobalManager::Instance().Zip().IsUseZip() &&
        GlobalManager::Instance().Zip().IsZipResExist(iconFullPath)) {
        //使用压缩包
        std::vector<uint8_t> fileData;
        GlobalManager::Instance().Zip().GetZipData(iconFullPath, fileData);
        ASSERT(!fileData.empty());
        if (!fileData.empty()) {
            bRet = WindowBase::SetWindowIcon(fileData, iconFilePath);
        }
    }
    else {
        //使用本地文件
        if (iconFullPath.IsExistsFile()) {
            bRet = WindowBase::SetWindowIcon(iconFullPath);
        }
        else {
            ASSERT(false);
        }
    }
    return bRet;
}

void Window::InitSkin(const DString& skinFolder, const DString& skinFile)
{
    m_skinFolder = skinFolder;
    m_skinFile = skinFile;
    m_windowBuilder.reset();
}

DString Window::GetSkinFolder()
{
    return m_skinFolder;
}

DString Window::GetSkinFile()
{
    return m_skinFile;
}

Control* Window::CreateControl(const DString& /*strClass*/)
{
    return nullptr;
}

void Window::GetCreateWindowAttributes(WindowCreateAttributes& createAttributes)
{
    //解析窗口关联的XML文件
    if (m_windowBuilder == nullptr) {
        ParseWindowXml();
    }

    //解析出窗口的属性
    if (m_windowBuilder != nullptr) {
        m_windowBuilder->ParseWindowCreateAttributes(this, createAttributes);
    }
}

void Window::ParseWindowXml()
{
    FilePath skinFolder(GetSkinFolder());
    DString xmlFile = GetSkinFile();
    if (skinFolder.IsEmpty() && xmlFile.empty()) {
        return;
    }

    //资源路径不支持绝对路径
    ASSERT(!skinFolder.IsAbsolutePath());
    if (skinFolder.IsAbsolutePath()) {
        return;
    }

    //保存资源所在路径
    SetResourcePath(skinFolder);
    SetXmlPath(FilePath());

    //XML文件所在路径，应是相对路径    
    DString skinXmlFileData;
    FilePath skinXmlFilePath;
    if (!xmlFile.empty() && xmlFile.front() == _T('<')) {
        //返回的内容是XML文件内容，而不是文件路径        
        skinXmlFileData = std::move(xmlFile);
    }
    else {
        const FilePath xmlFilePath(xmlFile);
        ASSERT(!xmlFilePath.IsAbsolutePath());
        if (xmlFilePath.IsAbsolutePath()) {
            return;
        }

        //保存XML文件所在路径
        size_t nPos = xmlFile.find_last_of(_T("/\\"));
        if (nPos != DString::npos) {
            DString xmlPath = xmlFile.substr(0, nPos);
            if (!xmlPath.empty()) {
                SetXmlPath(FilePath(xmlPath));
            }
        }
        skinXmlFilePath = xmlFilePath;
    }
    //解析XML文件
    bool bRet = false;
    m_windowBuilder = std::make_unique<WindowBuilder>();
    if (!skinXmlFileData.empty()) {
        bRet = m_windowBuilder->ParseXmlData(skinXmlFileData);
    }
    else {
        ASSERT(!skinXmlFilePath.IsEmpty());
        bRet = m_windowBuilder->ParseXmlFile(skinXmlFilePath, GetResourcePath());
    }
    if (!bRet) {
        m_windowBuilder.reset();
        SetResourcePath(FilePath());
        SetXmlPath(FilePath());
    }
}

void Window::PreInitWindow()
{
    if (!IsWindow()) {
        return;
    }
    //添加到全局管理器
    ASSERT(!GlobalManager::Instance().Windows().HasWindow(this));
    GlobalManager::Instance().Windows().AddWindow(this);

    ASSERT(m_windowRoot != nullptr);
    if (m_windowRoot == nullptr) {
        return;
    }
    m_windowRoot->CreateShadow(IsLayeredWindow());
    //解析窗口关联的XML文件
    if (m_windowBuilder == nullptr) {
        ParseWindowXml();
    }

    Box* pRoot = nullptr;
    if (m_windowBuilder != nullptr) {
        auto callback = UiBind(&Window::CreateControl, this, std::placeholders::_1);
        Control* pControl = m_windowBuilder->CreateControls(this, callback);
        if (pControl == nullptr) {
            // XML 解析或控件创建失败，记录并中止初始化
            ASSERT(!"Window::PreInitWindow: failed to create controls from XML");
            return;
        }
        pRoot = m_windowBuilder->ToBox(pControl);
        ASSERT(pRoot != nullptr);
        if (pRoot == nullptr) {
            // 根控件不是 Box 类型（XML 顶层必须为 Box）
            ASSERT(!"Window::PreInitWindow: root control is not a Box type");
            return;
        }
    }

    if (pRoot != nullptr) {
        //关联窗口附加阴影
        pRoot = AttachShadow(pRoot);

        if (IsUseSystemCaption()) {
            //使用系统标题栏的时候，关闭阴影
            SetShadowAttached(false);
        }

        //关联Root对象
        AttachBox(pRoot);

        //当前阴影状态
        bool bShadowAttached = IsShadowAttached();

        //更新自绘制标题栏状态
        OnUseSystemCaptionBarChanged();

        //初始化阴影        
        if (!m_bWindowShadowInited) {
            m_bWindowShadowInited = true;
            SetShadowAttached(bShadowAttached);
        }
        else if (!IsUseSystemCaption()) {
            //保持原有的阴影状态
            SetShadowAttached(bShadowAttached);
        }
    }
}

void Window::PostInitWindow()
{
    //创建渲染接口
    if (m_render == nullptr) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            m_render.reset(pRenderFactory->CreateRender(GetRenderDpi(), GetWindowHandle(), m_renderBackendType));
        }
    }
    ASSERT(m_render != nullptr);

    //根据XML的size属性，初始化窗口大小
    if ((m_szInitSize.cx > 0) && (m_szInitSize.cy > 0)) {
        Resize(m_szInitSize.cx, m_szInitSize.cy, true, false);
    }

    //检测是否需要根据root节点的auto类型设置窗口大小（比如菜单等有此种用法）
    if (AutoResizeWindow(false)) {
        //调整大小后，需要再次进行窗口居中
        if (NativeWnd()->NeedCenterWindowAfterCreated()) {
            CenterWindow();
        }
    }

    //创建后，Render大小与客户区大小同步
    ResizeRenderToClientSize();
}

void Window::PreCloseWindow()
{
    ClearInputStatus();
    OnPreCloseWindow();

    //销毁Tooltp窗口
    if (m_toolTip != nullptr) {
        m_toolTip->DestroyToolTip();
    }
}

void Window::PostCloseWindow()
{
    if (m_bPostQuitMsgWhenClosed) {
        this->PostQuitMsg(0);
    }
    OnCloseWindow();
}

void Window::FinalMessage()
{
    //按倒序清理资源
    ClearWindow();
    ClearWindowBase();

    //回调Final接口
    OnFinalMessage();
}

void Window::OnFinalMessage()
{
    if (!IsDoModal()) {
        delete this;
    }
}

void Window::ClearWindow()
{
    //回收控件
    GlobalManager::Instance().Windows().RemoveWindow(this);
    ReapObjects(GetRoot());

    m_controlFinder.Clear();
    m_toolTip.reset();
    m_render.reset();    
    m_windowRoot->Clear();

    RemoveAllClass();
    RemoveAllOptionGroups();
}

bool Window::AttachBox(Box* pRoot)
{
    ASSERT(IsWindow());
    SetFocusControl(nullptr); //设置m_pFocus相关的状态
    m_pEventKey = nullptr;
    m_pEventHover = nullptr;
    m_pEventToolTip = nullptr;
    m_pEventClick = nullptr;
    m_windowRoot->AttachBox(pRoot);
    m_controlFinder.SetRoot(m_windowRoot->GetRoot());
    m_bIsArranged = true;
    m_bFirstLayout = false;
    m_bInitLayout = false;
    return InitControls(m_windowRoot->GetRoot());
}

Box* Window::GetRoot() const
{
    return m_windowRoot->GetRoot();
}

Box* Window::GetXmlRoot() const
{
    return m_windowRoot->GetXmlRoot();
}

bool Window::InitControls(Control* pControl)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    m_controlFinder.AddControl(pControl);
    pControl->SetWindow(this);
    return true;
}

void Window::ReapObjects(Control* pControl)
{
    if (pControl == nullptr) {
        return;
    }
    if (pControl == m_pEventKey) {
        m_pEventKey = nullptr;
    }
    if (pControl == m_pEventHover) {
        m_pEventHover = nullptr;
    }
    if (pControl == m_pEventToolTip) {
        m_pEventToolTip = nullptr;
    }    
    if (pControl == m_pEventClick) {
        m_pEventClick = nullptr;
    }
    bool bFocusChanged = false;
    if (pControl == m_pFocus) {
        bFocusChanged = (m_pFocus != nullptr) ? true : false;
        m_pFocus = nullptr;        
    }
    if (!IsClosingWnd()) {
        m_controlFinder.RemoveControl(pControl);
        if (bFocusChanged) {
            OnFocusControlChanged();
        }
    }    
}

void Window::SetResourcePath(const FilePath& strPath)
{
    m_resourcePath = strPath;
    m_resourcePath.NormalizeDirectoryPath();
}

const FilePath& Window::GetResourcePath() const
{
    return m_resourcePath;
}

void Window::SetXmlPath(const FilePath& xmlPath)
{
    m_xmlPath = xmlPath;
    m_xmlPath.NormalizeDirectoryPath();
}

const FilePath& Window::GetXmlPath() const
{
    return m_xmlPath;
}

void Window::AddClass(const DString& strClassName, const DString& strControlAttrList)
{
    ASSERT(!strClassName.empty());
    ASSERT(!strControlAttrList.empty());
#ifdef _DEBUG
    //检查：避免误修改
    auto iter = m_defaultAttrHash.find(strClassName);
    if (iter != m_defaultAttrHash.end()) {
        ASSERT(iter->second == strControlAttrList);
    }
#endif
    m_defaultAttrHash[strClassName] = strControlAttrList;
}

DString Window::GetClassAttributes(const DString& strClassName) const
{
    auto it = m_defaultAttrHash.find(strClassName);
    if (it != m_defaultAttrHash.end()) {
        return it->second;
    }
    return _T("");
}

bool Window::RemoveClass(const DString& strClassName)
{
    auto it = m_defaultAttrHash.find(strClassName);
    if (it != m_defaultAttrHash.end()) {
        m_defaultAttrHash.erase(it);
        return true;
    }
    return false;
}

void Window::RemoveAllClass()
{
    m_defaultAttrHash.clear();
}

void Window::AddThemeColor(const DString& strName, const DString& strValue)
{
    m_colorMap.AddColor(strName, strValue);
}

void Window::AddThemeColor(const DString& strName, UiColor argb)
{
    m_colorMap.AddColor(strName, argb);
}

void Window::RemoveThemeColor(const DString& strName)
{
    m_colorMap.RemoveColor(strName);
}

UiColor Window::GetThemeColor(const DString& strName) const
{
    UiColor color = m_colorMap.GetColor(strName);
    if (color.IsEmpty() && (m_pColorManager != nullptr)) {
        //使用窗口自身的主题颜色管理器
        color = m_pColorManager->GetColor(strName);
    }
    return color;
}

bool Window::OpenColorTheme(const FilePath& themePath)
{
    ASSERT(!themePath.IsEmpty());
    if (themePath.IsEmpty()) {
        return false;
    }
    FilePath globalXmlFileName = FilePath(GlobalManager::Instance().Theme().GetGlobalXmlFileName());
    ASSERT(!globalXmlFileName.IsEmpty());
    if (globalXmlFileName.IsEmpty()) {
        return false;
    }

    FilePath themeFullPath = GlobalManager::Instance().Theme().GetThemeRootPath();
    themeFullPath /= themePath;
    themeFullPath /= globalXmlFileName;

    WindowBuilder windowBuilder;
    std::string xmlFileData = windowBuilder.ReadXmlFileData(themeFullPath);
    ASSERT(!xmlFileData.empty());
    if (xmlFileData.empty()) {
        return false;
    }
    return OpenColorThemeData(xmlFileData);
}

bool Window::OpenColorThemeData(const std::string& themeXmlFileData)
{
    ASSERT(!themeXmlFileData.empty());
    if (themeXmlFileData.empty()) {
        return false;
    }
    std::vector<unsigned char> xmlFileData;
    xmlFileData.resize(themeXmlFileData.size());
    memcpy(xmlFileData.data(), themeXmlFileData.data(), xmlFileData.size());

    WindowBuilder globalbuilder;
    if (!globalbuilder.ParseXmlData(xmlFileData)) {
        ASSERT(!"ParseXmlFile failed!");
        return false;
    }

    //初始化主题数据
    DString themeName;
    DString themeType;
    DString themeStyle;
    globalbuilder.ParseThemeInfo(themeName, themeType, themeStyle);
    ASSERT(!themeName.empty() && !themeType.empty() && !themeStyle.empty());
    if (themeName.empty() || themeType.empty() || themeStyle.empty()) {
        return false;
    }

    //颜色主题
    ThemeType readThemeType = GlobalManager::Instance().Theme().GetThemeTypeValue(themeType);
    ASSERT((readThemeType == ThemeType::kColor) || (readThemeType == ThemeType::kCombined));
    if ((readThemeType != ThemeType::kColor) &&
        (readThemeType != ThemeType::kCombined)) {
        return false;
    }

    ThemeStyle readThemeStyle = GlobalManager::Instance().Theme().GetThemeStyleValue(themeStyle);
    m_pColorManager = std::make_unique<ColorManager>();
    globalbuilder.ParseThemeColor(*m_pColorManager);
    m_pColorManager->SetColorThemeDarkMode(readThemeStyle == ThemeStyle::kDark);

    //主题变化后，重绘界面
    InvalidateAll();
    return true;
}

void Window::CloseColorTheme()
{
    m_pColorManager.reset();
    //主题变化后，重绘界面
    InvalidateAll();
}

bool Window::IsColorThemeDarkMode() const
{
    if (m_pColorManager != nullptr) {
        return m_pColorManager->IsColorThemeDarkMode();
    }
    else {
        return GlobalManager::Instance().Theme().GetCurrentThemeStyle() == ThemeStyle::kDark;
    }
}

const DString& Window::GetDefaultDisabledTextColor()
{
    ColorManager& colorManager = (m_pColorManager != nullptr) ? *m_pColorManager : GlobalManager::Instance().Color();
    return colorManager.GetDefaultDisabledTextColor();
}

const DString& Window::GetDefaultTextColor()
{
    ColorManager& colorManager = (m_pColorManager != nullptr) ? *m_pColorManager : GlobalManager::Instance().Color();
    return colorManager.GetDefaultTextColor();
}

bool Window::AddOptionGroup(const DString& strGroupName, Control* pControl)
{
    ASSERT(!strGroupName.empty());
    ASSERT(pControl != nullptr);
    if ((pControl == nullptr) || strGroupName.empty()) {
        return false;
    }
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
        if (it2 != it->second.end()) {
            return false;
        }
        it->second.push_back(pControl);
    }
    else {
        m_mOptionGroup[strGroupName].push_back(pControl);
    }
    return true;
}

std::vector<Control*>* Window::GetOptionGroup(const DString& strGroupName)
{
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        return &(it->second);
    }
    return nullptr;
}

void Window::RemoveOptionGroup(const DString& strGroupName, Control* pControl)
{
    ASSERT(!strGroupName.empty());
    ASSERT(pControl != nullptr);
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
        if (it2 != it->second.end()) {
            it->second.erase(it2);
        }

        if (it->second.empty()) {
            m_mOptionGroup.erase(it);
        }
    }
}

void Window::RemoveAllOptionGroups()
{
    m_mOptionGroup.clear();
}

bool Window::IsKeyDown(const EventArgs& msg, ModifierKey modifierKey) const
{
    switch (msg.eventType) {
    case kEventChar:
        if (modifierKey == ModifierKey::kFirstPress) {
            return msg.modifierKey & ModifierKey::kFirstPress;
        }
        else if (modifierKey == ModifierKey::kAlt) {
            return msg.modifierKey & ModifierKey::kAlt;
        }
        else if (modifierKey == ModifierKey::kIsSystemKey) {
            return msg.modifierKey & ModifierKey::kIsSystemKey;
        }
        break;

    case kEventKeyDown:
        if (modifierKey == ModifierKey::kFirstPress) {
            return msg.modifierKey & ModifierKey::kFirstPress;
        }
        else if (modifierKey == ModifierKey::kAlt) {
            return msg.modifierKey & ModifierKey::kAlt;
        }
        else if (modifierKey == ModifierKey::kIsSystemKey) {
            return msg.modifierKey & ModifierKey::kIsSystemKey;
        }
        break;

    case kEventKeyUp:
        if (modifierKey == ModifierKey::kAlt) {
            return msg.modifierKey & ModifierKey::kAlt;
        }
        else if (modifierKey == ModifierKey::kIsSystemKey) {
            return msg.modifierKey & ModifierKey::kIsSystemKey;
        }
        break;

    case kEventMouseWheel:
    {
        if (modifierKey == ModifierKey::kControl) {
            return msg.modifierKey & ModifierKey::kControl;
        }
        else if (modifierKey == ModifierKey::kShift) {
            return msg.modifierKey & ModifierKey::kShift;
        }
        break;
    }
    break;
    case kEventMouseHover:
    case kEventMouseMove:
    case kEventMouseButtonDown:
    case kEventMouseButtonUp:
    case kEventMouseDoubleClick:
    case kEventMouseRButtonDown:
    case kEventMouseRButtonUp:
    case kEventMouseRDoubleClick:
    case kEventMouseMButtonDown:
    case kEventMouseMButtonUp:
    case kEventMouseMDoubleClick:
        if (modifierKey == ModifierKey::kControl) {
            return msg.modifierKey & ModifierKey::kControl;
        }
        else if (modifierKey == ModifierKey::kShift) {
            return msg.modifierKey & ModifierKey::kShift;
        }
        break;
    default:
        break;
    }
    //默认从键盘状态获取
    if (modifierKey == ModifierKey::kControl) {
        return Keyboard::IsKeyDown(kVK_CONTROL);
    }
    else if (modifierKey == ModifierKey::kShift) {
        return Keyboard::IsKeyDown(kVK_SHIFT);
    }
    else if (modifierKey == ModifierKey::kAlt) {
        return Keyboard::IsKeyDown(kVK_MENU);
    }
    else if (modifierKey == ModifierKey::kWin) {
        return Keyboard::IsKeyDown(kVK_LWIN) || Keyboard::IsKeyDown(kVK_RWIN);
    }
    return false;
}

void Window::ClearImageCache()
{
    m_windowRoot->ClearImageCache();
}

void Window::OnUseSystemCaptionBarChanged()
{
    if (IsUseSystemCaption()) {
        //当开启系统标题栏时，需要关闭阴影
        SetShadowAttached(false);
    }
    else {
        //当关闭系统标题栏时，需要开启阴影
        SetShadowAttached(true);
    }
}

void Window::OnLayeredWindowChanged()
{
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    InvalidateAll();
}

void Window::InvalidateAll()
{
    if (IsWindow()) {
        UiRect rcClient;
        GetClientRect(rcClient);
        if (!rcClient.IsEmpty()) {
            Invalidate(rcClient);
        }
    }
}

void Window::OnWindowAlphaChanged()
{
    InvalidateAll();
}

void Window::OnWindowEnterFullscreen()
{
}

void Window::OnWindowExitFullscreen()
{
}

void Window::OnWindowDisplayScaleChanged(uint32_t /*nOldScaleFactor*/, uint32_t /*nNewScaleFactor*/)
{
}

void Window::GetShadowCorner(UiPadding& rcShadow) const
{
    rcShadow = m_windowRoot->GetShadowCorner();
}

void Window::GetCurrentShadowCorner(UiPadding& rcShadow) const
{
    rcShadow = m_windowRoot->GetCurrentShadowCorner();
}

bool Window::IsPtInCaptionBarControl(const UiPoint& pt) const
{
    Control* pControl = FindControl(pt);
    if (pControl) {
        if (pControl->CanPlaceCaptionBar()) {
            return true;
        }
    }
    return false;
}

bool Window::HasMinMaxBox(bool& /*bMinimizeBox*/, bool& /*bMaximizeBox*/) const
{
    return false;
}

bool Window::IsPtInMaximizeRestoreButton(const UiPoint& /*pt*/) const
{
    return false;
}

bool Window::NeedSetWindowRgn()
{
    if (!WindowBase::NeedSetWindowRgn()) {
        return false;
    }
    if (IsShadowAttached()) {
        ShadowType shadowType = GetShadowType();
        if (Shadow::IsShadowTypeNeedWindowRGN(shadowType)) {
            return true;//需要设置RGN
        }
    }
    return false;
}

UiSize Window::GetWindowRgnRoundCorner() const
{
    return WindowBase::GetWindowRgnRoundCorner();
}

void Window::UpdateLayeredWindowStyleEx(bool bRedraw)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bNeedLayeredWindow = false;
    if (IsShadowAttached()) {
        ShadowType shadowType = GetShadowType();
        if (shadowType == ShadowType::kShadowDefault) {
            shadowType = Shadow::GetDefaultShadowType(this);
        }
        bNeedLayeredWindow = Shadow::IsShadowTypeNeedLayeredWindow(shadowType);
    }
    if (IsLayeredWindow() != bNeedLayeredWindow) {
        OnRequestSetLayeredWindow(bNeedLayeredWindow, bRedraw);
    }
#else
    (void)bRedraw;
#endif
}

Box* Window::AttachShadow(Box* pRoot)
{
    return m_windowRoot->AttachShadow(pRoot);
}

void Window::SetShadowAttached(bool bShadowAttached)
{
    ASSERT(!m_windowRoot->IsControlFullscreen());
    if (m_windowRoot->IsControlFullscreen()) {
        return;
    }
    m_bWindowShadowInited = true;
    m_windowRoot->SetShadowAttached(bShadowAttached);
    UpdateWindowRGN(true); //处理RGN
    UpdateLayeredWindowStyleEx(true); //更新窗口的分层窗口属性
    OnWindowShadowTypeChanged();
}

void Window::SetShadowType(ShadowType nShadowType)
{
    ASSERT(!m_windowRoot->IsControlFullscreen());
    if (m_windowRoot->IsControlFullscreen()) {
        return;
    }
    m_bWindowShadowInited = true;
    m_windowRoot->SetShadowType(nShadowType);
    UpdateWindowRGN(true); //处理RGN
    UpdateLayeredWindowStyleEx(true); //更新窗口的分层窗口属性
    OnWindowShadowTypeChanged();
}

ShadowType Window::GetShadowType() const
{
    return m_windowRoot->GetShadowType();
}

DString Window::GetShadowImage() const
{
    return m_windowRoot->GetShadowImage();
}

void Window::SetShadowImage(const DString& shadowImage)
{
    m_windowRoot->SetShadowImage(shadowImage);
}

void Window::SetShadowBorderSize(int32_t nShadowBorderSize)
{
    m_windowRoot->SetShadowBorderSize(nShadowBorderSize);
}

int32_t Window::GetShadowBorderSize() const
{
    return m_windowRoot->GetShadowBorderSize();
}

void Window::SetShadowBorderColor(const DString& shadowBorderColor)
{
    m_windowRoot->SetShadowBorderColor(shadowBorderColor);
}

DString Window::GetShadowBorderColor() const
{
    return m_windowRoot->GetShadowBorderColor();
}

UiPadding Window::GetCurrentShadowCorner() const
{
    return m_windowRoot->GetCurrentShadowCorner();
}

bool Window::IsShadowAttached() const
{
    return m_windowRoot->IsShadowAttached();
}

UiPadding Window::GetShadowCorner() const
{
    return m_windowRoot->GetShadowCorner();
}

void Window::SetShadowCorner(const UiPadding& rcShadowCorner)
{
    m_windowRoot->SetShadowCorner(rcShadowCorner);
}

void Window::SetShadowBorderRound(UiSize szBorderRound)
{
    m_windowRoot->SetShadowBorderRound(szBorderRound);
}

void Window::SetEnableShadowSnap(bool bEnable)
{
    m_windowRoot->SetEnableShadowSnap(bEnable);
}

bool Window::IsEnableShadowSnap() const
{
    return m_windowRoot->IsEnableShadowSnap();
}

UiSize Window::GetShadowBorderRound() const
{
    return m_windowRoot->GetShadowBorderRound();
}

void Window::SetInitSize(int cx, int cy)
{
    ASSERT(IsWindow());
    if (GetRoot() == nullptr) {
        m_szInitSize.cx = cx;
        m_szInitSize.cy = cy;
    }
    else {
        Resize(cx, cy, true, false);
    }
}

void Window::OnDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    if ((nOldScaleFactor == nNewScaleFactor) || (nNewScaleFactor == 0)) {
        return;
    }
    if (!Dpi().CheckDisplayScaleFactor(nNewScaleFactor)) {
        return;
    }
    WindowBase::OnDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);

    //窗口阴影
    m_windowRoot->ChangeDpiScale(Dpi(), nOldScaleFactor, nNewScaleFactor);

    //更新窗口自身的DPI关联属性
    m_renderOffset = Dpi().GetScalePoint(m_renderOffset, nOldScaleFactor);

    //更新布局和控件的DPI关联属性
    SetArrange(true);

    Box* pRoot = GetRoot();
    if (pRoot != nullptr) {
        pRoot->ChangeDpiScale(nOldScaleFactor, nNewScaleFactor);
        pRoot->Arrange();
        Invalidate(pRoot->GetPos());
    }
}

LRESULT Window::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT Window::OnWindowPosChangedMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventWindowPosChanged);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnSizeMsg(WindowSizeType sizeType, const UiSize& /*newWindowSize*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();

    //调整Render的大小, 与客户区大小保持一致
    ResizeRenderToClientSize();

    Box* pRoot = GetRoot();
    if (pRoot != nullptr) {
        pRoot->Arrange();
    }
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        //最大化
        m_windowRoot->ProcessWindowMaximized();
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        //还原
        m_windowRoot->ProcessWindowRestored();
    }
    else {
        //大小变化
        m_windowRoot->ProcessWindowResized();
    }
    if (windowFlag.expired()) {
        return 0;
    }
    if (m_pFocus != nullptr) {        
        EventArgs msgData;
        msgData.eventData = (int32_t)sizeType;
        m_pFocus->SendEvent(kEventWindowSize, msgData);
    }
    return 0;
}

LRESULT Window::OnMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (m_pFocus != nullptr) {        
        EventArgs msgData;
        msgData.ptMouse = ptTopLeft;
        m_pFocus->SendEvent(kEventWindowMove, msgData);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    //如果窗口溢出屏幕，需要绘制整个窗口，避免溢出窗口外部的内容移入屏幕时无法正常绘制
    UiRect rcWindow;
    GetWindowRect(rcWindow);
    UiRect rcMonitor;
    GetMonitorWorkRect(rcMonitor);
    if ((rcWindow.left < rcMonitor.left) ||
        (rcWindow.top < rcMonitor.top) ||
        (rcWindow.right > rcMonitor.right) ||
        (rcWindow.bottom > rcMonitor.bottom)) {
        InvalidateAll();
    }
    return 0;
}

LRESULT Window::OnShowWindowMsg(bool bShow, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    OnShowWindow(bShow);
    return 0;
}

bool Window::ResizeRenderToClientSize() const
{
    bool bRet = false;
    UiRect rcClient;
    GetClientRect(rcClient);
    ASSERT(m_render != nullptr);
    if ((m_render != nullptr) && !rcClient.IsEmpty()) {
        if ((m_render->GetWidth() != rcClient.Width()) || (m_render->GetHeight() != rcClient.Height())) {
            bRet = m_render->Resize(rcClient.Width(), rcClient.Height());
            ASSERT(bRet && "Window::ResizeRenderToClientSize failed!");
        }
        else {
            bRet = true;
        }
    } 
    return bRet;
}

bool Window::OnPreparePaint()
{
    GlobalManager::Instance().AssertUIThread();
    if (!IsWindow()) {
        return false;
    }
    if (m_render == nullptr) {
        return false;
    }
    if (IsWindowMinimized() || (GetRoot() == nullptr)) {
        return false;
    }
    //更新状态，并创建Render等
    if (!PreparePaint(true)) {
        return false;
    }

    //完成界面布局的初始化的回调函数(在第一次绘制前调用)
    if (!m_bInitLayout) {
        m_bInitLayout = true;
        OnInitLayout();

        //进一步检查布局是否需要更新
        PreparePaint(true);
    }
    return true;
}

LRESULT Window::OnPaintMsg(const UiRect& rcPaint, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("PaintWindow 1, Window::OnPaintMsg");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    bHandled = false;
    if (!IsWindowFirstShown()) {
        //首次绘制的时候，需要完整绘制（避免初始窗口部分在屏幕外时，然后拖动窗口到屏幕中间时，界面显示不完整的问题）
        UiRect rc;
        GetClientRect(rc);
        bHandled = Paint(rc);
    }
    else {
        //非首次绘制时，只绘制脏区域
        bHandled = Paint(rcPaint);
    }

    //程序启动时间统计，统计到首次绘制完成
    static bool bStartupEnd = false;
    if (!bStartupEnd) {
        bStartupEnd = true;
        PerformanceUtilHelper::Instance().EndStat(_T("Application Startup")); //程序启动时间统计
    }
    return 0;
}
bool Window::Paint(const UiRect& rcPaint)
{
    GlobalManager::Instance().AssertUIThread();
    IRender* pRender = GetRender();
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_RICH_EDIT_DRAW_OPT)
    bool bNeedClearAlpha = true;  //Windows，使用系统RichEdit自身的绘制时，必须执行背景清零，否则文字显示会出现异常
#else
    bool bNeedClearAlpha = false; //默认不需要清零，窗口阴影自己负责清零
#endif
    if (!bNeedClearAlpha) {
        //动态检测是否需要做背景清零（按根容器的背景色是否设置了Alpha值，按跟容器是否设置了Alpha值）
        Box* pXmlRoot = GetXmlRoot();
        if (pXmlRoot->IsAlpha()) {
            bNeedClearAlpha = true;
        }
        else {
            UiColor bkColorValue;
            DString bkColor = pXmlRoot->GetBkColor();
            if (!bkColor.empty()) {
                bkColorValue = pXmlRoot->GetUiColor(bkColor);
            }
            if (bkColorValue.GetAlpha() != 255) {
                bNeedClearAlpha = true;
            }
        }
    }

    if (!rcPaint.IsEmpty() && bNeedClearAlpha) {
#if DUILIB_PERFORMANCE_STAT_ENABLED
        //性能统计
        static size_t statNameHash = 0;
        if (statNameHash == 0) {
            DString statName = _T("PaintWindow 2, Window::Paint ClearAlpha(Full)");
            statNameHash = std::hash<DString>{}(statName);
            PerformanceUtilHelper::Instance().AddStat(statName);
        }
        PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

        //背景清零
        pRender->ClearAlpha(rcPaint);
    }
    else if (IsShadowAttached()) {
        //仅对阴影部分清零，其他区域不清零
        const UiPadding rcShadowCorner = GetCurrentShadowCorner();        
        if (!rcShadowCorner.IsEmpty()) {
#if DUILIB_PERFORMANCE_STAT_ENABLED
            //性能统计
            static size_t statNameHash = 0;
            if (statNameHash == 0) {
                DString statName = _T("PaintWindow 2, Window::Paint ClearAlpha(Shadow)");
                statNameHash = std::hash<DString>{}(statName);
                PerformanceUtilHelper::Instance().AddStat(statName);
            }
            PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

            const UiSize szShadowBorderRound = GetShadowBorderRound();
            UiRect rcClient;
            GetClientRect(rcClient);
            if (!rcClient.IsEmpty()) {
                UiRect rcShadow;

                //上方阴影区域
                rcShadow = rcClient;
                rcShadow.bottom = rcClient.top + rcShadowCorner.top + szShadowBorderRound.cy;
                UiRect rcClearRect;
                if (UiRect::Intersect(rcClearRect, rcShadow, rcPaint)) {
                    pRender->ClearAlpha(rcClearRect);
                }
                //下方阴影区域
                rcShadow = rcClient;
                rcShadow.top = rcClient.bottom - rcShadowCorner.bottom - szShadowBorderRound.cy;
                if (UiRect::Intersect(rcClearRect, rcShadow, rcPaint)) {
                    pRender->ClearAlpha(rcClearRect);
                }
                //左侧阴影区域
                rcShadow = rcClient;
                rcShadow.right = rcClient.left + rcShadowCorner.left + szShadowBorderRound.cx;
                if (UiRect::Intersect(rcClearRect, rcShadow, rcPaint)) {
                    pRender->ClearAlpha(rcClearRect);
                }
                //右侧阴影区域
                rcShadow = rcClient;
                rcShadow.left = rcShadow.right - rcShadowCorner.right - szShadowBorderRound.cx;
                if (UiRect::Intersect(rcClearRect, rcShadow, rcPaint)) {
                    pRender->ClearAlpha(rcClearRect);
                }
            }
        }
    }

    // 绘制
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return false;
    }
    if (pRoot->IsVisible()) {
#if DUILIB_PERFORMANCE_STAT_ENABLED
        //性能统计
        static size_t statNameHash = 0;
        if (statNameHash == 0) {
            DString statName = _T("PaintWindow 3, Window::Paint AlphaPaint(Root Box)");
            statNameHash = std::hash<DString>{}(statName);
            PerformanceUtilHelper::Instance().AddStat(statName);
        }
        PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

        AutoClip rectClip(pRender, rcPaint, true);
        UiPoint ptOldWindOrg = pRender->OffsetWindowOrg(m_renderOffset);
        pRoot->AlphaPaint(pRender, rcPaint);
        pRender->SetWindowOrg(ptOldWindOrg);
    }
    else {
        UiColor bkColor = UiColor(UiColors::LightGray);
        if (!pRoot->GetBkColor().empty()) {
            bkColor = pRoot->GetUiColor(pRoot->GetBkColor());
        }
        pRender->FillRect(UiRectF::MakeFromRect(rcPaint), bkColor);
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_RICH_EDIT_DRAW_OPT)
    //开始绘制前，进行alpha通道修复
    if (!rcPaint.IsEmpty()) {
        PerformanceUtil statPerformance(_T("PaintWindow 4, Window::Paint RestoreAlpha"));
        Shadow* pShadow = m_windowRoot->GetShadow();
        if ((pShadow != nullptr) && IsShadowAttached() && !Shadow::IsSystemShadowType(GetShadowType()) &&
            (m_renderOffset.x == 0) && (m_renderOffset.y == 0)) {
            //补救由于Gdi绘制造成的alpha通道为0
            UiRect rcNewPaint = rcPaint;
            rcNewPaint.Intersect(pRoot->GetPosWithoutPadding());
            UiPadding rcRootPadding = pRoot->GetPadding();

            //考虑圆角
            rcRootPadding.left += 1;
            rcRootPadding.top += 1;
            rcRootPadding.right += 1;
            rcRootPadding.bottom += 1;
            pRender->RestoreAlpha(rcNewPaint, rcRootPadding);//目前只有Windows的RichEdit绘制导致窗体透明，所以才需要恢复
        }
        else {
            UiRect rcNewPaint = rcPaint;
            UiRect rcRootPaddingPos = pRoot->GetPosWithoutPadding();
            rcNewPaint.Intersect(rcRootPaddingPos);
            UiPadding rcRootPadding;
            pRender->RestoreAlpha(rcNewPaint, rcRootPadding);//目前只有Windows的RichEdit绘制导致窗体透明，所以才需要恢复
        }
    }
#endif
    return true;
}

LRESULT Window::OnSetFocusMsg(WindowBase* /*pLostFocusWindow*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    //获得焦点时，如果无焦点控件，则关闭输入法
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    ControlPtr pFocus = m_pFocus;
    if ((pFocus != nullptr) && pFocus->IsEnabled()){
        pFocus->SendEvent(kEventWindowSetFocus);

        //重新激活控件焦点（但不恢复Hovered状态，避免按钮等控件的显示状态异常）
        if (!windowFlag.expired() && (pFocus == m_pFocus)) {
            pFocus->SendEvent(kEventSetFocus);
        }
        if (!windowFlag.expired() && (pFocus == m_pFocus)) {
            UiPoint pt;
            GetCursorPos(pt);
            ScreenToClient(pt);
            if (pFocus->IsPointInWithScrollOffset(pt)) {
                //鼠标还在控件范围内，保持hot状态
                pFocus->SetState(kControlStateHovered);
            }
            else {
                //鼠标不再控件范围内，恢复为Normal状态
                pFocus->SetState(kControlStateNormal);
            }
        }
    }
    else {
        NativeWnd()->SetImeOpenStatus(false);
    }
    return 0;
}

LRESULT Window::OnKillFocusMsg(WindowBase* /*pSetFocusWindow*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    ControlPtr pEventClick = m_pEventClick;
    if (!Keyboard::IsKeyDown(VirtualKeyCode::kVK_LBUTTON) &&
        !Keyboard::IsKeyDown(VirtualKeyCode::kVK_RBUTTON) &&
        !Keyboard::IsKeyDown(VirtualKeyCode::kVK_MBUTTON)) {
        //只有鼠标按键未按下时清空，否则应该在鼠标弹起时清楚，避免影响非焦点状态时的点击功能
        m_pEventClick = nullptr;
    }

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (pEventClick != nullptr) {
        pEventClick->SendEvent(kEventWindowKillFocus);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    ControlPtr pFocus = m_pFocus;
    if ((pFocus != nullptr) && (pFocus != pEventClick)) {
        pFocus->SendEvent(kEventWindowKillFocus);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnImeSetContextMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeSetContext, nativeMsg.wParam, nativeMsg.lParam);
        if ((m_pFocus != nullptr) && m_pFocus->IsCefOsrImeMode()) {
            bHandled = true;
        }
    }
    return 0;
}

LRESULT Window::OnImeStartCompositionMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeStartComposition);
        if ((m_pFocus != nullptr) && m_pFocus->IsCefOsrImeMode()) {
            bHandled = true;
        }
    }
    return 0;
}

LRESULT Window::OnImeCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeComposition, nativeMsg.wParam, nativeMsg.lParam);
        if ((m_pFocus != nullptr) && m_pFocus->IsCefOsrImeMode()) {
            bHandled = true;
        }
    }
    return 0;
}

LRESULT Window::OnImeEndCompositionMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeEndComposition);
        if ((m_pFocus != nullptr) && m_pFocus->IsCefOsrImeMode()) {
            bHandled = true;
        }
    }
    return 0;
}

LRESULT Window::OnSetCursorMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    if (m_pEventClick != nullptr) {
        bHandled = true;
        return 0;
    }

    UiPoint pt;
    GetCursorPos(pt);
    ScreenToClient(pt);
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        //返回值待确认：如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续。
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.ptMouse = pt;
        pControl->SendEvent(kEventSetCursor, msgData);
        bHandled = true;
        if (windowFlag.expired()) {
            return 0;
        }
        else if (pControl->IsCefOSR()) {
            //离屏渲染模式下，需要让系统处理光标消息，否则光标会出现异常现象
            bHandled = false;
        }
    }
    return 0;
}

LRESULT Window::OnContextMenuMsg(const UiPoint& pt, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    ReleaseCapture();

    if ((pt.x != -1) && (pt.y != -1)) {
        SetLastMousePos(pt);
        Control* pControl = FindContextMenuControl(&pt);
        if (pControl != nullptr) {
            Control* ptControl = FindControl(pt);//当前点击点所在的控件
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.ptMouse = pt;
            msgData.lParam = (LPARAM)ptControl;
            pControl->SendEvent(kEventContextMenu, msgData);
            if (windowFlag.expired()) {
                return 0;
            }
        }
    }
    else {
        //如果用户键入 SHIFT+F10，则上下文菜单为 -1, -1，
        //应用程序应在当前所选内容的位置（而不是 (xPos、yPos) ）显示上下文菜单。
        Control* pControl = FindContextMenuControl(nullptr);
        if (pControl != nullptr) {
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.ptMouse = pt;
            msgData.lParam = 0;
            pControl->SendEvent(kEventContextMenu, msgData);
            if (windowFlag.expired()) {
                return 0;
            }
        }
    }
    return 0;
}

LRESULT Window::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kAlt) {
        if (vkCode != kVK_MENU) {
            ASSERT(Keyboard::IsKeyDown(kVK_MENU));
        }        
    }
#endif
    bHandled = false;
    LRESULT lResult = 0;
    if (modifierKey & ModifierKey::kAlt) {
        //含有组合按键: Alt
        m_pEventKey = m_pFocus;
        if (m_pEventKey != nullptr) {
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.vkCode = vkCode;
            msgData.modifierKey = modifierKey;
            msgData.wParam = nativeMsg.wParam;
            msgData.lParam = nativeMsg.lParam;
            m_pEventKey->SendEvent(kEventKeyDown, msgData);
            if (windowFlag.expired()) {
                return lResult;
            }
        }
        return lResult;
    }

    if ((vkCode == kVK_ESCAPE) && IsWindowFullscreen()) {
        //按ESC键时，退出全屏
        if (GetFullscreenControl() != nullptr) {
            ExitControlFullscreen();
        }
        else {
            ExitFullscreen();
        }
        return lResult;
    }
    if (m_pFocus != nullptr) {
        bool bMsgHandled = false;
        if (vkCode == kVK_TAB) {
            if (m_pFocus->IsVisible() &&
                m_pFocus->IsEnabled() &&
                m_pFocus->IsWantTab()) {
                bMsgHandled = false;
            }
            else {
                //通过TAB键切换焦点控件
                SetNextTabControl(!Keyboard::IsKeyDown(kVK_SHIFT));
                bMsgHandled = true;
            }
        }
        if(!bMsgHandled) {
            m_pEventKey = m_pFocus;
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.vkCode = vkCode;
            msgData.modifierKey = modifierKey;
            msgData.wParam = nativeMsg.wParam;
            msgData.lParam = nativeMsg.lParam;
            m_pEventKey->SendEvent(kEventKeyDown, msgData);
            if (windowFlag.expired()) {
                return lResult;
            }
        }
    }
    else {
        m_pEventKey = nullptr;
    }
    if ((vkCode == kVK_ESCAPE) && (m_pEventClick != nullptr)) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pEventClick->SendEvent(kEventMouseClickEsc);
        if (windowFlag.expired()) {
            return lResult;
        }
    }
    return lResult;
}

LRESULT Window::OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kAlt) {
        if (vkCode != kVK_MENU) {
            ASSERT(Keyboard::IsKeyDown(kVK_MENU));
        }
    }
#endif

    bHandled = false;
    LRESULT lResult = 0;
    if (m_pEventKey != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.vkCode = vkCode;
        msgData.modifierKey = modifierKey;
        msgData.wParam = nativeMsg.wParam;
        msgData.lParam = nativeMsg.lParam;
        m_pEventKey->SendEvent(kEventKeyUp, msgData);
        if (windowFlag.expired()) {
            return lResult;
        }
        m_pEventKey = nullptr;
    }
    return lResult;
}

LRESULT Window::OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kAlt) {
        ASSERT(Keyboard::IsKeyDown(kVK_MENU));
    }
#endif

    bHandled = false;
    LRESULT lResult = 0;
    ControlPtr pFocusControl = (m_pEventKey != nullptr) ? m_pEventKey : m_pFocus;
    if (pFocusControl != nullptr) {
        EventArgs msgData;
        msgData.vkCode = vkCode;
        msgData.modifierKey = modifierKey;
        msgData.wParam = nativeMsg.wParam;
        msgData.lParam = nativeMsg.lParam;
        msgData.eventData = nativeMsg.uMsg;
        pFocusControl->SendEvent(kEventChar, msgData);
    }
    return lResult;
}

LRESULT Window::OnHotKeyMsg(int32_t /*hotkeyId*/, VirtualKeyCode /*vkCode*/, uint32_t modifierKey, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kAlt) {
        ASSERT(Keyboard::IsKeyDown(kVK_MENU));
    }
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
    if (modifierKey & ModifierKey::kWin) {
        ASSERT(Keyboard::IsKeyDown(kVK_LWIN) || Keyboard::IsKeyDown(kVK_RWIN));
    }
#endif
    (void)modifierKey;
    bHandled = false;
    return 0;
}

LRESULT Window::OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    LRESULT lResult = 0;
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        EventArgs msgData;
        msgData.eventData = wheelDelta;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        msgData.wParam = nativeMsg.wParam;
        msgData.lParam = nativeMsg.lParam;
        pControl->SendEvent(kEventMouseWheel, msgData);
    }
    return lResult;
}

LRESULT Window::OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    //是否需要处理ToolTip(如果是NC消息，不处理，因为处理后会导致触发WM_MOUSEMOVE消息，影响流程)
    bool bProcessToolTip = !bFromNC;

    bHandled = false;
    LRESULT lResult = 0;
    if (bProcessToolTip) {
        m_toolTip->SetMouseTracking(this, true);
    }
    SetLastMousePos(pt);

    // Do not move the focus to the new control when the mouse is pressed
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (!IsCaptured()) {
        if (!HandleMouseEnterLeave(pt, modifierKey, bProcessToolTip)) {
            return lResult;
        }
    }
    if (windowFlag.expired()) {
        return lResult;
    }

    //全屏按钮的动态显示
    m_windowRoot->ProcessFullscreenButtonMouseMove(pt);

    EventArgs msgData;
    msgData.modifierKey = modifierKey;
    msgData.ptMouse = pt;
    msgData.wParam = nativeMsg.wParam;
    msgData.lParam = nativeMsg.lParam;
    if (m_pEventClick != nullptr) {        
        m_pEventClick->SendEvent(kEventMouseMove, msgData);
    }
    else if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseMove, msgData);
    }    
    return lResult;
}


bool Window::HandleMouseEnterLeave(const UiPoint& pt, uint32_t modifierKey, bool bHideToolTip)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    ControlPtr pNewHover = ControlPtr(FindControl(pt));
    //设置为新的Hover控件（必须先设置m_pEventHover的值，否则在Control::HandleEvent函数中，对鼠标消息Enter/Leave消息处理逻辑有冲突）
    ControlPtr pOldHover = m_pEventHover;
    m_pEventHover = pNewHover;

    ControlPtr pNewToolTip = ControlPtr(FindToolTipControl(pt));
    //设置为新的ToolTip控件
    ControlPtr pOldToolTip = m_pEventToolTip;
    m_pEventToolTip = pNewToolTip;
    if ((pNewToolTip != pOldToolTip) && (pOldToolTip != nullptr)) {
        if (bHideToolTip) {
            m_toolTip->HideToolTip();
        }
    }

    if ((pNewHover != pOldHover) && (pOldHover != nullptr)) {
        //Hover状态的控件发生变化，原来Hover控件的Tooltip应消失
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pOldHover->SendEvent(kEventMouseLeave, msgData);
        if (windowFlag.expired()) {
            return false;
        }
    }
    ASSERT(pNewHover == m_pEventHover);
    if (pNewHover != m_pEventHover) {
        return false;
    }

    if ((pNewHover != pOldHover) && (pNewHover != nullptr)) {
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pNewHover->SendEvent(kEventMouseEnter, msgData);
        if (windowFlag.expired()) {
            return false;
        }
    }
    return true;
}

LRESULT Window::OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    LRESULT lResult = 0;
    m_toolTip->SetMouseTracking(this, false);
    if (IsCaptured()) {
        //如果处于Capture状态，不显示ToolTip
        return lResult;
    }
    Control* pNewHover = FindControl(pt);
    if (pNewHover == nullptr) {
        return lResult;
    }
    std::weak_ptr<WeakFlag> hoveredFlag = pNewHover->GetWeakFlag();
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    EventArgs msgData;
    msgData.modifierKey = modifierKey;
    msgData.ptMouse = pt;
    msgData.wParam = nativeMsg.wParam;
    msgData.lParam = nativeMsg.lParam;
    pNewHover->SendEvent(kEventMouseHover, msgData);
    if (hoveredFlag.expired() || windowFlag.expired()) {
        return lResult;
    }

    //显示ToolTip的控件
    ControlPtr pOldToolTip = m_pEventToolTip;
    Control* pNewToolTip = FindToolTipControl(pt);
    if ((pNewToolTip != nullptr) && (pOldToolTip == pNewToolTip)) {
        //检查按需显示ToolTip信息    
        UiRect rect = pNewToolTip->GetPos();
        uint32_t maxWidth = pNewToolTip->GetToolTipWidth();
        DString toolTipText = pNewToolTip->GetToolTipText();
        m_toolTip->ShowToolTip(this, rect, maxWidth, pt, toolTipText);
    }
    return lResult;
}

LRESULT Window::OnMouseLeaveMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
    return 0;
}

LRESULT Window::OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseButtonDown, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonUp(kEventMouseButtonUp, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseDoubleClick, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseRButtonDown, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonUp(kEventMouseRButtonUp, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseRDoubleClick, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseMButtonDown, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonUp(kEventMouseMButtonUp, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
#ifdef _DEBUG
    if (modifierKey & ModifierKey::kControl) {
        ASSERT(Keyboard::IsKeyDown(kVK_CONTROL));
    }
    if (modifierKey & ModifierKey::kShift) {
        ASSERT(Keyboard::IsKeyDown(kVK_SHIFT));
    }
#endif

    bHandled = false;
    OnButtonDown(kEventMouseMDoubleClick, pt, nativeMsg, modifierKey);
    return 0;
}

LRESULT Window::OnCaptureChangedMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventCaptureChanged);
    }
    return 0;
}

LRESULT Window::OnWindowCloseMsg(uint32_t /*wParam*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

void Window::OnWindowCreateMsg(bool /*bDoModal*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
}

void Window::OnWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap)
{
    if (IsWindowMaximized() || IsWindowMinimized()) {
        //窗口最大化或者最小化时，不处理
        return;
    }
    UiRect rcSizeBox = GetSizeBox();

    //没有窗口边框，不处理
    if (rcSizeBox.left <= 0) {
        bLeftSnap = false;
    }
    if (rcSizeBox.top <= 0) {
        bTopSnap = false;
    }
    if (rcSizeBox.right <= 0) {
        bRightSnap = false;
    }
    if (rcSizeBox.bottom <= 0) {
        bBottomSnap = false;
    }
    m_windowRoot->SetWindowPosSnap(bLeftSnap, bRightSnap, bTopSnap, bBottomSnap);
}

void Window::OnButtonDown(EventType eventType, const UiPoint& pt, const NativeMsg& nativeMsg, uint32_t modifierKey)
{
    ASSERT(eventType == kEventMouseButtonDown ||
           eventType == kEventMouseRButtonDown ||
           eventType == kEventMouseMButtonDown ||
           eventType == kEventMouseDoubleClick ||
           eventType == kEventMouseRDoubleClick ||
           eventType == kEventMouseMDoubleClick);

    const bool bWindowFocused = IsWindowFocused();
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();    
    if ((eventType == kEventMouseButtonDown) || (eventType == kEventMouseMButtonDown) || (eventType == kEventMouseRButtonDown)) {
        SetCapture();
        if (windowFlag.expired()) {
            return;
        }
    }    
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        std::weak_ptr<WeakFlag> controlFlag = pControl->GetWeakFlag();
        std::weak_ptr<WeakFlag> clickFlag;
        if (m_pEventClick != nullptr) {
            clickFlag = m_pEventClick->GetWeakFlag();
        }
        ControlPtr pOldEventClick = m_pEventClick;
        m_pEventClick = pControl;
        bool bOldCheckSetWindowFocus = IsCheckSetWindowFocus();
        SetCheckSetWindowFocus(false);
        pControl->SetFocus();
        if (windowFlag.expired()) {
            return;
        }
        SetCheckSetWindowFocus(bOldCheckSetWindowFocus);
        if (controlFlag.expired()) {
            if (m_pEventClick.get() == pControl) {
                m_pEventClick = nullptr;
            }
            return;
        }
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        msgData.wParam = nativeMsg.wParam;
        msgData.lParam = nativeMsg.lParam;
        pControl->SendEvent(eventType, msgData);
        if (windowFlag.expired()) {
            return;
        }
        if ((pOldEventClick != nullptr) && (pOldEventClick != pControl) && !clickFlag.expired()) {
            pOldEventClick->SendEvent(kEventMouseClickChanged);
            if (windowFlag.expired()) {
                return;
            }
        }
    }
    else if (!IsUseSystemCaption()) {
        //检查是否点击在窗口阴影区域(实现鼠标点击阴影，穿透到后面窗口的功能)
        Shadow* pShadow = m_windowRoot->GetShadow();
        if ((pShadow != nullptr) && IsShadowAttached()) {
            pShadow->CheckMouseClickOnShadow(eventType, pt);
        }        
    }
    if (!bWindowFocused && !windowFlag.expired()) {
        //确保被点击的窗口有输入焦点(解决CEF窗口模式下，输入焦点无法从页面切换到地址栏的问题)
        CheckSetWindowFocus();
    }
}

void Window::OnButtonUp(EventType eventType, const UiPoint& pt, const NativeMsg& nativeMsg, uint32_t modifierKey)
{
    ASSERT(eventType == kEventMouseButtonUp || eventType == kEventMouseRButtonUp || eventType == kEventMouseMButtonUp);

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if ((eventType == kEventMouseButtonUp) || (eventType == kEventMouseRButtonUp) || (eventType == kEventMouseMButtonUp)) {
        ReleaseCapture();
    }
    if (windowFlag.expired()) {
        return;
    }
    SetLastMousePos(pt);    
    if (m_pEventClick != nullptr) {
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        msgData.wParam = nativeMsg.wParam;
        msgData.lParam = nativeMsg.lParam;
        m_pEventClick->SendEvent(eventType, msgData);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventClick = nullptr;
    }
}

void Window::ClearInputStatus()
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    m_pEventToolTip = nullptr;
    if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventHover = nullptr;
    }    
    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventClick = nullptr;
    }
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventKey = nullptr;
    }
    KillFocusControl();
}

Control* Window::GetFocusControl() const
{
    return m_pFocus.get();
}

Control* Window::GetEventClick() const
{
    return m_pEventClick.get();
}

void Window::SetFocusControl(Control* pControl)
{
    if (pControl == m_pFocus) {
        return;
    }

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    std::weak_ptr<WeakFlag> controlFlag;
    if (pControl != nullptr) {
        controlFlag = pControl->GetWeakFlag();
    }    
    ControlPtr pOldFocus = m_pFocus;
    if (pOldFocus != nullptr) {
        m_pFocus = nullptr;
        //WPARAM 是新的焦点控件接口        
        pOldFocus->SendEvent(kEventKillFocus, (WPARAM)pControl);
        if (windowFlag.expired()) {
            return;
        }        
        if ((pControl != nullptr) && controlFlag.expired()){
            //该控件已经销毁
            OnFocusControlChanged();
            return;
        }
    }
    if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsEnabled()) {
        ASSERT(pControl->GetWindow() == this);

        //设置控件为焦点控件
        m_pFocus = pControl;
        m_pFocus->SendEvent(kEventSetFocus);
        if (windowFlag.expired()) {
            return;
        }        
    }
    if (!windowFlag.expired() && (pOldFocus != m_pFocus)) {
        OnFocusControlChanged();
    }

    if (!windowFlag.expired() && (pControl != nullptr) && !controlFlag.expired()) {
        //确保窗口有焦点(但CEF 子窗口模式的控件，不与子窗口争焦点)
        if (IsCheckSetWindowFocus() && !pControl->IsCefNative()) {
            CheckSetWindowFocus();
            if (windowFlag.expired()) {
                return;
            }
        }
    }
}

void Window::SetCheckSetWindowFocus(bool bCheckSetWindowFocus)
{
    m_bCheckSetWindowFocus = bCheckSetWindowFocus;
}

bool Window::IsCheckSetWindowFocus() const
{
    return m_bCheckSetWindowFocus;
}

void Window::KillFocusControl()
{
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pFocus->SendEvent(kEventKillFocus);
        if (!windowFlag.expired()) {
            m_pFocus = nullptr;
            OnFocusControlChanged();
        }        
    }
}

void Window::OnFocusControlChanged()
{
    if (IsWindowFocused() && (m_pFocus == nullptr)) {
        //无焦点控件时，关闭输入法
        NativeWnd()->SetImeOpenStatus(false);
    }
}

Window* Window::WindowFromPoint(const UiPoint& pt, bool bIgnoreChildWindow)
{
    WindowBase* pWindow = WindowBaseFromPoint(pt, bIgnoreChildWindow);
    if (!GlobalManager::Instance().Windows().HasWindowBase(pWindow)) {
        //不是本进程窗口时，不使用，避免跨进程的窗口时导致崩溃
        pWindow = nullptr;
    }
    if (pWindow != nullptr) {
        return dynamic_cast<Window*>(pWindow);
    }
    return nullptr;
}

void Window::UpdateToolTip()
{
    //隐藏现有的，等待重新显示即会更新
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
}

Control* Window::GetHoverControl() const
{
    return m_pEventHover.get();
}

bool Window::SetNextTabControl(bool bForward)
{
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return false;
    }
    // Find next/previous tabbable control
    FINDTABINFO info1 = { 0 };
    info1.pFocus = m_pFocus.get();
    info1.bForward = bForward;
    Control* pControl = pRoot->FindControl(ControlFinder::FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    if (pControl == nullptr) {
        if (bForward) {
            // Wrap around
            FINDTABINFO info2 = { 0 };
            info2.pFocus = bForward ? nullptr : info1.pLast;
            info2.bForward = bForward;
            pControl = pRoot->FindControl(ControlFinder::FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
        }
        else {
            pControl = info1.pLast;
        }
    }
    if (pControl != nullptr) {
        SetFocusControl(pControl);
    }
    return true;
}

void Window::SetArrange(bool bArrange)
{
    m_bIsArranged = bArrange;
}

void Window::PostQuitMsgWhenClosed(bool bPostQuitMsg)
{
    m_bPostQuitMsgWhenClosed = bPostQuitMsg;
}

ui::IRender* Window::GetRender() const
{
    //这里加断言：业务流程调用到此处，render必须是已经创建完成的，否则逻辑有问题（比如估算控件大小，自动设置ToolTip宽度等功能均会有异常）
    ASSERT(m_render != nullptr);
    ResizeRenderToClientSize();
    return m_render.get();
}

class RenderWindowDpi: public IRenderDpi
{
public:
    explicit RenderWindowDpi(Window* pWindow): m_pWindow(pWindow)
    {
        m_windowFlag = pWindow->GetWeakFlag();
    }
    virtual ~RenderWindowDpi() override {}

    /** 根据界面缩放比来缩放整数
    * @param[in] iValue 整数
    * @return int 缩放后的值
    */
    virtual int32_t GetScaleInt(int32_t iValue) const override
    {
        const DpiManager& dpi = ((m_pWindow != nullptr) && !m_windowFlag.expired()) ? m_pWindow->Dpi() : GlobalManager::Instance().Dpi();
        return dpi.GetScaleInt(iValue);
    }

    /** 根据界面缩放比来缩放整数
    * @param [in] fValue 浮点数
    * @return 缩放后的值
    */
    virtual float GetScaleFloat(float fValue) const override
    {
        const DpiManager& dpi = ((m_pWindow != nullptr) && !m_windowFlag.expired()) ? m_pWindow->Dpi() : GlobalManager::Instance().Dpi();
        return dpi.GetScaleFloat(fValue);
    }

private:
    Window* m_pWindow;
    std::weak_ptr<WeakFlag> m_windowFlag;
};

std::shared_ptr<IRenderDpi> Window::GetRenderDpi()
{
    IRenderDpiPtr spRenderDpi = std::make_shared<RenderWindowDpi>(this);
    return spRenderDpi;
}

void Window::SetWindowAttributesApplied(bool bApplied)
{
    m_bWindowAttributesApplied = bApplied;
}

bool Window::IsWindowAttributesApplied() const
{
    return m_bWindowAttributesApplied;
}

void Window::OnShowWindow(bool bShow)
{
    if (bShow && !m_bFirstLayout && (GetRoot() != nullptr)) {
        //首次显示
        PreparePaint(false);
    }
}

bool Window::PreparePaint(bool bArrange)
{
    //在估算控件大小的时候，需要Render有宽高等数据，所以需要进行Resize初始化
    bool bRet = ResizeRenderToClientSize();
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return false;
    }

    bool bUpdated = false;
    if (m_bIsArranged && pRoot->IsArranged()) {
        //如果root配置的宽度和高度是auto类型的，自动调整窗口大小
        AutoResizeWindow(true);
        bUpdated = true;
    }

    //对控件进行布局
    if (bArrange) {
        ArrangeRoot();
        bUpdated = true;
    }

    if (bUpdated) {
        //期间可能会有修改窗口大小等操作，需要同步
        bRet = ResizeRenderToClientSize();
    }
    return bRet;
}

bool Window::AutoResizeWindow(bool bRepaint)
{
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return false;
    }
    bool bResized = false;
    if ((pRoot != nullptr) && (!pRoot->GetFixedWidth().IsStretch() || !pRoot->GetFixedHeight().IsStretch())) {
        //跟容器属性：如果宽度或者高度有不是拉伸类型的，根据跟容器的大小自动修改窗口大小
        UiSize maxSize(999999, 999999);
        const UiEstSize estSize = pRoot->EstimateSize(maxSize);
        if (!estSize.cx.IsStretch() || !estSize.cy.IsStretch()) {
            UiSize newSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
            newSize.cx = std::clamp(newSize.cx, pRoot->GetMinWidth(), pRoot->GetMaxWidth());
            newSize.cy = std::clamp(newSize.cy, pRoot->GetMinHeight(), pRoot->GetMaxHeight());

            if (Dpi().HasPixelDensity()) {
                //转换为窗口大小
                newSize.cx = (int32_t)std::round(newSize.cx / Dpi().GetPixelDensity());
                newSize.cy = (int32_t)std::round(newSize.cy / Dpi().GetPixelDensity());
            }

            UiRect rcWindow;
            GetWindowRect(rcWindow);
            if (estSize.cx.IsStretch()) {
                newSize.cx = rcWindow.Width();
            }
            if (estSize.cy.IsStretch()) {
                newSize.cy = rcWindow.Height();
            }
            //窗口的高度和宽度禁止设置为0（备注：SDL内部不支持）
            newSize.cx = std::max(newSize.cx, 1);
            newSize.cy = std::max(newSize.cy, 1);
            if ((rcWindow.Width() != newSize.cx) || (rcWindow.Height() != newSize.cy)) {
                Resize(newSize.cx, newSize.cy, true, false);
                bResized = true;
                if (bRepaint) {
                    InvalidateAll();
                }
            }
        }
    }
    return bResized;
}

void Window::ArrangeRoot()
{
    UiRect rcClient;
    GetClientRect(rcClient);
    if (rcClient.IsEmpty()) {
        return;
    }
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return;
    }

    //允许Root设置Margin
    UiRect rcRoot = rcClient;
    UiMargin rcRootMargin = pRoot->GetMargin();
    rcRoot.Deflate(rcRootMargin);

    if (m_bIsArranged) {
        m_bIsArranged = false;
        if (pRoot->IsArranged() || (pRoot->GetPos() != rcRoot)) {
            //所有控件的布局全部重排
            pRoot->SetPos(rcRoot);
        }
        else {
            //仅对有更新的控件的布局全部重排
            Control* pControl = pRoot->FindControl(ControlFinder::FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
            while (pControl != nullptr) {
                pControl->SetPos(pControl->GetPos());
                //ASSERT(!pControl->IsArranged());
                pControl = pRoot->FindControl(ControlFinder::FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
            }
        }
        if (!m_bFirstLayout) {
            m_bFirstLayout = true;
            OnFirstLayout();
        }
    }
    else if (pRoot->GetPos() != rcRoot) {
        //所有控件的布局全部重排
        pRoot->SetPos(rcRoot);
    }
}

void Window::SetRenderOffset(UiPoint renderOffset)
{
    m_renderOffset = renderOffset;
    InvalidateAll();
}

void Window::SetRenderOffsetX(int renderOffsetX)
{
    m_renderOffset.x = renderOffsetX;
    InvalidateAll();
}

void Window::SetRenderOffsetY(int renderOffsetY)
{
    m_renderOffset.y = renderOffsetY;
    InvalidateAll();
}

void Window::OnFirstLayout()
{
    Box* pRoot = GetRoot();
    if ((pRoot != nullptr) && pRoot->IsVisible()) {
        pRoot->SetFadeVisible(true);
    }
}

Control* Window::OnFindControl(const UiPoint& pt) const
{
    return FindControl(pt);
}

Control* Window::FindControl(const UiPoint& pt) const
{
    if (GetRoot() == nullptr) {
        return nullptr;
    }
    Control* pControl = m_controlFinder.FindControl(pt);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(0);
        pControl = nullptr;
    }
    return pControl;
}

Control* Window::FindToolTipControl(const UiPoint& pt) const
{
    if (GetRoot() == nullptr) {
        return nullptr;
    }
    Control* pControl = m_controlFinder.FindToolTipControl(pt);    
    if (pControl != nullptr) {
        Window* pWindow = pControl->GetWindow();
        if (pWindow != this) {
            //ASSERT(0); 对于菜单，在弹出子菜单时，会遇到此情况
            pControl = nullptr;
        }
    }
    return pControl;
}

Control* Window::FindContextMenuControl(const UiPoint* pt) const
{
    Control* pControl = m_controlFinder.FindContextMenuControl(pt);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(0);
        pControl = nullptr;
    }
    return pControl;
}

Box* Window::FindDroppableBox(const UiPoint& pt, uint8_t nDropInId) const
{
    Box* pControl = m_controlFinder.FindDroppableBox(pt, nDropInId);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(0);
        pControl = nullptr;
    }
    return pControl;
}

Control* Window::FindControl(const DString& strName) const
{
    return m_controlFinder.FindSubControlByName(GetRoot(), strName);
}

Control* Window::FindSubControlByPoint(Control* pParent, const UiPoint& pt) const
{
    return m_controlFinder.FindSubControlByPoint(pParent, pt);
}

Control* Window::FindSubControlByName(Control* pParent, const DString& strName) const
{
    return m_controlFinder.FindSubControlByName(pParent, strName);
}

Shadow* Window::GetShadow() const
{
    return m_windowRoot->GetShadow();
}

void Window::NotifyWindowEnterFullscreen()
{
    //全屏时，需要还原最大化时设置的外边距
    m_windowRoot->ProcessWindowEnterFullscreen();
}

void Window::NotifyWindowExitFullscreen()
{
    //窗口退出全屏状态
    m_windowRoot->ProcessWindowExitFullscreen();
}

bool Window::SetFullscreenControl(Control* pFullscreenControl, const DString& exitButtonClass)
{
    return m_windowRoot->SetFullscreenControl(pFullscreenControl, exitButtonClass);
}

void Window::ExitControlFullscreen()
{
    m_windowRoot->ExitControlFullscreen();
}

Control* Window::GetFullscreenControl() const
{
    return m_windowRoot->GetFullscreenControl();
}

void Window::OnDropEnterMsg(ControlDropType /*dropType*/, void* /*pDropData*/)
{
}

void Window::OnDropOverMsg(ControlDropType /*dropType*/, void* /*pDropData*/)
{
}

void Window::OnDropMsg(ControlDropType /*dropType*/, void* /*pDropData*/)
{
}

void Window::OnDropLeaveMsg()
{
}

void Window::OnDisplayResolutionChangedMsg(int32_t /*nColorDepth*/, int32_t /*nScreenWidth*/, int32_t /*nScreenHeight*/)
{
}

void Window::OnDisplayScaleChangedMsg(float /*fNewDisplayScale*/, float /*fNewPixelDensity*/)
{
}

void Window::OnDwmCompositionChangedMsg(bool bDwmCompositionEnabled)
{
    if (!bDwmCompositionEnabled && IsShadowAttached()) {
        //关闭DWM后（仅Window 7能关闭，Win 8 开始已经无法关闭DWM服务）
        ShadowType shadowType = GetShadowType();
        if (shadowType == ShadowType::kShadowDefault) {
            shadowType = Shadow::GetDefaultShadowType(this);
        }
        if (Shadow::IsSystemShadowType(shadowType)) {
            //自动切换到可用的阴影类型
            SetShadowType(ShadowType::kShadowNone);
        }
    }
}

void Window::NotifyLanguageChanged()
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (OnLanguageChanged() && !windowFlag.expired()) {
        SendWindowEvent(kWindowLanguageChangedMsg);
    }
}

void Window::NotifyThemeChanged()
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (OnThemeChanged() && !windowFlag.expired()) {
        SendWindowEvent(kWindowThemeChangedMsg);
    }
}

bool Window::OnLanguageChanged()
{
    return true;
}
bool Window::OnThemeChanged()
{
    return true;
}

} // namespace ui
