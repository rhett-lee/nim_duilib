#include "MyForm.h"

MyForm::MyForm()
{
}

MyForm::~MyForm()
{
}

DString MyForm::GetSkinFolder()
{
    return _T("my_skin");
}

DString MyForm::GetSkinFile()
{
    return _T("my_form.xml");
}

void MyForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 查找控件务必 dynamic_cast 并判空；引用不存在的 name 不会报错，只会拿到 nullptr
    if (auto* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")))) {
        btn->AttachClick([this](const ui::EventArgs& args) {
            // 取发送者控件用 GetSender()，不是 pSender（pSender 是 private）
            ui::Control* pSender = args.GetSender();
            if (pSender == nullptr) {
                return true;
            }
            return true;
        });
    }
}
