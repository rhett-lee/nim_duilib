#include "WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlDragable.h"
#include "duilib/Core/ScrollBar.h"

#include "duilib/Control/TreeView.h"
#include "duilib/Control/Combo.h"
#include "duilib/Control/ComboButton.h"
#include "duilib/Control/FilterCombo.h"
#include "duilib/Control/CheckCombo.h"
#include "duilib/Control/Slider.h"
#include "duilib/Control/Progress.h"
#include "duilib/Control/CircleProgress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/DateTime.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/GroupBox.h"

#include "duilib/Control/ColorControl.h"
#include "duilib/Control/ColorSlider.h"
#include "duilib/Control/ColorPickerRegular.h"
#include "duilib/Control/ColorPickerStatard.h"
#include "duilib/Control/ColorPickerStatardGray.h"
#include "duilib/Control/ColorPickerCustom.h"
#include "duilib/Control/Line.h"
#include "duilib/Control/IPAddress.h"
#include "duilib/Control/HotKey.h"
#include "duilib/Control/HyperLink.h"
#include "duilib/Control/ListCtrl.h"
#include "duilib/Control/PropertyGrid.h"
#include "duilib/Control/TabCtrl.h"

#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Box/TileBox.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Box/VirtualListBox.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/PathUtil.h"

#include "duilib/third_party/xml/pugixml.hpp"

namespace ui 
{

WindowBuilder::WindowBuilder()
{
    m_xml = std::make_unique<pugi::xml_document>();
}

WindowBuilder::~WindowBuilder()
{
}


Control* WindowBuilder::CreateControlByClass(const std::wstring& strControlClass, Window* pWindow)
{
    typedef std::function<Control* (Window* pWindow)> CreateControlFunction;
    static std::map<std::wstring, CreateControlFunction> createControlMap =
    {
        {DUI_CTR_BOX,  [](Window* pWindow) { return new Box(pWindow); }},
        {DUI_CTR_HBOX, [](Window* pWindow) { return new HBox(pWindow); }},
        {DUI_CTR_VBOX, [](Window* pWindow) { return new VBox(pWindow); }},
        {DUI_CTR_VTILE_BOX, [](Window* pWindow) { return new VTileBox(pWindow); }},
        {DUI_CTR_HTILE_BOX, [](Window* pWindow) { return new HTileBox(pWindow); }},
        {DUI_CTR_TABBOX, [](Window* pWindow) { return new TabBox(pWindow); }},

        {DUI_CTR_SCROLLBOX, [](Window* pWindow) { return new ScrollBox(pWindow); }},
        {DUI_CTR_HSCROLLBOX, [](Window* pWindow) { return new HScrollBox(pWindow); }},
        {DUI_CTR_VSCROLLBOX, [](Window* pWindow) { return new VScrollBox(pWindow); }},
        {DUI_CTR_HTILE_SCROLLBOX, [](Window* pWindow) { return new HTileScrollBox(pWindow); }},
        {DUI_CTR_VTILE_SCROLLBOX, [](Window* pWindow) { return new VTileScrollBox(pWindow); }},

        {DUI_CTR_LISTBOX_ITEM, [](Window* pWindow) { return new ListBoxItem(pWindow); }},
        {DUI_CTR_HLISTBOX, [](Window* pWindow) { return new HListBox(pWindow); }},
        {DUI_CTR_VLISTBOX, [](Window* pWindow) { return new VListBox(pWindow); }},
        {DUI_CTR_HTILE_LISTBOX, [](Window* pWindow) { return new HTileListBox(pWindow); }},
        {DUI_CTR_VTILE_LISTBOX, [](Window* pWindow) { return new VTileListBox(pWindow); }},
        {DUI_CTR_LISTCTRL, [](Window* pWindow) { return new ListCtrl(pWindow); }},
        {DUI_CTR_PROPERTY_GRID, [](Window* pWindow) { return new PropertyGrid(pWindow); }},

        {DUI_CTR_VIRTUAL_HTILE_LISTBOX, [](Window * pWindow) { return new VirtualHTileListBox(pWindow); }},
        {DUI_CTR_VIRTUAL_VTILE_LISTBOX, [](Window* pWindow) { return new VirtualVTileListBox(pWindow); }},
        {DUI_CTR_VIRTUAL_VLISTBOX, [](Window* pWindow) { return new VirtualVListBox(pWindow); }},
        {DUI_CTR_VIRTUAL_HLISTBOX, [](Window* pWindow) { return new VirtualHListBox(pWindow); }},

        {DUI_CTR_CONTROL, [](Window* pWindow) { return new Control(pWindow); }},
        {DUI_CTR_CONTROL_DRAGABLE, [](Window* pWindow) { return new ControlDragable(pWindow); }},
        {DUI_CTR_SCROLLBAR, [](Window* pWindow) { return new ScrollBar(pWindow); }},
        {DUI_CTR_LABEL, [](Window* pWindow) { return new Label(pWindow); }},
        {DUI_CTR_LABELBOX, [](Window* pWindow) { return new LabelBox(pWindow); }},
        {DUI_CTR_BUTTON, [](Window* pWindow) { return new Button(pWindow); }},
        {DUI_CTR_BUTTONBOX, [](Window* pWindow) { return new ButtonBox(pWindow); }},
        {DUI_CTR_OPTION, [](Window* pWindow) { return new Option(pWindow); }},
        {DUI_CTR_OPTIONBOX, [](Window* pWindow) { return new OptionBox(pWindow); }},
        {DUI_CTR_CHECKBOX, [](Window* pWindow) { return new CheckBox(pWindow); }},
        {DUI_CTR_CHECKBOXBOX, [](Window* pWindow) { return new CheckBoxBox(pWindow); }},
        {DUI_CTR_TREEVIEW, [](Window* pWindow) { return new TreeView(pWindow); }},
        {DUI_CTR_TREENODE, [](Window* pWindow) { return new TreeNode(pWindow); }},
        {DUI_CTR_COMBO, [](Window* pWindow) { return new Combo(pWindow); }},
        {DUI_CTR_COMBO_BUTTON, [](Window* pWindow) { return new ComboButton(pWindow); }},
        {DUI_CTR_FILTER_COMBO, [](Window* pWindow) { return new FilterCombo(pWindow); }},
        {DUI_CTR_CHECK_COMBO, [](Window* pWindow) { return new CheckCombo(pWindow); }},
        {DUI_CTR_SLIDER, [](Window* pWindow) { return new Slider(pWindow); }},
        {DUI_CTR_PROGRESS, [](Window* pWindow) { return new Progress(pWindow); }},
        {DUI_CTR_CIRCLEPROGRESS, [](Window* pWindow) { return new CircleProgress(pWindow); }},
        {DUI_CTR_RICHTEXT, [](Window* pWindow) { return new RichText(pWindow); }},
        {DUI_CTR_RICHEDIT, [](Window* pWindow) { return new RichEdit(pWindow); }},
        {DUI_CTR_DATETIME, [](Window* pWindow) { return new DateTime(pWindow); }},
        {DUI_CTR_COLOR_CONTROL, [](Window* pWindow) { return new ColorControl(pWindow); }},
        {DUI_CTR_COLOR_SLIDER, [](Window* pWindow) { return new ColorSlider(pWindow); }},
        {DUI_CTR_COLOR_PICKER_REGULAR, [](Window* pWindow) { return new ColorPickerRegular(pWindow); }},
        {DUI_CTR_COLOR_PICKER_STANDARD, [](Window* pWindow) { return new ColorPickerStatard(pWindow); }},
        {DUI_CTR_COLOR_PICKER_STANDARD_GRAY, [](Window* pWindow) { return new ColorPickerStatardGray(pWindow); }},
        {DUI_CTR_COLOR_PICKER_CUSTOM, [](Window* pWindow) { return new ColorPickerCustom(pWindow); }},
        {DUI_CTR_LINE, [](Window* pWindow) { return new Line(pWindow); }},
        {DUI_CTR_IPADDRESS, [](Window* pWindow) { return new IPAddress(pWindow); }},
        {DUI_CTR_HOTKEY, [](Window* pWindow) { return new HotKey(pWindow); }},
        {DUI_CTR_HYPER_LINK, [](Window* pWindow) { return new HyperLink(pWindow); }},
        {DUI_CTR_TAB_CTRL, [](Window* pWindow) { return new TabCtrl(pWindow); }},
        {DUI_CTR_TAB_CTRL_ITEM, [](Window* pWindow) { return new TabCtrlItem(pWindow); }},

        {DUI_CTR_SPLIT, [](Window* pWindow) { return new Split(pWindow); }},
        {DUI_CTR_SPLITBOX, [](Window* pWindow) { return new SplitBox(pWindow); }},
        {DUI_CTR_GROUP_BOX, [](Window* pWindow) { return new GroupBox(pWindow); }},
        {DUI_CTR_GROUP_HBOX, [](Window* pWindow) { return new GroupHBox(pWindow); }},
        {DUI_CTR_GROUP_VBOX, [](Window* pWindow) { return new GroupVBox(pWindow); }},

        {DUI_CTR_BOX_DRAGABLE, [](Window* pWindow) { return new BoxDragable(pWindow); }},
        {DUI_CTR_HBOX_DRAGABLE, [](Window* pWindow) { return new HBoxDragable(pWindow); }},
        {DUI_CTR_VBOX_DRAGABLE, [](Window* pWindow) { return new VBoxDragable(pWindow); }},
    };
    Control* pControl = nullptr;
    auto iter = createControlMap.find(strControlClass);
    if (iter != createControlMap.end()) {
        pControl = iter->second(pWindow);
    }
    return pControl;
}

bool WindowBuilder::IsXmlFileExists(const std::wstring& xml) const
{
    if (xml.empty()) {
        return false;
    }
    if (GlobalManager::Instance().Zip().IsUseZip()) {
        std::wstring sFile = PathUtil::JoinFilePath(GlobalManager::Instance().GetResourcePath(), xml);
        if (GlobalManager::Instance().Zip().IsZipResExist(sFile)) {
            return true;
        }
    }
    std::wstring xmlFilePath = GlobalManager::Instance().GetResourcePath();
    if (PathUtil::IsRelativePath(xml)) {
        xmlFilePath = PathUtil::JoinFilePath(xmlFilePath, xml);
    }
    else {
        xmlFilePath = xml;
    }
    return PathUtil::IsExistsPath(xmlFilePath);
}

Box* WindowBuilder::Create(const std::wstring& xml, 
                           CreateControlCallback pCallback,
                           Window* pWindow, 
                           Box* pParent, 
                           Box* pUserDefinedBox)
{
    ASSERT(!xml.empty() && _T("xml 参数为空！"));
    if (xml.empty()) {
        return nullptr;
    }
    bool isLoaded = false;
    //字符串以<开头认为是XML字符串，否则认为是XML文件
    //如果使用了 zip 压缩包，则从内存中读取
    if (xml.front() == _T('<')) {
        pugi::xml_parse_result result = m_xml->load_buffer(xml.c_str(), 
                                                           xml.size() * sizeof(std::wstring::value_type), 
                                                           pugi::parse_default, 
                                                           pugi::xml_encoding::encoding_utf16);
        if (result.status != pugi::status_ok) {
            ASSERT(!_T("WindowBuilder::Create load xml from string data failed!"));
            return nullptr;
        }
        isLoaded = true;
    }
    if (!isLoaded && GlobalManager::Instance().Zip().IsUseZip()) {
        std::wstring sFile = PathUtil::JoinFilePath(GlobalManager::Instance().GetResourcePath(), xml);
        std::vector<unsigned char> file_data;
        if (GlobalManager::Instance().Zip().GetZipData(sFile, file_data)) {
            pugi::xml_parse_result result = m_xml->load_buffer(file_data.data(), file_data.size());
            if (result.status != pugi::status_ok) {
                ASSERT(!_T("WindowBuilder::Create load xml from zip data failed!"));
                return nullptr;
            }
            isLoaded = true;
        }
    }
    if(!isLoaded) {
        std::wstring xmlFilePath = GlobalManager::Instance().GetResourcePath();
        if (PathUtil::IsRelativePath(xml)) {
            xmlFilePath = PathUtil::JoinFilePath(xmlFilePath, xml);
        }
        else {
            xmlFilePath = xml;
        }
        pugi::xml_parse_result result = m_xml->load_file(xmlFilePath.c_str());
        if (result.status != pugi::status_ok) {
            ASSERT(!_T("WindowBuilder::Create load xml file failed!"));
            return nullptr;
        }
        isLoaded = true;
    }
    m_xmlFilePath = xml;
    return Create(pCallback, pWindow, pParent, pUserDefinedBox);
}

Box* WindowBuilder::Create(CreateControlCallback pCallback, Window* pWindow, Box* pParent, Box* pUserDefinedBox)
{
    m_createControlCallback = pCallback;
    pugi::xml_node root = m_xml->root().first_child();
    ASSERT(!root.empty());
    if (root.empty()) {
        return nullptr;
    }

    if( pWindow != nullptr) {
        std::wstring strClass;
        std::wstring strName;
        std::wstring strValue;
        strClass = root.name();

        if( strClass == _T("Window") ) {
            if( pWindow->GetHWND() ) {
                //首先处理mininfo和maxinfo，因为其他属性有用到这两个属性的
                for (pugi::xml_attribute attr : root.attributes()) {
                    strName = attr.name();
                    strValue = attr.value();
                    if (strName == _T("mininfo")) {
                        UiSize size;
                        AttributeUtil::ParseSizeValue(strValue.c_str(), size);
                        pWindow->SetMinInfo(size.cx, size.cy, false, true);
                    }
                    else if (strName == _T("maxinfo")) {
                        UiSize size;
                        AttributeUtil::ParseSizeValue(strValue.c_str(), size);
                        pWindow->SetMaxInfo(size.cx, size.cy, false, true);
                    }
                }
                for (pugi::xml_attribute attr : root.attributes()) {
                    strName = attr.name();
                    strValue = attr.value();
                    if (strName == _T("size")) {
                        UiSize windowSize;
                        AttributeUtil::ParseWindowSize(pWindow, strValue.c_str(), windowSize);
                        int32_t cx = windowSize.cx;
                        int32_t cy = windowSize.cy;
                        UiSize minSize = pWindow->GetMinInfo(false);
                        UiSize maxSize = pWindow->GetMaxInfo(false);
                        if ((minSize.cx > 0) && (cx < minSize.cx)) {
                            cx = minSize.cx;
                        }
                        if ((maxSize.cx > 0) && (cx > maxSize.cx)) {
                            cx = maxSize.cx;
                        }
                        if ((minSize.cy > 0) && (cy < minSize.cy)) {
                            cy = minSize.cy;
                        }
                        if ((maxSize.cy > 0) && (cy > maxSize.cy)) {
                            cy = maxSize.cy;
                        }
                        pWindow->SetInitSize(cx, cy, false, false);
                    }
                    else if( strName == _T("sizebox") ) {
                        UiRect rcSizeBox;
                        AttributeUtil::ParseRectValue(strValue.c_str(), rcSizeBox);
                        pWindow->SetSizeBox(rcSizeBox, true);
                    }
                    else if( strName == _T("caption") ) {
                        UiRect rcCaption;
                        AttributeUtil::ParseRectValue(strValue.c_str(), rcCaption);
                        pWindow->SetCaptionRect(rcCaption, true);
                    }
                    else if (strName == _T("use_system_caption")) {
                        pWindow->SetUseSystemCaption(strValue == _T("true"));
                    }
                    else if( strName == _T("text") ) {
                        pWindow->SetText(strValue);
                    }
                    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
                        pWindow->SetTextId(strValue);
                    }
                    else if (strName == _T("round_corner") || strName == _T("roundcorner") ) {
                        UiSize size;
                        AttributeUtil::ParseSizeValue(strValue.c_str(), size);
                        pWindow->SetRoundCorner(size.cx, size.cy, true);
                    }                                
                    else if (strName == _T("alpha_fix_corner") || strName == _T("alphafixcorner")) {
                        UiRect rc;
                        AttributeUtil::ParseRectValue(strValue.c_str(), rc);
                        pWindow->SetAlphaFixCorner(rc, true);
                    }
                    else if (strName == _T("render_transparent")) {
                        pWindow->SetRenderTransparent(strValue == _T("true"));
                    }
                    else if ((strName == _T("shadow_attached")) || (strName == _T("shadowattached"))) {
                        //设置是否支持窗口阴影（阴影实现有两种：层窗口和普通窗口）
                        pWindow->SetShadowAttached(strValue == _T("true"));
                    }
                    else if ((strName == _T("shadow_image")) || (strName == _T("shadowimage"))) {
                        //设置阴影图片
                        pWindow->SetShadowImage(strValue);
                    }
                    else if ((strName == _T("shadow_corner")) || (strName == _T("shadowcorner"))) {
                        //设置窗口阴影的九宫格属性
                        UiPadding padding;
                        AttributeUtil::ParsePaddingValue(strValue.c_str(), padding);
                        pWindow->SetShadowCorner(padding, true);
                    }
                    else if ((strName == _T("layered_window")) || (strName == _T("layeredwindow"))) {
                        //设置是否设置层窗口属性（层窗口还是普通窗口）
                        pWindow->SetLayeredWindow(strValue == _T("true"));
                    }
                    else if (strName == _T("alpha")) {
                        //设置窗口的透明度（0 - 255），仅当使用层窗口时有效
                        int nAlpha = wcstol(strValue.c_str(), nullptr, 10);
                        ASSERT(nAlpha >= 0 && nAlpha <= 255);
                        if ((nAlpha >= 0) && (nAlpha <= 255)) {
                            pWindow->SetWindowAlpha(nAlpha);
                        }
                    }
                }
            }
        }

        if( strClass == _T("Global") ) {
            for(pugi::xml_node node : root.children()) {
                strClass = node.name();
                if( strClass == _T("Image") ) {
                    ASSERT(FALSE);    //废弃
                }
                else if (strClass == _T("FontResource")) {
                    std::wstring strFontFile;
                    std::wstring strFontName;
                    for (pugi::xml_attribute attr : node.attributes()) {
                        strName = attr.name();
                        strValue = attr.value();
                        if (strName == _T("file")) {
                            strFontFile = strValue;
                        }
                        else if (strName == _T("name")) {
                            strFontName = strValue;
                        }
                    }
                    if (!strFontFile.empty() && !strFontName.empty()) {
                        GlobalManager::Instance().Font().AddFontFile(strFontFile, strFontName);
                    }
                }
                else if( strClass == _T("Font")) {
                    ParseFontXmlNode(node);
                }
                else if( strClass == _T("Class") ) {
                    std::wstring strClassName;
                    std::wstring strAttribute;
                    for (pugi::xml_attribute attr : node.attributes()) {
                        strName = attr.name();
                        strValue = attr.value();
                        if( strName == _T("name") ) {
                            strClassName = strValue;
                        }
                        else {
                            strAttribute.append(StringUtil::Printf(_T(" %s=\"%s\""),
                                                strName.c_str(), strValue.c_str()));
                        }
                    }
                    if( !strClassName.empty() ) {
                        StringUtil::TrimLeft(strAttribute);
                        GlobalManager::Instance().AddClass(strClassName, strAttribute);
                    }
                }
                else if( strClass == _T("TextColor") ) {
                    std::wstring colorName = node.attribute(_T("name")).as_string();
                    std::wstring colorValue = node.attribute(_T("value")).as_string();
                    if(!colorName.empty() && !colorValue.empty()) {
                        ColorManager& colorManager = GlobalManager::Instance().Color();
                        colorManager.AddColor(colorName, colorValue);
                        if (colorName == _T("default_font_color")) {
                            colorManager.SetDefaultTextColor(colorName);
                        }
                        else if (colorName == _T("disabled_font_color")) {
                            colorManager.SetDefaultDisabledTextColor(colorName);
                        }
                    }
                }
            }
        }
        else if (strClass == _T("Window"))
        {
            for (pugi::xml_node node : root.children()) {
                strClass = node.name();
                if (strClass == _T("Class")) {
                    std::wstring strClassName;
                    std::wstring strAttribute;
                    for (pugi::xml_attribute attr : node.attributes()) {
                        strName = attr.name();
                        strValue = attr.value();
                        if (strName == _T("name")) {
                            strClassName = strValue;
                        }
                        else {
                            strAttribute.append(StringUtil::Printf(_T(" %s=\"%s\""),
                                                strName.c_str(), strValue.c_str()));
                        }
                    }
                    if (!strClassName.empty()) {
                        ASSERT(GlobalManager::Instance().GetClassAttributes(strClassName).empty());    //窗口中的Class不能与全局的重名
                        StringUtil::TrimLeft(strAttribute);
                        pWindow->AddClass(strClassName, strAttribute);
                    }
                }
                else if (strClass == _T("TextColor")) {
                    std::wstring strColorName;
                    std::wstring strColor;
                    for (pugi::xml_attribute attr : node.attributes()) {
                        strName = attr.name();
                        strValue = attr.value();
                        if (strName == _T("name")) {
                            strColorName = strValue;
                        }
                        else if (strName == _T("value")) {
                            strColor = strValue;
                        }
                    }
                    if (!strColorName.empty()) {
                        pWindow->AddTextColor(strColorName, strColor);
                    }
                }
                else if (strClass == _T("Font")) {
                    //Window节点下，允许定义字体
                    ParseFontXmlNode(node);
                }
            }
        }
    }

    for (pugi::xml_node node : root.children()) {
        std::wstring strClass = node.name();
        if (strClass == _T("Image") || strClass == _T("FontResource") || strClass == _T("Font")
            || strClass == _T("Class") || strClass == _T("TextColor")) {

        }
        else {
            if (!pUserDefinedBox) {
                return (Box*)ParseXmlNode(root, pParent, pWindow);
            }
            else {
                ParseXmlNode(node, pUserDefinedBox, pWindow);
                int i = 0;
                for (pugi::xml_attribute attr : node.attributes()) {
                    if (wcscmp(attr.name(), _T("class")) == 0) {
                        //class必须是第一个属性
                        ASSERT(i == 0);
                    }
                    ++i;
                    pUserDefinedBox->SetAttribute(attr.name(), attr.value());
                }
                return pUserDefinedBox;
            }
        }
    }

    return nullptr;
}

void WindowBuilder::ParseFontXmlNode(const pugi::xml_node& xmlNode) const
{
    std::wstring strName;
    std::wstring strValue;

    std::wstring strFontId;
    std::wstring strFontName;
    int size = 12;
    bool bold = false;
    bool underline = false;
    bool strikeout = false;
    bool italic = false;
    bool isDefault = false;
    for (pugi::xml_attribute attr : xmlNode.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if (strName == _T("id"))
        {
            strFontId = strValue;
        }
        else if (strName == _T("name")) {
            strFontName = strValue;
        }
        else if (strName == _T("size")) {
            size = wcstol(strValue.c_str(), nullptr, 10);
        }
        else if (strName == _T("bold")) {
            bold = (strValue == _T("true"));
        }
        else if (strName == _T("underline")) {
            underline = (strValue == _T("true"));
        }
        else if (strName == _T("strikeout")) {
            strikeout = (strValue == _T("true"));
        }
        else if (strName == _T("italic")) {
            italic = (strValue == _T("true"));
        }
        else if (strName == _T("default")) {
            isDefault = (strValue == _T("true"));
        }
    }
    if (!strFontName.empty() && !strFontId.empty()) {
        UiFont fontInfo;
        fontInfo.m_fontName = strFontName;
        fontInfo.m_fontSize = size;
        fontInfo.m_bBold = bold;
        fontInfo.m_bItalic = italic;
        fontInfo.m_bUnderline = underline;
        fontInfo.m_bStrikeOut = strikeout;
        GlobalManager::Instance().Font().AddFont(strFontId, fontInfo, isDefault);
    }
}

Control* WindowBuilder::ParseXmlNode(const pugi::xml_node& xmlNode, Control* pParent, Window* pWindow)
{
    if (xmlNode.empty()) {
        return nullptr;
    }
    Control* pReturn = nullptr;
    for (pugi::xml_node node : xmlNode.children()) {
        std::wstring strClass = node.name();
        if( (strClass == _T("Image")) || 
            (strClass == _T("Font"))  ||
            (strClass == _T("Class")) || 
            (strClass == _T("TextColor")) ) {
                continue;
        }

        Control* pControl = nullptr;
        if (strClass == _T("Include")) {
            if (node.attributes().empty()) {
                continue;
            }
            pugi::xml_attribute countAttr = node.attribute(_T("count"));
            int nCount = countAttr.as_int();
            if (nCount <= 0) {
                //默认值设置为1，count这个属性参数为可选
                nCount = 1;
            }
            pugi::xml_attribute sourceAttr = node.attribute(_T("src"));
            std::wstring sourceValue = sourceAttr.as_string();
            if (sourceValue.empty()) {
                sourceAttr = node.attribute(_T("source"));
                sourceValue = sourceAttr.as_string();                
            }
            if (!sourceValue.empty()) {
                StringUtil::ReplaceAll(_T("/"), _T("\\"), sourceValue);
                if (!m_xmlFilePath.empty()) {
                    //优先尝试在原XML文件相同目录加载
                    size_t pos = m_xmlFilePath.find_last_of(_T("\\/"));
                    if (pos != std::wstring::npos) {
                        std::wstring filePath = m_xmlFilePath.substr(0, pos);
                        filePath = PathUtil::JoinFilePath(filePath, sourceValue);
                        if (IsXmlFileExists(filePath)) {
                            sourceValue = filePath;
                        }
                    }
                }
            }
            ASSERT(!sourceValue.empty());
            if (sourceValue.empty()) {
                continue;
            }
            for ( int i = 0; i < nCount; i++ ) {
                WindowBuilder builder;
                pControl = builder.Create(sourceValue, m_createControlCallback, pWindow, (Box*)pParent);
            }
            continue;
        }
        else {
            pControl = CreateControlByClass(strClass, pWindow);
            if (pControl == nullptr) {
                if ((strClass == _T("Event")) || 
                    (strClass == _T("BubbledEvent"))) {
                    bool bBubbled = (strClass == _T("BubbledEvent"));
                    AttachXmlEvent(bBubbled, node, pParent);
                    continue;
                }
            }

            // User-supplied control factory
            if( pControl == nullptr) {
                pControl = GlobalManager::Instance().CreateControl(strClass);
            }

            if( pControl == nullptr && m_createControlCallback ) {
                pControl = m_createControlCallback(strClass);
            }
        }

        if(pControl == nullptr) {
            std::wstring nodeName = strClass;
            ASSERT(!"Found unknown node name, can't create control!");
            continue;
        }

        // TreeView相关必须先添加后解析
        if (strClass == DUI_CTR_TREENODE) {
            bool bAdded = false;
            TreeNode* pNode = dynamic_cast<TreeNode*>(pControl);
            ASSERT(pNode != nullptr);
            TreeView* pTreeView = dynamic_cast<TreeView*>(pParent);
            if (pTreeView != nullptr) {
                //一级子节点
                pTreeView->GetRootNode()->AddChildNode(pNode);
                bAdded = true;
            }
            else {
                //多级子节点
                TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pParent);
                if (pTreeNode != nullptr) {
                    pTreeNode->AddChildNode(pNode);
                    bAdded = true;
                }
            }
            if (!bAdded) {
                //尝试Combo控件
                Combo* pCombo = dynamic_cast<Combo*>(pParent);
                if (pCombo != nullptr) {
                    pCombo->GetTreeView()->GetRootNode()->AddChildNode(pNode);
                    bAdded = true;
                }
            }
            ASSERT(bAdded);
        }

        pControl->SetWindow(pWindow);
        
        // Process attributes
        if(!node.attributes().empty()) {
            //读取节点的属性，设置控件的属性
            int i = 0;
            for (pugi::xml_attribute attr : node.attributes()) {
                ASSERT(i == 0 || wcscmp(attr.name(), _T("class")) != 0);    //class必须是第一个属性
                ++i;
                pControl->SetAttribute(attr.name(), attr.value());
            }
        }

        if (strClass == DUI_CTR_RICHTEXT) {
            ParseRichTextXmlNode(node, pControl);
#ifdef _DEBUG
            //测试效果：反向生成带格式的文本，用于测试验证解析的正确性
            RichText* pRichText = dynamic_cast<RichText*>(pControl);
            std::wstring richText;
            if (pRichText) {
                richText = pRichText->ToString();
            }
            richText.clear();
#endif
        }
        else {
            // Add children
            if (!node.children().empty()) {
                //递归该节点的所有子节点，继续添加
                ParseXmlNode(node, (Box*)pControl, pWindow);
            }
        }

        // Attach to parent
        // 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
        if (pParent != nullptr && strClass != DUI_CTR_TREENODE) {
            Box* pContainer = dynamic_cast<Box*>(pParent);
            ASSERT(pContainer != nullptr);
            if (pContainer == nullptr) {
                return nullptr;
            }
            if( !pContainer->AddItem(pControl) ) {
                ASSERT(FALSE);
                delete pControl;
                continue;
            }
        }
        
        // Return first item
        if (pReturn == nullptr) {
            pReturn = pControl;
        }
    }
    return pReturn;
}

bool WindowBuilder::ParseRichTextXmlText(const std::wstring& xmlText, Control* pControl)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xmlText.c_str(),
                                                    xmlText.size() * sizeof(std::wstring::value_type),
                                                    pugi::parse_default,
                                                    pugi::xml_encoding::encoding_utf16);
    if (result.status != pugi::status_ok) {
        ASSERT(!_T("WindowBuilder::ParseRichTextXmlText load xml text failed!"));
        return false;
    }
    pugi::xml_node root = doc.root();
    std::wstring rootName = root.name();
    std::wstring rootValue = root.value();
    if (rootName.empty() && rootValue.empty()) {
        root = doc.root().first_child();
    }
    rootName = root.name();
    ASSERT(rootName == DUI_CTR_RICHTEXT);
    if (rootName != DUI_CTR_RICHTEXT) {
        return false;
    }
    return ParseRichTextXmlNode(root, pControl, nullptr);
}

bool WindowBuilder::ParseRichTextXmlNode(const pugi::xml_node& xmlNode, Control* pControl, RichTextSlice* pTextSlice)
{
    RichText* pRichText = dynamic_cast<RichText*>(pControl);
    ASSERT(pRichText != nullptr);
    if (pRichText == nullptr) {
        return false;
    }

    std::wstring nodeName;
    for (pugi::xml_node node : xmlNode.children()) {
        RichTextSlice textSlice;
        textSlice.m_nodeName = node.name();
        nodeName = textSlice.m_nodeName.c_str();

        bool bParseChildren = true;
        if (nodeName.empty()) {
            std::wstring nodeValue = node.value();
            if (!nodeValue.empty()) {
                textSlice.m_text = pRichText->TrimText(nodeValue);
            }
            //无节点名称，只读取文本内容, 不需要递归遍历子节点
            bParseChildren = false;
        }        
        else if (nodeName == _T("a")) {            
            textSlice.m_text = pRichText->TrimText(node.first_child().value());
            textSlice.m_linkUrl = StringUtil::Trim(node.attribute(_T("href")).as_string());
            //超级链接节点, 不需要递归遍历子节点
            bParseChildren = false;
        }
        else if (nodeName == _T("b")) {
            //粗体字
            textSlice.m_fontInfo.m_bBold = true;
        }
        else if (nodeName == _T("i")) {
            //斜体字
            textSlice.m_fontInfo.m_bItalic = true;
        }
        else if ((nodeName == _T("del")) || (nodeName == _T("s")) || (nodeName == _T("strike"))) {
            //删除字
            textSlice.m_fontInfo.m_bStrikeOut = true;
        }
        else if ( (nodeName == _T("ins")) || (nodeName == _T("u")) ){
            //下划线
            textSlice.m_fontInfo.m_bUnderline = true;
        }
        else if (nodeName == _T("bgcolor")) {
            //背景颜色
            textSlice.m_bgColor = StringUtil::Trim(node.attribute(_T("color")).as_string());
        }
        else if (nodeName == _T("font")) {
            //字体设置：文本颜色
            textSlice.m_textColor = node.attribute(_T("color")).as_string();
            textSlice.m_fontInfo.m_fontName = node.attribute(_T("face")).as_string();
            //字号不需要进行DPI缩放，绘制的时候，会根据当时的DPI进行缩放
            textSlice.m_fontInfo.m_fontSize = node.attribute(_T("size")).as_int();            
        }
        else if (nodeName == _T("br")) {
            textSlice.m_text = _T("\n");
            //换行节点, 不需要递归遍历子节点
            bParseChildren = false;
        }
        else {
            //遇到不认识的节点，忽略
            ASSERT(!"Found unknown xml node name!");
            continue;
        }
        if (bParseChildren) {
            //递归子节点
            ParseRichTextXmlNode(node, pRichText, &textSlice);
        }
        //将子节点添加到Control或者父节点(注意：std::move以后，textSlice对象失效)
        if (pTextSlice != nullptr) {
            pTextSlice->m_childs.emplace_back(std::move(textSlice));
        }
        else {
            pRichText->AppendTextSlice(std::move(textSlice));
        }
    }
    return true;
}

void WindowBuilder::AttachXmlEvent(bool bBubbled, const pugi::xml_node& node, Control* pParent)
{
    ASSERT(pParent != nullptr);
    if (pParent == nullptr) {
        return;
    }
    std::wstring strType;
    std::wstring strReceiver;
    std::wstring strApplyAttribute;
    std::wstring strName;
    std::wstring strValue;
    int i = 0;
    for (pugi::xml_attribute attr : node.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        ASSERT(i != 0 || strName == _T("type"));
        ASSERT(i != 1 || strName == _T("receiver"));
        ASSERT(i != 2 || strName == _T("applyattribute"));
        ++i;
        if( strName == _T("type") ) {
            strType = strValue;
        }
        else if( strName == _T("receiver") ) {
            strReceiver = strValue;
        }
        else if( strName == _T("applyattribute") ) {
            strApplyAttribute = strValue;
        }
    }

    auto typeList = StringUtil::Split(strType, _T(" "));
    auto receiverList = StringUtil::Split(strReceiver, _T(" "));
    for (auto itType = typeList.begin(); itType != typeList.end(); itType++) {
        if (receiverList.empty()) {
            receiverList.push_back(_T(""));
        }
        for (auto itReceiver = receiverList.begin(); itReceiver != receiverList.end(); itReceiver++) {
            EventType eventType = StringToEventType(*itType);
            if (eventType == EventType::kEventNone) {
                continue;
            }
            auto callback = UiBind(&Control::OnApplyAttributeList, pParent, *itReceiver, strApplyAttribute, std::placeholders::_1);
            if (!bBubbled) {
                pParent->AttachXmlEvent(eventType, callback);
            }
            else {
                pParent->AttachXmlBubbledEvent(eventType, callback);
            }
        }
    }
}

} // namespace ui
