#ifndef UI_CONTROL_GROUPBOX_H_
#define UI_CONTROL_GROUPBOX_H_

#include "duilib/Control/Label.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Image/Image.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/ControlPtrT.h"

#include <memory>

namespace ui
{

/** 分组容器
*/
template<typename InheritType>
class GroupBoxTemplate : public LabelTemplate<InheritType>
{
    typedef LabelTemplate<InheritType> BaseClass;
public:
    explicit GroupBoxTemplate(Window* pWindow);
    virtual ~GroupBoxTemplate() override;
        
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual UiEstSize EstimateSize(UiSize szAvailable) override;
    virtual void SetPos(UiRect rc) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintText(IRender* pRender) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual Control* FindControl(FINDCONTROLPROC Proc, void* pProcData, uint32_t uFlags,
                                 const UiPoint& ptMouse, const UiPoint& scrollPos) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 设置圆角大小
     */
    void SetCornerSize(UiSize cxyRound, bool bNeedDpiScale);

    /** 获取圆角大小
    */
    const UiSize& GetCornerSize() const;

    /** 设置线条宽度
    */
    void SetLineWidth(float fLineWidth, bool bNeedDpiScale);

    /** 获取线条宽度
    */
    float GetLineWidth() const;

    /** 设置线条颜色
    */
    void SetLineColor(const DString& lineColor);

    /** 获取线条颜色
    */
    const UiString& GetLineColor() const;

    /** 设置标题文字与边框线之间的间距（左右各留出该间距）
     * @param [in] nTextGap 间距值（像素）
     * @param [in] bNeedDpiScale 是否需要做DPI缩放
     */
    void SetTextGap(int32_t nTextGap, bool bNeedDpiScale);

    /** 获取标题文字与边框线之间的间距
    */
    int32_t GetTextGap() const;

    /** 设置是否允许折叠（点击标题区域折叠/展开），默认不允许
     * @param [in] bCollapsible true表示允许折叠，false表示不允许
     */
    void SetCollapsible(bool bCollapsible);

    /** 是否允许折叠
    */
    bool IsCollapsible() const;

    /** 设置折叠状态（仅在允许折叠时生效）
     * @param [in] bCollapsed true表示折叠，false表示展开
     */
    void SetCollapsed(bool bCollapsed);

    /** 是否处于折叠状态
    */
    bool IsCollapsed() const;

    /** 切换折叠/展开状态
    */
    void ToggleCollapsed();

    /** 设置展开状态下显示的图标
     * @param [in] imageString 图片资源字符串（支持SVG/PNG等，写法与XML中的图片属性一致）
     */
    void SetCollapseIconExpanded(const DString& imageString);

    /** 获取展开状态下显示的图标
    */
    DString GetCollapseIconExpanded() const;

    /** 设置折叠状态下显示的图标
     * @param [in] imageString 图片资源字符串（支持SVG/PNG等，写法与XML中的图片属性一致）
     */
    void SetCollapseIconCollapsed(const DString& imageString);

    /** 获取折叠状态下显示的图标
    */
    DString GetCollapseIconCollapsed() const;

    /** 设置折叠/展开动画的时长
     * @param [in] nMillSeconds 动画时长（毫秒），0表示不使用动画
     */
    void SetCollapseAnimMillSeconds(int32_t nMillSeconds);

    /** 获取折叠/展开动画的时长（毫秒），0表示不使用动画
    */
    int32_t GetCollapseAnimMillSeconds() const;

private:
    /** 获取一定透明度的颜色
    */
    UiColor GetFadeColor(UiColor color, uint8_t nFade) const;

    /** 应用折叠/展开状态
     * @param [in] bCollapsed true表示折叠，false表示展开
     * @param [in] bPlayAnim 是否播放动画
     */
    void ApplyCollapsed(bool bCollapsed, bool bPlayAnim);

    /** 计算标题区域的高度（标题区域为第一个子控件上方的区域）
    */
    int32_t CalcTitleHeight() const;

    /** 估算展开状态下的高度
     * @param [in] nFallbackHeight 无法估算时使用的兜底高度
     */
    int32_t EstimateExpandedHeight(int32_t nFallbackHeight);

    /** 鼠标的纵坐标是否位于标题区域内
    */
    bool IsInTitleArea(int32_t nMouseY) const;

    /** 是否处于折叠后的静止状态（不包含折叠/展开动画过程）
    */
    bool IsCollapsedStatic() const;

    /** 开始折叠/展开动画
     * @param [in] bCollapse true表示折叠，false表示展开
     */
    void StartCollapseAnim(bool bCollapse);

    /** 停止折叠/展开动画
    */
    void StopCollapseAnim();

    /** 动画过程中高度变化的处理
    */
    void OnCollapseAnimHeight(int32_t nHeight);

    /** 折叠/展开动画结束的处理
    */
    void OnCollapseAnimFinished();

    /** 绘制折叠图标（未配置图标时绘制简单的箭头）
    */
    void PaintCollapseIcon(IRender* pRender, const UiRect& rcIcon, UiColor lineColor) const;

private:
    //线条宽度
    float m_fLineWidth;

    //圆角大小（默认为直角, 无圆角）
    UiSize m_cornerSize;

    //线条颜色
    UiString m_lineColor;

    //标题文字与边框线之间的间距（避免线条紧贴文字）
    int32_t m_nTextGap;

    //是否允许折叠（点击标题区域折叠/展开）
    bool m_bCollapsible;

    //是否处于折叠状态
    bool m_bCollapsed;

    //折叠/展开动画的时长（毫秒），0表示不使用动画
    int32_t m_nCollapseAnimMs;

    //动画过程中的当前高度（-1表示当前没有动画）
    int32_t m_nAnimHeight;

    //最近一次完全展开时的高度（用于无法估算展开高度时的兜底）
    int32_t m_nExpandedHeight;

    //展开布局时测量到的标题区域高度（0表示尚未测量）
    int32_t m_nTitleHeight;

    //折叠/展开动画对象
    std::unique_ptr<AnimationPlayer> m_pCollapseAnimPlayer;

    //展开状态下显示的图标
    std::unique_ptr<Image> m_pCollapseIconExpanded;

    //折叠状态下显示的图标
    std::unique_ptr<Image> m_pCollapseIconCollapsed;
};

template<typename InheritType>
GroupBoxTemplate<InheritType>::GroupBoxTemplate(Window* pWindow):
    LabelTemplate<InheritType>(pWindow),
    m_fLineWidth(0),
    m_nTextGap(0),
    m_bCollapsible(false),
    m_bCollapsed(false),
    m_nCollapseAnimMs(200),
    m_nAnimHeight(-1),
    m_nExpandedHeight(0),
    m_nTitleHeight(0)
{
    SetAttribute(_T("text_align"), _T("top,left"));
    SetAttribute(_T("text_padding"), _T("8,0,0,0"));
    //标题文字两侧默认留出间距，避免边框线紧贴文字
    SetTextGap(4, true);
}

template<typename InheritType>
GroupBoxTemplate<InheritType>::~GroupBoxTemplate()
{
    //析构前停止动画定时器，避免定时器回调访问已析构对象（回调本身有弱引用保护）
    StopCollapseAnim();
}

template<typename InheritType>
inline DString GroupBoxTemplate<InheritType>::GetType() const { return _T("GroupBoxTemplate"); }

template<>
inline DString GroupBoxTemplate<Box>::GetType() const { return DUI_CTR_GROUP_BOX; }

template<>
inline DString GroupBoxTemplate<HBox>::GetType() const { return DUI_CTR_GROUP_HBOX; }

template<>
inline DString GroupBoxTemplate<VBox>::GetType() const { return DUI_CTR_GROUP_VBOX; }

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCornerSize(UiSize cxyRound, bool bNeedDpiScale)
{
    int32_t cx = cxyRound.cx;
    int32_t cy = cxyRound.cy;
    ASSERT(cx >= 0);
    ASSERT(cy >= 0);
    if ((cx < 0) || (cy < 0)) {
        return;
    }
    //两个参数要么同时等于0，要么同时大于0，否则参数无效
    ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
    if (cx == 0) {
        if (cy != 0) {
            return;
        }
    }
    else {
        if (cy == 0) {
            return;
        }
    }
    if (bNeedDpiScale) {
        this->Dpi().ScaleSize(cxyRound);
    }
    if (m_cornerSize != cxyRound) {
        m_cornerSize = cxyRound;
        this->Invalidate();
    }
}

template<typename InheritType>
const UiSize& GroupBoxTemplate<InheritType>::GetCornerSize() const
{
    return m_cornerSize;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (fLineWidth < 0) {
        fLineWidth = 0;
    }
    if (bNeedDpiScale) {
        fLineWidth = this->Dpi().GetScaleFloat(fLineWidth);
    }
    if (m_fLineWidth != fLineWidth) {
        m_fLineWidth = fLineWidth;
        this->Invalidate();
    }
}

template<typename InheritType>
float GroupBoxTemplate<InheritType>::GetLineWidth() const
{
    return m_fLineWidth;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineColor(const DString& lineColor)
{
    if (m_lineColor != lineColor) {
        m_lineColor = lineColor;
        this->Invalidate();
    }
}

template<typename InheritType>
const UiString& GroupBoxTemplate<InheritType>::GetLineColor() const
{
    return m_lineColor;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetTextGap(int32_t nTextGap, bool bNeedDpiScale)
{
    if (nTextGap < 0) {
        nTextGap = 0;
    }
    if (bNeedDpiScale) {
        nTextGap = this->Dpi().GetScaleInt(nTextGap);
    }
    if (m_nTextGap != nTextGap) {
        m_nTextGap = nTextGap;
        this->Invalidate();
    }
}

template<typename InheritType>
int32_t GroupBoxTemplate<InheritType>::GetTextGap() const
{
    return m_nTextGap;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCollapsible(bool bCollapsible)
{
    if (m_bCollapsible == bCollapsible) {
        return;
    }
    m_bCollapsible = bCollapsible;
    //切换可折叠能力时终止进行中的动画，避免动画与最终状态不一致
    StopCollapseAnim();
    //切换后重新布局：若此前已记录 collapsed=true，启用后直接呈现折叠态；禁用后恢复展开态
    this->ArrangeAncestor();
    this->Invalidate();
}

template<typename InheritType>
bool GroupBoxTemplate<InheritType>::IsCollapsible() const
{
    return m_bCollapsible;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCollapsed(bool bCollapsed)
{
    if (m_bCollapsed == bCollapsed) {
        //目标状态相同（无论是否正在播放动画）都忽略
        return;
    }
    if (!m_bCollapsible) {
        //允许在 collapsible 设置之前记录初始折叠状态（与XML属性的书写顺序无关）
        m_bCollapsed = bCollapsed;
        return;
    }
    ApplyCollapsed(bCollapsed, true);
}

template<typename InheritType>
bool GroupBoxTemplate<InheritType>::IsCollapsed() const
{
    return m_bCollapsed;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::ToggleCollapsed()
{
    SetCollapsed(!m_bCollapsed);
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCollapseAnimMillSeconds(int32_t nMillSeconds)
{
    if (nMillSeconds < 0) {
        nMillSeconds = 0;
    }
    m_nCollapseAnimMs = nMillSeconds;
}

template<typename InheritType>
int32_t GroupBoxTemplate<InheritType>::GetCollapseAnimMillSeconds() const
{
    return m_nCollapseAnimMs;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCollapseIconExpanded(const DString& imageString)
{
    if (m_pCollapseIconExpanded == nullptr) {
        if (imageString.empty()) {
            return;
        }
        m_pCollapseIconExpanded = std::make_unique<Image>();
    }
    if (m_pCollapseIconExpanded->GetImageString() != imageString) {
        m_pCollapseIconExpanded->ClearImageCache();
        m_pCollapseIconExpanded->SetImageString(imageString, this->Dpi());
        this->Invalidate();
    }
}

template<typename InheritType>
DString GroupBoxTemplate<InheritType>::GetCollapseIconExpanded() const
{
    return (m_pCollapseIconExpanded != nullptr) ? m_pCollapseIconExpanded->GetImageString() : DString();
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCollapseIconCollapsed(const DString& imageString)
{
    if (m_pCollapseIconCollapsed == nullptr) {
        if (imageString.empty()) {
            return;
        }
        m_pCollapseIconCollapsed = std::make_unique<Image>();
    }
    if (m_pCollapseIconCollapsed->GetImageString() != imageString) {
        m_pCollapseIconCollapsed->ClearImageCache();
        m_pCollapseIconCollapsed->SetImageString(imageString, this->Dpi());
        this->Invalidate();
    }
}

template<typename InheritType>
DString GroupBoxTemplate<InheritType>::GetCollapseIconCollapsed() const
{
    return (m_pCollapseIconCollapsed != nullptr) ? m_pCollapseIconCollapsed->GetImageString() : DString();
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::ApplyCollapsed(bool bCollapsed, bool bPlayAnim)
{
    m_bCollapsed = bCollapsed;
    if (!bPlayAnim || (m_nCollapseAnimMs <= 0)) {
        //无动画：立即切换
        StopCollapseAnim();
        this->ArrangeAncestor();
        this->Invalidate();
        return;
    }
    StartCollapseAnim(bCollapsed);
}

template<typename InheritType>
bool GroupBoxTemplate<InheritType>::IsCollapsedStatic() const
{
    return m_bCollapsible && m_bCollapsed && (m_nAnimHeight < 0);
}

template<typename InheritType>
int32_t GroupBoxTemplate<InheritType>::CalcTitleHeight() const
{
    //优先使用展开布局时测量到的标题区域高度（折叠后子控件会被压缩，不能再据此推算）
    int32_t nTitleHeight = m_nTitleHeight;
    if (nTitleHeight <= 0) {
        Control* pFirstChild = this->GetItemAt(0);
        if (pFirstChild != nullptr) {
            //标题区域为第一个子控件上方的区域
            nTitleHeight = pFirstChild->GetPos().top - this->GetPos().top;
            if (nTitleHeight <= 0) {
                //控件尚未布局时，按第一个子控件的上边距估算
                nTitleHeight = this->GetControlPadding().top + pFirstChild->GetMargin().top;
            }
        }
    }
    //兜底值：保证标题区域能够容纳标题文字与折叠图标
    const int32_t nMinHeight = this->Dpi().GetScaleInt(32);
    if (nTitleHeight < nMinHeight) {
        nTitleHeight = nMinHeight;
    }
    return nTitleHeight;
}

template<typename InheritType>
int32_t GroupBoxTemplate<InheritType>::EstimateExpandedHeight(int32_t nFallbackHeight)
{
    const int32_t nTitleHeight = CalcTitleHeight();
    //以当前宽度为约束、充足的可用高度做一次展开态测量
    int32_t nWidth = this->GetRect().Width();
    if (nWidth <= 0) {
        nWidth = INT32_MAX / 2;
    }
    UiEstSize estSize = BaseClass::EstimateSize(UiSize(nWidth, INT32_MAX / 2));
    if (estSize.cy.IsInt32()) {
        const int32_t nHeight = estSize.cy.GetInt32();
        if (nHeight >= nTitleHeight) {
            return nHeight;
        }
    }
    //无法估算时，使用最近一次展开高度或兜底高度
    if (nFallbackHeight > nTitleHeight) {
        return nFallbackHeight;
    }
    return nTitleHeight;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::StartCollapseAnim(bool bCollapse)
{
    const int32_t nTitleHeight = CalcTitleHeight();
    //以当前呈现高度为起点（支持动画中途反向）
    int32_t nFromHeight = (m_nAnimHeight >= 0) ? m_nAnimHeight : this->GetRect().Height();
    if (nFromHeight <= 0) {
        nFromHeight = bCollapse ? ((m_nExpandedHeight > nTitleHeight) ? m_nExpandedHeight : nTitleHeight)
                                : nTitleHeight;
    }
    int32_t nToHeight = bCollapse ? nTitleHeight : EstimateExpandedHeight(m_nExpandedHeight);
    if (nToHeight < nTitleHeight) {
        nToHeight = nTitleHeight;
    }
    if (nFromHeight == nToHeight) {
        //起止高度相同，无需动画，直接落到最终状态
        StopCollapseAnim();
        this->ArrangeAncestor();
        this->Invalidate();
        return;
    }

    //停止可能存在的旧动画，再以起始高度占位，避免首帧定时器触发前出现高度跳变
    StopCollapseAnim();
    m_nAnimHeight = nFromHeight;
    this->ArrangeAncestor();

    AnimationPlayer* pAnimationPlayer = new AnimationPlayer;
    m_pCollapseAnimPlayer.reset(pAnimationPlayer);
    pAnimationPlayer->SetAnimationType(AnimationType::kAnimationNone);
    //折叠用缓入（越收越快），展开用缓出（越展越慢），符合常见手感动效
    pAnimationPlayer->SetEasingFunctionType(bCollapse ? EaseInCubic : EaseOutCubic);
    pAnimationPlayer->SetTotalMillSeconds(m_nCollapseAnimMs);
    pAnimationPlayer->SetFrameIntervalMillSeconds(16); //约60帧/秒
    pAnimationPlayer->SetStartValue(nFromHeight);
    pAnimationPlayer->SetEndValue(nToHeight);

    ControlPtrT<GroupBoxTemplate> pGroupBox(this);
    AnimationPlayCallback playCallback = [pGroupBox](int32_t nNewValue) {
        if (pGroupBox == nullptr) {
            return;
        }
        pGroupBox->OnCollapseAnimHeight(nNewValue);
    };
    pAnimationPlayer->SetPlayCallback(playCallback);

    AnimationCompleteCallback completeCallback = [pGroupBox]() {
        if (pGroupBox == nullptr) {
            return;
        }
        pGroupBox->OnCollapseAnimFinished();
    };
    pAnimationPlayer->SetCompleteCallback(completeCallback);
    pAnimationPlayer->Start();
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::StopCollapseAnim()
{
    if (m_pCollapseAnimPlayer != nullptr) {
        m_pCollapseAnimPlayer->SetCompleteCallback(nullptr);
        m_pCollapseAnimPlayer->SetPlayCallback(nullptr);
        m_pCollapseAnimPlayer->Stop();
        m_pCollapseAnimPlayer.reset();
    }
    m_nAnimHeight = -1;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::OnCollapseAnimHeight(int32_t nHeight)
{
    m_nAnimHeight = nHeight;
    //请求父级重新布局：EstimateSize会返回当前动画高度，控件随之平滑收缩/展开
    this->ArrangeAncestor();
    this->Invalidate();
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::OnCollapseAnimFinished()
{
    m_nAnimHeight = -1;
    m_pCollapseAnimPlayer.reset();
    //动画结束后做一次最终布局，恢复按auto/fixed等高度模式的自然测量结果
    this->ArrangeAncestor();
    this->Invalidate();
}

template<typename InheritType>
UiEstSize GroupBoxTemplate<InheritType>::EstimateSize(UiSize szAvailable)
{
    UiEstSize estSize = BaseClass::EstimateSize(szAvailable);
    if (m_nAnimHeight >= 0) {
        //动画进行中：高度为动画的当前高度
        estSize.cy.SetInt32(m_nAnimHeight);
    }
    else if (m_bCollapsible && m_bCollapsed) {
        //折叠时高度固定为标题区域的高度，由布局系统自然收缩
        estSize.cy.SetInt32(CalcTitleHeight());
    }
    return estSize;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetPos(UiRect rc)
{
    if (m_nAnimHeight >= 0) {
        //动画进行中：呈现为动画当前高度
        rc.bottom = rc.top + m_nAnimHeight;
    }
    else if (m_bCollapsible && m_bCollapsed) {
        //折叠状态下，无论父级分配多高，只占标题区域的高度
        rc.bottom = rc.top + CalcTitleHeight();
    }
    BaseClass::SetPos(rc);

    //记录最近一次完全展开的实际高度，作为后续展开动画的兜底目标
    if ((m_nAnimHeight < 0) && !(m_bCollapsible && m_bCollapsed)) {
        const int32_t nHeight = rc.Height();
        if (nHeight > CalcTitleHeight()) {
            m_nExpandedHeight = nHeight;
        }
        //记录展开布局下的标题区域高度（第一个子控件上方的区域），折叠时以此高度显示
        Control* pFirstChild = this->GetItemAt(0);
        if (pFirstChild != nullptr) {
            const int32_t nTitleHeight = pFirstChild->GetPos().top - rc.top;
            if (nTitleHeight > 0) {
                m_nTitleHeight = nTitleHeight;
            }
        }
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    if (m_bCollapsible && m_bCollapsed && (m_nAnimHeight < 0)) {
        //折叠静止态：不绘制子控件
        return;
    }
    if (m_nAnimHeight >= 0) {
        //动画进行中：子控件仍按完整位置布局，裁剪到控件当前矩形内，形成高度收放效果
        int32_t nClipState = pRender->ClipRect(this->GetRect(), true);
        BaseClass::PaintChild(pRender, rcPaint);
        pRender->RestoreClip(nClipState);
        return;
    }
    BaseClass::PaintChild(pRender, rcPaint);
}

template<typename InheritType>
Control* GroupBoxTemplate<InheritType>::FindControl(FINDCONTROLPROC Proc, void* pProcData, uint32_t uFlags,
                                                    const UiPoint& ptMouse, const UiPoint& scrollPos)
{
    if (m_bCollapsible && m_bCollapsed && ((uFlags & UIFIND_HITTEST) != 0)) {
        //折叠时（含折叠动画过程）命中测试：正文子控件不接收鼠标事件，只匹配控件自身
        return Control::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
    }
    return BaseClass::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
}

template<typename InheritType>
bool GroupBoxTemplate<InheritType>::IsInTitleArea(int32_t nMouseY) const
{
    if (IsCollapsedStatic()) {
        //折叠后整个控件都是标题区域
        return true;
    }
    return (nMouseY >= 0) && (nMouseY < CalcTitleHeight());
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::PaintCollapseIcon(IRender* pRender, const UiRect& rcIcon, UiColor lineColor) const
{
    //未配置图标时，绘制一个简单的箭头，保证折叠功能有可用的指示
    const int32_t cx = rcIcon.CenterX();
    const int32_t cy = rcIcon.CenterY();
    const int32_t nSize = (std::max)(2, (std::min)(rcIcon.Width(), rcIcon.Height()) / 2);
    const float fPenWidth = (std::max)(1.0f, this->Dpi().GetScaleFloat(1.2f));
    UiPointF pt1;
    UiPointF pt2;
    UiPointF pt3;
    //折叠/展开动画过程中保持展开态图标，动画结束后再切换为折叠态图标
    if (IsCollapsedStatic()) {
        pt1 = UiPointF((float)(cx - nSize / 2), (float)(cy - nSize));
        pt2 = UiPointF((float)(cx + nSize / 2), (float)cy);
        pt3 = UiPointF((float)(cx - nSize / 2), (float)(cy + nSize));
    }
    else {
        //展开时显示下指箭头
        pt1 = UiPointF((float)(cx - nSize), (float)(cy - nSize / 2));
        pt2 = UiPointF((float)cx, (float)(cy + nSize / 2));
        pt3 = UiPointF((float)(cx + nSize), (float)(cy - nSize / 2));
    }
    pRender->DrawLine(pt1, pt2, lineColor, fPenWidth);
    pRender->DrawLine(pt2, pt3, lineColor, fPenWidth);
}

template<typename InheritType>
bool GroupBoxTemplate<InheritType>::ButtonUp(const EventArgs& msg)
{
    bool bHandled = BaseClass::ButtonUp(msg);
    if (!m_bCollapsible || !this->IsEnabled()) {
        return bHandled;
    }
    //只有点击标题区域时才折叠/展开，点击分组内的控件不处理
    UiPoint ptLayout;
    if (!this->MousePosToLayoutPos(msg.ptMouse, ptLayout)) {
        return bHandled;
    }
    if (IsInTitleArea(ptLayout.y)) {
        ToggleCollapsed();
        return true;
    }
    return bHandled;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::ClearImageCache()
{
    BaseClass::ClearImageCache();
    if (m_pCollapseIconExpanded != nullptr) {
        m_pCollapseIconExpanded->ClearImageCache();
    }
    if (m_pCollapseIconCollapsed != nullptr) {
        m_pCollapseIconCollapsed->ClearImageCache();
    }
}

template<typename InheritType>
UiColor GroupBoxTemplate<InheritType>::GetFadeColor(UiColor color, uint8_t nFade) const
{
    //只调整透明度，保持 RGB 分量不变，避免颜色被压暗后发灰
    color = UiColor(nFade, color.GetR(), color.GetG(), color.GetB());
    return color;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = this->GetExpandVarStrings(strValue2);
    if (strName == _T("corner_size")) {
        //圆角大小
        UiSize cxyRound;
        AttributeUtil::ParseSizeValue(strValue.c_str(), cxyRound);
        this->SetCornerSize(cxyRound, true);
    }
    else if (strName == _T("line_width")) {
        //线条宽度
        ASSERT(StringUtil::StringToFloat(strValue.c_str(), nullptr) >= 0);
        this->SetLineWidth(StringUtil::StringToFloat(strValue.c_str(), nullptr), true);
    }
    else if (strName == _T("line_color")) {
        //线条颜色
        this->SetLineColor(strValue);
    }
    else if (strName == _T("text_gap")) {
        //标题文字与边框线之间的间距
        this->SetTextGap(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("collapsible")) {
        //是否允许折叠
        this->SetCollapsible(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("collapsed")) {
        //初始折叠状态：XML解析阶段直接生效，不播放动画
        const bool bCollapsed = StringUtil::IsValueTrue(strValue);
        if (m_bCollapsible) {
            ApplyCollapsed(bCollapsed, false);
        }
        else {
            m_bCollapsed = bCollapsed;
        }
    }
    else if (strName == _T("collapse_anim")) {
        //折叠/展开动画时长（毫秒，0表示不使用动画）
        this->SetCollapseAnimMillSeconds(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("collapse_icon_expanded")) {
        //展开状态下显示的图标
        this->SetCollapseIconExpanded(strValue);
    }
    else if (strName == _T("collapse_icon_collapsed")) {
        //折叠状态下显示的图标
        this->SetCollapseIconCollapsed(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!this->Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiSize cxyRound = this->GetCornerSize();
    cxyRound = this->Dpi().GetScaleSize(cxyRound, nOldDpiScale);
    this->SetCornerSize(cxyRound, false);

    float fValue = this->GetLineWidth();
    fValue = this->Dpi().GetScaleFloat(fValue, nOldDpiScale);
    this->SetLineWidth(fValue, false);

    int32_t nTextGap = this->GetTextGap();
    nTextGap = this->Dpi().GetScaleInt(nTextGap, nOldDpiScale);
    this->SetTextGap(nTextGap, false);

    //标题区域高度的缓存值也需要按DPI缩放，避免折叠状态的高度失真
    if (m_nTitleHeight > 0) {
        m_nTitleHeight = this->Dpi().GetScaleInt(m_nTitleHeight, nOldDpiScale);
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::PaintText(IRender* pRender)
{
    //先绘制文字
    BaseClass::PaintText(pRender);

    if (pRender == nullptr) {
        return;
    }

    UiPadding rcPadding = this->GetControlPadding();
    DString textValue = this->GetText();//文本内容
    UiRect drawTextRect;//文本的绘制区域
    int32_t nTextWidth = 0;
    int32_t nTextHeight = 0;
    int32_t nTextLeft = 0;
    int32_t nClipState = -1;
    if (!textValue.empty()) {
        MeasureStringParam measureParam;
        measureParam.pFont = this->GetIFontById(this->GetFontId());
        UiRect textRect = pRender->MeasureString(textValue, measureParam);
        drawTextRect = this->GetRect();
        drawTextRect.Deflate(rcPadding);
        drawTextRect.Deflate(this->GetTextPadding());

        nTextWidth = textRect.Width();
        nTextHeight = textRect.Height();
        nTextLeft = drawTextRect.left;
        int32_t nTextRight = drawTextRect.left + nTextWidth;
        int32_t nTextBottom = drawTextRect.top + nTextHeight;

        drawTextRect.right = std::min(drawTextRect.right, nTextRight);
        drawTextRect.bottom = std::min(drawTextRect.bottom, nTextBottom);

        //标题文字左右两侧各留出间距，使边框线不紧贴文字（更透气，也更符合分组框的视觉习惯）
        if (m_nTextGap > 0) {
            const UiRect rcControl = this->GetRect();
            drawTextRect.left = std::max(drawTextRect.left - m_nTextGap, rcControl.left);
            drawTextRect.right = std::min(drawTextRect.right + m_nTextGap, rcControl.right);
        }
    }

    //折叠图标：计算绘制区域（位于标题文字的右侧，与文字垂直居中）
    const bool bShowCollapseIcon = m_bCollapsible && !this->GetRect().IsEmpty();
    Image* pCollapseIcon = nullptr;
    int32_t nIconWidth = 0;
    int32_t nIconHeight = 0;
    if (bShowCollapseIcon) {
        pCollapseIcon = IsCollapsedStatic() ? m_pCollapseIconCollapsed.get() : m_pCollapseIconExpanded.get();
        if (pCollapseIcon != nullptr) {
            this->LoadImageInfo(*pCollapseIcon);
            const std::shared_ptr<ImageInfo>& pImageInfo = pCollapseIcon->GetImageInfo();
            if ((pImageInfo != nullptr) && (pImageInfo->GetWidth() > 0) && (pImageInfo->GetHeight() > 0)) {
                nIconWidth = pImageInfo->GetWidth();
                nIconHeight = pImageInfo->GetHeight();
            }
            else {
                pCollapseIcon = nullptr;
            }
        }
        if (nIconWidth <= 0) {
            //未配置图标时，使用代码绘制箭头
            nIconWidth = this->Dpi().GetScaleInt(10);
            nIconHeight = nIconWidth;
        }
    }

    UiRect iconRect;
    if (bShowCollapseIcon) {
        const UiRect rcControl = this->GetRect();
        int32_t nIconGap = (m_nTextGap > 0) ? m_nTextGap : this->Dpi().GetScaleInt(4);
        iconRect.left = nTextLeft + nTextWidth + ((nTextWidth > 0) ? nIconGap : 0);
        if (nTextHeight > 0) {
            //与标题文字垂直居中
            iconRect.top = drawTextRect.top + (nTextHeight - nIconHeight) / 2;
        }
        else {
            //没有标题文字时，在标题区域内垂直居中
            iconRect.top = (CalcTitleHeight() - nIconHeight) / 2;
        }
        iconRect.right = iconRect.left + nIconWidth;
        iconRect.bottom = iconRect.top + nIconHeight;
        //避免图标超出控件范围
        if (iconRect.right > rcControl.right) {
            iconRect.Offset(rcControl.right - iconRect.right, 0);
        }
        if (iconRect.left < rcControl.left) {
            iconRect.Offset(rcControl.left - iconRect.left, 0);
        }
    }

    //设置剪辑区域：标题文字与折叠图标区域不绘制边框线
    UiRect rcClip = drawTextRect;
    if (bShowCollapseIcon) {
        rcClip.Union(iconRect);
    }
    if (!rcClip.IsEmpty()) {
        nClipState = pRender->ClipRect(rcClip, false);
    }

    //在文字底部绘制边框
    float fLineWidth = m_fLineWidth;
    if (fLineWidth <= 0) {
        fLineWidth = this->Dpi().GetScaleFloat(1.0f);
    }
    UiSize cornerSize = m_cornerSize;
    UiColor lineColor;
    if (!m_lineColor.empty()) {
        lineColor = this->GetUiColor(m_lineColor.c_str());
    }
    if (lineColor.IsEmpty()) {
        if ((this->GetWindow() != nullptr) && this->GetWindow()->IsColorThemeDarkMode()) {
            //深色主题
            lineColor = GetFadeColor(UiColor(UiColors::White), 96);
        }
        else {
            //浅色主题
            lineColor = GetFadeColor(UiColor(UiColors::Gray), 96);
        }        
    }

    //阴影偏移（随 DPI 缩放，保证高 DPI 下边缘依然柔和）
    int32_t nShadowOffset = (int32_t)std::round(this->Dpi().GetScaleFloat(1.0f));
    if (nShadowOffset < 1) {
        nShadowOffset = 1;
    }
    UiRect rc = this->GetRect();
    rc.Deflate(rcPadding);

    int32_t nLineWidthHalf = (int32_t)(std::round(fLineWidth / 2));
    rc.Deflate(nLineWidthHalf + nShadowOffset, nLineWidthHalf + nShadowOffset);

    if (drawTextRect.Height() > 0) {
        //让线条在文本中间
        rc.top += (drawTextRect.Height() / 2 - nLineWidthHalf);
    }

    if ((cornerSize.cx > 0) && (cornerSize.cy > 0)) {
        //先绘制个阴影效果
        if (nShadowOffset > 0) {
            UiColor fadeColor = GetFadeColor(lineColor, 24);
            UiRect fadeRect = rc;
            fadeRect.Inflate(nShadowOffset, nShadowOffset);
            pRender->DrawRoundRect(UiRectF::MakeFromRect(fadeRect), (float)cornerSize.cx, (float)cornerSize.cy, fadeColor, fLineWidth);
        }
        //绘制圆角矩形边框
        pRender->DrawRoundRect(UiRectF::MakeFromRect(rc), (float)cornerSize.cx, (float)cornerSize.cy, lineColor, fLineWidth);
    }
    else {
        //先绘制个阴影效果
        if (nShadowOffset > 0) {
            UiColor fadeColor = GetFadeColor(lineColor, 24);
            UiRect fadeRect = rc;
            fadeRect.Inflate(nShadowOffset, nShadowOffset);
            pRender->DrawRect(UiRectF::MakeFromRect(fadeRect), fadeColor, fLineWidth);
        }
        //绘制矩形边框
        pRender->DrawRect(UiRectF::MakeFromRect(rc), lineColor, fLineWidth);
    }

    if (nClipState >= 0) {
        //恢复剪辑区域
        pRender->RestoreClip(nClipState);
    }

    if (bShowCollapseIcon) {
        if (pCollapseIcon != nullptr) {
            //使用dest属性精确指定绘制位置（相对控件左上角）
            UiRect imageRect = iconRect;
            imageRect.Offset(-this->GetRect().left, -this->GetRect().top);
            //鼠标悬停时图标完全显示，否则半透明显示
            int32_t nFade = this->IsHoveredState() ? DUI_NOSET_VALUE : 200;
            DString imageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"),
                imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
            this->PaintImage(pRender, pCollapseIcon, imageModify, nFade);
        }
        else {
            PaintCollapseIcon(pRender, iconRect, lineColor);
        }
    }
}

/** 分组容器/垂直分组容器/水平分组容器
*/
typedef GroupBoxTemplate<Box>  GroupBox;
typedef GroupBoxTemplate<HBox> GroupHBox;
typedef GroupBoxTemplate<VBox> GroupVBox;

}

#endif // UI_CONTROL_GROUPBOX_H_
