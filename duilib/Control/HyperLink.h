#ifndef UI_CONTROL_HYPER_LINK_H_
#define UI_CONTROL_HYPER_LINK_H_

#pragma once

#include "duilib/Control/Label.h"

namespace ui {

/** 带有超级链接的文字, 如果URL为空的话，可以当作普通的文字按钮使用
*/
class HyperLink : public Label
{
public:
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override { return DUI_CTR_HYPER_LINK; }
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override
    {
        if (strName == L"url") {
            m_url = strValue;
        }
        else {
            __super::SetAttribute(strName, strValue);
        }
    }

    virtual void Activate() override
    {
        if (!IsActivatable()) {
            return;
        }
        std::wstring url = m_url.c_str();
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

}	// namespace ui

#endif // UI_CONTROL_HYPER_LINK_H_