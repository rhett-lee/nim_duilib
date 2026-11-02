#include "MainForm.h"

MainForm::MainForm():
    m_pLangDisplayName(nullptr),
    m_pLangFileName(nullptr)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("MultiLang");
}

DString MainForm::GetSkinFile()
{
    return _T("MultiLang.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pLangDisplayName = dynamic_cast<ui::Label*>(FindControl(_T("lang_display_name")));
    m_pLangFileName = dynamic_cast<ui::Label*>(FindControl(_T("lang_file_name")));

    //窗口初始化完成，先按当前语言填充一次
    UpdateLanguageInfo();
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();

    //语言已经切换完成，重新获取动态文本
    UpdateLanguageInfo();
    return bRet;
}

void MainForm::UpdateLanguageInfo()
{
    ui::GlobalManager& globalManager = ui::GlobalManager::Instance();
    if (m_pLangDisplayName != nullptr) {
        //语言显示名称来自各语言文件中的 LANGUAGE_DISPLAY_NAME，与语言切换菜单里显示的一致
        m_pLangDisplayName->SetText(globalManager.Lang().GetStringByID(DUILIB_LANGUAGE_DISPLAY_NAME));
    }
    if (m_pLangFileName != nullptr) {
        m_pLangFileName->SetText(globalManager.GetLanguageFileName());
    }
}
