#include "TimelineForm.h"

using namespace ui;

namespace
{
    /** 从语言文件读取指定 STRID 的文本（支持中英双语）
    */
    DString TimelineLang(const DString& strId)
    {
        return GlobalManager::Instance().Lang().GetStringByID(strId);
    }
}

TimelineForm::TimelineForm()
{
}

TimelineForm::~TimelineForm()
{
}

DString TimelineForm::GetSkinFolder()
{
    return _T("controls");
}

DString TimelineForm::GetSkinFile()
{
    return _T("timeline.xml");
}

void TimelineForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //节点点击事件回调演示
    ui::Timeline* pApproval = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_approval")));
    if (pApproval != nullptr) {
        pApproval->AttachItemClick([](const ui::EventArgs& args) {
            size_t index = static_cast<size_t>(args.wParam);
            //演示：点击节点触发回调（此处可扩展为弹窗显示详情等）
            (void)index;
            return true;
            });
    }

    //控制按钮
    ui::Button* pAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_add")));
    if (pAddBtn != nullptr) {
        pAddBtn->AttachClick([this](const ui::EventArgs&) {
            AddItem();
            return true;
            });
    }
    ui::Button* pRemoveBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_remove")));
    if (pRemoveBtn != nullptr) {
        pRemoveBtn->AttachClick([this](const ui::EventArgs&) {
            RemoveItem();
            return true;
            });
    }
    ui::Button* pClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_clear")));
    if (pClearBtn != nullptr) {
        pClearBtn->AttachClick([this](const ui::EventArgs&) {
            ClearItems();
            return true;
            });
    }
    ui::Button* pReverseBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_reverse")));
    if (pReverseBtn != nullptr) {
        pReverseBtn->AttachClick([this](const ui::EventArgs&) {
            ToggleReverse();
            return true;
            });
    }
}

void TimelineForm::AddItem()
{
    ui::Timeline* pTimeline = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_task")));
    if (pTimeline == nullptr) {
        return;
    }
    //动态追加一条进行中的条目
    DString strTime = TimelineLang(_T("STRID_TIMELINE_ADD_TIME"));
    DString strTitle = TimelineLang(_T("STRID_TIMELINE_ADD_TITLE"));
    DString strContent = TimelineLang(_T("STRID_TIMELINE_ADD_CONTENT"));
    pTimeline->AddItem(strTime, strTitle, strContent, ui::TimelineNodeStatus::kInProgress);
}

void TimelineForm::RemoveItem()
{
    ui::Timeline* pTimeline = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_task")));
    if ((pTimeline == nullptr) || (pTimeline->GetItemCount() == 0)) {
        return;
    }
    pTimeline->RemoveItemAt(pTimeline->GetItemCount() - 1);
}

void TimelineForm::ClearItems()
{
    ui::Timeline* pTimeline = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_task")));
    if (pTimeline != nullptr) {
        pTimeline->ClearItems();
        pTimeline->SetEmptyText(TimelineLang(_T("STRID_TIMELINE_EMPTY")));
    }
}

void TimelineForm::ToggleReverse()
{
    ui::Timeline* pApproval = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_approval")));
    if (pApproval != nullptr) {
        pApproval->SetReverse(!pApproval->IsReverse());
    }
    ui::Timeline* pTask = dynamic_cast<ui::Timeline*>(FindControl(_T("timeline_task")));
    if (pTask != nullptr) {
        pTask->SetReverse(!pTask->IsReverse());
    }
}
