#include "ProgressForm.h"
#include <cstdlib>

ProgressForm::ProgressForm():
    m_nProgressValue(0.0)
{
}

ProgressForm::~ProgressForm()
{
}

DString ProgressForm::GetSkinFolder()
{
    return _T("controls");
}

DString ProgressForm::GetSkinFile()
{
    return _T("progress.xml");
}

void ProgressForm::OnInitWindow()
{
    //启动一个定时器：统一驱动各进度控件递增演示
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI,
        ui::UiBind(this, [this]() {
                OnTimer();
            }),
        100);

    //随机设值按钮：给浮点进度条设置 0~1 随机值（触发值变化动画与事件）
    ui::Button* pAnimateBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_progress_animate")));
    if (pAnimateBtn != nullptr) {
        pAnimateBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Progress* pFloat = dynamic_cast<ui::Progress*>(FindControl(_T("progress_float")));
            if (pFloat != nullptr) {
                double v = static_cast<double>(std::rand() % 101) / 100.0;
                pFloat->SetValue(v);
            }
            return true;
            });
    }

    //切换转圈按钮：切换环形进度条的 indeterminate 状态
    ui::Button* pIndetBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_progress_indet")));
    if (pIndetBtn != nullptr) {
        pIndetBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CircleProgress* pIndet = dynamic_cast<ui::CircleProgress*>(FindControl(_T("circle_indeterminate")));
            if (pIndet != nullptr) {
                pIndet->SetIndeterminate(!pIndet->IsIndeterminate());
            }
            return true;
            });
    }

    //值变化事件演示：浮点进度条挂载 AttachValueChanged
    ui::Progress* pFloatProgress = dynamic_cast<ui::Progress*>(FindControl(_T("progress_float")));
    if (pFloatProgress != nullptr) {
        pFloatProgress->AttachValueChanged([this](const ui::EventArgs& args) {
            ui::Label* pEventLabel = dynamic_cast<ui::Label*>(FindControl(_T("progress_event_text")));
            if (pEventLabel != nullptr) {
                //精确浮点值从发送者读取（wParam 为整数截断值）
                double fValue = 0.0;
                ui::Control* pSender = args.GetSender();
                if (pSender != nullptr) {
                    ui::Progress* pProgress = dynamic_cast<ui::Progress*>(pSender);
                    if (pProgress != nullptr) {
                        fValue = pProgress->GetValue();
                    }
                }
                pEventLabel->SetText(ui::StringUtil::Printf(_T("Event: %.2f"), fValue));
            }
            return true;
            });
    }

    //进度文本自定义格式演示：浮点进度条用自定义格式替代默认的"xx%"
    if (pFloatProgress != nullptr) {
        pFloatProgress->SetTextFormatCallback([](double value, double /*minValue*/, double /*maxValue*/) {
            return ui::StringUtil::Printf(_T("%.2f / 1.00"), value);
            });
    }

    //进度完成事件演示：定时器驱动的进度条每走满一次触发一次
    ui::Progress* pLoopProgress = dynamic_cast<ui::Progress*>(FindControl(_T("progress11")));
    if (pLoopProgress != nullptr) {
        pLoopProgress->AttachProgressCompleted([this](const ui::EventArgs& /*args*/) {
            ui::Label* pEventLabel = dynamic_cast<ui::Label*>(FindControl(_T("progress_event_text")));
            if (pEventLabel != nullptr) {
                pEventLabel->SetText(_T("Completed!"));
            }
            return true;
            });
    }

    //===== Slider 交互测试 =====

    //键盘操作测试：把键盘/鼠标操作后的当前值回显到标签，便于手工验证
    ui::Slider* pKeyboardSlider = dynamic_cast<ui::Slider*>(FindControl(_T("slider_keyboard")));
    if (pKeyboardSlider != nullptr) {
        pKeyboardSlider->AttachValueChanged([this](const ui::EventArgs& /*args*/) {
            ui::Slider* pSlider = dynamic_cast<ui::Slider*>(FindControl(_T("slider_keyboard")));
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("slider_keyboard_text")));
            if ((pSlider != nullptr) && (pLabel != nullptr)) {
                pLabel->SetText(ui::StringUtil::Printf(
                    _T("step=%d, page_step=%d, value=%.0f"),
                    pSlider->GetChangeStep(), pSlider->GetPageStep(), pSlider->GetValue()));
            }
            return true;
            });
        //初始回显
        ui::Label* pKeyLabel = dynamic_cast<ui::Label*>(FindControl(_T("slider_keyboard_text")));
        if (pKeyLabel != nullptr) {
            pKeyLabel->SetText(ui::StringUtil::Printf(
                _T("step=%d, page_step=%d, value=%.0f"),
                pKeyboardSlider->GetChangeStep(), pKeyboardSlider->GetPageStep(), pKeyboardSlider->GetValue()));
        }
    }

    //刻度标签自定义格式演示：把值格式化为百分比
    ui::Slider* pTickSlider = dynamic_cast<ui::Slider*>(FindControl(_T("slider_ticks")));
    if (pTickSlider != nullptr) {
        pTickSlider->SetTickLabelCallback([](double value, int32_t /*index*/) {
            return ui::StringUtil::Printf(_T("%d%%"), (int32_t)value);
            });
    }

    //点击轨道行为对照：两个滑块共用一个标签，回显各自的值
    const TCHAR* ppClickNames[] = {_T("slider_click_jump"), _T("slider_click_release")};
    for (const TCHAR* pName : ppClickNames) {
        ui::Slider* pSlider = dynamic_cast<ui::Slider*>(FindControl(pName));
        if (pSlider != nullptr) {
            pSlider->AttachValueChanged([this, pName](const ui::EventArgs& /*args*/) {
                ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("slider_click_text")));
                ui::Slider* pSender = dynamic_cast<ui::Slider*>(FindControl(pName));
                if ((pLabel != nullptr) && (pSender != nullptr)) {
                    pLabel->SetText(ui::StringUtil::Printf(_T("%s = %.0f"), pName, pSender->GetValue()));
                }
                return true;
                });
        }
    }
}

void ProgressForm::OnTimer()
{
    std::vector<DString> controlList = {_T("progress11"), _T("progress12"), _T("progress13"), _T("progress14"),
                                        _T("progress21"), _T("progress22"), _T("progress23"), _T("progress24") };
    for (const DString& name : controlList) {
        ui::Progress* pProgress = dynamic_cast<ui::Progress*>(FindControl(name));
        if (pProgress != nullptr) {
            pProgress->SetValue(m_nProgressValue);
        }
    }

    //buffer 次级进度演示：进度条 12 的缓冲始终领先主进度 25
    ui::Progress* pBufferProgress = dynamic_cast<ui::Progress*>(FindControl(_T("progress12")));
    if (pBufferProgress != nullptr) {
        pBufferProgress->SetBufferValue(m_nProgressValue + 25.0);
    }

    m_nProgressValue += 0.4;
    if (m_nProgressValue > 100.0) {
        m_nProgressValue = 0.0;
    }
}
