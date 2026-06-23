#ifndef UI_CONTROL_HYPER_LINK_H_
#define UI_CONTROL_HYPER_LINK_H_

#include "duilib/Control/Label.h"

namespace ui {

/** 带有超级链接的文字, 如果URL为空的话，可以当作普通的文字按钮使用
*/
class DUILIB_API HyperLink : public Label
{
    typedef Label BaseClass;
public:
    explicit HyperLink(Window* pWindow):
        Label(pWindow),
        m_bShowUrlTooltip(true)
    {
    }
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override { return DUI_CTR_HYPER_LINK; }
    virtual void SetAttribute(const DString& strName, const DString& strValue2) override
    {
        DString strValue = GetExpandVarStrings(strValue2);
        if (strName == _T("url")) {
            SetUrl(strValue);
        }
        else if (strName == _T("show_url_tooltip")) {
            SetShowUrlTooltip(StringUtil::IsValueTrue(strValue));
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
    /** 设置URL
    */
    void SetUrl(const DString& url)
    {
        m_url = url;
        if (IsShowUrlTooltip()) {
            //更新Tooltip
            SetShowUrlTooltip(IsShowUrlTooltip());
        }
    }

    /** 获取URL
    */
    DString GetUrl() const
    {
        return m_url.c_str();
    }

    /** 是否设置URL为tooltip
    */
    void SetShowUrlTooltip(bool bShowUrlTooltip)
    {
        m_bShowUrlTooltip = bShowUrlTooltip;
        if (bShowUrlTooltip) {
            SetToolTipText(GetUrl());
        }
        else {
            SetToolTipText(_T(""));
        }
    }

    /** 是否显示URL为tootip
    */
    bool IsShowUrlTooltip() const
    {
        return m_bShowUrlTooltip;
    }

public:
    /** 监听超级链接被点击事件
     * @param [in] callback 超级链接被点击后的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachLinkClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventLinkClick, callback, callbackID); }

private:
    /** URL
    */
    UiString m_url;

    /** 是否设置URL为tooltip
    */
    bool m_bShowUrlTooltip;
};

}    // namespace ui

#endif // UI_CONTROL_HYPER_LINK_H_
