#ifndef UI_CORE_WINDOWBUILDER_H_
#define UI_CORE_WINDOWBUILDER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <functional>
#include <string>
#include <memory>

namespace ui 
{

class Box;
class Window;
class Control;

class CMarkup;
class CMarkupNode;

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


private:
    Control* _Parse(CMarkupNode* parent, Control* pParent = NULL, Window* pWindow = NULL);
	Control* CreateControlByClass(const std::wstring& strControlClass);
	void AttachXmlEvent(bool bBubbled, CMarkupNode& node, Control* pParent);

private:
    std::unique_ptr<CMarkup> m_xml;
	CreateControlCallback m_createControlCallback;
};

} // namespace ui

#endif // UI_CORE_WINDOWBUILDER_H_
