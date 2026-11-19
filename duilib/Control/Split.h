#ifndef UI_CONTROL_SPLIT_H_
#define UI_CONTROL_SPLIT_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

namespace ui
{

/** 分割条控件，可以通过拖动分割条改变左右或者上下两个控件的宽度或者高度
*   应用：如果放在横向布局（HLayout）中，则左右拖动
*        如果放在纵向布局（VLayout）中，则上下拖动
*   拖动时调整的是同一父容器中、位于分割条前面和后面的两个可见控件（非float）的固定宽度或者固定高度：
*    - 两个控件都是固定尺寸时，两者尺寸之和保持不变；
*    - 其中一个是拉伸类型时，只调整另一个控件，拉伸类型的控件由父容器自动分配尺寸；
*    - 两个控件都是拉伸类型时，鼠标按下后会把左侧（上侧）控件转换为固定尺寸，然后可以正常拖动。
*   如果父容器的布局不是HLayout或者VLayout，则不支持拖动（不显示调整光标）。
*   属性 double_click_reset 为 true 时，双击分割条可以把两侧控件恢复到首次拖动前的尺寸。
*   拖动开始、拖动过程和拖动结束时，分别触发 kEventSplitDragBegin、kEventSplitDraged、kEventSplitDragEnd 事件。
*/
template<typename InheritType = Control>
class SplitTemplate : public InheritType
{
    typedef InheritType BaseClass;
public:
    explicit SplitTemplate(Window* pWindow);

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
 
    /** 是否可以拖动
    */
    bool IsSplitDragValid() const;

    /** 停止拖动
    */
    void StopSplitDrag();

    /** 设置当只有一个控件的时候，是否允许调整其宽度
    */
    void SetEnableSplitSingle(bool bEnableSplitSingle);

    /** 获取当只有一个控件的时候，是否允许调整其宽度
    */
    bool IsEnableSplitSingle() const;

    /** 设置双击分割条时，是否把两侧控件恢复到首次拖动前的尺寸
    * @param [in] bDoubleClickReset true表示允许双击复位，false表示不处理双击
    */
    void SetDoubleClickReset(bool bDoubleClickReset);

    /** 获取双击分割条时，是否把两侧控件恢复到首次拖动前的尺寸
    */
    bool IsDoubleClickReset() const;

    /** 监听开始拖动的事件（WPARAM 为左侧/上侧控件，LPARAM 为右侧/下侧控件，可能为 nullptr）
     * @param [in] callback 开始拖动时触发的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachSplitDragBegin(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventSplitDragBegin, callback, callbackID); }

    /** 监听结束拖动的事件（WPARAM 为左侧/上侧控件，LPARAM 为右侧/下侧控件，可能为 nullptr）
     * @param [in] callback 结束拖动时触发的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachSplitDragEnd(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventSplitDragEnd, callback, callbackID); }

    /** 监听拖动的事件
     * @param [in] callback 被选择时触发的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachSplitDragged(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventSplitDraged, callback, callbackID); }

    /** 监听拖动的事件（函数名拼写有误，保留是为了兼容旧版本，建议使用AttachSplitDragged）
     * @param [in] callback 被选择时触发的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachSplitDraged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachSplitDragged(callback, callbackID); }

protected:
    virtual bool MouseEnter(const EventArgs& msg) override;
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool ButtonDoubleClick(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool OnWindowKillFocus(const EventArgs& msg) override;

private:
    //是否为垂直布局的容器
    bool IsVLayout(Control* pControl) const;

    //是否为水平布局的容器
    bool IsHLayout(Control* pControl) const;

    /** 查找同级的相邻控件（前一个和后一个可见的、非float的控件）
    * @param [out] pPrev 返回前一个控件，如果没有，返回nullptr
    * @param [out] pNext 返回后一个控件，如果没有，返回nullptr
    */
    void FindPrevNextControl(Control*& pPrev, Control*& pNext);

    /** 当前是否可以拖动（根据父容器的布局方向与相邻控件判断）
    */
    bool CanStartDrag();

    /** 更新鼠标光标：只有可以拖动时，才显示尺寸调整光标；不可拖动时恢复默认光标
    */
    void UpdateCursor();

    /** 调整两个控件中，其中一个（pFirst）控件的宽度或者高度
    * @param [in] nTotal 两个控件的总体宽度或者高度
    * @param [in] nOffset 鼠标的宽度或者高度偏移量
    * @param [in] pFirst 第一个控件接口，该函数会调整该接口对应控件的宽度或者高度
    * @param [in] nFirstFixedInt 第一个控件的原始宽度或者高度值（在鼠标按下时记录）
    * @param [in] pSecond 第二个控件接口，读取其最小值，避免调整第一个控件时未给留下足够最小空间
    * @return 返回第一个控件的宽度或者高度
    */
    int32_t CalculateControlPos(const int32_t nTotal, const int32_t nOffset,
                                Control* pFirst, const UiFixedInt& nFirstFixedInt,
                                const Control* pSecond) const;

    /** 仅调整一个控件的大小
    */
    void OnMouseMoveSingle(const EventArgs& msg);

    /** 获取控件在主布局方向（横向为宽度，纵向为高度）上的当前尺寸
    */
    int32_t GetAxisSize(const Control* pControl) const;

    /** 获取控件在主布局方向上的固定尺寸
    */
    const UiFixedInt& GetAxisFixedSize(const Control* pControl) const;

    /** 设置控件在主布局方向上的固定尺寸
    * @param [in] bArrange true表示立即触发布局，false表示只修改值（由后续操作统一触发布局）
    */
    void SetAxisFixedSize(Control* pControl, int32_t nValue, bool bArrange);

    /** 恢复控件在主布局方向上的固定尺寸（固定值/拉伸/自动类型均按原值恢复）
    * @param [in] bArrange true表示立即触发布局，false表示只修改值（由后续操作统一触发布局）
    * @return 返回true表示尺寸发生了变化
    */
    bool ResetAxisFixedSize(Control* pControl, const UiFixedInt& fixedSize, bool bArrange);

    /** 获取控件在主布局方向上的最小尺寸
    */
    int32_t GetAxisMinSize(const Control* pControl) const;

    /** 获取控件在主布局方向上的最大尺寸
    */
    int32_t GetAxisMaxSize(const Control* pControl) const;

    /** 获取父容器在主布局方向上可用的尺寸（扣除分割条自身与其它控件占用的空间）
    * @param [in] pControl 正在调整的控件（计算可用空间时排除该控件）
    */
    int32_t GetParentAvailableSize(const Control* pControl) const;

private:
    //左侧（左右拖动）或者上侧（上下拖动）的控件接口
    Control* m_pLeftTop;

    //右侧（左右拖动）或者下侧（上下拖动）的控件接口
    Control* m_pRightBottom;

    //左侧（左右拖动）或者上侧（上下拖动）的控件的宽度（左右拖动）或者高度（上下拖动）
    UiFixedInt m_nLeftTopFixedValue;

    //右侧（左右拖动）或者下侧（上下拖动）的控件的宽度（左右拖动）或者高度（上下拖动）
    UiFixedInt m_nRightBottomFixedValue;

    //首次拖动前，左侧（上侧）控件的宽度（左右拖动）或者高度（上下拖动），用于双击复位
    UiFixedInt m_nLeftTopInitFixedValue;

    //首次拖动前，右侧（下侧）控件的宽度（左右拖动）或者高度（上下拖动），用于双击复位
    UiFixedInt m_nRightBottomInitFixedValue;

    //是否已经记录首次拖动前的尺寸（用于双击复位）
    bool m_bInitSizeCaptured;

    //开始拖动时的鼠标位置
    UiPoint m_ptStart;

    //布局方向: true表示横向布局，false表示纵向布局
    bool m_bHLayout;

    //是否处于拖动状态（已发送拖动开始事件，尚未发送拖动结束事件）
    bool m_bDragStarted;

    /** 当只有一个控件的时候，是否允许调整其宽度
    */
    bool m_bEnableSplitSingle;

    /** 双击分割条时，是否把两侧控件恢复到首次拖动前的尺寸
    */
    bool m_bDoubleClickReset;
};

template<typename InheritType>
SplitTemplate<InheritType>::SplitTemplate(Window* pWindow):
    InheritType(pWindow),
    m_pLeftTop(nullptr),
    m_pRightBottom(nullptr),
    m_bInitSizeCaptured(false),
    m_bHLayout(true),
    m_bDragStarted(false),
    m_bEnableSplitSingle(false),
    m_bDoubleClickReset(false)
{
    this->SetMouseEnabled(true);
}

template<typename InheritType>
inline DString SplitTemplate<InheritType>::GetType() const { return DUI_CTR_SPLIT; }

template<>
inline DString SplitTemplate<Box>::GetType() const { return DUI_CTR_SPLITBOX; }

template<typename InheritType>
void SplitTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue)
{
    DString strNewValue = this->GetExpandVarStrings(strValue);
    if (strName == _T("enable_split_single")) {
        SetEnableSplitSingle(StringUtil::IsValueTrue(strNewValue));
    }
    else if (strName == _T("double_click_reset")) {
        SetDoubleClickReset(StringUtil::IsValueTrue(strNewValue));
    }
    else {
        BaseClass::SetAttribute(strName, strNewValue);
    }
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetEnableSplitSingle(bool bEnableSplitSingle)
{
    m_bEnableSplitSingle = bEnableSplitSingle;
    UpdateCursor();
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsEnableSplitSingle() const
{
    return m_bEnableSplitSingle;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetDoubleClickReset(bool bDoubleClickReset)
{
    m_bDoubleClickReset = bDoubleClickReset;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsDoubleClickReset() const
{
    return m_bDoubleClickReset;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsSplitDragValid() const
{
    bool bDragValid = false;
    if (IsEnableSplitSingle()) {
        bDragValid = (m_pLeftTop != nullptr) || (m_pRightBottom != nullptr);
    }
    else {
        bDragValid = (m_pLeftTop != nullptr) && (m_pRightBottom != nullptr);
    }
    return bDragValid;
}

template<typename InheritType>
void SplitTemplate<InheritType>::StopSplitDrag() 
{
    Control* pLeftTop = m_pLeftTop;
    Control* pRightBottom = m_pRightBottom;
    bool bDragStarted = m_bDragStarted;

    //先清理拖动状态，再发送事件（避免事件处理过程中再次结束拖动）
    m_pLeftTop = nullptr; 
    m_pRightBottom = nullptr;
    m_bDragStarted = false;

    if (bDragStarted) {
        this->SendEvent(kEventSplitDragEnd, (WPARAM)pLeftTop, (LPARAM)pRightBottom);
    }
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsVLayout(Control* pControl) const
{
    const Box* pBox = dynamic_cast<Box*>(pControl);
    if (pBox == nullptr) {
        return false;
    }
    const Layout* pLayout = pBox->GetLayout();
    if (pLayout == nullptr) {
        return false;
    }
    return pLayout->IsVLayout();
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsHLayout(Control* pControl) const
{
    const Box* pBox = dynamic_cast<Box*>(pControl);
    if (pBox == nullptr) {
        return false;
    }
    const Layout* pLayout = pBox->GetLayout();
    if (pLayout == nullptr) {
        return false;
    }
    return pLayout->IsHLayout();
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetAxisSize(const Control* pControl) const
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return 0;
    }
    return m_bHLayout ? pControl->GetWidth() : pControl->GetHeight();
}

template<typename InheritType>
const UiFixedInt& SplitTemplate<InheritType>::GetAxisFixedSize(const Control* pControl) const
{
    ASSERT(pControl != nullptr);
    return m_bHLayout ? pControl->GetFixedWidth() : pControl->GetFixedHeight();
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetAxisFixedSize(Control* pControl, int32_t nValue, bool bArrange)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return;
    }
    if (m_bHLayout) {
        pControl->SetFixedWidth(UiFixedInt(nValue), bArrange, false);
    }
    else {
        pControl->SetFixedHeight(UiFixedInt(nValue), bArrange, false);
    }
}

template<typename InheritType>
bool SplitTemplate<InheritType>::ResetAxisFixedSize(Control* pControl, const UiFixedInt& fixedSize, bool bArrange)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    if (GetAxisFixedSize(pControl) == fixedSize) {
        return false;
    }
    if (m_bHLayout) {
        pControl->SetFixedWidth(fixedSize, bArrange, false);
    }
    else {
        pControl->SetFixedHeight(fixedSize, bArrange, false);
    }
    return true;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetAxisMinSize(const Control* pControl) const
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return 0;
    }
    return m_bHLayout ? pControl->GetMinWidth() : pControl->GetMinHeight();
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetAxisMaxSize(const Control* pControl) const
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return 0;
    }
    return m_bHLayout ? pControl->GetMaxWidth() : pControl->GetMaxHeight();
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetParentAvailableSize(const Control* pControl) const
{
    Box* pParent = this->GetParent();
    ASSERT(pParent != nullptr);
    if (pParent == nullptr) {
        return 0;
    }
    UiRect rcParent = pParent->GetPosWithoutPadding();
    int32_t nAvailableSize = m_bHLayout ? rcParent.Width() : rcParent.Height();
    //扣除分割条自身占用的空间，保证调整后控件不会把分割条挤出父容器
    nAvailableSize -= (m_bHLayout ? this->GetWidth() : this->GetHeight());

    //扣除同一容器中其它控件占用的空间：固定尺寸的控件按当前尺寸计算，拉伸类型的控件按最小尺寸计算
    const Control* pThisControl = static_cast<const Control*>(this);
    const size_t nChildCount = pParent->GetItemCount();
    for (size_t i = 0; i < nChildCount; ++i) {
        Control* pChild = pParent->GetItemAt(i);
        if ((pChild == nullptr) || (pChild == pControl) || (pChild == pThisControl) ||
            !pChild->IsVisible() || pChild->IsFloat()) {
            continue;
        }
        if (GetAxisFixedSize(pChild).IsStretch()) {
            nAvailableSize -= GetAxisMinSize(pChild);
        }
        else {
            nAvailableSize -= GetAxisSize(pChild);
        }
    }
    if (nAvailableSize < 0) {
        nAvailableSize = 0;
    }
    return nAvailableSize;
}

template<typename InheritType>
void SplitTemplate<InheritType>::FindPrevNextControl(Control*& pPrev, Control*& pNext)
{
    pPrev = nullptr;
    pNext = nullptr;
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return;
    }
    size_t nChildCount = pParent->GetItemCount();
    bool bFoundThis = false;
    for (size_t i = 0; i < nChildCount; ++i) {
        Control* pControl = pParent->GetItemAt(i);
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }
        if (bFoundThis) {
            pNext = pControl;
            break;
        }
        if (pControl == this) {
            bFoundThis = true;
        }
        else {
            pPrev = pControl;
        }
    }
    if (!bFoundThis) {
        //当前控件不是父容器的直接子控件，无法确定相邻控件
        pPrev = nullptr;
        pNext = nullptr;
    }
}

template<typename InheritType>
bool SplitTemplate<InheritType>::CanStartDrag()
{
    if (!this->IsEnabled()) {
        return false;
    }
    Box* pParent = this->GetParent();
    if ((pParent == nullptr) || (!IsHLayout(pParent) && !IsVLayout(pParent))) {
        return false;
    }
    Control* pPrev = nullptr;
    Control* pNext = nullptr;
    FindPrevNextControl(pPrev, pNext);
    if (IsEnableSplitSingle()) {
        return (pPrev != nullptr) || (pNext != nullptr);
    }
    return (pPrev != nullptr) && (pNext != nullptr);
}

template<typename InheritType>
void SplitTemplate<InheritType>::UpdateCursor()
{
    if ((this->IsEnabled()) && CanStartDrag()) {
        Box* pParent = this->GetParent();
        if (IsHLayout(pParent)) {
            //横向布局：左右拖动
            if (this->GetCursorType() != CursorType::kCursorSizeWE) {
                this->SetCursorType(CursorType::kCursorSizeWE);
            }
            return;
        }
        else if (IsVLayout(pParent)) {
            //纵向布局：上下拖动
            if (this->GetCursorType() != CursorType::kCursorSizeNS) {
                this->SetCursorType(CursorType::kCursorSizeNS);
            }
            return;
        }
    }
    //当前不可拖动：如果光标是分割条设置的尺寸调整光标，则恢复为默认光标
    CursorType cursorType = this->GetCursorType();
    if ((cursorType == CursorType::kCursorSizeWE) || (cursorType == CursorType::kCursorSizeNS)) {
        this->SetCursorType(CursorType::kCursorArrow);
    }
}

template<typename InheritType>
bool SplitTemplate<InheritType>::MouseEnter(const EventArgs& msg)
{
    //只有可以拖动时，才显示尺寸调整光标
    UpdateCursor();
    return BaseClass::MouseEnter(msg);
}

template<typename InheritType>
bool SplitTemplate<InheritType>::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    //清空上一次拖动的状态，避免遗留旧的控件指针（如控件树或父容器布局在两次拖动之间发生了变化）
    StopSplitDrag();

    Box* pParent = this->GetParent();
    if (!this->IsEnabled() || (pParent == nullptr)) {
        return bRet;
    }
    Control* pPrev = nullptr;
    Control* pNext = nullptr;
    FindPrevNextControl(pPrev, pNext);
    bool bDragValid = false;
    if (IsEnableSplitSingle()) {
        bDragValid = (pPrev != nullptr) || (pNext != nullptr);
    }
    else {
        bDragValid = (pPrev != nullptr) && (pNext != nullptr);
    }
    bool bLayoutValid = false;
    if (bDragValid) {
        m_bHLayout = IsHLayout(pParent);
        bLayoutValid = m_bHLayout || IsVLayout(pParent);
    }
    bool bStartDrag = false;
    if (bLayoutValid) {
        m_pLeftTop = pPrev;
        m_pRightBottom = pNext;
        m_ptStart = msg.ptMouse;

        m_nLeftTopFixedValue = UiFixedInt(0);
        if (m_pLeftTop != nullptr) {
            m_nLeftTopFixedValue = GetAxisFixedSize(m_pLeftTop);
            if (!m_nLeftTopFixedValue.IsStretch()) {
                m_nLeftTopFixedValue.SetInt32(GetAxisSize(m_pLeftTop));
            }
        }

        m_nRightBottomFixedValue = UiFixedInt(0);
        if (m_pRightBottom != nullptr) {
            m_nRightBottomFixedValue = GetAxisFixedSize(m_pRightBottom);
            if (!m_nRightBottomFixedValue.IsStretch()) {
                m_nRightBottomFixedValue.SetInt32(GetAxisSize(m_pRightBottom));
            }
        }

        //首次开始拖动前，记录两个控件的固定尺寸（保留固定值/拉伸/自动类型），用于双击复位
        if (!m_bInitSizeCaptured) {
            m_nLeftTopInitFixedValue = (m_pLeftTop != nullptr) ? GetAxisFixedSize(m_pLeftTop) : UiFixedInt(0);
            m_nRightBottomInitFixedValue = (m_pRightBottom != nullptr) ? GetAxisFixedSize(m_pRightBottom) : UiFixedInt(0);
            m_bInitSizeCaptured = true;
        }

        //如果两个控件都是拉伸类型的，那么让左侧(上侧)的变为非拉伸的，允许拖动操作
        if ((m_pLeftTop != nullptr) && (m_pRightBottom != nullptr) &&
            m_nLeftTopFixedValue.IsStretch() && m_nRightBottomFixedValue.IsStretch()) {
            m_nLeftTopFixedValue.SetInt32(GetAxisSize(m_pLeftTop));
        }

        //进入拖动状态，在重绘后发送拖动开始事件
        if (!m_bDragStarted) {
            m_bDragStarted = true;
            bStartDrag = true;
        }
    }
    this->Invalidate();
    if (bStartDrag) {
        this->SendEvent(kEventSplitDragBegin, (WPARAM)m_pLeftTop, (LPARAM)m_pRightBottom);
    }
    return bRet;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    StopSplitDrag();
    return bRet;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::ButtonDoubleClick(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDoubleClick(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (!m_bDoubleClickReset || !m_bInitSizeCaptured || !CanStartDrag()) {
        return bRet;
    }
    //双击复位：把两侧控件恢复到首次拖动前的尺寸
    Control* pPrev = nullptr;
    Control* pNext = nullptr;
    FindPrevNextControl(pPrev, pNext);
    m_bHLayout = IsHLayout(this->GetParent());

    bool bAdjusted = false;
    if (pPrev != nullptr) {
        if (ResetAxisFixedSize(pPrev, m_nLeftTopInitFixedValue, false)) {
            bAdjusted = true;
        }
    }
    if (pNext != nullptr) {
        if (ResetAxisFixedSize(pNext, m_nRightBottomInitFixedValue, false)) {
            bAdjusted = true;
        }
    }
    if (bAdjusted) {
        //两个控件的值都恢复后，统一触发布局
        this->ArrangeAncestor();
        this->Invalidate();
    }
    return bRet;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnWindowKillFocus(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    //窗口失去焦点时收不到ButtonUp消息，需要结束拖动（避免遗留拖动状态，拖动状态改变后需要重绘）
    this->Invalidate();
    StopSplitDrag();
    return bRet;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::CalculateControlPos(const int32_t nTotal, const int32_t nOffset,
                                                       Control* pFirst, const UiFixedInt& nFirstFixedInt,
                                                       const Control* pSecond) const
{
    ASSERT((pFirst != nullptr) && (pSecond != nullptr));
    if ((pFirst == nullptr) || (pSecond == nullptr)) {
        return 0;
    }
    int32_t nMin = GetAxisMinSize(pFirst);          //最小值
    int32_t nMax = GetAxisMaxSize(pFirst);          //最大值
    int32_t nSecondMin = GetAxisMinSize(pSecond);   //另一侧控件的最小值

    //最大值需要给另一侧控件留出最小空间（另一侧的最小值为0时，也不能超过总尺寸）
    int32_t nMaxBySecond = nTotal - nSecondMin;
    if (nMax > nMaxBySecond) {
        nMax = nMaxBySecond;
    }
    //最大值不能小于最小值（配置冲突时，以本控件的最小值为准）
    if (nMax < nMin) {
        nMax = nMin;
    }

    //新值，可以是正数，也可以是负数（从鼠标按下时的值开始，累计鼠标的偏移量）
    int32_t nNewValue = nFirstFixedInt.GetInt32() + nOffset;
    //控制不小于最小值
    if (nNewValue < nMin) {
        nNewValue = nMin;
    }
    //控制不大于最大值
    if (nNewValue > nMax) {
        nNewValue = nMax;
    }
    return nNewValue;    
}

template<typename InheritType>
bool SplitTemplate<InheritType>::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    if (!IsSplitDragValid() || !this->IsMouseFocused()) {
        return bRet;
    }

    if ((m_pLeftTop == nullptr) || (m_pRightBottom == nullptr)) {
        //只有一个控件需要调整大小
        OnMouseMoveSingle(msg);
        return bRet;
    }
    
    int32_t nOffset = 0; //拖动的范围（宽度或者高度的偏移量, 可以是正数，也可以是负数）
    if (m_bHLayout) {
        nOffset = msg.ptMouse.x - m_ptStart.x;
    }
    else {
        nOffset = msg.ptMouse.y - m_ptStart.y;
    }

    //原地拖动，不需要调整
    if (nOffset == 0) {
        return bRet;
    }

    int32_t nTotal = 0; //总的宽度值或者总的高度值
    bool bLeftTopFixed = !m_nLeftTopFixedValue.IsStretch();
    bool bRightBottomFixed = !m_nRightBottomFixedValue.IsStretch();
    if (bLeftTopFixed && bRightBottomFixed) {
        nTotal = m_nLeftTopFixedValue.GetInt32() + m_nRightBottomFixedValue.GetInt32();
    }
    else {
        nTotal = GetAxisSize(m_pLeftTop) + GetAxisSize(m_pRightBottom);
    }

    //对于拉伸类型的控件，不调整，交给父容器自动调整
    Control* pControl1 = nullptr;
    Control* pControl2 = nullptr;
    int32_t nNewValue1 = -1;
    int32_t nNewValue2 = -1;
    if (bLeftTopFixed && bRightBottomFixed) {
        //两个控件都是固定尺寸：先按鼠标偏移量计算第一个控件的新值，再由总尺寸算出第二个控件的新值，
        //保证两个控件的尺寸之和保持不变，并且两个控件都不会超出各自允许的范围
        const int32_t nFirstMin = GetAxisMinSize(m_pLeftTop);
        const int32_t nFirstMax = GetAxisMaxSize(m_pLeftTop);
        const int32_t nSecondMin = GetAxisMinSize(m_pRightBottom);
        const int32_t nSecondMax = GetAxisMaxSize(m_pRightBottom);
        //第一个控件允许的最小值和最大值（同时给第二个控件留出足够的空间）
        int32_t nValidMin = nFirstMin;
        int32_t nValidMax = nFirstMax;
        if (nValidMin < nTotal - nSecondMax) {
            nValidMin = nTotal - nSecondMax;
        }
        if (nValidMax > nTotal - nSecondMin) {
            nValidMax = nTotal - nSecondMin;
        }
        if (nValidMax < nValidMin) {
            //配置的最小值/最大值互相冲突时，优先保证两个控件的最小值
            nValidMax = nValidMin;
        }
        int32_t nNewValue = m_nLeftTopFixedValue.GetInt32() + nOffset;
        if (nNewValue < nValidMin) {
            nNewValue = nValidMin;
        }
        if (nNewValue > nValidMax) {
            nNewValue = nValidMax;
        }
        pControl1 = m_pLeftTop;
        pControl2 = m_pRightBottom;
        nNewValue1 = nNewValue;
        nNewValue2 = nTotal - nNewValue;
    }
    else if (bLeftTopFixed) {
        pControl1 = m_pLeftTop;
        nNewValue1 = CalculateControlPos(nTotal, nOffset, m_pLeftTop, m_nLeftTopFixedValue, m_pRightBottom);
    }
    else if (bRightBottomFixed) {
        pControl2 = m_pRightBottom;
        nNewValue2 = CalculateControlPos(nTotal, -nOffset, m_pRightBottom, m_nRightBottomFixedValue, m_pLeftTop);
    }
    else {
        //两个控件都是拉伸类型的，不调整
        return bRet;
    }

    //只有尺寸发生变化时才设置新值，避免无效的布局和事件通知
    bool bAdjust1 = (pControl1 != nullptr) && (nNewValue1 >= 0) && (nNewValue1 != GetAxisSize(pControl1));
    bool bAdjust2 = (pControl2 != nullptr) && (nNewValue2 >= 0) && (nNewValue2 != GetAxisSize(pControl2));
    if (!bAdjust1 && !bAdjust2) {
        return bRet;
    }
    if (bAdjust1) {
        //两侧都需要调整时，第一侧先只修改值，由第二侧触发布局（避免每次鼠标移动都布局两次）
        SetAxisFixedSize(pControl1, nNewValue1, !bAdjust2);
    }
    if (bAdjust2) {
        SetAxisFixedSize(pControl2, nNewValue2, true);
    }
    this->SendEvent(kEventSplitDraged, (WPARAM)pControl1, (LPARAM)pControl2);
    return bRet;
}

template<typename InheritType>
void SplitTemplate<InheritType>::OnMouseMoveSingle(const EventArgs& msg)
{
    //只有一侧存在相邻控件时，才调整该控件的大小
    if ((m_pLeftTop == nullptr) == (m_pRightBottom == nullptr)) {
        return;
    }
    Control* pControl = (m_pLeftTop != nullptr) ? m_pLeftTop : m_pRightBottom;
    const UiFixedInt& nFixedInt = (m_pLeftTop != nullptr) ? m_nLeftTopFixedValue : m_nRightBottomFixedValue;
    if (nFixedInt.IsStretch()) {
        //控件为拉伸类型，不调整
        return;
    }

    int32_t nOffset = 0; //拖动的范围（宽度或者高度的偏移量, 可以是正数，也可以是负数）
    if (m_bHLayout) {
        nOffset = msg.ptMouse.x - m_ptStart.x;
    }
    else {
        nOffset = msg.ptMouse.y - m_ptStart.y;
    }
    if (nOffset == 0) {
        return;
    }

    //新值：不小于最小值，不大于最大值，并且不能超过父容器的可用空间
    int32_t nNewValue = nFixedInt.GetInt32() + nOffset;
    int32_t nMin = GetAxisMinSize(pControl);
    int32_t nMax = GetAxisMaxSize(pControl);
    int32_t nParentMax = GetParentAvailableSize(pControl);
    if (nMax > nParentMax) {
        nMax = nParentMax;
    }
    if (nMax < nMin) {
        nMax = nMin;
    }
    if (nNewValue < nMin) {
        nNewValue = nMin;
    }
    if (nNewValue > nMax) {
        nNewValue = nMax;
    }
    if (nNewValue == GetAxisSize(pControl)) {
        //尺寸没有变化，不需要调整
        return;
    }
    SetAxisFixedSize(pControl, nNewValue, true);
    this->SendEvent(kEventSplitDraged, (WPARAM)pControl, (LPARAM)nullptr);
}

typedef SplitTemplate<Control> Split;
typedef SplitTemplate<Box> SplitBox;

}//name space ui

#endif // UI_CONTROL_SPLIT_H_
