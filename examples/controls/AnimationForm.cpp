#include "AnimationForm.h"

namespace
{
    //动画图片在XML中的name属性值
    const DString kAnimationImageName = _T("bk_animation_test");

    //ICO按多帧动画加载时，每帧的播放间隔(毫秒)
    const int32_t kIconFrameDelayMs = 200;
}

AnimationForm::AnimationForm():
    m_nImageIndex(0),
    m_bImagePlaying(false),
    m_nFrameIndex(0),
    m_nFrameCount(0),
    m_nStartEventCount(0),
    m_nStopEventCount(0),
    m_nFrameEventCount(0)
{
}

AnimationForm::~AnimationForm()
{
}

DString AnimationForm::GetSkinFolder()
{
    return _T("controls");
}

DString AnimationForm::GetSkinFile()
{
    return _T("animation.xml");
}

void AnimationForm::OnInitWindow()
{
    //挂载动画播放相关事件
    ui::Control* pControl = GetImageControl();
    if (pControl != nullptr) {
        pControl->AttachImageAnimationStart([this](const ui::EventArgs& arg) {
            if (arg.wParam != 0) {
                const ui::ImageAnimationStatus& status = *((ui::ImageAnimationStatus*)arg.wParam);
                OnAnimationEvents(ui::EventType::kEventImageAnimationStart, status);
            }
            return true;
            });
        pControl->AttachImageAnimationStop([this](const ui::EventArgs& arg) {
            if (arg.wParam != 0) {
                const ui::ImageAnimationStatus& status = *((ui::ImageAnimationStatus*)arg.wParam);
                OnAnimationEvents(ui::EventType::kEventImageAnimationStop, status);
            }
            return true;
            });
        pControl->AttachImageAnimationPlayFrame([this](const ui::EventArgs& arg) {
            if (arg.wParam != 0) {
                const ui::ImageAnimationStatus& status = *((ui::ImageAnimationStatus*)arg.wParam);
                OnAnimationEvents(ui::EventType::kEventImageAnimationPlayFrame, status);
            }
            return true;
            });
    }

    InitImageList();
    InitPlayControls();

    //加载默认的测试图片
    ApplyImage();
}

bool AnimationForm::OnLanguageChanged()
{
    const bool bRet = ui::WindowImplBase::OnLanguageChanged();

    //刷新下拉框中各图片的显示名称（保持当前选择不变）
    ui::Combo* pCombo = dynamic_cast<ui::Combo*>(FindControl(_T("image_combo")));
    if ((pCombo != nullptr) && !m_imageList.empty()) {
        pCombo->DeleteAllItems();
        for (const TestImageItem& item : m_imageList) {
            pCombo->AddTextItem(ui::GlobalManager::Instance().Lang().GetStringByID(item.m_strNameId));
        }
        pCombo->SetCurSel(m_nImageIndex, false);
    }

    //刷新动态生成的文本
    UpdateFrameLabel();
    UpdateImageInfo();
    UpdateEventInfo();
    ui::Label* pLastEventLabel = dynamic_cast<ui::Label*>(FindControl(_T("event_last")));
    if (pLastEventLabel != nullptr) {
        pLastEventLabel->SetText(_T("-"));
    }
    return bRet;
}

void AnimationForm::InitImageList()
{
    m_imageList.clear();
    m_imageList.push_back({ _T("apng_test.png"),  _T("STRID_CONTROLS_ANI_IMAGE_APNG"),   false, 0 });
    m_imageList.push_back({ _T("gif_test.gif"),   _T("STRID_CONTROLS_ANI_IMAGE_GIF"),    false, 0 });
    m_imageList.push_back({ _T("fan.gif"),        _T("STRID_CONTROLS_ANI_IMAGE_FAN"),    false, 0 });
    m_imageList.push_back({ _T("webp_test.webp"), _T("STRID_CONTROLS_ANI_IMAGE_WEBP"),   false, 0 });
    m_imageList.push_back({ _T("icon_test.ico"),  _T("STRID_CONTROLS_ANI_IMAGE_ICO"),    true,  48 });
    m_imageList.push_back({ _T("pag_logo.pag"),   _T("STRID_CONTROLS_ANI_IMAGE_PAG"),    false, 0 });
    m_imageList.push_back({ _T("lottie.json"),    _T("STRID_CONTROLS_ANI_IMAGE_LOTTIE"), false, 0 });

    ui::Combo* pCombo = dynamic_cast<ui::Combo*>(FindControl(_T("image_combo")));
    if (pCombo == nullptr) {
        return;
    }
    pCombo->DeleteAllItems();
    for (const TestImageItem& item : m_imageList) {
        pCombo->AddTextItem(ui::GlobalManager::Instance().Lang().GetStringByID(item.m_strNameId));
    }
    pCombo->AttachSelect([this](const ui::EventArgs& /*arg*/) {
        ui::Combo* pImageCombo = dynamic_cast<ui::Combo*>(FindControl(_T("image_combo")));
        if (pImageCombo != nullptr) {
            const size_t nIndex = pImageCombo->GetCurSel();
            if (nIndex < m_imageList.size()) {
                m_nImageIndex = nIndex;
                //切换图片：相当于加载一张新图片（同时验证旧图片的释放与取消逻辑）
                ApplyImage();
            }
        }
        return true;
        });
    pCombo->SetCurSel(m_nImageIndex, false);
}

void AnimationForm::InitPlayControls()
{
    //播放/暂停
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("animation_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            if (m_bImagePlaying) {
                StopImageAnimation(ui::AnimationImagePos::kFrameCurrent, true);
            }
            else {
                //已经播放到最后一帧时，从第一帧重新开始播放
                const bool bPlayEnded = (m_nFrameCount > 0) && (m_nFrameIndex >= (m_nFrameCount - 1));
                StartImageAnimation(bPlayEnded ? ui::AnimationImagePos::kFrameFirst
                                               : ui::AnimationImagePos::kFrameCurrent);
            }
            return true;
            });
    }

    //停止（回到第一帧）
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("stop_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            StopImageAnimation(ui::AnimationImagePos::kFrameFirst, true);
            return true;
            });
    }

    //上一帧
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("prev_frame_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            StepImageFrame(-1);
            return true;
            });
    }

    //下一帧
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("next_frame_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            StepImageFrame(1);
            return true;
            });
    }

    //跳转到指定帧
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("goto_frame_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            GotoImageFrame();
            return true;
            });
    }

    //重新加载当前图片
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("reload_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*arg*/) {
            ApplyImage();
            return true;
            });
    }
}

DString AnimationForm::BuildImageString() const
{
    DString strImage;
    if (m_nImageIndex >= m_imageList.size()) {
        return strImage;
    }
    const TestImageItem& item = m_imageList[m_nImageIndex];
    strImage = ui::StringUtil::Printf(_T("file='render/%s' name='bk_animation_test' auto_play='%s' play_count='%d' valign='center' halign='center'"),
                                      item.m_strFileName.c_str(),
                                      GetAutoPlay() ? _T("true") : _T("false"),
                                      GetPlayCount());
    if (item.m_bIconAsAnimation) {
        strImage += ui::StringUtil::Printf(_T(" icon_as_animation='true' icon_size='%u' icon_frame_delay='%d'"),
                                           item.m_nIconSize, kIconFrameDelayMs);
    }
    return strImage;
}

void AnimationForm::ApplyImage()
{
    ui::Control* pControl = GetImageControl();
    if ((pControl == nullptr) || (m_nImageIndex >= m_imageList.size())) {
        return;
    }

    //先停止旧图片的播放，并释放旧的图片对象
    pControl->StopImageAnimation(kAnimationImageName, ui::AnimationImagePos::kFrameCurrent, false);
    pControl->SetBkImage(DString());

    //重置状态
    m_bImagePlaying = false;
    m_nFrameIndex = 0;
    m_nFrameCount = 0;
    m_nStartEventCount = 0;
    m_nStopEventCount = 0;
    m_nFrameEventCount = 0;
    UpdatePlayState(false);
    //清除上一张图片遗留的“最近事件”文本
    ui::Label* pLastEventLabel = dynamic_cast<ui::Label*>(FindControl(_T("event_last")));
    if (pLastEventLabel != nullptr) {
        pLastEventLabel->SetText(_T("-"));
    }

    //加载新图片
    pControl->SetBkImage(BuildImageString());
    ui::Image* pImage = pControl->FindImageByName(kAnimationImageName);
    if (pImage == nullptr) {
        UpdateFrameLabel();
        UpdateImageInfo();
        UpdateEventInfo();
        UpdateGotoFrameRange();
        return;
    }
    pControl->LoadImageInfo(*pImage);

    m_nFrameCount = (int32_t)pImage->GetFrameCount();
    UpdateFrameLabel();
    UpdateImageInfo();
    UpdateEventInfo();
    UpdateGotoFrameRange();

    if (pImage->IsMultiFrameImage()) {
        if (GetAutoPlay()) {
            StartImageAnimation(ui::AnimationImagePos::kFrameFirst);
        }
        else {
            pControl->SetImageAnimationFrame(kAnimationImageName, 0);
        }
    }
}

bool AnimationForm::GetAutoPlay() const
{
    const ui::CheckBox* pCheckBox = dynamic_cast<const ui::CheckBox*>(FindControl(_T("auto_play_check")));
    if (pCheckBox != nullptr) {
        return pCheckBox->IsSelected();
    }
    return true;
}

int32_t AnimationForm::GetPlayCount() const
{
    const ui::RichEdit* pRichEdit = dynamic_cast<const ui::RichEdit*>(FindControl(_T("play_count_edit")));
    if (pRichEdit != nullptr) {
        const int64_t nValue = pRichEdit->GetTextNumber();
        if (nValue < 0) {
            return -1;
        }
        return (int32_t)nValue;
    }
    return -1;
}

void AnimationForm::StartImageAnimation(ui::AnimationImagePos nStartFrame)
{
    ui::Control* pControl = GetImageControl();
    if (pControl == nullptr) {
        return;
    }
    pControl->StartImageAnimation(kAnimationImageName, nStartFrame, GetPlayCount());
}

void AnimationForm::StopImageAnimation(ui::AnimationImagePos nStopFrame, bool bTriggerEvent)
{
    ui::Control* pControl = GetImageControl();
    if (pControl == nullptr) {
        return;
    }
    pControl->StopImageAnimation(kAnimationImageName, nStopFrame, bTriggerEvent);
}

void AnimationForm::StepImageFrame(int32_t nStep)
{
    ui::Control* pControl = GetImageControl();
    if (pControl == nullptr) {
        return;
    }
    if (m_bImagePlaying) {
        //暂停后切换帧
        StopImageAnimation(ui::AnimationImagePos::kFrameCurrent, true);
    }
    const int32_t nFrameCount = (int32_t)pControl->GetImageAnimationFrameCount(kAnimationImageName);
    if (nFrameCount <= 0) {
        return;
    }
    int32_t nFrameIndex = (int32_t)pControl->GetImageAnimationFrameIndex(kAnimationImageName) + nStep;
    //循环取帧
    nFrameIndex %= nFrameCount;
    if (nFrameIndex < 0) {
        nFrameIndex += nFrameCount;
    }
    pControl->SetImageAnimationFrame(kAnimationImageName, nFrameIndex);

    m_nFrameIndex = nFrameIndex;
    m_nFrameCount = nFrameCount;
    UpdateFrameLabel();
    UpdateImageInfo();
}

void AnimationForm::GotoImageFrame()
{
    ui::Control* pControl = GetImageControl();
    if (pControl == nullptr) {
        return;
    }
    const ui::RichEdit* pRichEdit = dynamic_cast<const ui::RichEdit*>(FindControl(_T("goto_frame_number")));
    if (pRichEdit == nullptr) {
        return;
    }
    if (m_bImagePlaying) {
        //暂停后跳转帧
        StopImageAnimation(ui::AnimationImagePos::kFrameCurrent, true);
    }
    const int32_t nFrameCount = (int32_t)pControl->GetImageAnimationFrameCount(kAnimationImageName);
    if (nFrameCount <= 0) {
        return;
    }
    int32_t nFrameIndex = (int32_t)pRichEdit->GetTextNumber();
    if (nFrameIndex < 0) {
        nFrameIndex = 0;
    }
    if (nFrameIndex >= nFrameCount) {
        nFrameIndex = nFrameCount - 1;
    }
    pControl->SetImageAnimationFrame(kAnimationImageName, nFrameIndex);

    m_nFrameIndex = nFrameIndex;
    m_nFrameCount = nFrameCount;
    UpdateFrameLabel();
    UpdateImageInfo();
}

void AnimationForm::UpdatePlayState(bool bPlaying)
{
    m_bImagePlaying = bPlaying;
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("animation_btn")));
    if (pButton != nullptr) {
        pButton->SetTextId(bPlaying ? _T("STRID_CONTROLS_ANI_PAUSE") : _T("STRID_CONTROLS_ANI_PLAY"));
    }
}

void AnimationForm::UpdateFrameLabel(int32_t nFrameDelayMs)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("animation_frame")));
    if (pLabel == nullptr) {
        return;
    }
    if (m_nFrameCount <= 0) {
        pLabel->SetText(_T("[0/0]"));
        return;
    }
    if (nFrameDelayMs >= 0) {
        pLabel->SetText(ui::StringUtil::Printf(_T("[%d/%d] %d ms"), m_nFrameIndex, m_nFrameCount, nFrameDelayMs));
    }
    else {
        pLabel->SetText(ui::StringUtil::Printf(_T("[%d/%d]"), m_nFrameIndex, m_nFrameCount));
    }
}

void AnimationForm::UpdateImageInfo()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("image_info")));
    if (pLabel == nullptr) {
        return;
    }
    ui::Image* pImage = GetDuiImage();
    if (pImage == nullptr) {
        pLabel->SetText(_T("-"));
        return;
    }
    const std::shared_ptr<ui::ImageInfo>& pImageInfo = pImage->GetImageInfo();
    if (pImageInfo == nullptr) {
        pLabel->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_LOAD_ERROR")));
        return;
    }
    const int32_t nWidth = pImageInfo->GetWidth();
    const int32_t nHeight = pImageInfo->GetHeight();
    const int32_t nFrameCount = (int32_t)pImage->GetFrameCount();
    const int32_t nLoopCount = pImage->GetLoopCount();
    int32_t nFrameDelayMs = 0;
    if (nFrameCount > 0) {
        nFrameDelayMs = pImageInfo->GetFrameDelayMs(pImage->GetCurrentFrameIndex());
    }
    const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_INFO_FORMAT"));
    pLabel->SetText(ui::StringUtil::Printf(strFormat.c_str(), nWidth, nHeight, nFrameCount, nLoopCount, nFrameDelayMs));
}

void AnimationForm::UpdateEventInfo()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("event_info")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_EVENT_FORMAT"));
    pLabel->SetText(ui::StringUtil::Printf(strFormat.c_str(), m_nStartEventCount, m_nStopEventCount, m_nFrameEventCount));
}

void AnimationForm::UpdateGotoFrameRange()
{
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("goto_frame_number")));
    if (pRichEdit == nullptr) {
        return;
    }
    pRichEdit->SetMinNumber(0);
    pRichEdit->SetMaxNumber(m_nFrameCount > 0 ? (m_nFrameCount - 1) : 0);
}

ui::Control* AnimationForm::GetImageControl() const
{
    return FindControl(_T("animation_test"));
}

ui::Image* AnimationForm::GetDuiImage() const
{
    ui::Control* pControl = GetImageControl();
    if (pControl == nullptr) {
        return nullptr;
    }
    return pControl->FindImageByName(kAnimationImageName);
}

void AnimationForm::OnAnimationEvents(ui::EventType eventType, const ui::ImageAnimationStatus& status)
{
    ui::Label* pLastEventLabel = dynamic_cast<ui::Label*>(FindControl(_T("event_last")));
    if (eventType == ui::EventType::kEventImageAnimationStart) {
        //播放开始
        ++m_nStartEventCount;
        m_nFrameIndex = status.m_nFrameIndex;
        if (status.m_nFrameCount > 0) {
            m_nFrameCount = status.m_nFrameCount;
        }
        UpdatePlayState(true);
        UpdateGotoFrameRange();
        UpdateImageInfo();
        UpdateEventInfo();
        UpdateFrameLabel(status.m_nFrameDelayMs);
        if (pLastEventLabel != nullptr) {
            pLastEventLabel->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_EVENT_START")));
        }
    }
    else if (eventType == ui::EventType::kEventImageAnimationStop) {
        //播放停止
        ++m_nStopEventCount;
        m_nFrameIndex = status.m_nFrameIndex;
        if (status.m_nFrameCount > 0) {
            m_nFrameCount = status.m_nFrameCount;
        }
        UpdatePlayState(false);
        UpdateImageInfo();
        UpdateEventInfo();
        UpdateFrameLabel(status.m_nFrameDelayMs);
        if (pLastEventLabel != nullptr) {
            pLastEventLabel->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_EVENT_STOP")));
        }
    }
    else if (eventType == ui::EventType::kEventImageAnimationPlayFrame) {
        //播放帧
        ++m_nFrameEventCount;
        m_nFrameIndex = status.m_nFrameIndex;
        if (status.m_nFrameCount > 0) {
            m_nFrameCount = status.m_nFrameCount;
        }
        UpdatePlayState(true);
        UpdateEventInfo();
        UpdateFrameLabel(status.m_nFrameDelayMs);
        if (pLastEventLabel != nullptr) {
            const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CONTROLS_ANI_EVENT_FRAME"));
            pLastEventLabel->SetText(ui::StringUtil::Printf(strFormat.c_str(), m_nFrameIndex, m_nFrameCount));
        }
    }
}
