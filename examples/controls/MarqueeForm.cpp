#include "MarqueeForm.h"

using namespace ui;

namespace
{
    /** 从语言文件读取指定 STRID 的文本（支持中英双语）
    */
    DString MarqueeLang(const DString& strId)
    {
        return GlobalManager::Instance().Lang().GetStringByID(strId);
    }
}

MarqueeForm::MarqueeForm()
{
}

MarqueeForm::~MarqueeForm()
{
}

DString MarqueeForm::GetSkinFolder()
{
    return _T("controls");
}

DString MarqueeForm::GetSkinFile()
{
    return _T("marquee.xml");
}

void MarqueeForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //暂停/恢复演示
    ui::Button* pPauseBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_pause")));
    if (pPauseBtn != nullptr) {
        pPauseBtn->AttachClick([this](const ui::EventArgs&) {
            TogglePause();
            return true;
            });
    }

    //文本动态更新演示
    ui::Button* pUpdateBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_update")));
    if (pUpdateBtn != nullptr) {
        pUpdateBtn->AttachClick([this](const ui::EventArgs&) {
            UpdateText();
            return true;
            });
    }
}

void MarqueeForm::TogglePause()
{
    //遍历所有跑马灯控件，切换暂停状态（演示 Pause/Resume）
    static bool bPaused = false;
    bPaused = !bPaused;

    const DString kMarqueeNames[] = {
        _T("marquee_h_left"), _T("marquee_h_right"),
        _T("marquee_v_up"), _T("marquee_v_down")
    };
    for (const DString& strName : kMarqueeNames) {
        ui::Marquee* pMarquee = dynamic_cast<ui::Marquee*>(FindControl(strName));
        if (pMarquee != nullptr) {
            pMarquee->SetPaused(bPaused);
        }
    }
    //更新按钮文本，反馈当前状态
    ui::Button* pPauseBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_pause")));
    if (pPauseBtn != nullptr) {
        pPauseBtn->SetText(bPaused ? MarqueeLang(_T("STRID_MARQUEE_BTN_RESUME"))
                                   : MarqueeLang(_T("STRID_MARQUEE_BTN_PAUSE")));
    }
}

void MarqueeForm::UpdateText()
{
    //演示文本内容动态更新：轮换更新横向向左滚动跑马灯的文本
    static int nIndex = 0;
    ui::Marquee* pMarquee = dynamic_cast<ui::Marquee*>(FindControl(_T("marquee_h_left")));
    if (pMarquee == nullptr) {
        return;
    }
    static const DString kContentIds[] = {
        _T("STRID_MARQUEE_TEXT_UPDATE_1"),
        _T("STRID_MARQUEE_TEXT_UPDATE_2"),
        _T("STRID_MARQUEE_TEXT_UPDATE_3")
    };
    nIndex = (nIndex + 1) % 3;
    pMarquee->SetText(MarqueeLang(kContentIds[nIndex]));

    //演示速度调整
    static const int32_t kSpeeds[] = { 30, 60, 100 };
    pMarquee->SetScrollSpeed(kSpeeds[nIndex]);
}
