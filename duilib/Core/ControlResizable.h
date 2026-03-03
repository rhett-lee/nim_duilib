#ifndef UI_CORE_CONTROL_RESIZABLE_H_
#define UI_CORE_CONTROL_RESIZABLE_H_

#include "duilib/Core/ControlMovable.h"

namespace ui
{
/** 主要功能：支持通过鼠标左键按下来调整控件的大小(同时保留基类的可拖动位置功能)
*/
template<typename T = Control>
class UILIB_API ControlResizableT: public ControlMovableT<T>
{
    typedef ControlMovableT<T> BaseClass;
public:
    explicit ControlResizableT(Window* pWindow);
    virtual ~ControlResizableT() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 设置是否支持鼠标拖动改变控件的大小
    */
    void SetEnableResize(bool bEnable);

    /** 获取是否支持鼠标拖动改变控件的大小
    */
    bool IsEnableResize() const;

    /** 获取控件四边可拉伸范围的大小（未经DPI缩放的值）
    */
    const UiRect& GetSizeBox() const;

    /** 设置控件四边可拉伸范围的大小（未经DPI缩放的值）
    * @param [in] rcSizeBox 要设置的大小
    */
    void SetSizeBox(const UiRect& rcSizeBox);

    /** 设置调整大小时，保留的最小宽度(未经DPI缩放)
    */
    void SetResizeReserveWidth(int32_t nResizeReserveWidth);

    /** 获取调整大小时，保留的最小宽度(未经DPI缩放)
    */
    int32_t GetResizeReserveWidth() const;

    /** 设置调整大小时，保留的最小高度(未经DPI缩放)
    */
    void SetResizeReserveHeight(int32_t nResizeReserveHeight);

    /** 获取调整大小时，保留的最小高度(未经DPI缩放)
    */
    int32_t GetResizeReserveHeight() const;

    /** 设置调整控件大小时，是否确保子控件位于父容器内
    */
    void SetResizeKeepWithinParent(bool bKeepWithinParent);

    /** 获取调整控件大小时，是否确保子控件位于父容器内
    */
    bool IsResizeKeepWithinParent() const;

public:
    /** 当前是否处于拖动调整大小的操作中
    */
    bool IsResizing() const;

    /** 取消拖动调整大小操作
    */
    void CancelResizeOperation();

protected:
    /** @name 拖动相关的成员函数
    * @{ */

    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点
    virtual bool OnSetCursor(const EventArgs& msg) override;
    virtual void HandleEvent(const EventArgs& msg) override;

protected:
    /** 清除拖动调整大小的状态，恢复原状态
    */
    virtual void ClearResizeStatus();

    /** @} */

private:
    /** 调整大小的类型
    */
    enum class SizeType : uint8_t
    {
        kNone,
        kLeft,
        kTop,
        kRight,
        kBottom,
        kLeftTop,
        kRightTop,
        kLeftBottom,
        kRightBottom
    };

private:
    /** 鼠标移动时执行的拖动调整控件大小操作
    * @return 返回true表示处于拖动调整大小操作操作中，返回false表示未执行拖动调整大小操作
    */
    bool ResizeMouseMove(const EventArgs& msg);

    /** 鼠标弹起时执行的操作
    */
    void ResizeMouseUp(const EventArgs& msg);

    /** 根据鼠标所在位置确定调整大小的类型
    */
    SizeType GetCurrentSizeType(const UiPoint& ptMouse) const;

    /** 根据调整大小的类型，确定当前光标的样式
    */
    CursorType GetCurrentCursorType(SizeType sizeType) const;

    /** 将控件移动到鼠标所在位置
    */
    bool ResizeControlToPoint(const UiPoint& ptMouse);

    /** 获取调整控件的接口
    */
    Control* GetResizeControl();

    /** 获取调整控件的接口
    */
    const Control* GetResizeControl() const;

private:
    /** 是否支持拖动改变控件的大小(功能开关)
    */
    bool m_bEnableResize;

    /** 调整控件大小时，确保子控件位于父容器内
    */
    bool m_bKeepWithinParent;

    /** 是否鼠标左键按下
    */
    bool m_bMouseDown;

    /** 是否处于拖动调整大小的操作中
    */
    bool m_bResizing;

    /** 调整大小的类型
    */
    SizeType m_sizeType;

    /** 鼠标按下时的鼠标位置
    */
    UiPoint m_ptMouseDown;

    /** 鼠标按下时的控件矩形区域
    */
    UiRect m_rcMouseDown;

    /** 控件四边可拉伸范围信息（未经DPI缩放的值）
    */
    UiRect m_rcSizeBox;

    /** 调整大小时，保留的最小宽度(未经DPI缩放)
    */
    int32_t m_nResizeReserveWidth;

    /** 调整大小时，保留的最小高度(未经DPI缩放)
    */
    int32_t m_nResizeReserveHeight;
};

template<typename T>
ControlResizableT<T>::ControlResizableT(Window* pWindow):
    ControlMovableT<T>(pWindow),
    m_bEnableResize(true),
    m_bKeepWithinParent(false),
    m_bResizing(false),
    m_bMouseDown(false),
    m_sizeType(SizeType::kNone),
    m_nResizeReserveWidth(10),
    m_nResizeReserveHeight(10)
{
    //默认应为浮动控件，并保存控件位置
    this->SetKeepFloatPos(true);
    this->SetFloat(true);

    //默认禁止拖动控件位置功能
    this->SetEnableMovePos(false);
}

template<typename T>
ControlResizableT<T>::~ControlResizableT()
{
}

template<typename T>
inline DString ControlResizableT<T>::GetType() const { return DUI_CTR_CONTROL_RESIZABLE; }

template<>
inline DString ControlResizableT<Box>::GetType() const { return DUI_CTR_BOX_RESIZABLE; }

template<>
inline DString ControlResizableT<HBox>::GetType() const { return DUI_CTR_HBOX_RESIZABLE; }

template<>
inline DString ControlResizableT<VBox>::GetType() const { return DUI_CTR_VBOX_RESIZABLE; }

template<typename T>
void ControlResizableT<T>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("enable_resize")) {
        SetEnableResize(strValue == _T("true"));
    }
    else if (strName == _T("resize_size_box")) {
        UiRect rcSizeBox;
        AttributeUtil::ParseRectValue(strValue.c_str(), rcSizeBox, false);
        SetSizeBox(rcSizeBox);
    }
    else if (strName == _T("resize_reserve_width")) {
        SetResizeReserveWidth(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("resize_reserve_height")) {
        SetResizeReserveHeight(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("resize_keep_within_parent")) {
        SetResizeKeepWithinParent(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename T>
void ControlResizableT<T>::SetEnableResize(bool bEnable)
{
    m_bEnableResize = bEnable;
}

template<typename T>
bool ControlResizableT<T>::IsEnableResize() const
{
    return m_bEnableResize;
}

template<typename T>
const UiRect& ControlResizableT<T>::GetSizeBox() const
{
    return m_rcSizeBox;
}

template<typename T>
void ControlResizableT<T>::SetSizeBox(const UiRect& rcSizeBox)
{
    m_rcSizeBox = rcSizeBox;
    m_rcSizeBox.left = std::max(m_rcSizeBox.left, 0);
    m_rcSizeBox.top = std::max(m_rcSizeBox.top, 0);
    m_rcSizeBox.right = std::max(m_rcSizeBox.right, 0);
    m_rcSizeBox.bottom = std::max(m_rcSizeBox.bottom, 0);

    //同步到基类，可调整大小的边框部分，不可以拖动控件
    UiMargin rcNonDraggableMargin;
    rcNonDraggableMargin.left = m_rcSizeBox.left;
    rcNonDraggableMargin.top = m_rcSizeBox.top;
    rcNonDraggableMargin.right = m_rcSizeBox.right;
    rcNonDraggableMargin.bottom = m_rcSizeBox.bottom;
    this->SetNonDraggableMargin(rcNonDraggableMargin);
}

template<typename T>
void ControlResizableT<T>::SetResizeReserveWidth(int32_t nResizeReserveWidth)
{
    m_nResizeReserveWidth = nResizeReserveWidth;
    if (m_nResizeReserveWidth < 0) {
        m_nResizeReserveWidth = 0;
    }
}

template<typename T>
int32_t ControlResizableT<T>::GetResizeReserveWidth() const
{
    return m_nResizeReserveWidth;
}

template<typename T>
void ControlResizableT<T>::SetResizeReserveHeight(int32_t nResizeReserveHeight)
{
    m_nResizeReserveHeight = nResizeReserveHeight;
    if (m_nResizeReserveHeight < 0) {
        m_nResizeReserveHeight = 0;
    }
}

template<typename T>
int32_t ControlResizableT<T>::GetResizeReserveHeight() const
{
    return m_nResizeReserveHeight;
}

template<typename T>
void ControlResizableT<T>::SetResizeKeepWithinParent(bool bKeepWithinParent)
{
    m_bKeepWithinParent = bKeepWithinParent;
}

template<typename T>
bool ControlResizableT<T>::IsResizeKeepWithinParent() const
{
    return m_bKeepWithinParent;
}

template<typename T>
bool ControlResizableT<T>::IsResizing() const
{
    return m_bResizing;
}

template<typename T>
void ControlResizableT<T>::CancelResizeOperation()
{
    ClearResizeStatus();
}

template<typename T>
Control* ControlResizableT<T>::GetResizeControl()
{
    return this;
}

template<typename T>
const Control* ControlResizableT<T>::GetResizeControl() const
{
    return this;
}

template<typename T>
bool ControlResizableT<T>::ButtonDown(const EventArgs& msg)
{
    m_bMouseDown = false;
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (this->IsMovingPos()) {
        //当前正在处于控件移动操作中
        ClearResizeStatus();
        return bRet;
    }
    if (!IsEnableResize() || GetSizeBox().IsZero()) {
        //功能关闭
        return bRet;
    }

    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    m_bMouseDown = true;
    m_ptMouseDown = pt;

    m_rcMouseDown = GetResizeControl()->GetRect();
    return bRet;
}

template<typename T>
bool ControlResizableT<T>::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (this->IsMovingPos()) {
        //当前正在处于控件移动操作中
        ClearResizeStatus();
        return bRet;
    }
    if (!IsEnableResize() || GetSizeBox().IsZero()) {
        //功能关闭
        return bRet;
    }
    if (m_bMouseDown) {
        //鼠标按下时，调整控件大小
        ResizeMouseMove(msg);
    }
    else {
        //鼠标未按下时，改变光标样式，确定调整大小的类型
        m_sizeType = GetCurrentSizeType(msg.ptMouse);
    }
    return bRet;
}

template<typename T>
bool ControlResizableT<T>::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (IsEnableResize() && !GetSizeBox().IsZero() && IsResizing()) {
        ResizeMouseUp(msg);
    }
    ClearResizeStatus();
    return bRet;
}

template<typename T>
bool ControlResizableT<T>::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnWindowKillFocus(msg);
    ClearResizeStatus();
    return bRet;
}

template<typename T>
bool ControlResizableT<T>::OnSetCursor(const EventArgs& msg)
{
    if (m_sizeType == SizeType::kNone) {
        return BaseClass::OnSetCursor(msg);
    }
    else {
        CursorType cursorType = GetCurrentCursorType(m_sizeType);
        this->SetCursor(cursorType);
        return true;
    }
}

template<typename T>
void ControlResizableT<T>::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if (msg.IsSenderExpired()) {
        return;
    }
    if ((msg.eventType == kEventMouseClickChanged) ||
        (msg.eventType == kEventMouseClickEsc) ||
        (msg.eventType == kEventMouseRButtonDown)) {
        //取消操作
        ClearResizeStatus();
    }
}

template<typename T>
void ControlResizableT<T>::ClearResizeStatus()
{
    //恢复状态
    m_bResizing = false;
    m_bMouseDown = false;
    m_sizeType = SizeType::kNone;
}

template<typename T>
bool ControlResizableT<T>::ResizeMouseMove(const EventArgs& msg)
{
    if (!IsEnableResize() || GetSizeBox().IsZero()) {
        //该控件禁止拖动调整大小
        return false;
    }
    if (!m_bMouseDown) {
        return false;
    }

    //将控件移动到鼠标所在位置
    if (!ResizeControlToPoint(msg.ptMouse)) {
        return false;
    }

    if (!m_bResizing) {
        m_bResizing = true;
    }
    return m_bResizing;
}

template<typename T>
void ControlResizableT<T>::ResizeMouseUp(const EventArgs& msg)
{
    if (!IsEnableResize() || GetSizeBox().IsZero() || !IsResizing()) {
        return;
    }
    ResizeControlToPoint(msg.ptMouse);
    ClearResizeStatus();
}

template<typename T>
typename ControlResizableT<T>::SizeType ControlResizableT<T>::GetCurrentSizeType(const UiPoint& ptMouse) const
{
    UiPoint pt(ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());
    UiRect rcClient = GetResizeControl()->GetRect();
    UiRect rcSizeBox = GetSizeBox();
    this->Dpi().ScaleRect(rcSizeBox);
    if (pt.y < rcClient.top + rcSizeBox.top) {
        if (pt.y >= rcClient.top) {
            if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                return SizeType::kLeftTop;//在边框的左上角
            }
            else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                return SizeType::kRightTop;//在边框的右上角
            }
            else {
                return SizeType::kTop;//在上水平边框中
            }
        }
        else {
            return SizeType::kNone;//在工作区中
        }
    }
    else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
        if (pt.y <= rcClient.bottom) {
            if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                return SizeType::kLeftBottom;//在边框的左下角
            }
            else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                return SizeType::kRightBottom;//在边框的右下角
            }
            else {
                return SizeType::kBottom;//在下水平边框中
            }
        }
        else {
            return SizeType::kNone;//在工作区中
        }
    }

    if (pt.x < rcClient.left + rcSizeBox.left) {
        if (pt.x >= rcClient.left) {
            return SizeType::kLeft;//在左边框
        }
        else {
            return SizeType::kNone;//在工作区中
        }
    }
    if (pt.x > rcClient.right - rcSizeBox.right) {
        if (pt.x <= rcClient.right) {
            return SizeType::kRight;//在右边框中
        }
        else {
            return SizeType::kNone;//在工作区中
        }
    }
    return SizeType::kNone;//在工作区中
}

template<typename T>
CursorType ControlResizableT<T>::GetCurrentCursorType(SizeType sizeType) const
{
    CursorType cursorType = CursorType::kCursorArrow;
    switch (sizeType) {
    case SizeType::kTop://上边框
        cursorType = CursorType::kCursorSizeNS;
        break;
    case SizeType::kBottom://下边框
        cursorType = CursorType::kCursorSizeNS;
        break;
    case SizeType::kLeft://左边框
        cursorType = CursorType::kCursorSizeWE;
        break;
    case SizeType::kRight://右边框
        cursorType = CursorType::kCursorSizeWE;
        break;
    case SizeType::kLeftTop: //左上角
        cursorType = CursorType::kCursorSizeNWSE;
        break;
    case SizeType::kRightTop://右上角
        cursorType = CursorType::kCursorSizeNESW;
        break;
    case SizeType::kLeftBottom://左下角
        cursorType = CursorType::kCursorSizeNESW;
        break;
    case SizeType::kRightBottom://右下角
        cursorType = CursorType::kCursorSizeNWSE;
        break;
    default:
        break;
    }
    return cursorType;
}

template<typename T>
bool ControlResizableT<T>::ResizeControlToPoint(const UiPoint& ptMouse)
{
    UiPoint pt(ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    int32_t xOffset = pt.x - m_ptMouseDown.x;
    int32_t yOffset = pt.y - m_ptMouseDown.y;
    
    Control* pControl = GetResizeControl();
    UiRect rcNewPos = m_rcMouseDown;

    bool bSizeTypeLeft = false;
    bool bSizeTypeTop = false;
    bool bSizeTypeRight = false;
    bool bSizeTypeBottom = false;

    switch (m_sizeType) {
    case SizeType::kTop://上边框
        bSizeTypeTop = true;
        break;
    case SizeType::kBottom://下边框
        bSizeTypeBottom = true;
        break;
    case SizeType::kLeft://左边框
        bSizeTypeLeft = true;
        break;
    case SizeType::kRight://右边框
        bSizeTypeRight = true;
        break;
    case SizeType::kLeftTop: //左上角
        bSizeTypeLeft = true;
        bSizeTypeTop = true;
        break;
    case SizeType::kRightTop://右上角
        bSizeTypeRight = true;
        bSizeTypeTop = true;
        break;
    case SizeType::kLeftBottom://左下角
        bSizeTypeLeft = true;
        bSizeTypeBottom = true;
        break;
    case SizeType::kRightBottom://右下角
        bSizeTypeRight = true;
        bSizeTypeBottom = true;
        break;
    default:
        break;
    }

    int32_t nMinWidth = GetResizeReserveWidth();
    this->Dpi().ScaleInt(nMinWidth);
    nMinWidth = std::max(nMinWidth, pControl->GetMinWidth());
    nMinWidth = std::max(nMinWidth, 0);

    int32_t nMinHeight = GetResizeReserveHeight();
    this->Dpi().ScaleInt(nMinHeight);
    nMinHeight = std::max(nMinHeight, pControl->GetMinHeight());
    nMinHeight = std::max(nMinHeight, 0);

    int32_t nMaxWidth = pControl->GetMaxWidth();
    int32_t nMaxHeight = pControl->GetMaxHeight();
    if (nMaxWidth <= nMinWidth) {
        nMaxWidth = INT32_MAX;
    }
    if (nMaxHeight <= nMinHeight) {
        nMaxHeight = INT32_MAX;
    }

    Control* pParent = nullptr;
    if (IsResizeKeepWithinParent()) {
        pParent = pControl->GetParent();
    }
    UiRect rcParent;
    if (pParent != nullptr) {
        rcParent = pParent->GetRect();
        rcParent.Deflate(pParent->GetPadding());// 剪去内边距
    }

    if (bSizeTypeLeft) {
        //左侧
        rcNewPos.left += xOffset;
        if (rcNewPos.left > rcNewPos.right) {
            rcNewPos.left = rcNewPos.right;
        }
        if (xOffset > 0) {
            //变小
            if (rcNewPos.Width() < nMinWidth) {
                rcNewPos.left = rcNewPos.right - nMinWidth;
            }
        }
        else if (xOffset < 0) {
            //变大
            if (rcNewPos.Width() > nMaxWidth) {
                rcNewPos.left = rcNewPos.right - nMaxWidth;
            }
        }

        if (IsResizeKeepWithinParent() && (pParent != nullptr)) {
            //严格限制在父容器内，不允许溢出
            if (xOffset < 0) {
                if (rcNewPos.left < rcParent.left) {
                    rcNewPos.left = rcParent.left;
                    rcNewPos.Validate();
                }
            }
        }
    }
    if (bSizeTypeRight) {
        //右侧
        rcNewPos.right += xOffset;
        if (rcNewPos.right < rcNewPos.left) {
            rcNewPos.right = rcNewPos.left;
        }
        if (xOffset > 0) {
            //变大
            if (rcNewPos.Width() > nMaxWidth) {
                rcNewPos.right = rcNewPos.left + nMaxWidth;
            }
        }
        else if (xOffset < 0) {
            //变小
            if (rcNewPos.Width() < nMinWidth) {
                rcNewPos.right = rcNewPos.left + nMinWidth;
            }            
        }
        if (IsResizeKeepWithinParent() && (pParent != nullptr)) {
            //严格限制在父容器内，不允许溢出
            if (xOffset > 0) {
                if (rcNewPos.right > rcParent.right) {
                    rcNewPos.right = rcParent.right;
                    rcNewPos.Validate();
                }
            }
        }
    }
    if (bSizeTypeTop) {
        //上侧
        rcNewPos.top += yOffset;
        if (rcNewPos.top > rcNewPos.bottom) {
            rcNewPos.top = rcNewPos.bottom;
        }
        if (yOffset > 0) {
            //变小
            if (rcNewPos.Height() < nMinHeight) {
                rcNewPos.top = rcNewPos.bottom - nMinHeight;
            }
        }
        else if (yOffset < 0) {
            //变大
            if (rcNewPos.Height() > nMaxHeight) {
                rcNewPos.top = rcNewPos.bottom - nMaxHeight;
            }
        }
        if (IsResizeKeepWithinParent() && (pParent != nullptr)) {
            //严格限制在父容器内，不允许溢出
            if (xOffset < 0) {
                if (rcNewPos.top < rcParent.top) {
                    rcNewPos.top = rcParent.top;
                    rcNewPos.Validate();
                }
            }
        }
    }
    if (bSizeTypeBottom) {
        //下侧
        rcNewPos.bottom += yOffset;
        if (rcNewPos.bottom < rcNewPos.top) {
            rcNewPos.bottom = rcNewPos.top;
        }
        if (yOffset > 0) {
            //变大
            if (rcNewPos.Height() > nMaxHeight) {
                rcNewPos.bottom = rcNewPos.top + nMaxHeight;
            }
        }
        else if (yOffset < 0) {
            //变小
            if (rcNewPos.Height() < nMinHeight) {
                rcNewPos.bottom = rcNewPos.top + nMinHeight;
            }
        }
        if (IsResizeKeepWithinParent() && (pParent != nullptr)) {
            //严格限制在父容器内，不允许溢出
            if (xOffset > 0) {
                if (rcNewPos.bottom > rcParent.bottom) {
                    rcNewPos.bottom = rcParent.bottom;
                    rcNewPos.Validate();
                }
            }
        }
    }
    if (rcNewPos != pControl->GetPos()) {
        pControl->SetPos(rcNewPos);
    }    
    return true;
}

typedef ControlResizableT<Control> ControlResizable;
typedef ControlResizableT<Box> BoxResizable;
typedef ControlResizableT<HBox> HBoxResizable;
typedef ControlResizableT<VBox> VBoxResizable;

}

#endif // UI_CORE_CONTROL_RESIZABLE_H_
