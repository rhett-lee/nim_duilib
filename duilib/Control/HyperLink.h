#ifndef UI_CONTROL_HYPER_LINK_H_
#define UI_CONTROL_HYPER_LINK_H_

#include "duilib/Control/Label.h"

namespace ui {

/** 带有超级链接的文字, 如果URL为空的话，可以当作普通的文字按钮使用
*/
class HyperLink : public Label
{
    typedef Label BaseClass;
public:
    explicit HyperLink(Window* pWindow):
        Label(pWindow)
    {
    }
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override { return DUI_CTR_HYPER_LINK; }
    virtual void SetAttribute(const DString& strName, const DString& strValue) override
    {
        if (strName == _T("url")) {
            m_url = strValue;
        }
        else {
            BaseClass::SetAttribute(strName, strValue);
        }
    }

    virtual void Activate(const EventArgs* /*pMsg*/) override
    {
        if (!IsActivatable()) {
            return;
        }
        DString url = m_url.c_str();
        SendEvent(kEventLinkClick, (WPARAM)url.c_str());
    }

public:
    /** 监听超级链接被点击事件
     * @param[in] callback 超级链接被点击后的回调函数
     */
    void AttachLinkClick(const EventCallback& callback) { AttachEvent(kEventLinkClick, callback); }

private:
    /** URL
    */
    UiString m_url;
};

}    // namespace ui

#endif // UI_CONTROL_HYPER_LINK_H_
