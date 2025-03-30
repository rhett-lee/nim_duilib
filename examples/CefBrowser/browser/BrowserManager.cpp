#include "BrowserManager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "Windows/BrowserForm_Windows.h"
#else
    #include "BrowserForm.h"
#endif

const int kSplitFormXOffset = 20;    //自动拆分浏览器窗口后新窗口的x偏移坐标
const int kSplitFormYOffset = 20;    //自动拆分浏览器窗口后新窗口的y偏移坐标

BrowserManager::BrowserManager():
    m_bEnableMerge(true)
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

BrowserBox* BrowserManager::CreateBorwserBox(BrowserForm* browser_form, const std::string& id, const DString& url)
{
    BrowserBox *browser_box = nullptr;
    // 如果启用了窗口合并功能，就把新浏览器盒子都集中创建到某一个浏览器窗口里
    // 否则每个浏览器盒子都创建一个浏览器窗口
    if (m_bEnableMerge) {
        //多标签模式：一个窗口内允许有多个标签
        if (browser_form == nullptr) {
            browser_form = BrowserManager::GetInstance()->CreateBrowserForm();
            if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser"), true))) {
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
    }
    else {
        //多窗口模式：一个窗口仅支持一个标签
        browser_form = BrowserManager::GetInstance()->CreateBrowserForm();
        if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser"), true))) {
            return nullptr;
        }
        browser_box = browser_form->CreateBox(id, url);
        if (nullptr == browser_box) {
            return nullptr;
        }
        browser_form->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
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

void BrowserManager::SetEnableMerge(bool enable)
{
    if (m_bEnableMerge == enable) {
        return;
    }

    m_bEnableMerge = enable;

    if (m_bEnableMerge) {
        // 如果当前只有一个浏览器窗口或者浏览器盒子，就不需要进行合并操作
        if (m_boxMap.size() <= 1) {
            return;
        }

        // 选择第一个浏览器盒子所属的窗口作为合并窗口
        BrowserForm* merge_form = m_boxMap.begin()->second->GetBrowserForm();

        // 遍历所有浏览器盒子，脱离原浏览器窗口，再附加到合并窗口里
        for (auto it_box : m_boxMap) {
            ASSERT(nullptr != it_box.second);
            BrowserForm *parent_form = it_box.second->GetBrowserForm();
            if (merge_form != parent_form) {
                if (parent_form->DetachBox(it_box.second)) {
                    merge_form->AttachBox(it_box.second);
                }
            }
        }
    }
    else {
        // 如果当前只有一个浏览器盒子，就不需要进行拆分操作
        if (m_boxMap.size() <= 1) {
            return;
        }

        // 给新拆分的窗口设置坐标
        bool first_sort = true;
        ui::UiRect rect_old_form;
        BrowserForm *sort_form = nullptr;

        // 遍历所有浏览器盒子，脱离原浏览器窗口，创建新的浏览器窗口并附加浏览器盒子
        for (auto it_box : m_boxMap) {
            ASSERT(nullptr != it_box.second);
            BrowserForm *parent_form = it_box.second->GetBrowserForm();
            if (1 == parent_form->GetBoxCount()) {
                sort_form = parent_form;
            }
            else if (parent_form->DetachBox(it_box.second)) {
                BrowserForm* browser_form = BrowserManager::GetInstance()->CreateBrowserForm();
                if (!browser_form->CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser")))) {
                    ASSERT(0);
                    continue;
                }
                if (!browser_form->AttachBox(it_box.second)) {
                    ASSERT(0);
                    continue;
                }
                browser_form->ShowWindow(ui::kSW_SHOW);
                sort_form = browser_form;
            }

            if (nullptr != sort_form) {
                if (first_sort) {
                    first_sort = false;
                    sort_form->CenterWindow();
                    rect_old_form = sort_form->GetWindowPos(true);
                }
                else {
                    rect_old_form.left += sort_form->Dpi().GetScaleInt(kSplitFormXOffset);
                    rect_old_form.top += sort_form->Dpi().GetScaleInt(kSplitFormXOffset);
                    sort_form->SetWindowPos(ui::InsertAfterWnd(), rect_old_form.left, rect_old_form.top, rect_old_form.Width(), rect_old_form.Height(), ui::kSWP_NOSIZE);
                }
            }
        }
    }
}

bool BrowserManager::IsEnableMerge() const
{
    return m_bEnableMerge;
}

