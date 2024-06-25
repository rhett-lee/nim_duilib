#ifndef UI_CORE_TOOLTIP_H_
#define UI_CORE_TOOLTIP_H_

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/UiRect.h"
#include <string>

namespace ui 
{
class WindowBase;

/** ToolTip的实现
*/
class UILIB_API ToolTip
{
public:
    ToolTip();
    ToolTip(const ToolTip& r) = delete;
    ToolTip& operator = (const ToolTip& r) = delete;
    ~ToolTip();
    
public:
    /** 设置鼠标跟踪状态
    * @param [in] pParentWnd 父窗口
    * @param [in] bTracking 是否跟踪鼠标状态
    */
    void SetMouseTracking(WindowBase* pParentWnd, bool bTracking);

    /**@brief 显示ToolTip信息
    * @param [in] pParentWnd 父窗口
    * @param [in] rect Tooltip显示区域
    * @param [in] maxWidth Tooltip显示最大宽度
    * @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
    */
    void ShowToolTip(WindowBase* pParentWnd,
                     const UiRect& rect, 
                     uint32_t maxWidth,
                     const UiPoint& trackPos,
                     const DString& text);

    /** 隐藏ToolTip信息
    */
    void HideToolTip();

    /** 清除鼠标跟踪状态
    */
    void ClearMouseTracking();

private:
    //内部实现
    class TImpl;
    TImpl* m_impl;
};

} // namespace ui

#endif // UI_CORE_TOOLTIP_H_
