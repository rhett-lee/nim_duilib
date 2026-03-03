#ifndef UI_CONTROL_CHILD_WINDOW_H_
#define UI_CONTROL_CHILD_WINDOW_H_

#include "duilib/Core/Box.h"
#include "duilib/Control/ChildWindowEvents.h"

namespace ui
{
class ChildWindowImpl;

/** 子窗口控件，控件自身是一个操作系统的子窗口，界面库内部负责子窗口的创建和销毁，但界面库内部不执行子窗口的绘制
 *  由应用层自己负责子窗口的绘制
 *  子窗口的实现：Windows平台的实现为系统原生子窗口（带有WS_CHILD属性）；其他平台为SDL的弹出式窗口，非原生子窗口，SDL不支持原生子窗口
 */
class ChildWindow : public Box
{
    typedef Box BaseClass;
public:
    explicit ChildWindow(Window* pWindow);
    virtual ~ChildWindow() override;
    ChildWindow(const ChildWindow&) = delete;
    ChildWindow& operator=(const ChildWindow&) = delete;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetWindow(Window* pWindow) override;
    virtual void SetPos(UiRect rc) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 创建子窗口
    * @param [in] pChildWindowEvents 子窗口的消息回调接口
    */
    bool CreateChildWindow(ChildWindowEvents* pChildWindowEvents);

    /** 关闭子窗口（同步关闭）
    */
    void CloseChildWindow();

    /** 设置子窗口的消息回调接口
    * @param [in] pChildWindowEvents 子窗口的消息回调接口
    */
    void SetChildWindowEvents(ChildWindowEvents* pChildWindowEvents);

    /** 获取子窗口的外边距
     */
    UiMargin GetChildWindowMargin() const;

    /** 设置子窗口的外边距
     * @param [in] rcMargin 控件的外边距信息
     * @param [in] bNeedDpiScale 是否让外边距根据 DPI 适配，false 不适配 DPI
     */
    void SetChildWindowMargin(UiMargin rcMargin, bool bNeedDpiScale);

    /** 重绘矩形范围
    * @param [in] rect 重绘范围，为客户区坐标
    */
    void InvalidateChildWindowRect(const UiRect& rect);

    /** 重绘整个窗口
    */
    void InvalidateChildWindow();

    /** 更新窗口，执行重绘
    */
    void UpdateChildWindow() const;

    /** 获取子窗口的宽度和高度
    */
    void GetChildWindowRect(UiRect& rect) const;

    /** 设置子窗口是否为分层窗口（分层窗口是带有WS_EX_LAYERED属性的窗口，子窗口仅Windows8及后续平台支持）
     *  使用SDL时无效，SDL不支持动态修改，仅能在创建时运用该属性 
     */
    void SetChildWindowLayered(bool bWindowLayered);

protected:
    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

    /** 设置可用状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetEnabled(bool bChanged) override;

private:
    /** 调整子窗口的位置
    */
    void AdjustChildWindowPos();

    /** 注册子窗口依赖的回调事件
    */
    void RegisterWindowCallbacks(Window* pWindow);

    /** 取消子窗口依赖的回调事件
    */
    void UnregisterWindowCallbacks(Window* pWindow);

private:
    //子窗口的内部实现    
    std::unique_ptr<ChildWindowImpl> m_pChildWnd;

    //子窗口的外边距
    UiMargin m_childWindowMargin;

    //回调函数的ID，用于删除回调函数
    EventCallbackID m_callbackID;
};

}//namespace ui

#endif //UI_CONTROL_CHILD_WINDOW_H_
