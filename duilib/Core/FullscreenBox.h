#ifndef UI_CORE_FULLSCREEN_BOX_H_
#define UI_CORE_FULLSCREEN_BOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui 
{
class Window;

/** 全屏时的根容器
*/
class UILIB_API FullscreenBox: public Box
{
    typedef Box BaseClass;
public:
    FullscreenBox(Window* pWindow);
    virtual ~FullscreenBox() override;

    //控件类型
    virtual DString GetType() const override;

public:
    /** 进入控件全屏
    * @param [in] pOldRoot 原来的根容器接口
    * @param [in] pFullscreenControl 需要全屏的控件接口
    * @param [in] exitButtonClass 退出全屏按钮的Class名称，如果为空表示不显示退出全屏按钮
    */
    bool EnterControlFullscreen(Box* pOldRoot, Control* pFullscreenControl, const DString& exitButtonClass);

    /** 更新全屏控件(在已经是控件全屏的状态下)
    * @param [in] pFullscreenControl 需要全屏的控件接口
    * @param [in] exitButtonClass 退出全屏按钮的Class名称，如果为空表示不显示退出全屏按钮
    */
    bool UpdateControlFullscreen(Control* pFullscreenControl, const DString& exitButtonClass);

    /** 退出控件全屏
    */
    void ExitControlFullscreen();

    /** 处理全屏按钮的动态显示
    * @param [in] pt 当前鼠标所在位置，客户区坐标
    */
    void ProcessFullscreenButtonMouseMove(const UiPoint& pt);

    /** 获取全屏显示的控件
    * @return 返回全屏显示的控件接口，如果无全屏显示的控件返回nullptr
    */
    Control* GetFullscreenControl() const;

    /** 获取原来的Root容器
    */
    Box* GetOldRoot() const;

    /** 获取窗口原来的状态是最大化
    */
    bool IsWindowOldMaximized() const;

    /** 获取窗口原来的状态是全屏状态
    */
    bool IsWindowOldFullscreen() const;

private:
    /** 将控件从原来的容器中提取出来
    */
    void RemoveControlFromBox(Control* pFullscreenControl);

    /**还原全屏控件到原来的容器
    */
    void RestoreControlToBox();

    /** 更新"退出全屏"按钮
    */
    void UpdateExitFullscreenBtn(const DString& exitButtonClass);

private:
    /** 原来的Root容器
    */
    BoxPtr m_pOldRoot;

private:
    /** 当前全屏状态的控件
    */
    ControlPtr m_pFullscreenControl;

    /** 原来的父控件
    */
    BoxPtr m_pOldParent;

    /** 原来在父控件中的索引号
    */
    size_t m_nOldItemIndex;

    /** 控件原来的外边距
    */
    UiMargin m_rcOldMargin;

private:
    /** 退出全屏按钮
    */
    ControlPtr m_pExitFullscreenBtn;

    /** 退出全屏按钮的Class属性
    */
    DString m_exitButtonClass;

private:
    /** 窗口原来的状态是最大化
    */
    bool m_bWindowMaximized;

    /** 窗口原来的状态是全屏状态
    */
    bool m_bWindowFullscreen;
};

} // namespace ui

#endif // UI_CORE_FULLSCREEN_BOX_H_
