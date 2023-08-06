#pragma once

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

struct FileInfo;
class FileInfoItem : public ui::ListBoxItem
{
public:
    FileInfoItem();
    virtual ~FileInfoItem();
    void InitSubControls(const FileInfo& fileInfo, size_t nElementIndex);

private:
    //元素的索引号
    size_t m_nElementIndex;

    //图标控件
    ui::Control* m_pIconControl;

    //文字控件
    ui::Label* m_pTextControl;
};

