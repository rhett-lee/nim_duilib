#include "ToolTip.h"
#include "duilib/Core/WindowBase.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui
{

//ToolTip最大长度
#define TOOLTIP_MAX_LEN 511

//TODO: 暂不支持其他平台的ToolTip功能
class ToolTip::TImpl
{
public:
    TImpl();
    ~TImpl();

public:
    /** 设置鼠标跟踪状态
    * @param [in] pParentWnd 父窗口
    * @param [in] bTracking 是否跟踪鼠标状态
    */
    void SetMouseTracking(const WindowBase* pParentWnd, bool bTracking);

    /**@brief 显示ToolTip信息
    * @param [in] pParentWnd 父窗口
    * @param [in] rect Tooltip显示区域
    * @param [in] maxWidth Tooltip显示最大宽度
    * @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
    */
    void ShowToolTip(const WindowBase* pParentWnd,
                     const UiRect& rect, 
                     uint32_t maxWidth,
                     const UiPoint& trackPos,
                     const DString& text);

    /**@brief 隐藏ToolTip信息
    */
    void HideToolTip();

    /**@brief 清除鼠标跟踪状态
    */
    void ClearMouseTracking();
};

ToolTip::TImpl::TImpl()
{
}

ToolTip::TImpl::~TImpl()
{
}

void ToolTip::TImpl::SetMouseTracking(const WindowBase* /*pParentWnd*/, bool /*bTracking*/)
{
}

void ToolTip::TImpl::ShowToolTip(const WindowBase* /*pParentWnd*/,
                                 const UiRect& /*rect*/,
                                 uint32_t /*maxWidth*/,
                                 const UiPoint& /*trackPos*/,
                                 const DString& /*text*/)
{
}

void ToolTip::TImpl::HideToolTip()
{
}

void ToolTip::TImpl::ClearMouseTracking()
{
}

ToolTip::ToolTip()
{
    m_impl = new TImpl;
}

ToolTip::~ToolTip()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void ToolTip::SetMouseTracking(WindowBase* pParentWnd, bool bTracking)
{
    m_impl->SetMouseTracking(pParentWnd, bTracking);
}

void ToolTip::ShowToolTip(WindowBase* pParentWnd,
                          const UiRect& rect,
                          uint32_t maxWidth,
                          const UiPoint& trackPos,
                          const DString& text)
{
    m_impl->ShowToolTip(pParentWnd, rect, maxWidth, trackPos, text);
}

void ToolTip::HideToolTip()
{
    m_impl->HideToolTip();
}

void ToolTip::ClearMouseTracking()
{
    m_impl->ClearMouseTracking();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
