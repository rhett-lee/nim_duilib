#include "FileInfoItem.h"
#include "FileInfoList.h"

FileInfoItem::FileInfoItem():
    m_nElementIndex(ui::Box::InvalidIndex),
    m_pIconControl(nullptr),
    m_pTextControl(nullptr)
{
}

FileInfoItem::~FileInfoItem()
{
}

void FileInfoItem::InitSubControls(const FileInfo& fileInfo, size_t nElementIndex)
{
    m_nElementIndex = nElementIndex;
    if (m_pIconControl == nullptr) {
        m_pIconControl = FindSubControl(L"control_img");
    }
    if (m_pTextControl == nullptr) {
        m_pTextControl = dynamic_cast<ui::Label*>(FindSubControl(L"control_text"));
    }
    if (m_pTextControl != nullptr) {
        m_pTextControl->SetAutoToolTip(true);
        m_pTextControl->SetText(fileInfo.m_fileName);
    }
}
