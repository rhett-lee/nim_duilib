#include "BrowserManager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "Windows/BrowserForm_Windows.h"
#else
    #include "BrowserForm.h"
#endif

BrowserManager::BrowserManager()
{
}

BrowserManager::~BrowserManager()
{
}

BrowserManager* BrowserManager::GetInstance()
{
    static BrowserManager self;
    return &self;
}

BrowserForm* BrowserManager::CreateBrowserForm()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    return new BrowserForm_Windows;
#else
    return new BrowserForm;
#endif
}

BrowserBox* BrowserManager::CreateBorwserBox(BrowserForm* pBrowserForm, const std::string& browserId, const DString& url)
{
    BrowserBox* pBrowserBox = nullptr;
    if (pBrowserForm == nullptr) {
        pBrowserForm = BrowserManager::GetInstance()->CreateBrowserForm();
        if (!pBrowserForm->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser"), true))) {
            pBrowserForm = nullptr;
            return nullptr;
        }
        pBrowserForm->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);          
    }
    pBrowserBox = pBrowserForm->CreateBox(browserId, url);
    ASSERT(pBrowserBox != nullptr);
    if (pBrowserBox == nullptr) {
        return nullptr;
    }

    m_boxMap[browserId] = pBrowserBox;
    return pBrowserBox;
}

bool BrowserManager::IsBorwserBoxActive(const std::string& browserId)
{
    BrowserBox* pBrowserBox = FindBorwserBox(browserId);
    if (pBrowserBox != nullptr) {
        BrowserForm* parent_form = pBrowserBox->GetBrowserForm();
        return parent_form->IsActiveBox(pBrowserBox);
    }
    return false;
}

BrowserBox* BrowserManager::FindBorwserBox(const std::string& browserId)
{
    std::map<std::string, BrowserBox*>::const_iterator i = m_boxMap.find(browserId);
    if (i == m_boxMap.end()) {
        return nullptr;
    }
    else {
        return i->second;
    }
}

void BrowserManager::RemoveBorwserBox(const std::string& browserId, const BrowserBox* box)
{
    auto it_box = m_boxMap.find(browserId);
    if (it_box == m_boxMap.end()) {
        ASSERT(0);
    }
    else {
        if ((box == nullptr) || (box == it_box->second)) {
            m_boxMap.erase(it_box);
        }
        else {
            ASSERT(0);
        }
    }

    if (m_boxMap.empty()) {
        ui::CefManager::GetInstance()->PostQuitMessage(0);
    }
}

BrowserForm* BrowserManager::GetLastActiveBrowserForm() const
{
    BrowserForm* pLastActiveBrowserForm = nullptr;
    for (auto iter : m_boxMap) {
        if (iter.second != nullptr) {
            BrowserForm* pBrowserForm = iter.second->GetBrowserForm();
            if ((pBrowserForm != nullptr) && pBrowserForm->IsWindow() && pBrowserForm->IsWindowVisible()) {
                if (pBrowserForm->IsWindowForeground()) {
                    pLastActiveBrowserForm = pBrowserForm;
                    break;
                }
                else {
                    pLastActiveBrowserForm = pBrowserForm;
                }
            }
        }
    }
    return pLastActiveBrowserForm;
}

std::string BrowserManager::CreateBrowserID() const
{
    uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
    std::string id = ui::StringUtil::UInt64ToStringA(nTimeMS);
    return id;
}

