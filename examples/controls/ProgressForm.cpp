#include "ProgressForm.h"
#include <cstdlib>

ProgressForm::ProgressForm():
    m_nProgressValue(0.0)
    ,m_fGaugeValue(0.0)
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
            //动画曲线对照：三条不同缓动的进度条统一切到同一个随机目标值，便于横向对比
            const double fTarget = static_cast<double>(std::rand() % 101);
            const TCHAR* ppEasingNames[] = {_T("robust_ease_linear"), _T("robust_ease_out"), _T("robust_ease_elastic")};
            for (const TCHAR* pName : ppEasingNames) {
                ui::Progress* pEasing = dynamic_cast<ui::Progress*>(FindControl(pName));
                if (pEasing != nullptr) {
                    pEasing->SetValue(fTarget);
                }
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

    //===== CircleProgress 形态扩展测试 =====

    //仪表盘/缺口环：由OnTimer 统一递增，验证 span_angle 内的扫过效果
    const TCHAR* ppGaugeNames[] = {_T("gauge_half"), _T("gauge_gap")};
    for (const TCHAR* pName : ppGaugeNames) {
        ui::CircleProgress* pGauge = dynamic_cast<ui::CircleProgress*>(FindControl(pName));
        if (pGauge != nullptr) {
            pGauge->SetValue(0.0);
        }
    }

    //中心自定义内容演示：在环中心绘制一个随进度摆动的指针 + 中心数值
    //（说明：回调收到的是控件内相对坐标的矩形，可直接交给IRender 绘制）
    ui::CircleProgress* pGaugeCenter = dynamic_cast<ui::CircleProgress*>(FindControl(_T("gauge_center")));
    if (pGaugeCenter != nullptr) {
        pGaugeCenter->SetCenterCallback([pGaugeCenter](ui::IRender* pRender, const ui::UiRect& rcCenter, double fValue) {
            if (pRender == nullptr || rcCenter.IsEmpty()) {
                return true;
            }
            const int32_t nWidth = rcCenter.right - rcCenter.left;
            const int32_t nHeight = rcCenter.bottom - rcCenter.top;
            //中心点与指针长度
            const float fCx = static_cast<float>(rcCenter.left) + nWidth / 2.0f;
            const float fCy = static_cast<float>(rcCenter.top) + nHeight / 2.0f;
            const float fLen = static_cast<float>((nWidth < nHeight ? nWidth : nHeight)) * 0.42f;
            //起始 135°、跨度 270°（与 XML 一致），顺时针
            const float fPi = 3.14159265f;
            float fRatio = static_cast<float>((fValue - 0.0) / 100.0);
            if (fRatio < 0.0f) {
                fRatio = 0.0f;
            }
            else if (fRatio > 1.0f) {
                fRatio = 1.0f;
            }
            const float fDeg = 135.0f + 270.0f * fRatio;
            const float fRad = fDeg * fPi / 180.0f;
            const ui::UiPointF ptEnd(fCx + fLen * static_cast<float>(std::cos(fRad)),
                fCy + fLen * static_cast<float>(std::sin(fRad)));
            const ui::UiPointF ptTail(fCx - fLen * 0.18f * static_cast<float>(std::cos(fRad)),
                fCy - fLen * 0.18f * static_cast<float>(std::sin(fRad)));
            //指针（进度色）+ 中心轴点
            pRender->DrawLine(ptTail, ptEnd, pGaugeCenter->GetUiColor(_T("bg_progress")), 2.0f);
            pRender->FillCircle(ui::UiPointF(fCx, fCy), 3.0f, pGaugeCenter->GetUiColor(_T("text_default")));
            //中心数值文字（放在指针下方，避免与指针重叠）
            ui::DrawStringParam param;
            param.textRect = rcCenter;
            param.dwTextColor = pGaugeCenter->GetUiColor(_T("text_default"));
            param.pFont = pGaugeCenter->GetIFontById(_T("system_bold_14"));
            param.uFormat = ui::TEXT_SINGLELINE | ui::TEXT_HCENTER | ui::TEXT_VCENTER;
            param.textRect.left = rcCenter.left;
            param.textRect.right = rcCenter.right;
            param.textRect.top = rcCenter.bottom - static_cast<int>(fLen) - 6;
            param.textRect.bottom = param.textRect.top + 18;
            pRender->DrawString(ui::StringUtil::Printf(_T("%.0f"), fValue), param);
            return true;
            });
    }

    //===== 健壮性与一致性测试 =====

    //min/max 运行时钳位演示：max 由100 改为 50 时，当前值应自动回落到 50
    ui::Button* pShrinkBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_clamp_shrink")));
    if (pShrinkBtn != nullptr) {
        pShrinkBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Progress* pClamp = dynamic_cast<ui::Progress*>(FindControl(_T("robust_clamp")));
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("robust_clamp_text")));
            if (pClamp != nullptr) {
                //先重置一个超出新区间的值（80 > 50），再收紧 max
                pClamp->SetValue(80.0);
                pClamp->SetMaxValue(50);
                if (pLabel != nullptr) {
                    pLabel->SetText(ui::StringUtil::Printf(
                        _T("max=50, value=%.0f (auto-clamped), value>max is %s"),
                        pClamp->GetValue(), (pClamp->GetValue() <= 50.0) ? _T("false") : _T("true")));
                }
            }
            return true;
            });
    }
    ui::Button* pRestoreBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_clamp_restore")));
    if (pRestoreBtn != nullptr) {
        pRestoreBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Progress* pClamp = dynamic_cast<ui::Progress*>(FindControl(_T("robust_clamp")));
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("robust_clamp_text")));
            if (pClamp != nullptr) {
                pClamp->SetMaxValue(100);
                pClamp->SetValue(80.0);
                if (pLabel != nullptr) {
                    pLabel->SetText(ui::StringUtil::Printf(
                        _T("max=100, value=%.0f (restored)"), pClamp->GetValue()));
                }
            }
            return true;
            });
    }

    //动画曲线对照：三条进度条由"随机设值"按钮统一切到同一目标值，便于横向对比缓动差异
    //（不用 AttachValueChanged 驱动，避免 SetValue 触发事件造成自递归）
    const TCHAR* ppEasingNames[] = {_T("robust_ease_linear"), _T("robust_ease_out"), _T("robust_ease_elastic")};
    for (const TCHAR* pName : ppEasingNames) {
        ui::Progress* pEasing = dynamic_cast<ui::Progress*>(FindControl(pName));
        if (pEasing != nullptr) {
            //初值设为 0，点击"随机设值"后都跳到同一个目标值
            pEasing->SetValue(0.0);
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

    //CircleProgress 形态扩展演示：三个确定态表盘/缺口环同步递增
    //gauge_half:0~240（速度表）、gauge_gap/gauge_center: 0~100（评分环）
    m_fGaugeValue += 2.0;
    if (m_fGaugeValue > 100.0) {
        m_fGaugeValue = 0.0;
    }
    ui::CircleProgress* pGaugeHalf = dynamic_cast<ui::CircleProgress*>(FindControl(_T("gauge_half")));
    if (pGaugeHalf != nullptr) {
        pGaugeHalf->SetValue(m_fGaugeValue * 2.4);
    }
    ui::CircleProgress* pGaugeGap = dynamic_cast<ui::CircleProgress*>(FindControl(_T("gauge_gap")));
    if (pGaugeGap != nullptr) {
        pGaugeGap->SetValue(m_fGaugeValue);
    }
    ui::CircleProgress* pGaugeCenter = dynamic_cast<ui::CircleProgress*>(FindControl(_T("gauge_center")));
    if (pGaugeCenter != nullptr) {
        pGaugeCenter->SetValue(m_fGaugeValue);
    }

    m_nProgressValue += 0.4;
    if (m_nProgressValue > 100.0) {
        m_nProgressValue = 0.0;
    }
}
