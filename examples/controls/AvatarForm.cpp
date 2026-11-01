#include "AvatarForm.h"

AvatarForm::AvatarForm()
{
}

AvatarForm::~AvatarForm()
{
}

DString AvatarForm::GetSkinFolder()
{
    return _T("controls");
}

DString AvatarForm::GetSkinFile()
{
    return _T("avatar.xml");
}

void AvatarForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    m_pDynamic = dynamic_cast<ui::Avatar*>(FindControl(_T("avatar_dynamic")));
    BindDynamicDemo();
}

void AvatarForm::BindDynamicDemo()
{
    ui::Button* pBtnSize = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_size")));
    if (pBtnSize != nullptr) {
        pBtnSize->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnChangeSize();
            return true;
            });
    }

    ui::Button* pBtnShape = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_shape")));
    if (pBtnShape != nullptr) {
        pBtnShape->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnChangeShape();
            return true;
            });
    }

    ui::Button* pBtnText = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_text")));
    if (pBtnText != nullptr) {
        pBtnText->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnChangeText();
            return true;
            });
    }

    ui::Button* pBtnImage = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_image")));
    if (pBtnImage != nullptr) {
        pBtnImage->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnChangeImage();
            return true;
            });
    }

    ui::Button* pBtnClear = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_clear")));
    if (pBtnClear != nullptr) {
        pBtnClear->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnClearContent();
            return true;
            });
    }

    ui::Button* pBtnPalette = dynamic_cast<ui::Button*>(FindControl(_T("btn_avatar_palette")));
    if (pBtnPalette != nullptr) {
        pBtnPalette->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnTogglePalette();
            return true;
            });
    }
}

void AvatarForm::OnChangeSize()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    const int32_t sizeList[] = { 24, 32, 40, 64 };
    const size_t nSizeCount = sizeof(sizeList) / sizeof(sizeList[0]);
    m_nSizeIndex = (m_nSizeIndex + 1) % nSizeCount;
    m_pDynamic->SetSize(sizeList[m_nSizeIndex]);
    UpdateStateText();
}

void AvatarForm::OnChangeShape()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    const ui::AvatarShape shape = (m_pDynamic->GetShape() == ui::AvatarShape::kCircle)
        ? ui::AvatarShape::kSquare : ui::AvatarShape::kCircle;
    m_pDynamic->SetShape(shape);
    UpdateStateText();
}

void AvatarForm::OnChangeText()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    //文字优先级低于图片：切换文字时先清掉图片，否则看不出变化
    m_bShowImage = false;
    m_pDynamic->SetImage(DString());

    const DString textList[] = { _T("Tom"), _T("张伟"), _T("Lucy"), _T("李娜") };
    const size_t nTextCount = sizeof(textList) / sizeof(textList[0]);
    m_nTextIndex = (m_nTextIndex + 1) % nTextCount;
    m_pDynamic->SetText(textList[m_nTextIndex]);
    UpdateStateText();
}

void AvatarForm::OnChangeImage()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    m_bShowImage = !m_bShowImage;
    if (m_bShowImage) {
        //图片按 cover 填充，并按头像形状裁剪；路径相对于主题根目录
        m_pDynamic->SetImage(_T("file='public/avatar/avatar_picture_demo.svg'"));
    }
    else {
        m_pDynamic->SetImage(DString());
    }
    UpdateStateText();
}

void AvatarForm::OnClearContent()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    m_bShowImage = false;
    m_pDynamic->SetImage(DString());
    m_pDynamic->SetIcon(DString());
    m_pDynamic->SetTextId(DString());
    m_pDynamic->SetText(DString());
    UpdateStateText();
}

void AvatarForm::OnTogglePalette()
{
    if (m_pDynamic == nullptr) {
        return;
    }
    m_pDynamic->SetAutoPalette(!m_pDynamic->IsAutoPalette());
    UpdateStateText();
}

void AvatarForm::UpdateStateText()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("avatar_state")));
    if ((pLabel == nullptr) || (m_pDynamic == nullptr)) {
        return;
    }

    const DString strSize = ui::StringUtil::Printf(_T("%d"), (int)m_pDynamic->GetSize());
    const DString strShape = GetLangText((m_pDynamic->GetShape() == ui::AvatarShape::kSquare)
        ? _T("STRID_AVATAR_LABEL_SQUARE") : _T("STRID_AVATAR_LABEL_CIRCLE"));

    //内容按绘制优先级判定：图片 > 图标 > 文字 > 空
    DString strContent;
    if (!m_pDynamic->GetImage().empty()) {
        strContent = GetLangText(_T("STRID_AVATAR_LABEL_IMAGE"));
    }
    else if (!m_pDynamic->GetIcon().empty()) {
        strContent = GetLangText(_T("STRID_AVATAR_LABEL_ICON"));
    }
    else if (!m_pDynamic->GetText().empty()) {
        strContent = GetLangText(_T("STRID_AVATAR_STATE_CONTENT_TEXT"));
        ui::StringUtil::ReplaceAll(_T("{0}"), m_pDynamic->GetText(), strContent);
    }
    else {
        strContent = GetLangText(_T("STRID_AVATAR_STATE_CONTENT_EMPTY"));
    }

    const DString strPalette = GetLangText(m_pDynamic->IsAutoPalette()
        ? _T("STRID_AVATAR_STATE_PALETTE_ON") : _T("STRID_AVATAR_STATE_PALETTE_OFF"));

    DString strState = GetLangText(_T("STRID_AVATAR_STATE_FORMAT"));
    ui::StringUtil::ReplaceAll(_T("{0}"), strSize, strState);
    ui::StringUtil::ReplaceAll(_T("{1}"), strShape, strState);
    ui::StringUtil::ReplaceAll(_T("{2}"), strContent, strState);
    ui::StringUtil::ReplaceAll(_T("{3}"), strPalette, strState);
    pLabel->SetText(strState);
}

DString AvatarForm::GetLangText(const DString& strId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
