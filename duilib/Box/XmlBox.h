#ifndef UI_BOX_XMLBOX_H_
#define UI_BOX_XMLBOX_H_

#include "duilib/Core/Box.h"

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
    */
    void SetXmlPath(const DString& xmlPath);

    /** 获取XML文件的路
    */
    const DString& GetXmlPath() const;

    /** 设置图片资源所在路径（XML文件对应的资源根目录）
    */
    void SetResPath(const DString& resPath);

    /** 获取图片资源所在路径（XML文件对应的资源根目录）
    */
    const DString& GetResPath() const;

    /** 清除界面的预览控件内容
    */
    void ClearXmlBox();

    /** 加载XML完成事件的回调函数
     * @param [in] xmlPath XML文件的路径
     * @param [in] bSuccess true表示加载XML成功，false表示加载XML失败
     */
    using LoadXmlCallback = std::function<bool(const DString& xmlPath, bool bSuccess)>;

    /** 添加一个加载XML完成事件的回调函数
    * @param [in] callback 回调函数
    * @param [in] callbackId 回调函数的ID，删除回调函数时使用，由调用方确保ID的唯一性
    */
    void AddLoadXmlCallback(LoadXmlCallback callback, size_t callbackId);

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
    bool LoadXmlData(const DString& xmlPath);

    /** 加载XML完成通知
    */
    void OnXmlDataLoaded(const DString& xmlPath, bool bSuccess);

    /** 清除已经加载的XML数据和界面的预览控件内容
    * @param [in] xmlPreviewAttributesNew 最新的窗口预览公共属性
    */
    void ClearLoadedXmlData(const XmlPreviewAttributes& xmlPreviewAttributesNew);

    /** 从旧的列表中移除包含在新列表里面的元素
    */
    void RemoveValuesInNewList(std::vector<DString>& oldList, const std::vector<DString>& newList) const;

private:
    /** XML文件的路径
    */
    DString m_xmlPath;

    /** 图片资源所在路径（XML文件对应的资源根目录）
    */
    DString m_resPath;

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
