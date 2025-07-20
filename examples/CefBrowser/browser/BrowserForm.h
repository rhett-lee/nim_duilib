#ifndef EXAMPLES_BROWSER_FORM_H_
#define EXAMPLES_BROWSER_FORM_H_

// duilib
#include "duilib/duilib.h"

// CEF
#include "duilib/duilib_cef.h"

class BrowserBox;

namespace ui {
    class TabCtrlItem;
}

/** 离屏模式Cef多标签浏览器窗口
* @copyright (c) 2016, NetEase Inc. All rights reserved
* @author Redrain
* @date 2019/3/20
*/
class BrowserForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
    friend class DragDropManager;
public:
    BrowserForm();
    virtual ~BrowserForm() override;
    
    //覆盖虚函数
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;   

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

public:
    /** 创建BrowserBox对象
    * @param [in] pWindow 关联的窗口
    * @param [in] browserId 浏览器盒子的唯一标识，用于区分不同的标签页
    */
    virtual BrowserBox* CreateBrowserBox(ui::Window* pWindow, std::string browserId);

    /** 在本窗口内创建一个新浏览器盒子
    * @param[in] browserId 浏览器id
    * @param[in] url 初始化URL
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* CreateBox(const std::string& browserId, DString url);

    /** 关闭本窗口内的一个浏览器盒子
     * @param[in] browserId 浏览器id
     */
    bool CloseBox(const std::string& browserId);

    /** 把一个其他窗口内的浏览器盒子附加到本窗口内
    * @param[in] pBrowserBox 浏览器盒子
    * @return bool true 成功，false 失败
    */
    bool AttachBox(BrowserBox* pBrowserBox);

    /** 把本窗口内的一个浏览器盒子脱离窗口
    * @param[in] pBrowserBox 浏览器盒子
    * @return bool true 成功，false 失败
    */
    bool DetachBox(BrowserBox* pBrowserBox);

    /** 获取当前显示的浏览器盒子
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* GetSelectedBox();

    /** 激活并切换到某个浏览器盒子(同时让浏览器窗口激活)
    * @param[in] browserId 浏览器id
    * @return void 无返回值
    */
    void SetActiveBox(const std::string& browserId);

    /** 判断浏览器盒子是否处于激活状态(同时判断浏览器窗口是否被激活)
    * @param[in] pBrowserBox 浏览器盒子
    * @return bool true 是，false 否
    */
    bool IsActiveBox(const BrowserBox* pBrowserBox);

    /** 判断浏览器盒子是否处于激活状态(同时判断浏览器窗口是否被激活)
    * @param[in] browserId 浏览器id
    * @return bool true 是，false 否
    */
    bool IsActiveBox(const DString& browserId);

    /** 获取本窗口内浏览器盒子的总量
    * @return int    总量
    */
    int32_t GetBoxCount() const;

    /** 网站的FavIcon图标下载完成
    */
    void NotifyFavicon(const BrowserBox* pBrowserBox, CefRefPtr<CefImage> image);

    /** 设置某个浏览器对应的标签控件的标题
    * @param[in] browserId 浏览器id
    * @param[in] name 标题
    */
    void SetTabItemName(const DString& browserId, const DString& name);

    /** 设置某个浏览器对应的标签控件的URL
    * @param [in] browserId 浏览器id
    * @param [in] url URL
    */
    void SetURL(const std::string& browserId, const DString& url);

    /** Browser的加载状态发生变化，更新界面
    */
    void OnLoadingStateChange(BrowserBox* pBrowserBox);

public:
    /** 在新标签/新窗口中打开链接
    */
    void OpenLinkUrl(const DString& url, bool bInNewWindow);

protected:
    /** 点击事件
    */
    bool OnClicked(const ui::EventArgs& arg);

    /** 回车事件
    */
    bool OnReturn(const ui::EventArgs& arg);

    /** 处理标签控件的选中消息
    * @param [in] param 消息的相关信息
    * @return bool true 继续传递控件消息，false 停止传递控件消息
    */
    bool OnTabItemSelected(const ui::EventArgs& param);

    /** 处理标签控件的关闭按钮的单击消息
    * @param [in] param 消息的相关信息
    * @param [in] browserId 列表项对应的浏览器id
    * @return bool true 继续传递控件消息，false 停止传递控件消息
    */
    bool OnTabItemClose(const ui::EventArgs& param, const std::string& browserId);

    /** 在本窗口内查找浏览器盒子
    * @param [in] browserId 浏览器id
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* FindBox(const DString& browserId);

    /** 在本窗口内查找标签控件
    * @param [in] browserId 浏览器id
    * @return BrowserBox* 浏览器盒子
    */
    ui::TabCtrlItem* FindTabItem(const DString& browserId);

    /** 切换某个浏览器盒子为显示状态
    * @param [in] browserId 浏览器id
    * @return bool true 成功，false 失败
    */
    bool ChangeToBox(const DString& browserId);

    /** 新建了一个标签
    * @param [in] pTabItem 标签页的接口
    * @param [in] pBrowserBox 网页盒子的接口
    */
    virtual void OnCreateNewTabPage(ui::TabCtrlItem* pTabItem, BrowserBox* pBrowserBox);

    /** 关闭了一个标签
    * @param [in] pBrowserBox 网页盒子的接口
    */
    virtual void OnCloseTabPage(BrowserBox* pBrowserBox);

protected:
    /** 在执行拖拽操作前，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口
    * @param [in] browserId 浏览器id
    */
    bool OnBeforeDragBoxCallback(const DString& browserId);

    /** 在执行拖拽操作后，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口操作结果
    * @param [in] bDropSucceed 浏览器盒子是否被拖拽到了外部
    */
    void OnAfterDragBoxCallback(bool bDropSucceed);

    /** 判断是否要拖拽浏览器盒子
    * @param[in] param 处理浏览器窗口左侧会话合并列表项发送的事件
    * @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
    */
    bool OnProcessTabItemDrag(const ui::EventArgs& param);

    /** 将页面生成位图
    */
    std::shared_ptr<ui::IBitmap> GenerateWebPageBitmap(ui::CefControl* pCefControl);

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] bFromNC true表示这是NC消息（WM_NCMOUSEMOVE）, false 表示是WM_MOUSEMOVE消息
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const ui::UiPoint& pt, uint32_t modifierKey, bool bFromNC, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) override;

protected:
    /** 地址栏控件（用于显示和输入URL）
    */
    ui::RichEdit* m_pEditUrl;

    /** 标签栏
    */
    ui::TabCtrl* m_pTabCtrl;

    /** BrowserBox 管理器
    */
    ui::TabBox* m_pBorwserBoxTab;

    /** 当前激活的BrowserBox接口
    */
    BrowserBox* m_pActiveBrowserBox;

private:
    //处理浏览器盒子拖拽事件
    /** 鼠标左键是否按下
    */
    bool m_bButtonDown;

    /** 当前是否处于拖动状态
    */
    bool m_bDragState;

    /** 鼠标按下时的坐标点
    */
    ui::UiPoint m_oldDragPoint;

    /** 拖动的Browser ID
    */
    DString m_dragingBrowserId;
};

#endif //EXAMPLES_BROWSER_FORM_H_
