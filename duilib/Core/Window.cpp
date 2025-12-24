#include "Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ToolTip.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{
Window::Window() :
    m_pRoot(nullptr),
    m_pFocus(nullptr),
    m_pEventHover(nullptr),
    m_pEventToolTip(nullptr),
    m_pEventClick(nullptr),
    m_pEventKey(nullptr),
    m_rcAlphaFix(0, 0, 0, 0),
    m_bFirstLayout(false),
    m_bInitLayout(false),
    m_bWindowFirstShown(false),
    m_bIsArranged(false),
    m_bPostQuitMsgWhenClosed(false),
    m_renderBackendType(RenderBackendType::kRaster_BackendType),
    m_bWindowAttributesApplied(false),
    m_bCheckSetWindowFocus(false)
{
    m_toolTip = std::make_unique<ToolTip>();
}

Window::~Window()
{
    ASSERT(!IsWindow());
    ClearWindow(false);
}

void Window::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("shadow_type")) {
        //设置窗口的阴影类型
        Shadow::ShadowType nShadowType = Shadow::ShadowType::kShadowCount;
        if (Shadow::GetShadowType(strValue, nShadowType)) {
            SetShadowType(nShadowType);
        }
    }
    else if (strName == _T("shadow_attached")) {
        //是否开启阴影
        SetShadowAttached(strValue == _T("true"));
    }
    else if (strName == _T("drag_drop")) {
        //是否开启阴影
        SetEnableDragDrop(strValue == _T("true"));
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
    if (strList.find(_T('\"')) != DString::npos) {
        AttributeUtil::ParseAttributeList(strList, _T('\"'), attributeList);
    }
    else if (strList.find(_T('\'')) != DString::npos) {
        AttributeUtil::ParseAttributeList(strList, _T('\''), attributeList);
    }
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

bool Window::AttachWindowFirstShown(const EventCallback& callback, EventCallbackID callbackID)
{
    m_OnEvent[kEventWindowFirstShown].AddEventCallback(callback, callbackID);
    return !IsWindowFirstShown();
}

void Window::AttachWindowCreate(const EventCallback& callback, EventCallbackID callbackID)
{
    m_OnEvent[kEventWindowCreate].AddEventCallback(callback, callbackID);
}

void Window::AttachWindowClose(const EventCallback& callback, EventCallbackID callbackID)
{
    m_OnEvent[kEventWindowClose].AddEventCallback(callback, callbackID);
}

void Window::AttachWindowSetFocus(const EventCallback& callback, EventCallbackID callbackID)
{
    m_OnEvent[kEventWindowSetFocus].AddEventCallback(callback, callbackID);
}

void Window::AttachWindowKillFocus(const EventCallback& callback, EventCallbackID callbackID)
{
    m_OnEvent[kEventWindowKillFocus].AddEventCallback(callback, callbackID);
}

bool Window::HasWindowEventCallback(EventType eventType) const
{
    return m_OnEvent.find(eventType) != m_OnEvent.end();
}

bool Window::HasWindowEventCallbackByID(EventCallbackID callbackID) const
{
    for (auto iter = m_OnEvent.begin(); iter != m_OnEvent.end(); ++iter) {
        if (iter->second.HasEventCallbackByID(callbackID)) {
            return true;
        }
    }
    return false;
}

void Window::DetachWindowEventCallback(EventType eventType)
{
    auto iter = m_OnEvent.find(eventType);
    if (iter != m_OnEvent.end()) {
        m_OnEvent.erase(iter);
    }
}

void Window::DetachWindowEventCallbackByID(EventCallbackID callbackID)
{
    EventUtils::RemoveEventCallbackByID(m_OnEvent, callbackID);
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
            bRet = (m_render != nullptr) ? true : false;
        }
    }
    else {
        ASSERT(m_render->GetRenderBackendType() == backendType);
        bRet = (m_render->GetRenderBackendType() == backendType) ? true : false;
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
    bool bLocalPath = true;
    bool bResPath = true;
    FilePath iconFullPath = GlobalManager::Instance().GetExistsResFullPath(GetResourcePath(), GetXmlPath(), FilePath(iconFilePath), bLocalPath, bResPath);
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
        m_windowBuilder->ParseWindowCreateAttributes(createAttributes);
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

Shadow* Window::GetShadow() const
{
    ASSERT(m_shadow != nullptr);
    return m_shadow.get();
}

void Window::PreInitWindow()
{
    if (!IsWindow()) {
        return;
    }
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    ASSERT(m_shadow == nullptr);
    if (m_shadow != nullptr) {
        return;
    }

    //创建窗口阴影
    m_shadow = std::make_unique<Shadow>(this);
    if (m_shadow->IsUseDefaultShadowAttached()) {
        m_shadow->SetShadowAttached(IsLayeredWindow());
        m_shadow->SetUseDefaultShadowAttached(true);
    }

    //添加到全局管理器
    GlobalManager::Instance().Windows().AddWindow(this);

    //解析窗口关联的XML文件
    if (m_windowBuilder == nullptr) {
        ParseWindowXml();
    }

    Box* pRoot = nullptr;
    if (m_windowBuilder != nullptr) {
        auto callback = UiBind(&Window::CreateControl, this, std::placeholders::_1);
        Control* pControl = m_windowBuilder->CreateControls(this, callback);
        pRoot = m_windowBuilder->ToBox(pControl);
        ASSERT(pRoot != nullptr);
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

        //更新自绘制标题栏状态
        OnUseSystemCaptionBarChanged();
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
    ClearStatus();
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
    ClearWindow(true);
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

void Window::ClearWindow(bool bSendClose)
{
    bool bHasWindow = GlobalManager::Instance().Windows().HasWindow(this);
    if (bSendClose && bHasWindow) {
        //发送关闭事件
        WPARAM wParam = (WPARAM)GetCloseParam();
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        SendNotify(kEventWindowClose, wParam);
        if (windowFlag.expired()) {
            return;
        }
    }
    //回收控件
    GlobalManager::Instance().Windows().RemoveWindow(this);
    ReapObjects(GetRoot());

    m_controlFinder.Clear();
    Box* pRoot = m_pRoot.get();
    m_pRoot.reset();
    if (pRoot != nullptr) {
        delete pRoot;
        pRoot = nullptr;
    }

    RemoveAllClass();
    RemoveAllOptionGroups();

    m_toolTip.reset();
    m_shadow.reset();
    m_render.reset();    
}

bool Window::AttachBox(Box* pRoot)
{
    ASSERT(IsWindow());
    SetFocusControl(nullptr); //设置m_pFocus相关的状态
    m_pEventKey = nullptr;
    m_pEventHover = nullptr;
    m_pEventToolTip = nullptr;
    m_pEventClick = nullptr;
    // Remove the existing control-tree. We might have gotten inside this function as
    // a result of an event fired or similar, so we cannot just delete the objects and
    // pull the internal memory of the calling code. We'll delay the cleanup.
    if ((m_pRoot != nullptr) && (pRoot != m_pRoot)) {
        Box* pOldRoot = m_pRoot.get();
        m_pRoot.reset();
        if (pOldRoot != nullptr) {
            delete pOldRoot;
            pOldRoot = nullptr;
        }
    }
    // Set the dialog root element
    m_pRoot = pRoot;
    m_controlFinder.SetRoot(pRoot);
    // Go ahead...
    m_bIsArranged = true;
    m_bFirstLayout = false;
    m_bInitLayout = false;
    // Initiate all control
    return InitControls(m_pRoot.get());
}

Box* Window::GetRoot() const
{
    return m_pRoot.get();
}

Box* Window::GetXmlRoot() const
{
    Box* pXmlRoot = nullptr;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pXmlRoot = pShadow->GetAttachedXmlRoot();
    }
    if (pXmlRoot == nullptr) {
        pXmlRoot = m_pRoot.get();
    }
    return pXmlRoot;
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

void Window::AddTextColor(const DString& strName, const DString& strValue)
{
    m_colorMap.AddColor(strName, strValue);
}

void Window::AddTextColor(const DString& strName, UiColor argb)
{
    m_colorMap.AddColor(strName, argb);
}

UiColor Window::GetTextColor(const DString& strName) const
{
    return m_colorMap.GetColor(strName);
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
    Control* pRoot = nullptr;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pRoot = pShadow->GetShadowBox();
    }
    if (pRoot) {
        pRoot->ClearImageCache();
    }
    else if (m_pRoot != nullptr) {
        m_pRoot->ClearImageCache();
    }
}

void Window::OnUseSystemCaptionBarChanged()
{
    if (IsUseSystemCaption()) {
        //关闭阴影
        SetShadowAttached(false);
    }
}

void Window::OnLayeredWindowChanged()
{
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    Shadow* pShadow = GetShadow();
    if ((pShadow != nullptr) && pShadow->IsUseDefaultShadowAttached()) {
        pShadow->SetShadowAttached(IsLayeredWindow());
        pShadow->SetUseDefaultShadowAttached(true);
    }
    InvalidateAll();
}

void Window::InvalidateAll()
{
    UiRect rcClient;
    GetClientRect(rcClient);
    Invalidate(rcClient);
}

void Window::OnWindowAlphaChanged()
{
    InvalidateAll();
}

void Window::OnWindowEnterFullScreen()
{
}

void Window::OnWindowExitFullScreen()
{
}

void Window::OnWindowDisplayScaleChanged(uint32_t /*nOldScaleFactor*/, uint32_t /*nNewScaleFactor*/)
{
}

void Window::GetShadowCorner(UiPadding& rcShadow) const
{
    rcShadow.Clear();
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        rcShadow = pShadow->GetShadowCorner();
    }
}

void Window::GetCurrentShadowCorner(UiPadding& rcShadow) const
{
    rcShadow.Clear();
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        rcShadow = pShadow->GetCurrentShadowCorner();
    }
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

const UiRect& Window::GetAlphaFixCorner() const
{
    return m_rcAlphaFix;
}

void Window::SetAlphaFixCorner(const UiRect& rc, bool bNeedDpiScale)
{
    ASSERT((rc.left >= 0) && (rc.top >= 0) && (rc.right >= 0) && (rc.bottom >= 0));
    if ((rc.left >= 0) && (rc.top >= 0) && (rc.right >= 0) && (rc.bottom >= 0)) {
        m_rcAlphaFix = rc;
        if (bNeedDpiScale) {
            Dpi().ScaleRect(m_rcAlphaFix);
        }
    }
}

Box* Window::AttachShadow(Box* pRoot)
{
    //将阴影附加到窗口
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->AttachShadow(pRoot);
    }
    else {
        return pRoot;
    }
}

void Window::SetShadowAttached(bool bShadowAttached)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowAttached(bShadowAttached);
        OnWindowShadowTypeChanged();
    }
}

void Window::SetShadowType(Shadow::ShadowType nShadowType)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowType(nShadowType);
        //重绘窗口，否则会有绘制异常
        InvalidateAll();
        OnWindowShadowTypeChanged();
    }
}

Shadow::ShadowType Window::GetShadowType() const
{
    Shadow::ShadowType nShadowType = Shadow::ShadowType::kShadowDefault;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        nShadowType = pShadow->GetShadowType();
    }
    return nShadowType;
}

DString Window::GetShadowImage() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->GetShadowImage();
    }
    else {
        return DString();
    }
}

void Window::SetShadowImage(const DString& shadowImage)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowImage(shadowImage);
    }
}

void Window::SetShadowBorderSize(int32_t nShadowBorderSize)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderSize(nShadowBorderSize);
    }
}

int32_t Window::GetShadowBorderSize() const
{
    Shadow* pShadow = GetShadow();
    int32_t nShadowBorderSize = 0;    
    if (pShadow != nullptr) {
        nShadowBorderSize = pShadow->GetShadowBorderSize();
    }
    return nShadowBorderSize;
}

void Window::SetShadowBorderColor(const DString& shadowBorderColor)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderColor(shadowBorderColor);
    }
}

DString Window::GetShadowBorderColor() const
{
    Shadow* pShadow = GetShadow();
    DString shadowBorderColor;
    if (pShadow != nullptr) {
        shadowBorderColor = pShadow->GetShadowBorderColor();
    }
    return shadowBorderColor;
}

UiPadding Window::GetCurrentShadowCorner() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->GetCurrentShadowCorner();
    }
    else {
        return UiPadding();
    }
}

bool Window::IsShadowAttached() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->IsShadowAttached();
    }
    else {
        return false;
    }
}

bool Window::IsUseDefaultShadowAttached() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->IsUseDefaultShadowAttached();
    } 
    else {
        return false;
    }    
}

void Window::SetUseDefaultShadowAttached(bool bDefault)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetUseDefaultShadowAttached(bDefault);
    }
}

UiPadding Window::GetShadowCorner() const
{
    UiPadding rcShadowCorner;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        rcShadowCorner = pShadow->GetShadowCorner();
    }
    return rcShadowCorner;
}

void Window::SetShadowCorner(const UiPadding& rcShadowCorner)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowCorner(rcShadowCorner);
    }
}

void Window::SetShadowBorderRound(UiSize szBorderRound)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderRound(szBorderRound);
    }
}

void Window::SetEnableShadowSnap(bool bEnable)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetEnableShadowSnap(bEnable);
    }
}

bool Window::IsEnableShadowSnap() const
{
    bool bRet = false;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        bRet = pShadow->IsEnableShadowSnap();
    }
    return bRet;
}

UiSize Window::GetShadowBorderRound() const
{
    UiSize szBorderRound;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        szBorderRound = pShadow->GetShadowBorderRound();
    }
    return szBorderRound;
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
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->ChangeDpiScale(Dpi(), nOldScaleFactor, nNewScaleFactor);
    }

    //更新窗口自身的DPI关联属性
    m_rcAlphaFix = Dpi().GetScaleRect(m_rcAlphaFix, nOldScaleFactor);
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

LRESULT Window::OnSizeMsg(WindowSizeType sizeType, const UiSize& /*newWindowSize*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    //调整Render的大小, 与客户区大小保持一致
    ResizeRenderToClientSize();

    Box* pRoot = GetRoot();
    if (pRoot != nullptr) {
        pRoot->Arrange();
    }
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        //最大化
        if (!IsUseSystemCaption() && !IsWindowFullScreen()) {
            UiRect rcWindow;
            GetWindowRect(rcWindow);
            UiRect rcClientRect;
            GetClientRect(rcClientRect);
            int32_t cxClient = rcClientRect.Width();
            int32_t cyClient = rcClientRect.Height();
            if (Dpi().HasPixelDensity()) {
                Dpi().UnscaleInt(cxClient);
                Dpi().UnscaleInt(cyClient);
                Dpi().ScaleWindowSize(cxClient);
                Dpi().ScaleWindowSize(cyClient);
            }
            if ((cxClient == rcWindow.Width()) && (cyClient == rcWindow.Height())) {
                //全屏时，设置外边距，避免客户区的内容溢出屏幕
                UiRect rcWork;
                GetMonitorWorkRect(rcWork);

                UiMargin rcFullscreenMargin;
                if (rcWindow.left < rcWork.left) {
                    rcFullscreenMargin.left = rcWork.left - rcWindow.left;
                }
                if (rcWindow.top < rcWork.top) {
                    rcFullscreenMargin.top = rcWork.top - rcWindow.top;
                }
                if (rcWindow.right > rcWork.right) {
                    rcFullscreenMargin.right = rcWindow.right - rcWork.right;
                }
                if (rcWindow.bottom > rcWork.bottom) {
                    rcFullscreenMargin.bottom = rcWindow.bottom - rcWork.bottom;
                }
                if (Dpi().HasPixelDensity()) {
                    rcFullscreenMargin.left = (int32_t)std::round(rcFullscreenMargin.left * Dpi().GetPixelDensity());
                    rcFullscreenMargin.top = (int32_t)std::round(rcFullscreenMargin.top * Dpi().GetPixelDensity());
                    rcFullscreenMargin.right = (int32_t)std::round(rcFullscreenMargin.right * Dpi().GetPixelDensity());
                    rcFullscreenMargin.bottom = (int32_t)std::round(rcFullscreenMargin.bottom * Dpi().GetPixelDensity());
                }
                bool bHasShadowBox = false;
                Box* pXmlRoot = GetXmlRoot();
                Shadow* pShadow = GetShadow();
                if ((pShadow != nullptr) && pShadow->HasShadowBox()) {
                    bHasShadowBox = true;                    
                }
                if (pXmlRoot != nullptr) {
                    if (bHasShadowBox) {
                        //有阴影Box
                        UiMargin rcMargin = pXmlRoot->GetMargin();
                        rcMargin.left += (rcFullscreenMargin.left - m_rcFullscreenMargin.left);
                        rcMargin.top += (rcFullscreenMargin.top - m_rcFullscreenMargin.top);
                        rcMargin.right += (rcFullscreenMargin.right - m_rcFullscreenMargin.right);
                        rcMargin.bottom += (rcFullscreenMargin.bottom - m_rcFullscreenMargin.bottom);
                        m_rcFullscreenMargin = rcFullscreenMargin;
                        pXmlRoot->SetMargin(rcMargin, false);
                    }
                    else {
                        //无阴影Box
                        UiPadding rcPadding = pXmlRoot->GetPadding();
                        rcPadding.left += (rcFullscreenMargin.left - m_rcFullscreenMargin.left);
                        rcPadding.top += (rcFullscreenMargin.top - m_rcFullscreenMargin.top);
                        rcPadding.right += (rcFullscreenMargin.right - m_rcFullscreenMargin.right);
                        rcPadding.bottom += (rcFullscreenMargin.bottom - m_rcFullscreenMargin.bottom);
                        m_rcFullscreenMargin = rcFullscreenMargin;
                        pXmlRoot->SetPadding(rcPadding, false);
                    }                    
                }
            }
        }
        Shadow* pShadow = GetShadow();
        if (pShadow != nullptr) {
            pShadow->MaximizedOrRestored(true);
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        //还原
        Shadow* pShadow = GetShadow();
        if (pShadow != nullptr) {
            pShadow->MaximizedOrRestored(false);
        }
        //还原时，恢复外边距
        if (!m_rcFullscreenMargin.IsEmpty()) {
            bool bHasShadowBox = false;
            Box* pXmlRoot = GetXmlRoot();
            if ((pShadow != nullptr) && pShadow->HasShadowBox()) {
                bHasShadowBox = true;
            }
            if (pXmlRoot != nullptr) {
                if (bHasShadowBox) {
                    //有阴影Box
                    UiMargin rcMargin = pXmlRoot->GetMargin();
                    rcMargin.left -= m_rcFullscreenMargin.left;
                    rcMargin.top -= m_rcFullscreenMargin.top;
                    rcMargin.right -= m_rcFullscreenMargin.right;
                    rcMargin.bottom -= m_rcFullscreenMargin.right;
                    pXmlRoot->SetMargin(rcMargin, false);
                }
                else {
                    //无阴影Box
                    UiPadding rcPadding = pXmlRoot->GetPadding();
                    rcPadding.left -= m_rcFullscreenMargin.left;
                    rcPadding.top -= m_rcFullscreenMargin.top;
                    rcPadding.right -= m_rcFullscreenMargin.right;
                    rcPadding.bottom -= m_rcFullscreenMargin.right;
                    pXmlRoot->SetPadding(rcPadding, false);
                }
                m_rcFullscreenMargin.Clear();
            }
        }
    }
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.eventData = (int32_t)sizeType;
        m_pFocus->SendEvent(kEventWindowSize, msgData);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.ptMouse = ptTopLeft;
        m_pFocus->SendEvent(kEventWindowMove, msgData);
        if (windowFlag.expired()) {
            return 0;
        }
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

bool Window::IsWindowFirstShown() const
{
    return m_bWindowFirstShown;
}

LRESULT Window::OnPaintMsg(const UiRect& rcPaint, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    PerformanceStat statPerformance(_T("PaintWindow, Window::OnPaintMsg"));
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

    //首次绘制事件, 给一次回调
    if (!IsWindowFirstShown()) {
        m_bWindowFirstShown = true;

        //触发第一次绘制事件
        SendNotify(kEventWindowFirstShown);
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

    //开始绘制前，去掉alpha通道
    if (IsLayeredWindow()) {
        PerformanceStat statPerformance(_T("PaintWindow, Window::Paint ClearAlpha"));
        pRender->ClearAlpha(rcPaint);
    }

    // 绘制
    Box* pRoot = GetRoot();
    if (pRoot == nullptr) {
        return false;
    }
    if (pRoot->IsVisible()) {
        PerformanceStat statPerformance(_T("PaintWindow, Window::Paint Paint/PaintChild"));
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
        pRender->FillRect(rcPaint, bkColor);
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_RICH_EDIT_DRAW_OPT)
    //开始绘制前，进行alpha通道修复
    if (IsLayeredWindow()) {
        PerformanceStat statPerformance(_T("PaintWindow, Window::Paint RestoreAlpha"));
        Shadow* pShadow = GetShadow();
        if ((pShadow != nullptr) && pShadow->IsShadowAttached() &&
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
            pRender->RestoreAlpha(rcNewPaint, rcRootPadding);//目前只有Windows的RichEdit绘制导致窗体透明，所以才需要回复
        }
        else {
            UiRect rcAlphaFixCorner = GetAlphaFixCorner();
            if ((rcAlphaFixCorner.left > 0) || (rcAlphaFixCorner.top > 0) ||
                (rcAlphaFixCorner.right > 0) || (rcAlphaFixCorner.bottom > 0)) {
                UiRect rcNewPaint = rcPaint;
                UiRect rcRootPaddingPos = pRoot->GetPosWithoutPadding();
                rcRootPaddingPos.Deflate(rcAlphaFixCorner.left, rcAlphaFixCorner.top,
                                         rcAlphaFixCorner.right, rcAlphaFixCorner.bottom);
                rcNewPaint.Intersect(rcRootPaddingPos);
                UiPadding rcRootPadding;
                pRender->RestoreAlpha(rcNewPaint, rcRootPadding);//目前只有Windows的RichEdit绘制导致窗体透明，所以才需要回复
            }
        }
    }
#endif
    return true;
}

LRESULT Window::OnSetFocusMsg(WindowBase* /*pLostFocusWindow*/, const NativeMsg& nativeMsg, bool& bHandled)
{
    bHandled = false;
    //获得焦点时，如果无焦点控件，则关闭输入法
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    ControlPtr pFocus = m_pFocus;
    if (pFocus != nullptr) {
        pFocus->SendEvent(kEventWindowSetFocus);

        //重新激活控件焦点（但不恢复Hot状态，避免按钮等控件的显示状态异常）
        if (!windowFlag.expired() && (pFocus == m_pFocus)) {
            pFocus->SendEvent(kEventSetFocus);
        }
        if (!windowFlag.expired() && (pFocus == m_pFocus)) {
            UiPoint pt;
            GetCursorPos(pt);
            ScreenToClient(pt);
            if (pFocus->IsPointInWithScrollOffset(pt)) {
                //鼠标还在控件范围内，保持hot状态
                pFocus->SetState(kControlStateHot);
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

    if (!windowFlag.expired()) {
        SendNotify(kEventWindowSetFocus, nativeMsg.wParam);
    }
    return 0;
}

LRESULT Window::OnKillFocusMsg(WindowBase* /*pSetFocusWindow*/, const NativeMsg& nativeMsg, bool& bHandled)
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

    if (!windowFlag.expired()) {
        SendNotify(kEventWindowKillFocus, nativeMsg.wParam);
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

    if ((vkCode == kVK_ESCAPE) && IsWindowFullScreen()) {
        //按ESC键时，退出全屏
        ExitFullScreen();
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
    ASSERT_UNUSED_VARIABLE(modifierKey);

    //待添加（需确认，应该是要加在窗口上的）
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
    std::weak_ptr<WeakFlag> hoverFlag = pNewHover->GetWeakFlag();
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    EventArgs msgData;
    msgData.modifierKey = modifierKey;
    msgData.ptMouse = pt;
    msgData.wParam = nativeMsg.wParam;
    msgData.lParam = nativeMsg.lParam;
    pNewHover->SendEvent(kEventMouseHover, msgData);
    if (hoverFlag.expired() || windowFlag.expired()) {
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

void Window::OnCreateWndMsg(bool bDoModal, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    //给应用层发一个事件
    SendNotify(kEventWindowCreate, bDoModal ? 1 : 0);
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
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetWindowPosSnap(bLeftSnap, bRightSnap, bTopSnap, bBottomSnap);
    }
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
    Shadow* pShadow = GetShadow();
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
    else if (!IsUseSystemCaption() && (pShadow != nullptr) && IsShadowAttached()) {
        //检查是否点击在窗口阴影区域(实现鼠标点击阴影，穿透到后面窗口的功能)
        pShadow->CheckMouseClickOnShadow(eventType, pt);
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

void Window::ClearStatus()
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

bool Window::SendNotify(EventType eventType, WPARAM wParam, LPARAM lParam)
{
    EventArgs msg;
    msg.SetSender(nullptr);
    msg.eventType = eventType;
    msg.ptMouse = GetLastMousePos();
    msg.wParam = wParam;
    msg.lParam = lParam;

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    auto callback = m_OnEvent.find(msg.eventType);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }
    if (windowFlag.expired()) {
        return false;
    }

    callback = m_OnEvent.find(kEventAll);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }

    return true;
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
    if (m_bIsArranged) {
        m_bIsArranged = false;
        if (pRoot->IsArranged() || (pRoot->GetPos() != rcClient)) {
            //所有控件的布局全部重排
            pRoot->SetPos(rcClient);
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
    else if (pRoot->GetPos() != rcClient) {
        //所有控件的布局全部重排
        pRoot->SetPos(rcClient);
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

Control* Window::OnNativeFindControl(const UiPoint& pt) const
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

} // namespace ui
