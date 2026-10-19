#include "Carousel.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <cstdlib>

namespace ui
{

/** 切换动画帧间隔（毫秒）
*/
static constexpr int32_t kCarouselAnimFrameMs = 16;

Carousel::Carousel(Window* pWindow) :
    BaseClass(pWindow),
    m_itemImages(),
    m_slideImages(),
    m_prevImage(),
    m_nextImage(),
    m_indicatorNormalImage(),
    m_indicatorActiveImage(),
    m_nCurrentIndex(-1),
    m_bLoop(true),
    m_bAutoPlay(false),
    m_nInterval(3000),
    m_bHoverPause(true),
    m_nTransitionMs(300),
    m_bShowIndicator(true),
    m_bShowNavButton(true),
    m_nIndicatorSize(8),
    m_nIndicatorGap(8),
    m_nIndicatorBottom(10),
    m_nBtnSize(32),
    m_nBtnMargin(12),
    m_bAnimating(false),
    m_nAnimFromIndex(-1),
    m_nAnimToIndex(-1),
    m_nAnimDir(1),
    m_nAnimElapsed(0),
    m_bAnimTimerStarted(false),
    m_nAnimTimerId(0),
    m_bAutoTimerStarted(false),
    m_nAutoTimerId(0),
    m_animFlag(),
    m_autoFlag(),
    m_bHovering(false),
    m_nHoverBtn(0),
    m_nPressedBtn(0),
    m_bDragging(false),
    m_bDragMoved(false),
    m_ptDragStart(),
    m_nDragOffset(0),
    m_bMuteEvent(false)
{
    //按钮 / 指示点图片需要关联控件接口，以便解析相对路径与缓存管理
    m_prevImage.SetControl(this);
    m_nextImage.SetControl(this);
    m_indicatorNormalImage.SetControl(this);
    m_indicatorActiveImage.SetControl(this);
}

Carousel::~Carousel()
{
    StopAllTimers();
}

DString Carousel::GetType() const
{
    return DUI_CTR_CAROUSEL;
}

void Carousel::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("item_image")) {
        //幻灯片图片列表，`|` 分隔
        std::vector<DString> items;
        for (const DString& item : StringUtil::Split(strValue, _T("|"))) {
            if (!item.empty()) {
                items.push_back(item);
            }
        }
        SetItems(items);
    }
    else if ((strName == _T("current_index")) || (strName == _T("currentindex"))) {
        SetCurrentIndexNoEvent(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("loop")) {
        SetLoop(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("auto_play")) || (strName == _T("autoplay"))) {
        SetAutoPlay(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("interval")) {
        SetInterval(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("hover_pause")) || (strName == _T("hoverpause"))) {
        SetHoverPause(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("transition_ms")) || (strName == _T("transitionms"))) {
        SetTransitionMs(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("show_indicator")) || (strName == _T("showindicator"))) {
        m_bShowIndicator = StringUtil::IsValueTrue(strValue);
        Invalidate();
    }
    else if ((strName == _T("show_nav_button")) || (strName == _T("shownavbutton"))) {
        m_bShowNavButton = StringUtil::IsValueTrue(strValue);
        Invalidate();
    }
    else if ((strName == _T("indicator_normal_image")) || (strName == _T("indicatornormalimage"))) {
        m_indicatorNormalImage.SetImageString(kControlStateNormal, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("indicator_active_image")) || (strName == _T("indicatoractiveimage"))) {
        m_indicatorActiveImage.SetImageString(kControlStateNormal, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("indicator_size")) || (strName == _T("indicatorsize"))) {
        m_nIndicatorSize = StringUtil::StringToInt32(strValue);
        Invalidate();
    }
    else if ((strName == _T("indicator_gap")) || (strName == _T("indicatorgap"))) {
        m_nIndicatorGap = StringUtil::StringToInt32(strValue);
        Invalidate();
    }
    else if ((strName == _T("indicator_bottom")) || (strName == _T("indicatorbottom"))) {
        m_nIndicatorBottom = StringUtil::StringToInt32(strValue);
        Invalidate();
    }
    else if ((strName == _T("btn_prev_image")) || (strName == _T("btnprevimage"))) {
        m_prevImage.SetImageString(kControlStateNormal, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("btn_next_image")) || (strName == _T("btnnextimage"))) {
        m_nextImage.SetImageString(kControlStateNormal, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("btn_prev_hovered_image")) || (strName == _T("btnprevhoveredimage"))) {
        m_prevImage.SetImageString(kControlStateHovered, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("btn_next_hovered_image")) || (strName == _T("btnnexthoveredimage"))) {
        m_nextImage.SetImageString(kControlStateHovered, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("btn_prev_pressed_image")) || (strName == _T("btnprevpressedimage"))) {
        m_prevImage.SetImageString(kControlStatePressed, strValue, Dpi());
        Invalidate();
    }
    else if ((strName == _T("btn_next_pressed_image")) || (strName == _T("btnnextpressedimage"))) {
        m_nextImage.SetImageString(kControlStatePressed, strValue, Dpi());
        Invalidate();
    }
    else if (strName == _T("btn_size")) {
        m_nBtnSize = StringUtil::StringToInt32(strValue);
        Invalidate();
    }
    else if (strName == _T("btn_margin")) {
        m_nBtnMargin = StringUtil::StringToInt32(strValue);
        Invalidate();
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Carousel::ClearImageCache()
{
    BaseClass::ClearImageCache();
    for (std::unique_ptr<StateImage>& pImage : m_slideImages) {
        if (pImage != nullptr) {
            pImage->ClearImageCache();
        }
    }
    m_prevImage.ClearImageCache();
    m_nextImage.ClearImageCache();
    m_indicatorNormalImage.ClearImageCache();
    m_indicatorActiveImage.ClearImageCache();
}

// ---- 幻灯片数据模型 ----

void Carousel::RebuildSlideImages()
{
    m_slideImages.clear();
    m_slideImages.reserve(m_itemImages.size());
    for (const DString& strImage : m_itemImages) {
        std::unique_ptr<StateImage> pImage = std::make_unique<StateImage>();
        pImage->SetControl(this);
        pImage->SetImageString(kControlStateNormal, strImage, Dpi());
        m_slideImages.push_back(std::move(pImage));
    }
}

void Carousel::SetSlideImageAt(int32_t nIndex, const DString& strImage)
{
    if ((nIndex < 0) || (nIndex >= static_cast<int32_t>(m_itemImages.size()))) {
        return;
    }
    m_itemImages[nIndex] = strImage;
    if (nIndex < static_cast<int32_t>(m_slideImages.size())) {
        m_slideImages[nIndex]->SetImageString(kControlStateNormal, strImage, Dpi());
    }
}

Image* Carousel::GetSlideImagePtr(int32_t nIndex) const
{
    if ((nIndex < 0) || (nIndex >= static_cast<int32_t>(m_slideImages.size()))) {
        return nullptr;
    }
    return m_slideImages[nIndex]->GetStateImage(kControlStateNormal);
}

void Carousel::AddItem(const DString& strImage)
{
    m_itemImages.push_back(strImage);
    std::unique_ptr<StateImage> pImage = std::make_unique<StateImage>();
    pImage->SetControl(this);
    pImage->SetImageString(kControlStateNormal, strImage, Dpi());
    m_slideImages.push_back(std::move(pImage));
    ClampCurrentIndex();
    UpdateAutoPlayTimer();
    Invalidate();
}

void Carousel::InsertItem(int32_t nIndex, const DString& strImage)
{
    const int32_t nCount = GetItemCount();
    if (nIndex < 0) {
        nIndex = 0;
    }
    if (nIndex > nCount) {
        nIndex = nCount;
    }
    m_itemImages.insert(m_itemImages.begin() + nIndex, strImage);
    std::unique_ptr<StateImage> pImage = std::make_unique<StateImage>();
    pImage->SetControl(this);
    pImage->SetImageString(kControlStateNormal, strImage, Dpi());
    m_slideImages.insert(m_slideImages.begin() + nIndex, std::move(pImage));
    ClampCurrentIndex();
    UpdateAutoPlayTimer();
    Invalidate();
}

bool Carousel::RemoveItem(int32_t nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetItemCount())) {
        return false;
    }
    m_itemImages.erase(m_itemImages.begin() + nIndex);
    m_slideImages.erase(m_slideImages.begin() + nIndex);
    ClampCurrentIndex();
    UpdateAutoPlayTimer();
    Invalidate();
    return true;
}

void Carousel::ClearItems()
{
    m_itemImages.clear();
    m_slideImages.clear();
    StopAnimation();
    ClampCurrentIndex();
    UpdateAutoPlayTimer();
    Invalidate();
}

void Carousel::SetItems(const std::vector<DString>& items)
{
    m_itemImages = items;
    StopAnimation();
    RebuildSlideImages();
    ClampCurrentIndex();
    UpdateAutoPlayTimer();
    Invalidate();
}

DString Carousel::GetItem(int32_t nIndex) const
{
    if ((nIndex < 0) || (nIndex >= GetItemCount())) {
        return DString();
    }
    return m_itemImages[nIndex];
}

// ---- 索引与事件 ----

void Carousel::ClampCurrentIndex()
{
    const int32_t nCount = GetItemCount();
    if (nCount <= 0) {
        m_nCurrentIndex = -1;
        return;
    }
    if (m_nCurrentIndex < 0) {
        m_nCurrentIndex = 0;
    }
    else if (m_nCurrentIndex >= nCount) {
        m_nCurrentIndex = nCount - 1;
    }
}

void Carousel::EmitValueChanged(int32_t nOldIndex)
{
    if (m_bMuteEvent) {
        return;
    }
    //索引通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    CarouselValue value;
    value.index = m_nCurrentIndex;
    value.oldIndex = nOldIndex;
    SendEvent(kEventValueChanged, 0, 0, static_cast<void*>(&value));
}

void Carousel::ApplyIndexChange(int32_t nNewIndex)
{
    (void)nNewIndex;
    UpdateAutoPlayTimer();
}

void Carousel::SetCurrentIndex(int32_t nIndex)
{
    const int32_t nCount = GetItemCount();
    if (nCount <= 0) {
        if (m_nCurrentIndex != -1) {
            m_nCurrentIndex = -1;
            Invalidate();
        }
        return;
    }
    int32_t nNewIndex = nIndex;
    if (nNewIndex < 0) {
        nNewIndex = 0;
    }
    if (nNewIndex >= nCount) {
        nNewIndex = nCount - 1;
    }
    const int32_t nOldIndex = m_nCurrentIndex;
    if (nNewIndex == nOldIndex) {
        return;
    }
    m_nCurrentIndex = nNewIndex;
    StartAnimation((nOldIndex >= 0) ? nOldIndex : nNewIndex, nNewIndex);
    ApplyIndexChange(nNewIndex);
    Invalidate();
    EmitValueChanged(nOldIndex);
}

void Carousel::SetCurrentIndexNoEvent(int32_t nIndex)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    SetCurrentIndex(nIndex);
    m_bMuteEvent = bOldMute;
}

bool Carousel::Next()
{
    const int32_t nCount = GetItemCount();
    if (nCount <= 0) {
        return false;
    }
    int32_t nIndex = m_nCurrentIndex + 1;
    if (nIndex >= nCount) {
        if (!m_bLoop) {
            return false;
        }
        nIndex = 0;
    }
    SetCurrentIndex(nIndex);
    return true;
}

bool Carousel::Prev()
{
    const int32_t nCount = GetItemCount();
    if (nCount <= 0) {
        return false;
    }
    int32_t nIndex = m_nCurrentIndex - 1;
    if (nIndex < 0) {
        if (!m_bLoop) {
            return false;
        }
        nIndex = nCount - 1;
    }
    SetCurrentIndex(nIndex);
    return true;
}

bool Carousel::GoTo(int32_t nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetItemCount())) {
        return false;
    }
    SetCurrentIndex(nIndex);
    return true;
}

bool Carousel::NextNoEvent()
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    const bool bRet = Next();
    m_bMuteEvent = bOldMute;
    return bRet;
}

bool Carousel::PrevNoEvent()
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    const bool bRet = Prev();
    m_bMuteEvent = bOldMute;
    return bRet;
}

bool Carousel::GoToNoEvent(int32_t nIndex)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    const bool bRet = GoTo(nIndex);
    m_bMuteEvent = bOldMute;
    return bRet;
}

// ---- 行为属性 ----

void Carousel::SetLoop(bool bLoop)
{
    if (m_bLoop != bLoop) {
        m_bLoop = bLoop;
    }
}

void Carousel::SetAutoPlay(bool bAutoPlay)
{
    if (m_bAutoPlay != bAutoPlay) {
        m_bAutoPlay = bAutoPlay;
        UpdateAutoPlayTimer();
    }
}

void Carousel::SetInterval(int32_t nIntervalMs)
{
    if (nIntervalMs < 100) {
        nIntervalMs = 100;
    }
    if (m_nInterval != nIntervalMs) {
        m_nInterval = nIntervalMs;
        //重启定时器使新的间隔生效
        if (m_bAutoTimerStarted) {
            UpdateAutoPlayTimer();
        }
    }
}

void Carousel::SetHoverPause(bool bHoverPause)
{
    if (m_bHoverPause != bHoverPause) {
        m_bHoverPause = bHoverPause;
        UpdateAutoPlayTimer();
    }
}

void Carousel::SetTransitionMs(int32_t nMs)
{
    if (nMs < 0) {
        nMs = 0;
    }
    if (m_nTransitionMs != nMs) {
        m_nTransitionMs = nMs;
    }
}

// ---- 布局计算 ----

UiRect Carousel::GetContentRect() const
{
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    return rc;
}

UiRect Carousel::GetNavButtonRect(bool bNext) const
{
    const UiRect rc = GetContentRect();
    const int32_t nSize = Dpi().GetScaleInt(m_nBtnSize);
    const int32_t nMargin = Dpi().GetScaleInt(m_nBtnMargin);
    const int32_t nTop = rc.top + (rc.Height() - nSize) / 2;
    if (bNext) {
        return UiRect(rc.right - nMargin - nSize, nTop, rc.right - nMargin, nTop + nSize);
    }
    return UiRect(rc.left + nMargin, nTop, rc.left + nMargin + nSize, nTop + nSize);
}

UiRect Carousel::GetIndicatorRect(int32_t nIndex) const
{
    const UiRect rc = GetContentRect();
    const int32_t nCount = GetItemCount();
    const int32_t nSize = Dpi().GetScaleInt(m_nIndicatorSize);
    const int32_t nGap = Dpi().GetScaleInt(m_nIndicatorGap);
    const int32_t nBottom = Dpi().GetScaleInt(m_nIndicatorBottom);
    const int32_t nTotalWidth = nCount * nSize + ((nCount > 1) ? ((nCount - 1) * nGap) : 0);
    const int32_t nStartX = rc.left + (rc.Width() - nTotalWidth) / 2;
    const int32_t nTop = rc.bottom - nBottom - nSize;
    const int32_t nLeft = nStartX + nIndex * (nSize + nGap);
    return UiRect(nLeft, nTop, nLeft + nSize, nTop + nSize);
}

int32_t Carousel::HitTestNavButton(const UiPoint& ptMouse) const
{
    if (!m_bShowNavButton) {
        return 0;
    }
    if (GetNavButtonRect(false).ContainsPt(ptMouse)) {
        return -1;
    }
    if (GetNavButtonRect(true).ContainsPt(ptMouse)) {
        return 1;
    }
    return 0;
}

int32_t Carousel::HitTestIndicator(const UiPoint& ptMouse) const
{
    if (!m_bShowIndicator) {
        return -1;
    }
    const int32_t nCount = GetItemCount();
    for (int32_t i = 0; i < nCount; ++i) {
        //指示点命中区域略微外扩，便于点击
        UiRect rc = GetIndicatorRect(i);
        rc.Inflate(Dpi().GetScaleInt(2), Dpi().GetScaleInt(2));
        if (rc.ContainsPt(ptMouse)) {
            return i;
        }
    }
    return -1;
}

// ---- 绘制 ----

void Carousel::PaintImageAt(IRender* pRender, Image* pImage, const UiRect& rcDest, int32_t nFade) const
{
    if (pImage == nullptr) {
        return;
    }
    const UiRect rcCtrl = GetRect();
    const DString strModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"),
                                                 rcDest.left - rcCtrl.left, rcDest.top - rcCtrl.top,
                                                 rcDest.right - rcCtrl.left, rcDest.bottom - rcCtrl.top);
    if (nFade == DUI_NOSET_VALUE) {
        PaintImage(pRender, pImage, strModify);
    }
    else {
        PaintImage(pRender, pImage, strModify, nFade);
    }
}

void Carousel::PaintSlide(IRender* pRender, int32_t nIndex, int32_t nOffsetX, const UiRect& rcContent, int32_t nFade)
{
    Image* pImage = GetSlideImagePtr(nIndex);
    if (pImage == nullptr) {
        return;
    }
    const UiRect rcDest(rcContent.left + nOffsetX, rcContent.top,
                        rcContent.right + nOffsetX, rcContent.bottom);
    PaintImageAt(pRender, pImage, rcDest, nFade);
}

void Carousel::PaintNavButton(IRender* pRender, bool bNext, int32_t nFade)
{
    StateImage& stateImage = bNext ? m_nextImage : m_prevImage;
    const int32_t nDir = bNext ? 1 : -1;

    ControlStateType stateType = kControlStateNormal;
    if (m_nPressedBtn == nDir) {
        stateType = kControlStatePressed;
    }
    else if (m_nHoverBtn == nDir) {
        stateType = kControlStateHovered;
    }

    Image* pImage = stateImage.GetStateImage(stateType);
    if ((pImage == nullptr) && (stateType == kControlStatePressed)) {
        //按下态未配置图片时，回退悬停态
        pImage = stateImage.GetStateImage(kControlStateHovered);
    }
    if (pImage == nullptr) {
        pImage = stateImage.GetStateImage(kControlStateNormal);
    }
    if (pImage == nullptr) {
        return;
    }
    PaintImageAt(pRender, pImage, GetNavButtonRect(bNext), nFade);
}

void Carousel::PaintIndicators(IRender* pRender, int32_t nFade)
{
    const int32_t nCount = GetItemCount();
    if (nCount <= 0) {
        return;
    }
    for (int32_t i = 0; i < nCount; ++i) {
        Image* pImage = nullptr;
        if (i == m_nCurrentIndex) {
            pImage = m_indicatorActiveImage.GetStateImage(kControlStateNormal);
        }
        if (pImage == nullptr) {
            pImage = m_indicatorNormalImage.GetStateImage(kControlStateNormal);
        }
        if (pImage == nullptr) {
            continue;
        }
        PaintImageAt(pRender, pImage, GetIndicatorRect(i), nFade);
    }
}

void Carousel::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    //惰性启动自动播放定时器（窗口初始化完成后）
    UpdateAutoPlayTimer();

    const UiRect rcContent = GetContentRect();
    if ((rcContent.Width() <= 0) || (rcContent.Height() <= 0)) {
        return;
    }
    const int32_t nCount = GetItemCount();

    //禁用态整体降低透明度
    const int32_t nFade = IsEnabled() ? DUI_NOSET_VALUE : static_cast<int32_t>(GetAlpha() * 3 / 5);

    //幻灯片绘制（裁剪到内容区，避免滑动时绘制到控件外）
    {
        AutoClip clip(pRender, rcContent);
        const int32_t nWidth = rcContent.Width();
        if (m_bAnimating && (nCount > 0)) {
            //切换动画：同时绘制"上一张 / 下一张"，按进度施加水平偏移
            const float fT = GetAnimProgress();
            const int32_t nDir = m_nAnimDir;
            const int32_t nOffsetFrom = static_cast<int32_t>(-nDir * nWidth * fT);
            const int32_t nOffsetTo = static_cast<int32_t>(nDir * nWidth * (1.0f - fT));
            PaintSlide(pRender, m_nAnimFromIndex, nOffsetFrom, rcContent, nFade);
            PaintSlide(pRender, m_nAnimToIndex, nOffsetTo, rcContent, nFade);
        }
        else if (m_bDragging && (m_nDragOffset != 0) && (nCount > 0)) {
            //拖拽：当前幻灯片随位移移动，露出相邻幻灯片
            const int32_t nOffset = m_nDragOffset;
            PaintSlide(pRender, m_nCurrentIndex, nOffset, rcContent, nFade);
            if (nOffset > 0) {
                int32_t nPrevIndex = m_nCurrentIndex - 1;
                if ((nPrevIndex < 0) && m_bLoop) {
                    nPrevIndex = nCount - 1;
                }
                PaintSlide(pRender, nPrevIndex, nOffset - nWidth, rcContent, nFade);
            }
            else {
                int32_t nNextIndex = m_nCurrentIndex + 1;
                if ((nNextIndex >= nCount) && m_bLoop) {
                    nNextIndex = 0;
                }
                PaintSlide(pRender, nNextIndex, nOffset + nWidth, rcContent, nFade);
            }
        }
        else {
            PaintSlide(pRender, m_nCurrentIndex, 0, rcContent, nFade);
        }
    }

    //切换按钮
    if (m_bShowNavButton && (nCount > 1)) {
        PaintNavButton(pRender, false, nFade);
        PaintNavButton(pRender, true, nFade);
    }

    //页码指示点
    if (m_bShowIndicator && (nCount > 1)) {
        PaintIndicators(pRender, nFade);
    }
}

// ---- 切换动画 ----

void Carousel::StartAnimation(int32_t nFromIndex, int32_t nToIndex)
{
    StopAnimation();
    const int32_t nCount = GetItemCount();
    if ((m_nTransitionMs <= 0) || (nFromIndex < 0) || (nToIndex < 0) ||
        (nFromIndex >= nCount) || (nToIndex >= nCount) || !IsInited()) {
        return;
    }
    m_bAnimating = true;
    m_nAnimFromIndex = nFromIndex;
    m_nAnimToIndex = nToIndex;
    m_nAnimElapsed = 0;
    //动画方向：默认按索引差符号；循环越界（末->首 / 首->末）时按单步方向确定
    if ((nFromIndex == (nCount - 1)) && (nToIndex == 0)) {
        m_nAnimDir = 1;
    }
    else if ((nFromIndex == 0) && (nToIndex == (nCount - 1))) {
        m_nAnimDir = -1;
    }
    else {
        m_nAnimDir = (nToIndex > nFromIndex) ? 1 : -1;
    }

    m_bAnimTimerStarted = true;
    m_animFlag.Cancel(); //先取消旧的 weakFlag，避免残留
    std::function<void()> callback = UiBind(&Carousel::OnAnimationTick, this);
    m_nAnimTimerId = GlobalManager::Instance().Timer().AddTimer(
        m_animFlag.GetWeakFlag(), callback, static_cast<uint32_t>(kCarouselAnimFrameMs), -1);
}

void Carousel::StopAnimation()
{
    if (m_bAnimTimerStarted) {
        if (m_nAnimTimerId > 0) {
            GlobalManager::Instance().Timer().RemoveTimer(m_nAnimTimerId);
            m_nAnimTimerId = 0;
        }
        m_bAnimTimerStarted = false;
    }
    m_animFlag.Cancel();
    m_bAnimating = false;
    m_nAnimFromIndex = -1;
    m_nAnimToIndex = -1;
    m_nAnimElapsed = 0;
}

void Carousel::OnAnimationTick()
{
    if (!m_bAnimating) {
        return;
    }
    m_nAnimElapsed += kCarouselAnimFrameMs;
    if (m_nAnimElapsed >= m_nTransitionMs) {
        StopAnimation();
    }
    Invalidate();
}

float Carousel::GetAnimProgress() const
{
    if (!m_bAnimating || (m_nTransitionMs <= 0)) {
        return 1.0f;
    }
    float fT = static_cast<float>(m_nAnimElapsed) / static_cast<float>(m_nTransitionMs);
    if (fT < 0.0f) {
        fT = 0.0f;
    }
    if (fT > 1.0f) {
        fT = 1.0f;
    }
    return fT;
}

// ---- 自动播放 ----

bool Carousel::IsAutoPlayPaused() const
{
    return m_bHoverPause && m_bHovering;
}

void Carousel::UpdateAutoPlayTimer()
{
    const bool bNeedTimer = m_bAutoPlay && (GetItemCount() > 1) && IsEnabled() && IsInited();
    if (!bNeedTimer) {
        if (m_bAutoTimerStarted) {
            if (m_nAutoTimerId > 0) {
                GlobalManager::Instance().Timer().RemoveTimer(m_nAutoTimerId);
                m_nAutoTimerId = 0;
            }
            m_bAutoTimerStarted = false;
            m_autoFlag.Cancel();
        }
        return;
    }

    //已启动但间隔变化时重启；否则保持不变
    if (m_bAutoTimerStarted && (m_nAutoTimerId > 0)) {
        return;
    }
    m_bAutoTimerStarted = true;
    m_autoFlag.Cancel();
    std::function<void()> callback = UiBind(&Carousel::OnAutoPlayTick, this);
    m_nAutoTimerId = GlobalManager::Instance().Timer().AddTimer(
        m_autoFlag.GetWeakFlag(), callback, static_cast<uint32_t>(m_nInterval), -1);
}

void Carousel::OnAutoPlayTick()
{
    if (IsAutoPlayPaused() || !IsEnabled()) {
        return;
    }
    //自动前进一张；非循环模式下到达末页后不再前进
    const int32_t nCount = GetItemCount();
    if (nCount <= 1) {
        return;
    }
    if ((m_nCurrentIndex >= (nCount - 1)) && !m_bLoop) {
        return;
    }
    Next();
}

void Carousel::StopAllTimers()
{
    StopAnimation();
    if (m_bAutoTimerStarted) {
        if (m_nAutoTimerId > 0) {
            GlobalManager::Instance().Timer().RemoveTimer(m_nAutoTimerId);
            m_nAutoTimerId = 0;
        }
        m_bAutoTimerStarted = false;
    }
    m_autoFlag.Cancel();
}

// ---- 交互 ----

bool Carousel::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled() || (GetItemCount() <= 0)) {
        return BaseClass::ButtonDown(msg);
    }
    //命中切换按钮
    const int32_t nBtn = HitTestNavButton(msg.ptMouse);
    if (nBtn != 0) {
        m_nPressedBtn = nBtn;
        Invalidate();
        return true;
    }
    //命中页码指示点：直接跳转
    const int32_t nIndicator = HitTestIndicator(msg.ptMouse);
    if (nIndicator >= 0) {
        GoTo(nIndicator);
        return true;
    }
    //进入拖拽
    m_bDragging = true;
    m_bDragMoved = false;
    m_ptDragStart = msg.ptMouse;
    m_nDragOffset = 0;
    //获取鼠标焦点，保证按住期间持续收到鼠标消息
    SetMouseFocused(true);
    return true;
}

bool Carousel::ButtonUp(const EventArgs& msg)
{
    //结束拖拽并吸附到目标页
    if (m_bDragging) {
        m_bDragging = false;
        if (IsMouseFocused()) {
            SetMouseFocused(false);
        }
        const int32_t nOffset = m_nDragOffset;
        m_nDragOffset = 0;
        const int32_t nThreshold = GetContentRect().Width() / 6;
        if (m_bDragMoved && (nThreshold > 0)) {
            if (nOffset > nThreshold) {
                Prev();
            }
            else if (nOffset < -nThreshold) {
                Next();
            }
        }
        Invalidate();
    }

    //切换按钮点击（按下与抬起命中同一按钮才生效）
    if (m_nPressedBtn != 0) {
        const int32_t nBtn = HitTestNavButton(msg.ptMouse);
        if (nBtn == m_nPressedBtn) {
            if (nBtn == -1) {
                Prev();
            }
            else {
                Next();
            }
        }
        m_nPressedBtn = 0;
        Invalidate();
    }
    return BaseClass::ButtonUp(msg);
}

bool Carousel::MouseMove(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::MouseMove(msg);
    }
    bool bRedraw = false;

    //更新切换按钮悬停态
    const int32_t nHoverBtn = HitTestNavButton(msg.ptMouse);
    if (m_nHoverBtn != nHoverBtn) {
        m_nHoverBtn = nHoverBtn;
        bRedraw = true;
    }

    //拖拽：按水平位移切换（松手吸附）
    if (m_bDragging && IsMouseFocused()) {
        int32_t nOffset = msg.ptMouse.x - m_ptDragStart.x;
        if (!m_bDragMoved && (std::abs(nOffset) > Dpi().GetScaleInt(4))) {
            m_bDragMoved = true;
        }
        //非循环模式下，边界方向不允许越界拖拽
        const int32_t nCount = GetItemCount();
        if (!m_bLoop) {
            if ((nOffset > 0) && (m_nCurrentIndex <= 0)) {
                nOffset = 0;
                m_bDragMoved = false;
            }
            else if ((nOffset < 0) && (m_nCurrentIndex >= (nCount - 1))) {
                nOffset = 0;
                m_bDragMoved = false;
            }
        }
        if (m_nDragOffset != nOffset) {
            m_nDragOffset = nOffset;
            bRedraw = true;
        }
    }

    if (bRedraw) {
        Invalidate();
    }
    return BaseClass::MouseMove(msg);
}

bool Carousel::MouseEnter(const EventArgs& msg)
{
    m_bHovering = true;
    UpdateAutoPlayTimer();
    return BaseClass::MouseEnter(msg);
}

bool Carousel::MouseLeave(const EventArgs& msg)
{
    m_bHovering = false;
    bool bRedraw = false;
    if (m_nHoverBtn != 0) {
        m_nHoverBtn = 0;
        bRedraw = true;
    }
    if (m_nPressedBtn != 0) {
        m_nPressedBtn = 0;
        bRedraw = true;
    }
    if (m_bDragging) {
        m_bDragging = false;
        m_bDragMoved = false;
        m_nDragOffset = 0;
        if (IsMouseFocused()) {
            SetMouseFocused(false);
        }
        bRedraw = true;
    }
    UpdateAutoPlayTimer();
    if (bRedraw) {
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

void Carousel::SetWindow(Window* pWindow)
{
    if (GetWindow() != pWindow) {
        StopAllTimers();
    }
    BaseClass::SetWindow(pWindow);
}

} // namespace ui
