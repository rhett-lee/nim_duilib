#include "MainForm.h"

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
            ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
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
        pGenColorGroupBox->SetVisible(false);
    }
    ui::Control* pGenColorGroupBoxSplit = FindControl(_T("gen_color_groupbox_split"));
    if (pGenColorGroupBoxSplit != nullptr) {
        pGenColorGroupBoxSplit->SetVisible(false);
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
            if (!m_colorThemeXmlData.empty()) {
                ui::Clipboard::SetClipboardText(m_colorThemeXmlData);
            }
            return true;
            });
    }
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

    if (m_pThemeGenerator == nullptr) {
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

        m_pThemeGenerator = std::make_unique<ui::ThemeGenerator>();
        if (!m_pThemeGenerator->LoadConfigFromXml(xmlFileData)) {
            ASSERT(0);
            m_pThemeGenerator.reset();
            return;
        }
    }

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
    previewColors.push_back(std::make_pair<DString, DString>(_T("--accent"), _T("--accent_foreground")));
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
        pColorLabel->SetToolTipText(ui::StringConvert::UTF8ToT(colorName));
        pColorLabel->SetAttribute(_T("text_align"), _T("hcenter,vcenter"));
    }
}

