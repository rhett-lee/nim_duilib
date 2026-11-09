#include "MainForm.h"

#include "duilib/third_party/xml/pugixml.hpp"
#include "duilib/Core/ColorConverter.h"
#include "duilib/Utils/FileDialog.h"
#include "duilib/Utils/FileUtil.h"

#include <algorithm>
#include <iterator>

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("color_theme");
}

DString MainForm::GetSkinFile()
{
    return _T("color_theme.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //显示所有颜色
    ShowAllColors();    

    ui::Button* pOpenThemeMgrBtn = dynamic_cast<ui::Button*>(FindControl(_T("open_theme_manager")));
    if (pOpenThemeMgrBtn != nullptr) {
        pOpenThemeMgrBtn->AttachClick([](const ui::EventArgs&) {
            //打开主题配色管理器
            ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
            resourcePath /= ui::FilePath(_T("../../docs/Tools/ColorThemeMgr.html"));
            ui::SystemUtil::OpenUrl(resourcePath.NativePath());
            return true;
            });
    }

    ui::Button* pOpenNewWindowDark = dynamic_cast<ui::Button*>(FindControl(_T("open_new_window_as_dark")));
    if (pOpenNewWindowDark != nullptr) {
        pOpenNewWindowDark->AttachClick([](const ui::EventArgs&) {
            //以深色主题打开
            MainForm* window = new MainForm();
            window->CreateWnd(nullptr, ui::WindowCreateParam(_T("ColorTheme"), true));
            window->ShowWindow(ui::kSW_SHOW_NORMAL);
            window->OpenColorTheme(ui::FilePath(_T("color_dark")));
            return true;
            });
    }

    ui::Button* pOpenNewWindowLight = dynamic_cast<ui::Button*>(FindControl(_T("open_new_window_as_light")));
    if (pOpenNewWindowLight != nullptr) {
        pOpenNewWindowLight->AttachClick([](const ui::EventArgs&) {
            //以浅色色主题打开
            MainForm* window = new MainForm();
            window->CreateWnd(nullptr, ui::WindowCreateParam(_T("ColorTheme"), true));
            window->ShowWindow(ui::kSW_SHOW_NORMAL);
            window->OpenColorTheme(ui::FilePath(_T("color_light")));
            return true;
            });
    }

    //Flyout 浮层演示
    ui::Button* pFlyoutBottomBtn = dynamic_cast<ui::Button*>(FindControl(_T("flyout_show_bottom")));
    if (pFlyoutBottomBtn != nullptr) {
        pFlyoutBottomBtn->AttachClick([this](const ui::EventArgs& args) {
            ShowFlyoutDemo(args.GetSender(), ui::Flyout::Placement::Bottom);
            return true;
            });
    }
    ui::Button* pFlyoutRightBtn = dynamic_cast<ui::Button*>(FindControl(_T("flyout_show_right")));
    if (pFlyoutRightBtn != nullptr) {
        pFlyoutRightBtn->AttachClick([this](const ui::EventArgs& args) {
            ShowFlyoutDemo(args.GetSender(), ui::Flyout::Placement::Right);
            return true;
            });
    }

    //主题生成相关
    ui::CheckBox* pGenColorCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("gen_color_checkbox")));
    ui::Box* pGenColorBox = dynamic_cast<ui::Box*>(FindControl(_T("gen_color_box")));

    ui::Slider* pSliderHue = dynamic_cast<ui::Slider*>(FindControl(_T("gen_color_slider_hue")));
    ui::Label* pLabelHue = dynamic_cast<ui::Label*>(FindControl(_T("gen_color_label_hue")));

    ui::Slider* pSliderBaseChroma = dynamic_cast<ui::Slider*>(FindControl(_T("gen_color_slider_base_chroma")));
    ui::Label* pLabelBaseChroma = dynamic_cast<ui::Label*>(FindControl(_T("gen_color_label_base_chroma")));
    if ((pGenColorCheckBox == nullptr) || (pGenColorBox == nullptr) ||
        (pSliderHue == nullptr) || (pLabelHue == nullptr) ||
        (pSliderBaseChroma == nullptr) || (pLabelBaseChroma == nullptr)) {
        return;
    }

    pSliderHue->AttachValueChanged([this](const ui::EventArgs&) {
        OnGenColorParamChanged();
        return true;
        });
    pSliderBaseChroma->AttachValueChanged([this](const ui::EventArgs&) {
        OnGenColorParamChanged();
        return true;
        });

    ui::Option* pOptionColorType = dynamic_cast<ui::Option*>(FindControl(_T("gen_color_radio_light")));
    if (pOptionColorType != nullptr) {
        pOptionColorType->AttachSelect([this](const ui::EventArgs&) {
            OnGenColorParamChanged();
            return true;
            });
    }
    pOptionColorType = dynamic_cast<ui::Option*>(FindControl(_T("gen_color_radio_dark")));
    if (pOptionColorType != nullptr) {
        pOptionColorType->AttachSelect([this](const ui::EventArgs&) {
            OnGenColorParamChanged();
            return true;
            });
    }

    //初始状态
    pGenColorBox->SetEnabled(pGenColorCheckBox->IsSelected());
    OnGenColorEnableStateChanged(pGenColorCheckBox->IsSelected());
    ui::Control* pGenColorGroupBox = FindControl(_T("gen_color_groupbox"));
    if (pGenColorGroupBox != nullptr) {
        pGenColorGroupBox->SetVisible(pGenColorCheckBox->IsSelected());
    }
    ui::Control* pGenColorGroupBoxSplit = FindControl(_T("gen_color_groupbox_split"));
    if (pGenColorGroupBoxSplit != nullptr) {
        pGenColorGroupBoxSplit->SetVisible(pGenColorCheckBox->IsSelected());
    }

    pGenColorCheckBox->AttachSelect([this, pGenColorBox](const ui::EventArgs&) {
        ui::Control* pGenColorGroupBox = FindControl(_T("gen_color_groupbox"));
        if (pGenColorGroupBox != nullptr) {
            pGenColorGroupBox->SetVisible(true);
        }
        ui::Control* pGenColorGroupBoxSplit = FindControl(_T("gen_color_groupbox_split"));
        if (pGenColorGroupBoxSplit != nullptr) {
            pGenColorGroupBoxSplit->SetVisible(true);
        }
        pGenColorBox->SetEnabled(true);
        OnGenColorEnableStateChanged(true);
        return true;
        });

    pGenColorCheckBox->AttachUnSelect([this, pGenColorBox](const ui::EventArgs&) {
        pGenColorBox->SetEnabled(false);
        OnGenColorEnableStateChanged(false);
        ui::Control* pGenColorGroupBox = FindControl(_T("gen_color_groupbox"));
        if (pGenColorGroupBox != nullptr) {
            pGenColorGroupBox->SetVisible(false);
        }
        ui::Control* pGenColorGroupBoxSplit = FindControl(_T("gen_color_groupbox_split"));
        if (pGenColorGroupBoxSplit != nullptr) {
            pGenColorGroupBoxSplit->SetVisible(false);
        }
        return true;
        });

    //仅使用中性色
    ui::CheckBox* pGenNeutralColorCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("gen_color_neutral_mode")));
    if (pGenNeutralColorCheckBox != nullptr) {
        pGenNeutralColorCheckBox->AttachSelect([this](const ui::EventArgs&) {
            if (m_pThemeGenerator != nullptr) {
                m_pThemeGenerator->SetBgColored(false);
                m_pThemeGenerator->SetFgColored(false);
                m_pThemeGenerator->SetSurfaceColored(false);
                OnGenColorEnableStateChanged(true);
            }            
            return true;
            });
        pGenNeutralColorCheckBox->AttachUnSelect([this](const ui::EventArgs&) {
            if (m_pThemeGenerator != nullptr) {
                m_pThemeGenerator->SetBgColored(true);
                m_pThemeGenerator->SetFgColored(true);
                m_pThemeGenerator->SetSurfaceColored(true);
                OnGenColorEnableStateChanged(true);
            }
            return true;
            });
    }

    //复制XML文件数据
    ui::Button* pCopyXmlBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_copty_xml_data")));
    if (pCopyXmlBtn != nullptr) {
        pCopyXmlBtn->AttachClick([this](const ui::EventArgs&) {
            if (m_colorThemeXmlData.empty()) {
                OnGenColorParamChanged();
            }
            if (!m_colorThemeXmlData.empty()) {
                ui::Clipboard::SetClipboardText(m_colorThemeXmlData);
            }
            return true;
            });
    }

    //保存XML文件数据
    ui::Button* pSaveXmlBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_save_xml_data")));
    if (pSaveXmlBtn != nullptr) {
        pSaveXmlBtn->AttachClick([this](const ui::EventArgs&) {
            SaveXmlDataToFile();
            return true;
            });
    }

    //重置生成参数
    ui::Button* pResetBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_reset_gen_params")));
    if (pResetBtn != nullptr) {
        pResetBtn->AttachClick([this](const ui::EventArgs&) {
            ResetGenParams();
            return true;
            });
    }

    //初始化生成器参数滑块
    InitGenParamSliders();

    //初始化需要用代码填充数据的预览控件
    InitPreviewControls();

    //显示 global.xml 的完整配置
    ShowGlobalConfigInfo();

    //显示浅色/深色主题颜色对照表
    ShowLightDarkColorCompare();
}

void MainForm::OnGenColorEnableStateChanged(bool bEnabled)
{
    if (bEnabled) {
        OpenColorTheme(ui::FilePath(DUILIB_LIGHT_COLOR_PATH));

        //更新当前页面的配置
        OnGenColorParamChanged();
    }
    else {
        CloseColorTheme();
    }
}

void MainForm::OnGenColorParamChanged()
{
    ui::Slider* pSliderHue = dynamic_cast<ui::Slider*>(FindControl(_T("gen_color_slider_hue")));
    ui::Label* pLabelHue = dynamic_cast<ui::Label*>(FindControl(_T("gen_color_label_hue")));

    ui::Slider* pSliderBaseChroma = dynamic_cast<ui::Slider*>(FindControl(_T("gen_color_slider_base_chroma")));
    ui::Label* pLabelBaseChroma = dynamic_cast<ui::Label*>(FindControl(_T("gen_color_label_base_chroma")));
    if ((pSliderHue == nullptr) || (pLabelHue == nullptr) ||
        (pSliderBaseChroma == nullptr) || (pLabelBaseChroma == nullptr)) {
        return;
    }

    double fHue = pSliderHue->GetValue() / 10.0; // [0 - 360]
    if (fHue < 0.0) {
        fHue = 0.0;
    }
    if (fHue > 360.0) {
        fHue = 360.0;
    }
    double fBaseChroma = pSliderBaseChroma->GetValue() / 10000.0; // [0 - 0.03]
    if (fBaseChroma < 0.0) {
        fBaseChroma = 0.0;
    }
    if (fBaseChroma > 1.0) {
        fBaseChroma = 1.0;
    }

    DString hueString = ui::StringUtil::Printf(_T("%.02f"), fHue);
    pLabelHue->SetText(hueString);

    DString baseChromaString = ui::StringUtil::Printf(_T("%.04f"), fBaseChroma);
    pLabelBaseChroma->SetText(baseChromaString);

    CreateThemeGenerator();
    if (m_pThemeGenerator == nullptr) {
        return;
    }

    //读取生成器参数
    auto getSliderValue = [this](const DString& name, double fDefault) -> double {
        ui::Slider* pSlider = dynamic_cast<ui::Slider*>(FindControl(name));
        return (pSlider != nullptr) ? pSlider->GetValue() : fDefault;
    };

    double bgLightL = getSliderValue(_T("gen_color_slider_bg_l_light"), 970.0) / 1000.0;
    double bgDarkL = getSliderValue(_T("gen_color_slider_bg_l_dark"), 170.0) / 1000.0;
    double fgLightL = getSliderValue(_T("gen_color_slider_fg_l_light"), 220.0) / 1000.0;
    double fgDarkL = getSliderValue(_T("gen_color_slider_fg_l_dark"), 920.0) / 1000.0;
    double surfaceLightOffset = getSliderValue(_T("gen_color_slider_surface_light_offset"), 25.0) / 1000.0;
    double surfaceDarkOffset = getSliderValue(_T("gen_color_slider_surface_dark_offset"), 70.0) / 1000.0;
    double accentL = getSliderValue(_T("gen_color_slider_accent_l"), 620.0) / 1000.0;
    double accentC = getSliderValue(_T("gen_color_slider_accent_c"), 195.0) / 1000.0;

    //保留色度系数，只更新明度相关参数
    double bgLightLOld = 0.0, bgDarkLOld = 0.0, bgChroma = 0.0;
    m_pThemeGenerator->GetBgParams(bgLightLOld, bgDarkLOld, bgChroma);
    m_pThemeGenerator->SetBgParams(bgLightL, bgDarkL, bgChroma);

    double fgLightLOld = 0.0, fgDarkLOld = 0.0, fgChroma = 0.0;
    m_pThemeGenerator->GetFgParams(fgLightLOld, fgDarkLOld, fgChroma);
    m_pThemeGenerator->SetFgParams(fgLightL, fgDarkL, fgChroma);

    double surfaceLightOffsetOld = 0.0, surfaceDarkOffsetOld = 0.0, surfaceChroma = 0.0;
    m_pThemeGenerator->GetSurfaceParams(surfaceLightOffsetOld, surfaceDarkOffsetOld, surfaceChroma);
    m_pThemeGenerator->SetSurfaceParams(surfaceLightOffset, surfaceDarkOffset, surfaceChroma);

    double accentLightLOld = 0.0, accentDarkLOld = 0.0, accentCOld = 0.0;
    m_pThemeGenerator->GetAccentParams(accentLightLOld, accentDarkLOld, accentCOld);
    m_pThemeGenerator->SetAccentParams(accentL, accentL, accentC);

    //更新参数显示文本
    auto setLabelText = [this](const DString& name, double fValue, const DString& format) {
        ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(name));
        if (pLabel != nullptr) {
            pLabel->SetText(ui::StringUtil::Printf(format.c_str(), fValue));
        }
    };
    setLabelText(_T("gen_color_label_bg_l_light"), bgLightL, _T("%.02f"));
    setLabelText(_T("gen_color_label_bg_l_dark"), bgDarkL, _T("%.02f"));
    setLabelText(_T("gen_color_label_fg_l_light"), fgLightL, _T("%.02f"));
    setLabelText(_T("gen_color_label_fg_l_dark"), fgDarkL, _T("%.02f"));
    setLabelText(_T("gen_color_label_surface_light_offset"), surfaceLightOffset, _T("%.03f"));
    setLabelText(_T("gen_color_label_surface_dark_offset"), surfaceDarkOffset, _T("%.03f"));
    setLabelText(_T("gen_color_label_accent_l"), accentL, _T("%.02f"));
    setLabelText(_T("gen_color_label_accent_c"), accentC, _T("%.03f"));

    bool bDarkMode = false;
    ui::Option* pOptionColorType = dynamic_cast<ui::Option*>(FindControl(_T("gen_color_radio_dark")));
    if ((pOptionColorType != nullptr) && (pOptionColorType->IsSelected())){
        bDarkMode = true;
    }

    std::string colorThemeXmlData = m_pThemeGenerator->GenerateTheme(fHue, fBaseChroma, bDarkMode);
    DString testXml = ui::StringConvert::UTF8ToT(colorThemeXmlData);
    ASSERT(!colorThemeXmlData.empty());
    UNUSED_VARIABLE(testXml);
    if (!colorThemeXmlData.empty()) {
        //加载新的主题颜色配置
        OpenColorThemeData(colorThemeXmlData);
    }
    m_colorThemeXmlData.swap(colorThemeXmlData);

    //更新主色预览区(背景色和对应的前景色)
    std::vector<std::pair<DString, DString>> previewColors;
    previewColors.push_back(std::make_pair<DString, DString>(_T("--background"), _T("--foreground")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--foreground"), _T("--background")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--accent"), _T("--accent_foreground")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--accent_foreground"), _T("--accent")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--success"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--warning"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--error"), _T("")));

    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_1"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_2"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_3"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_4"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_5"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_6"), _T("")));
    previewColors.push_back(std::make_pair<DString, DString>(_T("--surface_7"), _T("")));

    for (auto iter : previewColors) {
        const DString& bg = iter.first;
        const DString& fg = iter.second;
        ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(bg));
        if (pLabel != nullptr) {
            std::string bkColor = m_pThemeGenerator->GetGeneratedColor(ui::StringConvert::TToUTF8(bg));
            pLabel->SetBkColor(ui::StringConvert::UTF8ToT(bkColor));
            pLabel->SetToolTipText(ui::StringConvert::UTF8ToT(bkColor));
            if (!fg.empty()) {
                std::string fgColor = m_pThemeGenerator->GetGeneratedColor(ui::StringConvert::TToUTF8(fg));
                pLabel->SetStateTextColor(ui::ControlStateType::kControlStateNormal, ui::StringConvert::UTF8ToT(fgColor));
            }
        }
    }

    //更新全量颜色卡片的提示信息
    UpdateAllColors();
}

void MainForm::ShowFlyoutDemo(ui::Control* pAnchor, ui::Flyout::Placement placement)
{
    if (pAnchor == nullptr) {
        return;
    }
    //同一锚点再次点击：关闭已有浮层；不同锚点：切换到新浮层
    ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
    if (pActive != nullptr) {
        if (pActive->GetAnchor() == pAnchor) {
            pActive->Dismiss();
            return;
        }
        pActive->Dismiss();
    }

    ui::Flyout* pFlyout = new ui::Flyout(this);
    pFlyout->SetSkinFolder(GetResourcePath().ToString());
    if (!pFlyout->ShowAt(pAnchor, _T("flyout_demo.xml"), placement)) {
        return;
    }
    ui::Button* pCloseBtn = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_close")));
    if (pCloseBtn != nullptr) {
        pCloseBtn->AttachClick([pFlyout](const ui::EventArgs&) {
            pFlyout->Dismiss();
            return true;
            });
    }
}

void MainForm::ShowAllColors()
{
    ui::Box* pAllColorsBox = dynamic_cast<ui::Box*>(FindControl(_T("all_colors")));
    if (pAllColorsBox == nullptr) {
        return;
    }

    //从当前主题加载默认的配置
    ui::FilePath globalXmlFileName = ui::FilePath(ui::GlobalManager::Instance().Theme().GetGlobalXmlFileName());
    ASSERT(!globalXmlFileName.IsEmpty());
    if (globalXmlFileName.IsEmpty()) {
        return;
    }

    ui::FilePath themeFullPath = ui::GlobalManager::Instance().Theme().GetThemeRootPath();
    themeFullPath /= ui::GlobalManager::Instance().Theme().GetCurrentColorThemeInfo().m_themePath;
    themeFullPath /= globalXmlFileName;

    ui::WindowBuilder windowBuilder;
    std::string xmlFileData = windowBuilder.ReadXmlFileData(themeFullPath);
    ASSERT(!xmlFileData.empty());
    if (xmlFileData.empty()) {
        return;
    }

    ui::ThemeGenerator themeGenerator;
    if (!themeGenerator.LoadConfigFromXml(xmlFileData)) {
        return;
    }
    //所有颜色
    std::map<std::string, ui::ThemeColorConfig> themeColorConfigs = themeGenerator.GetLoadedConfigs();
    m_allColorLabels.clear();

    //按type分组
    std::map<std::string, ui::GridBox*> themeTypeGroupBoxs;
    for (const auto& pair : themeColorConfigs) {
        const std::string& colorName = pair.first;
        const ui::ThemeColorConfig& colorConfig = pair.second;

        ui::GridBox* pBox = nullptr;
        auto iter = themeTypeGroupBoxs.find(colorConfig.type);
        if (iter != themeTypeGroupBoxs.end()) {
            pBox = iter->second;
        }
        else {
            ui::GroupVBox* pGroupVBox = new ui::GroupVBox(this);
            pAllColorsBox->AddItem(pGroupVBox);
            pGroupVBox->SetClass(_T("group_box_round"));
            pGroupVBox->SetAttribute(_T("height"), _T("auto"));
            pGroupVBox->SetAttribute(_T("padding"), _T("10,20,10,10"));
            pGroupVBox->SetAttribute(_T("control_padding"), _T("false"));
            pGroupVBox->SetText(ui::StringConvert::UTF8ToT(colorConfig.type));

            pBox = new ui::GridBox(this);
            themeTypeGroupBoxs[colorConfig.type] = pBox;
            pGroupVBox->AddItem(pBox);

            pBox->SetGridHeight(40, true);
            pBox->SetGridWidth(300, true);
            pBox->SetAttribute(_T("width"), _T("auto"));
            pBox->SetAttribute(_T("height"), _T("auto"));
            pBox->SetAttribute(_T("child_margin"), _T("8"));
            pBox->SetAttribute(_T("margin"), _T("10,10,10,10"));
        }

        ui::Label* pColorLabel = new ui::Label(this);
        pBox->AddItem(pColorLabel);
        pColorLabel->SetBkColor(ui::StringConvert::UTF8ToT(colorName));
        pColorLabel->SetText(ui::StringConvert::UTF8ToT(colorName));
        pColorLabel->SetToolTipText(ui::StringConvert::UTF8ToT(colorName + " = " + colorConfig.value));
        pColorLabel->SetAttribute(_T("text_align"), _T("hcenter,vcenter"));

        //按色块亮度自动选择文字颜色，保证文字可读性
        uint8_t alpha = 255, red = 0, green = 0, blue = 0;
        if (ui::ColorConverter::ParseHexColor(colorConfig.value, alpha, red, green, blue)) {
            double luminance = ui::ColorConverter::GetRelativeLuminance(red, green, blue);
            pColorLabel->SetStateTextColor(ui::ControlStateType::kControlStateNormal,
                                           (luminance > 0.5) ? _T("#FF000000") : _T("#FFFFFFFF"));
        }
        m_allColorLabels[colorName] = pColorLabel;
    }

    //刷新生成主题后的颜色值提示
    UpdateAllColors();
}

void MainForm::UpdateAllColors()
{
    if (m_allColorLabels.empty() || m_colorThemeXmlData.empty()) {
        return;
    }
    ui::ThemeGenerator themeGenerator;
    if (!themeGenerator.LoadConfigFromXml(m_colorThemeXmlData)) {
        return;
    }
    std::map<std::string, ui::ThemeColorConfig> themeColorConfigs = themeGenerator.GetLoadedConfigs();
    for (const auto& pair : themeColorConfigs) {
        auto iter = m_allColorLabels.find(pair.first);
        if ((iter != m_allColorLabels.end()) && (iter->second != nullptr)) {
            DString tooltip = ui::StringConvert::UTF8ToT(pair.first + " = " + pair.second.value);
            iter->second->SetToolTipText(tooltip);
        }
    }
}

void MainForm::CreateThemeGenerator()
{
    if (m_pThemeGenerator != nullptr) {
        return;
    }

    //从当前主题加载默认的配置
    ui::FilePath globalXmlFileName = ui::FilePath(ui::GlobalManager::Instance().Theme().GetGlobalXmlFileName());
    ASSERT(!globalXmlFileName.IsEmpty());
    if (globalXmlFileName.IsEmpty()) {
        return;
    }

    ui::FilePath themeFullPath = ui::GlobalManager::Instance().Theme().GetThemeRootPath();
    themeFullPath /= ui::GlobalManager::Instance().Theme().GetCurrentColorThemeInfo().m_themePath;
    themeFullPath /= globalXmlFileName;

    ui::WindowBuilder windowBuilder;
    std::string xmlFileData = windowBuilder.ReadXmlFileData(themeFullPath);
    ASSERT(!xmlFileData.empty());
    if (xmlFileData.empty()) {
        return;
    }

    auto pThemeGenerator = std::make_unique<ui::ThemeGenerator>();
    if (!pThemeGenerator->LoadConfigFromXml(xmlFileData)) {
        ASSERT(0);
        return;
    }
    m_pThemeGenerator = std::move(pThemeGenerator);
}

void MainForm::InitGenParamSliders()
{
    CreateThemeGenerator();
    if (m_pThemeGenerator == nullptr) {
        return;
    }

    //绑定参数滑块事件（使用固定的callbackID，重复调用时替换之前的回调）
    const DString sliderNames[] = {
        _T("gen_color_slider_bg_l_light"),
        _T("gen_color_slider_bg_l_dark"),
        _T("gen_color_slider_fg_l_light"),
        _T("gen_color_slider_fg_l_dark"),
        _T("gen_color_slider_surface_light_offset"),
        _T("gen_color_slider_surface_dark_offset"),
        _T("gen_color_slider_accent_l"),
        _T("gen_color_slider_accent_c"),
    };
    for (const DString& sliderName : sliderNames) {
        ui::Slider* pSlider = dynamic_cast<ui::Slider*>(FindControl(sliderName));
        if (pSlider != nullptr) {
            pSlider->AttachValueChanged([this](const ui::EventArgs&) {
                OnGenColorParamChanged();
                return true;
                }, 1);
        }
    }

    //读取生成器参数，同步滑块位置与参数文本
    double bgLightL = 0.0, bgDarkL = 0.0, bgBaseChroma = 0.0;
    m_pThemeGenerator->GetBgParams(bgLightL, bgDarkL, bgBaseChroma);

    double fgLightL = 0.0, fgDarkL = 0.0, fgBaseChroma = 0.0;
    m_pThemeGenerator->GetFgParams(fgLightL, fgDarkL, fgBaseChroma);

    double surfaceLightOffset = 0.0, surfaceDarkOffset = 0.0, surfaceBaseChroma = 0.0;
    m_pThemeGenerator->GetSurfaceParams(surfaceLightOffset, surfaceDarkOffset, surfaceBaseChroma);

    double accentLightL = 0.0, accentDarkL = 0.0, accentC = 0.0;
    m_pThemeGenerator->GetAccentParams(accentLightL, accentDarkL, accentC);

    auto setSliderValue = [this](const DString& sliderName, double fValue) {
        ui::Slider* pSlider = dynamic_cast<ui::Slider*>(FindControl(sliderName));
        if (pSlider != nullptr) {
            pSlider->SetValueNoEvent(fValue);
        }
    };
    setSliderValue(_T("gen_color_slider_bg_l_light"), bgLightL * 1000.0);
    setSliderValue(_T("gen_color_slider_bg_l_dark"), bgDarkL * 1000.0);
    setSliderValue(_T("gen_color_slider_fg_l_light"), fgLightL * 1000.0);
    setSliderValue(_T("gen_color_slider_fg_l_dark"), fgDarkL * 1000.0);
    setSliderValue(_T("gen_color_slider_surface_light_offset"), surfaceLightOffset * 1000.0);
    setSliderValue(_T("gen_color_slider_surface_dark_offset"), surfaceDarkOffset * 1000.0);
    setSliderValue(_T("gen_color_slider_accent_l"), accentLightL * 1000.0);
    setSliderValue(_T("gen_color_slider_accent_c"), accentC * 1000.0);

    auto setLabelText = [this](const DString& labelName, const DString& text) {
        ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(labelName));
        if (pLabel != nullptr) {
            pLabel->SetText(text);
        }
    };
    setLabelText(_T("gen_color_label_bg_l_light"), ui::StringUtil::Printf(_T("%.02f"), bgLightL));
    setLabelText(_T("gen_color_label_bg_l_dark"), ui::StringUtil::Printf(_T("%.02f"), bgDarkL));
    setLabelText(_T("gen_color_label_fg_l_light"), ui::StringUtil::Printf(_T("%.02f"), fgLightL));
    setLabelText(_T("gen_color_label_fg_l_dark"), ui::StringUtil::Printf(_T("%.02f"), fgDarkL));
    setLabelText(_T("gen_color_label_surface_light_offset"), ui::StringUtil::Printf(_T("%.03f"), surfaceLightOffset));
    setLabelText(_T("gen_color_label_surface_dark_offset"), ui::StringUtil::Printf(_T("%.03f"), surfaceDarkOffset));
    setLabelText(_T("gen_color_label_accent_l"), ui::StringUtil::Printf(_T("%.02f"), accentLightL));
    setLabelText(_T("gen_color_label_accent_c"), ui::StringUtil::Printf(_T("%.03f"), accentC));
}

void MainForm::ResetGenParams()
{
    CreateThemeGenerator();
    if (m_pThemeGenerator == nullptr) {
        return;
    }
    m_pThemeGenerator->ResetParams();
    InitGenParamSliders();
    OnGenColorParamChanged();
}

void MainForm::SaveXmlDataToFile()
{
    if (m_colorThemeXmlData.empty()) {
        //尚未生成主题数据时，先按当前参数生成一次
        OnGenColorParamChanged();
    }
    if (m_colorThemeXmlData.empty()) {
        return;
    }

    std::vector<ui::FileDialog::FileType> fileTypes;
    ui::FileDialog::FileType fileType;
    fileType.szName = _T("XML files");
    fileType.szExt = _T("*.xml");
    fileTypes.push_back(fileType);

    ui::FilePath filePath = ui::GlobalManager::Instance().Theme().GetThemeRootPath();
    filePath /= ui::FilePath(_T("color_theme_generated.xml"));

    ui::FileDialog fileDialog;
    if (!fileDialog.BrowseForFile(this, filePath, false, fileTypes, 0, _T("xml"), _T("color_theme_generated.xml"))) {
        return;
    }
    ui::FileUtil::WriteFileData(filePath, m_colorThemeXmlData);
}

void MainForm::ShowLightDarkColorCompare()
{
    ui::Box* pContainer = dynamic_cast<ui::Box*>(FindControl(_T("light_dark_colors_box")));
    if (pContainer == nullptr) {
        return;
    }

    //读取指定主题的全部颜色配置
    auto readThemeConfigs = [](const DString& themeName, std::map<std::string, ui::ThemeColorConfig>& colorConfigs) -> bool {
        ui::FilePath globalXmlFileName = ui::FilePath(ui::GlobalManager::Instance().Theme().GetGlobalXmlFileName());
        if (globalXmlFileName.IsEmpty()) {
            return false;
        }
        ui::FilePath themeFullPath = ui::GlobalManager::Instance().Theme().GetThemeRootPath();
        themeFullPath /= ui::FilePath(themeName);
        themeFullPath /= globalXmlFileName;

        ui::WindowBuilder windowBuilder;
        std::string xmlFileData = windowBuilder.ReadXmlFileData(themeFullPath);
        if (xmlFileData.empty()) {
            return false;
        }
        ui::ThemeGenerator themeGenerator;
        if (!themeGenerator.LoadConfigFromXml(xmlFileData)) {
            return false;
        }
        colorConfigs = themeGenerator.GetLoadedConfigs();
        return !colorConfigs.empty();
    };

    std::map<std::string, ui::ThemeColorConfig> lightConfigs;
    std::map<std::string, ui::ThemeColorConfig> darkConfigs;
    if (!readThemeConfigs(DUILIB_LIGHT_COLOR_PATH, lightConfigs)) {
        return;
    }
    if (!readThemeConfigs(DUILIB_DARK_COLOR_PATH, darkConfigs)) {
        return;
    }

    auto createLabel = [this](const DString& text, int32_t nWidth, const DString& bkColor = _T("")) -> ui::Label* {
        ui::Label* pLabel = new ui::Label(this);
        pLabel->SetText(text);
        pLabel->SetAttribute(_T("width"), ui::StringUtil::Printf(_T("%d"), nWidth));
        pLabel->SetAttribute(_T("height"), _T("22"));
        pLabel->SetAttribute(_T("valign"), _T("center"));
        if (!bkColor.empty()) {
            pLabel->SetBkColor(bkColor);
        }
        return pLabel;
    };

    //表头
    ui::HBox* pHeaderRow = new ui::HBox(this);
    pContainer->AddItem(pHeaderRow);
    pHeaderRow->SetAttribute(_T("height"), _T("28"));
    pHeaderRow->SetAttribute(_T("child_margin"), _T("4"));
    pHeaderRow->AddItem(createLabel(_T("Color Name"), 300));
    pHeaderRow->AddItem(createLabel(_T("Light"), 70));
    pHeaderRow->AddItem(createLabel(_T("Light HEX"), 120));
    pHeaderRow->AddItem(createLabel(_T("Dark"), 70));
    pHeaderRow->AddItem(createLabel(_T("Dark HEX"), 120));

    //按type分组，并以浅色主题的颜色顺序为基准
    std::map<std::string, std::vector<const ui::ThemeColorConfig*>> typeGroups;
    for (const auto& pair : lightConfigs) {
        typeGroups[pair.second.type].push_back(&pair.second);
    }
    for (auto& group : typeGroups) {
        std::sort(group.second.begin(), group.second.end(), [](const ui::ThemeColorConfig* pLeft, const ui::ThemeColorConfig* pRight) {
            return pLeft->node_order < pRight->node_order;
            });

        //分组标题
        ui::Label* pTypeLabel = createLabel(ui::StringConvert::UTF8ToT(group.first), 700);
        pTypeLabel->SetAttribute(_T("font"), _T("system_bold_14"));
        pTypeLabel->SetAttribute(_T("margin"), _T("0,8,0,0"));
        pContainer->AddItem(pTypeLabel);

        for (const ui::ThemeColorConfig* pLightConfig : group.second) {
            if (pLightConfig == nullptr) {
                continue;
            }
            auto darkIter = darkConfigs.find(pLightConfig->name);
            const DString darkValue = (darkIter != darkConfigs.end()) ?
                ui::StringConvert::UTF8ToT(darkIter->second.value) : DString(_T("-"));
            const DString lightValue = ui::StringConvert::UTF8ToT(pLightConfig->value);

            ui::HBox* pRow = new ui::HBox(this);
            pContainer->AddItem(pRow);
            pRow->SetAttribute(_T("height"), _T("24"));
            pRow->SetAttribute(_T("child_margin"), _T("4"));
            pRow->AddItem(createLabel(ui::StringConvert::UTF8ToT(pLightConfig->name), 300));
            pRow->AddItem(createLabel(_T(""), 70, lightValue));
            pRow->AddItem(createLabel(lightValue, 120));
            pRow->AddItem(createLabel(_T(""), 70, darkValue));
            pRow->AddItem(createLabel(darkValue, 120));
        }
    }
}

void MainForm::ShowGlobalConfigInfo()
{
    ui::FilePath globalXmlFileName = ui::FilePath(ui::GlobalManager::Instance().Theme().GetGlobalXmlFileName());
    if (globalXmlFileName.IsEmpty()) {
        return;
    }
    //读取默认主题的 global.xml（包含 Class/Font/Var/Alias 等完整全局配置）
    ui::FilePath themeFullPath = ui::GlobalManager::Instance().Theme().GetThemeRootPath();
    themeFullPath /= ui::GlobalManager::Instance().Theme().GetDefaultThemePath();
    themeFullPath /= globalXmlFileName;

    ui::WindowBuilder windowBuilder;
    std::string xmlFileData = windowBuilder.ReadXmlFileData(themeFullPath);
    if (xmlFileData.empty()) {
        return;
    }

    pugi::xml_document doc;
    if (!doc.load_buffer(xmlFileData.data(), xmlFileData.size())) {
        return;
    }
    pugi::xml_node root = doc.child(PUGIXML_TEXT("Global"));
    if (!root) {
        return;
    }

    auto addInfoLabel = [this](ui::Box* pBox, const DString& text, int32_t nWidth, const DString& bkColor = _T("")) -> ui::Label* {
        ui::Label* pLabel = new ui::Label(this);
        pLabel->SetText(text);
        if (nWidth > 0) {
            pLabel->SetAttribute(_T("width"), ui::StringUtil::Printf(_T("%d"), nWidth));
        }
        pLabel->SetAttribute(_T("height"), _T("22"));
        pLabel->SetAttribute(_T("valign"), _T("center"));
        if (!bkColor.empty()) {
            pLabel->SetBkColor(bkColor);
        }
        pBox->AddItem(pLabel);
        return pLabel;
    };

    //Theme + ThemeMeta
    ui::Box* pMetaBox = dynamic_cast<ui::Box*>(FindControl(_T("global_meta_box")));
    if (pMetaBox != nullptr) {
        pugi::xml_node themeNode = root.child(PUGIXML_TEXT("Theme"));
        DString themeInfo;
        if (themeNode) {
            themeInfo = ui::StringUtil::Printf(_T("Theme: name=%s, type=%s, style=%s, version=%s"),
                                               themeNode.attribute(PUGIXML_TEXT("name")).as_string(),
                                               themeNode.attribute(PUGIXML_TEXT("type")).as_string(),
                                               themeNode.attribute(PUGIXML_TEXT("style")).as_string(),
                                               themeNode.attribute(PUGIXML_TEXT("version")).as_string());
            addInfoLabel(pMetaBox, themeInfo, 0);
        }
        pugi::xml_node themeMetaNode = root.child(PUGIXML_TEXT("ThemeMeta"));
        for (pugi::xml_node propertyNode : themeMetaNode.children(PUGIXML_TEXT("Property"))) {
            DString propertyInfo = ui::StringUtil::Printf(_T("ThemeMeta.%s = %s"),
                                                          propertyNode.attribute(PUGIXML_TEXT("name")).as_string(),
                                                          propertyNode.attribute(PUGIXML_TEXT("value")).as_string());
            addInfoLabel(pMetaBox, propertyInfo, 0);
        }

        //字体文件与默认字体族
        for (pugi::xml_node fontFileNode : root.children(PUGIXML_TEXT("FontFile"))) {
            DString fontFileInfo = ui::StringUtil::Printf(_T("FontFile: %s (%s)"),
                                                          fontFileNode.attribute(PUGIXML_TEXT("file")).as_string(),
                                                          fontFileNode.attribute(PUGIXML_TEXT("desc")).as_string());
            addInfoLabel(pMetaBox, fontFileInfo, 0);
        }
        pugi::xml_node defaultFamilyNode = root.child(PUGIXML_TEXT("DefaultFontFamilyNames"));
        if (defaultFamilyNode) {
            addInfoLabel(pMetaBox, ui::StringUtil::Printf(_T("DefaultFontFamilyNames: windows=%s, macos=%s, linux=%s"),
                                                          defaultFamilyNode.attribute(PUGIXML_TEXT("windows")).as_string(),
                                                          defaultFamilyNode.attribute(PUGIXML_TEXT("macos")).as_string(),
                                                          defaultFamilyNode.attribute(PUGIXML_TEXT("linux")).as_string()), 0);
        }
        pugi::xml_node fallbackFamilyNode = root.child(PUGIXML_TEXT("FallbackFontFamilyNames"));
        if (fallbackFamilyNode) {
            addInfoLabel(pMetaBox, ui::StringUtil::Printf(_T("FallbackFontFamilyNames: windows=%s, macos=%s, linux=%s"),
                                                          fallbackFamilyNode.attribute(PUGIXML_TEXT("windows")).as_string(),
                                                          fallbackFamilyNode.attribute(PUGIXML_TEXT("macos")).as_string(),
                                                          fallbackFamilyNode.attribute(PUGIXML_TEXT("linux")).as_string()), 0);
        }
    }

    //Var
    ui::Box* pVarsBox = dynamic_cast<ui::Box*>(FindControl(_T("global_vars_box")));
    if (pVarsBox != nullptr) {
        auto varRange = root.children(PUGIXML_TEXT("Var"));
        addInfoLabel(pVarsBox, ui::StringUtil::Printf(_T("Variables (%d)"), static_cast<int32_t>(std::distance(varRange.begin(), varRange.end()))), 0);
        for (pugi::xml_node varNode : varRange) {
            ui::HBox* pRow = new ui::HBox(this);
            pVarsBox->AddItem(pRow);
            pRow->SetAttribute(_T("height"), _T("22"));
            addInfoLabel(pRow, varNode.attribute(PUGIXML_TEXT("name")).as_string(), 320);
            addInfoLabel(pRow, varNode.attribute(PUGIXML_TEXT("value")).as_string(), 0);
        }
    }

    //Alias
    ui::Box* pAliasBox = dynamic_cast<ui::Box*>(FindControl(_T("global_alias_box")));
    if (pAliasBox != nullptr) {
        auto aliasRange = root.children(PUGIXML_TEXT("Alias"));
        addInfoLabel(pAliasBox, ui::StringUtil::Printf(_T("Aliases (%d)"), static_cast<int32_t>(std::distance(aliasRange.begin(), aliasRange.end()))), 0);
        for (pugi::xml_node aliasNode : aliasRange) {
            DString aliasName = aliasNode.attribute(PUGIXML_TEXT("name")).as_string();
            DString aliasValue = aliasNode.attribute(PUGIXML_TEXT("value")).as_string();
            ui::HBox* pRow = new ui::HBox(this);
            pAliasBox->AddItem(pRow);
            pRow->SetAttribute(_T("height"), _T("24"));
            pRow->SetAttribute(_T("child_margin"), _T("4"));
            //仅当别名指向有效的颜色时才显示色块（部分别名指向的是Class，如"listitem"）
            DString bkColor;
            if (!ui::GlobalManager::Instance().Color().GetColor(aliasName).IsEmpty()) {
                bkColor = aliasName;
            }
            addInfoLabel(pRow, _T(""), 40, bkColor);
            addInfoLabel(pRow, aliasName, 200);
            addInfoLabel(pRow, DString(_T("-> ")) + aliasValue, 260);
        }
    }

    //Font
    ui::Box* pFontBox = dynamic_cast<ui::Box*>(FindControl(_T("global_font_box")));
    if (pFontBox != nullptr) {
        auto fontRange = root.children(PUGIXML_TEXT("Font"));
        addInfoLabel(pFontBox, ui::StringUtil::Printf(_T("Fonts (%d)"), static_cast<int32_t>(std::distance(fontRange.begin(), fontRange.end()))), 0);
        for (pugi::xml_node fontNode : fontRange) {
            DString fontId = fontNode.attribute(PUGIXML_TEXT("id")).as_string();
            DString fontName = fontNode.attribute(PUGIXML_TEXT("name")).as_string();
            int32_t fontSize = fontNode.attribute(PUGIXML_TEXT("size")).as_int();
            bool bBold = fontNode.attribute(PUGIXML_TEXT("bold")).as_bool();
            bool bItalic = fontNode.attribute(PUGIXML_TEXT("italic")).as_bool();

            ui::HBox* pRow = new ui::HBox(this);
            pFontBox->AddItem(pRow);
            pRow->SetAttribute(_T("height"), _T("26"));
            pRow->SetAttribute(_T("child_margin"), _T("4"));

            DString sampleText = ui::StringUtil::Printf(_T("Aa 0123 - %s %d%s%s"), fontName.c_str(), fontSize,
                                                        bBold ? _T(" Bold") : _T(""),
                                                        bItalic ? _T(" Italic") : _T(""));
            ui::Label* pSampleLabel = addInfoLabel(pRow, sampleText, 360);
            pSampleLabel->SetAttribute(_T("font"), fontId);
            addInfoLabel(pRow, fontId, 220);
        }
    }
}

void MainForm::InitPreviewControls()
{
    //ToolBar 动态工具项
    ui::ToolBar* pToolBar = dynamic_cast<ui::ToolBar*>(FindControl(_T("preview_toolbar")));
    if (pToolBar != nullptr) {
        pToolBar->AddToolButton(_T("preview_tb_new"), _T("New"), _T("toolbar_button"));
        pToolBar->AddToolButton(_T("preview_tb_open"), _T("Open"), _T("toolbar_button"));
        pToolBar->AddToolButton(_T("preview_tb_save"), _T("Save"), _T("toolbar_button"));
        pToolBar->AddToolSeparator(_T("preview_tb_sep1"), _T("toolbar_separator"));
        pToolBar->AddToolCheckBox(_T("preview_tb_check"), _T("Check"), true, _T("toolbar_check"));
        pToolBar->AddToolSeparator(_T("preview_tb_sep2"), _T("toolbar_separator"));
        pToolBar->AddToolButton(_T("preview_tb_help"), _T("Help"), _T("toolbar_button"));
    }

    //TreeGrid 数据
    ui::TreeGrid* pTreeGrid = dynamic_cast<ui::TreeGrid*>(FindControl(_T("preview_tree_grid")));
    if (pTreeGrid != nullptr) {
        ui::TreeGridNode* pRoot = pTreeGrid->AddNode(nullptr);
        if (pRoot != nullptr) {
            pRoot->SetCellText(0, _T("Project"));
            pRoot->SetCellText(1, _T("-"));
            pRoot->SetCellText(2, _T("Folder"));

            ui::TreeGridNode* pChild1 = pTreeGrid->AddNode(pRoot);
            if (pChild1 != nullptr) {
                pChild1->SetCellText(0, _T("duilib"));
                pChild1->SetCellText(1, _T("2.1 MB"));
                pChild1->SetCellText(2, _T("Folder"));
            }
            ui::TreeGridNode* pChild2 = pTreeGrid->AddNode(pRoot);
            if (pChild2 != nullptr) {
                pChild2->SetCellText(0, _T("examples"));
                pChild2->SetCellText(1, _T("1.4 MB"));
                pChild2->SetCellText(2, _T("Folder"));
            }
            ui::TreeGridNode* pChild3 = pTreeGrid->AddNode(pRoot);
            if (pChild3 != nullptr) {
                pChild3->SetCellText(0, _T("README.md"));
                pChild3->SetCellText(1, _T("8 KB"));
                pChild3->SetCellText(2, _T("File"));
            }
            pTreeGrid->SetNodeExpand(pRoot, true, false);
        }
    }

    ui::TreeGrid* pTreeGridCheck = dynamic_cast<ui::TreeGrid*>(FindControl(_T("preview_tree_grid_check")));
    if (pTreeGridCheck != nullptr) {
        ui::TreeGridNode* pRoot = pTreeGridCheck->AddNode(nullptr);
        if (pRoot != nullptr) {
            pRoot->SetCellText(0, _T("Components"));
            pRoot->SetCellText(1, _T("-"));
            pRoot->SetCellText(2, _T("Folder"));

            for (int32_t nIndex = 1; nIndex <= 4; ++nIndex) {
                ui::TreeGridNode* pChild = pTreeGridCheck->AddNode(pRoot);
                if (pChild != nullptr) {
                    pChild->SetCellText(0, ui::StringUtil::Printf(_T("Component %d"), nIndex));
                    pChild->SetCellText(1, ui::StringUtil::Printf(_T("%d KB"), nIndex * 16));
                    pChild->SetCellText(2, _T("File"));
                }
            }
            pTreeGridCheck->SetNodeExpand(pRoot, true, false);
        }
    }

    //滚动条常显，便于预览配色
    ui::ScrollBox* pVScrollBox = dynamic_cast<ui::ScrollBox*>(FindControl(_T("preview_vscroll_box")));
    if (pVScrollBox != nullptr) {
        ui::ScrollBar* pVScrollBar = pVScrollBox->GetVScrollBar();
        if (pVScrollBar != nullptr) {
            pVScrollBar->SetAttribute(_T("auto_hide_scroll"), _T("false"));
            pVScrollBar->SetVisible(true);
        }
    }
    ui::ScrollBox* pHScrollBox = dynamic_cast<ui::ScrollBox*>(FindControl(_T("preview_hscroll_box")));
    if (pHScrollBox != nullptr) {
        ui::ScrollBar* pHScrollBar = pHScrollBox->GetHScrollBar();
        if (pHScrollBar != nullptr) {
            pHScrollBar->SetAttribute(_T("auto_hide_scroll"), _T("false"));
            pHScrollBar->SetVisible(true);
        }
    }
}

