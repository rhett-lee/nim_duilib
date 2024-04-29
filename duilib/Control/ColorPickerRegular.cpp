#include "ColorPickerRegular.h"

namespace ui
{
/** 颜色选择器：常用颜色
*/
class ColorPickerRegularProvider : public ui::VirtualListBoxElement
{
public:
    ColorPickerRegularProvider();

    /** 设置颜色数据
    * @param [in] uiColors 外部提供的颜色数据，第一个是颜色名称，第二个是颜色值
    */
    void SetColors(const std::vector<std::pair<std::wstring, int32_t>>& uiColors);

    /** 创建一个数据项
    * @return 返回创建后的数据项指针
    */
    virtual Control* CreateElement() override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() const override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) const override;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    virtual void GetSelectedElements(std::vector<size_t>& selectedIndexs) const override;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const override;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) override;

    /** 选择一个颜色
    */
    size_t SelectColor(const UiColor& color);

    /** 获取当前选择的颜色
    */
    UiColor GetSelectedColor() const;

public:
    /** 常见颜色值，对应的字符串常量
    */
    void GetDefaultColors(std::vector<std::pair<std::wstring, int32_t>>& uiColors);

    /** 基本颜色值，对应的字符串常量
    */
    void GetBasicColors(std::vector<std::pair<std::wstring, int32_t>>& uiColors);

private:
    /** 颜色结构
    */
    struct RegularColor
    {
        //颜色的显示名称
        UiString colorName;
        //颜色值
        UiColor colorValue;
        //是否选择
        bool m_bSelected;

        //比较函数
        bool operator < (const RegularColor& r)
        {
            //按照颜色的灰度值排序
            double v1 = colorValue.GetR() * 0.299 + colorValue.GetG() * 0.587 + colorValue.GetB() * 0.114;
            double v2 = r.colorValue.GetR() * 0.299 + r.colorValue.GetG() * 0.587 + r.colorValue.GetB() * 0.114;
            return v1 < v2;
        }
    };

    /** 颜色列表
    */
    std::vector<RegularColor> m_colors;
};

ColorPickerRegular::ColorPickerRegular()
{
    m_regularColors = std::make_unique<ColorPickerRegularProvider>();
    SetDataProvider(m_regularColors.get());
    AttachSelect([this](const ui::EventArgs& args) {
        Control* pControl = GetItemAt(args.wParam);
        if (pControl != nullptr) {
            std::wstring colorString = pControl->GetBkColor();
            if (!colorString.empty()) {
                UiColor newColor = pControl->GetUiColor(colorString);
                SendEvent(kEventSelectColor, newColor.GetARGB());
            }
        }
        return true;
        });
}

std::wstring ColorPickerRegular::GetType() const { return DUI_CTR_COLOR_PICKER_REGULAR; }

void ColorPickerRegular::SelectColor(const UiColor& color)
{
    size_t nCurSel = m_regularColors->SelectColor(color);
    size_t nOldSel = GetCurSel();
    if (nCurSel != nOldSel) {
        Control* pControl = GetItemAt(nOldSel);
        if (pControl != nullptr) {
            ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
            if ((pListItem != nullptr) && (pListItem->IsSelected())) {
                pListItem->SetSelected(false);
            }
        }
        if (nCurSel < GetItemCount()) {
            SetCurSel(nCurSel);
        }        
    }
    Refresh();
    Invalidate();
}

UiColor ColorPickerRegular::GetSelectedColor() const
{
    return m_regularColors->GetSelectedColor();
}

void ColorPickerRegular::SetPos(UiRect rc)
{
    UiRect rect = rc;
    rect.Deflate(GetPadding());
    //自动计算每个颜色块的大小，适应拉伸
    int32_t elementCount = 0;
    VirtualListBoxElement* pListBoxElement = GetDataProvider();
    if (pListBoxElement != nullptr) {
        elementCount = (int32_t)pListBoxElement->GetElementCount();
    }
    VirtualVTileLayout* pVirtualLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
    if ((pVirtualLayout != nullptr) && (elementCount > 0)) {
        if ((m_szItem.cx == 0) && (m_szItem.cy == 0)) {
            m_szItem = pVirtualLayout->GetItemSize();
        }
        int32_t columns = pVirtualLayout->GetColumns();
        if (columns > 0) {
            int32_t rows = elementCount / columns;
            if ((elementCount % columns) != 0) {
                rows += 1;
            }
            if (rows <= 0) {
                rows += 1;
            }
            //自动计算Item大小            
            int32_t childMarginX = pVirtualLayout->GetChildMarginX();
            int32_t childMarginY = pVirtualLayout->GetChildMarginY();
            UiSize szItem;
            szItem.cx = (rect.Width() - childMarginX * (columns - 1)) / columns;
            szItem.cy = (rect.Height() - childMarginY * (rows - 1)) / rows;
            if ((szItem.cx > 0) && (szItem.cy > 0)) {
                pVirtualLayout->SetItemSize(szItem, false);
            }
            else if ((m_szItem.cx > 0) && (m_szItem.cy > 0)) {
                pVirtualLayout->SetItemSize(m_szItem, false);
            }
        }
    }
    __super::SetPos(rc);
}

void ColorPickerRegular::SetColumns(int32_t nColumns)
{
    ASSERT(nColumns > 0);
    if (nColumns <= 0) {
        return;
    }
    VirtualVTileLayout* pVirtualLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
    if (pVirtualLayout != nullptr) {
        pVirtualLayout->SetColumns(nColumns);
    }
}

void ColorPickerRegular::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"color_type") {
        if (strValue == L"basic") {
            //使用基本颜色
            std::vector<std::pair<std::wstring, int32_t>> uiColors;
            m_regularColors->GetBasicColors(uiColors);
            m_regularColors->SetColors(uiColors);
        }
        else {
            //使用默认颜色
            std::vector<std::pair<std::wstring, int32_t>> uiColors;
            m_regularColors->GetDefaultColors(uiColors);
            m_regularColors->SetColors(uiColors);
        }
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

////////////////////////////////////////////////////////////
//
ColorPickerRegularProvider::ColorPickerRegularProvider()
{
    std::vector<std::pair<std::wstring, int32_t>> uiColors;
    GetDefaultColors(uiColors);
    SetColors(uiColors);
}

void ColorPickerRegularProvider::SetColors(const std::vector<std::pair<std::wstring, int32_t>>& uiColors)
{
    m_colors.clear();
    RegularColor regularColor;
    regularColor.m_bSelected = false;
    for (const auto& color : uiColors) {
        regularColor.colorValue = UiColor(color.second);
        std::wstring colorString = ui::StringHelper::Printf(L"#%02X%02X%02X%02X",
            regularColor.colorValue.GetA(),
            regularColor.colorValue.GetR(),
            regularColor.colorValue.GetG(),
            regularColor.colorValue.GetB());
        std::wstring colorName = color.first;
        StringHelper::ReplaceAll(L",", L", ", colorName);
        colorName = colorString + L", " + colorName;
        regularColor.colorName = colorName;
        if (regularColor.colorValue.GetARGB() != UiColors::Transparent) {
            m_colors.push_back(regularColor);
        }
    }
    std::sort(m_colors.begin(), m_colors.end());
}

Control* ColorPickerRegularProvider::CreateElement()
{
    ListBoxItem* pControl = new ListBoxItem;
    UiRect borderSize(1, 1, 1, 1);
    pControl->SetBorderSize(borderSize);
    pControl->SetBorderColor(kControlStatePushed, L"blue");
    return pControl;
}

bool ColorPickerRegularProvider::FillElement(ui::Control* pControl, size_t nElementIndex)
{
    if ((pControl != nullptr) && (nElementIndex < m_colors.size())) {
        const RegularColor& regularColor = m_colors[nElementIndex];
        pControl->SetBkColor(regularColor.colorValue);
        pControl->SetToolTipText(regularColor.colorName.c_str());
        pControl->SetUserDataID(nElementIndex);
        return true;
    }
    return false;
}

size_t ColorPickerRegularProvider::GetElementCount() const
{
    return m_colors.size();
}

void ColorPickerRegularProvider::SetElementSelected(size_t nElementIndex, bool bSelected)
{
    if (nElementIndex < m_colors.size()) {
        RegularColor& regularColor = m_colors[nElementIndex];
        regularColor.m_bSelected = bSelected;
    }
}

bool ColorPickerRegularProvider::IsElementSelected(size_t nElementIndex) const
{
    if (nElementIndex < m_colors.size()) {
        const RegularColor& regularColor = m_colors[nElementIndex];
        return regularColor.m_bSelected;
    }
    return false;
}

void ColorPickerRegularProvider::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    size_t nCount = m_colors.size();
    for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
        const RegularColor& regularColor = m_colors[nElementIndex];
        if (regularColor.m_bSelected) {
            selectedIndexs.push_back(nElementIndex);
        }
    }
}

bool ColorPickerRegularProvider::IsMultiSelect() const
{
    return false;
}

void ColorPickerRegularProvider::SetMultiSelect(bool /*bMultiSelect*/)
{
}

size_t ColorPickerRegularProvider::SelectColor(const UiColor& color)
{
    size_t selectIndex = Box::InvalidIndex;
    for (size_t index = 0; index < m_colors.size(); ++index) {
        RegularColor& regularColor = m_colors[index];
        if (regularColor.colorValue == color) {
            regularColor.m_bSelected = true;
            selectIndex = index;
        }
        else {
            regularColor.m_bSelected = false;
        }
    }
    return selectIndex;
}

UiColor ColorPickerRegularProvider::GetSelectedColor() const
{
    for (const RegularColor& regularColor : m_colors) {
        if (regularColor.m_bSelected) {
            return regularColor.colorValue;
        }
    }
    return UiColor();
}

void ColorPickerRegularProvider::GetDefaultColors(std::vector<std::pair<std::wstring, int32_t>>& uiColors)
{
    uiColors = {
                {L"AliceBlue,爱丽丝蓝",UiColors::AliceBlue},
                {L"AntiqueWhite,古董白",UiColors::AntiqueWhite},
                {L"Aqua,浅绿色",UiColors::Aqua},
                {L"Aquamarine,海蓝宝石色",UiColors::Aquamarine},
                {L"Azure,蔚蓝色的",UiColors::Azure},
                {L"Beige,米色",UiColors::Beige},
                {L"Bisque,桔黄色",UiColors::Bisque},
                {L"Black,黑色",UiColors::Black},
                {L"BlanchedAlmond,白杏色",UiColors::BlanchedAlmond},
                {L"Blue,蓝色",UiColors::Blue},
                {L"BlueViolet,蓝紫罗兰色",UiColors::BlueViolet},
                {L"Brown,棕色，褐色",UiColors::Brown},
                {L"BurlyWood,实木色",UiColors::BurlyWood},
                {L"CadetBlue,军蓝色",UiColors::CadetBlue},
                {L"Chartreuse,黄绿色",UiColors::Chartreuse},
                {L"Chocolate,巧克力色",UiColors::Chocolate},
                {L"Coral,珊瑚色",UiColors::Coral},
                {L"CornflowerBlue,菊兰色",UiColors::CornflowerBlue},
                {L"Cornsilk,米绸色",UiColors::Cornsilk},
                {L"Crimson,暗红色的",UiColors::Crimson},
                {L"Cyan,青色",UiColors::Cyan},
                {L"DarkBlue,深蓝色",UiColors::DarkBlue},
                {L"DarkCyan,深青色",UiColors::DarkCyan},
                {L"DarkGoldenrod,深金黄黄",UiColors::DarkGoldenrod},
                {L"DarkGray,深灰色",UiColors::DarkGray},
                {L"DarkGreen,深绿色",UiColors::DarkGreen},
                {L"DarkKhaki,暗卡其色，深黄褐色",UiColors::DarkKhaki},
                {L"DarkMagenta,深品红色，暗洋红",UiColors::DarkMagenta},
                {L"DarkOliveGreen,暗橄榄绿",UiColors::DarkOliveGreen},
                {L"DarkOrange,深橙色",UiColors::DarkOrange},
                {L"DarkOrchid,暗紫色",UiColors::DarkOrchid},
                {L"DarkRed,深红色",UiColors::DarkRed},
                {L"DarkSalmon,暗肉色",UiColors::DarkSalmon},
                {L"DarkSeaGreen,深海蓝色",UiColors::DarkSeaGreen},
                {L"DarkSlateBlue,深灰蓝色",UiColors::DarkSlateBlue},
                {L"DarkSlateGray,暗绿色",UiColors::DarkSlateGray},
                {L"DarkTurquoise,暗宝石绿色",UiColors::DarkTurquoise},
                {L"DarkViolet,暗紫罗兰色",UiColors::DarkViolet},
                {L"DeepPink,深粉红色",UiColors::DeepPink},
                {L"DeepSkyBlue,深天蓝色",UiColors::DeepSkyBlue},
                {L"DimGray,暗灰色",UiColors::DimGray},
                {L"DodgerBlue,闪兰色",UiColors::DodgerBlue},
                {L"Firebrick,火砖色",UiColors::Firebrick},
                {L"FloralWhite,花白色",UiColors::FloralWhite},
                {L"ForestGreen,森林绿",UiColors::ForestGreen},
                {L"Fuchsia,紫红色",UiColors::Fuchsia},
                {L"Gainsboro,淡灰色",UiColors::Gainsboro},
                {L"GhostWhite,幽灵白色",UiColors::GhostWhite},
                {L"Gold,金色",UiColors::Gold},
                {L"Goldenrod,金麒麟色",UiColors::Goldenrod},
                {L"Gray,灰色",UiColors::Gray},
                {L"Green,绿色",UiColors::Green},
                {L"GreenYellow,黄绿色",UiColors::GreenYellow},
                {L"Honeydew,蜜色",UiColors::Honeydew},
                {L"HotPink,热粉红色",UiColors::HotPink},
                {L"IndianRed,印第安红",UiColors::IndianRed},
                {L"Indigo,靛蓝色",UiColors::Indigo},
                {L"Ivory,象牙色",UiColors::Ivory},
                {L"Khaki,黄褐色",UiColors::Khaki},
                {L"Lavender,薰衣草色，淡紫色",UiColors::Lavender},
                {L"LavenderBlush,淡紫红色",UiColors::LavenderBlush},
                {L"LawnGreen,草绿色",UiColors::LawnGreen},
                {L"LemonChiffon,柠檬绸色",UiColors::LemonChiffon},
                {L"LightBlue,淡蓝色",UiColors::LightBlue},
                {L"LightCoral,淡珊瑚色",UiColors::LightCoral},
                {L"LightCyan,淡青色",UiColors::LightCyan},
                {L"LightGoldenrodYellow,亮菊黄",UiColors::LightGoldenrodYellow},
                {L"LightGray,浅灰色",UiColors::LightGray},
                {L"LightGreen,淡绿色",UiColors::LightGreen},
                {L"LightPink,浅粉红",UiColors::LightPink},
                {L"LightSalmon,亮肉色",UiColors::LightSalmon},
                {L"LightSeaGreen,浅海蓝色",UiColors::LightSeaGreen},
                {L"LightSkyBlue,亮天蓝色",UiColors::LightSkyBlue},
                {L"LightSlateGray,亮蓝灰",UiColors::LightSlateGray},
                {L"LightSteelBlue,亮钢蓝色",UiColors::LightSteelBlue},
                {L"LightYellow,浅黄色",UiColors::LightYellow},
                {L"Lime,石灰色",UiColors::Lime},
                {L"LimeGreen,酸橙绿色",UiColors::LimeGreen},
                {L"Linen,亚麻色",UiColors::Linen},
                {L"Magenta,洋红色",UiColors::Magenta},
                {L"Maroon,紫褐色",UiColors::Maroon},
                {L"MediumAquamarine,中碧绿色",UiColors::MediumAquamarine},
                {L"MediumBlue,中蓝色",UiColors::MediumBlue},
                {L"MediumOrchid,中兰花紫",UiColors::MediumOrchid},
                {L"MediumPurple,中紫色",UiColors::MediumPurple},
                {L"MediumSeaGreen,中海洋绿",UiColors::MediumSeaGreen},
                {L"MediumSlateBlue,中板岩蓝",UiColors::MediumSlateBlue},
                {L"MediumSpringGreen,中春绿色",UiColors::MediumSpringGreen},
                {L"MediumTurquoise,中绿宝石色",UiColors::MediumTurquoise},
                {L"MediumVioletRed,中紫罗兰色",UiColors::MediumVioletRed},
                {L"MidnightBlue,深夜蓝",UiColors::MidnightBlue},
                {L"MintCream,薄荷色",UiColors::MintCream},
                {L"MistyRose,薄雾玫瑰色",UiColors::MistyRose},
                {L"Moccasin,鹿皮色",UiColors::Moccasin},
                {L"NavajoWhite,纳瓦白",UiColors::NavajoWhite},
                {L"Navy,海军蓝",UiColors::Navy},
                {L"OldLace,浅米色",UiColors::OldLace},
                {L"Olive,橄榄色",UiColors::Olive},
                {L"OliveDrab,深绿褐色",UiColors::OliveDrab},
                {L"Orange,橙色",UiColors::Orange},
                {L"OrangeRed,橙红色",UiColors::OrangeRed},
                {L"Orchid,兰花紫色",UiColors::Orchid},
                {L"PaleGoldenrod,淡黄色",UiColors::PaleGoldenrod},
                {L"PaleGreen,淡绿色",UiColors::PaleGreen},
                {L"PaleTurquoise,苍绿色",UiColors::PaleTurquoise},
                {L"PaleVioletRed,浅紫罗兰红",UiColors::PaleVioletRed},
                {L"PapayaWhip,番木色",UiColors::PapayaWhip},
                {L"PeachPuff,桃色",UiColors::PeachPuff},
                {L"Peru,秘鲁色",UiColors::Peru},
                {L"Pink,粉红色",UiColors::Pink},
                {L"Plum,李子色",UiColors::Plum},
                {L"PowderBlue,粉蓝色",UiColors::PowderBlue},
                {L"Purple,紫色",UiColors::Purple},
                {L"Red,红色",UiColors::Red},
                {L"RosyBrown,玫瑰棕色",UiColors::RosyBrown},
                {L"RoyalBlue,皇家蓝色",UiColors::RoyalBlue},
                {L"SaddleBrown,重褐色",UiColors::SaddleBrown},
                {L"Salmon,鲑鱼色，三文鱼色",UiColors::Salmon},
                {L"SandyBrown,沙棕色",UiColors::SandyBrown},
                {L"SeaGreen,海绿色",UiColors::SeaGreen},
                {L"SeaShell,海贝色",UiColors::SeaShell},
                {L"Sienna,黄土赭色",UiColors::Sienna},
                {L"Silver,银色",UiColors::Silver},
                {L"SkyBlue,天蓝色",UiColors::SkyBlue},
                {L"SlateBlue,石板蓝色",UiColors::SlateBlue},
                {L"SlateGray,石板灰色",UiColors::SlateGray},
                {L"Snow,雪白色",UiColors::Snow},
                {L"SpringGreen,春绿色",UiColors::SpringGreen},
                {L"SteelBlue,钢青色",UiColors::SteelBlue},
                {L"Tan,棕褐色",UiColors::Tan},
                {L"Teal,青色",UiColors::Teal},
                {L"Thistle,蓟色，是一种紫色",UiColors::Thistle},
                {L"Tomato,番茄色",UiColors::Tomato},
                {L"Transparent,透明的",UiColors::Transparent},
                {L"Turquoise,绿松色",UiColors::Turquoise},
                {L"Violet,紫罗兰色",UiColors::Violet},
                {L"Wheat,小麦色, 淡黄色",UiColors::Wheat},
                {L"White,白色",UiColors::White},
                {L"WhiteSmoke,白烟色",UiColors::WhiteSmoke},
                {L"Yellow,黄色",UiColors::Yellow},
                {L"YellowGreen,黄绿色",UiColors::YellowGreen}
    };
}

void ColorPickerRegularProvider::GetBasicColors(std::vector<std::pair<std::wstring, int32_t>>& uiColors)
{
    uiColors = {
        {L"玫瑰红",0xFFF08784},
        {L"玫瑰红",0xFFEB3324},
        {L"褐色",0xFF774342},
        {L"红色",0xFF8E403A},
        {L"深红色",0xFF3A0603},
        {L"天蓝",0xFF9FFCFD},
        {L"天蓝",0xFF73FBFD},
        {L"蓝色",0xFF3282F6},
        {L"蓝色",0xFF0023F5},
        {L"深蓝",0xFF00129A},
        {L"深蓝",0xFF16417C},
        {L"深蓝",0xFF000C7B},

        {L"浅黄色",0xFFFFFE91},
        {L"黄色",0xFFFFFD55},
        {L"橙色",0xFFF09B59},
        {L"橙色",0xFFF08650},
        {L"褐色",0xFF784315},
        {L"深黄色",0xFF817F26},
        {L"浅蓝色",0xFF7E84F7},
        {L"紫色",0xFF732BF5},
        {L"蓝色",0xFF3580BB},
        {L"深蓝",0xFF00023D},
        {L"深紫色",0xFF58135E},
        {L"深紫色",0xFF3A083E},

        {L"浅绿",0xFFA1FB8E},
        {L"绿色",0xFFA1FA4F},
        {L"绿色",0xFF75F94D},
        {L"浅绿",0xFF75FA61},
        {L"浅绿",0xFF75FA8D},
        {L"褐色",0xFF818049},
        {L"粉色",0xFFEF88BE},
        {L"浅紫色",0xFFEE8AF8},
        {L"淡紫色",0xFFEA3FF7},
        {L"粉色",0xFFEA3680},
        {L"淡紫色",0xFF7F82BB},
        {L"紫红色",0xFF75163F},

        {L"浅绿色",0xFF377D22},
        {L"深绿色",0xFF377E47},
        {L"深青色",0xFF367E7F},
        {L"青色",0xFF507F80},
        {L"深绿色",0xFF183E0C},
        {L"深青色",0xFF173F3F},
        {L"深紫色",0xFF741B7C},
        {L"深紫色",0xFF39107B},
        {L"黑色",0xFF000000},
        {L"灰色",0xFF808080},
        {L"浅灰",0xFFC0C0C0},
        {L"白色",0xFFFFFFFF}
    };
}

}//namespace ui
