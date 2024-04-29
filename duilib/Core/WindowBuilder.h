#ifndef UI_CORE_WINDOWBUILDER_H_
#define UI_CORE_WINDOWBUILDER_H_

#pragma once

#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiRect.h"
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
typedef std::function<Control* (const std::wstring&)> CreateControlCallback;

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
    /** 使用XML文件创建窗口布局等
    @param [in] xml 可以是文件文本内容，如果是XML文件内容，字符串需要以字符 '<'开头;
                也可以是XML文件的路径
    @param [in] pWindow 关联的窗口
    @param [in] pParent 父容器
    */
    Box* Create(const std::wstring& xml, 
                CreateControlCallback pCallback = CreateControlCallback(),
                Window* pWindow = nullptr, 
                Box* pParent = nullptr, 
                Box* pUserDefinedBox = nullptr);

    //使用缓存中的XML文件(即上面的Create函数传入的XML文件)创建窗口布局等
    Box* Create(CreateControlCallback pCallback = CreateControlCallback(), 
                Window* pWindow = nullptr,
                Box* pParent = nullptr, 
                Box* pUserDefinedBox = nullptr);

public:
    /** 解析带格式的文本内容，并设置到RichText Control对象
    */
    static bool ParseRichTextXmlText(const std::wstring& xmlText, Control* pControl);
    
    /** 解析带格式的文本内容，并设置到RichText Control对象
    */
    static bool ParseRichTextXmlNode(const pugi::xml_node& xmlNode, Control* pControl, RichTextSlice* pTextSlice = nullptr);

private:
    Control* ParseXmlNode(const pugi::xml_node& xmlNode, Control* pParent = nullptr, Window* pWindow = nullptr);
	Control* CreateControlByClass(const std::wstring& strControlClass);
	void AttachXmlEvent(bool bBubbled, const pugi::xml_node& node, Control* pParent);

    /** 判断XML文件是否存在
    */
    bool IsXmlFileExists(const std::wstring& xml) const;

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
    std::wstring m_xmlFilePath;
};

} // namespace ui

#endif // UI_CORE_WINDOWBUILDER_H_
