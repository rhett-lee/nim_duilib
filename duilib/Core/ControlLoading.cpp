#include "ControlLoading.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{
//属性
class LoadingAttribute
{
public:
    //Xml文件文件名，含相对路径
    UiString m_sXmlPath;

    //动画控件的名称
    UiString m_sAnimationControlName;

    //位置偏移（X方向）
    int32_t m_nOffsetX = -1;

    //位置偏移（X方向）
    int32_t m_nOffsetY = -1;

    //设置宽度，比如300
    int32_t m_nWidth = 0;

    //设置高度，比如200
    int32_t m_nHeight = 0;

    //在目标区域中横向对齐方式（仅当m_nOffsetX未指定时有效）
    UiString m_hAlign;

    //在目标区域中纵向对齐方式（仅当m_nOffsetY未指定时有效）
    UiString m_vAlign;

    //透明度（0 - 255）
    uint8_t m_bFade = 255;

    //动画结束以后，自动停止loading
    bool m_bAutoStopWhenAnimationEnd = false;
};

ControlLoading::ControlLoading(Control* pControl):
    m_bIsLoading(false),
    m_nIntervalMs(0),
    m_nCallbackCount(0),
    m_nMaxCount(-1),
    m_pControl(pControl),
    m_pAnimationControl(nullptr)
{
    ASSERT(pControl != nullptr);
    m_pLoadingAttribute = std::make_unique<LoadingAttribute>();
}

ControlLoading::~ControlLoading()
{
}

bool ControlLoading::SetLoadingAttribute(const DString& loadingAttribute)
{
    ASSERT(!m_bIsLoading);
    if (m_bIsLoading) {
        return false;
    }
    return InitAttribute(*m_pLoadingAttribute, loadingAttribute);
}

bool ControlLoading::InitAttribute(LoadingAttribute& loadingAttribute, const DString& loadingString)
{
    loadingAttribute = LoadingAttribute();
    if (loadingString.find(_T('=')) == DString::npos) {
        //不含有等号，说明没有属性，直接返回
        return false;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(loadingString, _T('\''), attributeList);
    for (const auto& attribute : attributeList) {
        const DString& name = attribute.first;
        const DString& value = attribute.second;
        if (name.empty() || value.empty()) {
            continue;
        }
        if (name == _T("file")) {
            //XML资源文件名，根据此设置去加载XML资源，最终放在一个Box容器中显示
            loadingAttribute.m_sXmlPath = value;
        }
        else if (name == _T("width")) {
            //设置宽度
            loadingAttribute.m_nWidth = StringUtil::StringToInt32(value);
        }
        else if (name == _T("height")) {
            //设置高度
            loadingAttribute.m_nHeight = StringUtil::StringToInt32(value);
        }
        else if (name == _T("offset_x")) {
            //设置X偏移
            loadingAttribute.m_nOffsetX = StringUtil::StringToInt32(value);
        }
        else if (name == _T("offset_y")) {
            //设置Y偏移
            loadingAttribute.m_nOffsetY = StringUtil::StringToInt32(value);
        }
        else if (name == _T("halign")) {
            //在目标区域中设置横向对齐方式            
            ASSERT((value == _T("left")) || (value == _T("center")) || (value == _T("right")));
            if ((value == _T("left")) || (value == _T("center")) || (value == _T("right"))) {
                loadingAttribute.m_hAlign = value;
            }
        }
        else if (name == _T("valign")) {
            //在目标区域中设置纵向对齐方式
            ASSERT((value == _T("top")) || (value == _T("center")) || (value == _T("bottom")));
            if ((value == _T("top")) || (value == _T("center")) || (value == _T("bottom"))) {
                loadingAttribute.m_vAlign = value;
            }
        }
        else if (name == _T("fade")) {
            //loading控件的透明度
            loadingAttribute.m_bFade = (uint8_t)StringUtil::StringToInt32(value);
        }
        else if (name == _T("auto_stop")) {
            //loading动画播放结束以后，自动停止loading状态
            loadingAttribute.m_bAutoStopWhenAnimationEnd = value == _T("true");
        }
        else if (name == _T("animation_control")) {
            //动画控件的名称，用于Loading功能与loading控件上的动画控件交互使用
            loadingAttribute.m_sAnimationControlName = value;
        }
        else {
            ASSERT(!"ControlLoading::InitAttribute: fount unknown attribute!");
        }
    }
    return !loadingAttribute.m_sXmlPath.empty();
}

void ControlLoading::PaintLoading(IRender* pRender, const UiRect& rcPaint)
{
    ASSERT((pRender != nullptr) && (m_pControl != nullptr));
    if ((pRender == nullptr) || (m_pControl == nullptr)){
        return;
    }
    if (m_bIsLoading && (m_pLoadingBox != nullptr)) {
        if ((m_pLoadingBox->GetWidth() <= 0) || (m_pLoadingBox->GetHeight() <= 0)) {
            //初始化显示位置
            UpdateLoadingPos();
        }
        m_pLoadingBox->AlphaPaint(pRender, rcPaint);
    }
}

void ControlLoading::UpdateLoadingPos()
{
    if (!m_bIsLoading || (m_pLoadingBox == nullptr) || (m_pControl == nullptr)) {
        return;
    }
    //初始化显示位置
    int32_t nBoxWidth = 0;
    int32_t nBoxHeight = 0;
    if ((m_pLoadingAttribute->m_nWidth <= 0) || (m_pLoadingAttribute->m_nHeight <= 0)) {
        //计算宽度和高度
        int32_t nWidth = 0;
        int32_t nHeight = 0;
        UiFixedInt fixedHeight = m_pLoadingBox->GetFixedHeight();
        UiFixedInt fixedWidth = m_pLoadingBox->GetFixedWidth();
        if (fixedHeight.IsAuto() || fixedWidth.IsAuto()) {
            UiSize rc;
            rc.cx = m_pControl->GetRect().Width();
            rc.cy = m_pControl->GetRect().Height();
            UiEstSize size = m_pLoadingBox->EstimateSize(rc);
            if (fixedWidth.IsAuto()) {
                nWidth = size.cx.GetInt32();
            }
            if (fixedHeight.IsAuto()) {
                nHeight = size.cy.GetInt32();
            }
        }

        if ((nWidth <= 0) && fixedWidth.IsInt32()){
            nWidth = fixedWidth.GetInt32();
        }
        if ((nHeight <= 0) && fixedHeight.IsInt32()) {
            nHeight = fixedHeight.GetInt32();
        }
        if (m_pLoadingAttribute->m_nWidth <= 0) {
            nBoxWidth = nWidth;
        }
        else {
            nBoxWidth = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nWidth);
        }
        if (m_pLoadingAttribute->m_nHeight <= 0) {
            nBoxHeight = nHeight;
        }
        else {
            nBoxHeight = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nHeight);
        }
    }
    else {
        nBoxWidth = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nWidth);
        nBoxHeight = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nHeight);
    }

    if ((nBoxWidth <= 0) || (nBoxWidth > m_pControl->GetRect().Width())) {
        nBoxWidth = m_pControl->GetRect().Width();
    }
    if ((nBoxHeight <= 0) || (nBoxHeight > m_pControl->GetRect().Height())) {
        nBoxHeight = m_pControl->GetRect().Height();
    }

    const UiRect rect = m_pControl->GetRect();
    UiRect rcLoading = m_pControl->GetRect();
    const int32_t nOffsetX = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nOffsetX);
    if ((nOffsetX >= 0) && (nOffsetX < rect.Width())) {
        rcLoading.left = rect.left + nOffsetX;
    }
    else {
        if (m_pLoadingAttribute->m_hAlign == _T("left")) {
            //靠左对齐
            rcLoading.left = rect.left;
        }
        else if (m_pLoadingAttribute->m_hAlign == _T("right")) {
            //靠右对齐
            rcLoading.left = rect.right - nBoxWidth;
        }
        else {
            //居中对齐
            rcLoading.left = rect.left + (rect.Width() - nBoxWidth) / 2;
        }        
    }
    rcLoading.right = rcLoading.left + nBoxWidth;

    const int32_t nOffsetY = m_pControl->Dpi().GetScaleInt(m_pLoadingAttribute->m_nOffsetY);
    if ((nOffsetY >= 0) && (nOffsetY < rect.Height())) {
        rcLoading.top = rect.top + nOffsetY;
    }
    else {
        if (m_pLoadingAttribute->m_vAlign == _T("top")) {
            rcLoading.top = rect.top;
        }
        else if (m_pLoadingAttribute->m_vAlign == _T("bottom")) {
            rcLoading.top = rect.bottom - nBoxHeight;
        }
        else {
            //居中对齐
            rcLoading.top = rect.top + (rect.Height() - nBoxHeight) / 2;
        }
    }
    rcLoading.bottom = rcLoading.top + nBoxHeight;

    //设置控件位置
    m_pLoadingBox->SetPos(rcLoading);
}

void ControlLoading::Loading()
{
    if (!m_bIsLoading) {
        return;
    }

    //触发loading事件
    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    ControlPtrT<Control> pControl(m_pControl);
    ControlPtrT<Box> pLoadingUiRootBox(m_pLoadingBox.get());
    ControlPtrT<Control> pAnimationControl(m_pAnimationControl);

    ControlLoadingStatus status;
    status.m_pControl = pControl;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_startTime);
    status.m_nElapsedTimeMs = (uint64_t)microseconds.count();
    status.m_pLoadingUiRootBox = pLoadingUiRootBox;
    status.m_pAnimationControl = pAnimationControl;
    status.m_bStopLoading = false;
    m_pControl->SendEvent(kEventLoading, (WPARAM)&status);

    if (!weakFlag.expired() && m_bIsLoading) {
        ++m_nCallbackCount;
        bool bStopLoading = status.m_bStopLoading;
        if ((m_nMaxCount >= 0) && (m_nCallbackCount > m_nMaxCount)) {
            //达到停止条件
            bStopLoading = true;
        }
        if (bStopLoading) {
            if (m_pControl != nullptr) {
                m_pControl->StopLoading();
            }
        }
    }
}

bool ControlLoading::StartLoading(int32_t nIntervalMs, int32_t nMaxCount)
{
    if (m_bIsLoading) {
        StopLoading();
    }
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return false;
    }

    m_pAnimationControl = nullptr;
    m_nCallbackCount = 0;
    if (m_pLoadingBox == nullptr) {        
        ASSERT(!m_pLoadingAttribute->m_sXmlPath.empty());
        if (m_pLoadingAttribute->m_sXmlPath.empty()) {
            return false;
        }
        Box* pLoadingBox = GlobalManager::Instance().CreateBox(m_pControl->GetWindow(), FilePath(m_pLoadingAttribute->m_sXmlPath.c_str()));
        ASSERT(pLoadingBox != nullptr);
        if (pLoadingBox == nullptr) {
            return false;
        }
        m_pLoadingBox.reset(pLoadingBox);
        if (m_pLoadingAttribute->m_bFade != 255) {
            m_pLoadingBox->SetAlpha(m_pLoadingAttribute->m_bFade);
        }
        if (!m_pLoadingAttribute->m_sAnimationControlName.empty()) {
            m_pAnimationControl = m_pControl->GetWindow()->FindSubControlByName(m_pLoadingBox.get(), m_pLoadingAttribute->m_sAnimationControlName.c_str());
        }
    }
    m_startTime = std::chrono::steady_clock::now();
    m_bIsLoading = true;
    m_nIntervalMs = nIntervalMs;
    if (m_nIntervalMs <= 0) {
        //设置默认值
        m_nIntervalMs = 50;
    }
    else if (m_nIntervalMs < 10) {
        //限制最小值
        m_nIntervalMs = 10;
    }
    m_nMaxCount = nMaxCount;
    GlobalManager::Instance().Timer().AddTimer(m_loadingFlag.GetWeakFlag(),
                                               UiBind(&ControlLoading::Loading, this),
                                               m_nIntervalMs);

    //动画结束以后，自动停止loading
    if ((m_pLoadingAttribute->m_bAutoStopWhenAnimationEnd) && (m_pAnimationControl != nullptr)) {
        ControlPtrT<Control> pControl(m_pControl);
        m_pAnimationControl->AttachImageAnimationStop([pControl](const EventArgs&) {
            if (pControl != nullptr) {
                pControl->StopLoading();
            }
            return true;
            });
    }

    //触发开始事件
    ControlPtrT<Control> pControl(m_pControl);
    ControlPtrT<Box> pLoadingUiRootBox(m_pLoadingBox.get());
    ControlPtrT<Control> pAnimationControl(m_pAnimationControl);

    ControlLoadingStatus status;
    status.m_pControl = pControl;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_startTime);
    status.m_nElapsedTimeMs = (uint64_t)microseconds.count();
    status.m_pLoadingUiRootBox = pLoadingUiRootBox;
    status.m_pAnimationControl = pAnimationControl;
    status.m_bStopLoading = false;
    m_pControl->SendEvent(kEventLoadingStart, (WPARAM)&status);
    return true;
}

void ControlLoading::StopLoading()
{
    if (!m_bIsLoading) {
        return;
    }
    m_bIsLoading = false;
    m_loadingFlag.Cancel();
    m_pLoadingBox.reset();
    m_pAnimationControl = nullptr;
    m_nCallbackCount = 0;

    //触发停止事件
    ControlPtrT<Control> pControl(m_pControl);
    ControlLoadingStatus status;
    status.m_pControl = pControl;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_startTime);
    status.m_nElapsedTimeMs = (uint64_t)microseconds.count();
    status.m_pLoadingUiRootBox = nullptr;
    status.m_pAnimationControl = nullptr;
    status.m_bStopLoading = false;
    m_pControl->SendEvent(kEventLoadingStop, (WPARAM)&status);
}

bool ControlLoading::IsLoading() const
{
    return m_bIsLoading;
}

Box* ControlLoading::GetLoadingUiRootBox() const
{
    return m_pLoadingBox.get();
}

}
