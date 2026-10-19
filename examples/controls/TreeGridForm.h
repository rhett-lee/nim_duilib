#ifndef EXAMPLES_CONTROLS_TREE_GRID_FORM_H_
#define EXAMPLES_CONTROLS_TREE_GRID_FORM_H_

#include "duilib/duilib.h"

/** TreeGrid 树表格控件演示窗口
*/
class TreeGridForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    TreeGridForm();
    virtual ~TreeGridForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

private:
    /** 构建第一个树表格（基础：多列 + 展开收起）的数据
    */
    void InitBasicTree(ui::TreeGrid* pTreeGrid);

    /** 构建第二个树表格（勾选级联 + 多选）的数据
    */
    void InitCheckTree(ui::TreeGrid* pTreeGrid);

    /** 级联设置该表格内所有节点的展开 / 收起状态
    */
    void SetAllExpanded(ui::TreeGrid* pTreeGrid, bool bExpand);

    /** 回显基础树表格的交互事件
    */
    void UpdateBasicEventText(const DString& strText);

    /** 回显勾选树表格的交互事件
    */
    void UpdateCheckEventText(const DString& strText);

    /** 功能测试：在指定父节点下新增一个节点（bChild 为 true 表示"新增子节点"语义）
    */
    void AddNewNode(ui::TreeGrid* pTreeGrid, ui::TreeGridNode* pParent, bool bChild);

    /** 展开到指定层（nLevel 从 1 开始计数）：展开深度小于 nLevel 的节点，使第 nLevel 层可见
    */
    void ExpandToLevel(ui::TreeGrid* pTreeGrid, int32_t nLevel);

    /** 回显功能测试区的状态文本
    */
    void UpdateTestText(const DString& strText);

private:
    /** 功能测试：动态新增节点时使用的自增序号
    */
    int32_t m_nAddCounter = 0;
};

#endif // EXAMPLES_CONTROLS_TREE_GRID_FORM_H_
