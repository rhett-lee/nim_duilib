#include "XmlBox.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Shadow.h"
#include <set>

namespace ui
{
XmlBox::XmlBox(Window* pWindow):
    Box(pWindow),
    m_pSubBox(nullptr)
{
    m_pXmlPreviewAttributes = std::make_unique<XmlPreviewAttributes>();
}

XmlBox::~XmlBox()
{
    size_t callbackId = (size_t)this;
    GlobalManager::Instance().RemoveResNotFoundCallback(callbackId);
}

DString XmlBox::GetType() const { return DUI_CTR_XMLBOX; }

void XmlBox::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("xml_path")) {
        SetXmlPath(strValue);
    }
    else if (strName == _T("res_path")) {
        SetResPath(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void XmlBox::SetXmlPath(const DString& xmlPath)
{
    if (IsInited()) {
        bool bRet = LoadXmlData(xmlPath);
        if (bRet) {
            m_xmlPath = xmlPath;
        }
        OnXmlDataLoaded(xmlPath, bRet);
    }
    else {
        m_xmlPath = xmlPath;
    }
}

const DString& XmlBox::GetXmlPath() const
{
    return m_xmlPath;
}

void XmlBox::SetResPath(const DString& resPath)
{
    m_resPath = resPath;
}

const DString& XmlBox::GetResPath() const
{
    return m_resPath;
}

void XmlBox::ClearXmlBox()
{
    XmlPreviewAttributes xmlPreviewAttributes;
    ClearLoadedXmlData(xmlPreviewAttributes);
    m_xmlPath.clear();
}

void XmlBox::AddLoadXmlCallback(LoadXmlCallback callback, size_t callbackId)
{
    if (callback != nullptr) {
        LoadXmlCallbackData data;
        data.m_callback = callback;
        data.m_callbackId = callbackId;
        m_loadXmlCallbacks.push_back(data);
    }
}

void XmlBox::RemoveLoadXmlCallback(size_t callbackId)
{
    auto iter = m_loadXmlCallbacks.begin();
    while (iter != m_loadXmlCallbacks.end()) {
        if (iter->m_callbackId == callbackId) {
            iter = m_loadXmlCallbacks.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void XmlBox::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //资源加载失败的回调函数
    size_t callbackId = (size_t)this;
    GlobalManager::Instance().AddResNotFoundCallback([this](const Control* pControl,
                                                            const FilePath& resPath,
                                                            FilePath& windowResPath,
                                                            FilePath& windowXmlPath) {
            if (!PlaceHolder::IsControlRelated(this, pControl)) {
                //该控件不是本容器的子控件，不相关
                return false;
            }
            (void)resPath;//调试时可以查看其值
            windowResPath = m_resPath;
            FilePath xmlPath(m_xmlPath);
            windowXmlPath = xmlPath.GetParentPath();
            return true;
        }, callbackId);

    bool bRet = LoadXmlData(m_xmlPath);
    OnXmlDataLoaded(m_xmlPath, bRet);
}

bool XmlBox::LoadXmlData(const DString& xmlPath)
{
    if (xmlPath.empty()) {
        //清除数据，返回成功
        XmlPreviewAttributes xmlPreviewAttributes;
        ClearLoadedXmlData(xmlPreviewAttributes);
        return true;
    }
    bool bRet = false;
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return bRet;
    }
    XmlPreviewAttributes xmlPreviewAttributes;
    Box* pSubBox = ui::GlobalManager::Instance().CreateBoxForXmlPreview(GetWindow(), ui::FilePath(xmlPath), xmlPreviewAttributes);
    if (pSubBox != nullptr) {
        ClearLoadedXmlData(xmlPreviewAttributes);

        //确认是否含有阴影
        DString shadowTypeString;
        const std::map<DString, DString>& windowAttributesMap = xmlPreviewAttributes.m_windowAttributes;
        auto iter = windowAttributesMap.find(_T("shadow_type"));
        if (iter != windowAttributesMap.end()) {
            shadowTypeString = iter->second;
        }
        bool bShadowAttached = true;
        iter = windowAttributesMap.find(_T("shadow_attached"));
        if (iter != windowAttributesMap.end()) {
            bShadowAttached = (iter->second == _T("true")) ? true : false;
        }

        Shadow::ShadowType nShadowType = Shadow::ShadowType::kShadowDefault;
        if (!shadowTypeString.empty() && !Shadow::GetShadowType(shadowTypeString, nShadowType)) {
            bShadowAttached = false;
        }
        else if (nShadowType == Shadow::ShadowType::kShadowCustom) {
            bShadowAttached = false;
        }
        if (bShadowAttached) {
            m_pShadow = std::make_unique<Shadow>(GetWindow());
            m_pShadow->SetShadowType(nShadowType);
            pSubBox = m_pShadow->AttachShadow(pSubBox);
        }

        AddItem(pSubBox);
        m_pSubBox = pSubBox;
        *m_pXmlPreviewAttributes = xmlPreviewAttributes;
        bRet = true;
    }
    return bRet;
}

void XmlBox::ClearLoadedXmlData(const XmlPreviewAttributes& xmlPreviewAttributesNew)
{
    //删除已经加载的XML数据对应的UI子控件
    if (m_pSubBox != nullptr) {
        RemoveItem(m_pSubBox);
        m_pSubBox = nullptr;
    }
    m_pShadow.reset();
    
    //删除上次加载在窗口下的公共属性，避免相互干扰
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        std::vector<DString> oldWindowClassList;
        oldWindowClassList.swap(m_pXmlPreviewAttributes->m_windowClassList);
        RemoveValuesInNewList(oldWindowClassList, xmlPreviewAttributesNew.m_windowClassList);
        for (const DString& className : oldWindowClassList) {
            pWindow->RemoveClass(className);
        }

        std::vector<DString> oldWindowTextColorList;
        oldWindowTextColorList.swap(m_pXmlPreviewAttributes->m_windowTextColorList);
        RemoveValuesInNewList(oldWindowTextColorList, xmlPreviewAttributesNew.m_windowTextColorList);
        for (const DString& textColor : oldWindowTextColorList) {
            pWindow->RemoveTextColor(textColor);
        }
    }

    std::vector<DString> oldGlobalFontIdList;
    oldGlobalFontIdList.swap(m_pXmlPreviewAttributes->m_globalFontIdList);
    RemoveValuesInNewList(oldGlobalFontIdList, xmlPreviewAttributesNew.m_globalFontIdList);
    for (const DString& fontId : oldGlobalFontIdList) {
        GlobalManager::Instance().Font().RemoveFontId(fontId);
    }
}

void XmlBox::RemoveValuesInNewList(std::vector<DString>& oldList, const std::vector<DString>& newList) const
{
    if (oldList.empty() || newList.empty()) {
        return;
    }
    std::set<DString> newValueSet;
    for (const DString& name : newList) {
        newValueSet.insert(name);
    }
    auto iter = oldList.begin();
    while (iter != oldList.end()) {
        if (newValueSet.find(*iter) != newValueSet.end()) {
            iter = oldList.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void XmlBox::OnXmlDataLoaded(const DString& xmlPath, bool bSuccess)
{
    std::vector<LoadXmlCallbackData> loadXmlCallbacks = m_loadXmlCallbacks;
    for (const LoadXmlCallbackData& callbackData : loadXmlCallbacks) {
        callbackData.m_callback(xmlPath, bSuccess);
    }
}

} //namespace ui
