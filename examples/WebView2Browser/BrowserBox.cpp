#include "BrowserBox.h"
#include "BrowserForm.h"
#include "BrowserManager.h"

using namespace ui;

BrowserBox::BrowserBox(ui::Window* pWindow, std::string id):
    ui::VBox(pWindow)
{
    m_pBrowserForm = nullptr;
    m_pWebView2Control = nullptr;
    m_browserId = id;
}

BrowserForm* BrowserBox::GetBrowserForm() const
{
    ASSERT(nullptr != m_pBrowserForm);
    ASSERT(m_pBrowserForm->IsWindow());
    return m_pBrowserForm;
}

ui::WebView2Control* BrowserBox::GetWebView2Control()
{
    return m_pWebView2Control;
}

const DString& BrowserBox::GetTitle() const
{
    return m_title;
}

void BrowserBox::InitBrowserBox(const DString& url)
{
    m_pWebView2Control = static_cast<ui::WebView2Control*>(FindSubControl(_T("webview2_control")));
    ASSERT(m_pWebView2Control != nullptr);
    if (m_pWebView2Control == nullptr) {
        return;
    }
    //挂载事件
    m_pWebView2Control->SetSourceChangedCallback([this](const DString& url) {
        ui::GlobalManager::Instance().AssertUIThread();
        m_url = url;
        m_pBrowserForm->SetURL(m_browserId, url);
        });

    m_pWebView2Control->SetDocumentTitleChangedCallback([this](const DString& title) {
        ui::GlobalManager::Instance().AssertUIThread();
        m_title = title;
        m_pBrowserForm->SetTabItemName(ui::StringConvert::UTF8ToT(m_browserId), title);
        });

    m_pWebView2Control->SetNavigationStateChangedCallback([this](WebView2Control::NavigationState /*state*/, HRESULT /*errorCode*/) {
        ui::GlobalManager::Instance().AssertUIThread();
        if (m_pBrowserForm != nullptr) {
            m_pBrowserForm->OnLoadingStateChange(this);
        }
        });

    m_pWebView2Control->SetHistoryChangedCallback([this]() {
        ui::GlobalManager::Instance().AssertUIThread();
        if (m_pBrowserForm != nullptr) {
            m_pBrowserForm->OnLoadingStateChange(this);
        }
        });

    m_pWebView2Control->SetFavIconChangedCallback([this](int32_t nWidth, int32_t nHeight, const std::vector<uint8_t>& imageData) {
        ui::GlobalManager::Instance().AssertUIThread();
        if (m_pBrowserForm != nullptr) {
            m_pBrowserForm->NotifyFavicon(this, nWidth, nHeight, imageData);
        }
        });

    //导航到该网址
    DString navigateUrl = url;
    if (navigateUrl.empty()) {
        navigateUrl = _T("www.baidu.com");
    }
    m_pWebView2Control->Navigate(navigateUrl);
}

void BrowserBox::UninitBrowserBox()
{
    BrowserManager::GetInstance()->RemoveBorwserBox(m_browserId, this);
}

void BrowserBox::SetWindow(Window* pWindow)
{
    m_pBrowserForm = dynamic_cast<BrowserForm*>(pWindow);
    ASSERT(m_pBrowserForm != nullptr);

    BaseClass::SetWindow(pWindow);
}

bool BrowserBox::OnSetFocus(const ui::EventArgs& msg)
{
    // Box获取焦点时把焦点转移给网页控件
    if (m_pWebView2Control) {
        m_pWebView2Control->SetFocus();
    }

    //不再调用基类的方法，避免覆盖输入法管理的逻辑（基类会关闭输入法）
    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHot);
        Invalidate();
    }
    return true;
}
