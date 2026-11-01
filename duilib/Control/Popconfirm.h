#ifndef UI_CONTROL_POPCONFIRM_H_
#define UI_CONTROL_POPCONFIRM_H_

#include "duilib/Control/Popover.h"

namespace ui
{
/** Popconfirm 卡片的图标（语义与 Ant Design Popconfirm 的 icon 属性一致）
 */
enum class PopconfirmIcon : uint8_t
{
    kWarning = 0,   //警告色圆形感叹号（默认）
    kInfo    = 1,   //主题色圆形信息符
    kSuccess = 2,   //成功色圆形对勾
    kError   = 3,   //错误色圆形叉号
    kNone    = 4    //不显示图标
};

/** Popconfirm 中"确定"按钮的样式（语义与 Ant Design Popconfirm 的 okType 属性一致）
 */
enum class PopconfirmOkType : uint8_t
{
    kPrimary = 0,   //主色按钮（默认）
    kDefault = 1,   //次要按钮（与"取消"同外观）
    kDanger  = 2    //危险操作按钮（红色，对应 antd 的 okType="danger"）
};

/** Popconfirm 确认气泡卡片（继承 Popover，锚点自身即触发器）
 *
 *  与 Popover 的差异：Popover 用于"展示信息"，Popconfirm 用于"就地确认"——
 *  卡片由"图标 + 标题 + 描述 + 取消/确定按钮"组成，用户点击按钮后卡片关闭并把结果通知外部。
 *  卡片窗口的方位、箭头、阴影、跟随锚点、hover/focus 触发等能力全部由 Popover 提供。
 *
 *  XML 属性（除 Popover 的全部属性外）：
 *    title          卡片标题（主问句，字面文本；继承自 Popover，可省略）
 *    title_id       卡片标题的多语言 ID（继承自 Popover，优先于 title）
 *    description    卡片描述（补充说明，字面文本，多行用 \\n 分隔，可省略）
 *    description_id 卡片描述的多语言 ID（优先于 description）
 *    icon           图标：warning（默认）/ info / success / error / none
 *    ok_text        "确定"按钮的字面文本（默认取多语言键 STRID_PUBLIC_STRING_OK）
 *    ok_text_id     "确定"按钮的多语言 ID
 *    cancel_text    "取消"按钮的字面文本（默认取多语言键 STRID_PUBLIC_STRING_CANCEL）
 *    cancel_text_id "取消"按钮的多语言 ID
 *    show_cancel    是否显示"取消"按钮，默认 true
 *    ok_type        "确定"按钮样式：primary（默认）/ default / danger
 *    close_on_ok    点击"确定"后是否关闭卡片，默认 true
 *
 *  事件：
 *  - kEventPopconfirmConfirm：点击"确定"按钮（或调用 Confirm()）时触发
 *  - kEventPopconfirmCancel：点击"取消"按钮（或调用 Cancel()）时触发
 *  - kEventPopoverOpenChanged（继承）：打开状态变化；关闭时 lParam 为 PopoverCloseReason，
 *    由按钮触发时为 kConfirm（4）/ kCancel（5）
 *
 *  皮肤类（global.xml 中定义）：popconfirm_icon_warning / popconfirm_icon_info /
 *  popconfirm_icon_success / popconfirm_icon_error / popconfirm_title / popconfirm_description /
 *  popconfirm_btn / popconfirm_cancel / popconfirm_ok_default / popconfirm_ok_primary / popconfirm_ok_danger
 *
 *  说明：不使用继承来的 content / content_id / content_xml（卡片内容由本控件生成），
 *        描述文本请使用 description / description_id。
 */
class DUILIB_API Popconfirm : public Popover
{
    typedef Popover BaseClass;

public:
    explicit Popconfirm(Window* pWindow);
    Popconfirm(const Popconfirm& r) = delete;
    Popconfirm& operator=(const Popconfirm& r) = delete;
    ~Popconfirm() override;

    /** 获取控件类型（DUI_CTR_POPCONFIRM）
     */
    DString GetType() const override;

    /** 设置XML属性
     */
    void SetAttribute(const DString& strName, const DString& strValue) override;

public:
    /** 设置卡片描述（字面文本，多行用 \\n 分隔） */
    void SetDescription(const DString& strDescription);
    /** 获取卡片描述（多语言 ID 优先，否则为字面文本） */
    DString GetDescription() const;

    /** 设置卡片描述（多语言 ID） */
    void SetDescriptionId(const DString& strDescriptionId);
    /** 获取卡片描述的多语言 ID */
    const DString& GetDescriptionId() const { return m_strDescriptionId; }

    /** 设置图标 */
    void SetIcon(PopconfirmIcon icon);
    /** 获取图标 */
    PopconfirmIcon GetIcon() const { return m_icon; }

    /** 设置"确定"按钮的字面文本（空串表示改用多语言 ID） */
    void SetOkText(const DString& strText);
    /** 获取"确定"按钮的文本（多语言 ID 优先，否则为字面文本） */
    DString GetOkText() const;

    /** 设置"确定"按钮的多语言 ID（空串表示改用字面文本） */
    void SetOkTextId(const DString& strTextId);
    /** 获取"确定"按钮的多语言 ID */
    const DString& GetOkTextId() const { return m_strOkTextId; }

    /** 设置"取消"按钮的字面文本（空串表示改用多语言 ID） */
    void SetCancelText(const DString& strText);
    /** 获取"取消"按钮的文本（多语言 ID 优先，否则为字面文本） */
    DString GetCancelText() const;

    /** 设置"取消"按钮的多语言 ID（空串表示改用字面文本） */
    void SetCancelTextId(const DString& strTextId);
    /** 获取"取消"按钮的多语言 ID */
    const DString& GetCancelTextId() const { return m_strCancelTextId; }

    /** 设置是否显示"取消"按钮，默认 true */
    void SetShowCancel(bool bShowCancel);
    /** 是否显示"取消"按钮 */
    bool IsShowCancel() const { return m_bShowCancel; }

    /** 设置"确定"按钮的样式 */
    void SetOkType(PopconfirmOkType okType);
    /** 获取"确定"按钮的样式 */
    PopconfirmOkType GetOkType() const { return m_okType; }

    /** 设置点击"确定"后是否关闭卡片，默认 true
     * @note 设为 false 时卡片保持打开，由业务在 kEventPopconfirmConfirm 回调中自行处理
     */
    void SetCloseOnOk(bool bCloseOnOk);
    /** 点击"确定"后是否关闭卡片 */
    bool IsCloseOnOk() const { return m_bCloseOnOk; }

    /** 等同点击"确定"按钮：派发 kEventPopconfirmConfirm，并按 close_on_ok 关闭卡片
     * @note 卡片未打开时同样派发事件（受控调用场景），但不会关闭任何浮层
     */
    void Confirm();

    /** 等同点击"取消"按钮：派发 kEventPopconfirmCancel，并关闭卡片
     */
    void Cancel();

    /** 监听"确定"事件
     * @param [in] callback 回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachConfirm(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventPopconfirmConfirm, callback, callbackID);
    }

    /** 监听"取消"事件
     * @param [in] callback 回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachCancel(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventPopconfirmCancel, callback, callbackID);
    }

protected:
    /** 构造卡片内容（图标 + 标题 / 描述 / 右对齐按钮行）
     */
    DString BuildCardBodyXml() const override;

    /** 卡片创建完成后绑定"确定"/"取消"按钮的点击事件
     */
    void InitializeCard(Flyout* pFlyout) override;

private:
    /** 追加文本属性（多语言 ID 优先，否则为字面文本；自动转义 XML 特殊字符）
     * @param [in,out] xml 拼接中的 XML 文本
     * @param [in] strTextId 多语言 ID（可为空）
     * @param [in] strText 字面文本（可为空）
     */
    void AppendTextAttribute(DString& xml, const DString& strTextId, const DString& strText) const;

    /** 获取"确定"按钮实际使用的样式类名 */
    DString GetOkClassName() const;

private:
    /** 卡片描述（字面文本 / 多语言 ID） */
    DString m_strDescription;
    DString m_strDescriptionId;

    /** 按钮文案（字面文本 / 多语言 ID；ID 默认取框架语言键） */
    DString m_strOkText;
    DString m_strOkTextId;
    DString m_strCancelText;
    DString m_strCancelTextId;

    /** 图标与"确定"按钮样式 */
    PopconfirmIcon m_icon = PopconfirmIcon::kWarning;
    PopconfirmOkType m_okType = PopconfirmOkType::kPrimary;

    /** 是否显示"取消"按钮 */
    bool m_bShowCancel = true;

    /** 点击"确定"后是否关闭卡片 */
    bool m_bCloseOnOk = true;
};

} // namespace ui

#endif // UI_CONTROL_POPCONFIRM_H_
