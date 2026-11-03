#ifndef EXAMPLES_CHAT_CHATFORM_H_
#define EXAMPLES_CHAT_CHATFORM_H_

// duilib
#include "duilib/duilib.h"

#include <string>
#include <vector>

/** 聊天会话数据
*/
struct ChatSession
{
    std::wstring name;        //会话名称
    std::wstring avatarText;  //头像文字
    std::wstring preview;     //最后一条消息预览
    std::wstring time;        //时间
    bool unread = false;      //是否有未读消息
};

/** 聊天主窗口（模拟 PC 版 IM 客户端）
 *
 *  界面结构：左侧导航栏 + 中间列表面板（会话/联系人/收藏可切换） + 右侧聊天区。
 *  消息区使用框架的 ChatMessageList 控件，支持动态追加消息、气泡左右分栏、自动滚动到底部。
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

    /** 绑定会话条目
    */
    void BindSessionItems();

    /** 绑定聊天区按钮
    */
    void BindChatButtons();

    /** 绑定消息列表的右键菜单（复制/全选）
    */
    void BindMessageListMenu();

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

    /** 把会话数据同步到左侧列表控件（名称/时间/预览/头像）
    */
    void RefreshSessionList();

    /** 初始化消息列表内容
    */
    void InitMessages();

    /** 切换左侧导航选中的栏目
    */
    void SwitchNavTab(NavTab tab);

    /** 刷新列表面板的显示状态
    */
    void UpdatePanelVisibility();

    /** 发送输入框中的消息
    */
    void SendInputMessage();

    /** 追加一条消息到消息列表
    */
    void AppendMessage(ui::ChatMessageSide side, const DString& strText, const DString& strAvatarText);

    /** 模拟对方回复
    */
    void SimulateReply();

    /** 更新当前会话的预览文本与时间
    */
    void UpdateSessionPreview(const DString& strText);

    /** 获取当前时间文本
    */
    DString GetCurrentTimeText() const;

    /** 当前会话索引是否有效
    */
    bool IsCurrentSessionValid() const;

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

    /** 当前会话索引
    */
    int m_nCurrentSession;
};

#endif // EXAMPLES_CHAT_CHATFORM_H_
