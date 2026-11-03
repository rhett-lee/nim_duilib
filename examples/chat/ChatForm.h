#ifndef EXAMPLES_CHAT_CHATFORM_H_
#define EXAMPLES_CHAT_CHATFORM_H_

// duilib
#include "duilib/duilib.h"

#include <string>
#include <vector>

/** 会话中的一条消息
 *
 *  除消息本身外还记录文案的资源 id：文案来自多语言资源时，切换语言可以重新取值，
 *  这样"按会话保存的历史"不会因为切换语言而变成另一种语言的残留。
 *
 *  图片相关字段与 ChatMessageList::Message 一一对应：
 *  avatarImage 为头像图片路径，imagePath 为图片消息的图片路径，
 *  quoteText/quoteName/quoteId 为引用消息的被引用信息，
 *  bRecalled 表示消息已被撤回。
*/
struct ChatMessage
{
    ui::ChatMessageList::Message message;   //消息内容
    DString textId;                         //文案的资源 id（为空表示不可翻译）
    DString timeId;                         //时间戳的资源 id（为空表示时间不随语言变化）
    DString avatarImage;                    //头像图片路径（为空则用头像文字）
    DString imagePath;                      //图片消息的图片路径
    int32_t imageWidth = 0;                 //图片显示宽度（<=0 表示按原始尺寸）
    int32_t imageHeight = 0;                //图片显示高度（<=0 表示按原始尺寸）
    DString quoteText;                      //引用消息：被引用的文本摘要
    DString quoteName;                      //引用消息：被引用者昵称
    int64_t quoteId = 0;                    //引用消息：被引用的消息 id
    bool bRecalled = false;                 //消息是否已撤回
};

/** 追加消息时携带的附加信息（图片、引用等）
*/
struct ChatMessageExtra
{
    DString avatarImage;        //头像图片路径
    DString imagePath;          //图片消息的图片路径
    int32_t imageWidth = 0;     //图片显示宽度
    int32_t imageHeight = 0;    //图片显示高度
    DString quoteText;          //引用消息：被引用的文本摘要
    DString quoteName;          //引用消息：被引用者昵称
    int64_t quoteId = 0;        //引用消息：被引用的消息 id
};

/** 聊天会话数据（含完整消息历史）
*/
struct ChatSession
{
    std::wstring name;                      //会话名称
    std::wstring avatarText;                //头像文字
    std::wstring preview;                   //最后一条消息预览
    std::wstring time;                      //时间
    DString nameId;                         //名称的资源 id（为空表示名称不随语言变化）
    DString timeId;                         //时间的资源 id（为空表示时间固定）
    DString previewId;                      //预览的资源 id（会话尚未生成消息时用）
    DString avatarImage;                    //会话头像图片路径（为空则用头像文字）
    int32_t unread = 0;                     //未读消息条数（0 表示不显示角标）
    bool seeded = false;                    //初始消息是否已经生成
    std::vector<ChatMessage> history;       //会话消息历史
};

/** 联系人数据
*/
struct ChatContact
{
    std::wstring name;                      //联系人名称
    std::wstring desc;                      //描述
    std::wstring avatarText;                //头像文字
};

/** 聊天主窗口（模拟 PC 版 IM 客户端）
 *
 *  界面结构：左侧导航栏 + 中间列表面板（会话/联系人/收藏可切换） + 右侧聊天区。
 *  消息区使用框架的 ChatMessageList 控件，支持动态追加消息、气泡左右分栏、自动滚动到底部。
 *
 *  数据流：会话数据（ChatSession::history）是唯一数据源，消息列表只是它的视图：
 *  切换会话时先把视图内容存回历史，再把目标会话的历史加载进去，因此每个会话的
 *  消息、发送状态都会保留；联系人点击可发起会话，会话列表顶部的搜索框按名称/预览过滤。
*/
class ChatForm : public ui::WindowImplBase
{
public:
    ChatForm();
    virtual ~ChatForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后初始化
    */
    virtual void OnInitWindow() override;

    /** 语言切换：XML 里的 text_id 由框架刷新，代码生成的文案需要自己重建
    */
    virtual bool OnLanguageChanged() override;

    /** 通知栏目的选中项
    */
    enum class NavTab
    {
        kChats = 0,     //会话
        kContacts = 1,  //联系人
        kFavorites = 2  //收藏
    };

public:
    /** 设置登录用户名称（登录成功后由登录窗口传入）
    */
    void SetLoginUser(const DString& strUser);

private:
    /** 绑定左侧导航按钮
    */
    void BindNavButtons();

    /** 绑定会话条目（点击切换会话）
    */
    void BindSessionItems();

    /** 绑定联系人条目（点击发起会话）
    */
    void BindContactItems();

    /** 绑定聊天区按钮
    */
    void BindChatButtons();

    /** 绑定会话列表顶部的搜索框（输入即过滤）
    */
    void BindSearchBox();

    /** 绑定消息列表的右键菜单（复制/全选）
    */
    void BindMessageListMenu();

    /** 绑定消息列表的点击/双击/右键回调
    */
    void BindMessageListEvents();

    /** 消息列表：点击事件（点击发送失败的状态区域时重发）
    */
    void OnMessageClick(const ui::ChatMessageEvent& messageEvent);

    /** 消息列表：双击事件（复制该条消息文本）
    */
    bool OnMessageDoubleClick(const ui::ChatMessageEvent& messageEvent);

    /** 消息列表：右键事件（先选中该条消息的文本，再弹出默认菜单）
    */
    bool OnMessageRightClick(const ui::ChatMessageEvent& messageEvent);

    /** 初始化会话与联系人数据（文案来自多语言资源，切换语言后需要重新调用）
    */
    void InitSessionData();

    /** 按多语言键弹出提示（Toast）
    */
    void ShowTip(const DString& strTextId);

    /** 绑定左下角的设置按钮，弹出设置菜单
    */
    void BindSettingsButton();

    /** 弹出设置菜单
    */
    void ShowSettingsMenu(const ui::UiPoint& point);

    /** 处理设置菜单中选中的菜单项
    */
    void OnSettingsMenuItemActivated(const DString& strMenuItemName);

    /** 退出程序：关闭主窗口并结束消息循环
    */
    void ExitApp();

    /** 刷新会话列表控件（名称/时间/预览/头像/未读角标），并按搜索关键字过滤
    */
    void RefreshSessionList();

    /** 刷新联系人列表控件的显示状态（按搜索关键字过滤）
    */
    void RefreshContactList();

    /** 更新"没有找到相关结果"的提示
    */
    void UpdateSearchEmptyTip();

    /** 切换导航栏目的选中项
    */
    void SwitchNavTab(NavTab tab);

    /** 刷新列表面板的显示状态
    */
    void UpdatePanelVisibility();

    /** 刷新聊天区标题（"界面演示 - 会话名"，两部分都随语言变化）
    */
    void UpdateChatTitle();

    /** 切换当前会话：保存当前视图到历史，再加载目标会话的历史
    */
    void SelectSession(int nSessionIndex);

    /** 生成会话的初始消息（只生成一次）
    */
    void EnsureSessionSeeded(int nSessionIndex);

    /** 把某个会话的历史加载到消息列表（视图）
    */
    void LoadSessionMessages(int nSessionIndex);

    /** 把消息列表当前内容存回会话历史
    */
    void SaveSessionMessages(int nSessionIndex);

    /** 追加一条消息到指定会话（当前会话直接追加到视图，其他会话只更新数据）
     *  @return 消息 id
    */
    int64_t AppendMessageToSession(int nSessionIndex, ui::ChatMessageSide side, const DString& strText,
                                   const DString& strAvatarText, const DString& strTextId,
                                   ui::ChatMessageStatus status = ui::ChatMessageStatus::kNone,
                                   const ChatMessageExtra& extra = ChatMessageExtra());

    /** 追加一条消息到当前会话
     *  @return 消息 id
    */
    int64_t AppendMessage(ui::ChatMessageSide side, const DString& strText, const DString& strAvatarText,
                          const DString& strTextId, const ChatMessageExtra& extra = ChatMessageExtra());

    /** 撤回一条消息（只能撤回自己发送的消息）
     *
     *  撤回后视图与历史同时更新：气泡变成一行灰色提示文字，且不再参与文本选中。
    */
    bool RecallMessage(int64_t nMessageId);

    /** 撤回鼠标位置命中的消息（右键菜单项"撤回"调用）
    */
    void RecallMessageAtCursor();

    /** 查询某条消息是否可以撤回（自己发送的、未撤回的、非系统提示的消息）
    */
    bool IsRecallableMessage(int64_t nMessageId) const;

    /** 跳转到指定消息（用于双击引用块定位到原消息）
    */
    bool JumpToMessage(int64_t nMessageId);

    /** 查找消息（返回nullptr表示没有找到）
    */
    ChatMessage* FindMessage(int64_t nMessageId, int* pSessionIndex = nullptr);

    /** 生成消息 id（全局唯一，跨会话）
    */
    int64_t GenerateMessageId();

    /** 按 id 修改某个会话历史中的消息状态
    */
    void SetHistoryMessageStatus(int nSessionIndex, int64_t nMessageId, ui::ChatMessageStatus status);

    /** 点击联系人：已有同名会话则切换过去，否则新建会话并发起聊天
    */
    void StartChatWithContact(int nContactIndex);

    /** 查找名称相同的会话（没有时返回 -1）
    */
    int FindSessionByName(const std::wstring& strName) const;

    /** 申请一个空闲的会话条目标题槽位（返回会话索引，没有可用槽位时返回 -1）
    */
    int AllocateSessionSlot();

    /** 发送输入框中的消息
    */
    void SendInputMessage();

    /** 发送一条图片消息（演示用：发送内置的示例图片资源）
    */
    void SendImageMessage();

    /** 引用最近一条消息，再发一条带引用的消息
    */
    void SendQuoteMessage();

    /** 模拟消息状态变化：发送中 → 已送达 → 已读（失败时停在发送失败）
    */
    void SimulateSendStatus(int64_t nMessageId, int nSessionIndex, bool bFail);

    /** 模拟对方回复（回复落在指定会话上，用户已切走时计入该会话的未读数）
    */
    void SimulateReply(int nSessionIndex);

    /** 更新指定会话的预览文本与时间
    */
    void UpdateSessionPreview(int nSessionIndex, const DString& strText);

    /** 获取当前时间文本
    */
    DString GetCurrentTimeText() const;

    /** 当前会话索引是否有效
    */
    bool IsCurrentSessionValid() const;

    /** 会话是否匹配当前搜索关键字
    */
    bool IsSessionMatchFilter(const ChatSession& session) const;

    /** 联系人是否匹配当前搜索关键字
    */
    bool IsContactMatchFilter(const ChatContact& contact) const;

    /** 复制一份消息，并按当前语言刷新其中的文案
    */
    void ApplyMessageText(ChatMessage& chatMessage) const;

private:
    /** 登录用户名称
    */
    DString m_strLoginUser;

    /** 当前选中的导航栏目
    */
    NavTab m_curNavTab;

    /** 会话列表数据
    */
    std::vector<ChatSession> m_sessions;

    /** 联系人列表数据
    */
    std::vector<ChatContact> m_contacts;

    /** 当前会话索引
    */
    int m_nCurrentSession;

    /** 搜索关键字（已转小写；为空表示不过滤）
    */
    std::wstring m_strSearchKey;

    /** 最近一条消息的 id（用于"引用上一条消息"）
    */
    int64_t m_nLastMessageId;
};

#endif // EXAMPLES_CHAT_CHATFORM_H_
