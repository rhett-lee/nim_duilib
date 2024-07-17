#ifndef UI_CORE_WINDOWBUILDER_H_
#define UI_CORE_WINDOWBUILDER_H_

#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Utils/FilePath.h"
#include <functional>
#include <string>
#include <memory>

namespace pugi
{
    //XML 解析器相关定义
    class xml_document;
    class xml_node;
}

namespace ui 
{

class Box;
class Window;
class Control;
class RichTextSlice;

/** 创建控件的回调函数
*/
typedef std::function<Control* (const DString&)> CreateControlCallback;

/** 根据XML文件，解析并创建控件和布局
*/
class UILIB_API WindowBuilder
{
public:
    WindowBuilder();
    ~WindowBuilder();

    WindowBuilder(const WindowBuilder&) = delete;
    WindowBuilder& operator = (const WindowBuilder&) = delete;

public:
    /** 使用XML文件内容创建窗口布局等
    * @param [in] xmlFileData 是文件文本内容，字符串需要以字符 '<'开头;
    * @param [in] pCallback 根据Class名称创建控件（或容器）的函数，适用于自定义控件
    * @param [in] pWindow 关联的窗口
    * @param [in] pParent 父容器，将该XML文件解析的节点，作为pParent容器的子节点
    * @param [in] pUserDefinedBox 用户自定义的父容器：如果不为nullptr, 将该XML文件解析的节点，作为pUserDefinedBox容器的子节点
    * @return 如果pUserDefinedBox不为nullptr, 返回pUserDefinedBox，否则返回解析XML后，生成的第一个节点接口（可能是Control，也可能是Box）
    */
    Control* CreateFromXmlData(const DString& xmlFileData,
                               CreateControlCallback pCallback = CreateControlCallback(),
                               Window* pWindow = nullptr, 
                               Box* pParent = nullptr, 
                               Box* pUserDefinedBox = nullptr);

    /** 使用XML文件创建窗口布局等
    * @param [in] xmlFilePath XML文件的路径
    * @param [in] pCallback 根据Class名称创建控件（或容器）的函数，适用于自定义控件
    * @param [in] pWindow 关联的窗口
    * @param [in] pParent 父容器，将该XML文件解析的节点，作为pParent容器的子节点
    * @param [in] pUserDefinedBox 用户自定义的父容器，将该XML文件解析的节点，作为pUserDefinedBox容器的子节点
    * @return 如果pUserDefinedBox不为nullptr, 返回pUserDefinedBox，否则返回解析XML后，生成的第一个节点接口（可能是Control，也可能是Box）
    */
    Control* CreateFromXmlFile(const FilePath& xmlFilePath,
                               CreateControlCallback pCallback = CreateControlCallback(),
                               Window* pWindow = nullptr, 
                               Box* pParent = nullptr, 
                               Box* pUserDefinedBox = nullptr);

    /** 使用缓存中已经解析过的XML文件或者数据创建窗口布局等（即CreateFromXmlData和CreateFromXmlFile解析后的结果）
    * @param [in] pCallback 根据Class名称创建控件（或容器）的函数，适用于自定义控件
    * @param [in] pWindow 关联的窗口
    * @param [in] pParent 父容器，将该XML文件解析的节点，作为pParent容器的子节点
    * @param [in] pUserDefinedBox 用户自定义的父容器，将该XML文件解析的节点，作为pUserDefinedBox容器的子节点
    * @return 如果pUserDefinedBox不为nullptr, 返回pUserDefinedBox，否则返回解析XML后，生成的第一个节点接口（可能是Control，也可能是Box）
    */
    Control* CreateFromCachedXml(CreateControlCallback pCallback = CreateControlCallback(),
                                 Window* pWindow = nullptr,
                                 Box* pParent = nullptr, 
                                 Box* pUserDefinedBox = nullptr);

    /** 将控件转换成容器, 内部做一些断言处理，确保转换失败的时候，能够报错
    */
    Box* ToBox(Control* pControl) const;

public:
    /** 解析带格式的文本内容，并设置到RichText Control对象
    * @param [in] xmlText 带格式的文本内容
    * @param [in] pControl RichText控件的接口
    */
    static bool ParseRichTextXmlText(const DString& xmlText, Control* pControl);
    
    /** 解析带格式的文本内容，并设置到RichText Control对象
    * @param [in] xmlNode 带格式的文本内容对应的XML节点
    * @param [in] pControl RichText控件的接口
    * @param [in] pTextSlice 文本片段节点接口，如果pTextSlice不为nullptr，XML节点的解析结果将填充到pTextSlice中；否则填充到pControl中
    */
    static bool ParseRichTextXmlNode(const pugi::xml_node& xmlNode, Control* pControl, RichTextSlice* pTextSlice = nullptr);

private:
    /** 解析窗口的属性(根XML节点名称："Window")
    */
    void ParseWindowAttributes(Window* pWindow, const pugi::xml_node& root) const;

    /** 解析窗口下的共享资源属性(根XML节点名称："Window")，这些属性只有本窗口能使用
    */
    void ParseWindowShareAttributes(Window* pWindow, const pugi::xml_node& root) const;

    /** 解析全局资源的属性(根XML节点名称："Global")，这些属性，所有窗口都可以使用
    */
    void ParseGlobalAttributes(const pugi::xml_node& root) const;

    /** 解析XML节点的子节点
    * @param [in] xmlNode xml节点
    * @param [in] pParent 父控件，可能是普通控件（参数只传入，未用到），也可能是容器（用时转换为容器）
    * @return 返回第一个创建的节点，可能是普通控件，也可能是容器
    */
    Control* ParseXmlNodeChildren(const pugi::xml_node& xmlNode, Control* pParent = nullptr, Window* pWindow = nullptr);

    /** 根据控件的Class名称，创建控件（或容器）
    */
    Control* CreateControlByClass(const DString& strControlClass, Window* pWindow);

    /** 创建XML事件（XML节点为<Event>或者<BubbledEvent>）
    *   举例子：
    *   <Option text="单项选择" margin="8,0,0,0" borderround="2,2" valign="center">
    *       <Event type="buttonup" receiver="tree" applyattribute="multi_select={false}" />
    *   </Option>
    */
    void AttachXmlEvent(bool bBubbled, const pugi::xml_node& node, Control* pParent);

    /** 判断XML文件是否存在
    */
    bool IsXmlFileExists(const FilePath& xmlFilePath) const;

    /** 解析字体节点
    */
    void ParseFontXmlNode(const pugi::xml_node& xmlNode) const;

private:
    
    /** 当前解析的XML文档对象
    */
    std::unique_ptr<pugi::xml_document> m_xml;

    /** 创建Control的回调接口
    */
    CreateControlCallback m_createControlCallback;

    /** 当前解析的XML文件路径
    */
    FilePath m_xmlFilePath;
};

} // namespace ui

#endif // UI_CORE_WINDOWBUILDER_H_
