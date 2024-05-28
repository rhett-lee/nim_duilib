#pragma once

// duilib
#include "duilib/duilib.h"

struct FileInfo;
class FileInfoItem : public ui::ListBoxItem
{
public:
    explicit FileInfoItem(ui::Window* pWindow);
    virtual ~FileInfoItem();
    void InitSubControls(const FileInfo& fileInfo, size_t nElementIndex);

private:
    /** 是否为图片文件
    */
    bool IsImageFile(const std::wstring& filePath) const;

private:
    //元素的索引号
    size_t m_nElementIndex;

    //图标控件
    ui::Control* m_pIconControl;

    //文字控件
    ui::Label* m_pTextControl;
};

