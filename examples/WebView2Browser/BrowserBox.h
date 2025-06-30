#ifndef EXAMPLES_BROWSER_BOX_H_
#define EXAMPLES_BROWSER_BOX_H_

// duilib
#include "duilib/duilib.h"

// WebView2
#include "duilib/duilib_webview2.h"

/** 标签页盒子
*/
class BrowserForm;
class BrowserBox : public ui::VBox
{
    typedef ui::VBox BaseClass;
public:
    friend class BrowserForm;

    /** 构造函数
    * @param [in] pWindow 关联的窗口
    * @param [in] id 浏览器盒子的唯一标识，用于区分不同的标签页
    */
    BrowserBox(ui::Window* pWindow, std::string id);
    virtual ~BrowserBox() override {};

    /**
    * 获取id
    * @return string 标识id
    */
    std::string GetId() const { return m_browserId; }

    /**
    * 获取本浏览器盒子所属的浏览器窗口指针
    * @return BrowserForm* 浏览器窗口
    */
    BrowserForm* GetBrowserForm() const;

    /** 获取WebView2Control控件指针
    */
    ui::WebView2Control* GetWebView2Control();

    /** 获取网页标题
    */
    const DString& GetTitle() const;

    /** 初始化浏览器盒子
    * @param [in] url 初始化URL
    */
    virtual void InitBrowserBox(const DString& url);

    /** 反初始化浏览器盒子
    */
    virtual void UninitBrowserBox();

    //////////////////////////////////////////////////////////////////////////
    //窗口合并功能相关的操作
public:
    /** 控件类型
    */
    virtual DString GetType() const override { return _T("BrowserBox"); }

    /**
    * 覆盖父类虚函数，用于指定本控件所属窗体
    * @param[in] pWindow 所属窗口指针
    */
    virtual void SetWindow(ui::Window* pWindow) override;

protected:
    /** 获取焦点
    */
    virtual bool OnSetFocus(const ui::EventArgs& msg) override;
   
private:
    ui::WebView2Control* m_pWebView2Control;
    BrowserForm* m_pBrowserForm;
    std::string m_browserId;
    DString m_url;
    DString m_title;
};

#endif //EXAMPLES_BROWSER_BOX_H_
