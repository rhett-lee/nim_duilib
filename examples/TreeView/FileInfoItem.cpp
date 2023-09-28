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
    if (m_pIconControl != nullptr) {
        if (!fileInfo.m_isFolder && IsImageFile(fileInfo.m_fileName)) {
            //图片文件，直接显示图片（屏蔽了，运行速度太慢，待优化）
            int32_t itemWidth = 64;// this->GetWidth();
            std::wstring imageString = fileInfo.m_filePath;
            if (itemWidth > 0) {
                imageString = ui::StringHelper::Printf(L"file='%s' halign='center' valign='center' width='%d'", imageString.c_str(), itemWidth);
            }
            else {
                imageString = ui::StringHelper::Printf(L"file='%s' halign='center' valign='center'", imageString.c_str());
            }
            m_pIconControl->SetBkImage(imageString);
        }
        else {
            //非图片文件或者文件夹，显示图标
            std::wstring iconString = ui::GlobalManager::Instance().Icon().GetIconString(fileInfo.m_hIcon);
            if (!iconString.empty()) {
                iconString = ui::StringHelper::Printf(L"file='%s' halign='center' valign='center'", iconString.c_str());
                m_pIconControl->SetBkImage(iconString);
            }
        }             
    }
}

bool FileInfoItem::IsImageFile(const std::wstring& filePath) const
{
    std::wstring fileExt;
    size_t pos = filePath.rfind(L'.');
    if (pos != std::wstring::npos) {
        fileExt = filePath.substr(pos);
    }
    fileExt = ui::StringHelper::MakeLowerString(fileExt);
    if (fileExt == L".svg") {
        return true;
    }
    /*if ((fileExt == L".svg") || 
        (fileExt == L".jpg") || 
        (fileExt == L".jpeg") || 
        (fileExt == L".gif") || 
        (fileExt == L".png") || 
        (fileExt == L".bmp") || 
        (fileExt == L".webp") || 
        (fileExt == L".ico")) {
        return true;
    }*/
    return false;
}
