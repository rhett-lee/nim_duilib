#ifndef EXAMPLES_FILEINFO_ITEM_H_
#define EXAMPLES_FILEINFO_ITEM_H_

// duilib
#include "duilib/duilib.h"

class FileInfoItem : public ui::ListBoxItem
{
public:
    explicit FileInfoItem(ui::Window* pWindow);
    virtual ~FileInfoItem() override;
    void InitSubControls(const ui::DirectoryTree::PathInfo& fileInfo, size_t nElementIndex);

private:
    /** 是否为图片文件
    */
    bool IsImageFile(const DString& filePath) const;

private:
    //元素的索引号
    size_t m_nElementIndex;

    //图标控件
    ui::Control* m_pIconControl;

    //文字控件
    ui::Label* m_pTextControl;
};

#endif //EXAMPLES_FILEINFO_ITEM_H_
