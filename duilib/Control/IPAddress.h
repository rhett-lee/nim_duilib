#ifndef UI_CONTROL_IPADDRESS_H_
#define UI_CONTROL_IPADDRESS_H_

#include "duilib/Box/HBox.h"

namespace ui
{
/** IP地址控件
*/
class RichEdit;
class DUILIB_API IPAddress: public HBox
{
    typedef HBox BaseClass;
public:
    explicit IPAddress(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点
     */
    virtual void SetFocus() override;

    /** 设置IP地址
    */
    void SetIPAddress(const DString& ipAddress);

    /** 获取IP地址
    */
    DString GetIPAddress() const;

    /** 四段是否均已填写（不校验数值范围）
     */
    bool IsComplete() const;

    /** 是否为完整且合法的IP地址（四段均已填写，且每段为 0~255 的纯数字）
     */
    bool IsValid() const;

    /** 清空IP地址
     */
    void ClearIPAddress();

    /** 是否处于聚焦状态（任一段编辑框获得焦点即为聚焦）
     */
    virtual bool IsFocused() const override;

protected:

    /** 初始化接口
    */
    virtual void OnInit() override;

    /** 将消息派发到消息处理函数
     * @param[in] msg 消息内容
     */
    virtual void SendEventMsg(const EventArgs& msg) override;

    /** 编辑框的焦点转移事件
    */
    void OnKillFocusEvent(RichEdit* pRichEdit, Control* pNewFocus);

    /** 编辑框的文本变化事件
    */
    void OnSegmentTextChanged(size_t index);

    /** 编辑框的按键事件
     * @return 返回 true 表示该按键未被本控件消费，继续交给编辑框默认处理；
     *         返回 false 表示已消费该按键，编辑框不再处理
     */
    bool OnSegmentKeyDown(size_t index, const EventArgs& msg);

    /** 编辑框获取焦点事件
    */
    void OnSegmentSetFocus(size_t index);

private:
    /** 让指定段的编辑框获得焦点
     * @param[in] bSelectAll 是否全选该段的内容
     * @param[in] bCaretAtStart 未全选时，光标是否置于段首（为 false 时置于段尾）
     */
    void FocusSegment(size_t index, bool bSelectAll, bool bCaretAtStart = false);

    /** 静默应用四段数值（不触发文本变化事件）
     * @param[in] values 四段数值（已限制在 0~255）
     */
    void ApplyIPAddress(const int32_t values[4]);

    /** 获取四段编辑框的原始文本（以 '.' 拼接，如 "1..0.1"），用于判断值是否真正变化
    */
    DString GetSegmentText() const;

    /** 刷新各段编辑框的背景色（当前聚焦的段高亮显示）
    */
    void UpdateSegmentBkColor();

    /** 当前IP地址发生变化时，对外发送文本变化事件
    */
    void NotifyValueChanged();

    /** 获取指定段的数值
    */
    int32_t GetSegmentNumber(size_t index) const;

    /** 设置指定段的数值
    */
    void SetSegmentNumber(size_t index, int32_t nValue);

private:
    /** 编辑框列表（由本控件创建并持有，不支持外部调用 RemoveAllItems 移除）
    */
    std::vector<RichEdit*> m_editList;

    /** 上次焦点在哪个控件上
    */
    RichEdit* m_pLastFocus;

    /** IP地址的初始值缓存（仅用于初始化前设置的值，初始化后以各段编辑框的文本为准）
    */
    UiString m_ipAddress;

    /** 上次对外通知的四段原始文本（用于判断值是否真正变化）
    */
    UiString m_lastIPAddress;

    /** 当前聚焦的段索引，-1 表示没有聚焦的段
    */
    int32_t m_nFocusIndex;

    /** 是否允许空值（为 true 时，四段都为空则 GetIPAddress 返回空字符串）
    */
    bool m_bAllowEmpty;

    /** 每段编辑框的宽度（逻辑像素）
    */
    int32_t m_nSegmentWidth;
};

}//namespace ui

#endif //UI_CONTROL_IPADDRESS_H_
