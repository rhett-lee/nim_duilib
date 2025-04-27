#ifndef EXAMPLES_SIMPLE_FILE_VIEW_H_
#define EXAMPLES_SIMPLE_FILE_VIEW_H_

// duilib
#include "duilib/duilib.h"

/** 文件信息
*/
typedef ui::DirectoryTree::PathInfo PathInfo;

class MainForm;
class SimpleFileView : public ui::VirtualListBoxElement
{
public:
    SimpleFileView(MainForm* pMainForm, ui::VirtualListBox* pListBox);
    virtual ~SimpleFileView() override;

    /** 创建一个数据项
    * @param [in] pVirtualListBox 关联的虚表的接口
    * @return 返回创建后的数据项指针
    */
    virtual ui::Control* CreateElement(ui::VirtualListBox* pVirtualListBox) override;

    /** 填充指定数据项
    * @param [in] pControl 数据项控件指针
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    */
    virtual bool FillElement(ui::Control* pControl, size_t nElementIndex) override;

    /** 获取数据项总数
    * @return 返回数据项总数
    */
    virtual size_t GetElementCount() const override;

    /** 设置选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @param [in] bSelected true表示选择状态，false表示非选择状态
    */
    virtual void SetElementSelected(size_t nElementIndex, bool bSelected) override;

    /** 获取选择状态
    * @param [in] nElementIndex 数据元素的索引ID，范围：[0, GetElementCount())
    * @return true表示选择状态，false表示非选择状态
    */
    virtual bool IsElementSelected(size_t nElementIndex) const override;

    /** 获取选择的元素列表
    * @param [in] selectedIndexs 返回当前选择的元素列表，有效范围：[0, GetElementCount())
    */
    virtual void GetSelectedElements(std::vector<size_t>& selectedIndexs) const override;

    /** 是否支持多选
    */
    virtual bool IsMultiSelect() const override;

    /** 设置是否支持多选，由界面层调用，保持与界面控件一致
    * @return bMultiSelect true表示支持多选，false表示不支持多选
    */
    virtual void SetMultiSelect(bool bMultiSelect) override;

public:
    /** 用新的文件列表，替换原来的列表
    */
    void SetFileList(const std::vector<PathInfo>& pathList);

    /** 清空文件列表
    */
    void ClearFileList(std::vector<PathInfo>& pathList) const;

private:
    /** 子项被双击时触发
     * @param[in] args 消息体
     * @return 始终返回 true
     */
    bool OnDoubleClickItem(const ui::EventArgs& args);

private:
    /** 关联的MainForm
    */
    MainForm* m_pMainForm;

    /** 文件列表的接口（右侧虚表显示的界面）
    */
    ui::ControlPtr<ui::VirtualListBox> m_pListBox;

    //文件列表
    std::vector<PathInfo> m_pathList;
};

#endif //EXAMPLES_SIMPLE_FILE_VIEW_H_
