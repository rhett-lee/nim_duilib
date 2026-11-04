#ifndef EXAMPLES_XMLPREVIEW_MAINFORM_H_
#define EXAMPLES_XMLPREVIEW_MAINFORM_H_

#include "duilib/duilib.h"

#include <vector>

/** XmlPreview 示例窗口
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

    /** 语言切换后刷新状态文本
    */
    virtual bool OnLanguageChanged() override;

    /** 窗口拖放事件：在整个预览区域内接收 XML 文件的拖入
     *  （预览区内的子控件会先被命中测试，导致控件级的拖放目标无法生效，
     *    因此在窗口级统一处理，并在落点位于预览区域内时截获该事件）
    */
    virtual void OnDropEnterMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropOverMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropLeaveMsg() override;

private:
    /** 判断拖放落点是否在预览区域内，并且拖入的是 XML 文件
    * @param [in] dropType 拖放操作的来源类型
    * @param [in] pDropData 拖放数据
    * @param [out] xmlFilePath 拖入的 XML 文件路径
    * @return 返回 true 表示可以接收该文件的拖放
    */
    bool IsDropInPreviewArea(ui::ControlDropType dropType, void* pDropData, ui::FilePath& xmlFilePath) const;

    /** 从拖入的文件列表中选出第一个 XML 文件
    */
    bool GetDropXmlFile(const std::vector<DString>& fileList, ui::FilePath& xmlFilePath) const;

    /** 设置预览区域的拖放高亮状态（拖入时用强调色边框提示）
    */
    void SetPreviewDropHighlight(bool bHighlight);

private:
    /** 初始化控件指针
    */
    void InitControls();

    /** 绑定控件事件
    */
    void InitEvents();

    /** 初始化自动刷新相关控件
    */
    void InitAutoRefresh();

    /** 打开文件选择框加载XML文件
    */
    void BrowseXmlFile();

    /** 清空预览内容
    */
    void ClearPreview();

    /** 更新文件路径显示
    */
    void UpdatePathText();

    /** 更新加载状态显示（成功/失败/等待）
    */
    void UpdateStatusText();

    /** 从磁盘重新读取当前文件的原始数据（用于自动刷新时比较内容变化）
    */
    void UpdateFileDataFromDisk();

    /** 根据自动刷新复选框状态启动/停止定时器
    */
    void ApplyAutoRefresh();

    /** 设置刷新间隔输入框是否可用
    */
    void EnableRefreshInterval(bool bEnable);

    /** 获取当前刷新间隔（秒），并限制在有效范围内
    */
    int32_t GetRefreshIntervalSeconds();

    /** 启动自动刷新定时器
    */
    void StartRefreshTimer(int32_t nIntervalSeconds);

    /** 停止自动刷新定时器
    */
    void StopRefreshTimer();

    /** 定时检查XML文件是否发生变化
    */
    void CheckXmlPreview();

    /** 格式化加载失败信息（文件不存在/不可读时使用本地化提示）
    */
    DString FormatLoadError(const ui::FilePath& xmlPath, const DString& errorText) const;

    /** 获取多语言字符串
    */
    DString GetLangString(const DString& stringId) const;

private:
    /** Xml预览控件
    */
    ui::XmlBox* m_pXmlBox;

    /** XML文件路径显示
    */
    ui::Label* m_pXmlFilePathLabel;

    /** 加载状态显示
    */
    ui::Label* m_pXmlLoadResult;

    /** 加载失败时的详细信息
    */
    ui::Label* m_pXmlFileError;

    /** 自动刷新间隔输入框
    */
    ui::RichEdit* m_pRefreshInterval;

    /** 自动刷新复选框
    */
    ui::CheckBox* m_pCheckAutoRefresh;

    /** 当前预览的XML文件路径
    */
    ui::FilePath m_xmlFilePath;

    /** 当前XML文件的原始数据（用于检测文件内容变化）
    */
    std::vector<uint8_t> m_xmlFileData;

    /** 当前定时器的时间间隔（秒），-1表示定时器未运行
    */
    int32_t m_nIntervalSeconds;

    /** 定时器ID，0表示未启动
    */
    size_t m_timerId;

    /** 最近一次是否加载成功
    */
    bool m_bLoadSuccess;

    /** 是否已设置XML文件
    */
    bool m_bHasXmlFile;

    /** 最近一次加载失败的错误信息
    */
    DString m_sLastError;

    /** 当前是否处于“拖放到预览区域”的状态（用于显示高亮边框）
    */
    bool m_bDragOverPreview;
};

#endif // EXAMPLES_XMLPREVIEW_MAINFORM_H_
