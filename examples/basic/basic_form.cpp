#include "basic_form.h"

BasicForm::BasicForm()
{
}

BasicForm::~BasicForm()
{
}

DString BasicForm::GetSkinFolder()
{
    return _T("basic");
}

DString BasicForm::GetSkinFile()
{
    return _T("basic.xml");
}

void BasicForm::OnInitWindow()
{
    ui::Button* pButton = (ui::Button*)FindControl(L"test");
    pButton->AttachClick([this](const ui::EventArgs& ){
        ui::RichEdit* pEdit = (ui::RichEdit*)FindControl(L"RichEdit");
        pEdit->LineScroll(423);
        //int32_t n = pEdit->LineIndex(423);
        //pEdit->EnsureCharVisible(n);
        return true;
        });

    std::vector<uint8_t> fileData;
    ui::FileUtil::ReadFileData(ui::FilePath(L"D:\\1.h"), fileData);
    fileData.push_back(0);
    fileData.push_back(0);

    DString text;
#ifdef DUILIB_UNICODE    
    text = ui::StringUtil::UTF8ToUTF16((const char*)fileData.data());
#else
    text = (const char*)fileData.data();
#endif
    ui::RichEdit* pEdit = (ui::RichEdit*)FindControl(L"RichEdit");
    //pEdit->SetText(text);

    __super::OnInitWindow();

}
