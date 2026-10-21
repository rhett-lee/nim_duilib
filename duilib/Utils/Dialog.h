#ifndef DUILIB_UTILS_DIALOG_H_
#define DUILIB_UTILS_DIALOG_H_

#include "duilib/Utils/WinImplBase.h"

#include <functional>
#include <vector>

namespace ui
{

/** 通用模态对话框
*   功能：
*   1. 内容区可放任意 XML 片段（容器、控件组合），由调用方通过 SetContentXml 指定
*   2. 底部按钮区由调用方自定义（数量、文字、默认按钮），不限于 MessageBoxWnd 的固定组合
*   3. 通过 Window::DoModal 模态显示，阻塞父窗口，返回被点击按钮的索引
*   4. 支持 ESC 键、标题栏关闭按钮取消；默认按钮初始获得焦点，按 Enter 即触发它
*   5. 标题、按钮文字支持多语言 ID（SetTextIdMode）
*   窗口布局由本类在运行时组装（标题栏 + 内容区 + 按钮区），不需要额外的 XML 文件。
*/
class DUILIB_API Dialog : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

public:
    /** 取消/关闭（ESC、标题栏关闭按钮）的返回值
    */
    static constexpr int32_t kResultCancel = -1;

    /** 对话框创建失败的返回值（与取消语义区分开）
    */
    static constexpr int32_t kResultFailed = -2;

public:
    Dialog();
    virtual ~Dialog() override;

    /** 设置窗口标题
    * @param [in] title 标题文字；SetTextIdMode(true) 时表示多语言 ID
    */
    void SetTitle(const DString& title);

    /** 设置文字是否使用多语言 ID（影响标题与按钮文字，默认 false）
    */
    void SetTextIdMode(bool bTextId);

    /** 是否使用多语言 ID
    */
    bool IsTextIdMode() const;

    /** 设置资源目录（相对资源根目录），默认 public
    * @param [in] skinFolder 资源子目录；内容 XML 中的资源路径建议使用资源根目录相对路径
    */
    void SetSkinFolder(const DString& skinFolder);

    /** 设置窗口大小（不含阴影），默认 480 x 360，最小 160 x 120
    * @param [in] nWidth 宽度；小于 160 时按 160 处理
    * @param [in] nHeight 高度；小于 120 时按 120 处理
    */
    void SetSize(int32_t nWidth, int32_t nHeight);

    /** 是否显示标题栏（默认 true）；隐藏后只能通过 ESC 或按钮关闭
    */
    void SetTitleBarVisible(bool bVisible);

    /** 设置内容区的 XML 片段
    * @param [in] contentXml 可包含任意控件/容器，例如 "<VBox>...</VBox>"；
    *                        为空时内容区留空
    */
    void SetContentXml(const DString& contentXml);

    /** 添加一个底部按钮（按添加顺序从左到右排列）
    * @param [in] id 按钮标识，必须非空且唯一；DoModal 返回后可通过 GetClickedButtonId 获取
    * @param [in] text 按钮文字；SetTextIdMode(true) 且该 ID 存在时按多语言 ID 处理
    * @param [in] bDefault 是否为默认按钮（蓝色样式，窗口打开时获得初始焦点）
    * @return true 添加成功；id 为空或与已有按钮重复时返回 false（不添加该按钮）
    */
    bool AddButton(const DString& id, const DString& text, bool bDefault = false);

    /** 设置窗口初始化完成后的回调，用于向内容区动态填充数据
    * @param [in] callback 参数为当前 Dialog 对象
    */
    void AttachInit(std::function<void(Dialog*)> callback);

    /** 按钮点击回调：在关闭对话框之前执行，用于读取内容区数据或校验输入
    * @param [in] callback 参数为当前 Dialog 对象和被点击按钮的索引；
    *                      返回 true 关闭对话框，返回 false 保持打开
    */
    void AttachButtonClick(std::function<bool(Dialog*, int32_t)> callback);

    /** 模态显示对话框
    * @param [in] pParentWindow 父窗口，可为 nullptr
    * @return 被点击按钮的索引（含 Enter 触发的默认按钮）；
    *         取消/关闭返回 kResultCancel；创建失败返回 kResultFailed
    */
    int32_t DoModal(ui::WindowBase* pParentWindow);

    /** 获取被点击按钮的索引（取消/关闭时为 -1）
    */
    int32_t GetClickedButtonIndex() const;

    /** 获取被点击按钮的标识（取消/关闭时为空）
    */
    DString GetClickedButtonId() const;

public:
    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

protected:
    /** 窗口创建完成后：设置标题、绑定按钮事件、设置默认焦点
    */
    virtual void OnInitWindow() override;

    /** 窗口即将关闭时，注销 Enter 键放行注册
    */
    virtual void OnPreCloseWindow() override;

    /** 键盘按下：TAB 导航时开启按钮的焦点矩形显示
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 传给 CloseWnd 的按钮结果编码基数
    *   避免与框架的关闭参数(kWindowCloseNormal/OK/Cancel)冲突；该编码仅内部使用
    */
    static constexpr int32_t kResultButtonBase = 100;

    /** 按钮配置
    */
    struct ButtonItem
    {
        DString id;         //按钮标识
        DString text;       //按钮文字或多语言 ID
        bool bDefault;      //是否为默认按钮
    };

    /** 组装窗口 XML（标题栏 + 内容区 + 按钮区）
    */
    DString BuildWindowXml();

    /** 返回默认按钮的索引：优先标记为默认的按钮，否则返回第一个按钮
    */
    int32_t GetDefaultButtonIndex() const;

    /** XML 属性值转义
    */
    static DString XmlEscape(const DString& text);

private:
    //窗口标题
    DString m_title;

    //内容区 XML 片段
    DString m_contentXml;

    //资源目录
    DString m_skinFolder;

    //按钮列表
    std::vector<ButtonItem> m_buttons;

    //窗口大小
    int32_t m_nWidth;
    int32_t m_nHeight;

    //文字是否为多语言 ID
    bool m_bTextId;

    //是否显示标题栏
    bool m_bTitleBarVisible;

    //初始化回调
    std::function<void(Dialog*)> m_initCallback;

    //按钮点击回调（返回 false 时阻止关闭）
    std::function<bool(Dialog*, int32_t)> m_buttonClickCallback;

    //被点击按钮的索引
    int32_t m_nClickedIndex;
};

}//namespace ui

#endif //DUILIB_UTILS_DIALOG_H_
