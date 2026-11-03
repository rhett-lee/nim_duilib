#include "ChatForm.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/Clipboard.h"
#include "duilib/Utils/ToastWnd.h"
#include "duilib/Utils/StringUtil.h"
#include <ctime>
#include <cwctype>

namespace
{
    /** 会话列表在 XML 中预置的条目数量（session_1 ... session_4）
     *
     *  XML 里预留的槽位是固定的：前三个是初始会话，第四个用于"联系人发起的新会话"。
    */
    constexpr int32_t kSessionSlots = 4;

    /** 转小写（用于搜索匹配，中英文都能工作）
    */
    std::wstring ToLower(const std::wstring& strText)
    {
        std::wstring strRet = strText;
        for (wchar_t& ch : strRet) {
            ch = static_cast<wchar_t>(::towlower(ch));
        }
        return strRet;
    }

    /** 大小写不敏感的子串匹配（strKey 为空时视为匹配）
    */
    bool ContainsNoCase(const std::wstring& strText, const std::wstring& strLowerKey)
    {
        if (strLowerKey.empty()) {
            return true;
        }
        return ToLower(strText).find(strLowerKey) != std::wstring::npos;
    }

    /** 拼出会话条目的控件名，如 session_1_name
    */
    DString MakeSessionControlName(int32_t nIndex, const TCHAR* szSuffix)
    {
        const std::wstring strName = _T("session_") + std::to_wstring(nIndex + 1) + szSuffix;
        return DString(strName.c_str());
    }

    /** 按资源 id 取当前语言的文案
    */
    DString GetLangText(const DString& strTextId)
    {
        return ui::GlobalManager::Instance().Lang().GetStringByID(strTextId);
    }

    /** 把会话历史里的消息转换成消息列表的消息对象
    *
     *  历史是唯一数据源，视图里的消息由历史生成，
     *  这样图片、引用、撤回等信息在切换会话后不会丢失。
    */
    ui::ChatMessageList::Message ToListMessage(const ChatMessage& chatMessage)
    {
        ui::ChatMessageList::Message message = chatMessage.message;
        message.avatarImage = chatMessage.avatarImage;
        message.imagePath = chatMessage.imagePath;
        message.imageWidth = chatMessage.imageWidth;
        message.imageHeight = chatMessage.imageHeight;
        message.quoteText = chatMessage.quoteText;
        message.quoteName = chatMessage.quoteName;
        message.quoteId = chatMessage.quoteId;
        message.bRecalled = chatMessage.bRecalled;
        return message;
    }
}

ChatForm::ChatForm() :
    m_curNavTab(NavTab::kChats),
    m_nCurrentSession(0),
    m_nLastMessageId(0)
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
    //会话与联系人数据
    InitSessionData();

    BindNavButtons();
    BindSessionItems();
    BindContactItems();
    BindChatButtons();
    BindSearchBox();
    BindSettingsButton();
    BindMessageListMenu();
    BindMessageListEvents();
    RefreshSessionList();
    RefreshContactList();
    SelectSession(0);
    UpdatePanelVisibility();
}

bool ChatForm::OnLanguageChanged()
{
    const bool bRet = ui::WindowImplBase::OnLanguageChanged();

    //先把当前视图存回历史，避免重建视图时丢掉刚发的消息
    if (IsCurrentSessionValid()) {
        SaveSessionMessages(m_nCurrentSession);
    }

    //会话/联系人名称与消息文案都记录了资源 id，按新语言重新取值
    for (ChatSession& session : m_sessions) {
        if (!session.nameId.empty()) {
            session.name = GetLangText(session.nameId).c_str();
            session.avatarText = session.name.substr(0, 1);
        }
        if (!session.timeId.empty()) {
            session.time = GetLangText(session.timeId).c_str();
        }
        for (ChatMessage& chatMessage : session.history) {
            ApplyMessageText(chatMessage);
        }
        //预览统一由"最后一条消息"派生；会话还没有消息时回退到预览资源 id。
        //两者都要按新语言重新取值，否则未打开过的会话会留着上一种语言的文案。
        if (!session.history.empty()) {
            session.preview = session.history.back().message.text.c_str();
        }
        else if (!session.previewId.empty()) {
            session.preview = GetLangText(session.previewId).c_str();
        }
    }
    for (size_t i = 0; i < m_contacts.size(); ++i) {
        const DString strIndex = std::to_wstring(i + 1).c_str();
        m_contacts[i].name = GetLangText(_T("STRID_CHAT_CONTACT_") + strIndex).c_str();
        m_contacts[i].desc = GetLangText(_T("STRID_CHAT_CONTACT_") + strIndex + _T("_DESC")).c_str();
        m_contacts[i].avatarText = m_contacts[i].name.substr(0, 1);
    }

    RefreshSessionList();
    RefreshContactList();
    LoadSessionMessages(m_nCurrentSession);
    UpdateChatTitle();
    UpdatePanelVisibility();
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
    //会话条目是 XML 里预置的固定槽位，点击后切换到对应的会话
    //注意：kEventClick 只有 Button 一类的控件才会派发，普通 Box 收不到该事件，
    //这里改用鼠标抬起事件（事件会从命中的子控件向上冒泡到条目本身）。
    for (int32_t i = 0; i < kSessionSlots; ++i) {
        const DString strName = MakeSessionControlName(i, _T(""));
        ui::HBox* pItem = dynamic_cast<ui::HBox*>(FindControl(strName.c_str()));
        if (pItem == nullptr) {
            continue;
        }
        pItem->AttachButtonUp([this, i](const ui::EventArgs& /*args*/) {
            SelectSession(i);
            return true;
            });
    }
}

void ChatForm::BindContactItems()
{
    //点击联系人即发起会话：已有同名会话时切过去，否则新建一个会话
    for (size_t i = 0; i < m_contacts.size(); ++i) {
        const DString strName = _T("contact_") + std::to_wstring(i + 1);
        ui::HBox* pItem = dynamic_cast<ui::HBox*>(FindControl(strName.c_str()));
        if (pItem == nullptr) {
            continue;
        }
        const int nContactIndex = static_cast<int>(i);
        pItem->AttachButtonUp([this, nContactIndex](const ui::EventArgs& /*args*/) {
            StartChatWithContact(nContactIndex);
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
                const DString strEmoji = GetLangText(_T("STRID_CHAT_MSG_EMOJI_SMILE"));
                pEdit->SetText((strText + strEmoji).c_str());
                pEdit->SetFocus();
            }
            return true;
            });
    }

    //图片按钮：弹出示例图片的选择菜单，选中后发送一条图片消息
    ui::Button* pImageBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_chat_image")));
    if (pImageBtn != nullptr) {
        pImageBtn->AttachClick([this, pImageBtn](const ui::EventArgs& /*args*/) {
            ui::UiRect rect = pImageBtn->GetPos();
            ui::UiPoint point;
            point.x = rect.left;
            point.y = rect.top;
            ClientToScreen(point);
            ShowImageMenu(point);
            return true;
            });
    }

    //引用按钮：引用最近一条消息并发送
    ui::Button* pQuoteBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_chat_quote")));
    if (pQuoteBtn != nullptr) {
        pQuoteBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            SendQuoteMessage();
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
        {_T("btn_chat_video"),  _T("STRID_CHAT_TIP_VIDEO")},
        {_T("btn_chat_phone"),  _T("STRID_CHAT_TIP_PHONE")},
        {_T("btn_chat_more"),   _T("STRID_CHAT_TIP_MORE")},
        {_T("btn_chat_history"),_T("STRID_CHAT_TIP_HISTORY")},
    };
    for (const ToolButton& item : toolButtons) {
        ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(item.name));
        if (pBtn == nullptr) {
            continue;
        }
        const DString strTipId = item.tipId;
        pBtn->AttachClick([this, strTipId](const ui::EventArgs& /*args*/) {
            ShowTip(strTipId);
            return true;
            });
    }
}

void ChatForm::BindSearchBox()
{
    ui::RichEdit2* pSearch = dynamic_cast<ui::RichEdit2*>(FindControl(_T("search_edit")));
    if (pSearch == nullptr) {
        return;
    }
    //输入即过滤：会话与联系人按名称/预览做大小写不敏感匹配
    pSearch->AttachTextChanged([this](const ui::EventArgs& /*args*/) {
        ui::RichEdit2* pEdit = dynamic_cast<ui::RichEdit2*>(FindControl(_T("search_edit")));
        if (pEdit != nullptr) {
            const DString strText = pEdit->GetText().c_str();
            const DString strTrim(ui::StringUtil::Trim(strText.c_str()).c_str());
            m_strSearchKey = ToLower(strTrim.c_str());
        }
        RefreshSessionList();
        RefreshContactList();
        UpdateSearchEmptyTip();
        return true;
        });

    //搜索图标：点击后把焦点交给搜索框
    ui::Button* pIcon = dynamic_cast<ui::Button*>(FindControl(_T("btn_search_icon")));
    if (pIcon != nullptr) {
        pIcon->AttachClick([pSearch](const ui::EventArgs& /*args*/) {
            pSearch->SetFocus();
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
        SwitchNavTab(NavTab::kContacts);
        ShowTip(_T("STRID_CHAT_TIP_NEW_SESSION"));
        return;
    }
    if (strMenuItemName == _T("menu_contacts")) {
        SwitchNavTab(NavTab::kContacts);
        return;
    }
    if (strMenuItemName == _T("menu_about")) {
        ShowTip(_T("STRID_CHAT_TIP_ABOUT"));
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
    //消息列表的右键菜单：使用示例专用的菜单（复制/全选/撤回）
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    ChatForm* pThis = this;
    //复制/全选由控件内置处理，撤回交给应用层（消息列表本身不知道撤回的业务规则）
    pMsgList->AttachPopupMenu(_T("chat/chat_message_menu.xml"),
        [pThis](const DString& strItemName, size_t /*nItemIndex*/) {
            if (pThis == nullptr) {
                return;
            }
            if (strItemName == _T("chat_menu_recall")) {
                pThis->RecallMessageAtCursor();
                return;
            }
            if (strItemName == _T("chat_menu_quote")) {
                //引用被右键的那条消息
                ui::ChatMessageList* pList = dynamic_cast<ui::ChatMessageList*>(pThis->FindControl(_T("message_list")));
                if (pList != nullptr) {
                    if (pThis->QuoteMessage(pList->GetContextMenuMessageId())) {
                        pThis->ShowTip(_T("STRID_CHAT_TIP_QUOTE_DONE"));
                    }
                }
                return;
            }
            ui::ChatMessageList* pList = dynamic_cast<ui::ChatMessageList*>(pThis->FindControl(_T("message_list")));
            if (pList != nullptr) {
                pList->OnContextMenuAction(strItemName);
            }
        },
        [pThis](ui::Menu* pMenu, ui::Control* /*pSender*/) {
            //菜单是在右键事件派发之前创建的，这里按鼠标位置命中的消息
            //刷新"引用/撤回"的可用状态（引用只对普通消息可用，撤回只对自己发送的消息可用）
            if ((pThis == nullptr) || (pMenu == nullptr)) {
                return;
            }
            ui::ChatMessageList* pList = dynamic_cast<ui::ChatMessageList*>(pThis->FindControl(_T("message_list")));
            if (pList == nullptr) {
                return;
            }
            const int64_t nMessageId = pList->GetContextMenuMessageId();
            const bool bEnableRecall = pThis->IsRecallableMessage(nMessageId);
            const bool bEnableQuote = pThis->IsQuotableMessage(nMessageId);
            struct ItemState
            {
                const TCHAR* itemName;
                const TCHAR* textName;
                const TCHAR* shortcutName;
                bool bEnabled;
            };
            const ItemState items[] = {
                {_T("chat_menu_recall"), _T("chat_menu_recall_text"), _T("chat_menu_recall_shortcut"), bEnableRecall},
                {_T("chat_menu_quote"),  _T("chat_menu_quote_text"),  _T("chat_menu_quote_shortcut"),  bEnableQuote},
            };
            for (const ItemState& item : items) {
                if (item.bEnabled) {
                    //菜单每次都是新建的，默认即为可用状态，这里不需要再"重新启用"一遍
                    continue;
                }
                const TCHAR* names[] = { item.itemName, item.textName, item.shortcutName };
                for (const TCHAR* name : names) {
                    ui::Control* pControl = pMenu->FindControl(name);
                    if (pControl != nullptr) {
                        pControl->SetEnabled(false);
                    }
                }
            }
        });
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
        SimulateSendStatus(messageEvent.id, m_nCurrentSession, false);
        return;
    }
    if (messageEvent.hitArea == ui::ChatMessageHitArea::kAvatar) {
        //点击头像：在文字头像与图片头像之间切换，演示图片头像的用法
        ToggleMessageAvatar(messageEvent.id);
    }
}

bool ChatForm::OnMessageDoubleClick(const ui::ChatMessageEvent& messageEvent)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return false;
    }
    //双击引用块：跳转到被引用的那条消息
    if (messageEvent.hitArea == ui::ChatMessageHitArea::kQuote) {
        const ui::ChatMessageList::Message* pMessage = pMsgList->GetMessageAt(messageEvent.nIndex);
        if ((pMessage != nullptr) && (pMessage->quoteId != 0)) {
            if (JumpToMessage(pMessage->quoteId)) {
                ShowTip(_T("STRID_CHAT_TIP_JUMP_QUOTE"));
                return true;
            }
        }
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

bool ChatForm::OnMessageRightClick(const ui::ChatMessageEvent& /*messageEvent*/)
{
    //右键菜单弹出时，控件已经按鼠标位置处理好选区（没有选区则选中该条消息），
    //这里只演示回调本身：返回 false 表示继续使用控件绑定的默认右键菜单。
    return false;
}

void ChatForm::ShowTip(const DString& strTextId)
{
    ui::ToastWnd::Show(this, GetLangText(strTextId), ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosBottom);
}

void ChatForm::InitSessionData()
{
    //三个初始会话
    m_sessions.clear();
    m_sessions.resize(3);
    for (size_t i = 0; i < m_sessions.size(); ++i) {
        const DString strIndex = std::to_wstring(i + 1).c_str();
        ChatSession& session = m_sessions[i];
        session.nameId = _T("STRID_CHAT_SESSION_NAME_") + strIndex;
        session.timeId = _T("STRID_CHAT_SESSION_TIME_") + strIndex;
        session.name = GetLangText(session.nameId).c_str();
        session.time = GetLangText(session.timeId).c_str();
        session.previewId = _T("STRID_CHAT_SESSION_MSG_") + strIndex;
        session.preview = GetLangText(session.previewId).c_str();
        //会话头像使用图片资源（图片头像与文字头像二选一，这里用图片）
        session.avatarImage = _T("chat_avatar_") + strIndex + _T(".svg");
        session.avatarText = session.name.substr(0, 1);
        session.unread = 0;
        session.seeded = false;
        session.history.clear();
    }
    //第三个会话预置一条未读，用来演示未读角标
    m_sessions[2].unread = 1;

    //三个联系人（其中"王强"没有会话，用来演示点击联系人发起新会话）
    m_contacts.clear();
    m_contacts.resize(3);
    for (size_t i = 0; i < m_contacts.size(); ++i) {
        const DString strIndex = std::to_wstring(i + 1).c_str();
        m_contacts[i].name = GetLangText(_T("STRID_CHAT_CONTACT_") + strIndex).c_str();
        m_contacts[i].desc = GetLangText(_T("STRID_CHAT_CONTACT_") + strIndex + _T("_DESC")).c_str();
        m_contacts[i].avatarText = m_contacts[i].name.substr(0, 1);
    }
    m_nCurrentSession = 0;
}

void ChatForm::RefreshSessionList()
{
    for (int32_t i = 0; i < kSessionSlots; ++i) {
        const DString strItemName = MakeSessionControlName(i, _T(""));
        ui::HBox* pItem = dynamic_cast<ui::HBox*>(FindControl(strItemName.c_str()));
        if (pItem == nullptr) {
            continue;
        }
        const bool bHasSession = (i < static_cast<int32_t>(m_sessions.size()));
        if (!bHasSession) {
            //没有数据的槽位直接隐藏，不占布局空间
            pItem->SetVisible(false);
            continue;
        }
        ChatSession& session = m_sessions[i];
        pItem->SetVisible(IsSessionMatchFilter(session));

        ui::Label* pName = dynamic_cast<ui::Label*>(FindControl(MakeSessionControlName(i, _T("_name")).c_str()));
        if (pName != nullptr) {
            pName->SetText(session.name.c_str());
        }
        ui::Label* pTime = dynamic_cast<ui::Label*>(FindControl(MakeSessionControlName(i, _T("_time")).c_str()));
        if (pTime != nullptr) {
            pTime->SetText(session.time.c_str());
        }
        ui::Label* pMsg = dynamic_cast<ui::Label*>(FindControl(MakeSessionControlName(i, _T("_msg")).c_str()));
        if (pMsg != nullptr) {
            pMsg->SetText(session.preview.c_str());
        }
        ui::Avatar* pAvatar = dynamic_cast<ui::Avatar*>(FindControl(MakeSessionControlName(i, _T("_avatar")).c_str()));
        if (pAvatar != nullptr) {
            pAvatar->SetText(session.avatarText.c_str());
        }
        ui::Badge* pBadge = dynamic_cast<ui::Badge*>(FindControl(MakeSessionControlName(i, _T("_badge")).c_str()));
        if (pBadge != nullptr) {
            //Badge 在数量 <= 0 时会自己隐藏，这里直接同步条数即可
            pBadge->SetCount(session.unread);
        }
    }
}

void ChatForm::RefreshContactList()
{
    for (size_t i = 0; i < m_contacts.size(); ++i) {
        const DString strName = _T("contact_") + std::to_wstring(i + 1);
        ui::HBox* pItem = dynamic_cast<ui::HBox*>(FindControl(strName.c_str()));
        if (pItem == nullptr) {
            continue;
        }
        pItem->SetVisible(IsContactMatchFilter(m_contacts[i]));
    }
}

void ChatForm::UpdateSearchEmptyTip()
{
    bool bAnyMatch = true;
    if (m_curNavTab == NavTab::kChats) {
        bAnyMatch = false;
        for (const ChatSession& session : m_sessions) {
            if (IsSessionMatchFilter(session)) {
                bAnyMatch = true;
                break;
            }
        }
    }
    else if (m_curNavTab == NavTab::kContacts) {
        bAnyMatch = false;
        for (const ChatContact& contact : m_contacts) {
            if (IsContactMatchFilter(contact)) {
                bAnyMatch = true;
                break;
            }
        }
    }
    ui::Label* pTip = dynamic_cast<ui::Label*>(FindControl(_T("search_empty")));
    if (pTip != nullptr) {
        pTip->SetVisible(!m_strSearchKey.empty() && !bAnyMatch);
    }
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
    //过滤结果为空时的提示只在"会话/联系人"两个栏目有意义
    UpdateSearchEmptyTip();
}

void ChatForm::SelectSession(int nSessionIndex)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    //切换前把当前视图的消息存回历史，否则切回来时会丢失
    if (IsCurrentSessionValid()) {
        SaveSessionMessages(m_nCurrentSession);
    }
    m_nCurrentSession = nSessionIndex;

    //进入会话即视为已读
    m_sessions[nSessionIndex].unread = 0;
    LoadSessionMessages(nSessionIndex);
    RefreshSessionList();

    //更新标题栏
    UpdateChatTitle();
}

void ChatForm::UpdateChatTitle()
{
    ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("chat_title")));
    if (pTitle == nullptr) {
        return;
    }
    //标题由两部分组成，两部分都来自多语言资源，切换语言后需要整体重建
    DString strTitle = GetLangText(_T("STRID_CHAT_TITLE"));
    if (IsCurrentSessionValid()) {
        strTitle += _T("  -  ");
        strTitle += m_sessions[m_nCurrentSession].name.c_str();
    }
    pTitle->SetText(strTitle.c_str());
}

void ChatForm::EnsureSessionSeeded(int nSessionIndex)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    ChatSession& session = m_sessions[nSessionIndex];
    if (session.seeded) {
        return;
    }
    session.seeded = true;

    //初始消息全部来自多语言资源，记下资源 id 以便切换语言时重建
    auto addMessage = [this, &session](ui::ChatMessageSide side, const DString& strTextId,
                                       ui::ChatMessageStatus status) {
        ChatMessage chatMessage;
        chatMessage.textId = strTextId;
        //消息 id 由会话历史统一分配：历史是唯一数据源，
        //视图加载历史时沿用同一个 id，引用与撤回才能准确对应到某一条消息
        chatMessage.message.id = GenerateMessageId();
        chatMessage.message.side = side;
        chatMessage.message.type = (side == ui::ChatMessageSide::kSystem) ? ui::ChatMessageType::kSystem
                                                                        : ui::ChatMessageType::kText;
        chatMessage.message.text = GetLangText(strTextId);
        chatMessage.timeId = session.timeId;
        chatMessage.message.avatarText = (side == ui::ChatMessageSide::kSelf)
                                         ? GetLangText(_T("STRID_CHAT_AVATAR_SELF"))
                                         : DString(session.avatarText.c_str());
        //头像图片：自己用通用头像，对方用会话头像
        chatMessage.avatarImage = (side == ui::ChatMessageSide::kSelf)
                                 ? DString(_T("avatar_user.svg")) : session.avatarImage;
        chatMessage.message.time = DString(session.time.c_str());
        chatMessage.message.status = status;
        session.history.push_back(chatMessage);
        };

    if (nSessionIndex == 0) {
        addMessage(ui::ChatMessageSide::kOther, _T("STRID_CHAT_MSG_RECV_1"), ui::ChatMessageStatus::kNone);
        //自己发出的消息带状态：演示"已读"
        addMessage(ui::ChatMessageSide::kSelf, _T("STRID_CHAT_MSG_SENT_1"), ui::ChatMessageStatus::kRead);
        addMessage(ui::ChatMessageSide::kOther, _T("STRID_CHAT_MSG_RECV_2"), ui::ChatMessageStatus::kNone);
    }
    else if (nSessionIndex == 1) {
        addMessage(ui::ChatMessageSide::kOther, _T("STRID_CHAT_MSG_RECV_3"), ui::ChatMessageStatus::kNone);
        addMessage(ui::ChatMessageSide::kSystem, _T("STRID_CHAT_MSG_SYSTEM_1"), ui::ChatMessageStatus::kNone);
    }
    else if (nSessionIndex == 2) {
        addMessage(ui::ChatMessageSide::kOther, _T("STRID_CHAT_MSG_RECV_4"), ui::ChatMessageStatus::kNone);
        addMessage(ui::ChatMessageSide::kSelf, _T("STRID_CHAT_MSG_SENT_2"), ui::ChatMessageStatus::kNone);
    }
    else {
        //由联系人新建的会话：先给一条系统提示，避免空白页
        addMessage(ui::ChatMessageSide::kSystem, _T("STRID_CHAT_MSG_SESSION_START"), ui::ChatMessageStatus::kNone);
    }

    //种子消息生成后，预览与"最后一条消息"保持一致：
    //预览的取值规则只有一条（最后一条消息），切换语言前后才不会变样。
    if (!session.history.empty()) {
        session.preview = session.history.back().message.text.c_str();
    }
}

void ChatForm::LoadSessionMessages(int nSessionIndex)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    pMsgList->ClearMessages();
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    EnsureSessionSeeded(nSessionIndex);

    //把会话历史逐条放回消息列表：消息 id 与状态都会保留
    for (const ChatMessage& chatMessage : m_sessions[nSessionIndex].history) {
        pMsgList->AddMessage(ToListMessage(chatMessage));
    }
    pMsgList->ScrollToBottom();
}

void ChatForm::SaveSessionMessages(int nSessionIndex)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if ((pMsgList == nullptr) || (nSessionIndex < 0) ||
        (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    ChatSession& session = m_sessions[nSessionIndex];

    //消息列表与历史按顺序一一对应（只会追加，不会插入或删除），
    //因此下标对齐即可把视图快照存回历史，同时保留每条消息的文案资源 id。
    //视图为空时不覆盖历史，避免把尚未加载出来的内容清掉。
    const size_t nCount = pMsgList->GetMessageCount();
    if (nCount == 0) {
        return;
    }
    std::vector<ChatMessage> history;
    history.reserve(nCount);
    for (size_t i = 0; i < nCount; ++i) {
        const ui::ChatMessageList::Message* pMessage = pMsgList->GetMessageAt(i);
        if (pMessage == nullptr) {
            continue;
        }
        ChatMessage chatMessage;
        chatMessage.message = *pMessage;
        if (i < session.history.size()) {
            chatMessage.textId = session.history[i].textId;
            chatMessage.timeId = session.history[i].timeId;
            chatMessage.avatarImage = session.history[i].avatarImage;
            chatMessage.imagePath = session.history[i].imagePath;
            chatMessage.imageWidth = session.history[i].imageWidth;
            chatMessage.imageHeight = session.history[i].imageHeight;
            chatMessage.quoteText = session.history[i].quoteText;
            chatMessage.quoteName = session.history[i].quoteName;
            chatMessage.quoteId = session.history[i].quoteId;
            chatMessage.bRecalled = session.history[i].bRecalled;
        }
        history.push_back(chatMessage);
    }
    session.history = history;
}

int64_t ChatForm::AppendMessageToSession(int nSessionIndex, ui::ChatMessageSide side, const DString& strText,
                                         const DString& strAvatarText, const DString& strTextId,
                                         ui::ChatMessageStatus status, const ChatMessageExtra& extra)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return 0;
    }
    EnsureSessionSeeded(nSessionIndex);

    ChatMessage chatMessage;
    chatMessage.textId = strTextId;
    chatMessage.message.side = side;
    chatMessage.message.type = (side == ui::ChatMessageSide::kSystem) ? ui::ChatMessageType::kSystem
                                                                     : ui::ChatMessageType::kText;
    chatMessage.message.text = strText;
    chatMessage.message.avatarText = strAvatarText;
    chatMessage.message.time = GetCurrentTimeText();
    chatMessage.message.status = status;

    //图片消息：带图片路径的消息按图片消息处理
    if (!extra.imagePath.empty()) {
        chatMessage.message.type = ui::ChatMessageType::kImage;
    }
    chatMessage.avatarImage = extra.avatarImage;
    chatMessage.imagePath = extra.imagePath;
    chatMessage.imageWidth = extra.imageWidth;
    chatMessage.imageHeight = extra.imageHeight;
    chatMessage.quoteText = extra.quoteText;
    chatMessage.quoteName = extra.quoteName;
    chatMessage.quoteId = extra.quoteId;

    int64_t nMessageId = 0;
    //消息 id 先由历史分配，视图沿用同一个 id：
    //这样"撤回/引用/改状态"在视图与历史之间不会出现两套 id
    chatMessage.message.id = GenerateMessageId();
    nMessageId = chatMessage.message.id;
    if (nSessionIndex == m_nCurrentSession) {
        //当前会话：直接追加到消息列表，用户马上能看到
        ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
        if (pMsgList != nullptr) {
            pMsgList->AddMessage(ToListMessage(chatMessage));
        }
    }
    m_sessions[nSessionIndex].history.push_back(chatMessage);
    if (nMessageId != 0) {
        m_nLastMessageId = nMessageId;
    }

    //更新会话预览/时间，并刷新列表（非当前会话的未读角标也在这里更新）
    //图片消息没有文本，预览用"[图片]"占位
    DString strPreview = strText;
    if (strPreview.empty() && !extra.imagePath.empty()) {
        strPreview = GetLangText(_T("STRID_CHAT_MSG_IMAGE_PLACEHOLDER"));
    }
    UpdateSessionPreview(nSessionIndex, strPreview);
    return nMessageId;
}

int64_t ChatForm::AppendMessage(ui::ChatMessageSide side, const DString& strText, const DString& strAvatarText,
                                const DString& strTextId, const ChatMessageExtra& extra)
{
    return AppendMessageToSession(m_nCurrentSession, side, strText, strAvatarText, strTextId,
                                  ui::ChatMessageStatus::kNone, extra);
}

void ChatForm::SetHistoryMessageStatus(int nSessionIndex, int64_t nMessageId, ui::ChatMessageStatus status)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size())) || (nMessageId == 0)) {
        return;
    }
    for (ChatMessage& chatMessage : m_sessions[nSessionIndex].history) {
        if (chatMessage.message.id == nMessageId) {
            chatMessage.message.status = status;
            return;
        }
    }
}

void ChatForm::StartChatWithContact(int nContactIndex)
{
    if ((nContactIndex < 0) || (nContactIndex >= static_cast<int>(m_contacts.size()))) {
        return;
    }
    const ChatContact contact = m_contacts[nContactIndex];

    int nSessionIndex = FindSessionByName(contact.name);
    if (nSessionIndex < 0) {
        nSessionIndex = AllocateSessionSlot();
        if (nSessionIndex < 0) {
            ShowTip(_T("STRID_CHAT_TIP_NO_SESSION_SLOT"));
            return;
        }
        ChatSession session;
        session.name = contact.name;
        session.avatarText = contact.avatarText;
        session.nameId = _T("STRID_CHAT_CONTACT_") + std::to_wstring(nContactIndex + 1);
        session.time = GetCurrentTimeText();
        session.unread = 0;
        session.seeded = false;
        if (nSessionIndex < static_cast<int>(m_sessions.size())) {
            m_sessions[nSessionIndex] = session;
        }
        else {
            m_sessions.push_back(session);
        }
        ShowTip(_T("STRID_CHAT_TIP_START_CHAT"));
    }

    //发起会话后切到"会话"栏目，并选中该会话
    SwitchNavTab(NavTab::kChats);
    SelectSession(nSessionIndex);
    RefreshSessionList();
}

int ChatForm::FindSessionByName(const std::wstring& strName) const
{
    for (size_t i = 0; i < m_sessions.size(); ++i) {
        if (m_sessions[i].name == strName) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int ChatForm::AllocateSessionSlot()
{
    //会话在 vector 中始终是连续的，下一个空位就是 size()，同时受 XML 槽位数限制
    const int nIndex = static_cast<int>(m_sessions.size());
    return (nIndex < kSessionSlots) ? nIndex : -1;
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
    if (!IsCurrentSessionValid()) {
        return;
    }

    //演示指令：/img 发送图片消息，/quote 引用上一条消息
    //（工具栏的图片按钮与引用按钮是同一套功能的可视化入口）
    if (strContent == _T("/img")) {
        pInput->SetText(_T(""));
        SendImageMessage(_T("chat_photo_demo.svg"), 160, 100);
        return;
    }
    if (strContent == _T("/quote")) {
        pInput->SetText(_T(""));
        SendQuoteMessage();
        return;
    }

    //新消息先以"发送中"状态插入，随后由 SimulateSendStatus 推进状态
    const int nSessionIndex = m_nCurrentSession;
    const int64_t nMessageId = AppendMessageToSession(
        nSessionIndex, ui::ChatMessageSide::kSelf, strContent,
        GetLangText(_T("STRID_CHAT_AVATAR_SELF")), DString(), ui::ChatMessageStatus::kSending);

    //清空输入框
    pInput->SetText(_T(""));
    pInput->SetFocus();

    //以 /fail 开头的文本用于演示"发送失败 + 点击重试"
    const bool bFail = (strContent.size() >= 5) && (strContent.substr(0, 5) == _T("/fail"));
    SimulateSendStatus(nMessageId, nSessionIndex, bFail);

    //模拟对方稍后回复
    SimulateReply(nSessionIndex);
}

void ChatForm::SendImageMessage(const DString& strImagePath, int32_t nWidth, int32_t nHeight)
{
    if (!IsCurrentSessionValid()) {
        return;
    }
    //演示用：发送一张内置的示例图片
    ChatMessageExtra extra;
    extra.avatarImage = _T("avatar_user.svg");
    extra.imagePath = strImagePath;
    extra.imageWidth = nWidth;
    extra.imageHeight = nHeight;
    AppendMessage(ui::ChatMessageSide::kSelf, DString(), GetLangText(_T("STRID_CHAT_AVATAR_SELF")),
                  DString(), extra);
}

void ChatForm::ShowImageMenu(const ui::UiPoint& point)
{
    ui::Menu* pMenu = new ui::Menu(this, nullptr);
    pMenu->SetSkinFolder(GetResourcePath().ToString());
    //图片按钮在输入区左下角，菜单向上展开，避免超出窗口底部
    pMenu->ShowMenu(_T("chat/chat_image_menu.xml"), point, ui::MenuPopupPosType::LEFT_BOTTOM);
    pMenu->AttachMenuItemActivated([this](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                          const DString& strItemName, size_t /*nItemIndex*/) {
        OnImageMenuItemActivated(strItemName);
        });
}

void ChatForm::OnImageMenuItemActivated(const DString& strMenuItemName)
{
    if (strMenuItemName == _T("chat_image_item_1")) {
        SendImageMessage(_T("chat_photo_demo.svg"), 160, 100);
        return;
    }
    if (strMenuItemName == _T("chat_image_item_2")) {
        SendImageMessage(_T("chat_photo_demo_2.svg"), 160, 100);
        return;
    }
}

void ChatForm::SendQuoteMessage()
{
    if (!IsCurrentSessionValid()) {
        return;
    }
    //引用最近一条可以引用的消息（系统提示与已撤回的消息不参与引用）
    int64_t nQuoteId = 0;
    const ChatSession& session = m_sessions[m_nCurrentSession];
    for (auto it = session.history.rbegin(); it != session.history.rend(); ++it) {
        if (IsQuotableMessage(it->message.id)) {
            nQuoteId = it->message.id;
            break;
        }
    }
    if (nQuoteId == 0) {
        ShowTip(_T("STRID_CHAT_TIP_NO_QUOTE"));
        return;
    }
    if (QuoteMessage(nQuoteId)) {
        ShowTip(_T("STRID_CHAT_TIP_QUOTE_DONE"));
    }
}

bool ChatForm::QuoteMessage(int64_t nMessageId)
{
    if (!IsCurrentSessionValid()) {
        return false;
    }
    int nSessionIndex = -1;
    ChatMessage* pMessage = FindMessage(nMessageId, &nSessionIndex);
    if ((pMessage == nullptr) || (nSessionIndex != m_nCurrentSession)) {
        return false;
    }
    if (!IsQuotableMessage(nMessageId)) {
        return false;
    }
    //被引用的是图片消息时没有文本，用"[图片]"占位
    DString strQuoteText = pMessage->message.text;
    if (strQuoteText.empty()) {
        strQuoteText = GetLangText(_T("STRID_CHAT_MSG_IMAGE_PLACEHOLDER"));
    }
    const DString strQuoteName = (pMessage->message.side == ui::ChatMessageSide::kSelf)
                                 ? GetLangText(_T("STRID_CHAT_AVATAR_SELF"))
                                 : DString(m_sessions[nSessionIndex].name.c_str());

    ChatMessageExtra extra;
    extra.avatarImage = _T("avatar_user.svg");
    extra.quoteText = strQuoteText;
    extra.quoteName = strQuoteName;
    extra.quoteId = nMessageId;
    AppendMessage(ui::ChatMessageSide::kSelf, GetLangText(_T("STRID_CHAT_MSG_QUOTE_BODY")),
                  GetLangText(_T("STRID_CHAT_AVATAR_SELF")), DString(), extra);
    return true;
}

bool ChatForm::IsQuotableMessage(int64_t nMessageId) const
{
    if (nMessageId == 0) {
        return false;
    }
    for (const ChatSession& session : m_sessions) {
        for (const ChatMessage& chatMessage : session.history) {
            if (chatMessage.message.id != nMessageId) {
                continue;
            }
            //系统提示与已撤回的消息不能引用
            if (chatMessage.bRecalled) {
                return false;
            }
            if (chatMessage.message.side == ui::ChatMessageSide::kSystem) {
                return false;
            }
            //文字与图片消息都可以引用，空消息没有可引用的内容
            if (chatMessage.message.text.empty() && chatMessage.imagePath.empty()) {
                return false;
            }
            return true;
        }
    }
    return false;
}

DString ChatForm::GetDefaultAvatarImage(ui::ChatMessageSide side, int nSessionIndex) const
{
    //自己发送的消息用通用头像，对方发来的消息用会话头像
    if (side == ui::ChatMessageSide::kSelf) {
        return DString(_T("avatar_user.svg"));
    }
    if ((nSessionIndex >= 0) && (nSessionIndex < static_cast<int>(m_sessions.size()))) {
        return m_sessions[nSessionIndex].avatarImage;
    }
    return DString();
}

void ChatForm::ToggleMessageAvatar(int64_t nMessageId)
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    const ui::ChatMessageList::Message* pMessage = pMsgList->GetMessageById(nMessageId);
    if (pMessage == nullptr) {
        return;
    }
    int nSessionIndex = -1;
    ChatMessage* pHistory = FindMessage(nMessageId, &nSessionIndex);
    if (pHistory == nullptr) {
        return;
    }

    //当前是图片头像则切回文字头像，当前是文字头像则切换成图片头像
    ui::ChatMessageList::Message newMessage = *pMessage;
    if (newMessage.avatarImage.empty()) {
        newMessage.avatarImage = GetDefaultAvatarImage(newMessage.side, nSessionIndex);
    }
    else {
        newMessage.avatarImage.clear();
    }
    if (newMessage.avatarImage.empty() && pMessage->avatarImage.empty()) {
        //没有可用的图片头像，保持原样
        return;
    }

    //视图
    pMsgList->UpdateMessage(nMessageId, newMessage);
    //历史：切换会话后重新加载时仍保持切换后的头像
    pHistory->avatarImage = newMessage.avatarImage;
    pHistory->message.avatarImage = newMessage.avatarImage;
    ShowTip(_T("STRID_CHAT_TIP_AVATAR_CLICK"));
}

bool ChatForm::RecallMessage(int64_t nMessageId)
{
    if (!IsRecallableMessage(nMessageId)) {
        return false;
    }
    const DString strRecallText = GetLangText(_T("STRID_CHAT_MSG_RECALLED_SELF"));
    //视图：撤回后只显示一行灰色提示文字
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList != nullptr) {
        pMsgList->RecallMessage(nMessageId, strRecallText);
    }
    //历史：同步更新，切换会话后仍然保持撤回状态
    int nSessionIndex = -1;
    ChatMessage* pMessage = FindMessage(nMessageId, &nSessionIndex);
    if (pMessage == nullptr) {
        return false;
    }
    pMessage->bRecalled = true;
    pMessage->message.text = strRecallText;
    pMessage->message.type = ui::ChatMessageType::kText;
    pMessage->quoteText.clear();
    pMessage->quoteName.clear();
    pMessage->quoteId = 0;
    if (nSessionIndex == m_nCurrentSession) {
        //会话预览同步更新
        UpdateSessionPreview(nSessionIndex, strRecallText);
    }
    return true;
}

void ChatForm::RecallMessageAtCursor()
{
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return;
    }
    //菜单项被点击时鼠标已经移到菜单上，要用菜单创建时记录的命中消息，
    //不能按当前鼠标位置去找
    const int64_t nMessageId = pMsgList->GetContextMenuMessageId();
    if (nMessageId == 0) {
        return;
    }
    if (RecallMessage(nMessageId)) {
        ShowTip(_T("STRID_CHAT_TIP_RECALL_DONE"));
    }
}

bool ChatForm::IsRecallableMessage(int64_t nMessageId) const
{
    if (nMessageId == 0) {
        return false;
    }
    for (const ChatSession& session : m_sessions) {
        for (const ChatMessage& chatMessage : session.history) {
            if (chatMessage.message.id != nMessageId) {
                continue;
            }
            //只能撤回自己发送的、尚未撤回的、非系统提示的消息
            if (chatMessage.bRecalled) {
                return false;
            }
            if (chatMessage.message.side != ui::ChatMessageSide::kSelf) {
                return false;
            }
            if (chatMessage.message.type == ui::ChatMessageType::kSystem) {
                return false;
            }
            return true;
        }
    }
    return false;
}

bool ChatForm::JumpToMessage(int64_t nMessageId)
{
    int nSessionIndex = -1;
    ChatMessage* pMessage = FindMessage(nMessageId, &nSessionIndex);
    if ((pMessage == nullptr) || (nSessionIndex < 0)) {
        return false;
    }
    //被引用的消息可能在别的会话里：先切到对应会话，再定位到消息
    if (nSessionIndex != m_nCurrentSession) {
        SelectSession(nSessionIndex);
    }
    ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(FindControl(_T("message_list")));
    if (pMsgList == nullptr) {
        return false;
    }
    return pMsgList->ScrollToMessage(nMessageId);
}

ChatMessage* ChatForm::FindMessage(int64_t nMessageId, int* pSessionIndex)
{
    for (size_t i = 0; i < m_sessions.size(); ++i) {
        for (ChatMessage& chatMessage : m_sessions[i].history) {
            if (chatMessage.message.id != nMessageId) {
                continue;
            }
            if (pSessionIndex != nullptr) {
                *pSessionIndex = static_cast<int>(i);
            }
            return &chatMessage;
        }
    }
    return nullptr;
}

int64_t ChatForm::GenerateMessageId()
{
    //消息 id 全局递增（跨会话唯一），历史与视图共用同一套 id
    static int64_t s_nNextMessageId = 1;
    return s_nNextMessageId++;
}

void ChatForm::SimulateSendStatus(int64_t nMessageId, int nSessionIndex, bool bFail)
{
    if (nMessageId == 0) {
        return;
    }
    //延时模拟发送过程：先"发送中"，随后变为"已送达"或"发送失败"，已送达后再变"已读"
    ChatForm* pThis = this;
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI,
        [pThis, nMessageId, nSessionIndex, bFail]() {
        //延时任务执行时窗口可能已被关闭，此时不能再操作控件
        if ((pThis == nullptr) || pThis->IsClosingWnd()) {
            return;
        }
        const ui::ChatMessageStatus status = bFail ? ui::ChatMessageStatus::kFailed
                                                   : ui::ChatMessageStatus::kDelivered;
        if (pThis->m_nCurrentSession == nSessionIndex) {
            ui::ChatMessageList* pMsgList = dynamic_cast<ui::ChatMessageList*>(
                pThis->FindControl(_T("message_list")));
            if (pMsgList != nullptr) {
                pMsgList->SetMessageStatus(nMessageId, status);
            }
        }
        else {
            //用户已经切到别的会话：状态只更新数据，等切回来时再显示
            pThis->SetHistoryMessageStatus(nSessionIndex, nMessageId, status);
        }
        if (bFail) {
            return;
        }
        ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI,
            [pThis, nMessageId, nSessionIndex]() {
            if ((pThis == nullptr) || pThis->IsClosingWnd()) {
                return;
            }
            if (pThis->m_nCurrentSession == nSessionIndex) {
                ui::ChatMessageList* pList = dynamic_cast<ui::ChatMessageList*>(
                    pThis->FindControl(_T("message_list")));
                if (pList != nullptr) {
                    pList->SetMessageStatus(nMessageId, ui::ChatMessageStatus::kRead);
                }
            }
            else {
                pThis->SetHistoryMessageStatus(nSessionIndex, nMessageId, ui::ChatMessageStatus::kRead);
            }
            }, 1400);
        }, 700);
}

void ChatForm::SimulateReply(int nSessionIndex)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    //延时1.5秒后回复，模拟真实聊天节奏
    //本类继承自 ui::Window，this 即为窗口对象
    ChatForm* pThis = this;
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, [pThis, nSessionIndex]() {
        //延时任务执行时窗口可能已被关闭，此时不能再操作控件
        if ((pThis == nullptr) || pThis->IsClosingWnd()) {
            return;
        }
        if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(pThis->m_sessions.size()))) {
            return;
        }
        //回复文案走多语言资源：取文案放在延时任务里，保证切换语言后回复也用当前语言
        const DString strTextId(_T("STRID_CHAT_MSG_REPLY_AUTO"));
        const DString strReply = GetLangText(strTextId);
        const DString strAvatar(pThis->m_sessions[nSessionIndex].avatarText.c_str());
        pThis->AppendMessageToSession(nSessionIndex, ui::ChatMessageSide::kOther, strReply, strAvatar, strTextId);

        //用户已经切到别的会话：这条回复计入该会话的未读数（会话列表出现角标）
        if (pThis->m_nCurrentSession != nSessionIndex) {
            ++pThis->m_sessions[nSessionIndex].unread;
            pThis->RefreshSessionList();
        }
        }, 1500);
}

void ChatForm::UpdateSessionPreview(int nSessionIndex, const DString& strText)
{
    if ((nSessionIndex < 0) || (nSessionIndex >= static_cast<int>(m_sessions.size()))) {
        return;
    }
    m_sessions[nSessionIndex].preview = strText.c_str();
    m_sessions[nSessionIndex].time = GetCurrentTimeText().c_str();
    RefreshSessionList();
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
    return (m_nCurrentSession >= 0) && (m_nCurrentSession < static_cast<int>(m_sessions.size()));
}

bool ChatForm::IsSessionMatchFilter(const ChatSession& session) const
{
    if (m_strSearchKey.empty()) {
        return true;
    }
    return ContainsNoCase(session.name, m_strSearchKey) || ContainsNoCase(session.preview, m_strSearchKey);
}

bool ChatForm::IsContactMatchFilter(const ChatContact& contact) const
{
    if (m_strSearchKey.empty()) {
        return true;
    }
    return ContainsNoCase(contact.name, m_strSearchKey) || ContainsNoCase(contact.desc, m_strSearchKey);
}

void ChatForm::ApplyMessageText(ChatMessage& chatMessage) const
{
    if (!chatMessage.textId.empty()) {
        chatMessage.message.text = GetLangText(chatMessage.textId);
    }
    //时间戳也可能带语言成分（如"上午 10:24"），同样按新语言重建
    if (!chatMessage.timeId.empty()) {
        chatMessage.message.time = GetLangText(chatMessage.timeId);
    }
}
