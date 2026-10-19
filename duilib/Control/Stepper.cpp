#include "Stepper.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <vector>

namespace ui
{

Stepper::Stepper(Window* pWindow) :
    BaseClass(pWindow),
    m_nCurrentIndex(0),
    m_bHorizontal(true),
    m_nNodeSize(24),
    m_bClickToJump(true),
    m_bMuteStepEvent(false),
    m_strConnectorColor(_T("border_control_normal")),
    m_strConnectorActiveColor(_T("color_success")),
    m_strDescColor(_T("text_muted")),
    m_strTitleFont(_T("system_bold_14")),
    m_strDescFont(_T("system_regular_12"))
{
    //各状态节点默认色（语义色名，索引对应 StepStatus）
    m_stateColors[static_cast<int>(StepStatus::kPending)] = _T("text_muted");
    m_stateColors[static_cast<int>(StepStatus::kActive)] = _T("color_accent");
    m_stateColors[static_cast<int>(StepStatus::kCompleted)] = _T("color_success");
    m_stateColors[static_cast<int>(StepStatus::kDisabled)] = _T("border_control_normal");

    //各状态标题默认色
    m_titleColors[static_cast<int>(StepStatus::kPending)] = _T("text_muted");
    m_titleColors[static_cast<int>(StepStatus::kActive)] = _T("text_default");
    m_titleColors[static_cast<int>(StepStatus::kCompleted)] = _T("color_success");
    m_titleColors[static_cast<int>(StepStatus::kDisabled)] = _T("text_muted");
}

Stepper::~Stepper()
{
}

DString Stepper::GetType() const
{
    return DUI_CTR_STEPPER;
}

void Stepper::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("step_title")) || (strName == _T("steptitle"))) {
        m_strItemTitleIds.clear();
        ParseItemsField(strValue, 0);
    }
    else if ((strName == _T("step_title_id")) || (strName == _T("steptitleid"))) {
        m_strItemTitleIds = strValue;
        ParseItemsField(strValue, 0, true);
    }
    else if ((strName == _T("step_desc")) || (strName == _T("stepdesc"))) {
        m_strItemDescIds.clear();
        ParseItemsField(strValue, 1);
    }
    else if ((strName == _T("step_desc_id")) || (strName == _T("stepdescid"))) {
        m_strItemDescIds = strValue;
        ParseItemsField(strValue, 1, true);
    }
    else if ((strName == _T("current")) || (strName == _T("current_index")) || (strName == _T("currentindex"))) {
        int32_t nIndex = StringUtil::StringToInt32(strValue);
        if (nIndex < 0) {
            nIndex = 0;
        }
        m_nCurrentIndex = static_cast<size_t>(nIndex);
        Invalidate();
    }
    else if ((strName == _T("direction")) || (strName == _T("orientation"))) {
        SetHorizontal(!(strValue == _T("vertical")));
    }
    else if ((strName == _T("click_to_jump")) || (strName == _T("clicktojump"))) {
        SetClickToJump(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("node_size")) || (strName == _T("nodesize"))) {
        SetNodeSize(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("disabled_steps")) || (strName == _T("disabledsteps"))) {
        ParseDisabledSteps(strValue);
    }
    else if ((strName == _T("node_color_pending")) || (strName == _T("nodecolorpending"))) {
        SetStateColor(StepStatus::kPending, strValue);
    }
    else if ((strName == _T("node_color_active")) || (strName == _T("nodecoloractive"))) {
        SetStateColor(StepStatus::kActive, strValue);
    }
    else if ((strName == _T("node_color_completed")) || (strName == _T("nodecolorcompleted"))) {
        SetStateColor(StepStatus::kCompleted, strValue);
    }
    else if ((strName == _T("node_color_disabled")) || (strName == _T("nodecolordisabled"))) {
        SetStateColor(StepStatus::kDisabled, strValue);
    }
    else if ((strName == _T("connector_color")) || (strName == _T("connectorcolor"))) {
        SetConnectorColor(strValue);
    }
    else if ((strName == _T("connector_active_color")) || (strName == _T("connectoractivecolor"))) {
        SetConnectorActiveColor(strValue);
    }
    else if ((strName == _T("title_color_active")) || (strName == _T("titlecoloractive"))) {
        SetTitleColor(StepStatus::kActive, strValue);
    }
    else if ((strName == _T("title_color_completed")) || (strName == _T("titlecolorcompleted"))) {
        SetTitleColor(StepStatus::kCompleted, strValue);
    }
    else if ((strName == _T("title_color_pending")) || (strName == _T("titlecolorpending"))) {
        SetTitleColor(StepStatus::kPending, strValue);
    }
    else if ((strName == _T("desc_color")) || (strName == _T("desccolor"))) {
        SetDescColor(strValue);
    }
    else if ((strName == _T("title_font")) || (strName == _T("titlefont"))) {
        SetTitleFont(strValue);
    }
    else if ((strName == _T("desc_font")) || (strName == _T("descfont"))) {
        SetDescFont(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Stepper::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //语言切换时，重新解析多语言数据绑定（step_title_id / step_desc_id）
    if (!m_strItemTitleIds.empty()) {
        ParseItemsField(m_strItemTitleIds, 0, true);
    }
    if (!m_strItemDescIds.empty()) {
        ParseItemsField(m_strItemDescIds, 1, true);
    }
}

void Stepper::ParseItemsField(const DString& strValue, int32_t nField, bool bTranslate)
{
    //按 `|` 分隔步骤，逐条填充指定字段；不足的步骤自动补空步骤
    std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
    size_t nIndex = 0;
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (bTranslate && !s.empty()) {
            //多语言ID模式：按 STRID 从语言文件取实际文本
            s = GlobalManager::GetTextById(s);
        }
        if (nIndex >= m_steps.size()) {
            m_steps.push_back(Step());
        }
        Step& step = m_steps[nIndex];
        switch (nField) {
        case 0: step.title = s; break;
        case 1: step.description = s; break;
        default: break;
        }
        ++nIndex;
    }
    Invalidate();
}

void Stepper::ParseDisabledSteps(const DString& strValue)
{
    //统一分隔符：把逗号替换为竖线，再按 `|` 拆分
    DString str = strValue;
    std::replace(str.begin(), str.end(), _T(','), _T('|'));
    std::list<DString> parts = StringUtil::Split(str, _T("|"));
    m_disabledSteps.clear();
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (s.empty()) {
            continue;
        }
        int32_t nIndex = StringUtil::StringToInt32(s);
        if (nIndex >= 0) {
            m_disabledSteps.push_back(static_cast<size_t>(nIndex));
        }
    }
    Invalidate();
}

size_t Stepper::GetCurrentIndex() const
{
    if (m_steps.empty()) {
        return 0;
    }
    if (m_nCurrentIndex >= m_steps.size()) {
        return m_steps.size() - 1;
    }
    return m_nCurrentIndex;
}

void Stepper::SetCurrentIndex(size_t nIndex)
{
    if (m_steps.empty()) {
        return;
    }
    const size_t nOld = GetCurrentIndex();
    if (nIndex >= m_steps.size()) {
        nIndex = m_steps.size() - 1;
    }
    if (nIndex == nOld) {
        return;
    }
    m_nCurrentIndex = nIndex;
    Invalidate();
    EmitStepChanged(nOld);
}

void Stepper::SetCurrentIndexNoEvent(size_t nIndex)
{
    const bool bOldMute = m_bMuteStepEvent;
    m_bMuteStepEvent = true;
    SetCurrentIndex(nIndex);
    m_bMuteStepEvent = bOldMute;
}

bool Stepper::Next()
{
    if (m_steps.empty()) {
        return false;
    }
    size_t i = GetCurrentIndex();
    while (i + 1 < m_steps.size()) {
        ++i;
        if (!IsStepDisabled(i)) {
            return GoTo(i);
        }
    }
    return false;
}

bool Stepper::NextNoEvent()
{
    const bool bOldMute = m_bMuteStepEvent;
    m_bMuteStepEvent = true;
    const bool bRet = Next();
    m_bMuteStepEvent = bOldMute;
    return bRet;
}

bool Stepper::Back()
{
    if (m_steps.empty()) {
        return false;
    }
    size_t i = GetCurrentIndex();
    while (i > 0) {
        --i;
        if (!IsStepDisabled(i)) {
            return GoTo(i);
        }
    }
    return false;
}

bool Stepper::BackNoEvent()
{
    const bool bOldMute = m_bMuteStepEvent;
    m_bMuteStepEvent = true;
    const bool bRet = Back();
    m_bMuteStepEvent = bOldMute;
    return bRet;
}

bool Stepper::GoTo(size_t nIndex)
{
    if (m_steps.empty()) {
        return false;
    }
    if (nIndex >= m_steps.size()) {
        return false;
    }
    if (IsStepDisabled(nIndex)) {
        return false;
    }
    if (nIndex == GetCurrentIndex()) {
        return false;
    }
    SetCurrentIndex(nIndex);
    return true;
}

bool Stepper::GoToNoEvent(size_t nIndex)
{
    const bool bOldMute = m_bMuteStepEvent;
    m_bMuteStepEvent = true;
    const bool bRet = GoTo(nIndex);
    m_bMuteStepEvent = bOldMute;
    return bRet;
}

void Stepper::EmitStepChanged(size_t nOldIndex)
{
    if (m_bMuteStepEvent) {
        return;
    }
    //当前步骤通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    StepValue value;
    value.currentIndex = GetCurrentIndex();
    value.oldIndex = nOldIndex;
    SendEvent(kEventValueChanged, (WPARAM)2, 0, static_cast<void*>(&value));
}

void Stepper::AddStep(const Step& step)
{
    m_steps.push_back(step);
    Invalidate();
}

void Stepper::AddStep(const DString& title, const DString& description)
{
    Step step;
    step.title = title;
    step.description = description;
    AddStep(step);
}

bool Stepper::InsertStep(size_t index, const Step& step)
{
    if (index > m_steps.size()) {
        return false;
    }
    m_steps.insert(m_steps.begin() + index, step);
    Invalidate();
    return true;
}

bool Stepper::RemoveStepAt(size_t index)
{
    if (index >= m_steps.size()) {
        return false;
    }
    m_steps.erase(m_steps.begin() + index);
    if (m_nCurrentIndex >= m_steps.size() && !m_steps.empty()) {
        m_nCurrentIndex = m_steps.size() - 1;
    }
    Invalidate();
    return true;
}

void Stepper::ClearSteps()
{
    if (!m_steps.empty()) {
        m_steps.clear();
        m_nCurrentIndex = 0;
        m_disabledSteps.clear();
        Invalidate();
    }
}

void Stepper::SetSteps(const std::vector<Step>& steps)
{
    m_steps = steps;
    if (m_nCurrentIndex >= m_steps.size() && !m_steps.empty()) {
        m_nCurrentIndex = m_steps.size() - 1;
    }
    Invalidate();
}

const Stepper::Step* Stepper::GetStepAt(size_t index) const
{
    if (index >= m_steps.size()) {
        return nullptr;
    }
    return &m_steps[index];
}

void Stepper::SetStepDisabled(size_t index, bool bDisabled)
{
    if (index >= m_steps.size()) {
        return;
    }
    auto it = std::find(m_disabledSteps.begin(), m_disabledSteps.end(), index);
    const bool bExists = (it != m_disabledSteps.end());
    if (bDisabled && !bExists) {
        m_disabledSteps.push_back(index);
        Invalidate();
    }
    else if (!bDisabled && bExists) {
        m_disabledSteps.erase(it);
        Invalidate();
    }
}

bool Stepper::IsStepDisabled(size_t index) const
{
    return std::find(m_disabledSteps.begin(), m_disabledSteps.end(), index) != m_disabledSteps.end();
}

void Stepper::SetDisabledSteps(const std::vector<size_t>& disabledSteps)
{
    m_disabledSteps = disabledSteps;
    Invalidate();
}

StepStatus Stepper::GetStepStatus(size_t index) const
{
    if (IsStepDisabled(index)) {
        return StepStatus::kDisabled;
    }
    const size_t nCurrent = GetCurrentIndex();
    if (index < nCurrent) {
        return StepStatus::kCompleted;
    }
    if (index == nCurrent) {
        return StepStatus::kActive;
    }
    return StepStatus::kPending;
}

void Stepper::SetHorizontal(bool bHorizontal)
{
    if (m_bHorizontal != bHorizontal) {
        m_bHorizontal = bHorizontal;
        Invalidate();
    }
}

void Stepper::SetNodeSize(int32_t nSize)
{
    if (nSize < 12) {
        nSize = 12;
    }
    if (nSize > 96) {
        nSize = 96;
    }
    if (m_nNodeSize != nSize) {
        m_nNodeSize = nSize;
        Invalidate();
    }
}

void Stepper::SetClickToJump(bool bEnable)
{
    m_bClickToJump = bEnable;
}

void Stepper::SetStateColor(StepStatus status, const DString& strColor)
{
    const int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 4)) {
        m_stateColors[nIndex] = strColor;
        Invalidate();
    }
}

const DString& Stepper::GetStateColor(StepStatus status) const
{
    const int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 4)) {
        return m_stateColors[nIndex];
    }
    static const DString kEmpty;
    return kEmpty;
}

void Stepper::SetTitleColor(StepStatus status, const DString& strColor)
{
    const int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 4)) {
        m_titleColors[nIndex] = strColor;
        Invalidate();
    }
}

const DString& Stepper::GetTitleColor(StepStatus status) const
{
    const int32_t nIndex = static_cast<int>(status);
    if ((nIndex >= 0) && (nIndex < 4)) {
        return m_titleColors[nIndex];
    }
    static const DString kEmpty;
    return kEmpty;
}

void Stepper::SetConnectorColor(const DString& strColor)
{
    if (m_strConnectorColor != strColor) {
        m_strConnectorColor = strColor;
        Invalidate();
    }
}

void Stepper::SetConnectorActiveColor(const DString& strColor)
{
    if (m_strConnectorActiveColor != strColor) {
        m_strConnectorActiveColor = strColor;
        Invalidate();
    }
}

void Stepper::SetDescColor(const DString& strColor)
{
    if (m_strDescColor != strColor) {
        m_strDescColor = strColor;
        Invalidate();
    }
}

void Stepper::SetTitleFont(const DString& strFontId)
{
    if (m_strTitleFont != strFontId) {
        m_strTitleFont = strFontId;
        Invalidate();
    }
}

void Stepper::SetDescFont(const DString& strFontId)
{
    if (m_strDescFont != strFontId) {
        m_strDescFont = strFontId;
        Invalidate();
    }
}

UiRect Stepper::CalcStepSlotRect(const UiRect& rc, size_t nCount, size_t index) const
{
    if (nCount == 0) {
        return UiRect();
    }
    if (m_bHorizontal) {
        const double slotW = static_cast<double>(rc.Width()) / static_cast<double>(nCount);
        const int32_t left = rc.left + static_cast<int32_t>(slotW * static_cast<double>(index));
        const int32_t right = rc.left + static_cast<int32_t>(slotW * static_cast<double>(index + 1));
        return UiRect(left, rc.top, right, rc.bottom);
    }
    const double slotH = static_cast<double>(rc.Height()) / static_cast<double>(nCount);
    const int32_t top = rc.top + static_cast<int32_t>(slotH * static_cast<double>(index));
    const int32_t bottom = rc.top + static_cast<int32_t>(slotH * static_cast<double>(index + 1));
    return UiRect(rc.left, top, rc.right, bottom);
}

int32_t Stepper::HitTestStep(const UiRect& rc, size_t nCount, const UiPoint& ptMouse) const
{
    if (nCount == 0) {
        return -1;
    }
    if (m_bHorizontal) {
        const double slotW = static_cast<double>(rc.Width()) / static_cast<double>(nCount);
        if (slotW <= 0) {
            return -1;
        }
        int32_t nIndex = static_cast<int32_t>((ptMouse.x - rc.left) / slotW);
        if ((nIndex < 0) || (nIndex >= static_cast<int32_t>(nCount))) {
            return -1;
        }
        return nIndex;
    }
    const double slotH = static_cast<double>(rc.Height()) / static_cast<double>(nCount);
    if (slotH <= 0) {
        return -1;
    }
    int32_t nIndex = static_cast<int32_t>((ptMouse.y - rc.top) / slotH);
    if ((nIndex < 0) || (nIndex >= static_cast<int32_t>(nCount))) {
        return -1;
    }
    return nIndex;
}

bool Stepper::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::ButtonDown(msg);
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (m_bClickToJump) {
        const int32_t nHit = HitTestStep(rc, m_steps.size(), msg.ptMouse);
        if (nHit >= 0) {
            GoTo(static_cast<size_t>(nHit));
            return true;
        }
    }
    return BaseClass::ButtonDown(msg);
}

void Stepper::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if ((rc.Width() <= 0) || (rc.Height() <= 0) || m_steps.empty()) {
        return;
    }

    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);
    if (m_bHorizontal) {
        PaintHorizontal(pRender, rc);
    }
    else {
        PaintVertical(pRender, rc);
    }
    spClip.reset();
}

void Stepper::PaintHorizontal(IRender* pRender, const UiRect& rc)
{
    const size_t nCount = m_steps.size();
    const int32_t nRadius = Dpi().GetScaleInt(m_nNodeSize) / 2;
    const int32_t nEdge = Dpi().GetScaleInt(2);
    const int32_t nGap = Dpi().GetScaleInt(6);
    const double slotW = static_cast<double>(rc.Width()) / static_cast<double>(nCount);
    const int32_t nNodeCenterY = rc.top + nRadius + nGap;

    //连接线（先画，节点覆盖其上）
    UiColor connColor = GetUiColor(m_strConnectorColor);
    if (connColor.IsEmpty()) {
        connColor = UiColor(0xFFD0D0D0);
    }
    UiColor connActive = GetUiColor(m_strConnectorActiveColor);
    if (connActive.IsEmpty()) {
        connActive = connColor;
    }
    const float fLineWidth = static_cast<float>(std::max(1, Dpi().GetScaleInt(2)));
    const size_t nCurrent = GetCurrentIndex();
    for (size_t i = 0; i + 1 < nCount; ++i) {
        const int32_t x1 = rc.left + static_cast<int32_t>(slotW * (static_cast<double>(i) + 0.5)) + nRadius + nEdge;
        const int32_t x2 = rc.left + static_cast<int32_t>(slotW * (static_cast<double>(i) + 1.5)) - nRadius - nEdge;
        if (x2 <= x1) {
            continue;
        }
        const UiColor color = (i < nCurrent) ? connActive : connColor;
        pRender->DrawLine(UiPointF(static_cast<float>(x1), static_cast<float>(nNodeCenterY)),
                          UiPointF(static_cast<float>(x2), static_cast<float>(nNodeCenterY)),
                          color, fLineWidth);
    }

    //节点与文本
    const int32_t nTextTop = nNodeCenterY + nRadius + nGap;
    for (size_t i = 0; i < nCount; ++i) {
        const int32_t cx = rc.left + static_cast<int32_t>(slotW * (static_cast<double>(i) + 0.5));
        const StepStatus status = GetStepStatus(i);
        PaintNode(pRender, UiPoint(cx, nNodeCenterY), nRadius, status, i, status == StepStatus::kDisabled);

        const int32_t slotLeft = rc.left + static_cast<int32_t>(slotW * static_cast<double>(i));
        const int32_t slotRight = rc.left + static_cast<int32_t>(slotW * static_cast<double>(i + 1));
        UiRect rcText(slotLeft + nGap, nTextTop, slotRight - nGap, rc.bottom);
        if (rcText.Width() <= 0) {
            continue;
        }

        const Step& step = m_steps[i];
        int32_t y = nTextTop;
        if (!step.title.empty()) {
            const int32_t nTitleH = MeasureTextHeight(pRender, step.title, rcText.Width(), m_strTitleFont);
            UiRect rcTitle(rcText.left, y, rcText.right, y + nTitleH);
            DrawStepperText(pRender, step.title, rcTitle, TEXT_HCENTER | TEXT_SINGLELINE | TEXT_TOP,
                            GetUiColor(GetTitleColor(status)), m_strTitleFont);
            y += nTitleH + nEdge;
        }
        if (!step.description.empty()) {
            UiRect rcDesc(rcText.left, y, rcText.right, rc.bottom);
            DrawStepperText(pRender, step.description, rcDesc, TEXT_HCENTER | TEXT_TOP,
                            GetUiColor(m_strDescColor), m_strDescFont);
        }
    }
}

void Stepper::PaintVertical(IRender* pRender, const UiRect& rc)
{
    const size_t nCount = m_steps.size();
    const int32_t nRadius = Dpi().GetScaleInt(m_nNodeSize) / 2;
    const int32_t nEdge = Dpi().GetScaleInt(2);
    const int32_t nGap = Dpi().GetScaleInt(8);
    const double slotH = static_cast<double>(rc.Height()) / static_cast<double>(nCount);
    const int32_t nNodeCenterX = rc.left + nRadius + nGap;

    //连接线
    UiColor connColor = GetUiColor(m_strConnectorColor);
    if (connColor.IsEmpty()) {
        connColor = UiColor(0xFFD0D0D0);
    }
    UiColor connActive = GetUiColor(m_strConnectorActiveColor);
    if (connActive.IsEmpty()) {
        connActive = connColor;
    }
    const float fLineWidth = static_cast<float>(std::max(1, Dpi().GetScaleInt(2)));
    const size_t nCurrent = GetCurrentIndex();
    for (size_t i = 0; i + 1 < nCount; ++i) {
        const int32_t y1 = rc.top + static_cast<int32_t>(slotH * (static_cast<double>(i) + 0.5)) + nRadius + nEdge;
        const int32_t y2 = rc.top + static_cast<int32_t>(slotH * (static_cast<double>(i) + 1.5)) - nRadius - nEdge;
        if (y2 <= y1) {
            continue;
        }
        const UiColor color = (i < nCurrent) ? connActive : connColor;
        pRender->DrawLine(UiPointF(static_cast<float>(nNodeCenterX), static_cast<float>(y1)),
                          UiPointF(static_cast<float>(nNodeCenterX), static_cast<float>(y2)),
                          color, fLineWidth);
    }

    //节点与文本
    const int32_t nTextLeft = nNodeCenterX + nRadius + nGap;
    for (size_t i = 0; i < nCount; ++i) {
        const int32_t cy = rc.top + static_cast<int32_t>(slotH * (static_cast<double>(i) + 0.5));
        const StepStatus status = GetStepStatus(i);
        PaintNode(pRender, UiPoint(nNodeCenterX, cy), nRadius, status, i, status == StepStatus::kDisabled);

        UiRect rcText(nTextLeft, rc.top, rc.right, rc.bottom);
        if (rcText.Width() <= 0) {
            continue;
        }

        const Step& step = m_steps[i];
        //标题 + 描述竖向居中于节点行
        int32_t nTitleH = 0;
        if (!step.title.empty()) {
            nTitleH = MeasureTextHeight(pRender, step.title, rcText.Width(), m_strTitleFont);
        }
        int32_t nDescH = 0;
        if (!step.description.empty()) {
            nDescH = MeasureTextHeight(pRender, step.description, rcText.Width(), m_strDescFont);
        }
        int32_t nTotalH = nTitleH + ((nTitleH > 0 && nDescH > 0) ? nEdge : 0) + nDescH;
        int32_t y = cy - nTotalH / 2;
        if ((y < rc.top) || (nTotalH >= rc.Height())) {
            y = cy - nTotalH / 2;
        }
        if (!step.title.empty()) {
            UiRect rcTitle(rcText.left, y, rcText.right, y + nTitleH);
            DrawStepperText(pRender, step.title, rcTitle, TEXT_LEFT | TEXT_SINGLELINE | TEXT_VCENTER,
                            GetUiColor(GetTitleColor(status)), m_strTitleFont);
            y += nTitleH + nEdge;
        }
        if (!step.description.empty()) {
            UiRect rcDesc(rcText.left, y, rcText.right, y + nDescH);
            DrawStepperText(pRender, step.description, rcDesc, TEXT_LEFT | TEXT_VCENTER,
                            GetUiColor(m_strDescColor), m_strDescFont);
        }
    }
}

void Stepper::PaintNode(IRender* pRender, const UiPoint& ptCenter, int32_t nRadius, StepStatus status,
                        size_t index, bool bDimmed)
{
    UiColor nodeColor = GetUiColor(m_stateColors[static_cast<int>(status)]);
    if (nodeColor.IsEmpty()) {
        nodeColor = UiColor(0xFF999999);
    }
    uint8_t uAlpha = bDimmed ? static_cast<uint8_t>(GetAlpha() * 3 / 5) : GetAlpha();
    const UiPointF centerPt(static_cast<float>(ptCenter.x), static_cast<float>(ptCenter.y));
    pRender->FillCircle(centerPt, static_cast<float>(nRadius), nodeColor, uAlpha);

    if (status == StepStatus::kCompleted) {
        //已完成：节点内绘制对勾
        const UiColor markColor(0xFFFFFFFF);
        const float fWidth = static_cast<float>(std::max(1, Dpi().GetScaleInt(2)));
        const float cx = static_cast<float>(ptCenter.x);
        const float cy = static_cast<float>(ptCenter.y);
        const float r = static_cast<float>(nRadius);
        pRender->DrawLine(UiPointF(cx - r * 0.45f, cy + r * 0.05f),
                          UiPointF(cx - r * 0.10f, cy + r * 0.40f),
                          markColor, fWidth);
        pRender->DrawLine(UiPointF(cx - r * 0.10f, cy + r * 0.40f),
                          UiPointF(cx + r * 0.50f, cy - r * 0.38f),
                          markColor, fWidth);
    }
    else {
        //待开始 / 进行中 / 禁用：节点内绘制序号
        const DString strNum = StringUtil::Printf(_T("%d"), static_cast<int>(index + 1));
        UiRect rcText(ptCenter.x - nRadius, ptCenter.y - nRadius, ptCenter.x + nRadius, ptCenter.y + nRadius);
        DrawStepperText(pRender, strNum, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                        UiColor(0xFFFFFFFF), _T("system_bold_12"));
    }
}

void Stepper::DrawStepperText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                              UiColor textColor, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = textColor;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

int32_t Stepper::MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty() || (nWidth <= 0)) {
        return 0;
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(strFontId);
    param.rectSize = nWidth;
    UiRect rc = pRender->MeasureString(strText, param);
    return rc.Height();
}

} // namespace ui
