#ifndef UI_BOX_XMLBOX_H_
#define UI_BOX_XMLBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Utils/FilePath.h"

namespace ui
{
struct XmlPreviewAttributes;
class Shadow;

/** 支持加载并预览界面库的XML文件的容器，可以用于预览XML文件中定义的控件显示效果
*/
class UILIB_API XmlBox : public Box
{
    typedef Box BaseClass;
public:
    explicit XmlBox(Window* pWindow);
    virtual ~XmlBox() override;

    //基类的虚函数
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

public:
    /** 设置XML文件的路径
    * @param [in] xmlPath XML文件的路径
    * @return 如果加载XML数据成功返回true，否则返回false
    */
    bool SetXmlFilePath(const FilePath& xmlPath);

    /** 获取XML文件的路
    */
    const FilePath& GetXmlFilePath() const;

    /** 设置图片资源所在路径（XML文件对应的资源根目录）
    */
    void SetResPath(const FilePath& resPath);

    /** 获取图片资源所在路径（XML文件对应的资源根目录）
    */
    const FilePath& GetResPath() const;

    /** 清除界面的预览控件内容
    */
    void ClearXmlBox();

    /** 获取已加载XML文件的完整路径（如果为磁盘文件，则为绝对路径）
    */
    const FilePath& GetXmlFileFullPath() const;

public:
    /** 加载XML完成事件的回调函数
     * @param [in] xmlPath XML文件的路径
     * @param [in] bSuccess true表示加载XML成功，false表示加载XML失败
     */
    using LoadXmlCallback = std::function<void (const ui::FilePath& xmlPath, bool bSuccess)>;

    /** 添加一个加载XML完成事件的回调函数
    * @param [in] callback 回调函数
    * @param [in] callbackId 回调函数的ID，删除回调函数时使用，由调用方确保ID的唯一性
    */
    void AddLoadXmlCallback(LoadXmlCallback callback, size_t callbackId = 0);

    /** 删除一个加载XML完成事件的回调函数
    * @param [in] callbackId 待删除回调函数的ID
    */
    void RemoveLoadXmlCallback(size_t callbackId);

protected:
    //用于初始化xml属性
    virtual void OnInit() override;

private:
    /** 加载并填充XML中定义的控件
    */
    bool LoadXmlData(const FilePath& xmlPath);

    /** 加载XML完成通知
    */
    void OnXmlDataLoaded(const FilePath& xmlPath, bool bSuccess);

    /** 清除已经加载的XML数据和界面的预览控件内容
    * @param [in] xmlPreviewAttributesNew 最新的窗口预览公共属性
    */
    void ClearLoadedXmlData(const XmlPreviewAttributes& xmlPreviewAttributesNew);

    /** 从旧的列表中移除包含在新列表里面的元素
    */
    void RemoveValuesInNewList(std::vector<DString>& oldList, const std::vector<DString>& newList) const;

    /** 获取XML数据和XML路径
    * @param [in] xmlInputPath 输入的XML路径
    * @param [in] windowResPath 窗口的资源路径
    * @param [out] xmlFileData 读取的XML文件数据
    * @param [out] xmlOutputPath 输出的XML路径，解析时用于查找Include的XML文件路径
    * @param [out] xmlResPath XML文件对应的资源文件路径
    */
    bool ReadXmlFileData(const FilePath& xmlInputPath, const FilePath& windowResPath,
                         std::vector<unsigned char>& xmlFileData, FilePath& xmlOutputPath, FilePath& xmlResPath) const;

    /** 从相对路径中，解析出第一级目录
    */
    FilePath GetFirstDirectory(const FilePath& resPath) const;

    /** 从绝对路径中解析出资源路径
    */
    FilePath GetResDirectory(FilePath xmlFilePath, const FilePath& windowResPath) const;

private:
    /** XML文件的路径
    */
    FilePath m_xmlPath;

    /** 图片资源所在路径（XML文件对应的资源根目录）
    */
    FilePath m_resPath;

    /** 当前XML文件对应的图片资源所在路径（XML文件对应的资源根目录）
    */
    FilePath m_xmlResPath;

    /** 已加载XML文件的完整路径（如果为磁盘文件，则为绝对路径）
    */
    FilePath m_xmlFileFullPath;

    /** 关联的窗口公共属性和全局属性，切换XML时需要从窗口对象中移除这些属性，避免相互干扰
    */
    std::unique_ptr<XmlPreviewAttributes> m_pXmlPreviewAttributes;

    /** 添加到容器的子容器接口
    */
    Box* m_pSubBox;

    /** 窗口阴影
    */
    std::unique_ptr<Shadow> m_pShadow;

    /** XML加载完成事件的回调函数相关数据
    */
    struct LoadXmlCallbackData
    {
        LoadXmlCallback m_callback;
        size_t m_callbackId;
    };
    std::vector<LoadXmlCallbackData> m_loadXmlCallbacks;
};

} //namespace ui

#endif // UI_BOX_XMLBOX_H_
