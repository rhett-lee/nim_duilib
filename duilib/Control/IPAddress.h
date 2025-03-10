#ifndef UI_CONTROL_IPADDRESS_H_
#define UI_CONTROL_IPADDRESS_H_

#include "duilib/Box/HBox.h"

namespace ui
{
/** IP地址控件
*/
class RichEdit;
class IPAddress: public HBox
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

private:
    /** 编辑框列表
    */
    std::vector<RichEdit*> m_editList;

    /** 上次焦点在哪个控件上
    */
    RichEdit* m_pLastFocus;

    /** IP地址
    */
    UiString m_ipAddress;
};

}//namespace ui

#endif //UI_CONTROL_IPADDRESS_H_
