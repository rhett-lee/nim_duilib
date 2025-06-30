#include "BrowserManager.h"
#include "BrowserForm.h"
#include "TestApplication.h"

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
    return new BrowserForm;
}

BrowserBox* BrowserManager::CreateBorwserBox(BrowserForm* browser_form, std::string id, const DString& url)
{
    if (id.empty()) {
        id = CreateBrowserID();
    }
    BrowserBox *browser_box = nullptr;
    //多标签模式：一个窗口内允许有多个标签
    if (browser_form == nullptr) {
        browser_form = BrowserManager::GetInstance()->CreateBrowserForm();
        if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("WebView2Browser"), true))) {
            browser_form = nullptr;
            return nullptr;
        }
        browser_form->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);          
    }
    browser_box = browser_form->CreateBox(id, url);
    ASSERT(browser_box != nullptr);
    if (browser_box == nullptr) {
        return nullptr;
    }
    m_boxMap[id] = browser_box;
    return browser_box;
}

bool BrowserManager::IsBorwserBoxActive(const std::string& id)
{
    BrowserBox* browser_box = FindBorwserBox(id);
    if (browser_box != nullptr) {
        BrowserForm* parent_form = browser_box->GetBrowserForm();
        return parent_form->IsActiveBox(browser_box);
    }
    return false;
}

BrowserBox* BrowserManager::FindBorwserBox(const std::string& id)
{
    std::map<std::string, BrowserBox*>::const_iterator i = m_boxMap.find(id);
    if (i == m_boxMap.end()) {
        return nullptr;
    }
    else {
        return i->second;
    }
}

void BrowserManager::RemoveBorwserBox(const std::string& id, const BrowserBox* box)
{
    auto it_box = m_boxMap.find(id);
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
        //退出
        TestApplication::Instance().PostQuitMessage();
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
