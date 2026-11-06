#include "MainForm.h"
#include <list>
#include <map>

/** 用户设置文件中的键值对
*/
typedef std::map<std::string, std::string> SettingsMap;

/** 解析用户设置文本中的一行：键=值（以'#'或者';'开头的行为注释行）
*/
void ParseSettingsText(const std::string& text, SettingsMap& settings)
{
    settings.clear();
    std::list<std::string> lines = ui::StringUtil::Split(text, "\n");
    for (std::string& line : lines) {
        ui::StringUtil::Trim(line);
        if (line.empty() || (line[0] == '#') || (line[0] == ';')) {
            continue;
        }
        const size_t nPos = line.find('=');
        if (nPos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, nPos);
        std::string value = line.substr(nPos + 1);
        ui::StringUtil::Trim(key);
        ui::StringUtil::Trim(value);
        if (!key.empty()) {
            settings[key] = value;
        }
    }
}

/** 读取用户设置文件（文本格式：每行一个"键=值"，UTF-8编码）
*/
void ReadSettingsFile(const ui::FilePath& filePath, SettingsMap& settings)
{
    settings.clear();
    std::vector<uint8_t> fileData;
    if (!ui::FileUtil::ReadFileData(filePath, fileData) || fileData.empty()) {
        return;
    }
    std::string text(fileData.begin(), fileData.end());
    //去除UTF-8 BOM
    if ((text.size() >= 3) &&
        (static_cast<uint8_t>(text[0]) == 0xEF) &&
        (static_cast<uint8_t>(text[1]) == 0xBB) &&
        (static_cast<uint8_t>(text[2]) == 0xBF)) {
        text.erase(0, 3);
    }
    ParseSettingsText(text, settings);
}

/** 从用户设置中读取一个整数
*/
int32_t GetSettingInt(const SettingsMap& settings, const std::string& key, int32_t nDefault)
{
    auto iter = settings.find(key);
    if (iter == settings.end()) {
        return nDefault;
    }
    return ui::StringUtil::StringToInt32(iter->second.c_str());
}

/** 从用户设置中读取一个字符串
*/
std::string GetSettingString(const SettingsMap& settings, const std::string& key, const std::string& defaultText)
{
    auto iter = settings.find(key);
    if (iter == settings.end()) {
        return defaultText;
    }
    return iter->second;
}

/** 把整数写入用户设置文本
*/
void AppendSettingInt(std::string& settingsText, const std::string& key, int32_t nValue)
{
    settingsText += key;
    settingsText += '=';
    settingsText += std::to_string(nValue);
    settingsText += '\n';
}

/** 把字符串写入用户设置文本（需要先转换为UTF-8）
*/
void AppendSettingString(std::string& settingsText, const std::string& key, const std::string& value)
{
    settingsText += key;
    settingsText += '=';
    settingsText += value;
    settingsText += '\n';
}

/** 设置文件的当前版本号
*/
const int32_t kSettingsVersion = 2;

/** 把旧版本的设置迁移到当前版本
*   以后新增设置键时，只需在这里追加一条版本规则，LoadSettings 中不用再判断版本号
*/
void MigrateSettings(SettingsMap& settings)
{
    const int32_t nVersion = GetSettingInt(settings, "settings_version", 0);

    //版本2：开始保存左侧面板宽度、缩放比例、富文本模式；旧版本中同名的设置不可信，丢弃后改用默认值
    if (nVersion < 2) {
        settings.erase("left_panel_width");
        settings.erase("zoom_percent");
        settings.erase("rich_text");
    }
}

ui::FilePath MainForm::GetSettingsFilePath() const
{
    //保存在程序目录下的user子目录中（跨平台，且不依赖系统的用户配置目录）
    ui::FilePath filePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    filePath.JoinFilePath(ui::FilePath(_T("user")));
    filePath.JoinFilePath(ui::FilePath(_T("RichEdit_settings.ini")));
    return filePath;
}

void MainForm::LoadSettings()
{
    SettingsMap settings;
    ReadSettingsFile(GetSettingsFilePath(), settings);
    //旧版本的设置先统一迁移到当前版本
    MigrateSettings(settings);

    //1. 左侧面板的宽度（窗口的位置与大小不保存，始终使用XML中的默认值，便于修改XML后立即生效）
    const int32_t nLeftPanelWidth = GetSettingInt(settings, "left_panel_width", -1);
    if (nLeftPanelWidth > 0) {
        ui::Control* pLeftPanel = FindControl(_T("left_panel"));
        if (pLeftPanel != nullptr) {
            const int32_t nMinWidth = 60;
            const int32_t nMaxWidth = 2048;
            pLeftPanel->SetFixedWidth(ui::UiFixedInt(std::max(nMinWidth, std::min(nLeftPanelWidth, nMaxWidth))), true, true);
        }
    }

    if (m_pRichEdit != nullptr) {
        //2. 缩放比例
        const int32_t nZoomPercent = GetSettingInt(settings, "zoom_percent", 100);
        if (nZoomPercent > 0) {
            m_pRichEdit->SetZoomPercent((uint32_t)nZoomPercent);
        }

        //3. 自动换行
        const int32_t nWordWrap = GetSettingInt(settings, "word_wrap", -1);
        if (nWordWrap >= 0) {
            m_pRichEdit->SetWordWrap(nWordWrap != 0);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
            m_pRichEdit->SetAttribute(_T("hscrollbar"), (nWordWrap != 0) ? _T("false") : _T("true"));
#endif
        }

        //4. 富文本模式（该模式缺少跨平台兼容性，旧版本设置不恢复，默认关闭）
        const int32_t nRichText = GetSettingInt(settings, "rich_text", -1);
        if (nRichText >= 0) {
            m_bUpdatingRichTextCheck = true;
            m_bLoadingDocument = true;
            m_pRichEdit->SetRichText(nRichText != 0);
            m_bLoadingDocument = false;
            m_bUpdatingRichTextCheck = false;
        }

        //同步复选框的选中状态
        ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_word_wrap")));
        if (pCheckBox != nullptr) {
            pCheckBox->SetSelected(m_pRichEdit->IsWordWrap());
        }
        pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_rich_text")));
        if (pCheckBox != nullptr) {
            pCheckBox->SetSelected(m_pRichEdit->IsRichText());
        }
    }

    //5. 字体名称
    const std::string fontNameUtf8 = GetSettingString(settings, "font_name", std::string());
    if (!fontNameUtf8.empty()) {
        SetFontName(ui::StringConvert::UTF8ToT(fontNameUtf8));
    }
}

void MainForm::SaveSettings() const
{
    std::string settingsText;
    settingsText += "# RichEdit 示例的用户设置（程序自动生成，删除该文件后恢复默认设置）\n";
    AppendSettingInt(settingsText, "settings_version", kSettingsVersion);

    //1. 左侧面板的宽度（保存未缩放的值；窗口的位置与大小不保存）
    int32_t nLeftPanelWidth = -1;
    ui::Control* pLeftPanel = FindControl(_T("left_panel"));
    if (pLeftPanel != nullptr) {
        nLeftPanelWidth = Dpi().GetUnscaleInt(pLeftPanel->GetWidth());
    }
    AppendSettingInt(settingsText, "left_panel_width", nLeftPanelWidth);

    if (m_pRichEdit != nullptr) {
        //2. 缩放比例
        AppendSettingInt(settingsText, "zoom_percent", (int32_t)m_pRichEdit->GetZoomPercent());

        //3. 自动换行
        AppendSettingInt(settingsText, "word_wrap", m_pRichEdit->IsWordWrap() ? 1 : 0);

        //4. 富文本模式
        AppendSettingInt(settingsText, "rich_text", m_pRichEdit->IsRichText() ? 1 : 0);
    }

    //5. 字体名称
    AppendSettingString(settingsText, "font_name", ui::StringConvert::TToUTF8(m_lastFontName));

    const ui::FilePath filePath = GetSettingsFilePath();
    //创建设置文件所在的目录（不存在时自动创建）
    ui::FilePathUtil::CreateDirectories(filePath.GetParentPath().ToString());
    const std::vector<uint8_t> fileData(settingsText.begin(), settingsText.end());
    ui::FileUtil::WriteFileData(filePath, fileData);
}
