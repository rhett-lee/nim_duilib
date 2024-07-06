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
    @param [in] xmlFileData 是文件文本内容，字符串需要以字符 '<'开头;
    @param [in] pWindow 关联的窗口
    @param [in] pParent 父容器
    */
    Box* CreateFromXmlData(const DString& xmlFileData, 
                           CreateControlCallback pCallback = CreateControlCallback(),
                           Window* pWindow = nullptr, 
                           Box* pParent = nullptr, 
                           Box* pUserDefinedBox = nullptr);

    /** 使用XML文件创建窗口布局等
    @param [in] xmlFilePath XML文件的路径
    @param [in] pWindow 关联的窗口
    @param [in] pParent 父容器
    */
    Box* CreateFromXmlFile(const FilePath& xmlFilePath, 
                           CreateControlCallback pCallback = CreateControlCallback(),
                           Window* pWindow = nullptr, 
                           Box* pParent = nullptr, 
                           Box* pUserDefinedBox = nullptr);

    /** 使用缓存中的XML文件(即上面的Create函数传入的XML文件)创建窗口布局等
    */
    Box* Create(CreateControlCallback pCallback = CreateControlCallback(), 
                Window* pWindow = nullptr,
                Box* pParent = nullptr, 
                Box* pUserDefinedBox = nullptr);

public:
    /** 解析带格式的文本内容，并设置到RichText Control对象
    */
    static bool ParseRichTextXmlText(const DString& xmlText, Control* pControl);
    
    /** 解析带格式的文本内容，并设置到RichText Control对象
    */
    static bool ParseRichTextXmlNode(const pugi::xml_node& xmlNode, Control* pControl, RichTextSlice* pTextSlice = nullptr);

private:
    Control* ParseXmlNode(const pugi::xml_node& xmlNode, Control* pParent = nullptr, Window* pWindow = nullptr);
    Control* CreateControlByClass(const DString& strControlClass, Window* pWindow);
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
