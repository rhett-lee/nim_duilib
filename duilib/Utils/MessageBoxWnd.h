#ifndef DUILIB_UTILS_MESSAGE_BOX_WND_H_
#define DUILIB_UTILS_MESSAGE_BOX_WND_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Button.h"

namespace ui
{

/** 自绘皮肤的模态消息框
*   功能：
*   1. 支持 5 种按钮组合：确定 / 确定+取消 / 是+否 / 是+否+取消 / 重试+取消
*   2. 支持 4 种标准图标：信息/警告/错误/疑问，也可无图标
*   3. 通过 Window::DoModal 模态显示，阻塞父窗口，返回用户点击的按钮
*   4. 支持 ESC 键、标题栏关闭按钮取消；Enter 键触发默认按钮
*   5. 消息文本支持多行（自动换行），窗口高度随文本自适应
*   6. 按钮文字允许调用方自定义（多语言场景）
*/
class DUILIB_API MessageBoxWnd : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

public:
    /** 单个按钮的标志位，可按位组合
    */
    enum ButtonFlag : uint32_t
    {
        kButtonOK      = 0x01,  //"确定"按钮
        kButtonCancel  = 0x02,  //"取消"按钮
        kButtonYes     = 0x04,  //"是"按钮
        kButtonNo      = 0x08,  //"否"按钮
        kButtonRetry   = 0x10   //"重试"按钮
    };

    /** 常用按钮组合（与 Win32 MessageBox 的 MB_* 风格对应）
    */
    enum ButtonFlags : uint32_t
    {
        kButtonsOK            = kButtonOK,
        kButtonsOKCancel      = kButtonOK | kButtonCancel,
        kButtonsYesNo         = kButtonYes | kButtonNo,
        kButtonsYesNoCancel   = kButtonYes | kButtonNo | kButtonCancel,
        kButtonsRetryCancel   = kButtonRetry | kButtonCancel
    };

    /** 图标类型
    */
    enum IconType : uint32_t
    {
        kIconNone      = 0,  //无图标
        kIconInfo      = 1,  //信息（蓝色 i）
        kIconWarning   = 2,  //警告（黄色 !）
        kIconError     = 3,  //错误（红色 X）
        kIconQuestion  = 4   //疑问（蓝色 ?）
    };

    /** 模态返回值，取值与 Win32 MessageBox 的 ID* 返回值保持一致：
    *   IDOK=1 IDCANCEL=2 IDRETRY=4 IDYES=6 IDNO=7
    */
    enum Result : int32_t
    {
        kResultOK      = 1,
        kResultCancel  = 2,
        kResultRetry   = 4,
        kResultYes     = 6,
        kResultNo      = 7
    };

    /** 按钮文字自定义（某项为空时使用默认中文文字）
    */
    struct ButtonText
    {
        DString ok;       //确定
        DString cancel;   //取消
        DString yes;      //是
        DString no;       //否
        DString retry;    //重试
    };

public:
    /** 模态显示一个消息框
    * @param [in] pParentWindow 父窗口（模态期间不可操作），可为 nullptr
    * @param [in] text 消息内容，支持 _T('\n') 换行，长文本自动换行
    * @param [in] title 窗口标题
    * @param [in] buttonFlags 按钮组合，参见 enum ButtonFlags
    * @param [in] iconType 图标类型，参见 enum IconType
    * @param [in] pButtonText 自定义按钮文字，可为 nullptr（使用默认文字）
    * @param [in] bTextId 各个文字(text,title,pButtonText)均使用多语言ID，支持多语言切换
    * @return 返回用户选择的按钮（enum Result）；窗口创建失败返回 -1
    */
    static int32_t Show(ui::Window* pParentWindow,
                        const DString& text,
                        const DString& title,
                        uint32_t buttonFlags = kButtonsOK,
                        IconType iconType = kIconNone,
                        const ButtonText* pButtonText = nullptr,
                        bool bTextId = false);

private:
    MessageBoxWnd();
    virtual ~MessageBoxWnd() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后，填充文本/图标并按参数显隐按钮
    */
    virtual void OnInitWindow() override;

    /** 窗口即将关闭时，注销 Enter 键放行注册
    */
    virtual void OnPreCloseWindow() override;

    /** 键盘按下：TAB 导航时按需开启功能按钮的焦点矩形显示（避免初始弹出即显示焦点环）
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 根据按钮组合，初始化按钮显隐、文字、样式与事件
    */
    void InitButtons();

    /** 返回默认按钮的返回值：优先"确定"，其次"是"，最后"重试"
    */
    int32_t GetDefaultResult() const;

    /** 规范化 DoModal 的返回值：
    *   Enter 键返回 kResultOK，但组合中没有"确定"时映射为默认按钮；
    *   ESC 键、标题栏关闭按钮、点击"取消"按钮，统一返回取消语义(kResultCancel)，
    *   即使组合中没有"取消"按钮也不能映射为默认按钮，避免用户放弃选择时触发肯定性动作。
    */
    static int32_t NormalizeResult(int32_t nResult, uint32_t buttonFlags);

private:
    //消息内容
    DString m_text;

    //窗口标题
    DString m_title;

    //按钮组合标志
    uint32_t m_buttonFlags;

    //图标类型
    IconType m_iconType;

    //按钮文字
    ButtonText m_buttonText;

    //消息内容控件
    ui::Label* m_pTextLabel;

    //标题栏文字控件
    ui::Label* m_pTitleLabel;

    //图标控件
    ui::Control* m_pIconControl;

    //各个文字(text, title, pButtonText)均使用多语言ID，支持多语言切换
    bool m_bTextId;
};

} //namespace ui

#endif // DUILIB_UTILS_MESSAGE_BOX_WND_H_
