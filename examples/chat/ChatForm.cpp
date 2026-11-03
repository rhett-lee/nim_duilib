#include "ChatForm.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/Clipboard.h"
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
    InitSessionData();

    BindNavButtons();
    BindSessionItems();
    BindChatButtons();
    BindSettingsButton();
    BindMessageListMenu();
    BindMessageListEvents();
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

bool ChatForm::OnLanguageChanged()
{
    const bool bRet = ui::WindowImplBase::OnLanguageChanged();
    //XML 里的 text_id 由框架自动刷新；代码生成的会话数据与消息文本需要自己重建
    InitSessionData();
    RefreshSessionList();
    InitMessages();
    //标题栏带有登录用户名，同样需要重建
    ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("chat_title")));
    if ((pTitle != nullptr) && !m_strLoginUser.empty()) {
        const DString strTitle = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_TITLE"));
        pTitle->SetText((strTitle + _T("  -  ") + m_strLoginUser).c_str());
    }
    return bRet;
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
                const DString strEmoji = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_MSG_EMOJI_SMILE")).c_str();
                pEdit->SetText((strText + strEmoji).c_str());
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
        point.y = rect.top;
        ClientToScreen(point);
        ShowSettingsMenu(point);
        return true;
        });
}

void ChatForm::ShowSettingsMenu(const ui::UiPoint& point)
{
    ui::Menu* pMenu = new ui::Menu(this, nullptr);
    pMenu->SetSkinFolder(GetResourcePath().ToString());
    //设置按钮在左下角，菜单改为底边对齐按钮顶边、向上展开，
    //否则菜单会盖住按钮并超出窗口/屏幕底部。
    pMenu->ShowMenu(_T("chat/menu_settings.xml"), point, ui::MenuPopupPosType::LEFT_BOTTOM);
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

void ChatForm::BindMessageListMenu()
{
    //消息列表的右键菜单：使用 ChatMessageList 内置的"复制/全选"菜单
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    //不指定菜单XML时，控件内部使用框架自带的通用编辑菜单（文案已国际化）
    pMsgList->AttachPopupMenu();
}

void ChatForm::BindMessageListEvents()
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    pMsgList->AttachMessageClick([this](const ui::ChatMessageEvent& messageEvent) {
        OnMessageClick(messageEvent);
        });
    pMsgList->AttachMessageDoubleClick([this](const ui::ChatMessageEvent& messageEvent) {
        return OnMessageDoubleClick(messageEvent);
        });
    pMsgList->AttachMessageRightClick([this](const ui::ChatMessageEvent& messageEvent) {
        return OnMessageRightClick(messageEvent);
        });
}

void ChatForm::OnMessageClick(const ui::ChatMessageEvent& messageEvent)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    if ((messageEvent.hitArea == ui::ChatMessageHitArea::kStatus) &&
        (messageEvent.status == ui::ChatMessageStatus::kFailed)) {
        //点击"发送失败，点击重试"：重新走一遍发送流程
        pMsgList->SetMessageStatus(messageEvent.id, ui::ChatMessageStatus::kSending);
        SimulateSendStatus(messageEvent.id, false);
        return;
    }
    if (messageEvent.hitArea == ui::ChatMessageHitArea::kAvatar) {
        ShowTip(_T("STRID_CHAT_TIP_AVATAR_CLICK"));
    }
}

bool ChatForm::OnMessageDoubleClick(const ui::ChatMessageEvent& messageEvent)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return false;
    }
    const ui::ChatMessageList::Message* pMessage = pMsgList->GetMessageAt(messageEvent.nIndex);
    if ((pMessage == nullptr) || pMessage->text.empty()) {
        return false;
    }
    //双击复制该条消息，不再执行默认的"选中该词"
    ui::Clipboard::SetClipboardText(pMessage->text);
    ShowTip(_T("STRID_CHAT_TIP_COPY_MESSAGE"));
    return true;
}

bool ChatForm::OnMessageRightClick(const ui::ChatMessageEvent& messageEvent)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return false;
    }
    //仅在当前没有选区时，才选中该条消息的全文；
    //已有部分选区时保持用户选中的内容，否则右键会把拖选的结果覆盖掉。
    if (!pMsgList->HasSelection()) {
        pMsgList->SelectMessageText(messageEvent.nIndex);
    }
    //返回 false：继续弹出控件绑定的默认右键菜单（复制/全选）
    return false;
}

void ChatForm::ShowTip(const DString& strTextId)
{
    ui::ToastWnd::Show(this,
        ui::GlobalManager::Instance().Lang().GetStringByID(strTextId),
        ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosBottom);
}

void ChatForm::InitSessionData()
{
    auto getText = [](const DString& strId) -> DString {
        return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
        };

    m_sessions.resize(3);
    for (size_t i = 0; i < m_sessions.size(); ++i) {
        const DString strIndex = std::to_wstring(i + 1).c_str();
        m_sessions[i].name = getText(_T("STRID_CHAT_SESSION_NAME_") + strIndex);
        m_sessions[i].time = getText(_T("STRID_CHAT_SESSION_TIME_") + strIndex);
        m_sessions[i].preview = getText(_T("STRID_CHAT_SESSION_MSG_") + strIndex);
        //头像文字取名称首字符，中英文都可用
        m_sessions[i].avatarText = m_sessions[i].name.substr(0, 1);
        m_sessions[i].unread = false;
    }
    m_sessions[2].unread = true;
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
    const DString strAvatarSelf = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_AVATAR_SELF"));
    auto getText = [](const DString& strId) -> DString {
        return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
        };

    if (m_nCurrentSession == 0) {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, getText(_T("STRID_CHAT_MSG_RECV_1")), session.avatarText, strTime);
        //自己发出的消息带状态：演示"已读"
        ui::ChatMessageList::Message sent;
        sent.side = ui::ChatMessageSide::kSelf;
        sent.status = ui::ChatMessageStatus::kRead;
        sent.text = getText(_T("STRID_CHAT_MSG_SENT_1"));
        sent.avatarText = strAvatarSelf;
        sent.time = strTime;
        pMsgList->AddMessage(sent);
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, getText(_T("STRID_CHAT_MSG_RECV_2")), session.avatarText, strTime);
    }
    else if (m_nCurrentSession == 1) {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, getText(_T("STRID_CHAT_MSG_RECV_3")), session.avatarText, strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kSystem, getText(_T("STRID_CHAT_MSG_SYSTEM_1")), _T(""), _T(""));
    }
    else {
        pMsgList->AddMessage(ui::ChatMessageSide::kOther, getText(_T("STRID_CHAT_MSG_RECV_4")), session.avatarText, strTime);
        pMsgList->AddMessage(ui::ChatMessageSide::kSelf, getText(_T("STRID_CHAT_MSG_SENT_2")), strAvatarSelf, strTime);
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

    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    int64_t nMessageId = 0;
    if (pMsgList != nullptr) {
        //新消息先以"发送中"状态插入，随后由 SimulateSendStatus 推进状态
        ui::ChatMessageList::Message message;
        message.side = ui::ChatMessageSide::kSelf;
        message.status = ui::ChatMessageStatus::kSending;
        message.text = strContent;
        message.avatarText = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_AVATAR_SELF"));
        message.time = GetCurrentTimeText();
        nMessageId = pMsgList->AddMessage(message);
    }
    UpdateSessionPreview(strContent);

    //清空输入框
    pInput->SetText(_T(""));
    pInput->SetFocus();

    //以 /fail 开头的文本用于演示"发送失败 + 点击重试"
    const bool bFail = (strContent.size() >= 5) && (strContent.substr(0, 5) == _T("/fail"));
    SimulateSendStatus(nMessageId, bFail);

    //模拟对方稍后回复
    SimulateReply();
}

int64_t ChatForm::AppendMessage(ui::ChatMessageSide side, const DString& strText, const DString& strAvatarText)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return 0;
    }
    return pMsgList->AddMessage(side, strText, strAvatarText, GetCurrentTimeText());
}

void ChatForm::SimulateSendStatus(int64_t nMessageId, bool bFail)
{
    if (nMessageId == 0) {
        return;
    }
    //延时模拟发送过程：先"发送中"，随后变为"已送达"或"发送失败"，已送达后再变"已读"
    ChatForm* pThis = this;
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, [pThis, nMessageId, bFail]() {
        //延时任务执行时窗口可能已被关闭，此时不能再操作控件
        if ((pThis == nullptr) || pThis->IsClosingWnd()) {
            return;
        }
        ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(pThis->FindControl(_T("message_list")));
        if (pMsgList == nullptr) {
            return;
        }
        pMsgList->SetMessageStatus(nMessageId, bFail ? ui::ChatMessageStatus::kFailed
                                                     : ui::ChatMessageStatus::kDelivered);
        if (bFail) {
            return;
        }
        ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, [pThis, nMessageId]() {
            if ((pThis == nullptr) || pThis->IsClosingWnd()) {
                return;
            }
            ui::ChatMessageList* pList = dynamic_cast<ui::ChatMessageList*>(pThis->FindControl(_T("message_list")));
            if (pList != nullptr) {
                pList->SetMessageStatus(nMessageId, ui::ChatMessageStatus::kRead);
            }
            }, 1400);
        }, 700);
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
        //回复文案走多语言资源：取文案放在延时任务里，保证切换语言后回复也用当前语言
        const DString strReply = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_MSG_REPLY_AUTO")).c_str();
        pThis->AppendMessage(ui::ChatMessageSide::kOther, strReply, strAvatar);
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
