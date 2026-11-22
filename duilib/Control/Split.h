#ifndef UI_CONTROL_SPLIT_H_
#define UI_CONTROL_SPLIT_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/Label.h"
#include "duilib/Animation/AnimationPlayer.h"

#include <memory>

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
*   属性 double_click_reset 为 true 时，双击分割条可以把两侧控件恢复到首次拖动前的尺寸；恢复过程带短暂的尺寸过渡动画，
*   动画时长由属性 reset_anim 控制（单位毫秒，默认为 150，0 表示不使用动画）。
*   属性 drag_threshold 用于设置拖动阈值：鼠标在布局方向上移动超过该距离后才开始调整尺寸（单位是 96 DPI 的逻辑像素，默认为 2，0 表示不启用阈值）。
*   属性 left_top_ctrl_name / right_bottom_ctrl_name 可以显式指定要调整的两个控件的名称（在同一个窗口内查找），
*   不指定时默认使用同一父容器中、位于分割条前面和后面的两个可见控件；指定目标控件后支持跨容器调整尺寸。
*   控件获得焦点后，支持键盘操作：按布局方向的方向键微调尺寸（Shift + 方向键为大步进），步长由属性 keyboard_step 控制。
*   属性 drag_size_tip 为 true 时，拖动过程中在分割条附近显示当前尺寸提示（默认不显示）。
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

    /** 设置双击复位时的过渡动画时长
    * @param [in] nMillSeconds 动画时长（毫秒），0 表示不使用动画，默认值为 150
    */
    void SetResetAnimMillSeconds(int32_t nMillSeconds);

    /** 获取双击复位时的过渡动画时长（毫秒）
    */
    int32_t GetResetAnimMillSeconds() const;

    /** 设置拖动阈值（鼠标移动超过该距离后才开始调整控件尺寸，避免轻微抖动导致尺寸变化）
    * @param [in] nDragThreshold 阈值，单位是 96 DPI 的逻辑像素，0 表示不启用阈值，默认值为 2
    */
    void SetDragThreshold(int32_t nDragThreshold);

    /** 获取拖动阈值（单位是 96 DPI 的逻辑像素）
    */
    int32_t GetDragThreshold() const;

    /** 设置左侧（上侧）目标控件的名称
    * @param [in] strCtrlName 目标控件的名称，在同一个窗口内查找；设置为空时，使用同一父容器中的前一个可见控件
    */
    void SetLeftTopCtrlName(const DString& strCtrlName);

    /** 获取左侧（上侧）目标控件的名称
    */
    const DString& GetLeftTopCtrlName() const;

    /** 设置右侧（下侧）目标控件的名称
    * @param [in] strCtrlName 目标控件的名称，在同一个窗口内查找；设置为空时，使用同一父容器中的后一个可见控件
    */
    void SetRightBottomCtrlName(const DString& strCtrlName);

    /** 获取右侧（下侧）目标控件的名称
    */
    const DString& GetRightBottomCtrlName() const;

    /** 设置键盘操作时，每次按方向键调整的尺寸（单位是 96 DPI 的逻辑像素）
    * @param [in] nKeyboardStep 调整步长，0 表示不启用键盘操作，默认值为 10；按 Shift + 方向键时为 10 倍步长
    */
    void SetKeyboardStep(int32_t nKeyboardStep);

    /** 获取键盘操作时，每次按方向键调整的尺寸（单位是 96 DPI 的逻辑像素）
    */
    int32_t GetKeyboardStep() const;

    /** 设置拖动过程中，是否在分割条附近显示当前尺寸的提示
    * @param [in] bDragSizeTip true表示显示尺寸提示，false表示不显示，默认值为false
    */
    void SetDragSizeTip(bool bDragSizeTip);

    /** 获取拖动过程中，是否在分割条附近显示当前尺寸的提示
    */
    bool IsDragSizeTip() const;

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
    virtual bool OnKeyDown(const EventArgs& msg) override;

private:
    //是否为垂直布局的容器
    bool IsVLayout(Control* pControl) const;

    //是否为水平布局的容器
    bool IsHLayout(Control* pControl) const;

    /** 判断拖动阈值是否已达到（达到后即开始调整控件尺寸）
    * @param [in] nOffset 鼠标在布局方向上的偏移量（像素）
    * @return true表示已达到阈值，可以调整尺寸；false表示尚未达到阈值
    */
    bool IsDragThresholdExceeded(int32_t nOffset);

    /** 按名称查找目标控件（在同一个窗口内查找，并校验其可用性）
    * @param [in] strCtrlName 控件名称
    * @return 返回查找到的控件，未查找到或者控件不可用时返回nullptr
    */
    Control* FindTargetControl(const DString& strCtrlName) const;

    /** 准备调整尺寸所需的目标控件和初始尺寸（鼠标按下或者键盘操作前调用）
    * @return true表示可以调整尺寸；false表示当前不可调整
    */
    bool PrepareDragTargets();

    /** 按布局方向的偏移量调整两侧控件的尺寸（鼠标拖动与键盘操作共用）
    * @param [in] nOffset 偏移量（像素），正数表示向右/向下移动分割条
    */
    void ApplyDragOffset(int32_t nOffset);

    /** 只有一侧控件时，按偏移量调整该控件的尺寸（鼠标拖动与键盘操作共用）
    * @param [in] nOffset 偏移量（像素），正数表示向右/向下移动分割条
    */
    void ApplyDragOffsetSingle(int32_t nOffset);

    /** 显示拖动尺寸提示
    * @param [in] nSize 当前调整的控件在布局方向上的尺寸
    */
    void ShowDragSizeTip(int32_t nSize);

    /** 隐藏拖动尺寸提示
    */
    void HideDragSizeTip();

    /** 计算双击复位时，左侧（上侧）控件在布局方向上的目标尺寸
    * @param [in] pPrev 左侧（上侧）控件
    * @param [in] pNext 右侧（下侧）控件
    * @param [in] nTotal 两侧控件当前的尺寸之和
    * @return 返回目标尺寸；无法计算时返回 -1
    */
    int32_t CalcResetTargetSize(const Control* pPrev, const Control* pNext, int32_t nTotal) const;

    /** 启动双击复位的尺寸动画
    * @param [in] pPrev 左侧（上侧）控件
    * @param [in] pNext 右侧（下侧）控件
    * @param [in] nFrom 动画起始尺寸（左侧/上侧控件）
    * @param [in] nTo 动画结束尺寸（左侧/上侧控件）
    * @param [in] nTotal 两侧控件当前的尺寸之和（只有一侧控件时为 0）
    */
    void StartResetAnim(Control* pPrev, Control* pNext, int32_t nFrom, int32_t nTo, int32_t nTotal);

    /** 停止双击复位的尺寸动画（不改变控件当前的尺寸）
    */
    void StopResetAnim();

    /** 双击复位动画的每一帧：设置两侧控件的尺寸（不恢复原始类型）
    * @param [in] nValue 左侧（上侧）控件在当前帧的尺寸
    */
    void OnResetAnimFrame(int32_t nValue);

    /** 双击复位动画结束时：恢复两侧控件的原始尺寸类型
    */
    void OnResetAnimFinished();

    /** 立即把两侧控件恢复到首次拖动前的尺寸（不使用动画）
    */
    void ApplyResetImmediately();

    /** 查找同级的相邻控件（前一个和后一个可见的、非float的控件）
    *   如果通过 left_top_ctrl_name / right_bottom_ctrl_name 指定了目标控件，则优先使用指定的控件
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

    //拖动阈值：鼠标移动超过该距离后才开始调整控件尺寸（单位是 96 DPI 的逻辑像素）
    int32_t m_nDragThreshold;

    //本次拖动是否已经超过拖动阈值
    bool m_bDragThresholdExceeded;

    //左侧（上侧）目标控件的名称（为空时，使用同一父容器中的前一个可见控件）
    DString m_sLeftTopCtrlName;

    //右侧（下侧）目标控件的名称（为空时，使用同一父容器中的后一个可见控件）
    DString m_sRightBottomCtrlName;

    //键盘操作时，每次按方向键调整的尺寸（单位是 96 DPI 的逻辑像素，0 表示不启用键盘操作）
    int32_t m_nKeyboardStep;

    //拖动过程中，是否在分割条附近显示当前尺寸的提示
    bool m_bDragSizeTip;

    //拖动尺寸提示的控件（拖动过程中动态创建，随父容器一起销毁）
    ControlPtr m_pDragSizeTip;

    /** 当只有一个控件的时候，是否允许调整其宽度
    */
    bool m_bEnableSplitSingle;

    /** 双击分割条时，是否把两侧控件恢复到首次拖动前的尺寸
    */
    bool m_bDoubleClickReset;

    /** 双击复位时的过渡动画时长（毫秒），0表示不使用动画
    */
    int32_t m_nResetAnimMs;

    /** 双击复位动画对象
    */
    std::unique_ptr<AnimationPlayer> m_pResetAnimPlayer;

    /** 双击复位动画中，左侧（上侧）控件的当前尺寸 */
    ControlPtr m_pResetAnimPrev;

    /** 双击复位动画中，右侧（下侧）控件的当前尺寸 */
    ControlPtr m_pResetAnimNext;

    /** 双击复位动画中，两侧控件当前的尺寸之和（0表示只有一侧控件） */
    int32_t m_nResetAnimTotal;
};

template<typename InheritType>
SplitTemplate<InheritType>::SplitTemplate(Window* pWindow):
    InheritType(pWindow),
    m_pLeftTop(nullptr),
    m_pRightBottom(nullptr),
    m_bInitSizeCaptured(false),
    m_bHLayout(true),
    m_bDragStarted(false),
    m_nDragThreshold(2),
    m_bDragThresholdExceeded(false),
    m_nKeyboardStep(10),
    m_bDragSizeTip(false),
    m_bEnableSplitSingle(false),
    m_bDoubleClickReset(false),
    m_nResetAnimMs(150),
    m_nResetAnimTotal(0)
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
    else if (strName == _T("reset_anim")) {
        //双击复位时的过渡动画时长（毫秒，0表示不使用动画）
        SetResetAnimMillSeconds(StringUtil::StringToInt32(strNewValue));
    }
    else if (strName == _T("drag_threshold")) {
        SetDragThreshold(StringUtil::StringToInt32(strNewValue));
    }
    else if (strName == _T("left_top_ctrl_name")) {
        SetLeftTopCtrlName(strNewValue);
    }
    else if (strName == _T("right_bottom_ctrl_name")) {
        SetRightBottomCtrlName(strNewValue);
    }
    else if (strName == _T("keyboard_step")) {
        SetKeyboardStep(StringUtil::StringToInt32(strNewValue));
    }
    else if (strName == _T("drag_size_tip")) {
        SetDragSizeTip(StringUtil::IsValueTrue(strNewValue));
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
void SplitTemplate<InheritType>::SetResetAnimMillSeconds(int32_t nMillSeconds)
{
    if (nMillSeconds < 0) {
        nMillSeconds = 0;
    }
    m_nResetAnimMs = nMillSeconds;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetResetAnimMillSeconds() const
{
    return m_nResetAnimMs;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetDragThreshold(int32_t nDragThreshold)
{
    if (nDragThreshold < 0) {
        nDragThreshold = 0;
    }
    m_nDragThreshold = nDragThreshold;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetDragThreshold() const
{
    return m_nDragThreshold;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetLeftTopCtrlName(const DString& strCtrlName)
{
    m_sLeftTopCtrlName = strCtrlName;
}

template<typename InheritType>
const DString& SplitTemplate<InheritType>::GetLeftTopCtrlName() const
{
    return m_sLeftTopCtrlName;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetRightBottomCtrlName(const DString& strCtrlName)
{
    m_sRightBottomCtrlName = strCtrlName;
}

template<typename InheritType>
const DString& SplitTemplate<InheritType>::GetRightBottomCtrlName() const
{
    return m_sRightBottomCtrlName;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetKeyboardStep(int32_t nKeyboardStep)
{
    if (nKeyboardStep < 0) {
        nKeyboardStep = 0;
    }
    m_nKeyboardStep = nKeyboardStep;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::GetKeyboardStep() const
{
    return m_nKeyboardStep;
}

template<typename InheritType>
void SplitTemplate<InheritType>::SetDragSizeTip(bool bDragSizeTip)
{
    m_bDragSizeTip = bDragSizeTip;
    if (!m_bDragSizeTip) {
        HideDragSizeTip();
    }
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsDragSizeTip() const
{
    return m_bDragSizeTip;
}

template<typename InheritType>
void SplitTemplate<InheritType>::ShowDragSizeTip(int32_t nSize)
{
    if (!m_bDragSizeTip) {
        return;
    }
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return;
    }
    const DpiManager& dpi = this->Dpi();
    //提示框的宽度和高度（固定尺寸：避免设置文本时触发父容器重新布局，导致浮动控件的位置被重置）
    const int32_t nTipWidth = dpi.GetScaleInt(76);
    const int32_t nTipHeight = dpi.GetScaleInt(22);

    Label* pTip = dynamic_cast<Label*>(m_pDragSizeTip.get());
    if (pTip == nullptr) {
        //首次显示时，动态创建提示控件（float 控件，不参与父容器的布局）
        pTip = new Label(this->GetWindow());
        pTip->SetFloat(true);
        //保持外部设置的位置：父容器布局时，浮动控件的位置不会被重置
        pTip->SetKeepFloatPos(true);
        pTip->SetMouseEnabled(false);
        pTip->SetBkColor(_T("#E6323232"));
        const int32_t nRound = dpi.GetScaleInt(4);
        pTip->SetBorderRound(UiSize(nRound, nRound));
        pTip->SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER, false);
        pTip->SetStateTextColor(kControlStateNormal, _T("#FFFFFFFF"));
        //宽度和高度使用固定值（而非默认的auto），设置文本时只重绘，不触发布局重排
        pTip->SetFixedWidth(UiFixedInt(nTipWidth), false, false);
        pTip->SetFixedHeight(UiFixedInt(nTipHeight), false, false);
        pParent->AddItem(pTip);
        m_pDragSizeTip = pTip;
        if (m_pDragSizeTip.get() == nullptr) {
            return;
        }
    }

    //计算提示框的位置：显示在分割条的中部，横向布局在右侧、纵向布局在下方；空间不足时放到另一侧
    //注意：SetPos 使用的是窗口坐标系（与 GetPos/GetRect 一致）
    const UiRect rcSplit = this->GetPos();
    const UiRect rcParent = pParent->GetRect();
    const int32_t nMargin = dpi.GetScaleInt(6);
    int32_t nLeft = 0;
    int32_t nTop = 0;
    if (m_bHLayout) {
        nLeft = rcSplit.right + nMargin;
        if (nLeft + nTipWidth > rcParent.right) {
            nLeft = rcSplit.left - nTipWidth - nMargin;
        }
        nTop = rcSplit.top + (rcSplit.Height() - nTipHeight) / 2;
    }
    else {
        nTop = rcSplit.bottom + nMargin;
        if (nTop + nTipHeight > rcParent.bottom) {
            nTop = rcSplit.top - nTipHeight - nMargin;
        }
        nLeft = rcSplit.left + (rcSplit.Width() - nTipWidth) / 2;
    }
    //限制在父容器范围内（提示框是父容器的子控件，超出部分会被裁剪）
    if (nLeft + nTipWidth > rcParent.right) {
        nLeft = rcParent.right - nTipWidth;
    }
    if (nTop + nTipHeight > rcParent.bottom) {
        nTop = rcParent.bottom - nTipHeight;
    }
    if (nLeft < rcParent.left) {
        nLeft = rcParent.left;
    }
    if (nTop < rcParent.top) {
        nTop = rcParent.top;
    }
    pTip->SetPos(UiRect(nLeft, nTop, nLeft + nTipWidth, nTop + nTipHeight));
    //显示的数值使用 96 DPI 的逻辑像素（与XML中配置的尺寸单位一致）
    pTip->SetText(StringUtil::Printf(_T("%d px"), dpi.GetUnscaleInt(nSize)));
    pTip->SetVisible(true);
}

template<typename InheritType>
void SplitTemplate<InheritType>::HideDragSizeTip()
{
    Control* pTip = m_pDragSizeTip.get();
    if (pTip != nullptr) {
        pTip->SetVisible(false);
    }
}

template<typename InheritType>
Control* SplitTemplate<InheritType>::FindTargetControl(const DString& strCtrlName) const
{
    if (strCtrlName.empty()) {
        return nullptr;
    }
    Window* pWindow = this->GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    Control* pControl = pWindow->FindControl(strCtrlName);
    const Control* pThisControl = static_cast<const Control*>(this);
    if ((pControl == nullptr) || (pControl == pThisControl) || (!pControl->IsVisible()) || pControl->IsFloat()) {
        return nullptr;
    }
    return pControl;
}

template<typename InheritType>
bool SplitTemplate<InheritType>::IsDragThresholdExceeded(int32_t nOffset)
{
    if (m_bDragThresholdExceeded) {
        return true;
    }
    if (m_nDragThreshold > 0) {
        //阈值的单位是 96 DPI 的逻辑像素，需要按当前 DPI 缩放后与物理像素的偏移量比较
        const int32_t nThreshold = this->Dpi().GetScaleInt(m_nDragThreshold);
        const int32_t nAbsOffset = (nOffset >= 0) ? nOffset : -nOffset;
        if (nAbsOffset < nThreshold) {
            return false;
        }
    }
    m_bDragThresholdExceeded = true;
    return true;
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
    HideDragSizeTip();

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
    //目标控件可能通过 left_top_ctrl_name / right_bottom_ctrl_name 指定到其它容器中，
    //此时应以目标控件所在父容器的可用空间计算
    Control* pTargetParent = (pControl != nullptr) ? pControl->GetParent() : nullptr;
    Box* pBox = dynamic_cast<Box*>(pTargetParent);
    const bool bSameParent = (pTargetParent == pParent);
    if (pBox != nullptr) {
        pParent = pBox;
    }

    UiRect rcParent = pParent->GetPosWithoutPadding();
    int32_t nAvailableSize = m_bHLayout ? rcParent.Width() : rcParent.Height();
    if (bSameParent) {
        //扣除分割条自身占用的空间，保证调整后控件不会把分割条挤出父容器
        nAvailableSize -= (m_bHLayout ? this->GetWidth() : this->GetHeight());
    }

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

    //先查找同一父容器中位于分割条前面/后面的相邻控件
    Control* pPrevSibling = nullptr;
    Control* pNextSibling = nullptr;
    Box* pParent = this->GetParent();
    if (pParent != nullptr) {
        size_t nChildCount = pParent->GetItemCount();
        bool bFoundThis = false;
        for (size_t i = 0; i < nChildCount; ++i) {
            Control* pControl = pParent->GetItemAt(i);
            if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
                continue;
            }
            if (bFoundThis) {
                pNextSibling = pControl;
                break;
            }
            const Control* pThisControl = static_cast<const Control*>(this);
            if (pControl == pThisControl) {
                bFoundThis = true;
            }
            else {
                pPrevSibling = pControl;
            }
        }
        if (!bFoundThis) {
            //当前控件不是父容器的直接子控件，无法确定相邻控件
            pPrevSibling = nullptr;
            pNextSibling = nullptr;
        }
    }

    //如果指定了目标控件的名称，则优先使用指定的控件（支持跨容器调整）
    if (m_sLeftTopCtrlName.empty()) {
        pPrev = pPrevSibling;
    }
    else {
        pPrev = FindTargetControl(m_sLeftTopCtrlName);
    }
    if (m_sRightBottomCtrlName.empty()) {
        pNext = pNextSibling;
    }
    else {
        pNext = FindTargetControl(m_sRightBottomCtrlName);
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
    //新的拖动开始前，停止尚未结束的复位动画，避免动画与拖动争抢控件尺寸
    StopResetAnim();

    if (!this->IsEnabled()) {
        return bRet;
    }
    bool bStartDrag = false;
    if (PrepareDragTargets()) {
        m_ptStart = msg.ptMouse;
        m_bDragThresholdExceeded = false;
        //获取焦点后，支持键盘操作（方向键微调尺寸）
        this->SetFocus();

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
bool SplitTemplate<InheritType>::PrepareDragTargets()
{
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return false;
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
    if (!bLayoutValid) {
        return false;
    }

    m_pLeftTop = pPrev;
    m_pRightBottom = pNext;
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
    return true;
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

    //先停止可能尚未结束的复位动画
    StopResetAnim();

    if ((m_nResetAnimMs <= 0) || ((pPrev == nullptr) && (pNext == nullptr))) {
        ApplyResetImmediately();
        return bRet;
    }

    //计算动画的起始尺寸和目标尺寸（动画值代表左侧/上侧控件的尺寸）
    const int32_t nFrom = (pPrev != nullptr) ? GetAxisSize(pPrev) : GetAxisSize(pNext);
    const int32_t nTotal = ((pPrev != nullptr) && (pNext != nullptr)) ? (GetAxisSize(pPrev) + GetAxisSize(pNext)) : 0;
    const int32_t nTo = CalcResetTargetSize(pPrev, pNext, nTotal);
    if (nTo < 0) {
        //无法预估动画目标（例如只有一侧控件且恢复为拉伸类型），直接复位
        ApplyResetImmediately();
        return bRet;
    }
    if (nFrom == nTo) {
        //尺寸没有变化，直接复位（恢复控件的原始尺寸类型）
        ApplyResetImmediately();
        return bRet;
    }
    StartResetAnim(pPrev, pNext, nFrom, nTo, nTotal);
    return bRet;
}

template<typename InheritType>
int32_t SplitTemplate<InheritType>::CalcResetTargetSize(const Control* pPrev, const Control* pNext, int32_t nTotal) const
{
    //确定动画对象：优先左侧（上侧）控件；只有右侧（下侧）控件时使用它
    const Control* pFirst = (pPrev != nullptr) ? pPrev : pNext;
    if (pFirst == nullptr) {
        return -1;
    }
    const UiFixedInt& initFirst = (pPrev != nullptr) ? m_nLeftTopInitFixedValue : m_nRightBottomInitFixedValue;
    const UiFixedInt& initOther = (pPrev != nullptr) ? m_nRightBottomInitFixedValue : m_nLeftTopInitFixedValue;

    int32_t nTarget = -1;
    if (!initFirst.IsStretch()) {
        //恢复为固定尺寸：直接使用首次拖动前记录的值
        nTarget = initFirst.GetInt32();
    }
    else if ((pPrev != nullptr) && (pNext != nullptr) && !initOther.IsStretch()) {
        //恢复为拉伸类型：用两侧尺寸之和减去另一侧的固定尺寸
        nTarget = nTotal - initOther.GetInt32();
    }
    else if ((pPrev != nullptr) && (pNext != nullptr)) {
        //两侧都恢复为拉伸类型：按均分计算
        nTarget = nTotal / 2;
    }
    else {
        //只有一侧控件且恢复为拉伸类型：最终尺寸由父容器决定，无法预估
        return -1;
    }
    //限制在控件允许的最小/最大尺寸范围内
    const int32_t nMinSize = GetAxisMinSize(pFirst);
    const int32_t nMaxSize = GetAxisMaxSize(pFirst);
    if (nTarget < nMinSize) {
        nTarget = nMinSize;
    }
    if (nTarget > nMaxSize) {
        nTarget = nMaxSize;
    }
    return nTarget;
}

template<typename InheritType>
void SplitTemplate<InheritType>::StartResetAnim(Control* pPrev, Control* pNext, int32_t nFrom, int32_t nTo, int32_t nTotal)
{
    //动画期间把两侧控件都转换为固定尺寸，避免父容器的自动布局打断动画
    if (pPrev != nullptr) {
        SetAxisFixedSize(pPrev, nFrom, false);
    }
    if (pNext != nullptr) {
        const int32_t nNext = (nTotal > 0) ? (nTotal - nFrom) : GetAxisSize(pNext);
        SetAxisFixedSize(pNext, nNext, false);
    }
    //触发布局：优先通过目标控件触发，兼容目标控件位于其它容器的情况
    Control* pArrangeControl = (pNext != nullptr) ? pNext : pPrev;
    if (pArrangeControl != nullptr) {
        pArrangeControl->ArrangeAncestor();
    }
    else {
        this->ArrangeAncestor();
    }

    m_pResetAnimPrev = pPrev;
    m_pResetAnimNext = pNext;
    m_nResetAnimTotal = nTotal;

    AnimationPlayer* pAnim = new AnimationPlayer;
    m_pResetAnimPlayer.reset(pAnim);
    pAnim->SetAnimationType(AnimationType::kAnimationNone);
    //缓出效果：复位过程先快后慢，符合"回到默认尺寸"的操作直觉
    pAnim->SetEasingFunctionType(EaseOutCubic);
    pAnim->SetTotalMillSeconds(m_nResetAnimMs);
    pAnim->SetFrameIntervalMillSeconds(16);
    pAnim->SetStartValue(nFrom);
    pAnim->SetEndValue(nTo);

    ControlPtrT<SplitTemplate<InheritType>> pThis(this);
    pAnim->SetPlayCallback([pThis](int32_t nValue) {
        if (pThis == nullptr) {
            return;
        }
        pThis->OnResetAnimFrame(nValue);
    });
    pAnim->SetCompleteCallback([pThis]() {
        if (pThis == nullptr) {
            return;
        }
        pThis->OnResetAnimFinished();
    });
    pAnim->Start();
}

template<typename InheritType>
void SplitTemplate<InheritType>::StopResetAnim()
{
    if (m_pResetAnimPlayer != nullptr) {
        m_pResetAnimPlayer->SetPlayCallback(nullptr);
        m_pResetAnimPlayer->SetCompleteCallback(nullptr);
        m_pResetAnimPlayer->Stop();
        m_pResetAnimPlayer.reset();
    }
    m_pResetAnimPrev = nullptr;
    m_pResetAnimNext = nullptr;
    m_nResetAnimTotal = 0;
}

template<typename InheritType>
void SplitTemplate<InheritType>::OnResetAnimFrame(int32_t nValue)
{
    Control* pPrev = m_pResetAnimPrev.get();
    Control* pNext = m_pResetAnimNext.get();
    if ((pPrev == nullptr) && (pNext == nullptr)) {
        return;
    }
    if ((pPrev != nullptr) && (pNext != nullptr)) {
        //两侧控件一起动画：两者尺寸之和保持不变
        SetAxisFixedSize(pPrev, nValue, false);
        SetAxisFixedSize(pNext, m_nResetAnimTotal - nValue, true);
    }
    else if (pPrev != nullptr) {
        SetAxisFixedSize(pPrev, nValue, true);
    }
    else if (pNext != nullptr) {
        SetAxisFixedSize(pNext, nValue, true);
    }
}

template<typename InheritType>
void SplitTemplate<InheritType>::OnResetAnimFinished()
{
    m_pResetAnimPlayer.reset();

    //动画结束：恢复两侧控件的原始尺寸类型（固定/拉伸/自动）
    bool bAdjusted = false;
    Control* pPrev = m_pResetAnimPrev.get();
    if ((pPrev != nullptr) && ResetAxisFixedSize(pPrev, m_nLeftTopInitFixedValue, false)) {
        bAdjusted = true;
    }
    Control* pNext = m_pResetAnimNext.get();
    if ((pNext != nullptr) && ResetAxisFixedSize(pNext, m_nRightBottomInitFixedValue, false)) {
        bAdjusted = true;
    }
    m_pResetAnimPrev = nullptr;
    m_pResetAnimNext = nullptr;
    m_nResetAnimTotal = 0;

    //统一触发布局，确保恢复拉伸/自动类型后的最终位置正确
    //（优先通过目标控件触发，兼容目标控件位于其它容器的情况）
    Control* pArrangeControl = (pPrev != nullptr) ? pPrev : pNext;
    if (pArrangeControl != nullptr) {
        pArrangeControl->ArrangeAncestor();
    }
    else {
        this->ArrangeAncestor();
    }
    if (bAdjusted) {
        this->Invalidate();
    }
}

template<typename InheritType>
void SplitTemplate<InheritType>::ApplyResetImmediately()
{
    Control* pPrev = nullptr;
    Control* pNext = nullptr;
    FindPrevNextControl(pPrev, pNext);

    bool bAdjusted = false;
    if ((pPrev != nullptr) && ResetAxisFixedSize(pPrev, m_nLeftTopInitFixedValue, false)) {
        bAdjusted = true;
    }
    if ((pNext != nullptr) && ResetAxisFixedSize(pNext, m_nRightBottomInitFixedValue, false)) {
        bAdjusted = true;
    }
    if (bAdjusted) {
        //两个控件的值都恢复后，统一触发布局（优先通过目标控件触发，兼容目标控件位于其它容器的情况）
        Control* pArrangeControl = (pPrev != nullptr) ? pPrev : pNext;
        if (pArrangeControl != nullptr) {
            pArrangeControl->ArrangeAncestor();
        }
        else {
            this->ArrangeAncestor();
        }
        this->Invalidate();
    }
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
bool SplitTemplate<InheritType>::OnKeyDown(const EventArgs& msg)
{
    bool bRet = BaseClass::OnKeyDown(msg);
    if (bRet || msg.IsSenderExpired()) {
        return bRet;
    }
    if ((msg.eventType != kEventKeyDown) || !this->IsEnabled() || !this->IsKeyboardEnabled() || (m_nKeyboardStep <= 0)) {
        return bRet;
    }
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return bRet;
    }
    const bool bHLayout = IsHLayout(pParent);
    if (!bHLayout && !IsVLayout(pParent)) {
        return bRet;
    }

    //只有布局方向上的方向键才处理：横向布局用左右键，纵向布局用上下键
    int32_t nDirection = 0;
    if (bHLayout) {
        if (msg.vkCode == kVK_LEFT) {
            nDirection = -1;
        }
        else if (msg.vkCode == kVK_RIGHT) {
            nDirection = 1;
        }
    }
    else {
        if (msg.vkCode == kVK_UP) {
            nDirection = -1;
        }
        else if (msg.vkCode == kVK_DOWN) {
            nDirection = 1;
        }
    }
    if (nDirection == 0) {
        return bRet;
    }

    //默认按步长调整，Shift + 方向键时为大步进（10 倍步长）
    int32_t nStep = this->Dpi().GetScaleInt(m_nKeyboardStep);
    if ((msg.modifierKey & kShift) != 0) {
        nStep *= 10;
    }
    if (nStep <= 0) {
        nStep = 1;
    }
    //键盘调整前，停止尚未结束的复位动画，避免动画与键盘操作争抢控件尺寸
    StopResetAnim();
    if (!PrepareDragTargets()) {
        return bRet;
    }
    const int32_t nOffset = nDirection * nStep;
    if ((m_pLeftTop != nullptr) && (m_pRightBottom != nullptr)) {
        ApplyDragOffset(nOffset);
    }
    else {
        ApplyDragOffsetSingle(nOffset);
    }
    return true;
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

    //鼠标移动未超过拖动阈值时，不调整尺寸（避免轻微抖动导致尺寸变化）
    if (!IsDragThresholdExceeded(nOffset)) {
        return bRet;
    }
    ApplyDragOffset(nOffset);
    //显示当前尺寸提示（仅鼠标拖动时显示）
    Control* pTipControl = (m_pLeftTop != nullptr) ? m_pLeftTop : m_pRightBottom;
    if (pTipControl != nullptr) {
        ShowDragSizeTip(GetAxisSize(pTipControl));
    }
    return bRet;
}

template<typename InheritType>
void SplitTemplate<InheritType>::ApplyDragOffset(int32_t nOffset)
{
    if ((m_pLeftTop == nullptr) || (m_pRightBottom == nullptr)) {
        return;
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
        return;
    }

    //只有尺寸发生变化时才设置新值，避免无效的布局和事件通知
    bool bAdjust1 = (pControl1 != nullptr) && (nNewValue1 >= 0) && (nNewValue1 != GetAxisSize(pControl1));
    bool bAdjust2 = (pControl2 != nullptr) && (nNewValue2 >= 0) && (nNewValue2 != GetAxisSize(pControl2));
    if (!bAdjust1 && !bAdjust2) {
        return;
    }
    if (bAdjust1) {
        //两侧都需要调整时，第一侧先只修改值，由第二侧触发布局（避免每次鼠标移动都布局两次）
        SetAxisFixedSize(pControl1, nNewValue1, !bAdjust2);
    }
    if (bAdjust2) {
        SetAxisFixedSize(pControl2, nNewValue2, true);
    }
    this->SendEvent(kEventSplitDraged, (WPARAM)pControl1, (LPARAM)pControl2);
}

template<typename InheritType>
void SplitTemplate<InheritType>::OnMouseMoveSingle(const EventArgs& msg)
{
    //只有一侧存在相邻控件时，才调整该控件的大小
    if ((m_pLeftTop == nullptr) == (m_pRightBottom == nullptr)) {
        return;
    }
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

    //鼠标移动未超过拖动阈值时，不调整尺寸（避免轻微抖动导致尺寸变化）
    if (!IsDragThresholdExceeded(nOffset)) {
        return;
    }
    ApplyDragOffsetSingle(nOffset);
    //显示当前尺寸提示（仅鼠标拖动时显示）
    Control* pTipControl = (m_pLeftTop != nullptr) ? m_pLeftTop : m_pRightBottom;
    if (pTipControl != nullptr) {
        ShowDragSizeTip(GetAxisSize(pTipControl));
    }
}

template<typename InheritType>
void SplitTemplate<InheritType>::ApplyDragOffsetSingle(int32_t nOffset)
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
