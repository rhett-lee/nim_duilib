#include "Control.h"
#include "duilib/Core/ControlLoading.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/StateColorMap.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/PerformanceUtil.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include "ControlDropTargetImpl_Windows.h"
#endif

#ifdef DUILIB_BUILD_FOR_SDL
    #include "ControlDropTargetImpl_SDL.h"
#endif

namespace ui 
{
Control::Control(Window* pWindow) :
    PlaceHolder(pWindow),
    m_bContextMenuUsed(false),
    m_bMouseFocused(false),
    m_bNoFocus(false),
    m_bAllowTabstop(true),
    m_cursorType(CursorType::kCursorArrow),
    m_controlState(kControlStateNormal),
    m_nAlpha(255),
    m_nHotAlpha(0),
    m_bBoxShadowPainted(false),
    m_uUserDataID((size_t)-1),
    m_bShowFocusRect(false),
    m_nPaintOrder(0),
    m_bBordersOnTop(true)
{
}

Control::~Control()
{
    //从延迟绘制列表中删除
    GlobalManager::Instance().Image().RemoveDelayPaintData(this);

    //清理动画相关资源，避免定时器再产生回调，引发错误
    if (m_pAnimationData != nullptr) {
        if (m_pAnimationData->m_animationManager != nullptr) {
            m_pAnimationData->m_animationManager->Clear(this);
        }
        m_pAnimationData->m_animationManager.reset();
    }

    Window* pWindow = GetWindow();
    if (pWindow) {
        pWindow->ReapObjects(this);
    }
}

DString Control::GetType() const { return DUI_CTR_CONTROL; }

void Control::SetAttribute(const DString& strName, const DString& strValue)
{
    ASSERT(GetWindow() != nullptr);//由于需要做DPI感知功能，所以必须先设置关联窗口
    if (strName == _T("class")) {
        SetClass(strValue);
    }
    else if (strName == _T("halign")) {
        if (strValue == _T("left")) {
            SetHorAlignType(HorAlignType::kAlignLeft);
        }
        else if (strValue == _T("center")) {
            SetHorAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue == _T("right")) {
            SetHorAlignType(HorAlignType::kAlignRight);
        }
        else {
            ASSERT(0);
        }
    }
    else if (strName == _T("valign")) {
        if (strValue == _T("top")) {
            SetVerAlignType(VerAlignType::kAlignTop);
        }
        else if (strValue == _T("center")) {
            SetVerAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue == _T("bottom")) {
            SetVerAlignType(VerAlignType::kAlignBottom);
        }
        else {
            ASSERT(0);
        }
    }
    else if (strName == _T("align")) {
        //水平方向对齐方式
        if (strValue.find(_T("left")) != DString::npos) {
            SetHorAlignType(HorAlignType::kAlignLeft);
        }
        else if (strValue.find(_T("hcenter")) != DString::npos) {
            SetHorAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            SetHorAlignType(HorAlignType::kAlignRight);
        }
        //垂直方向对齐方式
        if (strValue.find(_T("top")) != DString::npos) {
            SetVerAlignType(VerAlignType::kAlignTop);
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            SetVerAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            SetVerAlignType(VerAlignType::kAlignBottom);
        }
    }
    else if (strName == _T("margin")) {
        UiMargin rcMargin;
        AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
        SetMargin(rcMargin, true);
    }
    else if (strName == _T("padding")) {
        UiPadding rcPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
        SetPadding(rcPadding, true);
    }
    else if (strName == _T("control_padding")) {
        SetEnableControlPadding(strValue == _T("true"));
    }
    else if (strName == _T("bkcolor")) {
        //背景色
        SetBkColor(strValue);
    }
    else if (strName == _T("bkcolor2")) {
        //第二背景色（实现渐变背景色）
        SetBkColor2(strValue);
    }
    else if (strName == _T("bkcolor2_direction")) {
        //第二背景色的方向："1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下
        SetBkColor2Direction(strValue);
    }
    else if (strName == _T("fore_color")) {
        //前景色
        SetForeColor(strValue);
    }
    else if ((strName == _T("border_size")) || (strName == _T("bordersize"))) {
        //边线宽度
        DString nValue = strValue;
        if (nValue.find(_T(',')) == DString::npos) {
            int32_t nBorderSize = StringUtil::StringToInt32(strValue);
            if (nBorderSize < 0) {
                nBorderSize = 0;
            }
            UiRectF rcBorder((float)nBorderSize, (float)nBorderSize, (float)nBorderSize, (float)nBorderSize);
            SetBorderSize(rcBorder, true);
        }
        else {
            UiMargin rcMargin;
            AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
            UiRectF rcBorder((float)rcMargin.left, (float)rcMargin.top, (float)rcMargin.right, (float)rcMargin.bottom);
            SetBorderSize(rcBorder, true);
        }
    }
    else if (strName == _T("border_dash_style")) {
        //边线的线形（四个边的边线的线形只能一致，不支持分开设置）
        IPen::DashStyle dashStyle = IPen::kDashStyleSolid;
        if (strValue == _T("solid")) {
            dashStyle = IPen::kDashStyleSolid;
        }
        else if (strValue == _T("dash")) {
            dashStyle = IPen::kDashStyleDash;
        }
        else if (strValue == _T("dot")) {
            dashStyle = IPen::kDashStyleDot;
        }
        else if (strValue == _T("dash_dot")) {
            dashStyle = IPen::kDashStyleDashDot;
        }
        else if (strValue == _T("dash_dot_dot")) {
            dashStyle = IPen::kDashStyleDashDotDot;
        }
        SetBorderDashStyle((int8_t)dashStyle);
    }
    else if (strName == _T("borders_on_top")) {
        //边框是否在顶层（即先绘制子控件，后绘制边框，避免边框被子控件覆盖）
        SetBordersOnTop(strValue == _T("true"));
    }
    else if ((strName == _T("border_round")) || (strName == _T("borderround"))) {
        //圆角大小
        UiSize cxyRound;
        AttributeUtil::ParseSizeValue(strValue.c_str(), cxyRound);
        SetBorderRound(cxyRound);
    }
    else if ((strName == _T("box_shadow")) || (strName == _T("boxshadow"))) {
        SetBoxShadow(strValue);
    }
    else if (strName == _T("width")) {
        if (strValue == _T("stretch")) {
            //宽度为拉伸：由父容器负责分配宽度
            SetFixedWidth(UiFixedInt::MakeStretch(), true, true);
        }
        else if (strValue == _T("auto")) {
            //宽度为自动：根据控件的文本、图片等自动计算宽度
            SetFixedWidth(UiFixedInt::MakeAuto(), true, true);
        }
        else if (!strValue.empty()) {
            if (strValue.back() == _T('%')) {
                //宽度为拉伸：由父容器负责按百分比分配宽度，比如 width="30%"，代表该控件的宽度期望值为父控件宽度的30%
                int32_t iValue = StringUtil::StringToInt32(strValue);
                if ((iValue <= 0) || (iValue > 100)) {
                    iValue = 100;
                }
                SetFixedWidth(UiFixedInt::MakeStretch(iValue), true, false);
            }
            else {
                //宽度为固定值
                ASSERT(StringUtil::StringToInt32(strValue) >= 0);
                SetFixedWidth(UiFixedInt(StringUtil::StringToInt32(strValue)), true, true);
            }
        }
        else {
            SetFixedWidth(UiFixedInt(0), true, true);
        }
    }
    else if (strName == _T("height")) {
        if (strValue == _T("stretch")) {
            //高度为拉伸：由父容器负责分配高度
            SetFixedHeight(UiFixedInt::MakeStretch(), true, true);
        }
        else if (strValue == _T("auto")) {
            //高度为自动：根据控件的文本、图片等自动计算高度
            SetFixedHeight(UiFixedInt::MakeAuto(), true, true);
        }
        else if (!strValue.empty()) {
            if (strValue.back() == _T('%')) {
                //高度为拉伸：由父容器负责按百分比分配高度，比如 height="30%"，代表该控件的高度期望值为父控件高度的30%
                int32_t iValue = StringUtil::StringToInt32(strValue);
                if ((iValue <= 0) || (iValue > 100)) {
                    iValue = 100;
                }
                SetFixedHeight(UiFixedInt::MakeStretch(iValue), true, false);
            }
            else {
                //高度为固定值
                ASSERT(StringUtil::StringToInt32(strValue) >= 0);
                SetFixedHeight(UiFixedInt(StringUtil::StringToInt32(strValue)), true, true);
            }
        }
        else {
            SetFixedHeight(UiFixedInt(0), true, true);
        }
    }
    else if (strName == _T("state")) {
        if (strValue == _T("normal")) {
            SetState(kControlStateNormal);
        }
        else if (strValue == _T("hot")) {
            SetState(kControlStateHot);
        }
        else if (strValue == _T("pushed")) {
            SetState(kControlStatePushed);
        }
        else if (strValue == _T("disabled")) {
            SetState(kControlStateDisabled);
        }
        else {
            ASSERT(0);
        }
    }
    else if ((strName == _T("cursor_type")) || (strName == _T("cursortype"))) {
        if (strValue == _T("arrow")) {
            SetCursorType(CursorType::kCursorArrow);
        }
        else if (strValue == _T("ibeam")) {
            SetCursorType(CursorType::kCursorIBeam);
        }
        else if (strValue == _T("hand")) {
            SetCursorType(CursorType::kCursorHand);
        }
        else if (strValue == _T("wait")) {
            SetCursorType(CursorType::kCursorWait);
        }
        else if (strValue == _T("cross")) {
            SetCursorType(CursorType::kCursorCross);
        }
        else if (strValue == _T("size_we")) {
            SetCursorType(CursorType::kCursorSizeWE);
        }
        else if (strValue == _T("size_ns")) {
            SetCursorType(CursorType::kCursorSizeNS);
        }
        else if (strValue == _T("size_nwse")) {
            SetCursorType(CursorType::kCursorSizeNWSE);
        }
        else if (strValue == _T("size_nesw")) {
            SetCursorType(CursorType::kCursorSizeNESW);
        }
        else if (strValue == _T("size_all")) {
            SetCursorType(CursorType::kCursorSizeAll);
        }
        else if (strValue == _T("no")) {
            SetCursorType(CursorType::kCursorNo);
        }
        else if (strValue == _T("progress")) {
            SetCursorType(CursorType::kCursorProgress);
        }
        else {
            ASSERT(0);
        }
    }
    else if ((strName == _T("render_offset")) || (strName == _T("renderoffset"))) {
        UiPoint renderOffset;
        AttributeUtil::ParsePointValue(strValue.c_str(), renderOffset);
        SetRenderOffset(renderOffset, true);
    }
    else if ((strName == _T("normal_color")) || (strName == _T("normalcolor"))) {
        SetStateColor(kControlStateNormal, strValue);
    }
    else if ((strName == _T("hot_color")) || (strName == _T("hotcolor"))) {
        SetStateColor(kControlStateHot, strValue);
    }
    else if ((strName == _T("pushed_color")) || (strName == _T("pushedcolor"))) {
        SetStateColor(kControlStatePushed, strValue);
    }
    else if ((strName == _T("disabled_color")) || (strName == _T("disabledcolor"))) {
        SetStateColor(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("border_color")) || (strName == _T("bordercolor"))) {
        SetBorderColor(strValue);
    }
    else if (strName == _T("normal_border_color")) {
        SetBorderColor(kControlStateNormal, strValue);
    }
    else if (strName == _T("hot_border_color")) {
        SetBorderColor(kControlStateHot, strValue);
    }
    else if (strName == _T("pushed_border_color")) {
        SetBorderColor(kControlStatePushed, strValue);
    }
    else if (strName == _T("disabled_border_color")) {
        SetBorderColor(kControlStateDisabled, strValue);
    }
    else if (strName == _T("focus_border_color")) {
        SetFocusBorderColor(strValue);
    }
    else if ((strName == _T("left_border_size")) || (strName == _T("leftbordersize"))) {
        SetLeftBorderSize((float)StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("top_border_size")) || (strName == _T("topbordersize"))) {
        SetTopBorderSize((float)StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("right_border_size")) || (strName == _T("rightbordersize"))) {
        SetRightBorderSize((float)StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("bottom_border_size")) || (strName == _T("bottombordersize"))) {
        SetBottomBorderSize((float)StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("bkimage")) {
        SetBkImage(strValue);
    }
    else if ((strName == _T("min_width")) || (strName == _T("minwidth"))) {
        SetMinWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("max_width")) || (strName == _T("maxwidth"))) {
        SetMaxWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("min_height")) || (strName == _T("minheight"))) {
        SetMinHeight(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("max_height")) || (strName == _T("maxheight"))) {
        SetMaxHeight(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("name")) {
        SetName(strValue);
    }
    else if ((strName == _T("tooltip_text")) || (strName == _T("tooltiptext"))) {
        SetToolTipText(strValue);
    }
    else if ((strName == _T("tooltip_text_id")) || (strName == _T("tooltip_textid")) || (strName == _T("tooltiptextid"))) {
        SetToolTipTextId(strValue);
    }
    else if (strName == _T("tooltip_width")) {

        SetToolTipWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("data_id")) || (strName == _T("dataid"))) {
        SetDataID(strValue);
    }
    else if ((strName == _T("user_data_id")) || (strName == _T("user_dataid"))) {
        SetUserDataID(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("enabled")) {
        SetEnabled(strValue == _T("true"));
    }
    else if ((strName == _T("mouse_enabled")) || (strName == _T("mouse"))) {
        SetMouseEnabled(strValue == _T("true"));
    }
    else if ((strName == _T("keyboard_enabled")) || (strName == _T("keyboard"))) {
        SetKeyboardEnabled(strValue == _T("true"));
    }
    else if (strName == _T("visible")) {
        SetVisible(strValue == _T("true"));
    }
    else if ((strName == _T("fade_visible")) || (strName == _T("fadevisible"))) {
        SetFadeVisible(strValue == _T("true"));
    }
    else if (strName == _T("float")) {
        SetFloat(strValue == _T("true"));
    }
    else if (strName == _T("keep_float_pos")) {
        SetKeepFloatPos(strValue == _T("true"));
    }
    else if (strName == _T("cache")) {
        //忽略该选项：对应功能已经删除
    }
    else if ((strName == _T("no_focus")) || (strName == _T("nofocus"))) {
        SetNoFocus();
    }
    else if (strName == _T("alpha")) {
        SetAlpha(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("normal_image")) || (strName == _T("normalimage"))) {
        SetStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == _T("hot_image")) || (strName == _T("hotimage"))) {
        SetStateImage(kControlStateHot, strValue);
    }
    else if ((strName == _T("pushed_image")) || (strName == _T("pushedimage"))) {
        SetStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == _T("disabled_image")) || (strName == _T("disabledimage"))) {
        SetStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("fore_normal_image")) || (strName == _T("forenormalimage"))) {
        SetForeStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == _T("fore_hot_image")) || (strName == _T("forehotimage"))) {
        SetForeStateImage(kControlStateHot, strValue);
    }
    else if ((strName == _T("fore_pushed_image")) || (strName == _T("forepushedimage"))) {
        SetForeStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == _T("fore_disabled_image")) || (strName == _T("foredisabledimage"))) {
        SetForeStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("fade_alpha")) || (strName == _T("fadealpha"))) {
        GetAnimationManager().SetFadeAlpha(strValue == _T("true"));
    }
    else if ((strName == _T("fade_hot")) || (strName == _T("fadehot"))) {
        GetAnimationManager().SetFadeHot(strValue == _T("true"));
    }
    else if ((strName == _T("fade_width")) || (strName == _T("fadewidth"))) {
        GetAnimationManager().SetFadeWidth(strValue == _T("true"));
    }
    else if ((strName == _T("fade_height")) || (strName == _T("fadeheight"))) {
        GetAnimationManager().SetFadeHeight(strValue == _T("true"));
    }
    else if ((strName == _T("fade_in_out_x_from_left")) || (strName == _T("fadeinoutxfromleft"))) {
        GetAnimationManager().SetFadeInOutX(strValue == _T("true"), false);
    }
    else if ((strName == _T("fade_in_out_x_from_right")) || (strName == _T("fadeinoutxfromright"))) {
        GetAnimationManager().SetFadeInOutX(strValue == _T("true"), true);
    }
    else if ((strName == _T("fade_in_out_y_from_top")) || (strName == _T("fadeinoutyfromtop"))) {
        GetAnimationManager().SetFadeInOutY(strValue == _T("true"), false);
    }
    else if ((strName == _T("fade_in_out_y_from_bottom")) || (strName == _T("fadeinoutyfrombottom"))) {
        GetAnimationManager().SetFadeInOutY(strValue == _T("true"), true);
    }
    else if ((strName == _T("tab_stop")) || (strName == _T("tabstop"))) {
        SetTabStop(strValue == _T("true"));
    }
    else if (strName == _T("loading")) {
        SetLoadingAttribute(strValue);
    }
    else if (strName == _T("show_focus_rect")) {
        SetShowFocusRect(strValue == _T("true"));
    }
    else if (strName == _T("focus_rect_color")) {
        SetFocusRectColor(strValue);
    }
    else if (strName == _T("paint_order")) {
        uint8_t nPaintOrder = TruncateToUInt8(StringUtil::StringToInt32(strValue));
        SetPaintOrder(nPaintOrder);
    }
    else if ((strName == _T("start_image_animation")) || (strName == _T("start_gif_play"))) {
        ParseStartImageAnimation(strValue);
    }
    else if ((strName == _T("stop_image_animation")) || (strName == _T("stop_gif_play"))) {
        ParseStopImageAnimation(strValue);
    }
    else if (strName == _T("set_image_animation_frame")) {
        ParseSetImageAnimationFrame(strValue);
    }
    else if (strName == _T("enable_drag_drop")) {
        //是否允许拖放操作
        SetEnableDragDrop(strValue == _T("true"));
    }
    else if (strName == _T("enable_drop_file")) {
        //是否允许拖放文件操作
        SetEnableDropFile(strValue == _T("true"));
    }
    else if (strName == _T("drop_file_types")) {
        //拖放文件的扩展名列表
        SetDropFileTypes(strValue);
    }
    else if (strName == _T("row_span")) {
        //设置单元格合并属性（占几行），仅在GridLayout布局中生效
        SetRowSpan(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("col_span")) {
        //设置单元格合并属性（占几列），仅在GridLayout布局中生效
        SetColumnSpan(StringUtil::StringToInt32(strValue));
    }
    else {
        ASSERT(!"Control::SetAttribute失败: 发现不能识别的属性");
    }
}

void Control::ParseStartImageAnimation(const DString& value)
{
    std::vector<DString> paramList;
    auto params = StringUtil::Split(value, _T(","));
    for (DString& v : params) {
        StringUtil::Trim(v);
        paramList.push_back(v);
    }
    DString imageName;
    AnimationImagePos nStartFrame = AnimationImagePos::kFrameCurrent;
    int32_t nPlayCount = 0;
    if (paramList.size() > 0) {
        imageName = paramList[0];
    }
    if (paramList.size() > 1) {
        int32_t nFrame = StringUtil::StringToInt32(paramList[1]);
        if (nFrame == 0) {
            nStartFrame = AnimationImagePos::kFrameFirst;
        }
        else if (nFrame == 2) {
            nStartFrame = AnimationImagePos::kFrameLast;
        }
        else {
            ASSERT(nFrame == 1);
        }
    }
    if (paramList.size() > 2) {
        nPlayCount = StringUtil::StringToInt32(paramList[2]);
    }
    StartImageAnimation(imageName, nStartFrame, nPlayCount);
}

void Control::ParseStopImageAnimation(const DString& value)
{
    std::vector<DString> paramList;
    auto params = StringUtil::Split(value, _T(","));
    for (DString& v : params) {
        StringUtil::Trim(v);
        paramList.push_back(v);
    }
    DString imageName;
    AnimationImagePos nStartFrame = AnimationImagePos::kFrameCurrent;
    bool bTriggerEvent = true;
    if (paramList.size() > 0) {
        imageName = paramList[0];
    }
    if (paramList.size() > 1) {
        int32_t nFrame = StringUtil::StringToInt32(paramList[1]);
        if (nFrame == 0) {
            nStartFrame = AnimationImagePos::kFrameFirst;
        }
        else if (nFrame == 2) {
            nStartFrame = AnimationImagePos::kFrameLast;
        }
        else {
            ASSERT(nFrame == 1);
        }
    }
    if (paramList.size() > 2) {
        bTriggerEvent = (paramList[2] == _T("true")) || (paramList[2] == _T("1"));
    }
    StopImageAnimation(imageName, nStartFrame, bTriggerEvent);
}

void Control::ParseSetImageAnimationFrame(const DString& value)
{
    std::vector<DString> paramList;
    auto params = StringUtil::Split(value, _T(","));
    for (DString& v : params) {
        StringUtil::Trim(v);
        paramList.push_back(v);
    }
    DString imageName;
    int32_t nFrameIndex = -1;
    if (paramList.size() > 0) {
        imageName = paramList[0];
    }
    if (paramList.size() > 1) {
        nFrameIndex = StringUtil::StringToInt32(paramList[1]);        
    }
    SetImageAnimationFrame(imageName, nFrameIndex);
}

void Control::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiMargin rcMargin = GetMargin();
    rcMargin = Dpi().GetScaleMargin(rcMargin, nOldDpiScale);
    SetMargin(rcMargin, false);

    UiPadding rcPadding = GetPadding();
    rcPadding = Dpi().GetScalePadding(rcPadding, nOldDpiScale);
    SetPadding(rcPadding, false);

    UiRectF rcBorder = GetBorderSize();
    rcBorder.left = Dpi().GetScaleFloat(rcBorder.left, nOldDpiScale);
    rcBorder.top = Dpi().GetScaleFloat(rcBorder.top, nOldDpiScale);
    rcBorder.right = Dpi().GetScaleFloat(rcBorder.right, nOldDpiScale);
    rcBorder.bottom = Dpi().GetScaleFloat(rcBorder.bottom, nOldDpiScale);
    SetBorderSize(rcBorder, false);
   
    UiPoint renderOffset = GetRenderOffset();
    renderOffset = Dpi().GetScalePoint(renderOffset, nOldDpiScale);
    SetRenderOffset(renderOffset, false);

    int32_t nMinWidth = GetMinWidth();
    if (nMinWidth >= 0) {
        nMinWidth = Dpi().GetScaleInt(nMinWidth, nOldDpiScale);
        SetMinWidth(nMinWidth, false);
    }

    int32_t nMaxWidth = GetMaxWidth();
    if ((nMaxWidth >= 0) && (nMaxWidth != INT32_MAX)){
        nMaxWidth = Dpi().GetScaleInt(nMaxWidth, nOldDpiScale);
        SetMaxWidth(nMaxWidth, false);
    }

    int32_t nMinHeight = GetMinHeight();
    if (nMinHeight >= 0) {
        nMinHeight = Dpi().GetScaleInt(nMinHeight, nOldDpiScale);
        SetMinHeight(nMinHeight, false);
    }    

    int32_t nMaxHeight = GetMaxHeight();
    if ((nMaxHeight >= 0) && (nMaxHeight != INT32_MAX)) {
        nMaxHeight = Dpi().GetScaleInt(nMaxHeight, nOldDpiScale);
        SetMaxHeight(nMaxHeight, false);
    }

    int32_t nToolTipWidth = GetToolTipWidth();
    if (nToolTipWidth >= 0) {
        nToolTipWidth = Dpi().GetScaleInt(nToolTipWidth, nOldDpiScale);
        SetToolTipWidth(nToolTipWidth, false);
    }

    UiMargin rcBkImageMargin = GetBkImageMargin();
    SetBkImageMargin(rcBkImageMargin, false);//这个值不需要做DPI缩放，直接转存为当前DPI的值

    UiFixedInt fixedWidth = GetFixedWidth();
    if (fixedWidth.IsInt32()) {
        int32_t nFixedWidth = Dpi().GetScaleInt(fixedWidth.GetInt32(), nOldDpiScale);
        SetFixedWidth(UiFixedInt(nFixedWidth), true, false);
    }
    UiFixedInt fixedHeight = GetFixedHeight();
    if (fixedHeight.IsInt32()) {
        int32_t nFixedHeight = Dpi().GetScaleInt(fixedHeight.GetInt32(), nOldDpiScale);
        SetFixedHeight(UiFixedInt(nFixedHeight), true, false);
    }

    //对于auto类型的控件，需要重新评估大小
    SetReEstimateSize(true);
}

void Control::SetClass(const DString& strClass)
{
    if (strClass.empty()) {
        return;
    }
    std::list<DString> splitList = StringUtil::Split(strClass, _T(" "));
    for (auto it = splitList.begin(); it != splitList.end(); it++) {
        DString pDefaultAttributes = GlobalManager::Instance().GetClassAttributes((*it));
        Window* pWindow = GetWindow();
        if (pDefaultAttributes.empty() && (pWindow != nullptr)) {
            pDefaultAttributes = pWindow->GetClassAttributes(*it);
        }

        ASSERT(!pDefaultAttributes.empty());
        if (!pDefaultAttributes.empty()) {
            ApplyAttributeList(pDefaultAttributes);
        }
    }
}

void Control::ApplyAttributeList(const DString& strList)
{
    //属性列表，先解析，然后再应用
    if (strList.empty()) {
        return;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    if (strList.find(_T('\"')) != DString::npos) {
        AttributeUtil::ParseAttributeList(strList, _T('\"'), attributeList);
    }    
    else if (strList.find(_T('\'')) != DString::npos) {
        AttributeUtil::ParseAttributeList(strList, _T('\''), attributeList);
    }
    for (const auto& attribute : attributeList) {
        SetAttribute(attribute.first, attribute.second);
    }
}

bool Control::OnApplyAttributeList(const DString& strReceiver, const DString& strList, const EventArgs& /*eventArgs*/)
{
    bool isFindSubControl = false;
    DString receiverName = strReceiver;
    if (receiverName.size() >= 2) {
        if (receiverName.substr(0, 2) == _T(".\\") || receiverName.substr(0, 2) == _T("./")) {
            receiverName = receiverName.substr(2);
            isFindSubControl = true;
        }
    }
    Control* pReceiverControl = nullptr;
    if (isFindSubControl) {
        Box* pBox = dynamic_cast<Box*>(this);
        if (pBox != nullptr) {
            pReceiverControl = pBox->FindSubControl(receiverName);
        }
    }
    else if (!receiverName.empty()) {
        pReceiverControl = GetWindow()->FindControl(receiverName);
    }
    else {
        pReceiverControl = this;
    }

    DString strValueList = strList;
    //这个是手工写入的属性，以花括号{}代替双引号，编写的时候就不需要转义字符了；
    StringUtil::ReplaceAll(_T("{"), _T("\""), strValueList);
    StringUtil::ReplaceAll(_T("}"), _T("\""), strValueList);

    if (pReceiverControl != nullptr) {        
        pReceiverControl->ApplyAttributeList(strValueList);
        return true;
    }
    else {
        if (strReceiver == _T("#window#")) {
            //一个特殊的Receiver，代表关联窗口
            if (GetWindow() != nullptr) {
                GetWindow()->ApplyAttributeList(strValueList);
                return true;
            }
        }
        ASSERT(!"Control::OnApplyAttributeList error：Receiver Control not found!");
        return false;
    }
}

AnimationManager& Control::GetAnimationManager()
{
    if (m_pAnimationData == nullptr) {
        m_pAnimationData = std::make_unique<TAnimationData>();
    }
    if (m_pAnimationData->m_animationManager == nullptr) {
        m_pAnimationData->m_animationManager = std::make_unique<AnimationManager>();
        m_pAnimationData->m_animationManager->Init(this);
    }
    return *m_pAnimationData->m_animationManager;
}

DString Control::GetBkColor() const
{
    return (m_pColorData != nullptr) ? m_pColorData->m_strBkColor.c_str() : DString();
}

void Control::SetBkColor(const DString& strColor)
{
    ASSERT(strColor.empty() || HasUiColor(strColor));
    if (m_pColorData == nullptr) {
        m_pColorData = std::make_unique<TColorData>();
    }
    if (m_pColorData->m_strBkColor == strColor) {
        return;
    }
    m_pColorData->m_strBkColor = strColor;
    Invalidate();
}

void Control::SetBkColor(const UiColor& color)
{
    if (color.IsEmpty()) {
        SetBkColor(_T(""));
    }
    else {
        SetBkColor(GetColorString(color));
    }
}

void Control::SetBkColor2(const DString& strColor)
{
    ASSERT(strColor.empty() || HasUiColor(strColor));
    if (m_pColorData == nullptr) {
        m_pColorData = std::make_unique<TColorData>();
    }
    if (m_pColorData->m_strBkColor2 == strColor) {
        return;
    }
    m_pColorData->m_strBkColor2 = strColor;
    Invalidate();
}

void Control::SetBkColor2(const UiColor& color)
{
    if (color.IsEmpty()) {
        SetBkColor2(_T(""));
    }
    else {
        SetBkColor2(GetColorString(color));
    }
}

DString Control::GetBkColor2() const
{
    return (m_pColorData != nullptr) ? m_pColorData->m_strBkColor2.c_str() : DString();
}

void Control::SetBkColor2Direction(const DString& direction)
{
    int8_t nDirection = GetColor2Direction(direction);
    if (m_pColorData == nullptr) {
        m_pColorData = std::make_unique<TColorData>();
    }
    if (m_pColorData->m_nBkColor2Direction != nDirection) {
        m_pColorData->m_nBkColor2Direction = nDirection;
        Invalidate();
    }
}

DString Control::GetBkColor2Direction() const
{
    DString strBkColor2Direction = _T("1");
    if (m_pColorData != nullptr) {
        if (m_pColorData->m_nBkColor2Direction == 2) {
            strBkColor2Direction = _T("2");
        }
        else if (m_pColorData->m_nBkColor2Direction == 3) {
            strBkColor2Direction = _T("3");
        }
        else if (m_pColorData->m_nBkColor2Direction == 4) {
            strBkColor2Direction = _T("4");
        }
    }
    return strBkColor2Direction;
}

int8_t Control::GetColor2Direction(const UiString& bkColor2Direction) const
{
    int8_t nColor2Direction = 1;
    //渐变背景色
    if (bkColor2Direction == _T("2")) {
        nColor2Direction = 2;
    }
    else if (bkColor2Direction == _T("3")) {
        nColor2Direction = 3;
    }
    else if (bkColor2Direction == _T("4")) {
        nColor2Direction = 4;
    }
    return nColor2Direction;
}

DString Control::GetForeColor() const
{
    return (m_pColorData != nullptr) ? m_pColorData->m_strForeColor.c_str() : DString();
}

void Control::SetForeColor(const DString& strColor)
{
    ASSERT(strColor.empty() || HasUiColor(strColor));
    if (m_pColorData == nullptr) {
        m_pColorData = std::make_unique<TColorData>();
    }
    if (m_pColorData->m_strForeColor == strColor) {
        return;
    }
    m_pColorData->m_strForeColor = strColor;
    Invalidate();
}

void Control::SetForeColor(const UiColor& color)
{
    if (color.IsEmpty()) {
        SetForeColor(_T(""));
    }
    else {
        SetForeColor(GetColorString(color));
    }
}

DString Control::GetStateColor(ControlStateType stateType) const
{
    if (m_pColorMap != nullptr) {
        return m_pColorMap->GetStateColor(stateType);
    }
    return DString();
}

void Control::SetStateColor(ControlStateType stateType, const DString& strColor)
{
    ASSERT(strColor.empty() || HasUiColor(strColor));
    if (m_pColorMap != nullptr) {
        if (m_pColorMap->GetStateColor(stateType) == strColor) {
            return;
        }
    }
    if (m_pColorMap == nullptr) {
        m_pColorMap = std::make_unique<StateColorMap>();
        m_pColorMap->SetControl(this);
    }
    m_pColorMap->SetStateColor(stateType, strColor);
    if (stateType == kControlStateHot) {
        GetAnimationManager().SetFadeHot(true);
    }
    Invalidate();
}

DString Control::GetBkImage() const
{
    if (m_pBkImage != nullptr) {
        return m_pBkImage->GetImageString();
    }
    return DString();
}

std::string Control::GetUTF8BkImage() const
{
    std::string strOut = StringConvert::TToUTF8(GetBkImage());
    return strOut;
}

void Control::SetBkImage(const DString& strImage)
{
    if (!strImage.empty()) {
        if (m_pBkImage == nullptr) {
            m_pBkImage = std::make_unique<Image>();
            m_pBkImage->SetControl(this);
        }
    }
    bool bChanged = false;
    if (m_pBkImage != nullptr) {
        if (m_pBkImage->GetImageString() != strImage) {
            bChanged = true;
            m_pBkImage->SetImageString(strImage, Dpi());
        }
    }
    if (bChanged) {
        RelayoutOrRedraw();
    }
}

void Control::SetUTF8BkImage(const std::string& strImage)
{
    DString strOut = StringConvert::UTF8ToT(strImage);
    SetBkImage(strOut);
}

bool Control::SetLoadingAttribute(const DString& loadingAttribute)
{
    bool bRet = false;
    if (!loadingAttribute.empty()) {
        if (m_pOtherData == nullptr) {
            m_pOtherData = std::make_unique<TOtherData>();
        }
        if (m_pOtherData->m_pLoading == nullptr) {
            m_pOtherData->m_pLoading = std::make_unique<ControlLoading>(this);
        }
        else {
            if (m_pOtherData->m_pLoading->IsLoading()) {
                m_pOtherData->m_pLoading->StopLoading();
            }
        }
        bRet = m_pOtherData->m_pLoading->SetLoadingAttribute(loadingAttribute);
        if (!bRet) {
            m_pOtherData->m_pLoading.reset();
        }
    }
    else {
        bRet = true;
        if (m_pOtherData != nullptr) {
            m_pOtherData->m_pLoading.reset();
        }       
    }
    return bRet;
}

bool Control::StartLoading(int32_t nIntervalMs, int32_t nMaxCount)
{
    bool bRet = false;
    ASSERT((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr));
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        bRet = m_pOtherData->m_pLoading->StartLoading(nIntervalMs, nMaxCount);
    }
    if (bRet) {
        SetEnabled(false);
        ASSERT(GetLoadingUiRootBox() != nullptr);
    }
    return bRet;
}

void Control::StopLoading()
{
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        m_pOtherData->m_pLoading->StopLoading();
    }
    SetEnabled(true);
}

bool Control::IsLoading() const
{
    bool bRet = false;
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        bRet = m_pOtherData->m_pLoading->IsLoading();
    }
    return bRet;
}

Box* Control::GetLoadingUiRootBox() const
{
    Box* pLoadingUiRootBox = nullptr;
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        pLoadingUiRootBox = m_pOtherData->m_pLoading->GetLoadingUiRootBox();
    }
    return pLoadingUiRootBox;
}

bool Control::HasStateImages(void) const
{
    if (m_pImageMap != nullptr) {
        return m_pImageMap->HasStateImages();
    }
    return false;
}

bool Control::HasStateImage(StateImageType stateImageType) const
{
    if (m_pImageMap != nullptr) {
        return m_pImageMap->HasStateImage(stateImageType);
    }
    return false;
}

DString Control::GetStateImage(StateImageType imageType, ControlStateType stateType) const
{
    if (m_pImageMap != nullptr) {
        return m_pImageMap->GetImageString(imageType, stateType);
    }
    return DString();
}

void Control::SetStateImage(StateImageType imageType, ControlStateType stateType, const DString& strImage)
{
    if (m_pImageMap == nullptr) {
        m_pImageMap = std::make_unique<StateImageMap>();
        m_pImageMap->SetControl(this);
    }
    m_pImageMap->SetImageString(imageType, stateType, strImage, Dpi());
}

bool Control::PaintStateImage(IRender* pRender, StateImageType stateImageType, 
                              ControlStateType stateType, 
                              const DString& sImageModify,
                              UiRect* pDestRect)
{
    if (m_pImageMap != nullptr) {
        return m_pImageMap->PaintStateImage(pRender, stateImageType, stateType, sImageModify, pDestRect);
    }
    return false;
}

UiSize Control::GetStateImageSize(StateImageType imageType, ControlStateType stateType)
{
    Image* pImage = nullptr;
    if (m_pImageMap != nullptr) {
        pImage = m_pImageMap->GetStateImage(imageType, stateType);
    }
    UiSize imageSize;
    if (pImage != nullptr) {
        LoadImageInfo(*pImage);
        std::shared_ptr<ImageInfo> imageInfo = pImage->GetImageInfo();
        if (imageInfo != nullptr) {
            imageSize.cx = imageInfo->GetWidth();
            imageSize.cy = imageInfo->GetHeight();
        }
    }
    return imageSize;
}

void Control::ClearStateImages()
{
    m_pImageMap.reset();
    RelayoutOrRedraw();
}

DString Control::GetStateImage(ControlStateType stateType) const
{
    return GetStateImage(kStateImageBk, stateType);
}

void Control::SetStateImage(ControlStateType stateType, const DString& strImage)
{
    if (stateType == kControlStateHot) {
        GetAnimationManager().SetFadeHot(true);
    }
    SetStateImage(kStateImageBk, stateType, strImage);
    RelayoutOrRedraw();
}

DString Control::GetForeStateImage(ControlStateType stateType) const
{
    return GetStateImage(kStateImageFore, stateType);
}

void Control::SetForeStateImage(ControlStateType stateType, const DString& strImage)
{
    if (stateType == kControlStateHot) {
        GetAnimationManager().SetFadeHot(true);
    }
    SetStateImage(kStateImageFore, stateType, strImage);
    Invalidate();
}

bool Control::AdjustStateImagesMarginLeft(int32_t leftOffset, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleInt(leftOffset);
    }
    if (leftOffset == 0) {
        return false;
    }
    std::vector<Image*> allImages;
    if (m_pImageMap != nullptr) {
        m_pImageMap->GetAllImages(allImages);
    }
    bool bSetOk = false;
    UiMargin rcMargin;
    for (Image* pImage : allImages) {
        if (pImage == nullptr) {
            continue;
        }
        rcMargin = pImage->GetImageMargin(Dpi());
        rcMargin.left += leftOffset;
        if (rcMargin.left < 0) {
            rcMargin.left = 0;
        }
        if (!pImage->GetImageMargin(Dpi()).Equals(rcMargin)) {
            pImage->SetImageMargin(rcMargin, false, Dpi());
            bSetOk = true;
        }
    }
    if (bSetOk) {
        Invalidate();
    }
    return bSetOk;
}

UiMargin Control::GetBkImageMargin() const
{
    UiMargin rcMargin;
    if (m_pBkImage != nullptr) {
        rcMargin = m_pBkImage->GetImageMargin(Dpi());
    }
    return rcMargin;
}

bool Control::SetBkImageMargin(UiMargin rcMargin, bool bNeedDpiScale)
{
    bool bSetOk = false;
    if (m_pBkImage != nullptr) {
        if (bNeedDpiScale) {
            Dpi().ScaleMargin(rcMargin);
        }
        if (!m_pBkImage->GetImageMargin(Dpi()).Equals(rcMargin)) {
            m_pBkImage->SetImageMargin(rcMargin, false, Dpi());
            bSetOk = true;
            Invalidate();
        }        
    }
    return bSetOk;
}

bool Control::IsBkImagePaintEnabled() const
{
    if (m_pBkImage != nullptr) {
        return m_pBkImage->IsImagePaintEnabled();
    }
    return true;
}

void Control::SetBkImagePaintEnabled(bool bEnable)
{
    if (m_pBkImage != nullptr) {
        bool isChange = m_pBkImage->IsImagePaintEnabled() != bEnable;        
        if (isChange) {
            m_pBkImage->SetImagePaintEnabled(bEnable);
            Invalidate();
        }
    }
}

DString Control::GetBkImagePath() const
{
    if (m_pBkImage != nullptr) {
        return m_pBkImage->GetImagePath();
    }
    return DString();
}

UiSize Control::GetBkImageSize() const
{
    UiSize imageSize;
    if (m_pBkImage != nullptr) {
        LoadImageInfo(*m_pBkImage);
        std::shared_ptr<ImageInfo> imageInfo = m_pBkImage->GetImageInfo();
        if (imageInfo != nullptr) {
            imageSize.cx = imageInfo->GetWidth();
            imageSize.cy = imageInfo->GetHeight();
        }
    }
    return imageSize;
}

ControlStateType Control::GetState() const
{
    return static_cast<ControlStateType>(m_controlState);
}

void Control::SetState(ControlStateType controlState)
{
    if (controlState == kControlStateNormal) {
        m_nHotAlpha = 0;
    }
    else if (controlState == kControlStateHot) {
        m_nHotAlpha = 255;
    }
    PrivateSetState(controlState);
    Invalidate();
}

void Control::PrivateSetState(ControlStateType controlState)
{
    if (GetState() != controlState) {
        ControlStateType oldState = GetState();
        m_controlState = TruncateToInt8(controlState);
        SendEvent(kEventStateChange, controlState, oldState);
        Invalidate();
    }
}

bool Control::IsHotState() const
{
    return (GetState() == kControlStateHot) ? true : false;
}

DString Control::GetBorderColor(ControlStateType stateType) const
{
    DString borderColor;
    if ((m_pBorderData != nullptr) && (m_pBorderData->m_pBorderColorMap != nullptr)) {
        borderColor = m_pBorderData->m_pBorderColorMap->GetStateColor(stateType);
    }
    return borderColor;
}

void Control::SetBorderColor(const DString& strBorderColor)
{
    SetBorderColor(kControlStateNormal, strBorderColor);
    SetBorderColor(kControlStateHot, strBorderColor);
    SetBorderColor(kControlStatePushed, strBorderColor);
    SetBorderColor(kControlStateDisabled, strBorderColor);
}

void Control::SetBorderColor(ControlStateType stateType, const DString& strBorderColor)
{
    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_pBorderColorMap == nullptr) {
        m_pBorderData->m_pBorderColorMap = std::make_unique<StateColorMap>();
        m_pBorderData->m_pBorderColorMap->SetControl(this);
    }
    if (GetBorderColor(stateType) != strBorderColor) {
        m_pBorderData->m_pBorderColorMap->SetStateColor(stateType, strBorderColor);
        Invalidate();
    }
}

void Control::SetFocusBorderColor(const DString& strBorderColor)
{
    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_focusBorderColor != strBorderColor) {
        m_pBorderData->m_focusBorderColor = strBorderColor;
        Invalidate();
    }
}

DString Control::GetFocusBorderColor() const
{
    if (m_pBorderData != nullptr) {
        return m_pBorderData->m_focusBorderColor.c_str();
    }
    return DString();
}

void Control::SetBorderSize(UiRectF rc, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        rc.left = Dpi().GetScaleFloat(rc.left);
        rc.top = Dpi().GetScaleFloat(rc.top);
        rc.right = Dpi().GetScaleFloat(rc.right);
        rc.bottom = Dpi().GetScaleFloat(rc.bottom);
    }
    rc.left = std::max(rc.left, 0.0f);
    rc.top = std::max(rc.top, 0.0f);
    rc.right = std::max(rc.right, 0.0f);
    rc.bottom = std::max(rc.bottom, 0.0f);

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_rcBorderSize != rc) {
        m_pBorderData->m_rcBorderSize = rc;
        Invalidate();
    }    
}

UiRectF Control::GetBorderSize() const
{
    return (m_pBorderData != nullptr) ? m_pBorderData->m_rcBorderSize : UiRectF();
}

float Control::GetLeftBorderSize() const
{
    return (m_pBorderData != nullptr) ? m_pBorderData->m_rcBorderSize.left : 0.0f;
}

void Control::SetLeftBorderSize(float fSize, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fSize = Dpi().GetScaleFloat(fSize);
    }

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_rcBorderSize.left != fSize) {
        m_pBorderData->m_rcBorderSize.left = fSize;
        Invalidate();
    }    
}

float Control::GetTopBorderSize() const
{
    return (m_pBorderData != nullptr) ? m_pBorderData->m_rcBorderSize.top : 0.0f;
}

void Control::SetTopBorderSize(float fSize, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fSize = Dpi().GetScaleFloat(fSize);
    }

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_rcBorderSize.top != fSize) {
        m_pBorderData->m_rcBorderSize.top = fSize;
        Invalidate();
    }
}

float Control::GetRightBorderSize() const
{
    return (m_pBorderData != nullptr) ? m_pBorderData->m_rcBorderSize.right : 0.0f;
}

void Control::SetRightBorderSize(float fSize, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fSize = Dpi().GetScaleFloat(fSize);
    }

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_rcBorderSize.right != fSize) {
        m_pBorderData->m_rcBorderSize.right = fSize;
        Invalidate();
    }    
}

float Control::GetBottomBorderSize() const
{
    return  (m_pBorderData != nullptr) ? m_pBorderData->m_rcBorderSize.bottom : 0.0f;
}

void Control::SetBottomBorderSize(float fSize, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fSize = Dpi().GetScaleFloat(fSize);
    }

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_rcBorderSize.bottom != fSize) {
        m_pBorderData->m_rcBorderSize.bottom = fSize;
        Invalidate();
    }    
}

void Control::SetBorderDashStyle(int8_t borderDashStyle)
{
    IPen::DashStyle dashStyle = IPen::kDashStyleSolid;
    if (borderDashStyle == IPen::kDashStyleDash) {
        dashStyle = IPen::kDashStyleDash;
    }
    else if (borderDashStyle == IPen::kDashStyleDot) {
        dashStyle = IPen::kDashStyleDot;
    }
    else if (borderDashStyle == IPen::kDashStyleDashDot) {
        dashStyle = IPen::kDashStyleDashDot;
    }
    else if (borderDashStyle == IPen::kDashStyleDashDotDot) {
        dashStyle = IPen::kDashStyleDashDotDot;
    }

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    if (m_pBorderData->m_borderDashStyle != dashStyle) {
        m_pBorderData->m_borderDashStyle = (int8_t)dashStyle;
        Invalidate();
    }
}

int8_t Control::GetBorderDashStyle() const
{
    if (m_pBorderData != nullptr) {
        return m_pBorderData->m_borderDashStyle;
    }
    return 0;
}

void Control::SetBordersOnTop(bool bBordersOnTop)
{
    if (m_bBordersOnTop != bBordersOnTop) {
        m_bBordersOnTop = bBordersOnTop;
        Invalidate();
    }
}

bool Control::IsBordersOnTop() const
{
    return m_bBordersOnTop;
}

bool Control::GetBorderRound(float& fRoundWidth, float& fRoundHeight) const
{
    fRoundWidth = 0.0f;
    fRoundHeight = 0.0f;
    if (HasBorderRound() && (m_pBorderData != nullptr)) {
        fRoundWidth = Dpi().GetScaleFloat(m_pBorderData->m_borderRound.cx);
        fRoundHeight = Dpi().GetScaleFloat(m_pBorderData->m_borderRound.cy);
        return true;
    }
    return false;
}

bool Control::HasBorderRound() const
{
    return (m_pBorderData != nullptr) && (m_pBorderData->m_borderRound.cx > 0) && (m_pBorderData->m_borderRound.cy > 0);
}

void Control::SetBorderRound(UiSize borderRound)
{
    int32_t cx = borderRound.cx;
    int32_t cy = borderRound.cy;
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

    if (m_pBorderData == nullptr) {
        m_pBorderData = std::make_unique<TBorderData>();
    }
    UiSize16& borderRoundData = m_pBorderData->m_borderRound;
    if ((borderRoundData.cx != borderRound.cx) || (borderRoundData.cy != borderRound.cy)) {
        borderRoundData.cx = ui::TruncateToInt16(borderRound.cx);
        borderRoundData.cy = ui::TruncateToInt16(borderRound.cy);
        Invalidate();
    }
}

void Control::SetBoxShadow(const DString& strShadow)
{
    if (strShadow.empty()) {
        return;
    }
    if (m_pOtherData == nullptr) {
        m_pOtherData = std::make_unique<TOtherData>();
    }
    if (m_pOtherData->m_pBoxShadow == nullptr) {
        m_pOtherData->m_pBoxShadow = std::make_unique<BoxShadow>(this);
    }
    m_pOtherData->m_pBoxShadow->SetBoxShadowString(strShadow);
}

CursorType Control::GetCursorType() const
{
    return m_cursorType;
}

void Control::SetCursorType(CursorType cursorType)
{
    m_cursorType = cursorType;
}

DString Control::GetToolTipText() const
{
    DString strText;
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pTooltip != nullptr)) {
        strText = m_pOtherData->m_pTooltip->m_sToolTipText.c_str();
        if (strText.empty() && !m_pOtherData->m_pTooltip->m_sToolTipTextId.empty()) {
            strText = GlobalManager::Instance().Lang().GetStringViaID(m_pOtherData->m_pTooltip->m_sToolTipTextId.c_str());
        }
    }
    return strText;
}

std::string Control::GetUTF8ToolTipText() const
{
    std::string strOut = StringConvert::TToUTF8(GetToolTipText());
    return strOut;
}

void Control::SetToolTipText(const DString& strText)
{
    if (m_pOtherData == nullptr) {
        m_pOtherData = std::make_unique<TOtherData>();
    }
    if (m_pOtherData->m_pTooltip == nullptr) {
        m_pOtherData->m_pTooltip = std::make_unique<TTooltipData>();
    }
    if (strText != m_pOtherData->m_pTooltip->m_sToolTipText) {
        DString strTemp(strText);
        StringUtil::ReplaceAll(_T("<n>"), _T("\r\n"), strTemp);
        m_pOtherData->m_pTooltip->m_sToolTipText = strTemp;
        Invalidate();

        if (GetWindow() != nullptr) {
            Control* pHover = GetWindow()->GetHoverControl();
            if (pHover == this) {
                //更新ToolTip的显示
                GetWindow()->UpdateToolTip();
            }
        }
    }    
}

void Control::SetUTF8ToolTipText(const std::string& strText)
{
    DString strOut = StringConvert::UTF8ToT(strText);
    SetToolTipText(strOut);
}

void Control::SetToolTipTextId(const DString& strTextId)
{
    if (m_pOtherData == nullptr) {
        m_pOtherData = std::make_unique<TOtherData>();
    }
    if (m_pOtherData->m_pTooltip == nullptr) {
        m_pOtherData->m_pTooltip = std::make_unique<TTooltipData>();
    }

    if (m_pOtherData->m_pTooltip->m_sToolTipTextId != strTextId) {
        m_pOtherData->m_pTooltip->m_sToolTipTextId = strTextId;
        Invalidate();

        if (GetWindow() != nullptr) {
            Control* pHover = GetWindow()->GetHoverControl();
            if (pHover == this) {
                //更新ToolTip的显示
                GetWindow()->UpdateToolTip();
            }
        }
    }
}

void Control::SetUTF8ToolTipTextId(const std::string& strTextId)
{
    DString strOut = StringConvert::UTF8ToT(strTextId);
    SetToolTipTextId(strOut);
}

void Control::SetToolTipWidth(int32_t nWidth, bool bNeedDpiScale)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nWidth);
    }
    if (m_pOtherData == nullptr) {
        m_pOtherData = std::make_unique<TOtherData>();
    }
    if (m_pOtherData->m_pTooltip == nullptr) {
        m_pOtherData->m_pTooltip = std::make_unique<TTooltipData>();
    }
    m_pOtherData->m_pTooltip->m_nTooltipWidth = nWidth;
}

int32_t Control::GetToolTipWidth(void) const
{
    int32_t nTooltipWidth = 0;
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pTooltip != nullptr)) {
        nTooltipWidth = m_pOtherData->m_pTooltip->m_nTooltipWidth;
    }
    return nTooltipWidth;
}

void Control::SetContextMenuUsed(bool bMenuUsed)
{
    m_bContextMenuUsed = bMenuUsed;
}

DString Control::GetDataID() const
{
    return m_sUserDataID.c_str();
}

std::string Control::GetUTF8DataID() const
{
    std::string strOut = StringConvert::TToUTF8(GetDataID());
    return strOut;
}

void Control::SetDataID(const DString& strText)
{
    m_sUserDataID = strText;
}

void Control::SetUTF8DataID(const std::string& strText)
{
    m_sUserDataID = StringConvert::UTF8ToT(strText);
}

void Control::SetUserDataID(size_t dataID)
{
    m_uUserDataID = dataID;
}

size_t Control::GetUserDataID() const
{
    return m_uUserDataID;
}

void Control::SetFadeVisible(bool bVisible)
{
    if (bVisible) {
        GetAnimationManager().Appear();
    }
    else {
        GetAnimationManager().Disappear();
    }
}

void Control::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    const bool bVisible = IsVisible();
    if (!bVisible) {
        // 确保本控件不再是焦点控件
        EnsureNoFocus();

        // 暂停本控件的动画播放
        PauseImageAnimation();
    }
    if (bChanged) {
        // 让父容器重现布局
        ArrangeAncestor();

        // 最后，触发可见状态变化事件，通知应用层
        WPARAM wParam = bVisible ? 1 : 0;
        SendEvent(kEventVisibleChange, wParam);
    }
}

void Control::OnSetEnabled(bool bChanged)
{
    BaseClass::OnSetEnabled(bChanged);
    if (IsEnabled()) {
        PrivateSetState(kControlStateNormal);
        m_nHotAlpha = 0;
    }
    else {
        PrivateSetState(kControlStateDisabled);
    }

    if (!IsEnabled()) {
        PauseImageAnimation();
    }
    if (bChanged) {
        Invalidate();
    }    
}

bool Control::IsFocused() const
{
    Window* pWindow = GetWindow();
    return ((pWindow != nullptr) && (pWindow->GetFocusControl() == this) );
}

void Control::SetFocus()
{
    if (m_bNoFocus) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pWindow->SetFocusControl(this);
    }
}

uint32_t Control::GetControlFlags() const
{
    return IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void Control::SetNoFocus()
{
    m_bNoFocus = true;
    EnsureNoFocus();
}

void Control::SetShowFocusRect(bool bShowFocusRect)
{
    m_bShowFocusRect = bShowFocusRect;
}

bool Control::IsShowFocusRect() const
{
    return m_bShowFocusRect;
}

void Control::SetFocusRectColor(const DString& focusRectColor)
{
    if (m_pColorData == nullptr) {
        m_pColorData = std::make_unique<TColorData>();
    }
    if (m_pColorData->m_focusRectColor == focusRectColor) {
        return;
    }
    m_pColorData->m_focusRectColor = focusRectColor;
    Invalidate();
}

DString Control::GetFocusRectColor() const
{
    if (m_pColorData != nullptr) {
        return m_pColorData->m_focusRectColor.c_str();
    }
    return DString();
}

void Control::Activate(const EventArgs* /*pMsg*/)
{
}

bool Control::IsActivatable() const
{
    if (!IsVisible() || !IsEnabled()) {
        return false;
    }
    return true;
}

Control* Control::FindControl(FINDCONTROLPROC Proc, void* pProcData,
                              uint32_t uFlags, const UiPoint& ptMouse,
                              const UiPoint& scrollPos)
{
    if (Proc == nullptr) {
        return nullptr;
    }
    if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
        return nullptr;
    }
    if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
        return nullptr;
    }
    UiPoint pt(ptMouse);
    pt.Offset(scrollPos);
#ifdef _DEBUG
    if (((uFlags & UIFIND_HITTEST) != 0) && ((uFlags & UIFIND_DRAG_DROP) == 0) && (pProcData != nullptr)) {
        if (!IsFloat()) {
            UiPoint ptOrg(*(UiPoint*)pProcData);
            ptOrg.Offset(GetScrollOffsetInScrollBox());
            ASSERT(ptOrg == pt);
        }
    }
#endif // _DEBUG
    if ((uFlags & UIFIND_HITTEST) != 0 && 
        (!IsMouseEnabled() || !GetRect().ContainsPt(pt))) {
        return nullptr;
    }
    return Proc(this, pProcData);
}

Control* Control::FindControl(const DString& name)
{
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        return pWindow->FindSubControlByName(this, name);
    }
    return nullptr;
}

UiRect Control::GetPos() const
{
    return BaseClass::GetPos();
}

void Control::SetPos(UiRect rc)
{
    //有很多类似的代码：SetPos(GetPos()), 代表设置位置，并重新绘制
    rc.Validate();
    SetArranged(false);
    bool isPosChanged = !GetRect().Equals(rc);

    UiRect rcOldRect = GetRect();
    if (rcOldRect.IsEmpty()) {
        rcOldRect = rc;//避免为空
    }
    // 如果存在box-shadow，需要包含它扩展绘制的区域
    rcOldRect = GetBoxShadowExpandedRect(rcOldRect);

    SetRect(rc);
    if (GetWindow() == nullptr) {
        return;
    }
    UiRect rcNewRect = GetRect();
    rcNewRect = GetBoxShadowExpandedRect(rcNewRect);

    UiRect rcInvalidateRect = rcOldRect;
    rcInvalidateRect.Union(rcNewRect);// 旧矩形范围和新矩形范围的合集，均需要标记为脏区域

    bool needInvalidate = true;
    UiRect rcTemp;
    UiRect rcParent;
    UiPoint offset = GetScrollOffsetInScrollBox();
    rcInvalidateRect.Offset(-offset.x, -offset.y);// 转换为窗口内的客户区坐标
    Control* pParent = GetParent();
    while (pParent != nullptr) {
        rcTemp = rcInvalidateRect;
        rcParent = pParent->GetPos();
        rcParent = pParent->GetBoxShadowExpandedRect(rcParent);
        UiPoint offsetParent = pParent->GetScrollOffsetInScrollBox();
        rcParent.Offset(-offsetParent.x, -offsetParent.y);// 转换为窗口内的客户区坐标
        if (!UiRect::Intersect(rcInvalidateRect, rcTemp, rcParent)) {
            needInvalidate = false;
            break;
        }
        pParent = pParent->GetParent();
    }
    if (needInvalidate && (GetWindow() != nullptr)) {
        GetWindow()->Invalidate(rcInvalidateRect);
    }

    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        m_pOtherData->m_pLoading->UpdateLoadingPos();
    }

    if (isPosChanged) {
        SendEvent(kEventResize);
    }    
}

UiEstSize Control::EstimateSize(UiSize szAvailable)
{
    UiFixedSize fixedSize = GetFixedSize();
    if (!fixedSize.cx.IsAuto() && !fixedSize.cy.IsAuto()) {
        //如果宽高都不是auto属性，则直接返回
        return MakeEstSize(fixedSize);
    }
    szAvailable.Validate();
    if (!IsReEstimateSize(szAvailable)) {
        //使用缓存中的估算结果
        return GetEstimateSize();
    }

    //设置估算图片宽高的类型，用于优化性能（有些属性设置后，图片可以延迟加载）
    EstimateImageType estImageType = EstimateImageType::kBoth;
    if (!fixedSize.cx.IsAuto() || !fixedSize.cy.IsAuto()) {
        if (fixedSize.cx.IsAuto()) {
            estImageType = EstimateImageType::kWidthOnly;
        }
        else {
            estImageType = EstimateImageType::kHeightOnly;
        }
    }
    UiSize szControlSize = EstimateControlSize(szAvailable, estImageType);

    //选取图片和文本区域高度和宽度的最大值
    if (fixedSize.cx.IsAuto()) {
        fixedSize.cx.SetInt32(szControlSize.cx);
    }
    if (fixedSize.cy.IsAuto()) {
        fixedSize.cy.SetInt32(szControlSize.cy);
    }
    //保持结果到缓存，避免每次都重新估算
    UiEstSize estSize = MakeEstSize(fixedSize);
    SetEstimateSize(estSize, szAvailable);
    SetReEstimateSize(false);
    return estSize;
}

UiSize Control::EstimateControlSize(UiSize szAvailable, EstimateImageType estImageType)
{
    //估算图片区域大小
    UiSize imageSize = EstimateImage(szAvailable, estImageType);

    //估算文本区域大小, 函数计算时，已经包含了内边距
    UiSize textSize = EstimateText(szAvailable);

    UiSize szControlSize;
    szControlSize.cx = std::max(imageSize.cx, textSize.cx);
    szControlSize.cy = std::max(imageSize.cy, textSize.cy);
    return szControlSize;
}

UiSize Control::EstimateText(UiSize /*szAvailable*/)
{
    return UiSize(0, 0);
}

UiSize Control::EstimateImage(UiSize szAvailable, EstimateImageType estImageType)
{
    UiSize imageSize;
    Image* image = GetEstimateImage();
    uint32_t nImageInfoWidth = 0;
    uint32_t nImageInfoHeight = 0;
    if (image != nullptr) {
        bool bNeedLoadImage = true;//是否需要加载图片
        ImageLoadParam loadParam = image->GetImageLoadParam();        
        loadParam.GetImageFixedSize(nImageInfoWidth, nImageInfoHeight);
        if (estImageType == EstimateImageType::kWidthOnly) {
            if (nImageInfoWidth > 0) {
                bNeedLoadImage = false;
            }
        }
        else if (estImageType == EstimateImageType::kHeightOnly) {
            if (nImageInfoHeight > 0) {
                bNeedLoadImage = false;
            }
        }
        else {
            if ((nImageInfoWidth > 0) && (nImageInfoHeight > 0)) {
                bNeedLoadImage = false;
            }
        }
        //加载图片：需要获取图片的宽和高
        if (bNeedLoadImage) {
            LoadImageInfo(*image);
            std::shared_ptr<ImageInfo> imageInfo = image->GetImageInfo();
            if (imageInfo != nullptr) {
                nImageInfoWidth = imageInfo->GetWidth();
                nImageInfoHeight = imageInfo->GetHeight();
            }
        }
        else {
            if (nImageInfoWidth == 0) {
                nImageInfoWidth = nImageInfoHeight;//冗余设置，实际上不需要宽度
            }
            if (nImageInfoHeight == 0) {
                nImageInfoHeight = nImageInfoWidth;//冗余设置，实际上不需要高度
            }
        }
    }
    //控件自身的内边距
    const UiPadding rcControlPadding = GetControlPadding();

    if ((nImageInfoWidth > 0) && (nImageInfoHeight > 0) && (image != nullptr)) {
        ImageAttribute imageAttribute = image->GetImageAttribute();
        UiRect rcDest;
        bool hasDestAttr = false;
        UiRect rcImageDestRect = imageAttribute.GetImageDestRect(nImageInfoWidth, nImageInfoHeight, Dpi());
        if (ImageAttribute::HasValidImageRect(rcImageDestRect)) {
            //使用配置中指定的目标区域（已按配置做好DPI自适应）：优先作为图片大小的依据
            rcDest = rcImageDestRect;
            if (rcDest.left < 0) {
                rcDest.left = 0;
            }
            if (rcDest.top < 0) {
                rcDest.top = 0;
            }
            hasDestAttr = true;
        }
        UiRect rcSource = imageAttribute.GetImageSourceRect();
        if (imageAttribute.m_bImageDpiScaleEnabled) {
            //该图片支持DPI自适应
            Dpi().ScaleRect(rcSource);
        }        
        if (rcSource.right > (int32_t)nImageInfoWidth) {
            rcSource.right = (int32_t)nImageInfoWidth;
        }
        if (rcSource.bottom > (int32_t)nImageInfoHeight) {
            rcSource.bottom = (int32_t)nImageInfoHeight;
        }

        if (rcDest.Width() > 0) {
            //以0为基点，right为边界
            imageSize.cx = rcDest.right;
        }
        else if (rcSource.Width() > 0) {
            imageSize.cx = rcSource.Width();
        }
        else {
            imageSize.cx = nImageInfoWidth;
        }

        if (rcDest.Height() > 0) {
            //以0为基点，bottom为边界
            imageSize.cy = rcDest.bottom;
        }
        else if (rcSource.Height() > 0) {
            imageSize.cy = rcSource.Height();
        }
        else {
            imageSize.cy = nImageInfoHeight;
        }
        if (!hasDestAttr) {
            //如果没有rcDest属性，则需要增加图片的外边距（图片自身的外边距属性）
            UiMargin rcImageMargin = imageAttribute.GetImageMargin(Dpi());
            imageSize.cx += (rcImageMargin.left + rcImageMargin.right);
            imageSize.cy += (rcImageMargin.top + rcImageMargin.bottom);
        }
        if (imageAttribute.m_bAdaptiveDestRect) {
            //自动适应目标区域（等比例缩放图片）：根据图片大小，调整绘制区域
            const int32_t nImageWidth = rcSource.Width();
            const int32_t nImageHeight = rcSource.Height();
            UiRect rcControlDest = UiRect(0, 0,
                                          szAvailable.cx - rcControlPadding.left - rcControlPadding.right,
                                          szAvailable.cy - rcControlPadding.top - rcControlPadding.bottom);
            rcControlDest.Validate();
            if (rcControlDest.Width() > 0 && rcControlDest.Height() > 0) {
                rcControlDest = ImageAttribute::CalculateAdaptiveRect(nImageWidth, nImageHeight,
                                                                      rcControlDest,
                                                                      imageAttribute.m_hAlign.c_str(),
                                                                      imageAttribute.m_vAlign.c_str());
                imageSize.cx = rcControlDest.Width();
                imageSize.cy = rcControlDest.Height();
            }
        }
    }

    //图片大小，需要附加控件的内边距
    if (imageSize.cx > 0) {
        imageSize.cx += (rcControlPadding.left + rcControlPadding.right);
    }
    if (imageSize.cy > 0) {
        imageSize.cy += (rcControlPadding.top + rcControlPadding.bottom);
    }
    return imageSize;
}

Image* Control::GetEstimateImage()
{
    Image* estimateImage = nullptr;
    if ((m_pBkImage != nullptr) && !m_pBkImage->GetImagePath().empty()) {
        estimateImage = m_pBkImage.get();
    }
    else if(m_pImageMap != nullptr){
        estimateImage = m_pImageMap->GetEstimateImage(kStateImageBk);
        if (estimateImage == nullptr) {
            estimateImage = m_pImageMap->GetEstimateImage(kStateImageSelectedBk);
        }
    }
    return estimateImage;
}

bool Control::IsPointInWithScrollOffset(const UiPoint& point) const
{
    UiPoint scrollOffset = GetScrollOffsetInScrollBox();
    UiPoint newPoint = point;
    newPoint.Offset(scrollOffset);
    return GetRect().ContainsPt(newPoint);
}

void Control::SendEvent(EventType eventType, WPARAM wParam, LPARAM lParam)
{
    EventArgs msg;
    msg.SetSender(this);
    msg.eventType = eventType;
    msg.vkCode = VirtualKeyCode::kVK_None;
    msg.wParam = wParam;
    msg.lParam = lParam;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        msg.ptMouse = pWindow->GetLastMousePos();
    }
    msg.modifierKey = 0;
    msg.eventData = 0;

    //派发消息
    SendEventMsg(msg);
}

void Control::SendEvent(EventType eventType, EventArgs msg)
{
    ASSERT((msg.eventType == eventType) || (msg.eventType == EventType::kEventNone));
    msg.eventType = eventType;
    msg.SetSender(this);
    if ((msg.ptMouse.x == 0) && (msg.ptMouse.y == 0)) {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            msg.ptMouse = pWindow->GetLastMousePos();
        }
    }
    //派发消息
    SendEventMsg(msg);
}

void Control::SendEventMsg(const EventArgs& msg)
{
//#ifdef _DEBUG
//    DString eventType = EventTypeToString(msg.eventType);
//    DString type = GetType();
//    DStringW::value_type buf[256] = {};
//    swprintf_s(buf, _T("Control::SendEventMsg: type=%s, eventType=%s\r\n"), type.c_str(), eventType.c_str());
//    ::OutputDebugStringW(buf);    
//#endif

    bool bRet = true;
    if (!IsDisabledEvents(msg)) {
        bRet = FireAllEvents(msg);
    }
    if(bRet) {
        HandleEvent(msg);
    }
}

bool Control::IsDisabledEvents(const EventArgs& msg) const
{
    if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
        //当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
        if (!IsEnabled() || !IsMouseEnabled()) {
            return true;
        }
    }
    else if ((msg.eventType > kEventKeyBegin) && (msg.eventType < kEventKeyEnd)) {
        //当前控件禁止接收键盘消息时，将键盘相关消息转发给上层处理
        if (!IsEnabled() || !IsKeyboardEnabled()) {
            return true;
        }
    }
    return false;
}

void Control::HandleEvent(const EventArgs& msg)
{
    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        return;
    }
    if( msg.eventType == kEventSetCursor ) {
        if (OnSetCursor(msg)) {
            return;
        }        
    }
    else if (msg.eventType == kEventSetFocus) {
        if (OnSetFocus(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventKillFocus) {
        if (OnKillFocus(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventWindowSetFocus) {
        if (OnWindowSetFocus(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventWindowKillFocus) {
        if (OnWindowKillFocus(msg)) {
            return;
        }
    }    
    else if (msg.eventType == kEventCaptureChanged) {
        if (OnCaptureChanged(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventImeSetContext) {
        if (OnImeSetContext(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventImeStartComposition) {
        if (OnImeStartComposition(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventImeComposition) {
        if (OnImeComposition(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventImeEndComposition) {
        if (OnImeEndComposition(msg)) {
            return;
        }
    }
    else if( msg.eventType == kEventMouseEnter ) {
        if (GetWindow()) {
            if (!IsChild(this, GetWindow()->GetHoverControl())) {
                return;
            }
        }
        if (MouseEnter(msg)) {
            return;
        }
    }
    else if( msg.eventType == kEventMouseLeave ) {
        if (GetWindow()) {
            if (IsChild(this, GetWindow()->GetHoverControl())) {
                return;
            }
        }
        if (MouseLeave(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseButtonDown) {
        if (ButtonDown(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseButtonUp) {
        if (ButtonUp(msg)) {
            return;
        }        
    }
    else if (msg.eventType == kEventMouseDoubleClick) {
        if (ButtonDoubleClick(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseRButtonDown) {
        if (RButtonDown(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseRButtonUp) {
        if (RButtonUp(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseRDoubleClick) {
        if (RButtonDoubleClick(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseMove) {
        if (MouseMove(msg)) {
            return;
        }        
    }
    else if (msg.eventType == kEventMouseHover) {
        if (MouseHover(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventMouseWheel) {
        if (MouseWheel(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventContextMenu) {
        if (MouseMenu(msg)) {
            return;
        }        
    }
    else if (msg.eventType == kEventChar) {
        if (OnChar(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventKeyDown) {
        if (OnKeyDown(msg)) {
            return;
        }
    }
    else if (msg.eventType == kEventKeyUp) {
        if (OnKeyUp(msg)) {
            return;
        }
    }
    if (!weakFlag.expired() && (GetParent() != nullptr)) {
        GetParent()->SendEventMsg(msg);
    }
}

bool Control::HasHotState()
{
    bool bState = false;
    if (m_pColorMap != nullptr) {
        bState = m_pColorMap->HasHotColor();
    }
    if (!bState && (m_pImageMap != nullptr)) {
        bState = m_pImageMap->HasHotImage();
    }
    if (!bState && (m_pBorderData != nullptr) && (m_pBorderData->m_pBorderColorMap != nullptr)) {
        bState = m_pBorderData->m_pBorderColorMap->HasHotColor();
    }
    return bState;
}

bool Control::MouseEnter(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if( IsEnabled() ) {
        if (GetState() == kControlStateNormal) {
            PrivateSetState(kControlStateHot);
            if (HasHotState()) {
                GetAnimationManager().MouseEnter();
                Invalidate();
            }
            return false;
        }
        else {
            return true;
        }
    }
    return false;
}

bool Control::MouseLeave(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if( IsEnabled() ) {
        if (GetState() == kControlStateHot) {
            PrivateSetState(kControlStateNormal);
            if (HasHotState()) {
                GetAnimationManager().MouseLeave();
                Invalidate();
            }
            return false;
        }
        else {
            return true;
        }
    }

    return false;
}

bool Control::ButtonDown(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if( IsEnabled() ) {
        PrivateSetState(kControlStatePushed);
        SetMouseFocused(true);
        Invalidate();
    }
    return true;
}

bool Control::ButtonUp(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if( IsMouseFocused() ) {
        SetMouseFocused(false);
        auto player = GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot);
        if (player != nullptr) {
            player->Stop();
        }
        Invalidate();
        if( IsPointInWithScrollOffset(msg.ptMouse) ) {
            PrivateSetState(kControlStateHot);
            m_nHotAlpha = 255;
            Activate(&msg);
        }
        else {
            PrivateSetState(kControlStateNormal);
            m_nHotAlpha = 0;
        }
    }
    return true;
}

bool Control::ButtonDoubleClick(const EventArgs& /*msg*/)
{
    return true;
}

bool Control::RButtonDown(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (IsEnabled()) {
        SetMouseFocused(true);
    }
    return true;
}

bool Control::RButtonUp(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (IsMouseFocused()) {
        SetMouseFocused(false);
        if (IsPointInWithScrollOffset(msg.ptMouse)) {
            EventArgs msgData;
            msgData.ptMouse = msg.ptMouse;
            msgData.modifierKey = msg.modifierKey;
            SendEvent(kEventRClick, msgData);
        }
    }
    return true;
}

bool Control::RButtonDoubleClick(const EventArgs& /*msg*/)
{
    return true;
}

bool Control::MButtonDown(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (IsEnabled()) {
        SetMouseFocused(true);
    }
    return true;
}

bool Control::MButtonUp(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (IsMouseFocused()) {
        SetMouseFocused(false);
    }
    return true;
}

bool Control::MButtonDoubleClick(const EventArgs& /*msg*/)
{
    return true;
}

bool Control::MouseMove(const EventArgs& /*msg*/)
{
    return true;
}

bool Control::MouseHover(const EventArgs& /*msg*/)
{
    return true;
}

bool Control::MouseWheel(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    //int32_t wheelDelta = msg.eventData;
    return false;
}

bool Control::MouseMenu(const EventArgs& /*msg*/)
{
    //按Shif + F10由系统产生上下文菜单, 或者点击右键触发菜单：默认不处理，交由父控件处理
    return false;
}

bool Control::OnChar(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnKeyDown(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnKeyUp(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnSetCursor(const EventArgs& /*msg*/)
{
    switch (m_cursorType) {
    case CursorType::kCursorHand:
        {
            if (IsEnabled()) {
                SetCursor(CursorType::kCursorHand);
            }
            else {
                SetCursor(CursorType::kCursorArrow);
            }
        }
        break;
    default:
        SetCursor(m_cursorType);
        break;
    }
    return true;
}

void Control::SetCursor(CursorType cursorType)
{
    GlobalManager::Instance().Cursor().SetCursor(cursorType);
}

bool Control::OnSetFocus(const EventArgs& /*msg*/)
{
#if defined (DUILIB_BUILD_FOR_WIN)
    //默认情况下，控件获得焦点时，关闭输入法
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pWindow->NativeWnd()->SetImeOpenStatus(false);
    }
#endif

    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHot);
        Invalidate();
    }
    return true;
}

bool Control::OnKillFocus(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (GetState() == kControlStateHot) {
        SetState(kControlStateNormal);
    }
    else if (GetState() == kControlStatePushed) {
        //失去焦点时，修复控件状态（如果鼠标按下时，窗口失去焦点，鼠标弹起事件这个控件就收不到了）
        SetMouseFocused(false);
        auto player = GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot);
        if (player != nullptr) {
            player->Stop();
        }
        SetState(kControlStateNormal);
    }
    Invalidate();
    return true;
}

bool Control::OnWindowSetFocus(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnWindowKillFocus(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnCaptureChanged(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnImeSetContext(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnImeStartComposition(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnImeComposition(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::OnImeEndComposition(const EventArgs& /*msg*/)
{
    //默认不处理，交由父控件处理
    return false;
}

bool Control::PaintImage(IRender* pRender,
                         Image* pImage,
                         const DString& strModify, int32_t nFade, 
                         IMatrix* pMatrix,
                         const UiRect* pDestRect,
                         UiRect* pPaintedRect) const
{
    PerformanceStat statPerformance(_T("Control::PaintImage"));
    //注解：strModify参数，目前外部传入的主要是："destscale='false' dest='%d,%d,%d,%d'"
    //                   也有一个类传入了：_T(" corner='%d,%d,%d,%d'")。
    if (pImage == nullptr) {
        //这里可能为空，不需要加断言，为空直接返回
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }

    Image& duiImage = *pImage;
    if (duiImage.HasImageError()) {
        //图片出现解码错误，不绘制
        if (!duiImage.IsDecodeEventFired()) {
            //重用原图时，此事件需要补充
            FireImageEvent(pImage, pImage->GetImagePath(), false, false, true);
        }        
        return false;
    }

    if (duiImage.GetImagePath().empty()) {
        return false;
    }

    if (!duiImage.GetImageAttribute().m_bPaintEnabled) {
        //该图片禁止绘制，返回
        return false;
    }

    LoadImageInfo(duiImage, true);
    std::shared_ptr<ImageInfo> imageInfo = duiImage.GetImageInfo();
    if (duiImage.GetImageAttribute().IsAssertEnabled()) {
        ASSERT(imageInfo != nullptr);
    }
    if (imageInfo == nullptr) {
        //图片加载失败了
        duiImage.SetImageError(true);
        return false;
    }
    ASSERT((imageInfo->GetWidth() > 0) && (imageInfo->GetHeight() > 0));
    if ((imageInfo->GetWidth() <= 0) || (imageInfo->GetHeight() <= 0)) {
        duiImage.SetImageError(true);
        return false;
    }

//#ifdef _DEBUG
//    if (this->GetBkImagePtr() == &duiImage) {
//        DString log = StringUtil::Printf(_T("BkImage: Width=%d, Height=%d, LoadScale=%d, fScale=%.02f"),
//            imageInfo->GetWidth(), imageInfo->GetHeight(),
//            imageInfo->GetLoadDpiScale(), imageInfo->GetImageSizeScale());
//        const_cast<Control*>(this)->SetToolTipText(log);
//    }
//#endif

    ImageAttribute newImageAttribute = duiImage.GetImageAttribute();
    if (!strModify.empty()) {
        newImageAttribute.ModifyAttribute(strModify, Dpi());
    }
    UiRect rcDest = GetRect();
    rcDest.Deflate(GetControlPadding());//去掉内边距
    if (pDestRect != nullptr) {
        //使用外部传入的矩形区域绘制图片
        rcDest = *pDestRect;
    }
    UiRect rcImageDestRect = newImageAttribute.GetImageDestRect(imageInfo->GetWidth(), imageInfo->GetHeight(), Dpi());
    if (ImageAttribute::HasValidImageRect(rcImageDestRect)) {
        //使用配置中指定的目标区域(已按配置做过DPI自适应)
        rcDest = rcImageDestRect;
        rcDest.Offset(GetRect().left, GetRect().top);
    }

    UiRect rcDestCorners;
    UiRect rcSource = newImageAttribute.GetImageSourceRect();
    UiRect rcSourceCorners = newImageAttribute.GetImageCorner();
    imageInfo->ScaleImageSourceRect(Dpi(), rcDestCorners, rcSource, rcSourceCorners);
    
    //运用rcMargin、hAlign、vAlign 三个图片属性
    rcDest.Deflate(newImageAttribute.GetImageMargin(Dpi()));
    rcDest.Validate();
    rcSource.Validate();
    const int32_t nImageWidth = rcSource.Width();
    const int32_t nImageHeight = rcSource.Height();

    bool bAdaptiveDestRect = newImageAttribute.m_bAdaptiveDestRect; //自动适应目标区域（等比例缩放后，按指定对齐方式绘制）
    if (!bAdaptiveDestRect && (!newImageAttribute.m_hAlign.empty() || !newImageAttribute.m_vAlign.empty())) {
        if (!newImageAttribute.m_hAlign.empty() && (nImageWidth > rcDest.Width())) {
            bAdaptiveDestRect = true;
        }
        else if (!newImageAttribute.m_vAlign.empty() && (nImageHeight > rcDest.Height())) {
            bAdaptiveDestRect = true;
        }
    }
    if (bAdaptiveDestRect) {
        //自动适应目标区域（等比例缩放图片）：根据图片大小，调整绘制区域
        rcDest = ImageAttribute::CalculateAdaptiveRect(nImageWidth, nImageHeight,
                                                       rcDest,
                                                       newImageAttribute.m_hAlign.c_str(),
                                                       newImageAttribute.m_vAlign.c_str());
    }
    else {
        //应用对齐方式后，图片将不再拉伸，而是按原大小展示
        if (!newImageAttribute.m_hAlign.empty()) {
            if (newImageAttribute.m_hAlign == _T("left")) {
                rcDest.right = rcDest.left + nImageWidth;
            }
            else if (newImageAttribute.m_hAlign == _T("center")) {
                rcDest.left = rcDest.CenterX() - nImageWidth / 2;
                rcDest.right = rcDest.left + nImageWidth;
            }
            else if (newImageAttribute.m_hAlign == _T("right")) {
                rcDest.left = rcDest.right - nImageWidth;
            }
            else {
                rcDest.right = rcDest.left + nImageWidth;
            }

            if (newImageAttribute.m_vAlign.empty()) {
                rcDest.bottom = rcDest.top + nImageHeight;
            }
        }
        if (!newImageAttribute.m_vAlign.empty()) {
            if (newImageAttribute.m_vAlign == _T("top")) {
                rcDest.bottom = rcDest.top + nImageHeight;
            }
            else if (newImageAttribute.m_vAlign == _T("center")) {
                rcDest.top = rcDest.CenterY() - nImageHeight / 2;
                rcDest.bottom = rcDest.top + nImageHeight;
            }
            else if (newImageAttribute.m_vAlign == _T("bottom")) {
                rcDest.top = rcDest.bottom - nImageHeight;
            }
            else {
                rcDest.bottom = rcDest.top + nImageHeight;
            }

            if (newImageAttribute.m_hAlign.empty()) {
                rcDest.right = rcDest.left + nImageWidth;
            }
        }
    }

    //计算得到的rcDest备份，多帧情况下，会对rcDest修改
    const UiRect rcImageDect = rcDest;
    if (pPaintedRect) {
        //返回绘制的目标区域
        *pPaintedRect = rcImageDect;
    }
    //设置动画图片的区域
    duiImage.SetDrawDestRect(rcImageDect);

    //获取需要绘制的位图图片    
    std::shared_ptr<IBitmap> pBitmap;

    //图片数据是否正在延迟解码中（多线程解码图片数据）
    bool bDataPending = false;

    //是否遇到图片解码错误
    bool bDecodeError = false;

    if (duiImage.IsMultiFrameImage()) {
        //多帧图片
        AnimationFramePtr pAnimationFrame = duiImage.GetCurrentFrame(rcImageDect, rcSource, rcSourceCorners);
        ASSERT(pAnimationFrame != nullptr);
        if (pAnimationFrame == nullptr) {
            return false;
        }
        if (pAnimationFrame->m_pBitmap != nullptr) {
            pBitmap = pAnimationFrame->m_pBitmap;

            //运用部分参数(rcDest需要等比例缩小)
            const int32_t nDestWidth = rcDest.Width();
            const int32_t nDestHeight = rcDest.Height();
            if (pAnimationFrame->m_nOffsetX != 0) {
                float fImageScaleX = static_cast<float>(pAnimationFrame->m_pBitmap->GetWidth()) / imageInfo->GetWidth();
                float fRectScaleX = static_cast<float>(nDestWidth) / imageInfo->GetWidth();
                rcDest.left += ImageUtil::GetScaledImageOffset(pAnimationFrame->m_nOffsetX, fRectScaleX);
                rcDest.right = rcDest.left + (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nDestWidth, fImageScaleX);
            }
            if (pAnimationFrame->m_nOffsetY != 0) {
                float fImageScaleY = static_cast<float>(pAnimationFrame->m_pBitmap->GetHeight()) / imageInfo->GetHeight();
                float fRectScaleY = static_cast<float>(nDestHeight) / imageInfo->GetHeight();
                rcDest.top += ImageUtil::GetScaledImageOffset(pAnimationFrame->m_nOffsetY, fRectScaleY);
                rcDest.bottom = rcDest.top + (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nDestHeight, fImageScaleY);
            }
        }
        else if (pAnimationFrame->m_bDataPending) {
            //数据尚未准备好, 可忽略
            ASSERT(pAnimationFrame->m_pBitmap == nullptr);
            if (duiImage.GetImageAttribute().m_bAsyncLoad) {
                bDataPending = true;
            }
            else {
                ASSERT(!"pAnimationFrame->m_bDataPending is invalid!");
            }
        }
        else if (pAnimationFrame->m_bDataError) {
            //遇到图片解码错误
            bDecodeError = true;
        }
        else {
            //其他未知情况，流程有错误
            ASSERT(!"pAnimationFrame->m_pBitmap is invalid!");
        }
    }
    else {
        //单帧图片
        bool bImageStretch = true;//绘制图片时会不会被拉伸
        if (newImageAttribute.IsTiledDraw()) {
            //当设置平铺时，无需拉伸图片
            bImageStretch = false;
        }
        else if (newImageAttribute.m_bWindowShadowMode) {
            //阴影模式：不拉伸，避免四个角变形
            bImageStretch = false;
        }        
        pBitmap = duiImage.GetCurrentBitmap(bImageStretch, rcImageDect, rcSource, rcSourceCorners, &bDecodeError);
        if (pBitmap == nullptr) {
            if (!bDecodeError && duiImage.GetImageAttribute().m_bAsyncLoad) {
                bDataPending = true;
            }
        }
    }

    bool bPainted = false;
    if (pBitmap != nullptr) {
        bPainted = true;
        //校验rcSource(多帧的情况下，实际图片与总宽高可能不符，需要进一步校验)
        if ((rcSource.left < 0) || (rcSource.left >= (int32_t)pBitmap->GetWidth())) {
            rcSource.left = 0;
        }
        if ((rcSource.top < 0) || (rcSource.top >= (int32_t)pBitmap->GetHeight())) {
            rcSource.top = 0;
        }
        if ((rcSource.right < 0) || (rcSource.right > (int32_t)pBitmap->GetWidth())) {
            rcSource.right = (int32_t)pBitmap->GetWidth();
        }
        if ((rcSource.bottom < 0) || (rcSource.bottom > (int32_t)pBitmap->GetHeight())) {
            rcSource.bottom = (int32_t)pBitmap->GetHeight();
        }

        //图片透明度属性
        uint8_t iFade = (nFade == DUI_NOSET_VALUE) ? newImageAttribute.m_bFade : static_cast<uint8_t>(nFade);
        if (pMatrix != nullptr) {
            //矩阵绘制: 对不支持的属性，增加断言，避免出错
            ASSERT(newImageAttribute.GetImageCorner().IsEmpty());
            ASSERT(!newImageAttribute.IsTiledDraw());
            pRender->DrawImageRect(m_rcPaint, pBitmap.get(), rcDest, rcSource, iFade, pMatrix);
        }
        else {
            TiledDrawParam tiledDrawParam;
            if (newImageAttribute.m_pTiledDrawParam != nullptr) {
                tiledDrawParam = newImageAttribute.GetTiledDrawParam(Dpi());
            }
            pRender->DrawImage(m_rcPaint, pBitmap.get(), rcDest, rcDestCorners, rcSource, rcSourceCorners,
                               iFade,
                               newImageAttribute.IsTiledDraw() ? &tiledDrawParam : nullptr,
                               newImageAttribute.m_bWindowShadowMode);
        }

        //绘制成功后，从延迟绘制列表中删除
        GlobalManager::Instance().Image().RemoveDelayPaintData(pImage);
    }
    else if (bDataPending) {
        //当前为异步加载图片, 添加到延迟绘制列表        
        Control* pControl = const_cast<Control*>(this);
        DString imageKey = imageInfo->GetImageKey();
        GlobalManager::Instance().Image().AddDelayPaintData(pControl, pImage, imageKey);
    }
    else if (bDecodeError) {
        //遇到图片解码错误
        duiImage.SetImageError(true);
    }
    //按需启动动画
    if (!duiImage.HasImageError()) {
        if (duiImage.IsMultiFrameImage()) {
            duiImage.CheckStartImageAnimation();
        }
    }    
    return bPainted;
}

std::unique_ptr<AutoClip> Control::CreateRectClip(IRender* pRender, const UiRect& rc, bool bClip) const
{
    if (!bClip) {
        return nullptr;
    }
    return std::make_unique<AutoClip>(pRender, rc, bClip);
}

std::unique_ptr<AutoClip> Control::CreateRoundClip(IRender* pRender, const UiRect& rc, bool bRoundClip) const
{
    float fRoundWidth = 0;
    float fRoundHeight = 0;
    if (!bRoundClip || !GetBorderRound(fRoundWidth, fRoundHeight)) {
        return nullptr;
    }
    return std::make_unique<AutoClip>(pRender, rc, fRoundWidth, fRoundHeight, bRoundClip);
}

void Control::SetPaintRect(const UiRect& rect)
{
    m_rcPaint = rect;
}

std::unique_ptr<IRender> Control::CreateTempRender() const
{
    std::unique_ptr<IRender> spTempRender;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        ASSERT(GetWindow() != nullptr);
        IRenderDpiPtr spRenderDpi;
        if (GetWindow() != nullptr) {
            spRenderDpi = GetWindow()->GetRenderDpi();
        }
        spTempRender.reset(pRenderFactory->CreateRender(spRenderDpi));
    }
    return spTempRender;
}

void Control::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (GetRect().IsEmpty()) {
        return;
    }    
    if (m_nAlpha == 0) {
        //控件完全透明，不绘制
        return;
    }

    UiRect rcTemp; //本控件范围内的脏区域，本次需要绘制的区域
    if (!UiRect::Intersect(rcTemp, rcPaint, GetBoxShadowExpandedRect(GetRect()))) {//如果包含box-shadow的区域内为脏区域，就需要进行绘制
        return;
    }
    UiRect::Intersect(m_rcPaint, rcPaint, GetRect()); //设置m_rcPaint的值

    //是否为直角矩形区域设置为剪辑区域
    const bool bRectClip = IsClip();

    //是否为圆角矩形区域设置为剪辑区域
    const bool bRoundClip = IsClip() && ShouldBeRoundRectFill();

    //当前控件是否设置了透明度（透明度值不是255）
    const bool bAlpha = IsAlpha();

    //本控件是否设置了box-shadow（控件阴影效果）
    const bool bPaintBoxShadow = HasBoxShadow();

    //控件绘制的位置偏移（用于控件的动画效果）
    const UiPoint renderOffset = GetRenderOffset();

    if (bAlpha) {
        //当设置了透明度时，该控件（若为容器则包含子控件）需要完整绘制
        UiRect rcPaintRect = GetRect();
        SetPaintRect(rcPaintRect);
        if (m_pTempRender == nullptr) {
            m_pTempRender = CreateTempRender();
        }
        IRender* pTempRender = m_pTempRender.get();
        ASSERT(pTempRender != nullptr);
        if (pTempRender == nullptr) {
            return;
        }
        if ((pTempRender->GetWidth() != GetRect().Width()) || (pTempRender->GetHeight() != GetRect().Height())) {
            if (!pTempRender->Resize(GetRect().Width(), GetRect().Height())) {
                //存在错误，绘制失败
                ASSERT(!"pTempRender->Resize failed!");
                return;
            }
        }
        
        if ((pTempRender->GetWidth() > 0) && (pTempRender->GetHeight() > 0)) {
            // 将控件（如果是容器，则包含子控件），完整绘制到缓存新的render中
            // 绘制前，首先清除原内容
            pTempRender->Clear(UiColor());

            const UiPoint ptOffset(GetRect().left, GetRect().top);
            const UiPoint ptOldOrg = pTempRender->OffsetWindowOrg(ptOffset);

            std::unique_ptr<AutoClip> rectCacheClip = CreateRectClip(pTempRender, GetRect(), bRectClip);
            std::unique_ptr<AutoClip> roundCacheClip = CreateRoundClip(pTempRender, GetRect(), bRoundClip);

            //首先绘制自己
            Paint(pTempRender, rcPaintRect);

            //设置了透明度，将子控件绘制到pTempRender上面，然后整体AlphaBlend到pRender
            PaintChild(pTempRender, rcPaintRect);
            if (IsBordersOnTop()) {
                PaintBorder(pTempRender);  //绘制边框
            }
            PaintLoading(pTempRender, rcPaintRect); //绘制Loading图片，无状态，需要在绘制完子控件后再绘制
            PaintForeColor(pTempRender); //绘制前景色

            pTempRender->SetWindowOrg(ptOldOrg);
        }

        //如果配置了box-shadow，先绘制，因为box-shadow会超出rect边界绘制(如果使用剪辑区域，会显示不全)        
        if (bPaintBoxShadow) {
            m_bBoxShadowPainted = false;
            PaintShadow(pRender);
            m_bBoxShadowPainted = true;
        }
        UiPoint ptOldOrg = pRender->OffsetWindowOrg(renderOffset);//控件的位置偏移，显示为动画效果
        std::unique_ptr<AutoClip> rectClip = CreateRectClip(pRender, GetRect(), bRectClip);
        std::unique_ptr<AutoClip> roundClip = CreateRoundClip(pRender, GetRect(), bRoundClip);

        int32_t xOffset = std::max(rcPaintRect.left - GetRect().left, 0);
        int32_t yOffset = std::max(rcPaintRect.top - GetRect().top, 0);
        pRender->AlphaBlend(rcPaintRect.left,
                            rcPaintRect.top,
                            rcPaintRect.Width() - xOffset,
                            rcPaintRect.Height() - yOffset,
                            pTempRender,
                            xOffset,
                            yOffset,
                            rcPaintRect.Width() - xOffset,
                            rcPaintRect.Height() - yOffset,
                            static_cast<uint8_t>(m_nAlpha));
        if (bPaintBoxShadow) {
            //Paint绘制后，立即复位标志，避免影响其他绘制逻辑
            m_bBoxShadowPainted = false;
        }
        pRender->SetWindowOrg(ptOldOrg);//恢复视图原点
        UiRect::Intersect(m_rcPaint, rcPaint, GetRect()); //设置m_rcPaint的值
    }
    else {
        //本控件未设置透明度，不使用缓存绘制，直接在目标render上绘制本控件（若为容器，则也包含子控件）        
        UiPoint ptOldOrg = pRender->OffsetWindowOrg(renderOffset);//控件的位置偏移，显示为动画效果

        //如果配置了box-shadow，先绘制，因为box-shadow会超出rect边界绘制(如果使用剪辑区域，会显示不全)        
        if (bPaintBoxShadow) {
            m_bBoxShadowPainted = false;
            PaintShadow(pRender);
            m_bBoxShadowPainted = true;
        }

        std::unique_ptr<AutoClip> rectClip = CreateRectClip(pRender, GetRect(), bRectClip);
        std::unique_ptr<AutoClip> roundClip = CreateRoundClip(pRender, GetRect(), bRoundClip);
        Paint(pRender, rcPaint);        //绘制控件自身
        if (bPaintBoxShadow) {
            //Paint绘制后，立即复位标志，避免影响其他绘制逻辑
            m_bBoxShadowPainted = false;
        }
        PaintChild(pRender, rcPaint);   //绘制子控件
        if (IsBordersOnTop()) {
            PaintBorder(pRender);       //绘制边框
        }
        PaintLoading(pRender, rcPaint); //绘制Loading状态，无状态，需要在绘制完子控件后再绘制
        PaintForeColor(pRender);        //绘制前景色

        pRender->SetWindowOrg(ptOldOrg);//恢复视图原点
    }
}

void Control::Paint(IRender* pRender, const UiRect& rcPaint)
{
    UiRect rcTemp; //本控件范围内的脏区域，本次需要绘制的区域
    if (!UiRect::Intersect(rcTemp, rcPaint, GetBoxShadowExpandedRect(GetRect()))) {//如果包含box-shadow的区域内为脏区域，就需要进行绘制
        return;
    }
    UiRect::Intersect(m_rcPaint, rcPaint, GetRect()); //设置m_rcPaint的值

    if (!m_bBoxShadowPainted) {
        //绘制box-shadow，可能会超出rect边界绘制(如果使用裁剪，可能会显示不全)
        PaintShadow(pRender);
    }    

    //绘制其他内容
    PaintBkColor(pRender);        //背景颜色(覆盖整个矩形)
    PaintStateColors(pRender);    //控件指定状态的颜色：普通状态、焦点状态、按下状态、禁用状态(覆盖整个矩形)
    PaintBkImage(pRender);        //背景图片，无状态
    PaintStateImages(pRender);    //先绘制背景图片，然后绘制前景图片，每个图片有指定的状态：普通状态、焦点状态、按下状态、禁用状态
    PaintText(pRender);           //绘制文本
    if (!IsBordersOnTop()) {
        PaintBorder(pRender);     //绘制边框
    }    
    PaintFocusRect(pRender);      //绘制焦点状态
}

void Control::PaintShadow(IRender* pRender)
{
    if (!HasBoxShadow()) {
        return;
    }
    BoxShadow boxShadow(this);
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pBoxShadow != nullptr)) {
        boxShadow = *m_pOtherData->m_pBoxShadow;
    }

    ASSERT(pRender != nullptr);
    if (pRender != nullptr) {
        UiSize borderRound;
        float fRoundWidth = 0;
        float fRoundHeight = 0;
        if (GetBorderRound(fRoundWidth, fRoundHeight)) {
            borderRound.cx = (int32_t)(fRoundWidth + 0.5f);
            borderRound.cy = (int32_t)(fRoundHeight + 0.5f);
        }
        pRender->DrawBoxShadow(GetRect(),
                               borderRound,
                               boxShadow.m_cpOffset,
                               boxShadow.m_nBlurRadius,
                               boxShadow.m_nSpreadRadius,
                               GlobalManager::Instance().Color().GetColor(boxShadow.m_strColor));
    }    
}

void Control::PaintBkColor(IRender* pRender)
{
    if ((m_pColorData == nullptr) || m_pColorData->m_strBkColor.empty()) {
        return;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    UiColor dwBackColor = GetUiColor(m_pColorData->m_strBkColor.c_str());
    if(dwBackColor.GetARGB() != 0) {
        int32_t nBorderSize = 0;
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.left > 0.001f) &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.right) &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.top)   &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.bottom)) {
            //四个边都存在，且大小相同
            nBorderSize = static_cast<int32_t>(m_pBorderData->m_rcBorderSize.left);//不做四舍五入
        }
        nBorderSize /= 2;

        //背景填充矩形范围
        UiRect fillRect = GetRect();
        if (nBorderSize > 0) { 
            //如果存在边线，则填充的时候，不填充边线所在位置，避免出现背景色的锯齿现象
            UiRect borderRect(nBorderSize, nBorderSize, nBorderSize, nBorderSize);
            fillRect.Deflate(borderRect.left, borderRect.top, borderRect.right, borderRect.bottom);
        }
        if (ShouldBeRoundRectFill()) {
            //需要绘制圆角矩形，填充也需要填充圆角矩形
            float fRoundWidth = 0;
            float fRoundHeight = 0;
            GetBorderRound(fRoundWidth, fRoundHeight);
            FillRoundRect(pRender, fillRect, fRoundWidth, fRoundHeight, dwBackColor);
        }
        else {            
            UiColor dwBackColor2;
            if ((m_pColorData != nullptr) && !m_pColorData->m_strBkColor2.empty()) {
                dwBackColor2 = GetUiColor(m_pColorData->m_strBkColor2.c_str());
            }
            if (!dwBackColor2.IsEmpty()) {
                //渐变背景色
                int8_t nColor2Direction = 1;
                if (m_pColorData != nullptr) {
                    nColor2Direction = m_pColorData->m_nBkColor2Direction;
                }
                pRender->FillRect(fillRect, dwBackColor, dwBackColor2, nColor2Direction);
            }
            else {
                pRender->FillRect(fillRect, dwBackColor);
            }            
        }
    }
}

void Control::PaintForeColor(IRender* pRender)
{
    if ((m_pColorData == nullptr) || m_pColorData->m_strForeColor.empty()) {
        return;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    UiColor dwForeColor = GetUiColor(m_pColorData->m_strForeColor.c_str());
    if (dwForeColor.GetARGB() != 0) {
        int32_t nBorderSize = 0;
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.left > 0.001f) &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.right) &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.top) &&
            IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.bottom)) {
            //四个边都存在，且大小相同
            nBorderSize = static_cast<int32_t>(m_pBorderData->m_rcBorderSize.left);//不做四舍五入
        }
        nBorderSize /= 2;

        //背景填充矩形范围
        UiRect fillRect = GetRect();
        if (nBorderSize > 0) {
            //如果存在边线，则填充的时候，不填充边线所在位置，避免出现背景色的锯齿现象
            UiRect borderRect(nBorderSize, nBorderSize, nBorderSize, nBorderSize);
            fillRect.Deflate(borderRect.left, borderRect.top, borderRect.right, borderRect.bottom);
        }
        if (ShouldBeRoundRectFill()) {
            //需要绘制圆角矩形，填充也需要填充圆角矩形
            float fRoundWidth = 0;
            float fRoundHeight = 0;
            GetBorderRound(fRoundWidth, fRoundHeight);
            FillRoundRect(pRender, fillRect, fRoundWidth, fRoundHeight, dwForeColor);
        }
        else {
            pRender->FillRect(fillRect, dwForeColor);
        }
    }
}

void Control::PaintBorder(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    UiColor dwBorderColor;
    DString borderColor;
    if (IsFocused()) {
        if (borderColor.empty()) {
            borderColor = GetFocusBorderColor();
        }
    }
    if (borderColor.empty()) {
        borderColor = GetBorderColor(GetState());
    }
    if (!borderColor.empty()) {
        dwBorderColor = GetUiColor(borderColor);
    }
    if (dwBorderColor.GetARGB() == 0) {
        return;
    }
    bool bPainted = false;
    if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.left > 0.001f) &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.right) &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.top)   &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.bottom)) {
        //四个边都存在，且大小相同，则直接绘制矩形, 支持圆角矩形
        if (ShouldBeRoundRectBorders()) {
            //仅圆角矩形，使用这个函数绘制边线
            PaintBorders(pRender, GetRect(), m_pBorderData->m_rcBorderSize.left, dwBorderColor, GetBorderDashStyle());
            bPainted = true;
        }
    }

    if(!bPainted) {
        //非圆角矩形，四个边分别按照设置绘制边线
        const float epsilon = 0.001f;
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.left > epsilon)) {
            //左边线
            UiRect rcBorder = GetRect();
            if (std::fabs(m_pBorderData->m_rcBorderSize.left - 1.0f) < epsilon) {
                rcBorder.bottom -= 1;
            }
            const float fWidth = (float)m_pBorderData->m_rcBorderSize.left;
            UiPointF pt1((float)rcBorder.left + fWidth / 2, (float)rcBorder.top);
            UiPointF pt2((float)rcBorder.left + fWidth / 2, (float)rcBorder.bottom);
            DrawBorderLine(pRender, pt1, pt2, m_pBorderData->m_rcBorderSize.left, dwBorderColor, GetBorderDashStyle());
        }
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.top > epsilon)) {
            //上边线
            UiRect rcBorder = GetRect();
            if (std::fabs(m_pBorderData->m_rcBorderSize.top - 1.0f) < epsilon) {
                rcBorder.right -= 1;
            }
            const float fWidth = (float)m_pBorderData->m_rcBorderSize.top;
            UiPointF pt1((float)rcBorder.left, (float)rcBorder.top + fWidth / 2);
            UiPointF pt2((float)rcBorder.right, (float)rcBorder.top + fWidth / 2);
            DrawBorderLine(pRender, pt1, pt2, m_pBorderData->m_rcBorderSize.top, dwBorderColor, GetBorderDashStyle());
        }
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.right > epsilon)) {
            //右边线
            UiRect rcBorder = GetRect();
            if (std::fabs(m_pBorderData->m_rcBorderSize.right - 1.0f) < epsilon) {
                rcBorder.bottom -= 1;
            }
            const float fWidth = (float)m_pBorderData->m_rcBorderSize.right;
            UiPointF pt1((float)rcBorder.right - fWidth / 2, (float)rcBorder.top);
            UiPointF pt2((float)rcBorder.right - fWidth / 2, (float)rcBorder.bottom);
            DrawBorderLine(pRender, pt1, pt2, m_pBorderData->m_rcBorderSize.right, dwBorderColor, GetBorderDashStyle());
        }
        if ((m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.bottom > epsilon)) {
            //下边线
            UiRect rcBorder = GetRect();
            if (std::fabs(m_pBorderData->m_rcBorderSize.bottom - 1.0f) < epsilon) {
                rcBorder.right -= 1;
            }
            const float fWidth = (float)m_pBorderData->m_rcBorderSize.bottom;
            UiPointF pt1((float)rcBorder.left, (float)rcBorder.bottom - fWidth / 2);
            UiPointF pt2((float)rcBorder.right, (float)rcBorder.bottom - fWidth / 2);
            DrawBorderLine(pRender, pt1, pt2, m_pBorderData->m_rcBorderSize.bottom, dwBorderColor, GetBorderDashStyle());
        }
    }
}

void Control::DrawBorderLine(IRender* pRender, const UiPointF& pt1, const UiPointF& pt2,
                             float fBorderSize, UiColor dwBorderColor, int8_t borderDashStyle)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    IPen::DashStyle dashStyle = (IPen::DashStyle)borderDashStyle;
    if (dashStyle == IPen::DashStyle::kDashStyleSolid) {
        //普通实线
        pRender->DrawLine(pt1, pt2, dwBorderColor, fBorderSize);
    }
    else {
        //其他线形
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IPen> pPen(pRenderFactory->CreatePen(dwBorderColor, fBorderSize));
            pPen->SetDashStyle(dashStyle);
            pRender->DrawLine(pt1, pt2, pPen.get());
        }
        else {
            pRender->DrawLine(pt1, pt2, dwBorderColor, fBorderSize);
        }
    }
}

void Control::PaintBorders(IRender* pRender, UiRect rcDraw,
                           float fBorderSize, UiColor dwBorderColor, int8_t borderDashStyle) const
{
    if ((pRender == nullptr) || rcDraw.IsEmpty() || (fBorderSize < 0.1) || (dwBorderColor.GetARGB() == 0)) {
        return;
    }

    //绘制边线：确保边线在矩形范围内
    UiRectF rcDrawF((float)rcDraw.left, (float)rcDraw.top, (float)rcDraw.right, (float)rcDraw.bottom);
    float fHalfBorderSize = fBorderSize / 2;
    rcDrawF.left += fHalfBorderSize;
    rcDrawF.top += fHalfBorderSize;
    rcDrawF.right -= fHalfBorderSize;
    rcDrawF.bottom -= fHalfBorderSize;

    if (ShouldBeRoundRectBorders()) {
        float fRoundWidth = 0;
        float fRoundHeight = 0;
        GetBorderRound(fRoundWidth, fRoundHeight);        
        DrawRoundRect(pRender, rcDrawF, fRoundWidth, fRoundHeight, dwBorderColor, fBorderSize, borderDashStyle);
    }
    else {
        if (borderDashStyle == IPen::DashStyle::kDashStyleSolid) {
            //普通实线
            pRender->DrawRect(rcDrawF, dwBorderColor, fBorderSize, false);
        }
        else {
            //其他线形
            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            if (pRenderFactory != nullptr) {
                std::unique_ptr<IPen> pPen(pRenderFactory->CreatePen(dwBorderColor, fBorderSize));
                pPen->SetDashStyle((IPen::DashStyle)borderDashStyle);
                pRender->DrawRect(rcDrawF, pPen.get(), false);
            }
            else {
                pRender->DrawRect(rcDrawF, dwBorderColor, fBorderSize, false);
            }
        }
    }
}

bool Control::ShouldBeRoundRectFill() const
{
    bool isRoundRect = false;
    if ((m_pBorderData != nullptr) &&
        (IsFloatEqual(m_pBorderData->m_rcBorderSize.left, 0.0f) || (m_pBorderData->m_rcBorderSize.left > 0.001f)) &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.right)      &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.top)        &&
        IsFloatEqual(m_pBorderData->m_rcBorderSize.left, m_pBorderData->m_rcBorderSize.bottom)) {
        //四个边大小相同(无论是零还是大于零)，支持圆角矩形
        if (HasBorderRound()) {
            isRoundRect = true;
        }
    }
    else {
        if (HasBorderRound()) {
            isRoundRect = true;
        }
    }
    if (isRoundRect) {
        //窗口最大化时，对于Root Box控件，不绘制圆角边线
        if (IsRootBox() && (GetWindow() != nullptr)) {
            if (GetWindow()->IsWindowMaximized()) {
                isRoundRect = false;
            }
        }
    }    
    return isRoundRect;
}

bool Control::ShouldBeRoundRectBorders() const
{
    bool isRoundRect = ShouldBeRoundRectFill();
    return isRoundRect && (m_pBorderData != nullptr) && (m_pBorderData->m_rcBorderSize.left > 0.001f);
}

void Control::PaintFocusRect(IRender* pRender)
{
    if ((pRender != nullptr) && IsShowFocusRect() && IsFocused()) {
        DoPaintFocusRect(pRender);    //绘制焦点状态
    }
}

void Control::DoPaintFocusRect(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        return;
    }
    float fWidth =  Dpi().GetScaleFloat(1.0f); //画笔宽度
    UiColor dwBorderColor;//画笔颜色
    DString focusRectColor = GetFocusRectColor();
    if (!focusRectColor.empty()) {
        dwBorderColor = GetUiColor(focusRectColor);
    }
    if(dwBorderColor.IsEmpty()) {
        dwBorderColor = UiColor(UiColors::Gray);
    }
    UiRect rcBorderSize(1, 1, 1, 1);
    UiRect rcFocusRect = GetRect();
    int32_t nFocusWidth = Dpi().GetScaleInt(2); //矩形间隙
    rcFocusRect.Deflate(nFocusWidth, nFocusWidth);
    if (rcFocusRect.IsEmpty()) {
        return;
    }
    std::unique_ptr<IPen> pPen(pRenderFactory->CreatePen(dwBorderColor, fWidth));
    pPen->SetDashStyle(IPen::DashStyle::kDashStyleDot);

    if (rcBorderSize.left > 0) {
        //左边线
        UiPoint pt1(rcFocusRect.left, rcFocusRect.top);
        UiPoint pt2(rcFocusRect.left, rcFocusRect.bottom);
        pRender->DrawLine(pt1, pt2, pPen.get());
    }
    if (rcBorderSize.top > 0) {
        //上边线
        UiPoint pt1(rcFocusRect.left, rcFocusRect.top);
        UiPoint pt2(rcFocusRect.right, rcFocusRect.top);
        pRender->DrawLine(pt1, pt2, pPen.get());
    }
    if (rcBorderSize.right > 0) {
        //右边线
        UiPoint pt1(rcFocusRect.right, rcFocusRect.top);
        UiPoint pt2(rcFocusRect.right, rcFocusRect.bottom);
        pRender->DrawLine(pt1, pt2, pPen.get());
    }
    if (rcBorderSize.bottom > 0) {
        //下边线
        UiPoint pt1(rcFocusRect.left, rcFocusRect.bottom);
        UiPoint pt2(rcFocusRect.right, rcFocusRect.bottom);
        pRender->DrawLine(pt1, pt2, pPen.get());
    }
}

bool Control::IsRootBox() const
{
    bool isRootBox = false;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        if ((Control*)pWindow->GetRoot() == this) {
            isRootBox = true;
        }
        else if ((Control*)pWindow->GetXmlRoot() == this) {
            isRootBox = true;
        }
    }
    return isRootBox;
}

bool Control::IsWindowRoundRect() const
{
    bool isWindowRoundRect = false;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        UiSize roundCorner = pWindow->GetRoundCorner();
        if ((roundCorner.cx > 0) && (roundCorner.cy > 0)) {
            isWindowRoundRect = true;
        }
    }
    return isWindowRoundRect;
}

void Control::AddRoundRectPath(IPath* path, const UiRectF& rc, float rx, float ry) const
{
    UiRect rcI((int32_t)(rc.left + 0.5f), (int32_t)(rc.top + 0.5f), (int32_t)(rc.right + 0.5f), (int32_t)(rc.bottom + 0.5f));
    AddRoundRectPath(path, rcI, rx, ry);
}

void Control::AddRoundRectPath(IPath* path, const UiRect& rc, float rx, float ry) const
{
    ASSERT(path != nullptr);
    if (path == nullptr) {
        return;
    }
    //确保圆角宽度和高度都是偶数
    UiSize roundSize;
    roundSize.cx = (int32_t)(rx + 0.5f);
    roundSize.cy = (int32_t)(ry + 0.5f);
    
    if ((roundSize.cx % 2) != 0) {
        roundSize.cx += 1;
    }
    if ((roundSize.cy % 2) != 0) {
        roundSize.cy += 1;
    }
    //这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
    path->AddArc(UiRect(rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
    path->AddLine(rc.left + roundSize.cx / 2, rc.top, rc.right - roundSize.cx / 2, rc.top);
    path->AddArc(UiRect(rc.right - roundSize.cx, rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
    path->AddLine(rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
    path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
    path->AddLine(rc.right - roundSize.cx / 2, rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
    path->AddArc(UiRect(rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
    path->AddLine(rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
    path->Close();
}

void Control::DrawRoundRect(IRender* pRender, const UiRect& rc, float rx, float ry,
                            UiColor dwBorderColor, float fBorderSize,
                            int8_t borderDashStyle) const
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRoundRect(pRender, rcF, rx, ry, dwBorderColor, fBorderSize, borderDashStyle);
}

void Control::DrawRoundRect(IRender* pRender, const UiRectF& rc, float rx, float ry,
                            UiColor dwBorderColor, float fBorderSize,
                            int8_t borderDashStyle) const
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (pRender->GetRenderType() != RenderType::kRenderType_Skia) {
        ASSERT(0);//目前没有其他类型的绘制引擎，代码走不到这里了。
        pRender->DrawRoundRect(rc, rx, ry, dwBorderColor, fBorderSize);
        return;
    }
    bool isDrawOk = false;
    if (IsRootBox() && IsWindowRoundRect()) {
        //使用与Windows一致的绘制方式，避免与Windows的不一致
        //参见：WindowBase::OnSizeMsg中的CreateRoundRectRgn（Skia的圆角画法和CreateRoundRectRgn不一样）
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(dwBorderColor, fBorderSize));
            std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
            if (pen && path) {
                if (borderDashStyle != IPen::DashStyle::kDashStyleSolid) {
                    pen->SetDashStyle((IPen::DashStyle)borderDashStyle);
                }
                //这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
                AddRoundRectPath(path.get(), rc, rx, ry);
                pRender->DrawPath(path.get(), pen.get());
                isDrawOk = true;
            }
        }
    }
    if(!isDrawOk) {
        if (borderDashStyle == IPen::DashStyle::kDashStyleSolid) {
            //普通实线
            pRender->DrawRoundRect(rc, rx, ry, dwBorderColor, fBorderSize);
        }
        else {
            //其他线形
            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            if (pRenderFactory != nullptr) {
                std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(dwBorderColor, fBorderSize));
                pen->SetDashStyle((IPen::DashStyle)borderDashStyle);
                pRender->DrawRoundRect(rc, rx, ry, pen.get());
            }
            else {
                pRender->DrawRoundRect(rc, rx, ry, dwBorderColor, fBorderSize);
            }
        }
    }
}

void Control::FillRoundRect(IRender* pRender, const UiRect& rc, float rx, float ry, UiColor dwColor) const
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (pRender->GetRenderType() != RenderType::kRenderType_Skia) {
        //非Skia引擎
        pRender->FillRoundRect(rc, rx, ry, dwColor);
        return;
    }

    bool isDrawOk = false;
    if (IsRootBox() && IsWindowRoundRect()) {
        //使用与Windows一致的绘制方式，避免与Windows的不一致
        //参见：WindowBase::OnSizeMsg中的CreateRoundRectRgn（Skia的圆角画法和CreateRoundRectRgn不一样）
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(dwColor));
            std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
            if (brush && path) {
                //这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
                AddRoundRectPath(path.get(), rc, rx, ry);
                UiColor dwBackColor2;
                if ((m_pColorData != nullptr) && !m_pColorData->m_strBkColor2.empty()) {
                    dwBackColor2 = GetUiColor(m_pColorData->m_strBkColor2.c_str());
                }
                if (!dwBackColor2.IsEmpty()) {
                    //渐变背景色
                    int8_t nColor2Direction = 1;
                    if (m_pColorData != nullptr) {
                        nColor2Direction = m_pColorData->m_nBkColor2Direction;
                    }
                    pRender->FillPath(path.get(), rc, dwColor, dwBackColor2, nColor2Direction);
                }
                else {
                    pRender->FillPath(path.get(), brush.get());
                }                
                isDrawOk = true;
            }
        }
    }
    if (!isDrawOk) {
        UiColor dwBackColor2;
        if ((m_pColorData != nullptr) && !m_pColorData->m_strBkColor2.empty()) {
            dwBackColor2 = GetUiColor(m_pColorData->m_strBkColor2.c_str());
        }
        if (!dwBackColor2.IsEmpty()) {
            //渐变背景色
            int8_t nColor2Direction = 1;
            if (m_pColorData != nullptr) {
                nColor2Direction = m_pColorData->m_nBkColor2Direction;
            }
            pRender->FillRoundRect(rc, rx, ry, dwColor, dwBackColor2, nColor2Direction);
        }
        else {
            pRender->FillRoundRect(rc, rx, ry, dwColor);
        }
    }    
}

void Control::PaintBkImage(IRender* pRender)
{
    if (m_pBkImage != nullptr) {
        PaintImage(pRender, m_pBkImage.get());
    }    
}

Image* Control::GetBkImagePtr() const
{
    return m_pBkImage.get();
}

void Control::PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const
{
    if (m_pColorMap != nullptr) {
        m_pColorMap->PaintStateColor(pRender, rcPaint, stateType);
    }
}

void Control::PaintStateColors(IRender* pRender)
{
    if (m_pColorMap != nullptr) {
        m_pColorMap->PaintStateColor(pRender, m_rcPaint, GetState());
    }    
}

void Control::PaintStateImages(IRender* pRender)
{
    if (m_pImageMap != nullptr) {
        m_pImageMap->PaintStateImage(pRender, kStateImageBk, GetState());
        m_pImageMap->PaintStateImage(pRender, kStateImageFore, GetState());
    }    
}

void Control::PaintText(IRender* /*pRender*/)
{
    return;
}

void Control::PaintLoading(IRender* pRender, const UiRect& rcPaint)
{
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pLoading != nullptr)) {
        m_pOtherData->m_pLoading->PaintLoading(pRender, rcPaint);
    }
}

void Control::SetAlpha(int64_t alpha)
{
    ASSERT(alpha >= 0 && alpha <= 255);
    if (m_nAlpha != (uint8_t)alpha) {
        m_nAlpha = (uint8_t)alpha;
        Invalidate();
    }
}

void Control::SetHotAlpha(int64_t nHotAlpha)
{
    ASSERT(nHotAlpha >= 0 && nHotAlpha <= 255);
    if (m_nHotAlpha != (uint8_t)nHotAlpha) {
        m_nHotAlpha = (uint8_t)nHotAlpha;
        Invalidate();
    }
}

void Control::SetTabStop(bool enable)
{
    m_bAllowTabstop = enable;
}

UiPoint Control::GetRenderOffset() const
{
    if (m_pAnimationData != nullptr) {
        return m_pAnimationData->m_renderOffset;
    }
    return UiPoint();
}

void Control::SetRenderOffset(UiPoint renderOffset, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScalePoint(renderOffset);
    }
    if (m_pAnimationData == nullptr) {
        m_pAnimationData = std::make_unique<TAnimationData>();
    }
    if (m_pAnimationData->m_renderOffset != renderOffset) {
        m_pAnimationData->m_renderOffset = renderOffset;
        Invalidate();
    }    
}

void Control::SetRenderOffsetX(int64_t renderOffsetX)
{
    int32_t x = TruncateToInt32(renderOffsetX);
    if (m_pAnimationData == nullptr) {
        m_pAnimationData = std::make_unique<TAnimationData>();
    }
    if (m_pAnimationData->m_renderOffset.x != x) {
        m_pAnimationData->m_renderOffset.x = x;
        Invalidate();
    }
}

void Control::SetRenderOffsetY(int64_t renderOffsetY)
{
    int32_t y = TruncateToInt32(renderOffsetY);
    if (m_pAnimationData == nullptr) {
        m_pAnimationData = std::make_unique<TAnimationData>();
    }
    if (m_pAnimationData->m_renderOffset.y != y) {
        m_pAnimationData->m_renderOffset.y = y;
        Invalidate();
    }
}

void Control::PauseImageAnimation()
{
    //停止该控件内的所有动画
    if (m_pBkImage != nullptr) {
        m_pBkImage->PauseImageAnimation();
    }
    if (m_pImageMap != nullptr) {
        m_pImageMap->PauseImageAnimation();
    }
}

Image* Control::FindImageByName(const DString& imageName) const
{
    Image* pImage = nullptr;
    if (imageName.empty()) {
        //为空则使用背景图片
        pImage = m_pBkImage.get();
    }
    else if ((m_pBkImage != nullptr) && (m_pBkImage->GetImageAttribute().m_sImageName == imageName)) {
        //背景图片
        pImage = m_pBkImage.get();
    }
    else if (m_pImageMap != nullptr) {
        //状态图片
        pImage = m_pImageMap->FindImageByName(imageName);
    }
    return pImage;
}

bool Control::StartImageAnimation(const DString& imageName,
                                  AnimationImagePos nStartFrame,
                                  int32_t nPlayCount)
{
    GlobalManager::Instance().AssertUIThread();
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return false;
    }
    if (!LoadImageInfo(*pImage)) {
        return false;
    }
    return pImage->StartImageAnimation(nStartFrame, nPlayCount);
}

bool Control::StopImageAnimation(const DString& imageName,
                                 AnimationImagePos nStopFrame,
                                 bool bTriggerEvent)
{
    GlobalManager::Instance().AssertUIThread();
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return false;
    }
    if (pImage != nullptr) {
        pImage->StopImageAnimation(nStopFrame, bTriggerEvent);
        return true;
    }
    return false;
}

bool Control::SetImageAnimationFrame(int32_t nFrameIndex)
{
    return SetImageAnimationFrame(DString(), nFrameIndex);
}

bool Control::SetImageAnimationFrame(const DString& imageName, int32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(nFrameIndex >= 0);
    if (nFrameIndex < 0) {
        return false;
    }
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return false;
    }
    if (pImage != nullptr) {
        pImage->SetCurrentFrameIndex((uint32_t)nFrameIndex);
        //重绘
        Invalidate();
        return true;
    }
    return false;
}

uint32_t Control::GetImageAnimationFrameIndex() const
{
    return GetImageAnimationFrameIndex(DString());
}

uint32_t Control::GetImageAnimationFrameIndex(const DString& imageName) const
{
    GlobalManager::Instance().AssertUIThread();
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return 0;
    }
    return pImage->GetCurrentFrameIndex();
}

uint32_t Control::GetImageAnimationFrameCount()
{
    return GetImageAnimationFrameCount(DString());
}

uint32_t Control::GetImageAnimationFrameCount(const DString& imageName)
{
    GlobalManager::Instance().AssertUIThread();
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return 0;
    }
    if (!LoadImageInfo(*pImage)) {
        return 0;
    }
    return pImage->GetFrameCount();
}

bool  Control::IsImageAnimationLoaded() const
{
    return IsImageAnimationLoaded(DString());
}

bool  Control::IsImageAnimationLoaded(const DString& imageName) const
{
    GlobalManager::Instance().AssertUIThread();
    Image* pImage = FindImageByName(imageName);
    if (pImage == nullptr) {
        return false;
    }
    return pImage->GetImageInfo() != nullptr;
}

/** 多线程解码的数据结构(异步，在子线程解码)
*/
struct Control::TAsyncImageDecode
{
    ControlPtr m_pControl;                //关联的控件接口
    ControlPtrT<Image> m_pImage;          //关联的图片接口
    DString m_imagePath;                  //加载图片的路径

    std::shared_ptr<IImage> m_pImageData; //图片数据接口    
    DString m_imageKey;                   //图片数据的KEY，用于更新UI显示
    size_t m_nTaskId = 0;                 //在子线程中的任务ID

    uint32_t m_nFrameCount = 0;           //该图片共有多少帧
    uint32_t m_nDecodeCount = 0;          //共执行多少次异步解码

    bool m_bDecodeExecuted = false;       //释放执行过图片解码操作
    bool m_bDecodeResult = false;         //异步解码是否成功
    bool m_bDecodeError = false;          //异步解码是否遇到错误
};

/** 多线程解码的实现函数(参数使用TAsyncImageDecode智能指针，避免影响std::shared_ptr<IImage>的引用计数)
*/
void Control::AsyncDecodeImageData(std::shared_ptr<TAsyncImageDecode> pAsyncDecoder)
{
    //需要确保在UI线程中执行
    GlobalManager::Instance().AssertUIThread();
    if ((pAsyncDecoder == nullptr) || (pAsyncDecoder->m_pImageData == nullptr)) {
        return;
    }
    std::shared_ptr<IImage>& pImageData = pAsyncDecoder->m_pImageData;
    if (!pImageData->IsAsyncDecodeEnabled() || pImageData->IsAsyncDecodeFinished()) {
        //不需要在线程中解码或者已经解码完成
        return;
    }

    if (pAsyncDecoder->m_nTaskId == 0) {
        if (pImageData->GetAsyncDecodeTaskId() != 0) {
            //不能并行执行任务，已经有线程在解码了
            return;
        }
    }

    //放在子线程中解码
    ThreadManager& threadManager = GlobalManager::Instance().Thread();
    int32_t nThreadIdentifier = ui::kThreadUI;
    std::vector<int32_t> threadIdentifiers;
    if (pImageData->GetImageType() == ImageType::kImageAnimation) {
        //多帧图片
        threadIdentifiers.push_back(ui::kThreadImage2);
        threadIdentifiers.push_back(ui::kThreadImage1);
        threadIdentifiers.push_back(ui::kThreadWorker);
    }
    else {
        //单帧图片
        threadIdentifiers.push_back(ui::kThreadImage1);
        threadIdentifiers.push_back(ui::kThreadImage2);
        threadIdentifiers.push_back(ui::kThreadWorker);
    }
    for (int32_t nThread : threadIdentifiers) {
        if (threadManager.HasThread(nThread)) {
            nThreadIdentifier = nThread;
            break;
        }
    }
    //异步解码完成的通知函数，在主线程中执行
    auto AsyncDecodeImageFinishNotify = [pAsyncDecoder]() {
            //需要确保在UI线程中执行
            GlobalManager::Instance().AssertUIThread();
            if (pAsyncDecoder == nullptr) {
                return;
            }
            if (!pAsyncDecoder->m_bDecodeExecuted) {
                //未执行图片解码操作，不需要再处理
                return;
            }
            int32_t nUseCount = pAsyncDecoder->m_pImageData.use_count(); //资源引用计数
            if (nUseCount == 1) {
                //资源已经释放，不需要再处理
                return;
            }

            //解码计数
            pAsyncDecoder->m_nDecodeCount++;

            //合并数据
            pAsyncDecoder->m_pImageData->MergeAsyncDecodeData();

            //通知相关的控件，重绘界面
            GlobalManager::Instance().Image().DelayPaintImage(pAsyncDecoder->m_imageKey);

            bool bDecodeFinished = pAsyncDecoder->m_pImageData->IsAsyncDecodeFinished();
            bool bDecodeEnabled = pAsyncDecoder->m_pImageData->IsAsyncDecodeEnabled();
            ASSERT(pAsyncDecoder->m_nDecodeCount <= pAsyncDecoder->m_nFrameCount);
            if (pAsyncDecoder->m_nDecodeCount == pAsyncDecoder->m_nFrameCount) {
                ASSERT(bDecodeFinished);
            }

            if (!bDecodeFinished && bDecodeEnabled &&
                pAsyncDecoder->m_bDecodeResult &&
                !pAsyncDecoder->m_bDecodeError &&
                (pAsyncDecoder->m_nDecodeCount <= pAsyncDecoder->m_nFrameCount)) {
                //如果未完成，则继续解码下一帧
                pAsyncDecoder->m_bDecodeExecuted = false;
                AsyncDecodeImageData(pAsyncDecoder);
            }
            else {
                //清除任务ID(仅在完成时清除)
                pAsyncDecoder->m_pImageData->SetAsyncDecodeTaskId(0);

                if ((pAsyncDecoder->m_pControl != nullptr) && (pAsyncDecoder->m_pImage != nullptr)) {
                    bool bDecodeError = true; //默认为解码错误
                    if (!pAsyncDecoder->m_bDecodeError && bDecodeFinished) {
                        //解码完成
                        bDecodeError = false;
                    }
                    if (pAsyncDecoder->m_bDecodeError) {
                        pAsyncDecoder->m_pImage->SetImageError(true);
                    }
                    pAsyncDecoder->m_pControl->FireImageEvent(pAsyncDecoder->m_pImage.get(), pAsyncDecoder->m_imagePath, false, false, bDecodeError);
                }
            }
        };

    //确认需要解码的帧索引号
    uint32_t nCurFrameIndex = 0;
    if (pImageData->GetImageType() == ImageType::kImageAnimation) {
        //多帧
        std::shared_ptr<IAnimationImage> pAnimationImage = pImageData->GetImageAnimation();
        if (pAnimationImage != nullptr) {
            const int32_t nFrameCount = pAnimationImage->GetFrameCount();
            if (nFrameCount > 1) {
                nCurFrameIndex = pAnimationImage->GetDecodedFrameIndex() + 1;
                if (nCurFrameIndex >= (uint32_t)nFrameCount) {
                    nCurFrameIndex = nFrameCount - 1;
                }
            }
        }
    }
    else {
        //单帧
        nCurFrameIndex = 0;
    }

    //异步解码的函数，在子线程中执行
    auto AsyncDecodeImageFunction = [pAsyncDecoder, nCurFrameIndex, AsyncDecodeImageFinishNotify]() {
            int32_t nUseCount = pAsyncDecoder->m_pImageData.use_count(); //资源引用计数(当计数为1时，表示资源已经释放，不需要再解码)
            if ((nUseCount > 1) &&
                !pAsyncDecoder->m_pImageData->IsAsyncDecodeFinished() &&
                pAsyncDecoder->m_pImageData->IsAsyncDecodeEnabled()) {

                //取消操作判断函数
                auto IsAborted = [pAsyncDecoder]() {
                        if (pAsyncDecoder->m_pImageData.use_count() == 1) {
                            //已经释放：待完善细节
                            return true;
                        }
                        return false;
                    };
                //对图片数据进行异步解码
                pAsyncDecoder->m_bDecodeExecuted = true;
                pAsyncDecoder->m_bDecodeResult = pAsyncDecoder->m_pImageData->AsyncDecode(nCurFrameIndex, IsAborted, &pAsyncDecoder->m_bDecodeError);
            }

            // 通知UI（无论是否执行过图片解码操作，均需要通知UI，
            // 主要目的是让pAsyncDecoder->m_pImageData这个智能指针对象在UI线程中释放，避免在子线程释放导致资源冲突，引发程序崩溃）
            size_t nTaskId = GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, AsyncDecodeImageFinishNotify);
            ASSERT_UNUSED_VARIABLE(nTaskId > 0);
        };

    //放入子线程中，开始解码
    size_t nTaskId = threadManager.PostTask(nThreadIdentifier, AsyncDecodeImageFunction);
    pAsyncDecoder->m_nTaskId = nTaskId;
    pAsyncDecoder->m_pImageData->SetAsyncDecodeTaskId(nTaskId);
}

bool Control::LoadImageInfo(Image& duiImage, bool bPaintImage) const
{
    GlobalManager::Instance().AssertUIThread();
    //DPI缩放百分比
    const uint32_t nLoadDpiScale = Dpi().GetScale();
    if (duiImage.GetImageInfo() != nullptr) {
        //如果图片缓存存在，并且DPI缩放百分比没变化，则不再加载（当图片变化的时候，会清空这个缓存）
        if (duiImage.GetImageInfo()->GetLoadDpiScale() == nLoadDpiScale) {
            return true;
        }        
    }
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return false;
    }

    if (duiImage.HasImageError()) {
        //如果图片加载失败，则不重新加载该图片
        return false;
    }

    DString sImagePath = duiImage.GetImagePath();
    if (duiImage.GetImageAttribute().IsAssertEnabled()) {
        ASSERT(!sImagePath.empty());
    }
    if (sImagePath.empty()) {
        //图片资源路径为空，标记加载失败
        duiImage.SetImageError(true);
        FireImageEvent(&duiImage, sImagePath, true, true, false);
        return false;
    }
    ImageLoadPath imageLoadPath; //图片加载路径信息
    imageLoadPath.m_pathType = ImageLoadPathType::kUnknownPath;    
    IconManager& iconManager = GlobalManager::Instance().Icon();
    if (iconManager.IsIconString(sImagePath)) {
        uint32_t nIconID = iconManager.GetIconID(sImagePath);
        if (iconManager.IsImageString(nIconID)) {
            //资源图片路径（一次性更新，更新后iconManager.IsIconString就是false了）
            DString iconImageString = iconManager.GetImageString(nIconID);
            ASSERT(!iconImageString.empty());
            DString oldImageString = duiImage.GetImageString();
            duiImage.SetImageString(iconImageString, pWindow->Dpi());
            duiImage.UpdateImageAttribute(oldImageString, pWindow->Dpi());
            sImagePath = duiImage.GetImagePath();//更新图片路径为资源指定的路径
            ASSERT(!sImagePath.empty());
            if (sImagePath.empty()) {
                //图片资源路径为空，标记加载失败
                duiImage.SetImageError(true);
                FireImageEvent(&duiImage, sImagePath, true, true, false);
                return false;
            }
        }
        else {
            //ICON图标数据，虚拟路径
            imageLoadPath.m_pathType = ImageLoadPathType::kVirtualPath;
        }
    }
    if (imageLoadPath.m_pathType == ImageLoadPathType::kVirtualPath) {
        //ICON图标数据，虚拟路径
        imageLoadPath.m_imageFullPath = sImagePath;
    }
    else {
        //非图标数据：获取图片资源的完整路径（磁盘绝对路径或者zip压缩包内的相对路径）       
        FilePath resPath(sImagePath);
        bool bLocalPath = true;
        bool bResPath = true;
        FilePath imageFullPath = GlobalManager::Instance().GetExistsResFullPath(pWindow->GetResourcePath(), pWindow->GetXmlPath(), resPath, bLocalPath, bResPath);
        if (!imageFullPath.IsEmpty()) {
            imageLoadPath.m_imageFullPath = imageFullPath.NativePath();
            if (bLocalPath) {
                if (bResPath) {
                    imageLoadPath.m_pathType = ImageLoadPathType::kLocalResPath;
                }
                else {
                    imageLoadPath.m_pathType = ImageLoadPathType::kLocalPath;
                }
            }
            else {
                imageLoadPath.m_pathType = ImageLoadPathType::kZipResPath;
            }
        }
    }
    if (duiImage.GetImageAttribute().IsAssertEnabled()) {
        ASSERT(!imageLoadPath.m_imageFullPath.IsEmpty());
    }
    if (imageLoadPath.m_imageFullPath.IsEmpty()) {
        //图片资源文件不存在, 标记加载失败
        duiImage.SetImageError(true);
        FireImageEvent(&duiImage, imageLoadPath.m_imageFullPath.NativePath(), true, true, false);
        return false;
    }

    ImageLoadParam imageLoadParam = duiImage.GetImageLoadParam();
    imageLoadParam.SetLoadDpiScale(nLoadDpiScale);  //设置加载的DPI百分比
    imageLoadParam.SetImageLoadPath(imageLoadPath); //设置图片资源的路径
    std::shared_ptr<ImageInfo> imageInfo = duiImage.GetImageInfo();
    if ((imageInfo == nullptr) ||
        (imageInfo->GetLoadKey() != imageLoadParam.GetLoadKey(nLoadDpiScale))) {
        //第1种情况：如果图片没有加载则执行加载图片；
        //第2种情况：如果图片发生变化，则重新加载该图片        

        //是否开启图片加载优化(以最小的比例加载图片，占有内存最少，绘制速度最快)，开启条件总结为：
        //1. 仅当绘制时加载图片可以开启该项优化，因为此时加载的图片，改变加载比例时只影响图片的显示效果，并不影响控件和图片的布局
        //2. 如果图片指定了不支持DPI自适应（dpi_scale="false"），那么关闭该项优化
        //3. 如果绘制属性指定为平铺（xtiled="true" 或者 ytiled="true"），那么关闭该项优化
        //4. 如果绘制属性指定为阴影（window_shadow_mode="true"），那么关闭该项优化
        //5. 如果绘制属性指定为自适应（adaptive_dest_rect="true"），那么关闭该项优化
        //6. 如果绘制属性指定为九宫格绘制（corner="left,top,right,bottom"），那么关闭该项优化
        bool bEnableImageLoadSizeOpt = bPaintImage;
        if (duiImage.GetImageAttribute().IsTiledDraw() ||
            duiImage.GetImageAttribute().m_bWindowShadowMode ||
           !duiImage.GetImageAttribute().m_bImageDpiScaleEnabled ||
            duiImage.GetImageAttribute().m_bAdaptiveDestRect ||
            duiImage.GetImageAttribute().HasImageCorner()) {
            bEnableImageLoadSizeOpt = false;
        }
        
        uint32_t nImageSetWidth = 0;
        uint32_t nImageSetHeight = 0;
        if (imageLoadParam.GetImageFixedSize(nImageSetWidth, nImageSetHeight)) {
            //如果图片指定了宽度或者高度(举例:width="100" 或 height="100"这种)，可以在加载时，计算最适合的缩放比，以提高效率，但不会有影响
            imageLoadParam.SetMaxDestRectSize(UiSize((int32_t)nImageSetWidth, (int32_t)nImageSetHeight));
        }
        else if (bEnableImageLoadSizeOpt) {
            //绘制阶段加载的图片，不需要图片宽高来确定目标区域，可做加载优化（对于大图，可以加载一个小图，保证绘制质量的情况下，提高绘制速度，并减少内存占用）
            imageLoadParam.SetMaxDestRectSize(UiSize(GetRect().Width(), GetRect().Height()));
        }

        bool bImageDataFromCache = false;
        imageInfo = GlobalManager::Instance().Image().GetImage(imageLoadParam, bImageDataFromCache);
        duiImage.SetImageInfo(imageInfo);
        if (imageInfo != nullptr) {
            //检查并启动多线程解码，在子线程中解码图片数据
            std::shared_ptr<IImage> pImageData = imageInfo->GetImageData();
            if (pImageData != nullptr) {
                std::shared_ptr<TAsyncImageDecode> pAsyncDecoder = std::make_shared<TAsyncImageDecode>();                
                pAsyncDecoder->m_nFrameCount = imageInfo->GetFrameCount();
                pAsyncDecoder->m_nDecodeCount = 0;
                pAsyncDecoder->m_nTaskId = 0;
                pAsyncDecoder->m_pImageData = std::move(pImageData);
                pAsyncDecoder->m_imageKey = imageInfo->GetImageKey();
                pAsyncDecoder->m_pControl = const_cast<Control*>(this);
                pAsyncDecoder->m_pImage = &duiImage;
                pAsyncDecoder->m_imagePath = imageLoadPath.m_imageFullPath.NativePath();

                if (!bImageDataFromCache) {
                    //重新加载的图片
                    AsyncDecodeImageData(pAsyncDecoder);
                }
                else if (pAsyncDecoder->m_pImageData->IsAsyncDecodeEnabled() &&
                         !pAsyncDecoder->m_pImageData->IsAsyncDecodeFinished()) {
                    //从缓存中获取的图片，但尚未加载
                    AsyncDecodeImageData(pAsyncDecoder);
                }
            }
        }
    }
    if (imageInfo == nullptr) {
        //标记加载失败
        duiImage.SetImageError(true);
    }

    //图片加载结果的回调事件（异步）
    bool bLoadError = (imageInfo == nullptr);
    FireImageEvent(&duiImage, imageLoadPath.m_imageFullPath.NativePath(), true, bLoadError, false);
    return imageInfo ? true : false;
}

void Control::FireImageEvent(Image* pImagePtr, const DString& imageFilePath, bool bLoadImage, bool bLoadError, bool bDecodeError) const
{
    if (pImagePtr == nullptr) {
        return;
    }
    if (!bLoadImage) {
        //标记解码完成事件已经通知
        pImagePtr->SetDecodeEventFired(true);
    }
    ControlPtr pControl(const_cast<Control*>(this));        //图片关联控件
    ControlPtrT<Image> pImage(pImagePtr);                   //图片资源接口

    ImageDecodeResult decodeResult;
    decodeResult.m_pControl = pControl.get();               //图片关联控件
    decodeResult.m_pImage = pImage.get();                   //图片资源接口
    decodeResult.m_imageFilePath = imageFilePath;           //图片路径
    decodeResult.m_imageName = pImage->GetImageName();      //图片名称，唯一ID
    decodeResult.m_bBkImage = (GetBkImagePtr() == pImagePtr);   //该图片是否为背景图片
    decodeResult.m_bLoadError = bLoadError;                     //该图片是否存在加载错误
    decodeResult.m_bDecodeError = bDecodeError;                 //该图片是否存在数据解码错误

    auto LoadImageCallback = [pControl, pImage, bLoadImage, decodeResult]() {
            if ((pControl != nullptr) && (pImage != nullptr)) {
                if (bLoadImage) {
                    pControl->SendEvent(kEventImageLoad, (WPARAM)&decodeResult);
                }
                else {
                    pControl->SendEvent(kEventImageDecode, (WPARAM)&decodeResult);
                }
            }
        };
    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, LoadImageCallback);
}

void Control::ClearImageCache()
{
    if (m_pImageMap != nullptr) {
        m_pImageMap->ClearImageCache();
    }
    if (m_pBkImage != nullptr) {
        m_pBkImage->ClearImageCache();
    }
}

void Control::AttachEvent(EventType type, const EventCallback& callback)
{
    EventMap& attachEventMap = GetAttachEventMap();
    attachEventMap[type] += callback;
    if ((type == kEventContextMenu) || (type == kEventAll)) {
        SetContextMenuUsed(true);
    }
}

void Control::DetachEvent(EventType type)
{
    if (!HasAttachEventMap()) {
        return;
    }
    EventMap& attachEventMap = GetAttachEventMap();
    auto event = attachEventMap.find(type);
    if (event != attachEventMap.end()) {
        attachEventMap.erase(event);
    }
    if ((type == kEventContextMenu) || (type == kEventAll)) {
        if ((attachEventMap.find(kEventAll) == attachEventMap.end()) &&
            (attachEventMap.find(kEventContextMenu) == attachEventMap.end())) {
            SetContextMenuUsed(false);
        }
    }
}

void Control::AttachXmlEvent(EventType eventType, const EventCallback& callback)
{
    EventMap& xmlEventMap = GetXmlEventMap();
    xmlEventMap[eventType] += callback;
}

void Control::DetachXmlEvent(EventType type)
{
    if (!HasXmlEventMap()) {
        return;
    }
    EventMap& xmlEventMap = GetXmlEventMap();
    auto event = xmlEventMap.find(type);
    if (event != xmlEventMap.end()) {
        xmlEventMap.erase(event);
    }
}

void Control::AttachBubbledEvent(EventType eventType, const EventCallback& callback)
{
    EventMap& bubbledEventMap = GetBubbledEventMap();
    bubbledEventMap[eventType] += callback;
}

void Control::DetachBubbledEvent(EventType eventType)
{
    if (!HasBubbledEventMap()) {
        return;
    }
    EventMap& bubbledEventMap = GetBubbledEventMap();
    auto event = bubbledEventMap.find(eventType);
    if (event != bubbledEventMap.end()) {
        bubbledEventMap.erase(eventType);
    }
}

void Control::AttachXmlBubbledEvent(EventType eventType, const EventCallback& callback)
{
    EventMap& xmlBubbledEventMap = GetXmlBubbledEventMap();
    xmlBubbledEventMap[eventType] += callback;
}

void Control::DetachXmlBubbledEvent(EventType eventType)
{
    if (!HasXmlBubbledEventMap()) {
        return;
    }
    EventMap& xmlBubbledEventMap = GetXmlBubbledEventMap();
    auto event = xmlBubbledEventMap.find(eventType);
    if (event != xmlBubbledEventMap.end())    {
        xmlBubbledEventMap.erase(eventType);
    }
}

bool Control::FireAllEvents(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
    std::weak_ptr<WeakFlag> weakflag = GetWeakFlag();
    bool bRet = true;//当值为false时，就不再调用回调函数和处理函数

    if (msg.GetSender() == this) {
        //备注：EventMap 和 XmlEventMap里面的回调函数，需要校验消息的发送者是否为控件自身
        if (bRet && HasAttachEventMap() && !GetAttachEventMap().empty()) {
            const EventMap& attachEventMap = GetAttachEventMap();
            auto callback = attachEventMap.find(msg.eventType);
            if (callback != attachEventMap.end()) {
                bRet = callback->second(msg);
            }
            if (weakflag.expired() || msg.IsSenderExpired()) {
                return false;
            }

            callback = attachEventMap.find(kEventAll);
            if (callback != attachEventMap.end()) {
                bRet = callback->second(msg);
            }
            if (weakflag.expired() || msg.IsSenderExpired()) {
                return false;
            }
        }

        if (bRet && HasXmlEventMap() && !GetXmlEventMap().empty()) {
            const EventMap& xmlEventMap = GetXmlEventMap();
            auto callback = xmlEventMap.find(msg.eventType);
            if (callback != xmlEventMap.end()) {
                bRet = callback->second(msg);
            }
            if (weakflag.expired() || msg.IsSenderExpired()) {
                return false;
            }

            callback = xmlEventMap.find(kEventAll);
            if (callback != xmlEventMap.end()) {
                bRet = callback->second(msg);
            }
            if (weakflag.expired() || msg.IsSenderExpired()) {
                return false;
            }
        }
    }

    //备注：BubbledEventMap 和 XmlBubbledEventMap里面的回调函数，不需要校验消息的发送者是否为控件自身
    if (bRet && HasBubbledEventMap() && !GetBubbledEventMap().empty()) {
        const EventMap& bubbledEventMap = GetBubbledEventMap();
        auto callback = bubbledEventMap.find(msg.eventType);
        if (callback != bubbledEventMap.end()) {
            bRet = callback->second(msg);
        }
        if (weakflag.expired() || msg.IsSenderExpired()) {
            return false;
        }

        callback = bubbledEventMap.find(kEventAll);
        if (callback != bubbledEventMap.end()) {
            bRet = callback->second(msg);
        }
        if (weakflag.expired() || msg.IsSenderExpired()) {
            return false;
        }
    }

    if (bRet && HasXmlBubbledEventMap() && !GetXmlBubbledEventMap().empty()) {
        const EventMap& xmlBubbledEventMap = GetXmlBubbledEventMap();
        auto callback = xmlBubbledEventMap.find(msg.eventType);
        if (callback != xmlBubbledEventMap.end()) {
            bRet = callback->second(msg);
        }
        if (weakflag.expired() || msg.IsSenderExpired()) {
            return false;
        }

        callback = xmlBubbledEventMap.find(kEventAll);
        if (callback != xmlBubbledEventMap.end()) {
            bRet = callback->second(msg);
        }
        if (weakflag.expired() || msg.IsSenderExpired()) {
            return false;
        }
    }
    return bRet && !weakflag.expired();
}

bool Control::HasEventCallback(EventType eventType) const
{
    if (m_pEventMapData == nullptr) {
        return false;
    }
    if (!m_pEventMapData->m_attachEvent.empty()) {
        const EventMap& eventMap = m_pEventMapData->m_attachEvent;
        if (eventMap.find(eventType) != eventMap.end()) {
            return true;
        }
    }
    if (m_pEventMapData->m_pXmlEvent != nullptr){
        const EventMap& eventMap = *m_pEventMapData->m_pXmlEvent;
        if (!eventMap.empty() && eventMap.find(eventType) != eventMap.end()) {
            return true;
        }
    }
    if (m_pEventMapData->m_pBubbledEvent != nullptr) {
        const EventMap& eventMap = *m_pEventMapData->m_pBubbledEvent;
        if (!eventMap.empty() && eventMap.find(eventType) != eventMap.end()) {
            return true;
        }
    }
    if (m_pEventMapData->m_pXmlBubbledEvent != nullptr) {
        const EventMap& eventMap = *m_pEventMapData->m_pXmlBubbledEvent;
        if (!eventMap.empty() && eventMap.find(eventType) != eventMap.end()) {
            return true;
        }
    }
    return false;
}

bool Control::HasUiColor(const DString& colorName) const
{
    if (colorName.empty()) {
        return false;
    }
    UiColor color = GetUiColorByName(colorName);
    return color.GetARGB() != 0;
}

UiColor Control::GetUiColor(const DString& colorName) const
{
    if (colorName.empty()) {
        return UiColor();
    }
    UiColor color = GetUiColorByName(colorName);
    ASSERT(!color.IsEmpty());
    return color;
}

UiColor Control::GetUiColorByName(const DString& colorName) const
{
    UiColor color;
    if (colorName.empty()) {
        return color;
    }
    if (colorName.at(0) == _T('#')) {
        //优先级1：以'#'字符开头，直接指定颜色值，举例：#FFFFFFFF
        color = ColorManager::ConvertToUiColor(colorName);
    }
    if (color.GetARGB() == 0) {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            //优先级2：获取在配置XML中的<Window>节点中定义子节点<TextColor>指定的颜色
            color = pWindow->GetTextColor(colorName);
        }
    }
    if (color.GetARGB() == 0) {
        //优先级3：获取在global.xml中的<Global>节点中定义子节点<TextColor>指定的颜色
        color = GlobalManager::Instance().Color().GetColor(colorName);
    }
    if (color.GetARGB() == 0) {
        //优先级4：直接指定预定义的颜色别名
        color = GlobalManager::Instance().Color().GetStandardColor(colorName);
    }
    ASSERT(color.GetARGB() != 0);
    return color;
}

DString Control::GetColorString(const UiColor& color) const
{
    if (color.IsEmpty()) {
        return DString();
    }
    else {
        return StringUtil::Printf(_T("#%02X%02X%02X%02X"), color.GetA(), color.GetR(), color.GetG(), color.GetB());
    }
}

bool Control::HasBoxShadow() const
{
    if ((m_pOtherData != nullptr) && (m_pOtherData->m_pBoxShadow != nullptr)) {
        return m_pOtherData->m_pBoxShadow->HasShadow();
    }
    return false;
}

UiRect Control::GetBoxShadowExpandedRect(const UiRect& rc) const
{
    if ((m_pOtherData != nullptr) &&
        (m_pOtherData->m_pBoxShadow != nullptr) &&
         m_pOtherData->m_pBoxShadow->HasShadow()) {
        return m_pOtherData->m_pBoxShadow->GetExpandedRect(rc);
    }
    return rc;
}

bool Control::IsSelectableType() const
{
    return false;
}

bool Control::IsWantTab() const
{
    return false;
}

bool Control::CanPlaceCaptionBar() const
{
    return false;
}

bool Control::CheckVisibleAncestor(void) const
{
    bool isVisible = IsVisible();
    if (isVisible) {
        Control* parent = GetParent();
        while (parent != nullptr)
        {
            if (!parent->IsVisible()) {
                isVisible = false;
                break;
            }
            parent = parent->GetParent();
        }
    }
    return isVisible;
}

bool Control::IsKeyDown(const EventArgs& msg, ModifierKey modifierKey) const
{
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        return pWindow->IsKeyDown(msg, modifierKey);
    }
    return false;
}

void Control::EnsureNoFocus()
{
    Window* pWindow = GetWindow();
    if ((pWindow != nullptr) && pWindow->GetFocusControl() != nullptr) {
        if (pWindow->GetFocusControl() == this) {
            pWindow->SetFocusControl(nullptr);
        }
    }
}

bool Control::ScreenToClient(UiPoint& pt)
{
    Window* pWindow = GetWindow();
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return false;
    }
    pWindow->ScreenToClient(pt);
    return true;
}

bool Control::ClientToScreen(UiPoint& pt)
{
    Window* pWindow = GetWindow();
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return false;
    }
    pWindow->ClientToScreen(pt);
    return true;
}

void Control::SetPaintOrder(uint8_t nPaintOrder)
{
    m_nPaintOrder = nPaintOrder;
}

uint8_t Control::GetPaintOrder() const
{
    return m_nPaintOrder;
}

IFont* Control::GetIFontById(const DString& strFontId) const
{
    return GlobalManager::Instance().Font().GetIFont(strFontId, this->Dpi());
}

bool Control::HasDestroyEventCallback() const
{
    if (m_pEventMapData == nullptr) {
        return false;
    }
    const EventMap& eventMap = m_pEventMapData->m_attachEvent;
    return eventMap.find(kEventDestroy) != eventMap.end();
}

EventMap& Control::GetAttachEventMap()
{
    if (m_pEventMapData == nullptr) {
        m_pEventMapData = std::make_unique<TEventMapData>();
    }
    return m_pEventMapData->m_attachEvent;
}

bool Control::HasAttachEventMap() const
{
    return m_pEventMapData != nullptr;
}

EventMap& Control::GetXmlEventMap()
{
    if (m_pEventMapData == nullptr) {
        m_pEventMapData = std::make_unique<TEventMapData>();
    }
    if (m_pEventMapData->m_pXmlEvent == nullptr) {
        m_pEventMapData->m_pXmlEvent = new EventMap;
    }
    return *m_pEventMapData->m_pXmlEvent;
}

bool Control::HasXmlEventMap() const
{
    return (m_pEventMapData != nullptr) && (m_pEventMapData->m_pXmlEvent != nullptr);
}

EventMap& Control::GetBubbledEventMap()
{
    if (m_pEventMapData == nullptr) {
        m_pEventMapData = std::make_unique<TEventMapData>();
    }
    if (m_pEventMapData->m_pBubbledEvent == nullptr) {
        m_pEventMapData->m_pBubbledEvent = new EventMap;
    }
    return *m_pEventMapData->m_pBubbledEvent;
}

bool Control::HasBubbledEventMap() const
{
    return (m_pEventMapData != nullptr) && (m_pEventMapData->m_pBubbledEvent != nullptr);
}

EventMap& Control::GetXmlBubbledEventMap()
{
    if (m_pEventMapData == nullptr) {
        m_pEventMapData = std::make_unique<TEventMapData>();
    }
    if (m_pEventMapData->m_pXmlBubbledEvent == nullptr) {
        m_pEventMapData->m_pXmlBubbledEvent = new EventMap;
    }
    return *m_pEventMapData->m_pXmlBubbledEvent;
}

bool Control::HasXmlBubbledEventMap() const
{
    return (m_pEventMapData != nullptr) && (m_pEventMapData->m_pXmlBubbledEvent != nullptr);
}

void Control::SetEnableDragDrop(bool bEnable)
{
    if (m_pDragDropData == nullptr) {
        m_pDragDropData = std::make_unique<TDragDropData>();
    }
    m_pDragDropData->m_bDragDropEnabled = bEnable;
}

bool Control::IsEnableDragDrop() const
{
    return (m_pDragDropData != nullptr) && m_pDragDropData->m_bDragDropEnabled;
}

void Control::SetEnableDropFile(bool bEnable)
{
    if (m_pDragDropData == nullptr) {
        m_pDragDropData = std::make_unique<TDragDropData>();
    }
    m_pDragDropData->m_bDropFileEnabled = bEnable;
    m_pDragDropData->m_bDropFileEnabledDefined = true;
}

bool Control::IsEnableDropFile() const
{
    if (m_pDragDropData != nullptr) {
        if (m_pDragDropData->m_bDropFileEnabledDefined) {
            return m_pDragDropData->m_bDropFileEnabled;
        }
        else {
            return m_pDragDropData->m_bDragDropEnabled;
        }
    }
    return false;
}

void Control::SetDropFileTypes(const DString& fileTypes)
{
    if (m_pDragDropData == nullptr) {
        m_pDragDropData = std::make_unique<TDragDropData>();
    }
    m_pDragDropData->m_dropFileTypes = fileTypes;
}

DString Control::GetDropFileTypes() const
{
    DString fileTypes;
    if (m_pDragDropData != nullptr) {
        fileTypes = m_pDragDropData->m_dropFileTypes.c_str();
    }
    return fileTypes;
}

ControlDropTarget_Windows* Control::GetControlDropTarget()
{
#ifdef DUILIB_BUILD_FOR_WIN
    if (IsEnableDragDrop() && IsEnabled()) {
        if (m_pDragDropData != nullptr) {
            m_pDragDropData = std::make_unique<TDragDropData>();
            m_pDragDropData->m_bDragDropEnabled = true;
        }
        m_pDragDropData->m_pDropTargetWindows = std::make_shared<ControlDropTargetImpl_Windows>(this);
        return m_pDragDropData->m_pDropTargetWindows.get();
    }
#endif
    return nullptr;
}

ControlDropTarget_SDL* Control::GetControlDropTarget_SDL()
{
#ifdef DUILIB_BUILD_FOR_SDL
    if (IsEnableDragDrop() && IsEnabled()) {
        if (m_pDragDropData == nullptr) {
            m_pDragDropData = std::make_unique<TDragDropData>();
            m_pDragDropData->m_bDragDropEnabled = true;
        }
        m_pDragDropData->m_pDropTargetSDL = std::make_shared<ControlDropTargetImpl_SDL>(this);
        return m_pDragDropData->m_pDropTargetSDL.get();
    }
#endif
    return nullptr;
}

} // namespace ui
