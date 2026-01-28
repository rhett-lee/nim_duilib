#ifndef UI_CORE_CONTROL_MOVABLE_H_
#define UI_CORE_CONTROL_MOVABLE_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Utils/StringUtil.h"

//鼠标移动操作，最短的距离
#define MOVE_OPERATION_MIN_PT 5

namespace ui
{
/** 主要功能：支持通过鼠标左键按下来调整控件位置
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

    /** 设置拖动调整位置时，控件的透明度
    */
    void SetMovePosAlpha(uint8_t nAlpha);

    /** 获取拖动调整位置时，控件的透明度
    */
    uint8_t GetMovePosAlpha() const;

    /** 当前是否处于拖动调整位置的操作中
    */
    bool IsInMovingPos() const;

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

    /** @} */

private:
    /** 鼠标按下时执行的拖动调整控件位置操作
    */
    void MovePosMouseDown(const EventArgs& msg);

    /** 鼠标移动时执行的拖动调整控件位置操作
    * @return 返回true表示处于拖动调整位置操作操作中，返回false表示未执行拖动调整位置操作
    */
    bool MovePosMouseMove(const EventArgs& msg);

    /** 鼠标弹起时执行的操作
    */
    void MovePosMouseUp(const EventArgs& msg);

    /** 将控件移动到鼠标所在位置
    */
    bool MoveControlToPoint(const UiPoint& ptMouse);

    /** 获取调整控件的接口
    */
    Control* GetMovePosControl();

private:
    /** @name 在相同窗口内同一个Box内拖动相关的成员变量
    * @{ */

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

    /** @} */
};

template<typename T>
ControlMovableT<T>::ControlMovableT(Window* pWindow):
    T(pWindow),
    m_bEnableMovePos(true),
    m_bMoveParentPos(false),
    m_bMovingPos(false),
    m_bMouseDown(false),
    m_nMovePosAlpha(216),
    m_nOldAlpha(255)
{
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
bool ControlMovableT<T>::IsInMovingPos() const
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

    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    m_bMouseDown = true;
    m_ptMouseDown = pt;

    m_rcMouseDown = GetMovePosControl()->GetRect();
    MovePosMouseDown(msg); 
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
    MovePosMouseUp(msg);
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
        //取消拖动调序或者拖出操作
        ClearMovePosStatus();
    }
}

template<typename T>
void ControlMovableT<T>::ClearMovePosStatus()
{
    //恢复拖动调序操作的状态
    if (m_bMovingPos) {
        Control* pControl = GetMovePosControl();
        pControl->SetAlpha(m_nOldAlpha);
        m_nOldAlpha = 255;
        m_bMovingPos = false;
    }
    m_bMouseDown = false;
}

template<typename T>
void ControlMovableT<T>::MovePosMouseDown(const EventArgs& /*msg*/)
{
    if (!IsEnableMovePos()) {
        return;
    }
    m_bMouseDown = true;
}

template<typename T>
bool ControlMovableT<T>::MovePosMouseMove(const EventArgs& msg)
{
    if (!IsEnableMovePos()) {
        //该控件禁止拖动调序
        return false;
    }
    if (!m_bMouseDown) {
        return false;
    }

    //将控件移动到鼠标所在位置
    if (!MoveControlToPoint(msg.ptMouse)) {
        return false;
    }

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
    if (!IsEnableMovePos() || !IsInMovingPos()) {
        return;
    }
    MoveControlToPoint(msg.ptMouse);
    ClearMovePosStatus();
}

template<typename T>
bool ControlMovableT<T>::MoveControlToPoint(const UiPoint& ptMouse)
{
    UiPoint pt(ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    int32_t xOffset = pt.x - m_ptMouseDown.x;
    int32_t yOffset = pt.y - m_ptMouseDown.y;
    const int32_t nMinOffset = this->Dpi().GetScaleInt(MOVE_OPERATION_MIN_PT);

    if (!m_bMovingPos) {
        if ((std::abs(xOffset) < nMinOffset) && (std::abs(yOffset) < nMinOffset)) {
            //未达到调整的阈值
            return false;
        }
    }

    Control* pControl = GetMovePosControl();
    UiRect rcNewPos = m_rcMouseDown;
    rcNewPos.left += xOffset;
    rcNewPos.right += xOffset;

    rcNewPos.top += yOffset;
    rcNewPos.bottom += yOffset;

    pControl->SetPos(rcNewPos);
    return true;
}

typedef ControlMovableT<Control> ControlMovable;
typedef ControlMovableT<Box> BoxMovable;
typedef ControlMovableT<HBox> HBoxMovable;
typedef ControlMovableT<VBox> VBoxMovable;

}

#endif // UI_CORE_CONTROL_MOVABLE_H_
