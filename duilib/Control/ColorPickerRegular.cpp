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
    void SetColors(const std::vector<std::pair<DString, int32_t>>& uiColors);

    /** 创建一个数据项
    * @param [in] pVirtualListBox 关联的虚表的接口
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement(ui::VirtualListBox* pVirtualListBox) override;

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
    void GetDefaultColors(std::vector<std::pair<DString, int32_t>>& uiColors);

    /** 基本颜色值，对应的字符串常量
    */
    void GetBasicColors(std::vector<std::pair<DString, int32_t>>& uiColors);

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
        bool m_bSelected = false;

        //比较函数
        bool operator < (const RegularColor& r) const
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

ColorPickerRegular::ColorPickerRegular(Window* pWindow):
    VirtualVTileListBox(pWindow)
{
    m_regularColors = std::make_unique<ColorPickerRegularProvider>();
    SetDataProvider(m_regularColors.get());
    AttachSelect([this](const ui::EventArgs& args) {
        Control* pControl = GetItemAt(args.wParam);
        if (pControl != nullptr) {
            DString colorString = pControl->GetBkColor();
            if (!colorString.empty()) {
                UiColor newColor = pControl->GetUiColor(colorString);
                SendEvent(kEventSelectColor, newColor.GetARGB());
            }
        }
        return true;
        });
}

DString ColorPickerRegular::GetType() const { return DUI_CTR_COLOR_PICKER_REGULAR; }

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
                pVirtualLayout->SetItemSize(szItem);
            }
            else if ((m_szItem.cx > 0) && (m_szItem.cy > 0)) {
                pVirtualLayout->SetItemSize(m_szItem);
            }
        }
    }
    BaseClass::SetPos(rc);
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

void ColorPickerRegular::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("color_type")) {
        if (strValue == _T("basic")) {
            //使用基本颜色
            std::vector<std::pair<DString, int32_t>> uiColors;
            m_regularColors->GetBasicColors(uiColors);
            m_regularColors->SetColors(uiColors);
        }
        else {
            //使用默认颜色
            std::vector<std::pair<DString, int32_t>> uiColors;
            m_regularColors->GetDefaultColors(uiColors);
            m_regularColors->SetColors(uiColors);
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

////////////////////////////////////////////////////////////
//
ColorPickerRegularProvider::ColorPickerRegularProvider()
{
    std::vector<std::pair<DString, int32_t>> uiColors;
    GetDefaultColors(uiColors);
    SetColors(uiColors);
}

void ColorPickerRegularProvider::SetColors(const std::vector<std::pair<DString, int32_t>>& uiColors)
{
    m_colors.clear();
    RegularColor regularColor;
    regularColor.m_bSelected = false;
    for (const auto& color : uiColors) {
        regularColor.colorValue = UiColor(color.second);
        DString colorString = ui::StringUtil::Printf(_T("#%02X%02X%02X%02X"),
            regularColor.colorValue.GetA(),
            regularColor.colorValue.GetR(),
            regularColor.colorValue.GetG(),
            regularColor.colorValue.GetB());
        DString colorName = color.first;
        StringUtil::ReplaceAll(_T(","), _T(", "), colorName);
        colorName = colorString + _T(", ") + colorName;
        regularColor.colorName = colorName;
        if (regularColor.colorValue.GetARGB() != UiColors::Transparent) {
            m_colors.push_back(regularColor);
        }
    }
    std::sort(m_colors.begin(), m_colors.end());
}

Control* ColorPickerRegularProvider::CreateElement(VirtualListBox* pVirtualListBox)
{
    ASSERT(pVirtualListBox != nullptr);
    if (pVirtualListBox == nullptr) {
        return nullptr;
    }
    ASSERT(pVirtualListBox->GetWindow() != nullptr);
    if (pVirtualListBox->GetWindow() == nullptr) {
        return nullptr;
    }
    ListBoxItem* pControl = new ListBoxItem(pVirtualListBox->GetWindow());
    float fBorderSize = 2.0f;
    UiRectF borderSize(fBorderSize, fBorderSize, fBorderSize, fBorderSize);
    pControl->SetBorderSize(borderSize, true);
    pControl->SetBorderColor(kControlStatePushed, _T("blue"));
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

void ColorPickerRegularProvider::GetDefaultColors(std::vector<std::pair<DString, int32_t>>& uiColors)
{
    uiColors = {
                {_T("AliceBlue,爱丽丝蓝"),UiColors::AliceBlue},
                {_T("AntiqueWhite,古董白"),UiColors::AntiqueWhite},
                {_T("Aqua,浅绿色"),UiColors::Aqua},
                {_T("Aquamarine,海蓝宝石色"),UiColors::Aquamarine},
                {_T("Azure,蔚蓝色的"),UiColors::Azure},
                {_T("Beige,米色"),UiColors::Beige},
                {_T("Bisque,桔黄色"),UiColors::Bisque},
                {_T("Black,黑色"),UiColors::Black},
                {_T("BlanchedAlmond,白杏色"),UiColors::BlanchedAlmond},
                {_T("Blue,蓝色"),UiColors::Blue},
                {_T("BlueViolet,蓝紫罗兰色"),UiColors::BlueViolet},
                {_T("Brown,棕色，褐色"),UiColors::Brown},
                {_T("BurlyWood,实木色"),UiColors::BurlyWood},
                {_T("CadetBlue,军蓝色"),UiColors::CadetBlue},
                {_T("Chartreuse,黄绿色"),UiColors::Chartreuse},
                {_T("Chocolate,巧克力色"),UiColors::Chocolate},
                {_T("Coral,珊瑚色"),UiColors::Coral},
                {_T("CornflowerBlue,菊兰色"),UiColors::CornflowerBlue},
                {_T("Cornsilk,米绸色"),UiColors::Cornsilk},
                {_T("Crimson,暗红色的"),UiColors::Crimson},
                {_T("Cyan,青色"),UiColors::Cyan},
                {_T("DarkBlue,深蓝色"),UiColors::DarkBlue},
                {_T("DarkCyan,深青色"),UiColors::DarkCyan},
                {_T("DarkGoldenrod,深金黄黄"),UiColors::DarkGoldenrod},
                {_T("DarkGray,深灰色"),UiColors::DarkGray},
                {_T("DarkGreen,深绿色"),UiColors::DarkGreen},
                {_T("DarkKhaki,暗卡其色，深黄褐色"),UiColors::DarkKhaki},
                {_T("DarkMagenta,深品红色，暗洋红"),UiColors::DarkMagenta},
                {_T("DarkOliveGreen,暗橄榄绿"),UiColors::DarkOliveGreen},
                {_T("DarkOrange,深橙色"),UiColors::DarkOrange},
                {_T("DarkOrchid,暗紫色"),UiColors::DarkOrchid},
                {_T("DarkRed,深红色"),UiColors::DarkRed},
                {_T("DarkSalmon,暗肉色"),UiColors::DarkSalmon},
                {_T("DarkSeaGreen,深海蓝色"),UiColors::DarkSeaGreen},
                {_T("DarkSlateBlue,深灰蓝色"),UiColors::DarkSlateBlue},
                {_T("DarkSlateGray,暗绿色"),UiColors::DarkSlateGray},
                {_T("DarkTurquoise,暗宝石绿色"),UiColors::DarkTurquoise},
                {_T("DarkViolet,暗紫罗兰色"),UiColors::DarkViolet},
                {_T("DeepPink,深粉红色"),UiColors::DeepPink},
                {_T("DeepSkyBlue,深天蓝色"),UiColors::DeepSkyBlue},
                {_T("DimGray,暗灰色"),UiColors::DimGray},
                {_T("DodgerBlue,闪兰色"),UiColors::DodgerBlue},
                {_T("Firebrick,火砖色"),UiColors::Firebrick},
                {_T("FloralWhite,花白色"),UiColors::FloralWhite},
                {_T("ForestGreen,森林绿"),UiColors::ForestGreen},
                {_T("Fuchsia,紫红色"),UiColors::Fuchsia},
                {_T("Gainsboro,淡灰色"),UiColors::Gainsboro},
                {_T("GhostWhite,幽灵白色"),UiColors::GhostWhite},
                {_T("Gold,金色"),UiColors::Gold},
                {_T("Goldenrod,金麒麟色"),UiColors::Goldenrod},
                {_T("Gray,灰色"),UiColors::Gray},
                {_T("Green,绿色"),UiColors::Green},
                {_T("GreenYellow,黄绿色"),UiColors::GreenYellow},
                {_T("Honeydew,蜜色"),UiColors::Honeydew},
                {_T("HotPink,热粉红色"),UiColors::HotPink},
                {_T("IndianRed,印第安红"),UiColors::IndianRed},
                {_T("Indigo,靛蓝色"),UiColors::Indigo},
                {_T("Ivory,象牙色"),UiColors::Ivory},
                {_T("Khaki,黄褐色"),UiColors::Khaki},
                {_T("Lavender,薰衣草色，淡紫色"),UiColors::Lavender},
                {_T("LavenderBlush,淡紫红色"),UiColors::LavenderBlush},
                {_T("LawnGreen,草绿色"),UiColors::LawnGreen},
                {_T("LemonChiffon,柠檬绸色"),UiColors::LemonChiffon},
                {_T("LightBlue,淡蓝色"),UiColors::LightBlue},
                {_T("LightCoral,淡珊瑚色"),UiColors::LightCoral},
                {_T("LightCyan,淡青色"),UiColors::LightCyan},
                {_T("LightGoldenrodYellow,亮菊黄"),UiColors::LightGoldenrodYellow},
                {_T("LightGray,浅灰色"),UiColors::LightGray},
                {_T("LightGreen,淡绿色"),UiColors::LightGreen},
                {_T("LightPink,浅粉红"),UiColors::LightPink},
                {_T("LightSalmon,亮肉色"),UiColors::LightSalmon},
                {_T("LightSeaGreen,浅海蓝色"),UiColors::LightSeaGreen},
                {_T("LightSkyBlue,亮天蓝色"),UiColors::LightSkyBlue},
                {_T("LightSlateGray,亮蓝灰"),UiColors::LightSlateGray},
                {_T("LightSteelBlue,亮钢蓝色"),UiColors::LightSteelBlue},
                {_T("LightYellow,浅黄色"),UiColors::LightYellow},
                {_T("Lime,石灰色"),UiColors::Lime},
                {_T("LimeGreen,酸橙绿色"),UiColors::LimeGreen},
                {_T("Linen,亚麻色"),UiColors::Linen},
                {_T("Magenta,洋红色"),UiColors::Magenta},
                {_T("Maroon,紫褐色"),UiColors::Maroon},
                {_T("MediumAquamarine,中碧绿色"),UiColors::MediumAquamarine},
                {_T("MediumBlue,中蓝色"),UiColors::MediumBlue},
                {_T("MediumOrchid,中兰花紫"),UiColors::MediumOrchid},
                {_T("MediumPurple,中紫色"),UiColors::MediumPurple},
                {_T("MediumSeaGreen,中海洋绿"),UiColors::MediumSeaGreen},
                {_T("MediumSlateBlue,中板岩蓝"),UiColors::MediumSlateBlue},
                {_T("MediumSpringGreen,中春绿色"),UiColors::MediumSpringGreen},
                {_T("MediumTurquoise,中绿宝石色"),UiColors::MediumTurquoise},
                {_T("MediumVioletRed,中紫罗兰色"),UiColors::MediumVioletRed},
                {_T("MidnightBlue,深夜蓝"),UiColors::MidnightBlue},
                {_T("MintCream,薄荷色"),UiColors::MintCream},
                {_T("MistyRose,薄雾玫瑰色"),UiColors::MistyRose},
                {_T("Moccasin,鹿皮色"),UiColors::Moccasin},
                {_T("NavajoWhite,纳瓦白"),UiColors::NavajoWhite},
                {_T("Navy,海军蓝"),UiColors::Navy},
                {_T("OldLace,浅米色"),UiColors::OldLace},
                {_T("Olive,橄榄色"),UiColors::Olive},
                {_T("OliveDrab,深绿褐色"),UiColors::OliveDrab},
                {_T("Orange,橙色"),UiColors::Orange},
                {_T("OrangeRed,橙红色"),UiColors::OrangeRed},
                {_T("Orchid,兰花紫色"),UiColors::Orchid},
                {_T("PaleGoldenrod,淡黄色"),UiColors::PaleGoldenrod},
                {_T("PaleGreen,淡绿色"),UiColors::PaleGreen},
                {_T("PaleTurquoise,苍绿色"),UiColors::PaleTurquoise},
                {_T("PaleVioletRed,浅紫罗兰红"),UiColors::PaleVioletRed},
                {_T("PapayaWhip,番木色"),UiColors::PapayaWhip},
                {_T("PeachPuff,桃色"),UiColors::PeachPuff},
                {_T("Peru,秘鲁色"),UiColors::Peru},
                {_T("Pink,粉红色"),UiColors::Pink},
                {_T("Plum,李子色"),UiColors::Plum},
                {_T("PowderBlue,粉蓝色"),UiColors::PowderBlue},
                {_T("Purple,紫色"),UiColors::Purple},
                {_T("Red,红色"),UiColors::Red},
                {_T("RosyBrown,玫瑰棕色"),UiColors::RosyBrown},
                {_T("RoyalBlue,皇家蓝色"),UiColors::RoyalBlue},
                {_T("SaddleBrown,重褐色"),UiColors::SaddleBrown},
                {_T("Salmon,鲑鱼色，三文鱼色"),UiColors::Salmon},
                {_T("SandyBrown,沙棕色"),UiColors::SandyBrown},
                {_T("SeaGreen,海绿色"),UiColors::SeaGreen},
                {_T("SeaShell,海贝色"),UiColors::SeaShell},
                {_T("Sienna,黄土赭色"),UiColors::Sienna},
                {_T("Silver,银色"),UiColors::Silver},
                {_T("SkyBlue,天蓝色"),UiColors::SkyBlue},
                {_T("SlateBlue,石板蓝色"),UiColors::SlateBlue},
                {_T("SlateGray,石板灰色"),UiColors::SlateGray},
                {_T("Snow,雪白色"),UiColors::Snow},
                {_T("SpringGreen,春绿色"),UiColors::SpringGreen},
                {_T("SteelBlue,钢青色"),UiColors::SteelBlue},
                {_T("Tan,棕褐色"),UiColors::Tan},
                {_T("Teal,青色"),UiColors::Teal},
                {_T("Thistle,蓟色，是一种紫色"),UiColors::Thistle},
                {_T("Tomato,番茄色"),UiColors::Tomato},
                {_T("Transparent,透明的"),UiColors::Transparent},
                {_T("Turquoise,绿松色"),UiColors::Turquoise},
                {_T("Violet,紫罗兰色"),UiColors::Violet},
                {_T("Wheat,小麦色, 淡黄色"),UiColors::Wheat},
                {_T("White,白色"),UiColors::White},
                {_T("WhiteSmoke,白烟色"),UiColors::WhiteSmoke},
                {_T("Yellow,黄色"),UiColors::Yellow},
                {_T("YellowGreen,黄绿色"),UiColors::YellowGreen}
    };
}

void ColorPickerRegularProvider::GetBasicColors(std::vector<std::pair<DString, int32_t>>& uiColors)
{
    uiColors = {
        {_T("玫瑰红"),0xFFF08784},
        {_T("玫瑰红"),0xFFEB3324},
        {_T("褐色"),0xFF774342},
        {_T("红色"),0xFF8E403A},
        {_T("深红色"),0xFF3A0603},
        {_T("天蓝"),0xFF9FFCFD},
        {_T("天蓝"),0xFF73FBFD},
        {_T("蓝色"),0xFF3282F6},
        {_T("蓝色"),0xFF0023F5},
        {_T("深蓝"),0xFF00129A},
        {_T("深蓝"),0xFF16417C},
        {_T("深蓝"),0xFF000C7B},

        {_T("浅黄色"),0xFFFFFE91},
        {_T("黄色"),0xFFFFFD55},
        {_T("橙色"),0xFFF09B59},
        {_T("橙色"),0xFFF08650},
        {_T("褐色"),0xFF784315},
        {_T("深黄色"),0xFF817F26},
        {_T("浅蓝色"),0xFF7E84F7},
        {_T("紫色"),0xFF732BF5},
        {_T("蓝色"),0xFF3580BB},
        {_T("深蓝"),0xFF00023D},
        {_T("深紫色"),0xFF58135E},
        {_T("深紫色"),0xFF3A083E},

        {_T("浅绿"),0xFFA1FB8E},
        {_T("绿色"),0xFFA1FA4F},
        {_T("绿色"),0xFF75F94D},
        {_T("浅绿"),0xFF75FA61},
        {_T("浅绿"),0xFF75FA8D},
        {_T("褐色"),0xFF818049},
        {_T("粉色"),0xFFEF88BE},
        {_T("浅紫色"),0xFFEE8AF8},
        {_T("淡紫色"),0xFFEA3FF7},
        {_T("粉色"),0xFFEA3680},
        {_T("淡紫色"),0xFF7F82BB},
        {_T("紫红色"),0xFF75163F},

        {_T("浅绿色"),0xFF377D22},
        {_T("深绿色"),0xFF377E47},
        {_T("深青色"),0xFF367E7F},
        {_T("青色"),0xFF507F80},
        {_T("深绿色"),0xFF183E0C},
        {_T("深青色"),0xFF173F3F},
        {_T("深紫色"),0xFF741B7C},
        {_T("深紫色"),0xFF39107B},
        {_T("黑色"),0xFF000000},
        {_T("灰色"),0xFF808080},
        {_T("浅灰"),0xFFC0C0C0},
        {_T("白色"),0xFFFFFFFF}
    };
}

}//namespace ui
