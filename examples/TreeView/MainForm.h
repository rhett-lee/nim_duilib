#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "FileInfoList.h"
#include <stack>

class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 触发树节点点击事件, 选择对应的目录
     */
    void SelectSubPath(const ui::FilePath& filePath);

private:
    /** 已获取指定目录的内容
    * @param [in] pTreeNode 当前的节点
    * @param [in] path 路径
    * @param [in] folderList 返回path目录中的所有子目录列表
    * @param [in] fileList 返回path目录中的所有文件列表
    */
    void OnShowFolderContents(ui::TreeNode* pTreeNode, const ui::FilePath& path,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& folderList,
                              const std::shared_ptr<std::vector<ui::DirectoryTree::PathInfo>>& fileList);

    /** 在地址栏中输入回车
    */
    bool OnAddressBarReturn(const ui::EventArgs& msg);

    /** 刷新
    */
    void Refresh();

    /** 刷新左树完成后，更新文件显示区域
    */
    void OnRefresh();

    /** 向上
    */
    void ShowUp();

    /** 向后
    */
    void ShowBack();

    /** 向前
    */
    void ShowForward();

    /** 更新界面状态
    */
    void UpdateCommandUI();

private:
    /** 设置当前选择的树节点
    */
    void SetShowTreeNode(ui::TreeNode* pTreeNode);

private:
    /** 左侧树节点的接口
    */
    ui::DirectoryTree* m_pTree;

    /** 当前路径显示
    */
    ui::RichEdit* m_pAddressBar;

    /** 文件列表（右侧虚表显示的数据）
    */
    FileInfoList m_fileList;

    /** 文件列表的接口（右侧虚表显示的界面）
    */
    ui::VirtualListBox* m_pListBox;

private:
    /** 当前显示的树节点
    */
    ui::TreeNode* m_pTreeNode;

    /** 当前选择树节点的父节点列表（用于刷新机制）
    */
    std::vector<ui::TreeNode*> m_parentTreeNodes;

private:
    /** 向上按钮
    */
    ui::Button* m_pBtnUp;

    /** 后退按钮
    */
    ui::Button* m_pBtnBack;

    /** 前进按钮
    */
    ui::Button* m_pBtnForward;

    /** 后退列表
    */
    std::stack<ui::TreeNode*> m_backStack;

    /** 前进列表
    */
    std::stack<ui::TreeNode*> m_forwardStack;

    /** 是否可以添加到前进后退列表
    */
    bool m_bCanAddBackForward;
};

#endif //EXAMPLES_MAIN_FORM_H_
