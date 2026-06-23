#include "ColorPickerStandard.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

ColorPickerStandard::ColorPickerStandard(Window* pWindow):
    Control(pWindow),
    m_radius(0)
{
    InitColorMap();
}

DString ColorPickerStandard::GetType() const { return DUI_CTR_COLOR_PICKER_STANDARD; }

void ColorPickerStandard::SelectColor(const UiColor& color)
{
    m_selectedColor = color;
    Invalidate();
}

void ColorPickerStandard::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }
        
    UiRect rect = GetRect();
    UiPadding rcPadding = this->GetControlPadding(); //内边距
    rect.Deflate(rcPadding);
    DrawColorMap(pRender, rect);
}

void ColorPickerStandard::DrawColorMap(IRender* pRender, const UiRect& rect)
{
    int32_t rectSize = std::min(rect.Width(), rect.Height());
    const int32_t radius = static_cast<int32_t>(rectSize / 13 / 2 / std::cos(30 / 57.2957795f)); //半径
    m_radius = radius;
    const float distance = radius * std::cos(30 / 57.2957795f); //中心点到边的垂直距离

    UiPointF firstCenterPt = UiPointF((float)rect.CenterX(), (float)rect.CenterY()); //矩形中心点坐标
    firstCenterPt.x = firstCenterPt.x - distance * 2 * 6 * std::sin(30 / 57.2957795f); //第一个六边形中心点X坐标
    firstCenterPt.y = firstCenterPt.y - distance * 2 * 6 * std::cos(30 / 57.2957795f); //第一个六边形中心点Y坐标

    //当前选择的图像绘制参数
    const UiColor penColor = UiColor(UiColors::Orange);
    const float penWidth = Dpi().GetScaleFloat(2);
    UiPointF selectCenterPt;
    UiColor selectBrushColor;

    // 总共 13 行六边形，最大颜色数 = 7+8+9+10+11+12+13+12+11+10+9+8+7 = 127
    // m_colorMap 应与实际六边形数一致
    const size_t totalHexagons = 127;
    if (m_colorMap.size() != totalHexagons) {
        // 颜色表大小不对，重新初始化
        InitColorMap();
    }

    size_t colorIndex = 0;
    for (int32_t y = 0; y < 13; ++y) { //共计13行
        int32_t count = 0;
        if (y < 7) {
            count = 7 + y;
        }
        else {
            count = 7 + (13 - y - 1);
        }
        for (int32_t x = 0; x < count; ++x) {
            UiPointF centerPt = firstCenterPt;
            if (y < 7) {
                centerPt.x += distance * 2 * x - distance * y;
            }
            else {
                centerPt.x += distance * 2 * x - distance * (13 - y - 1);
            }
            centerPt.y += radius * 1.5f * y;
            
            bool bSelected = false;
            UiColor brushColor = UiColor(UiColors::Salmon);
            if (colorIndex < m_colorMap.size()) {
                brushColor = m_colorMap[colorIndex].color;
            }
            // 始终更新 centerPt（无论 m_colorMap 是否有对应项）
            if (colorIndex < m_colorMap.size()) {
                m_colorMap[colorIndex].centerPt = centerPt;
                if (m_selectedColor == brushColor) {
                    //当前选择的颜色，边框加粗显示
                    bSelected = true;
                    selectCenterPt = centerPt;
                    selectBrushColor = brushColor;
                }
            }
            if (!bSelected) {
                DrawRegularHexagon(pRender, centerPt, radius, UiColor(), 0.0f, brushColor);
            }            
            ++colorIndex;
        }
    }

    //画选择的六边形（为了避免选择边框被后面画的内容覆盖，影响美观度）
    if (!selectBrushColor.IsEmpty()) {
        DrawRegularHexagon(pRender, selectCenterPt, radius, penColor, penWidth, selectBrushColor);
    }
}

bool ColorPickerStandard::DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
                                            const UiColor& penColor, float penWidth, const UiColor& brushColor)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    ASSERT(radius > 0); //多边形的半径
    if (radius <= 0) {
        return false;
    }

    constexpr const int32_t count = 6; //多边形的边数
    //正多边形上任意一个顶点的坐标为： x = r * cos(θ) y = r * sin(θ) 
    std::vector<UiPointF> polygonPoints;
    for (int32_t i = 0; i < count; ++i) {
        int32_t degree = i * 60 + 30;// +30是为了使顶点在中心点的最上方
        float radian = degree / 57.2957795f;
        float x = radius * std::cos(radian) + 0.5f;
        float y = radius * std::sin(radian) + 0.5f;
        polygonPoints.push_back(UiPointF(centerPt.x + x, centerPt.y + y));
    }

    std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
    path->AddPolygon(polygonPoints.data(), (int32_t)polygonPoints.size());
    path->Close();

    bool bRet = false;
    if (!brushColor.IsEmpty()) {
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
        pRender->FillPath(path.get(), brush.get());
        bRet = true;
    }
    if (!penColor.IsEmpty() && (penWidth > 0.1f)) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(penColor, penWidth));
        pRender->DrawPath(path.get(), pen.get());
        bRet = true;
    }
    return bRet;
}

bool ColorPickerStandard::MouseMove(const EventArgs& msg)
{
    //更新ToolTip信息
    if (GetRect().ContainsPt(msg.ptMouse)) {
        UiColor color;
        DString colorNameId;
        if (GetColorInfo(msg.ptMouse, color, colorNameId)) {
            DString colorStringTip = GlobalManager::GetTextById(colorNameId) ;
            colorStringTip += _T("(");
            colorStringTip += StringUtil::Printf(_T("#%02X%02X%02X%02X"), color.GetA(), color.GetR(), color.GetG(), color.GetB());
            colorStringTip += _T(")");
            SetToolTipText(colorStringTip);
        }
        else {
            SetToolTipText(_T(""));
        }
    }
    return BaseClass::MouseMove(msg);
}

bool ColorPickerStandard::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (GetRect().ContainsPt(msg.ptMouse)) {
        UiColor color;
        DString colorNameId;
        if (GetColorInfo(msg.ptMouse, color, colorNameId)) {
            //选择了当前的颜色
            m_selectedColor = color;
            Invalidate();
            SendEvent(kEventSelectColor, color.GetARGB());
        }
        else {
            Invalidate();
        }
    }
    return bRet;
}

bool ColorPickerStandard::GetColorInfo(const UiPoint& ptMouse, UiColor& ptColor, DString& colorNameId) const
{
    struct ColorPt
    {
        //颜色值
        UiColor color;

        //颜色名称ID
        DString colorNameId;

        //该颜色值对应的正六边形中心点与ptMouse之间的距离
        float distance;

        //比较函数，距离最近的排在前面
        bool operator < (const ColorPt& r) const
        {
            return distance < r.distance;
        }
    };

    std::vector<ColorPt> maybeColors;
    for (const ColorInfo& colorInfo : m_colorMap) {
        float distance = GetPointsDistance(colorInfo.centerPt,
                                            UiPointF((float)ptMouse.x, (float)ptMouse.y));
        if (distance <= m_radius) {
            maybeColors.push_back({ colorInfo.color, colorInfo.colorNameId.c_str(), distance});
        }
    }

    if (maybeColors.empty()) {
        return false;
    }
    else {
        std::sort(maybeColors.begin(), maybeColors.end()); //选取鼠标点距离正六边形中心点最近的那个颜色
        ptColor = maybeColors.front().color;
        colorNameId = maybeColors.front().colorNameId;
        return true;
    }        
}

float ColorPickerStandard::GetPointsDistance(const UiPointF& pt1, const UiPointF& pt2) const
{
    float a = std::abs(pt1.x - pt2.x);
    float b = std::abs(pt1.y - pt2.y);
    float c = sqrtf(a * a + b * b);
    return c;
}

void ColorPickerStandard::InitColorMap()
{
    m_colorMap = {
        {UiColor(0xFF003366), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_BLUE_1"), UiPointF()},          // 深蓝1
        {UiColor(0xFF336699), _T("STRID_PUBLIC_COLORPICKER_TIP3_BLUE_GRAY_1"), UiPointF()},          // 蓝灰1
        {UiColor(0xFF3366CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_MID_BLUE_1"), UiPointF()},           // 中蓝1
        {UiColor(0xFF003399), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_BLUE_2"), UiPointF()},          // 深蓝2
        {UiColor(0xFF000099), _T("STRID_PUBLIC_COLORPICKER_TIP3_NAVY_BLUE_1"), UiPointF()},          // 藏青1
        {UiColor(0xFF0000CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_NAVY_BLUE_2"), UiPointF()},          // 藏青2
        {UiColor(0xFF000066), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_NAVY"), UiPointF()},            // 深藏青
        {UiColor(0xFF006666), _T("STRID_PUBLIC_COLORPICKER_TIP3_TEAL_1"), UiPointF()},               // 青绿色1
        {UiColor(0xFF006699), _T("STRID_PUBLIC_COLORPICKER_TIP3_CYAN_BLUE_1"), UiPointF()},          // 青蓝1
        {UiColor(0xFF0099CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_SKY_BLUE_1"), UiPointF()},           // 天蓝1
        {UiColor(0xFF0066CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_BLUE_1"), UiPointF()},        // 亮蓝1
        {UiColor(0xFF0033CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_VIVID_BLUE_1"), UiPointF()},         // 鲜蓝1
        {UiColor(0xFF0000FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_BLUE"), UiPointF()},             // 纯蓝
        {UiColor(0xFF3333FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_BLUE_1"), UiPointF()},         // 浅蓝1
        {UiColor(0xFF333399), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_LAVENDER_1"), UiPointF()},      // 深薰衣草紫1
        {UiColor(0xFF669999), _T("STRID_PUBLIC_COLORPICKER_TIP3_GRAY_TEAL"), UiPointF()},            // 灰青色
        {UiColor(0xFF009999), _T("STRID_PUBLIC_COLORPICKER_TIP3_TEAL_2"), UiPointF()},               // 青绿色2
        {UiColor(0xFF33CCCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_CYAN_1"), UiPointF()},         // 浅青1
        {UiColor(0xFF00CCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_CYAN_1"), UiPointF()},               // 青色1
        {UiColor(0xFF0099FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_SKY_BLUE_2"), UiPointF()},           // 天蓝2
        {UiColor(0xFF0066FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_BLUE_2"), UiPointF()},        // 亮蓝2
        {UiColor(0xFF3366FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_BLUE_2"), UiPointF()},         // 浅蓝2
        {UiColor(0xFF3333CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_LAVENDER_2"), UiPointF()},      // 深薰衣草紫2
        {UiColor(0xFF666699), _T("STRID_PUBLIC_COLORPICKER_TIP3_SLATE_GRAY"), UiPointF()},           // 石板灰
        {UiColor(0xFF339966), _T("STRID_PUBLIC_COLORPICKER_TIP3_MINT_GREEN_1"), UiPointF()},         // 薄荷绿1
        {UiColor(0xFF00CC99), _T("STRID_PUBLIC_COLORPICKER_TIP3_TURQUOISE_1"), UiPointF()},          // 绿松石1
        {UiColor(0xFF00FFCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_TURQUOISE_2"), UiPointF()},          // 绿松石2
        {UiColor(0xFF00FFFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_CYAN"), UiPointF()},            // 纯青
        {UiColor(0xFF33CCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_CYAN_2"), UiPointF()},         // 浅青2
        {UiColor(0xFF3399FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_BLUE_3"), UiPointF()},         // 浅蓝3
        {UiColor(0xFF6699FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_BLUE_1"), UiPointF()},          // 淡蓝1
        {UiColor(0xFF6666FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_BLUE_2"), UiPointF()},          // 淡蓝2
        {UiColor(0xFF6600FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_VIOLET_1"), UiPointF()},             // 紫罗兰1
        {UiColor(0xFF6600CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_VIOLET_1"), UiPointF()},        // 深紫罗兰1
        {UiColor(0xFF339933), _T("STRID_PUBLIC_COLORPICKER_TIP3_MID_GREEN_1"), UiPointF()},          // 中绿1
        {UiColor(0xFF00CC66), _T("STRID_PUBLIC_COLORPICKER_TIP3_MINT_GREEN_2"), UiPointF()},         // 薄荷绿2
        {UiColor(0xFF00FF99), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_GREEN_1"), UiPointF()},        // 浅绿1
        {UiColor(0xFF66FFCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_1"), UiPointF()},         // 淡绿1
        {UiColor(0xFF66FFFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_CYAN"), UiPointF()},            // 淡青
        {UiColor(0xFF66CCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_BLUE_3"), UiPointF()},          // 淡蓝3
        {UiColor(0xFF99CCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_SKY_BLUE_3"), UiPointF()},           // 天蓝3
        {UiColor(0xFF9999FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LAVENDER_1"), UiPointF()},           // 薰衣草紫1
        {UiColor(0xFF9966FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LILAC_1"), UiPointF()},              // 丁香紫1
        {UiColor(0xFF9933FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LILAC_2"), UiPointF()},              // 丁香紫2
        {UiColor(0xFF9900FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURPLE_1"), UiPointF()},             // 紫色1
        {UiColor(0xFF006600), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_GREEN_1"), UiPointF()},         // 深绿1
        {UiColor(0xFF00CC00), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_GREEN_1"), UiPointF()},       // 亮绿1
        {UiColor(0xFF00FF00), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_GREEN"), UiPointF()},           // 纯绿
        {UiColor(0xFF66FF99), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_2"), UiPointF()},         // 淡绿2
        {UiColor(0xFF99FFCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_3"), UiPointF()},         // 淡绿3
        {UiColor(0xFFCCFFFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_CYAN_2"), UiPointF()},          // 淡青2
        {UiColor(0xFFCCCCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_LAVENDER"), UiPointF()},        // 淡薰衣草紫
        {UiColor(0xFFCC99FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LILAC_3"), UiPointF()},              // 丁香紫3
        {UiColor(0xFFCC66FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_LILAC_4"), UiPointF()},              // 丁香紫4
        {UiColor(0xFFCC33FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURPLE_2"), UiPointF()},             // 紫色2
        {UiColor(0xFFCC00FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_MAGENTA_1"), UiPointF()},            // 品红1
        {UiColor(0xFF9900CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_PURPLE_1"), UiPointF()},        // 深紫1
        {UiColor(0xFF003300), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_GREEN_2"), UiPointF()},         // 深绿2
        {UiColor(0xFF009933), _T("STRID_PUBLIC_COLORPICKER_TIP3_MID_GREEN_2"), UiPointF()},          // 中绿2
        {UiColor(0xFF33CC33), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_GREEN_2"), UiPointF()},       // 亮绿2
        {UiColor(0xFF66FF66), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_GREEN_2"), UiPointF()},        // 浅绿2
        {UiColor(0xFF99FF99), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_4"), UiPointF()},         // 淡绿4
        {UiColor(0xFFCCFFCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_5"), UiPointF()},         // 淡绿5
        {UiColor(0xFFFFFFFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_WHITE"), UiPointF()},           // 纯白
        {UiColor(0xFFFFCCFF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_PINK_1"), UiPointF()},          // 淡粉1
        {UiColor(0xFFFF99FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_PINK_2"), UiPointF()},          // 淡粉2
        {UiColor(0xFFFF66FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PINK_1"), UiPointF()},               // 粉色1
        {UiColor(0xFFFF00FF), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_MAGENTA"), UiPointF()},        // 纯品红
        {UiColor(0xFFCC00CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_MAGENTA_2"), UiPointF()},            // 品红2
        {UiColor(0xFF660066), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_MAGENTA"), UiPointF()},         // 深品红
        {UiColor(0xFF336600), _T("STRID_PUBLIC_COLORPICKER_TIP3_OLIVE_GREEN_1"), UiPointF()},        // 橄榄绿1
        {UiColor(0xFF009900), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_GREEN_3"), UiPointF()},         // 深绿3
        {UiColor(0xFF66FF33), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_GREEN_3"), UiPointF()},        // 浅绿3
        {UiColor(0xFF99FF66), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_6"), UiPointF()},         // 淡绿6
        {UiColor(0xFFCCFF99), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_7"), UiPointF()},         // 淡绿7
        {UiColor(0xFFFFFFCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_YELLOW_1"), UiPointF()},        // 淡黄1
        {UiColor(0xFFFFCCCC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_PINK_3"), UiPointF()},          // 淡粉3
        {UiColor(0xFFFF99CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PINK_2"), UiPointF()},               // 粉色2
        {UiColor(0xFFFF66CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_PINK_3"), UiPointF()},               // 粉色3
        {UiColor(0xFFFF33CC), _T("STRID_PUBLIC_COLORPICKER_TIP3_HOT_PINK_1"), UiPointF()},           // 艳粉1
        {UiColor(0xFFCC0099), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_PINK_1"), UiPointF()},          // 深粉1
        {UiColor(0xFF993399), _T("STRID_PUBLIC_COLORPICKER_TIP3_MAUVE_1"), UiPointF()},              // 紫红1
        {UiColor(0xFF333300), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_OLIVE"), UiPointF()},           // 深橄榄
        {UiColor(0xFF669900), _T("STRID_PUBLIC_COLORPICKER_TIP3_OLIVE_GREEN_2"), UiPointF()},        // 橄榄绿2
        {UiColor(0xFF99FF33), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_GREEN_4"), UiPointF()},        // 浅绿4
        {UiColor(0xFFCCFF66), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_GREEN_8"), UiPointF()},         // 淡绿8
        {UiColor(0xFFFFFF99), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_YELLOW_2"), UiPointF()},        // 淡黄2
        {UiColor(0xFFFFCC99), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_ORANGE_1"), UiPointF()},        // 淡橙1
        {UiColor(0xFFFF9999), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_RED_1"), UiPointF()},           // 淡红1
        {UiColor(0xFFFF6699), _T("STRID_PUBLIC_COLORPICKER_TIP3_PINK_4"), UiPointF()},               // 粉色4
        {UiColor(0xFFFF3399), _T("STRID_PUBLIC_COLORPICKER_TIP3_HOT_PINK_2"), UiPointF()},           // 艳粉2
        {UiColor(0xFFCC3399), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_PINK_2"), UiPointF()},          // 深粉2
        {UiColor(0xFF990099), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_MAUVE"), UiPointF()},           // 深紫红
        {UiColor(0xFF666633), _T("STRID_PUBLIC_COLORPICKER_TIP3_OLIVE_GRAY"), UiPointF()},           // 橄榄灰
        {UiColor(0xFF99CC00), _T("STRID_PUBLIC_COLORPICKER_TIP3_YELLOW_GREEN_1"), UiPointF()},       // 黄绿1
        {UiColor(0xFFCCFF33), _T("STRID_PUBLIC_COLORPICKER_TIP3_LIGHT_GREEN_5"), UiPointF()},        // 浅绿5
        {UiColor(0xFFFFFF66), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_YELLOW_3"), UiPointF()},        // 淡黄3
        {UiColor(0xFFFFCC66), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_YELLOW_4"), UiPointF()},        // 淡黄4
        {UiColor(0xFFFF9966), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_ORANGE_2"), UiPointF()},        // 淡橙2
        {UiColor(0xFFFF6666), _T("STRID_PUBLIC_COLORPICKER_TIP3_PALE_RED_2"), UiPointF()},           // 淡红2
        {UiColor(0xFFFF0066), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_PINK"), UiPointF()},          // 亮粉
        {UiColor(0xFFCC6699), _T("STRID_PUBLIC_COLORPICKER_TIP3_MAUVE_2"), UiPointF()},              // 紫红2
        {UiColor(0xFF993366), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_MAUVE_2"), UiPointF()},         // 深紫红2
        {UiColor(0xFF999966), _T("STRID_PUBLIC_COLORPICKER_TIP3_YELLOW_GRAY"), UiPointF()},          // 黄灰
        {UiColor(0xFFCCCC00), _T("STRID_PUBLIC_COLORPICKER_TIP3_YELLOW_GREEN_2"), UiPointF()},       // 黄绿2
        {UiColor(0xFFFFFF00), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_YELLOW"), UiPointF()},          // 纯黄
        {UiColor(0xFFFFCC00), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_YELLOW_1"), UiPointF()},      // 亮黄1
        {UiColor(0xFFFF9933), _T("STRID_PUBLIC_COLORPICKER_TIP3_ORANGE_1"), UiPointF()},             // 橙色1
        {UiColor(0xFFFF6600), _T("STRID_PUBLIC_COLORPICKER_TIP3_ORANGE_2"), UiPointF()},             // 橙色2
        {UiColor(0xFFFF5050), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_RED_1"), UiPointF()},         // 亮红1
        {UiColor(0xFFCC0066), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_PINK_3"), UiPointF()},          // 深粉3
        {UiColor(0xFF660033), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_RED_PURPLE"), UiPointF()},      // 深红紫
        {UiColor(0xFF996633), _T("STRID_PUBLIC_COLORPICKER_TIP3_BROWN_1"), UiPointF()},              // 棕色1
        {UiColor(0xFFCC9900), _T("STRID_PUBLIC_COLORPICKER_TIP3_GOLDEN_YELLOW"), UiPointF()},        // 金黄色
        {UiColor(0xFFFF9900), _T("STRID_PUBLIC_COLORPICKER_TIP3_ORANGE_3"), UiPointF()},             // 橙色3
        {UiColor(0xFFCC6600), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_ORANGE_1"), UiPointF()},        // 深橙1
        {UiColor(0xFFFF3300), _T("STRID_PUBLIC_COLORPICKER_TIP3_BRIGHT_ORANGE"), UiPointF()},        // 亮橙
        {UiColor(0xFFFF0000), _T("STRID_PUBLIC_COLORPICKER_TIP3_PURE_RED"), UiPointF()},             // 纯红
        {UiColor(0xFFCC0000), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_RED_1"), UiPointF()},           // 深红1
        {UiColor(0xFF990033), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_RED_2"), UiPointF()},           // 深红2
        {UiColor(0xFF663300), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_BROWN_1"), UiPointF()},         // 深棕1
        {UiColor(0xFF996600), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_YELLOW_1"), UiPointF()},        // 深黄1
        {UiColor(0xFFCC3300), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_ORANGE_2"), UiPointF()},        // 深橙2
        {UiColor(0xFF993300), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_BROWN_2"), UiPointF()},         // 深棕2
        {UiColor(0xFF990000), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_RED_3"), UiPointF()},           // 深红3
        {UiColor(0xFF800000), _T("STRID_PUBLIC_COLORPICKER_TIP3_MAROON"), UiPointF()},               // 栗色
        {UiColor(0xFF993333), _T("STRID_PUBLIC_COLORPICKER_TIP3_DARK_PINK_4"), UiPointF()}           // 深粉4
    };
    ASSERT(m_colorMap.size() == 127);
}

}//namespace ui
