#include "ColorPickerRegular.h"
#include "duilib/Core/GlobalManager.h"

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
        //颜色的显示名称的语言
        UiString colorNameId;
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

ColorPickerRegular::~ColorPickerRegular()
{
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

void ColorPickerRegular::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
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
        regularColor.colorNameId = color.first;
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
    float fBorderSize = 2.0f;//TODO
    UiRectF borderSize(fBorderSize, fBorderSize, fBorderSize, fBorderSize);
    pControl->SetBorderSize(borderSize, true);
    pControl->SetBorderColor(kControlStatePressed, _T("blue"));
    return pControl;
}

bool ColorPickerRegularProvider::FillElement(ui::Control* pControl, size_t nElementIndex)
{
    if ((pControl != nullptr) && (nElementIndex < m_colors.size())) {
        const RegularColor& regularColor = m_colors[nElementIndex];
        pControl->SetBkColor(regularColor.colorValue);

        DString colorString = StringUtil::Printf(_T("#%02X%02X%02X%02X"),
                                                 regularColor.colorValue.GetA(),
                                                 regularColor.colorValue.GetR(),
                                                 regularColor.colorValue.GetG(),
                                                 regularColor.colorValue.GetB());
        DString colorInfo = GlobalManager::Instance().GetTextById(regularColor.colorNameId.c_str());
        colorInfo += _T("(");
        colorInfo += colorString;
        colorInfo += _T(")");
        pControl->SetToolTipText(colorInfo);
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
    // 多语言字符串ID映射（中文版本）
    uiColors = {
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_AliceBlue"), UiColors::AliceBlue},            // AliceBlue,爱丽丝蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_AntiqueWhite"), UiColors::AntiqueWhite},      // AntiqueWhite,古董白
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Aqua"), UiColors::Aqua},                      // Aqua,浅绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Aquamarine"), UiColors::Aquamarine},          // Aquamarine,海蓝宝石色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Azure"), UiColors::Azure},                    // Azure,蔚蓝色的
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Beige"), UiColors::Beige},                    // Beige,米色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Bisque"), UiColors::Bisque},                  // Bisque,桔黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Black"), UiColors::Black},                    // Black,黑色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_BlanchedAlmond"), UiColors::BlanchedAlmond},  // BlanchedAlmond,白杏色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Blue"), UiColors::Blue},                      // Blue,蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_BlueViolet"), UiColors::BlueViolet},          // BlueViolet,蓝紫罗兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Brown"), UiColors::Brown},                    // Brown,棕色，褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_BurlyWood"), UiColors::BurlyWood},            // BurlyWood,实木色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_CadetBlue"), UiColors::CadetBlue},            // CadetBlue,军蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Chartreuse"), UiColors::Chartreuse},          // Chartreuse,黄绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Chocolate"), UiColors::Chocolate},            // Chocolate,巧克力色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Coral"), UiColors::Coral},                    // Coral,珊瑚色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_CornflowerBlue"), UiColors::CornflowerBlue},  // CornflowerBlue,菊兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Cornsilk"), UiColors::Cornsilk},              // Cornsilk,米绸色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Crimson"), UiColors::Crimson},                // Crimson,暗红色的
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Cyan"), UiColors::Cyan},                      // Cyan,青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkBlue"), UiColors::DarkBlue},              // DarkBlue,深蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkCyan"), UiColors::DarkCyan},              // DarkCyan,深青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkGoldenrod"), UiColors::DarkGoldenrod},    // DarkGoldenrod,深金黄黄
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkGray"), UiColors::DarkGray},              // DarkGray,深灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkGreen"), UiColors::DarkGreen},            // DarkGreen,深绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkKhaki"), UiColors::DarkKhaki},            // DarkKhaki,暗卡其色，深黄褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkMagenta"), UiColors::DarkMagenta},        // DarkMagenta,深品红色，暗洋红
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkOliveGreen"), UiColors::DarkOliveGreen},  // DarkOliveGreen,暗橄榄绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkOrange"), UiColors::DarkOrange},          // DarkOrange,深橙色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkOrchid"), UiColors::DarkOrchid},          // DarkOrchid,暗紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkRed"), UiColors::DarkRed},                // DarkRed,深红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkSalmon"), UiColors::DarkSalmon},          // DarkSalmon,暗肉色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkSeaGreen"), UiColors::DarkSeaGreen},      // DarkSeaGreen,深海蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkSlateBlue"), UiColors::DarkSlateBlue},    // DarkSlateBlue,深灰蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkSlateGray"), UiColors::DarkSlateGray},    // DarkSlateGray,暗绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkTurquoise"), UiColors::DarkTurquoise},    // DarkTurquoise,暗宝石绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DarkViolet"), UiColors::DarkViolet},          // DarkViolet,暗紫罗兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DeepPink"), UiColors::DeepPink},              // DeepPink,深粉红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DeepSkyBlue"), UiColors::DeepSkyBlue},        // DeepSkyBlue,深天蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DimGray"), UiColors::DimGray},                // DimGray,暗灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_DodgerBlue"), UiColors::DodgerBlue},          // DodgerBlue,闪兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Firebrick"), UiColors::Firebrick},            // Firebrick,火砖色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_FloralWhite"), UiColors::FloralWhite},        // FloralWhite,花白色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_ForestGreen"), UiColors::ForestGreen},        // ForestGreen,森林绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Fuchsia"), UiColors::Fuchsia},                // Fuchsia,紫红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Gainsboro"), UiColors::Gainsboro},            // Gainsboro,淡灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_GhostWhite"), UiColors::GhostWhite},          // GhostWhite,幽灵白色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Gold"), UiColors::Gold},                      // Gold,金色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Goldenrod"), UiColors::Goldenrod},            // Goldenrod,金麒麟色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Gray"), UiColors::Gray},                      // Gray,灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Green"), UiColors::Green},                    // Green,绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_GreenYellow"), UiColors::GreenYellow},        // GreenYellow,黄绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Honeydew"), UiColors::Honeydew},              // Honeydew,蜜色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_HotPink"), UiColors::HotPink},                // HotPink,热粉红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_IndianRed"), UiColors::IndianRed},            // IndianRed,印第安红
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Indigo"), UiColors::Indigo},                  // Indigo,靛蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Ivory"), UiColors::Ivory},                    // Ivory,象牙色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Khaki"), UiColors::Khaki},                    // Khaki,黄褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Lavender"), UiColors::Lavender},              // Lavender,薰衣草色，淡紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LavenderBlush"), UiColors::LavenderBlush},    // LavenderBlush,淡紫红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LawnGreen"), UiColors::LawnGreen},            // LawnGreen,草绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LemonChiffon"), UiColors::LemonChiffon},      // LemonChiffon,柠檬绸色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightBlue"), UiColors::LightBlue},            // LightBlue,淡蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightCoral"), UiColors::LightCoral},          // LightCoral,淡珊瑚色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightCyan"), UiColors::LightCyan},            // LightCyan,淡青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightGoldenrodYellow"), UiColors::LightGoldenrodYellow}, // LightGoldenrodYellow,亮菊黄
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightGray"), UiColors::LightGray},            // LightGray,浅灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightGreen"), UiColors::LightGreen},          // LightGreen,淡绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightPink"), UiColors::LightPink},            // LightPink,浅粉红
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightSalmon"), UiColors::LightSalmon},        // LightSalmon,亮肉色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightSeaGreen"), UiColors::LightSeaGreen},    // LightSeaGreen,浅海蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightSkyBlue"), UiColors::LightSkyBlue},      // LightSkyBlue,亮天蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightSlateGray"), UiColors::LightSlateGray},  // LightSlateGray,亮蓝灰
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightSteelBlue"), UiColors::LightSteelBlue},  // LightSteelBlue,亮钢蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LightYellow"), UiColors::LightYellow},        // LightYellow,浅黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Lime"), UiColors::Lime},                      // Lime,石灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_LimeGreen"), UiColors::LimeGreen},            // LimeGreen,酸橙绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Linen"), UiColors::Linen},                    // Linen,亚麻色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Magenta"), UiColors::Magenta},                // Magenta,洋红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Maroon"), UiColors::Maroon},                  // Maroon,紫褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumAquamarine"), UiColors::MediumAquamarine}, // MediumAquamarine,中碧绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumBlue"), UiColors::MediumBlue},          // MediumBlue,中蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumOrchid"), UiColors::MediumOrchid},      // MediumOrchid,中兰花紫
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumPurple"), UiColors::MediumPurple},      // MediumPurple,中紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumSeaGreen"), UiColors::MediumSeaGreen},  // MediumSeaGreen,中海洋绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumSlateBlue"), UiColors::MediumSlateBlue}, // MediumSlateBlue,中板岩蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumSpringGreen"), UiColors::MediumSpringGreen}, // MediumSpringGreen,中春绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumTurquoise"), UiColors::MediumTurquoise}, // MediumTurquoise,中绿宝石色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MediumVioletRed"), UiColors::MediumVioletRed}, // MediumVioletRed,中紫罗兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MidnightBlue"), UiColors::MidnightBlue},      // MidnightBlue,深夜蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MintCream"), UiColors::MintCream},            // MintCream,薄荷色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_MistyRose"), UiColors::MistyRose},            // MistyRose,薄雾玫瑰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Moccasin"), UiColors::Moccasin},              // Moccasin,鹿皮色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_NavajoWhite"), UiColors::NavajoWhite},        // NavajoWhite,纳瓦白
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Navy"), UiColors::Navy},                      // Navy,海军蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_OldLace"), UiColors::OldLace},                // OldLace,浅米色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Olive"), UiColors::Olive},                    // Olive,橄榄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_OliveDrab"), UiColors::OliveDrab},            // OliveDrab,深绿褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Orange"), UiColors::Orange},                  // Orange,橙色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_OrangeRed"), UiColors::OrangeRed},            // OrangeRed,橙红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Orchid"), UiColors::Orchid},                  // Orchid,兰花紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PaleGoldenrod"), UiColors::PaleGoldenrod},    // PaleGoldenrod,淡黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PaleGreen"), UiColors::PaleGreen},            // PaleGreen,淡绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PaleTurquoise"), UiColors::PaleTurquoise},    // PaleTurquoise,苍绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PaleVioletRed"), UiColors::PaleVioletRed},    // PaleVioletRed,浅紫罗兰红
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PapayaWhip"), UiColors::PapayaWhip},          // PapayaWhip,番木色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PeachPuff"), UiColors::PeachPuff},            // PeachPuff,桃色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Peru"), UiColors::Peru},                      // Peru,秘鲁色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Pink"), UiColors::Pink},                      // Pink,粉红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Plum"), UiColors::Plum},                      // Plum,李子色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_PowderBlue"), UiColors::PowderBlue},          // PowderBlue,粉蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Purple"), UiColors::Purple},                  // Purple,紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Red"), UiColors::Red},                        // Red,红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_RosyBrown"), UiColors::RosyBrown},            // RosyBrown,玫瑰棕色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_RoyalBlue"), UiColors::RoyalBlue},            // RoyalBlue,皇家蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SaddleBrown"), UiColors::SaddleBrown},        // SaddleBrown,重褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Salmon"), UiColors::Salmon},                  // Salmon,鲑鱼色，三文鱼色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SandyBrown"), UiColors::SandyBrown},          // SandyBrown,沙棕色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SeaGreen"), UiColors::SeaGreen},              // SeaGreen,海绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SeaShell"), UiColors::SeaShell},              // SeaShell,海贝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Sienna"), UiColors::Sienna},                  // Sienna,黄土赭色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Silver"), UiColors::Silver},                  // Silver,银色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SkyBlue"), UiColors::SkyBlue},                // SkyBlue,天蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SlateBlue"), UiColors::SlateBlue},            // SlateBlue,石板蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SlateGray"), UiColors::SlateGray},            // SlateGray,石板灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Snow"), UiColors::Snow},                      // Snow,雪白色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SpringGreen"), UiColors::SpringGreen},        // SpringGreen,春绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_SteelBlue"), UiColors::SteelBlue},            // SteelBlue,钢青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Tan"), UiColors::Tan},                        // Tan,棕褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Teal"), UiColors::Teal},                      // Teal,青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Thistle"), UiColors::Thistle},                // Thistle,蓟色，是一种紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Tomato"), UiColors::Tomato},                  // Tomato,番茄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Transparent"), UiColors::Transparent},        // Transparent,透明的
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Turquoise"), UiColors::Turquoise},            // Turquoise,绿松色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Violet"), UiColors::Violet},                  // Violet,紫罗兰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Wheat"), UiColors::Wheat},                    // Wheat,小麦色, 淡黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_White"), UiColors::White},                    // White,白色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_WhiteSmoke"), UiColors::WhiteSmoke},          // WhiteSmoke,白烟色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_Yellow"), UiColors::Yellow},                  // Yellow,黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP1_YellowGreen"), UiColors::YellowGreen}         // YellowGreen,黄绿色
    };
}

void ColorPickerRegularProvider::GetBasicColors(std::vector<std::pair<DString, int32_t>>& uiColors)
{
    uiColors = {
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_ROSE_RED"),0xFFF08784},          // 玫瑰红
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_ROSE_RED"),0xFFEB3324},          // 玫瑰红
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BROWN"),0xFF774342},            // 褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_RED"),0xFF8E403A},              // 红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_RED"),0xFF3A0603},          // 深红色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_SKY_BLUE"),0xFF9FFCFD},          // 天蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_SKY_BLUE"),0xFF73FBFD},          // 天蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BLUE"),0xFF3282F6},              // 蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BLUE"),0xFF0023F5},              // 蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_BLUE"),0xFF00129A},         // 深蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_BLUE"),0xFF16417C},         // 深蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_BLUE"),0xFF000C7B},         // 深蓝

        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_YELLOW"),0xFFFFFE91},      // 浅黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_YELLOW"),0xFFFFFD55},            // 黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_ORANGE"),0xFFF09B59},            // 橙色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_ORANGE"),0xFFF08650},            // 橙色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BROWN"),0xFF784315},             // 褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_YELLOW"),0xFF817F26},       // 深黄色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_BLUE"),0xFF7E84F7},        // 浅蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PURPLE"),0xFF732BF5},            // 紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BLUE"),0xFF3580BB},              // 蓝色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_BLUE"),0xFF00023D},         // 深蓝
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_PURPLE"),0xFF58135E},       // 深紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_PURPLE"),0xFF3A083E},       // 深紫色

        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_GREEN"),0xFFA1FB8E},       // 浅绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_GREEN"),0xFFA1FA4F},             // 绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_GREEN"),0xFF75F94D},             // 绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_GREEN"),0xFF75FA61},       // 浅绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_GREEN"),0xFF75FA8D},       // 浅绿
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BROWN"),0xFF818049},             // 褐色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PINK"),0xFFEF88BE},              // 粉色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_PURPLE"),0xFFEE8AF8},      // 浅紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PALE_PURPLE"),0xFFEA3FF7},       // 淡紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PINK"),0xFFEA3680},              // 粉色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PALE_PURPLE"),0xFF7F82BB},       // 淡紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_MAGENTA"),0xFF75163F},           // 紫红色

        {_T("STRID_PUBLIC_COLORPICKER_TIP2_PALE_GREEN"),0xFF377D22},        // 浅绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_GREEN"),0xFF377E47},        // 深绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_CYAN"),0xFF367E7F},         // 深青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_CYAN"),0xFF507F80},              // 青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_GREEN"),0xFF183E0C},        // 深绿色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_CYAN"),0xFF173F3F},         // 深青色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_PURPLE"),0xFF741B7C},       // 深紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_DARK_PURPLE"),0xFF39107B},       // 深紫色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_BLACK"),0xFF000000},             // 黑色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_GRAY"),0xFF808080},              // 灰色
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_LIGHT_GRAY"),0xFFC0C0C0},        // 浅灰
        {_T("STRID_PUBLIC_COLORPICKER_TIP2_WHITE"),0xFFFFFFFF}              // 白色
    };
}

}//namespace ui
