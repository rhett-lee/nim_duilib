#include "ChatForm.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/ToastWnd.h"
#include "duilib/Utils/StringUtil.h"
#include <ctime>

ChatForm::ChatForm() :
    m_curNavTab(NavTab::kChats),
    m_nCurrentSession(0)
{
}

ChatForm::~ChatForm()
{
}

DString ChatForm::GetSkinFolder()
{
    return _T("chat");
}

DString ChatForm::GetSkinFile()
{
    return _T("wechat.xml");
}

void ChatForm::SetLoginUser(const DString& strUser)
{
    m_strLoginUser = strUser;
}

void ChatForm::OnInitWindow()
{
    //会话数据
    m_sessions.resize(3);
    m_sessions[0].name = L"张伟";
    m_sessions[0].avatarText = L"张";
    m_sessions[0].preview = L"新的设计稿已经发到邮箱了，记得看一下";
    m_sessions[0].time = L"上午 10:24";
    m_sessions[1].name = L"产品设计组";
    m_sessions[1].avatarText = L"设";
    m_sessions[1].preview = L"[图片]";
    m_sessions[1].time = L"昨天";
    m_sessions[2].name = L"李娜";
    m_sessions[2].avatarText = L"李";
    m_sessions[2].preview = L"好的，周三下午的会议我准时参加";
    m_sessions[2].time = L"星期一";
    m_sessions[2].unread = true;

    BindNavButtons();
    BindSessionItems();
    BindChatButtons();
    BindSettingsButton();
    RefreshSessionList();
    InitMessages();
    UpdatePanelVisibility();

    //标题栏显示登录用户
    ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("chat_title")));
    if ((pTitle != nullptr) && !m_strLoginUser.empty()) {
        DString strTitle = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_TITLE")).c_str();
        pTitle->SetText((strTitle + _T("  -  ") + m_strLoginUser).c_str());
    }
}

void ChatForm::BindNavButtons()
{
    struct NavItem
    {
        const TCHAR* name;
        NavTab tab;
    };
    const NavItem items[] = {
        {_T("nav_chat"), NavTab::kChats},
        {_T("nav_contacts"), NavTab::kContacts},
        {_T("nav_favorites"), NavTab::kFavorites},
    };
    for (const NavItem& item : items) {
        ui::Option* pBtn = dynamic_cast<ui::Option*>(FindControl(item.name));
        if (pBtn == nullptr) {
            continue;
        }
        const NavTab tab = item.tab;
        pBtn->AttachClick([this, tab](const ui::EventArgs& /*args*/) {
            SwitchNavTab(tab);
            return true;
            });
    }
}

void ChatForm::BindSessionItems()
{
    //会话条目数量与数据保持一致
    const size_t nCount = (m_sessions.size() < 3) ? m_sessions.size() : 3;
    for (size_t i = 0; i < nCount; ++i) {
        const DString strName = _T("session_") + std::to_wstring(i + 1);
        ui::HBox* pItem = dynamic_cast<ui::HBox*>(FindControl(strName.c_str()));
        if (pItem == nullptr) {
            continue;
        }
        const int nIndex = static_cast<int>(i);
        pItem->AttachClick([this, nIndex](const ui::EventArgs& /*args*/) {
            if (m_nCurrentSession != nIndex) {
                m_nCurrentSession = nIndex;
                InitMessages();
                //更新标题
                ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("chat_title")));
                if (pTitle != nullptr) {
                    DString strTitle = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_TITLE")).c_str();
                    pTitle->SetText((strTitle + _T("  -  ") + m_sessions[nIndex].name).c_str());
                }
            }
            return true;
            });
    }
}

void ChatForm::BindChatButtons()
{
    //发送按钮
    ui::Button* pSendBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_send")));
    if (pSendBtn != nullptr) {
        pSendBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            SendInputMessage();
            return true;
            });
    }

    //输入框回车发送（Ctrl+Enter 换行）
    ui::RichEdit2* pInput = dynamic_cast<ui::RichEdit2*>(FindControl(_T("input_edit")));
    if (pInput != nullptr) {
        pInput->AttachReturn([this](const ui::EventArgs& /*args*/) {
            SendInputMessage();
            return true;
            });
    }

    //表情按钮：插入一个表情符号到输入框
    ui::Button* pEmojiBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_chat_emoji")));
    if (pEmojiBtn != nullptr) {
        pEmojiBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::RichEdit2* pEdit = dynamic_cast<ui::RichEdit2*>(FindControl(_T("input_edit")));
            if (pEdit != nullptr) {
                const DString strText = pEdit->GetText().c_str();
                pEdit->SetText((strText + _T("[微笑]")).c_str());
                pEdit->SetFocus();
            }
            return true;
            });
    }

    //其余工具按钮：给出明确的提示，避免点击后毫无反应
    struct ToolButton
    {
        const TCHAR* name;
        const TCHAR* tipId;
    };
    const ToolButton toolButtons[] = {
        {_T("btn_add_session"), _T("STRID_CHAT_TIP_NEW_SESSION")},
        {_T("btn_search_icon"), _T("STRID_CHAT_TIP_SEARCH")},
        {_T("btn_chat_video"),  _T("STRID_CHAT_TIP_VIDEO")},
        {_T("btn_chat_phone"),  _T("STRID_CHAT_TIP_PHONE")},
        {_T("btn_chat_more"),   _T("STRID_CHAT_TIP_MORE")},
        {_T("btn_chat_image"),  _T("STRID_CHAT_TIP_IMAGE")},
        {_T("btn_chat_history"),_T("STRID_CHAT_TIP_HISTORY")},
    };
    for (const ToolButton& item : toolButtons) {
        ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(item.name));
        if (pBtn == nullptr) {
            continue;
        }
        const DString strTipId = item.tipId;
        pBtn->AttachClick([this, strTipId](const ui::EventArgs& /*args*/) {
            const DString strText = ui::GlobalManager::Instance().Lang().GetStringByID(strTipId).c_str();
            ui::ToastWnd::Show(this, strText, ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosBottom);
            return true;
            });
    }
}

void ChatForm::BindSettingsButton()
{
    ui::Button* pSettingsBtn = dynamic_cast<ui::Button*>(FindControl(_T("nav_settings")));
    if (pSettingsBtn == nullptr) {
        return;
    }
    pSettingsBtn->AttachClick([this, pSettingsBtn](const ui::EventArgs& /*args*/) {
        ui::UiRect rect = pSettingsBtn->GetPos();
        ui::UiPoint point;
        point.x = rect.left;
        point.y = rect.bottom;
        ClientToScreen(point);
        ShowSettingsMenu(point);
        return true;
        });
}

void ChatForm::ShowSettingsMenu(const ui::UiPoint& point)
{
    ui::Menu* pMenu = new ui::Menu(this, nullptr);
    pMenu->SetSkinFolder(GetResourcePath().ToString());
    pMenu->ShowMenu(_T("chat/menu_settings.xml"), point);
    pMenu->AttachMenuItemActivated([this](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                          const DString& strItemName, size_t /*nItemIndex*/) {
        OnSettingsMenuItemActivated(strItemName);
        });
}

void ChatForm::OnSettingsMenuItemActivated(const DString& strMenuItemName)
{
    if (strMenuItemName == _T("menu_logout")) {
        ExitApp();
        return;
    }
    if (strMenuItemName == _T("menu_new_chat")) {
        SwitchNavTab(NavTab::kChats);
        ui::ToastWnd::Show(this,
            ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_TIP_NEW_SESSION")),
            ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosBottom);
        return;
    }
    if (strMenuItemName == _T("menu_contacts")) {
        SwitchNavTab(NavTab::kContacts);
        return;
    }
    if (strMenuItemName == _T("menu_about")) {
        ui::ToastWnd::Show(this,
            ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_TIP_ABOUT")),
            ui::ToastWnd::kTypeInfo, 2500, ui::ToastWnd::kPosBottom);
        return;
    }
}

void ChatForm::ExitApp()
{
    //异步关闭主窗口；主窗口设置了 PostQuitMsgWhenClosed(true)，关闭后消息循环结束
    //这里用 CloseWnd 而非 Close：菜单回调执行期间同步销毁窗口不安全
    CloseWnd();
}

void ChatForm::RefreshSessionList()
{
    //把会话数据同步到列表控件（名称/时间/预览/头像）
    for (size_t i = 0; i < m_sessions.size(); ++i) {
        const size_t nIndex = i;
        const DString strPrefix = _T("session_") + std::to_wstring(i + 1);
        ui::Label* pName = dynamic_cast<ui::Label*>(FindControl((strPrefix + _T("_name")).c_str()));
        if (pName != nullptr) {
            pName->SetText(m_sessions[nIndex].name.c_str());
        }
        ui::Label* pTime = dynamic_cast<ui::Label*>(FindControl((strPrefix + _T("_time")).c_str()));
        if (pTime != nullptr) {
            pTime->SetText(m_sessions[nIndex].time.c_str());
        }
        ui::Label* pMsg = dynamic_cast<ui::Label*>(FindControl((strPrefix + _T("_msg")).c_str()));
        if (pMsg != nullptr) {
            pMsg->SetText(m_sessions[nIndex].preview.c_str());
        }
        ui::Avatar* pAvatar = dynamic_cast<ui::Avatar*>(FindControl((strPrefix + _T("_avatar")).c_str()));
        if (pAvatar != nullptr) {
            pAvatar->SetText(m_sessions[nIndex].avatarText.c_str());
        }
    }
}

void ChatForm::InitMessages()
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    pMsgList->ClearMessages();
    if (!IsCurrentSessionValid()) {
        return;
    }
    const ChatSession& session = m_sessions[m_nCurrentSession];
    const DString strTime = GetCurrentTimeText();

    if (m_nCurrentSession == 0) {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, _T("下午好，新版本的界面已经整理好了"), session.avatarText, strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kSelf, _T("好的，我来看看"), _T("我"), strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, _T("收到，辛苦"), session.avatarText, strTime);
    }
    else if (m_nCurrentSession == 1) {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, _T("新版设计稿已经上传到共享目录了"), session.avatarText, strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kSystem, _T("你已加入“产品设计组”"), _T(""), _T(""));
    }
    else {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, _T("好的，周三下午的会议我准时参加"), session.avatarText, strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kSelf, _T("收到，到时候见"), _T("我"), strTime);
    }
    pMsgList->ScrollToBottom();
}

void ChatForm::SwitchNavTab(NavTab tab)
{
    m_curNavTab = tab;
    UpdatePanelVisibility();
}

void ChatForm::UpdatePanelVisibility()
{
    //三个列表面板由 TabBox 承载，切换选中项即可（框架自动处理可见性与重排）
    ui::TabBox* pPanelBox = dynamic_cast<ui::TabBox*>(FindControl(_T("panel_box")));
    if (pPanelBox != nullptr) {
        pPanelBox->SelectItem(static_cast<size_t>(m_curNavTab));
    }

    //导航按钮的选中状态
    struct NavItem
    {
        const TCHAR* name;
        NavTab tab;
    };
    const NavItem items[] = {
        {_T("nav_chat"), NavTab::kChats},
        {_T("nav_contacts"), NavTab::kContacts},
        {_T("nav_favorites"), NavTab::kFavorites},
    };
    for (const NavItem& item : items) {
        ui::Option* pBtn = dynamic_cast<ui::Option*>(FindControl(item.name));
        if (pBtn != nullptr) {
            //不触发事件，避免递归切换
            pBtn->Selected(item.tab == m_curNavTab, false);
        }
    }

    //会话列表顶部的标题随栏目变化
    const TCHAR* titleIds[] = {_T("STRID_CHAT_PANEL_CHATS"), _T("STRID_CHAT_PANEL_CONTACTS"),
                                _T("STRID_CHAT_PANEL_FAVORITES")};
    ui::Label* pPanelTitle = dynamic_cast<ui::Label*>(FindControl(_T("panel_title")));
    if (pPanelTitle != nullptr) {
        const int nIndex = static_cast<int>(m_curNavTab);
        pPanelTitle->SetTextId(titleIds[nIndex]);
    }

}

void ChatForm::SendInputMessage()
{
    ui::RichEdit2* pInput = dynamic_cast<ui::RichEdit2*>(FindControl(_T("input_edit")));
    if (pInput == nullptr) {
        return;
    }
    const DString strText = pInput->GetText().c_str();
    //去掉首尾空白
    const DString strContent(ui::StringUtil::Trim(strText.c_str()).c_str());

    if (strContent.empty()) {
        return;
    }

    AppendMessage(ui::ChatMessageSide::kSelf, strContent, _T("我"));
    UpdateSessionPreview(strContent);

    //清空输入框
    pInput->SetText(_T(""));
    pInput->SetFocus();

    //模拟对方稍后回复
    SimulateReply();
}

void ChatForm::AppendMessage(ui::ChatMessageSide side, const DString& strText, const DString& strAvatarText)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    pMsgList->AddMessage(side, strText, strAvatarText, GetCurrentTimeText());
}

void ChatForm::SimulateReply()
{
    if (!IsCurrentSessionValid()) {
        return;
    }
    const ChatSession& session = m_sessions[m_nCurrentSession];
    //延时1.5秒后回复，模拟真实聊天节奏
    //本类继承自 ui::Window，this 即为窗口对象
    ChatForm* pThis = this;
    const DString strAvatar = session.avatarText;
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, [pThis, strAvatar]() {
        //延时任务执行时窗口可能已被关闭，此时不能再操作控件
        if ((pThis == nullptr) || pThis->IsClosingWnd()) {
            return;
        }
        pThis->AppendMessage(ui::ChatMessageSide::kOther, _T("好的，收到，我看一下再回复你"), strAvatar);
        }, 1500);
}

void ChatForm::UpdateSessionPreview(const DString& strText)
{
    if (!IsCurrentSessionValid()) {
        return;
    }
    m_sessions[m_nCurrentSession].preview = strText;
    m_sessions[m_nCurrentSession].time = GetCurrentTimeText();

    //更新会话列表中对应条目的预览文本
    const DString strName = _T("session_") + std::to_wstring(m_nCurrentSession + 1) + _T("_msg");
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(strName.c_str()));
    if (pLabel != nullptr) {
        pLabel->SetText(strText.c_str());
    }
    const DString strTimeName = _T("session_") + std::to_wstring(m_nCurrentSession + 1) + _T("_time");
    ui::Label* pTimeLabel = dynamic_cast<ui::Label*>(FindControl(strTimeName.c_str()));
    if (pTimeLabel != nullptr) {
        pTimeLabel->SetText(m_sessions[m_nCurrentSession].time.c_str());
    }
}

DString ChatForm::GetCurrentTimeText() const
{
    const std::time_t tNow = std::time(nullptr);
    std::tm tmNow = { 0 };
    localtime_s(&tmNow, &tNow);
    wchar_t szBuf[32] = { 0 };
    swprintf_s(szBuf, 32, _T("%02d:%02d"), tmNow.tm_hour, tmNow.tm_min);
    return DString(szBuf);
}

bool ChatForm::IsCurrentSessionValid() const
{
    return (m_nCurrentSession >= 0) && (m_nCurrentSession < (int)m_sessions.size());
}
