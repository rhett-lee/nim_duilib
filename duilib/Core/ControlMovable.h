#ifndef UI_CORE_CONTROL_MOVABLE_H_
#define UI_CORE_CONTROL_MOVABLE_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{
/** 主要功能：支持通过鼠标左键按下来调整控件的位置
*/
template<typename T = Control>
class UILIB_API ControlMovableT: public T
{
    typedef T BaseClass;
public:
    explicit ControlMovableT(Window* pWindow);
    virtual ~ControlMovableT() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;    
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 设置是否支持鼠标拖动改变控件的位置
    */
    void SetEnableMovePos(bool bEnable);

    /** 获取是否支持鼠标拖动改变控件的位置
    */
    bool IsEnableMovePos() const;

    /** 设置是否调整父容器的控件位置
    * @param [in] bMoveParentPos true表示调整父容器的位置，false表示调整本控件自身的位置
    */
    void SetMoveParentPos(bool bMoveParentPos);

    /** 获取是否调整父容器的控件位置
    */
    bool IsMoveParentPos() const;

    /** 设置拖动调整位置时，控件的透明度(0 - 255)
    */
    void SetMovePosAlpha(uint8_t nAlpha);

    /** 获取拖动调整位置时，控件的透明度
    */
    uint8_t GetMovePosAlpha() const;

    /** 设置控件可移动矩形的外边距（外边距定义的四周区域不可点击拖动，仅中心区域可拖动，未经DPI缩放）
    */
    void SetNonDraggableMargin(const UiMargin& rcNonDraggableMargin);

    /** 获取控件可移动矩形的外边距（外边距定义的四周区域不可点击拖动，仅中心区域可拖动，未经DPI缩放）
    */
    const UiMargin& GetNonDraggableMargin() const;

    /** 设置控件可移动矩形的边框范围（四周可点击拖动，但中心区域不可拖动，未经DPI缩放）
    */
    void SetDraggableBorder(const UiPadding& rcDraggableBorder);

    /** 获取控件可移动矩形的边框范围（四周可点击拖动，但中心区域不可拖动，未经DPI缩放）
    */
    const UiPadding& GetDraggableBorder() const;

    /** 设置横向移动时，在父容器内保留的高度，避免控件溢出父容器(未经DPI缩放)
    */
    void SetMovePosReserveWidth(int32_t nReserveWidth);

    /** 获取横向移动时，在父容器内保留的高度，避免控件溢出父容器(未经DPI缩放)
    */
    int32_t GetMovePosReserveWidth() const;

    /** 设置纵向移动时，在父容器内保留的宽度，避免控件溢出父容器(未经DPI缩放)
    */
    void SetMovePosReserveHeight(int32_t nReserveHeight);

    /** 获取纵向移动时，在父容器内保留的宽度，避免控件溢出父容器(未经DPI缩放)
    */
    int32_t GetMovePosReserveHeight() const;

    /** 设置移动控件时，是否确保子控件位于父容器内
    */
    void SetMovePosKeepWithinParent(bool bKeepWithinParent);

    /** 获取移动控件时，是否确保子控件位于父容器内
    */
    bool IsMovePosKeepWithinParent() const;

public:
    /** 当前是否处于拖动调整位置的操作中
    */
    bool IsMovingPos() const;

    /** 取消拖动调整位置操作
    */
    void CancelMovePosOperation();

protected:
    /** @name 拖动相关的成员函数
    * @{ */

    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点
    virtual void HandleEvent(const EventArgs& msg) override;

protected:
    /** 清除拖动调整位置的状态，恢复原状态
    */
    virtual void ClearMovePosStatus();

    /** 将控件限制在父容器范围内，避免完全溢出父容器
    * @param [in] pControl 控件的接口    
    * @param [in] xOffset X方向的移动距离
    * @param [in] yOffset Y方向的移动距离
    * @param [in] rcNewPos 控件新的位置
    */
    void AdjustRectInParentBox(Control* pControl, int32_t xOffset, int32_t yOffset, UiRect& rcNewPos) const;

    /** @} */

private:
    /** 鼠标移动时执行的拖动调整控件位置操作
    * @return 返回true表示处于拖动调整位置操作操作中，返回false表示未执行拖动调整位置操作
    */
    bool MovePosMouseMove(const EventArgs& msg);

    /** 鼠标弹起时执行的操作
    */
    void MovePosMouseUp(const EventArgs& msg);

    /** 将控件移动到鼠标所在位置
    */
    void MoveControlToPoint(const UiPoint& ptMouse);

    /** 获取调整控件的接口
    */
    Control* GetMovePosControl();

    /** 当前点击点是否处于可拖动区域
    */
    bool IsInDraggableArea(const UiPoint& ptMouse) const;

private:
    /** 是否支持拖动改变控件的位置(功能开关)
    */
    bool m_bEnableMovePos;

    /** 是否调整父容器的控件位置
    */
    bool m_bMoveParentPos;

    /** 是否鼠标左键按下
    */
    bool m_bMouseDown;

    /** 是否处于拖动调整位置的操作中
    */
    bool m_bMovingPos;

    /** 移动控件时，确保子控件位于父容器内
    */
    bool m_bKeepWithinParent;

    /** 拖动调整位置时，控件的透明度
    */
    uint8_t m_nMovePosAlpha;

    /** 控件原来的透明度
    */
    uint8_t m_nOldAlpha;

    /** 鼠标按下时的鼠标位置
    */
    UiPoint m_ptMouseDown;

    /** 鼠标按下时的控件矩形区域
    */
    UiRect m_rcMouseDown;

    /** 控件可移动矩形的外边距（外边距定义的四周区域不可点击拖动，仅中心区域可拖动，未经DPI缩放）
    */
    UiMargin m_rcNonDraggableMargin;

    /** 控件可移动矩形的边框范围（四周可点击拖动，但中心区域不可拖动，未经DPI缩放）
    */
    UiPadding m_rcDraggableBorder;

    /** 横向移动时，在父容器内保留的高度，避免控件完全溢出父容器(未经DPI缩放)
    */
    int32_t m_nMovePosReserveWidth;

    /** 纵向移动时，在父容器内保留的宽度，避免控件完全溢出父容器(未经DPI缩放)
    */
    int32_t m_nMovePosReserveHeight;
};

template<typename T>
ControlMovableT<T>::ControlMovableT(Window* pWindow):
    T(pWindow),
    m_bEnableMovePos(true),
    m_bMoveParentPos(false),
    m_bMovingPos(false),
    m_bMouseDown(false),
    m_bKeepWithinParent(false),
    m_nMovePosAlpha(216),
    m_nOldAlpha(255),
    m_nMovePosReserveWidth(20),
    m_nMovePosReserveHeight(20)
{
    //默认应为浮动控件，并保存控件位置
    this->SetKeepFloatPos(true);
    this->SetFloat(true);
}

template<typename T>
ControlMovableT<T>::~ControlMovableT()
{
}

template<typename T>
inline DString ControlMovableT<T>::GetType() const { return DUI_CTR_CONTROL_MOVABLE; }

template<>
inline DString ControlMovableT<Box>::GetType() const { return DUI_CTR_BOX_MOVABLE; }

template<>
inline DString ControlMovableT<HBox>::GetType() const { return DUI_CTR_HBOX_MOVABLE; }

template<>
inline DString ControlMovableT<VBox>::GetType() const { return DUI_CTR_VBOX_MOVABLE; }

template<typename T>
void ControlMovableT<T>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("enable_move_pos")) {
        SetEnableMovePos(strValue == _T("true"));
    }
    else if (strName == _T("move_parent_pos")) {
        SetMoveParentPos(strValue == _T("true"));
    }
    else if (strName == _T("move_pos_alpha")) {
        SetMovePosAlpha((uint8_t)StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("move_pos_non_draggable_margin")) {
        UiMargin rcNonDraggableMargin;
        AttributeUtil::ParseMarginValue(strValue.c_str(), rcNonDraggableMargin);
        SetNonDraggableMargin(rcNonDraggableMargin);
    }
    else if (strName == _T("move_pos_draggable_border")) {
        UiPadding rcDraggableBorder;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcDraggableBorder);
        SetDraggableBorder(rcDraggableBorder);
    }
    else if (strName == _T("move_pos_reserve_width")) {
        SetMovePosReserveWidth(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("move_pos_reserve_height")) {
        SetMovePosReserveHeight(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("move_pos_keep_within_parent")) {
        SetMovePosKeepWithinParent(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename T>
void ControlMovableT<T>::SetEnableMovePos(bool bEnable)
{
    m_bEnableMovePos = bEnable;
}

template<typename T>
bool ControlMovableT<T>::IsEnableMovePos() const
{
    return m_bEnableMovePos;
}

template<typename T>
void ControlMovableT<T>::SetMoveParentPos(bool bMoveParentPos)
{
    m_bMoveParentPos = bMoveParentPos;
}

template<typename T>
bool ControlMovableT<T>::IsMoveParentPos() const
{
    return m_bMoveParentPos;
}

template<typename T>
void ControlMovableT<T>::SetMovePosAlpha(uint8_t nAlpha)
{
    m_nMovePosAlpha = nAlpha;
}

template<typename T>
uint8_t ControlMovableT<T>::GetMovePosAlpha() const
{
    return m_nMovePosAlpha;
}

template<typename T>
void ControlMovableT<T>::SetNonDraggableMargin(const UiMargin& rcNonDraggableMargin)
{
    m_rcNonDraggableMargin = rcNonDraggableMargin;
    m_rcNonDraggableMargin.left = std::max(m_rcNonDraggableMargin.left, 0);
    m_rcNonDraggableMargin.top = std::max(m_rcNonDraggableMargin.top, 0);
    m_rcNonDraggableMargin.right = std::max(m_rcNonDraggableMargin.right, 0);
    m_rcNonDraggableMargin.bottom = std::max(m_rcNonDraggableMargin.bottom, 0);
}

template<typename T>
const UiMargin& ControlMovableT<T>::GetNonDraggableMargin() const
{
    return m_rcNonDraggableMargin;
}

template<typename T>
void ControlMovableT<T>::SetDraggableBorder(const UiPadding& rcDraggableBorder)
{
    m_rcDraggableBorder = rcDraggableBorder;
    m_rcDraggableBorder.left = std::max(m_rcDraggableBorder.left, 0);
    m_rcDraggableBorder.top = std::max(m_rcDraggableBorder.top, 0);
    m_rcDraggableBorder.right = std::max(m_rcDraggableBorder.right, 0);
    m_rcDraggableBorder.bottom = std::max(m_rcDraggableBorder.bottom, 0);
}

template<typename T>
const UiPadding& ControlMovableT<T>::GetDraggableBorder() const
{
    return m_rcDraggableBorder;
}

template<typename T>
void ControlMovableT<T>::SetMovePosReserveWidth(int32_t nReserveWidth)
{
    m_nMovePosReserveWidth = nReserveWidth;
    if (m_nMovePosReserveWidth < 0) {
        m_nMovePosReserveWidth = 0;
    }
}

template<typename T>
int32_t ControlMovableT<T>::GetMovePosReserveWidth() const
{
    return m_nMovePosReserveWidth;
}

template<typename T>
void ControlMovableT<T>::SetMovePosReserveHeight(int32_t nReserveHeight)
{
    m_nMovePosReserveHeight = nReserveHeight;
    if (m_nMovePosReserveHeight < 0) {
        m_nMovePosReserveHeight = 0;
    }
}

template<typename T>
int32_t ControlMovableT<T>::GetMovePosReserveHeight() const
{
    return m_nMovePosReserveHeight;
}

template<typename T>
void ControlMovableT<T>::SetMovePosKeepWithinParent(bool bKeepWithinParent)
{
    m_bKeepWithinParent = bKeepWithinParent;
}

template<typename T>
bool ControlMovableT<T>::IsMovePosKeepWithinParent() const
{
    return m_bKeepWithinParent;
}

template<typename T>
bool ControlMovableT<T>::IsMovingPos() const
{
    return m_bMovingPos;
}

template<typename T>
void ControlMovableT<T>::CancelMovePosOperation()
{
    ClearMovePosStatus();
}

template<typename T>
Control* ControlMovableT<T>::GetMovePosControl()
{
    if (IsMoveParentPos() && (this->GetParent() != nullptr)) {
        return this->GetParent();
    }
    return this;
}

template<typename T>
bool ControlMovableT<T>::IsInDraggableArea(const UiPoint& ptMouse) const
{
    UiPoint pt(ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());
    UiRect rect = this->GetRect();
    if (!rect.ContainsPt(pt)) {
        return false;
    }
    if (!m_rcNonDraggableMargin.IsEmpty() || !m_rcNonDraggableMargin.IsEmpty()) {
        if (!m_rcNonDraggableMargin.IsEmpty()) {
            UiRect rc = rect;
            UiMargin rcMargin = m_rcNonDraggableMargin;
            this->Dpi().ScaleMargin(rcMargin);
            rc.Deflate(rcMargin);
            rc.Validate();
            if (rc.ContainsPt(pt)) {
                return true;
            }
        }
        if (!m_rcDraggableBorder.IsEmpty()) {
            UiRect rc = rect;
            UiPadding rcPadding = m_rcDraggableBorder;
            this->Dpi().ScalePadding(rcPadding);
            rc.Deflate(rcPadding);
            rc.Validate();
            if (rc.ContainsPt(pt)) {
                return false;
            }
            return true;
        }        
        return false;
    }
    else {
        return true;
    }
}

template<typename T>
bool ControlMovableT<T>::ButtonDown(const EventArgs& msg)
{
    m_bMouseDown = false;
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (!IsEnableMovePos()) {
        //功能关闭
        return bRet;
    }

    if (!IsInDraggableArea(msg.ptMouse)) {
        //点击位置不在可拖动区域
        return bRet;
    }

    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    m_bMouseDown = true;
    m_ptMouseDown = pt;

    m_rcMouseDown = GetMovePosControl()->GetRect();
    return bRet;
}

template<typename T>
bool ControlMovableT<T>::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_bMouseDown) {
        MovePosMouseMove(msg);
    }
    return bRet;
}

template<typename T>
bool ControlMovableT<T>::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_bMouseDown) {
        MovePosMouseUp(msg);
    }
    ClearMovePosStatus();
    return bRet;
}

template<typename T>
bool ControlMovableT<T>::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnWindowKillFocus(msg);
    ClearMovePosStatus();
    return bRet;
}

template<typename T>
void ControlMovableT<T>::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if (msg.IsSenderExpired()) {
        return;
    }
    if ((msg.eventType == kEventMouseClickChanged) ||
        (msg.eventType == kEventMouseClickEsc) ||
        (msg.eventType == kEventMouseRButtonDown)) {
        //取消操作
        ClearMovePosStatus();
    }
}

template<typename T>
void ControlMovableT<T>::ClearMovePosStatus()
{
    //恢复状态
    if (m_bMovingPos) {
        Control* pControl = GetMovePosControl();
        pControl->SetAlpha(m_nOldAlpha);
        m_nOldAlpha = 255;
        m_bMovingPos = false;
    }
    m_bMouseDown = false;
}

template<typename T>
bool ControlMovableT<T>::MovePosMouseMove(const EventArgs& msg)
{
    if (!IsEnableMovePos()) {
        //该控件禁止拖动调整大小
        return false;
    }
    if (!m_bMouseDown) {
        return false;
    }

    //将控件移动到鼠标所在位置
    MoveControlToPoint(msg.ptMouse);

    if (!m_bMovingPos) {
        Control* pControl = GetMovePosControl();
        pControl->SetKeepFloatPos(true);
        if (!pControl->IsFloat()) {
            pControl->SetFloat(true);
            MoveControlToPoint(msg.ptMouse);
        }

        m_bMovingPos = true;
        m_nOldAlpha = (uint8_t)pControl->GetAlpha();
        //设置为半透明的效果
        pControl->SetAlpha(m_nMovePosAlpha);
    }
    return m_bMovingPos;
}

template<typename T>
void ControlMovableT<T>::MovePosMouseUp(const EventArgs& msg)
{
    if (!IsEnableMovePos() || !IsMovingPos()) {
        return;
    }
    MoveControlToPoint(msg.ptMouse);
    ClearMovePosStatus();
}

template<typename T>
void ControlMovableT<T>::MoveControlToPoint(const UiPoint& ptMouse)
{
    UiPoint pt(ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    int32_t xOffset = pt.x - m_ptMouseDown.x;
    int32_t yOffset = pt.y - m_ptMouseDown.y;

    Control* pControl = GetMovePosControl();
    UiRect rcNewPos = m_rcMouseDown;
    rcNewPos.left += xOffset;
    rcNewPos.right += xOffset;

    rcNewPos.top += yOffset;
    rcNewPos.bottom += yOffset;

    //将控件限制在父容器范围内，避免完全溢出父容器
    AdjustRectInParentBox(pControl, xOffset, yOffset, rcNewPos);

    //调整控件的位置
    pControl->SetPos(rcNewPos);
}

template<typename T>
void ControlMovableT<T>::AdjustRectInParentBox(Control* pControl, int32_t xOffset, int32_t yOffset, UiRect& rcNewPos) const
{
    Control* pParent = nullptr;
    if (pControl != nullptr) {
        pParent = pControl->GetParent();
    }
    if (pParent == nullptr) {
        return;
    }
    UiRect rcParent = pParent->GetRect();
    rcParent.Deflate(pParent->GetPadding());// 剪去内边距

    if (IsMovePosKeepWithinParent()) {
        //将控件完全限制在父容器范围内，避免溢出
        if (xOffset < 0) {
            //向左移动
            if (rcNewPos.left < rcParent.left) {
                int32_t offset = rcParent.left - rcNewPos.left;
                rcNewPos.Offset(offset, 0);
            }
        }
        else if (xOffset > 0) {
            //向右移动
            if (rcNewPos.right > rcParent.right) {
                int32_t offset = rcNewPos.right - rcParent.right;
                rcNewPos.Offset(-offset, 0);
            }
        }
        if (yOffset < 0) {
            //向上移动
            if (rcNewPos.top < rcParent.top) {
                int32_t offset = rcParent.top - rcNewPos.top;
                rcNewPos.Offset(0, offset);
            }
        }
        else if (yOffset > 0) {
            //向下移动
            if (rcNewPos.bottom > rcParent.bottom) {
                int32_t offset = rcNewPos.bottom - rcParent.bottom;
                rcNewPos.Offset(0, -offset);
            }
        }
    }
    else {
        //允许部分溢出，但不允许完全溢出
        int32_t nReserveValueX = GetMovePosReserveWidth();
        this->Dpi().ScaleInt(nReserveValueX);
        if (nReserveValueX > 0) {
            if (xOffset < 0) {
                //向左移动
                if (rcNewPos.right < (rcParent.left + nReserveValueX)) {
                    int32_t offset = (rcParent.left + nReserveValueX) - rcNewPos.right;
                    rcNewPos.Offset(offset, 0);
                }
            }
            else if (xOffset > 0) {
                //向右移动
                if (rcNewPos.left > (rcParent.right - nReserveValueX)) {
                    int32_t offset = rcNewPos.left - (rcParent.right - nReserveValueX);
                    rcNewPos.Offset(-offset, 0);
                }
            }
        }

        int32_t nReserveValueY = GetMovePosReserveHeight();
        this->Dpi().ScaleInt(nReserveValueY);
        if (yOffset < 0) {
            //向上移动
            if (rcNewPos.bottom < (rcParent.top + nReserveValueY)) {
                int32_t offset = (rcParent.top + nReserveValueY) - rcNewPos.bottom;
                rcNewPos.Offset(0, offset);
            }
        }
        else if (yOffset > 0) {
            //向下移动
            if (rcNewPos.top > (rcParent.bottom - nReserveValueY)) {
                int32_t offset = rcNewPos.top - (rcParent.bottom - nReserveValueY);
                rcNewPos.Offset(0, -offset);
            }
        }
    }
}

typedef ControlMovableT<Control> ControlMovable;
typedef ControlMovableT<Box> BoxMovable;
typedef ControlMovableT<HBox> HBoxMovable;
typedef ControlMovableT<VBox> VBoxMovable;

}

#endif // UI_CORE_CONTROL_MOVABLE_H_
