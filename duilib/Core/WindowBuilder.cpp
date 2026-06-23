#include "WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlDragable.h"
#include "duilib/Core/ControlMovable.h"
#include "duilib/Core/ControlResizable.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/WindowCreateAttributes.h"

#include "duilib/Control/TreeView.h"
#include "duilib/Control/DirectoryTree.h"
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
#include "duilib/Control/ColorPickerStandard.h"
#include "duilib/Control/ColorPickerStandardGray.h"
#include "duilib/Control/ColorPickerCustom.h"
#include "duilib/Control/Line.h"
#include "duilib/Control/IPAddress.h"
#include "duilib/Control/HotKey.h"
#include "duilib/Control/HyperLink.h"
#include "duilib/Control/ListCtrl.h"
#include "duilib/Control/PropertyGrid.h"
#include "duilib/Control/TabCtrl.h"
#include "duilib/Control/IconControl.h"
#include "duilib/Control/BitmapControl.h"
#include "duilib/Control/AddressBar.h"
#include "duilib/Control/MenuListBox.h"
#include "duilib/Control/MenuBar.h"
#include "duilib/Control/ChildWindow.h"

#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/XmlBox.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Box/GridBox.h"
#include "duilib/Box/TileBox.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Box/VirtualListBox.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"

#include "duilib/third_party/xml/pugixml.hpp"
#include <set>

namespace ui 
{

WindowBuilder::WindowBuilder()
{
    m_xml = std::make_unique<pugi::xml_document>();
}

WindowBuilder::~WindowBuilder()
{
    m_xml.reset();
}

Control* WindowBuilder::CreateControlByClass(const DString& strControlClass, Window* pWindow)
{
    typedef std::function<Control* (Window* pWindow)> CreateControlFunction;
    static std::map<DString, CreateControlFunction> createControlMap =
    {
        {DUI_CTR_BOX,  [](Window* pWindow) { return new Box(pWindow); }},
        {DUI_CTR_HBOX, [](Window* pWindow) { return new HBox(pWindow); }},
        {DUI_CTR_VBOX, [](Window* pWindow) { return new VBox(pWindow); }},
        {DUI_CTR_HFLOWBOX, [](Window* pWindow) { return new HFlowBox(pWindow); }},
        {DUI_CTR_VFLOWBOX, [](Window* pWindow) { return new VFlowBox(pWindow); }},
        {DUI_CTR_XMLBOX,  [](Window* pWindow) { return new XmlBox(pWindow); }},
        {DUI_CTR_VTILE_BOX, [](Window* pWindow) { return new VTileBox(pWindow); }},
        {DUI_CTR_HTILE_BOX, [](Window* pWindow) { return new HTileBox(pWindow); }},
        {DUI_CTR_TABBOX, [](Window* pWindow) { return new TabBox(pWindow); }},
        {DUI_CTR_GRIDBOX, [](Window* pWindow) { return new GridBox(pWindow); }},
        {DUI_CTR_GRID_SCROLLBOX, [](Window* pWindow) { return new GridScrollBox(pWindow); }},

        {DUI_CTR_SCROLLBOX, [](Window* pWindow) { return new ScrollBox(pWindow); }},
        {DUI_CTR_HSCROLLBOX, [](Window* pWindow) { return new HScrollBox(pWindow); }},
        {DUI_CTR_VSCROLLBOX, [](Window* pWindow) { return new VScrollBox(pWindow); }},
        {DUI_CTR_HFLOW_SCROLLBOX, [](Window* pWindow) { return new HFlowScrollBox(pWindow); }},
        {DUI_CTR_VFLOW_SCROLLBOX, [](Window* pWindow) { return new VFlowScrollBox(pWindow); }},
        {DUI_CTR_HTILE_SCROLLBOX, [](Window* pWindow) { return new HTileScrollBox(pWindow); }},
        {DUI_CTR_VTILE_SCROLLBOX, [](Window* pWindow) { return new VTileScrollBox(pWindow); }},

        {DUI_CTR_LISTBOX_ITEM, [](Window* pWindow) { return new ListBoxItem(pWindow); }},
        {DUI_CTR_LISTBOX_ITEM_HBOX, [](Window* pWindow) { return new ListBoxItemH(pWindow); }},
        {DUI_CTR_LISTBOX_ITEM_VBOX, [](Window* pWindow) { return new ListBoxItemV(pWindow); }},
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
        {DUI_CTR_CONTROL_MOVABLE, [](Window* pWindow) { return new ControlMovable(pWindow); }},
        {DUI_CTR_CONTROL_RESIZABLE, [](Window* pWindow) { return new ControlResizable(pWindow); }},
        {DUI_CTR_SCROLLBAR, [](Window* pWindow) { return new ScrollBar(pWindow); }},
        {DUI_CTR_LABEL, [](Window* pWindow) { return new Label(pWindow); }},
        {DUI_CTR_LABELBOX, [](Window* pWindow) { return new LabelBox(pWindow); }},
        {DUI_CTR_LABELHBOX, [](Window* pWindow) { return new LabelHBox(pWindow); } },
        {DUI_CTR_LABELVBOX, [](Window* pWindow) { return new LabelVBox(pWindow); }},
        {DUI_CTR_BUTTON, [](Window* pWindow) { return new Button(pWindow); }},
        {DUI_CTR_BUTTONBOX, [](Window* pWindow) { return new ButtonBox(pWindow); }},
        {DUI_CTR_BUTTONHBOX, [](Window* pWindow) { return new ButtonHBox(pWindow); }},
        {DUI_CTR_BUTTONVBOX, [](Window* pWindow) { return new ButtonVBox(pWindow); }},
        {DUI_CTR_OPTION, [](Window* pWindow) { return new Option(pWindow); }},
        {DUI_CTR_OPTIONBOX, [](Window* pWindow) { return new OptionBox(pWindow); }},
        {DUI_CTR_CHECKBOX, [](Window* pWindow) { return new CheckBox(pWindow); }},
        {DUI_CTR_CHECKBOXBOX, [](Window* pWindow) { return new CheckBoxBox(pWindow); }},
        {DUI_CTR_CHECKBOXHBOX, [](Window* pWindow) { return new CheckBoxHBox(pWindow); }},
        {DUI_CTR_CHECKBOXVBOX, [](Window* pWindow) { return new CheckBoxVBox(pWindow); }},
        {DUI_CTR_TREEVIEW, [](Window* pWindow) { return new TreeView(pWindow); }},
        {DUI_CTR_DIRECTORY_TREE, [](Window* pWindow) { return new DirectoryTree(pWindow); }},
        {DUI_CTR_TREENODE, [](Window* pWindow) { return new TreeNode(pWindow); }},
        {DUI_CTR_COMBO, [](Window* pWindow) { return new Combo(pWindow); }},
        {DUI_CTR_COMBO_BUTTON, [](Window* pWindow) { return new ComboButton(pWindow); }},
        {DUI_CTR_FILTER_COMBO, [](Window* pWindow) { return new FilterCombo(pWindow); }},
        {DUI_CTR_CHECK_COMBO, [](Window* pWindow) { return new CheckCombo(pWindow); }},
        {DUI_CTR_SLIDER, [](Window* pWindow) { return new Slider(pWindow); }},
        {DUI_CTR_PROGRESS, [](Window* pWindow) { return new Progress(pWindow); }},
        {DUI_CTR_CIRCLEPROGRESS, [](Window* pWindow) { return new CircleProgress(pWindow); }},
        {DUI_CTR_RICHTEXT, [](Window* pWindow) { return new RichText(pWindow); }},
        {DUI_CTR_RICHTEXT_BOX, [](Window* pWindow) { return new RichTextBox(pWindow); }},
        {DUI_CTR_RICHTEXT_HBOX, [](Window* pWindow) { return new RichTextHBox(pWindow); }},
        {DUI_CTR_RICHTEXT_VBOX, [](Window* pWindow) { return new RichTextVBox(pWindow); }},
        {DUI_CTR_RICHEDIT, [](Window* pWindow) { return new RichEdit(pWindow); }},
        {DUI_CTR_RICHEDIT2, [](Window* pWindow) { return new RichEdit2(pWindow); }},
        {DUI_CTR_DATETIME, [](Window* pWindow) { return new DateTime(pWindow); }},
        {DUI_CTR_COLOR_CONTROL, [](Window* pWindow) { return new ColorControl(pWindow); }},
        {DUI_CTR_COLOR_SLIDER, [](Window* pWindow) { return new ColorSlider(pWindow); }},
        {DUI_CTR_COLOR_PICKER_REGULAR, [](Window* pWindow) { return new ColorPickerRegular(pWindow); }},
        {DUI_CTR_COLOR_PICKER_STANDARD, [](Window* pWindow) { return new ColorPickerStandard(pWindow); }},
        {DUI_CTR_COLOR_PICKER_STANDARD_GRAY, [](Window* pWindow) { return new ColorPickerStandardGray(pWindow); }},
        {DUI_CTR_COLOR_PICKER_CUSTOM, [](Window* pWindow) { return new ColorPickerCustom(pWindow); }},
        {DUI_CTR_LINE, [](Window* pWindow) { return new Line(pWindow); }},
        {DUI_CTR_IPADDRESS, [](Window* pWindow) { return new IPAddress(pWindow); }},
        {DUI_CTR_HOTKEY, [](Window* pWindow) { return new HotKey(pWindow); }},
        {DUI_CTR_HYPER_LINK, [](Window* pWindow) { return new HyperLink(pWindow); }},
        {DUI_CTR_TAB_CTRL, [](Window* pWindow) { return new TabCtrl(pWindow); }},
        {DUI_CTR_TAB_CTRL_ITEM, [](Window* pWindow) { return new TabCtrlItem(pWindow); }},
        {DUI_CTR_ICON_CONTROL, [](Window* pWindow) { return new IconControl(pWindow); }},
        {DUI_CTR_BITMAP_CONTROL, [](Window* pWindow) { return new BitmapControl(pWindow); }},
        {DUI_CTR_ADDRESS_BAR, [](Window* pWindow) { return new AddressBar(pWindow); }},
        {DUI_CTR_MENU_BAR, [](Window* pWindow) { return new MenuBar(pWindow); }},
        {DUI_CTR_CHILD_WINDOW, [](Window* pWindow) { return new ChildWindow(pWindow); }},

        {DUI_CTR_SPLIT, [](Window* pWindow) { return new Split(pWindow); }},
        {DUI_CTR_SPLITBOX, [](Window* pWindow) { return new SplitBox(pWindow); }},
        {DUI_CTR_GROUP_BOX, [](Window* pWindow) { return new GroupBox(pWindow); }},
        {DUI_CTR_GROUP_HBOX, [](Window* pWindow) { return new GroupHBox(pWindow); }},
        {DUI_CTR_GROUP_VBOX, [](Window* pWindow) { return new GroupVBox(pWindow); }},

        {DUI_CTR_BOX_DRAGABLE, [](Window* pWindow) { return new BoxDragable(pWindow); }},
        {DUI_CTR_HBOX_DRAGABLE, [](Window* pWindow) { return new HBoxDragable(pWindow); }},
        {DUI_CTR_VBOX_DRAGABLE, [](Window* pWindow) { return new VBoxDragable(pWindow); }},

        {DUI_CTR_BOX_MOVABLE, [](Window* pWindow) { return new BoxMovable(pWindow); }},
        {DUI_CTR_HBOX_MOVABLE, [](Window* pWindow) { return new HBoxMovable(pWindow); }},
        {DUI_CTR_VBOX_MOVABLE, [](Window* pWindow) { return new VBoxMovable(pWindow); }},

        {DUI_CTR_BOX_RESIZABLE, [](Window* pWindow) { return new BoxResizable(pWindow); }},
        {DUI_CTR_HBOX_RESIZABLE, [](Window* pWindow) { return new HBoxResizable(pWindow); }},
        {DUI_CTR_VBOX_RESIZABLE, [](Window* pWindow) { return new VBoxResizable(pWindow); }},
    };
    Control* pControl = nullptr;
    auto iter = createControlMap.find(strControlClass);
    if (iter != createControlMap.end()) {
        pControl = iter->second(pWindow);
    }
    return pControl;
}

bool WindowBuilder::ParseXmlData(const DString& xmlFileData, const FilePath& xmlFilePath)
{
    ASSERT(!xmlFileData.empty() && _T("xmlFileData is empty!"));
    if (xmlFileData.empty()) {
        return false;
    }
    bool isLoaded = false;
    //字符串以<开头认为是XML字符串，否则认为是XML文件
    //如果使用了 zip 压缩包，则从内存中读取
    if (xmlFileData.front() == _T('<')) {
#ifdef DUILIB_UNICODE
        pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf16;
#else
        pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf8;
#endif
        pugi::xml_parse_result result = m_xml->load_buffer(xmlFileData.c_str(),
                                                           xmlFileData.size() * sizeof(DString::value_type),
                                                           pugi::parse_default, encoding);
        isLoaded = result.status == pugi::status_ok;
    }
    if (!isLoaded) {
        ASSERT(!_T("WindowBuilder::ParseXmlData load xmlFileData failed!"));
        return false;
    }
    m_xmlFilePath = xmlFilePath;
    return true;
}

bool WindowBuilder::ParseXmlData(const std::vector<unsigned char>& xmlFileData, const FilePath& xmlFilePath)
{
    ASSERT(!xmlFileData.empty() && _T("xmlFileData is empty!"));
    if (xmlFileData.empty()) {
        return false;
    }
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_auto;
    pugi::xml_parse_result result = m_xml->load_buffer(xmlFileData.data(),
                                                       xmlFileData.size(),
                                                       pugi::parse_default, encoding);
    bool isLoaded = result.status == pugi::status_ok;
    if (!isLoaded) {
        ASSERT(!_T("WindowBuilder::ParseXmlData load xmlFileData failed!"));
        return false;
    }
    m_xmlFilePath = xmlFilePath;
    return true;
}

bool WindowBuilder::ParseXmlFile(const FilePath& xmlFilePath, const FilePath& windowResPath)
{
    ASSERT(!xmlFilePath.IsEmpty() && _T("xmlFilePath is empty!"));
    if (xmlFilePath.IsEmpty()) {
        return false;
    }

    bool isLoaded = false;
    FilePath xmlFileFullPath;
    std::vector<uint8_t> xmlFileData;
    const ThemeManager& themeMgr = GlobalManager::Instance().Theme();
    if (themeMgr.GetResFile(xmlFilePath, windowResPath, xmlFileFullPath, xmlFileData)) {
        if (!xmlFileData.empty()) {
            pugi::xml_parse_result result = m_xml->load_buffer(xmlFileData.data(), xmlFileData.size());
            if (result.status != pugi::status_ok) {
                ASSERT(!_T("WindowBuilder::ParseXmlFile load xml from zip data failed!"));
                return false;
            }
            isLoaded = true;
        }
        else if (!xmlFileFullPath.IsEmpty()) {
            pugi::xml_parse_result result = m_xml->load_file(xmlFileFullPath.NativePathA().c_str());
            if (result.status != pugi::status_ok) {
                ASSERT(!_T("WindowBuilder::ParseXmlFile load xml file failed!"));
                return false;
            }
            isLoaded = true;
        }
    }
    if (!isLoaded) {
        ASSERT(!_T("WindowBuilder::ParseXmlFile load xmlFilePath failed!"));
        return false;
    }
    m_xmlFilePath = xmlFilePath;
    return true;
}

std::string WindowBuilder::ReadXmlFileData(const FilePath& xmlFilePath, const FilePath& windowResPath) const
{
    ASSERT(!xmlFilePath.IsEmpty() && _T("xmlFilePath is empty!"));
    if (xmlFilePath.IsEmpty()) {
        return std::string();
    }

    std::string xmlFileString;
    FilePath xmlFileFullPath;
    std::vector<uint8_t> xmlFileData;
    const ThemeManager& themeMgr = GlobalManager::Instance().Theme();
    if (themeMgr.GetResFile(xmlFilePath, windowResPath, xmlFileFullPath, xmlFileData)) {
        if (xmlFileData.empty() && !xmlFileFullPath.IsEmpty()) {
            FileUtil::ReadFileData(xmlFileFullPath, xmlFileData);
            
        }
        if (!xmlFileData.empty()) {
            xmlFileString = std::string((const char*)xmlFileData.data(), xmlFileData.size());
        }
    }
    return xmlFileString;
}

Control* WindowBuilder::CreateControls(Window* pWindow, CreateControlCallback pCallback, Box* pParent, Box* pUserDefinedBox)
{
    //校验窗口：必须存在，否则DPI自适应功能等功能会失效，导致界面布局不正确
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return nullptr;
    }
    if ((pParent != nullptr) && (pParent->GetWindow() == nullptr)) {
        pParent->SetWindow(pWindow);
    }
    if ((pUserDefinedBox != nullptr) && (pUserDefinedBox->GetWindow() == nullptr)) {
        pUserDefinedBox->SetWindow(pWindow);
    }

    m_createControlCallback = pCallback;
    pugi::xml_node root = m_xml->root().first_child();
    ASSERT(!root.empty());
    if (root.empty()) {
        return nullptr;
    }

    if( pWindow != nullptr) {
        DString strClass;
        DString strName;
        DString strValue;
        strClass = root.name();
        if( strClass == _T("Window") ) {
            if (!pWindow->IsWindowAttributesApplied()) {
                //窗口的属性，只设置一次，避免XML中的包含的XML文件（Include标签）再次设置窗口属性，导致混乱
                ParseWindowAttributes(pWindow, root);
                pWindow->SetWindowAttributesApplied(true);
            }            
            ParseWindowShareAttributes(pWindow, root);
        }
        else if( strClass == _T("Global") ) {
            ParseGlobalAttributes(root);
        }
    }

    for (pugi::xml_node node : root.children()) {
        DString strClass = node.name();
        if (IsIgnoreNodeName(strClass)) {
            //忽略这几个属性
        }
        else {
            if (pUserDefinedBox == nullptr) {
                return ParseXmlNodeChildren(root, pParent, pWindow);
            }
            else {
                ParseXmlNodeChildren(node, pUserDefinedBox, pWindow);
                int i = 0;
                for (pugi::xml_attribute attr : node.attributes()) {
                    if (StringUtil::StringCompare(attr.name(), _T("class")) == 0) {
                        //class必须是第一个属性
                        ASSERT_UNUSED_VARIABLE(i == 0);
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

bool WindowBuilder::ParseWindowCreateAttributes(Window* pWindow, WindowCreateAttributes& createAttributes)
{
    pugi::xml_node root = m_xml->root().first_child();
    ASSERT(!root.empty());
    if (root.empty()) {
        return false;
    }
    DString strClass = root.name();
    ASSERT(strClass == _T("Window"));
    if (strClass != _T("Window")) {
        return false;
    }

    UiSize szMinSize;
    UiSize szMaxSize;
    bool bScaledCX = false;
    bool bScaledCY = false;
    bool bPercentCX = false;
    bool bPercentCY = false;

    //窗口配置的size是否包含阴影
    bool bSizeContainShadow = false;

    //XML文件中是否定义了分层窗口属性
    bool bIsLayeredWindowDefined = false;

    //阴影相关参数
    ShadowType nShadowType = ShadowType::kShadowDefault;
    UiPadding rcShadowCorner;

    RenderBackendType backendType = RenderBackendType::kRaster_BackendType;
    DString strName;
    DString strValue;
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if (strName == _T("render_backend_type")) {            
            if (StringUtil::IsEqualNoCase(strValue, _T("GL")) || StringUtil::IsEqualNoCase(strValue, _T("GPU"))) {
                backendType = RenderBackendType::kNativeGL_BackendType;
            }
            else if (StringUtil::IsEqualNoCase(strValue, _T("CPU"))) {
                backendType = RenderBackendType::kRaster_BackendType;
            }
        }
        else if (strName == _T("use_system_caption")) {
            createAttributes.m_bUseSystemCaption = (StringUtil::IsValueTrue(strValue));
            createAttributes.m_bUseSystemCaptionDefined = true;
        }
        else if ((strName == _T("size_box")) || (strName == _T("sizebox"))) {
            AttributeUtil::ParseRectValue(strValue.c_str(), createAttributes.m_rcSizeBox);
            createAttributes.m_bSizeBoxDefined = true;
        }
        else if (strName == _T("caption")) {
            AttributeUtil::ParseRectValue(strValue.c_str(), createAttributes.m_rcCaption);
            createAttributes.m_bCaptionDefined = true;
        }
        else if ((strName == _T("shadow_attached")) || (strName == _T("shadowattached"))) {
            createAttributes.m_bShadowAttached = (StringUtil::IsValueTrue(strValue));
            createAttributes.m_bShadowAttachedDefined = true;
        }
        else if (strName == _T("shadow_type")) {
            //设置阴影类型
            Shadow::GetShadowType(strValue, nShadowType);
        }
        else if ((strName == _T("shadow_corner")) || (strName == _T("shadowcorner"))) {
            //设置窗口阴影的九宫格属性            
            AttributeUtil::ParsePaddingValue(strValue.c_str(), rcShadowCorner);
        }
        else if ((strName == _T("layered_window")) || (strName == _T("layeredwindow"))) {
            createAttributes.m_bIsLayeredWindow = (StringUtil::IsValueTrue(strValue));
            createAttributes.m_bIsLayeredWindowDefined = true;
            bIsLayeredWindowDefined = true;
        }
        else if (strName == _T("alpha")) {
            //设置窗口的透明度（0 - 255），仅当使用层窗口时有效，在在UpdateLayeredWindow函数中作为参数使用
            int32_t nAlpha = StringUtil::StringToInt32(strValue);
            ASSERT(nAlpha >= 0 && nAlpha <= 255);
            if ((nAlpha >= 0) && (nAlpha < 255)) {
                createAttributes.m_nLayeredWindowAlpha = (uint8_t)nAlpha;
                createAttributes.m_bLayeredWindowAlphaDefined = true;
            }
        }
        else if (strName == _T("opacity")) {
            //设置窗口的不透明度（0 - 255），该值在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)
            const int32_t nAlpha = StringUtil::StringToInt32(strValue);
            ASSERT(nAlpha >= 0 && nAlpha <= 255);
            if ((nAlpha >= 0) && (nAlpha < 255)) {
                createAttributes.m_nLayeredWindowOpacity = (uint8_t)nAlpha;
                createAttributes.m_bLayeredWindowOpacityDefined = true;
            }
        }
        else if (strName == _T("size")) {
            AttributeUtil::ParseWindowSize(nullptr, strValue.c_str(), createAttributes.m_szInitSize, &bScaledCX, &bScaledCY, &bPercentCX, &bPercentCY);
            createAttributes.m_bInitSizeDefined = true;
        }
        else if (strName == _T("size_contain_shadow")) {
            //窗口配置的size是否包含阴影
            bSizeContainShadow = (StringUtil::IsValueTrue(strValue));
        }
        else if ((strName == _T("min_size")) || (strName == _T("mininfo"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), szMinSize);
        }
        else if ((strName == _T("max_size")) || (strName == _T("maxinfo"))) {
            AttributeUtil::ParseSizeValue(strValue.c_str(), szMaxSize);
        }
        else if (strName == _T("sdl_render_name")) {
            //期望的SDL Render的名称
            createAttributes.m_sdlRenderName = strValue;
        }
    }

    //设置分层窗口的默认值
    const bool bUseSystemCaption = createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption;
    const bool bShadowAttached = !createAttributes.m_bShadowAttachedDefined || (createAttributes.m_bShadowAttachedDefined && createAttributes.m_bShadowAttached);
    if (!createAttributes.m_bIsLayeredWindowDefined &&
        !bUseSystemCaption &&
        createAttributes.m_bLayeredWindowOpacityDefined) {
        //使用Opacity属性，默认需要开启分层窗口
        createAttributes.m_bIsLayeredWindowDefined = true;
        createAttributes.m_bIsLayeredWindow = true;
    }
    else if (!createAttributes.m_bIsLayeredWindowDefined && bUseSystemCaption) {
        //使用系统标题栏属性，默认不开启分层窗口
        createAttributes.m_bIsLayeredWindowDefined = true;
        createAttributes.m_bIsLayeredWindow = false;
    }
    if (!createAttributes.m_bIsLayeredWindowDefined && bShadowAttached && !bUseSystemCaption) {
        //阴影启用
        ShadowType shadowType = nShadowType;
        if (shadowType == ShadowType::kShadowDefault) {
            shadowType = Shadow::GetDefaultShadowType(pWindow);
        }
        if (Shadow::IsShadowTypeNeedLayeredWindow(shadowType)) {
            //使用自绘阴影，默认需要开启分层窗口
            createAttributes.m_bIsLayeredWindowDefined = true;
            createAttributes.m_bIsLayeredWindow = true;
        }
    }

    //校验
#ifdef _DEBUG
    //使用系统标题栏时，不应该开启分层窗口属性
    if (bUseSystemCaption) {
        ASSERT(!createAttributes.m_bIsLayeredWindow);
    }    
    //使用Opacity属性时，应该开启分层窗口属性
    if (createAttributes.m_bLayeredWindowOpacityDefined) {
        ASSERT(createAttributes.m_bIsLayeredWindow);
    }
#endif

#if defined (DUILIB_BUILD_FOR_SDL)
    //默认开启支持透明度(SDL在Windows系统中，未使用分层窗口属性)
    if (!bIsLayeredWindowDefined) {
        createAttributes.m_bIsLayeredWindowDefined = true;
        createAttributes.m_bIsLayeredWindow = true;
    }
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //使用OpenGL时，不能使用层窗口
        if (!createAttributes.m_bLayeredWindowOpacityDefined &&
            createAttributes.m_bIsLayeredWindowDefined && createAttributes.m_bIsLayeredWindow) {
            ASSERT(!createAttributes.m_bIsLayeredWindow);
            createAttributes.m_bIsLayeredWindow = false;
        }
    }
#else
    UNUSED_VARIABLE(backendType);
#endif

    //评估阴影的九宫格属性, 用于确定最终的窗口大小
    if (createAttributes.m_bShadowAttachedDefined && !createAttributes.m_bShadowAttached) {
        //阴影被禁用
        rcShadowCorner.Clear();
    }
    else if (rcShadowCorner.IsEmpty()) {
        UiSize szBorderRound;
        DString shadowImage;
        if (pWindow != nullptr) {
            bool bLayeredWindow = createAttributes.m_bIsLayeredWindowDefined && createAttributes.m_bIsLayeredWindow;
            bool bOldLayeredWindow = pWindow->IsLayeredWindow();
            pWindow->SetLayeredWindow(bLayeredWindow, false);
            Shadow::GetShadowParam(pWindow, nShadowType, szBorderRound, rcShadowCorner, shadowImage);
            pWindow->SetLayeredWindow(bOldLayeredWindow, false);
        }
        else {
            Shadow::GetShadowParam(pWindow, nShadowType, szBorderRound, rcShadowCorner, shadowImage);
        }
    }
    createAttributes.m_rcShadowCorner = rcShadowCorner;

    if (createAttributes.m_bInitSizeDefined) {
        int32_t cx = createAttributes.m_szInitSize.cx;
        int32_t cy = createAttributes.m_szInitSize.cy;
        UiSize minSize = szMinSize;
        UiSize maxSize = szMaxSize;
        if (bScaledCX) {
            GlobalManager::Instance().Dpi().ScaleWindowSize(minSize.cx);
            GlobalManager::Instance().Dpi().ScaleWindowSize(maxSize.cx);
        }
        if (bScaledCY) {
            GlobalManager::Instance().Dpi().ScaleWindowSize(minSize.cy);
            GlobalManager::Instance().Dpi().ScaleWindowSize(maxSize.cy);
        }
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
        if (!bScaledCX) {
            GlobalManager::Instance().Dpi().ScaleWindowSize(cx);
        }
        if (!bScaledCY) {
            GlobalManager::Instance().Dpi().ScaleWindowSize(cy);
        }
        if (!bSizeContainShadow) {
            if (!bPercentCX) {
                GlobalManager::Instance().Dpi().ScaleWindowSize(rcShadowCorner.left);
                GlobalManager::Instance().Dpi().ScaleWindowSize(rcShadowCorner.right);
                cx += rcShadowCorner.left + rcShadowCorner.right;
            }
            if (!bPercentCY) {
                GlobalManager::Instance().Dpi().ScaleWindowSize(rcShadowCorner.top);
                GlobalManager::Instance().Dpi().ScaleWindowSize(rcShadowCorner.bottom);
                cy += rcShadowCorner.top + rcShadowCorner.bottom;
            }
        }
        AttributeUtil::ValidateWindowSize(nullptr, cx, cy);
        createAttributes.m_szInitSize.cx = cx;
        createAttributes.m_szInitSize.cy = cy;
    }

    return true;
}

void WindowBuilder::ParseWindowAttributes(Window* pWindow, const pugi::xml_node& root) const
{
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return;
    }

    std::set<DString> knownNames;//支持的属性名称
    DString strName;
    DString strValue;

    bool bInitRenderBackendType = false;
    //首先设置"render_backend_type"属性
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if (strName == _T("render_backend_type")) {
            knownNames.insert(strName);
            RenderBackendType backendType = RenderBackendType::kRaster_BackendType;
            if (StringUtil::IsEqualNoCase(strValue, _T("GL")) || StringUtil::IsEqualNoCase(strValue, _T("GPU"))) {
                backendType = RenderBackendType::kNativeGL_BackendType;
            }
            else if (StringUtil::IsEqualNoCase(strValue, _T("CPU"))) {
                backendType = RenderBackendType::kRaster_BackendType;
            }
            else {
                ASSERT(0);
            }
            pWindow->SetRenderBackendType(backendType);
            bInitRenderBackendType = true;
            break;
        }
    }
    if (!bInitRenderBackendType) {
        //首先初始化Render后台绘制方式, 此调用会创建Render
        pWindow->SetRenderBackendType(RenderBackendType::kRaster_BackendType);
    }
     
    //首先处理min_size/max_size/use_system_caption，因为其他属性有用到这些个属性的
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if ((strName == _T("min_size")) || (strName == _T("mininfo"))) {
            knownNames.insert(strName);
            UiSize size;
            AttributeUtil::ParseSizeValue(strValue.c_str(), size);
            pWindow->SetWindowMinimumSize(size, true);
        }
        else if ((strName == _T("max_size")) || (strName == _T("maxinfo"))) {
            knownNames.insert(strName);
            UiSize size;
            AttributeUtil::ParseSizeValue(strValue.c_str(), size);
            pWindow->SetWindowMaximumSize(size, true);
        }
        else if (strName == _T("use_system_caption")) {
            knownNames.insert(strName);
            pWindow->SetUseSystemCaption(StringUtil::IsValueTrue(strValue));
        }
    }
    //窗口配置的size是否包含阴影
    bool bSizeContainShadow = false;

    //窗口阴影是否开启
    bool bShadowAttached = false;
    bool bHasShadowAttached = false;
    ShadowType nShadowType = ShadowType::kShadowDefault;

    //注：如果use_system_caption为true，则层窗口关闭（因为这两个属性互斥的）
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if ((strName == _T("size_box")) || (strName == _T("sizebox"))) {
            knownNames.insert(strName);
            UiRect rcSizeBox;
            AttributeUtil::ParseRectValue(strValue.c_str(), rcSizeBox, false);
            pWindow->SetSizeBox(rcSizeBox, true);
        }
        else if (strName == _T("caption")) {
            knownNames.insert(strName);
            UiRect rcCaption;
            AttributeUtil::ParseRectValue(strValue.c_str(), rcCaption);
            pWindow->SetCaptionRect(rcCaption, true);
        }
        else if (strName == _T("snap_layout_menu")) {
            knownNames.insert(strName);
            pWindow->SetEnableSnapLayoutMenu(StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("sys_menu")) {
            knownNames.insert(strName);
            pWindow->SetEnableSysMenu(StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("sys_menu_rect")) {
            knownNames.insert(strName);
            UiRect rcSysMenuRect;
            AttributeUtil::ParseRectValue(strValue.c_str(), rcSysMenuRect);
            pWindow->SetSysMenuRect(rcSysMenuRect, true);
        }
        else if (strName == _T("icon")) {
            knownNames.insert(strName);
            if (!strValue.empty()) {
                //设置窗口图标
                pWindow->SetWindowIcon(strValue);
            }
        }
        else if (strName == _T("text")) {
            knownNames.insert(strName);
            pWindow->SetText(strValue);
        }
        else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
            knownNames.insert(strName);
            pWindow->SetTextId(strValue);
        }
        else if (strName == _T("round_corner") || strName == _T("roundcorner")) {
            knownNames.insert(strName);
            UiSize size;
            AttributeUtil::ParseSizeValue(strValue.c_str(), size);
            pWindow->SetRoundCorner(size.cx, size.cy, true);
        }
        else if (strName == _T("size_contain_shadow")) {
            knownNames.insert(strName);
            //窗口配置的size是否包含阴影
            bSizeContainShadow = (StringUtil::IsValueTrue(strValue));
        }
        else if ((strName == _T("shadow_attached")) || (strName == _T("shadowattached"))) {
            knownNames.insert(strName);
            //设置是否支持窗口阴影（阴影实现有两种：分层窗口和普通窗口）
            bShadowAttached = (StringUtil::IsValueTrue(strValue));
            bHasShadowAttached = true;            
        }
        else if (strName == _T("shadow_type")) {
            knownNames.insert(strName);
            //设置阴影类型
            if (Shadow::GetShadowType(strValue, nShadowType)) {
                pWindow->SetShadowType(nShadowType);
            }
        }
        else if ((strName == _T("shadow_image")) || (strName == _T("shadowimage"))) {
            knownNames.insert(strName);
            //设置阴影图片
            pWindow->SetShadowImage(strValue);
        }
        else if ((strName == _T("shadow_corner")) || (strName == _T("shadowcorner"))) {
            knownNames.insert(strName);
            //设置窗口阴影的九宫格属性
            UiPadding padding;
            AttributeUtil::ParsePaddingValue(strValue.c_str(), padding);
            pWindow->SetShadowCorner(padding);
        }
        else if (strName == _T("shadow_border_round")) {
            knownNames.insert(strName);
            //设置窗口阴影的圆角大小
            UiSize szBorderRound;
            AttributeUtil::ParseSizeValue(strValue.c_str(), szBorderRound);
            pWindow->SetShadowBorderRound(szBorderRound);
        }
        else if (strName == _T("shadow_border_size")) {
            knownNames.insert(strName);
            //设置窗口阴影的边框大小
            pWindow->SetShadowBorderSize(StringUtil::StringToInt32(strValue));
        }
        else if (strName == _T("shadow_border_color")) {
            knownNames.insert(strName);
            //设置窗口阴影的边框颜色
            pWindow->SetShadowBorderColor(strValue);
        }
        else if (strName == _T("shadow_snap")) {
            knownNames.insert(strName);
            //设置阴影是否支持窗口贴边操作
            pWindow->SetEnableShadowSnap(StringUtil::IsValueTrue(strValue));
        }
        else if ((strName == _T("layered_window")) || (strName == _T("layeredwindow"))) {
            knownNames.insert(strName);
            //设置是否设置分层窗口属性（分层窗口还是普通窗口）
            pWindow->SetLayeredWindow(StringUtil::IsValueTrue(strValue), false);
        }
        else if (strName == _T("alpha")) {
            knownNames.insert(strName);
            //设置窗口的透明度（0 - 255），仅当使用层窗口时有效，在在UpdateLayeredWindow函数中作为参数使用
            int32_t nAlpha = StringUtil::StringToInt32(strValue);
            ASSERT(nAlpha >= 0 && nAlpha <= 255);
            if ((nAlpha >= 0) && (nAlpha <= 255)) {
                pWindow->SetLayeredWindowAlpha(nAlpha);
            }
        }
        else if (strName == _T("drag_drop")) {
            knownNames.insert(strName);
            pWindow->SetEnableDragDrop(StringUtil::IsValueTrue(strValue));
        }
    }

    if (bHasShadowAttached) {
        //后设置，避免受到"shadow_type"的影响
        if (!bShadowAttached) {
            pWindow->SetShadowAttached(bShadowAttached);
        }
        else if (!pWindow->IsUseSystemCaption()) {
            pWindow->SetShadowAttached(bShadowAttached);
        }
    }

    bool bScaledCX = false;
    bool bScaledCY = false;
    bool bPercentCX = false;
    bool bPercentCY = false;

    //最后设置窗口的初始化大小，因为初始化大小与是否阴影等相关
    bool bLayeredWindowOpacityDefined = false;
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if (strName == _T("size")) {
            knownNames.insert(strName);
            UiSize windowSize;
            AttributeUtil::ParseWindowSize(pWindow, strValue.c_str(), windowSize, &bScaledCX, &bScaledCY, &bPercentCX, &bPercentCY);
            int32_t cx = windowSize.cx;
            int32_t cy = windowSize.cy;
            UiSize minSize = pWindow->GetWindowMinimumSize();
            UiSize maxSize = pWindow->GetWindowMaximumSize();
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

            if (!bSizeContainShadow) {
                //XML配置中指定的窗口大小，如果设置的是固定值，则不包含阴影部分
                UiPadding rcShadowCorner = pWindow->GetShadowCorner();                
                if (!bPercentCX && pWindow->IsShadowAttached() && !pWindow->IsWindowMaximized()) {
                    pWindow->Dpi().ScaleWindowSize(rcShadowCorner.left);
                    pWindow->Dpi().ScaleWindowSize(rcShadowCorner.right);
                    cx += rcShadowCorner.left + rcShadowCorner.right;
                }
                if (!bPercentCY && pWindow->IsShadowAttached() && !pWindow->IsWindowMaximized()) {
                    pWindow->Dpi().ScaleWindowSize(rcShadowCorner.top);
                    pWindow->Dpi().ScaleWindowSize(rcShadowCorner.bottom);
                    cy += rcShadowCorner.top + rcShadowCorner.bottom;
                }
            }
            AttributeUtil::ValidateWindowSize(pWindow, cx, cy);
            pWindow->SetInitSize(cx, cy);
        }
        else if (strName == _T("opacity")) {
            knownNames.insert(strName);
            //设置窗口的不透明度（0 - 255），该值在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)
            const int32_t nAlpha = StringUtil::StringToInt32(strValue);
            ASSERT(nAlpha >= 0 && nAlpha <= 255);
            if ((nAlpha >= 0) && (nAlpha <= 255)) {
                pWindow->SetLayeredWindowOpacity(nAlpha);
                bLayeredWindowOpacityDefined = true;
            }
        }
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (pWindow->GetRenderBackendType() == RenderBackendType::kNativeGL_BackendType) {
        //使用OpenGL时，不能使用层窗口
        if (!bLayeredWindowOpacityDefined || (pWindow->GetLayeredWindowOpacity() == 255)) {
            pWindow->SetLayeredWindow(false, false);
        }
        if (pWindow->IsShadowAttached() && !pWindow->IsUseSystemCaption()) {
            //如果使用了阴影，则自动切换为使用系统标题栏，避免出现显示异常
            pWindow->SetUseSystemCaption(true);
        }
    }
#else
    UNUSED_VARIABLE(bLayeredWindowOpacityDefined);
#endif

#ifdef _DEBUG
    //检查是否有不支持的属性，然后预警，减少配置错误问题
    std::vector<DString> unknownNames;
    for (pugi::xml_attribute attr : root.attributes()) {
        strName = attr.name();
        if (knownNames.find(strName) == knownNames.end()) {
            unknownNames.push_back(strName);
        }
    }
    ASSERT_UNUSED_VARIABLE(unknownNames.empty() && "Found unknown window attributes in xml!");
#endif
}

void WindowBuilder::ParseWindowShareAttributes(Window* pWindow, const pugi::xml_node& root)
{
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return;
    }

    DString strName;
    DString strValue;
    DString strClass;

    //解析该窗口下的共享资源
    for (pugi::xml_node node : root.children()) {
        strClass = node.name();
        if (strClass == _T("Class")) {
            DString strClassName;
            DString strAttribute;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == _T("name")) {
                    strClassName = strValue;
                }
                else {
                    GlobalManager::Instance().ExpandVarStrings(strValue);
                    strAttribute.append(StringUtil::Printf(_T(" %s=\"%s\""), strName.c_str(), strValue.c_str()));
                }
            }
            if (!strClassName.empty()) {
                //窗口中的Class不能与全局的重名, 否则该Class是无效的
                ASSERT(GlobalManager::Instance().GetClassAttributes(strClassName).empty()); 
                StringUtil::TrimLeft(strAttribute);
                pWindow->AddClass(strClassName, strAttribute);
                m_windowClassList.push_back(strClassName);
            }
        }
        else if ((strClass == _T("ThemeColor")) || (strClass == _T("TextColor"))) {
            DString strColorName;
            DString strColor;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == _T("name")) {
                    strColorName = strValue;
                }
                else if (strName == _T("value")) {
                    GlobalManager::Instance().ExpandVarStrings(strValue);
                    strColor = strValue;
                }
            }
            if (!strColorName.empty()) {
                pWindow->AddThemeColor(strColorName, strColor);
                m_windowThemeColorList.push_back(strColorName);
            }
        }
        else if (strClass == _T("Font")) {
            //Window节点下，允许定义字体
            ParseFontXmlNode(node);
        }
    }
}

/** 获取操作系统平台的字体属性名称
*/
static DString GetPlatformFontPropName()
{
#ifdef DUILIB_BUILD_FOR_WIN
    return _T("windows");
#elif defined DUILIB_BUILD_FOR_MACOS
    return _T("macos");
#elif defined DUILIB_BUILD_FOR_LINUX
    return _T("linux");
#elif defined DUILIB_BUILD_FOR_FREEBSD
    return _T("freebsd");
#else
    return _T("value");
#endif
}

void WindowBuilder::ParseGlobalAttributes(const pugi::xml_node& root)
{
    const DString fontPropName = GetPlatformFontPropName();
    DString strClass;
    DString strName;
    DString strValue;
    for (pugi::xml_node node : root.children()) {
        strClass = node.name();
        if (strClass == _T("DefaultFontFamilyNames")) {
            DString defaultFontFamilyNames;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == fontPropName) {
                    // 显式接收 ExpandVarStrings 的返回值，表达"使用展开后的字符串"的意图
                    defaultFontFamilyNames = GlobalManager::Instance().ExpandVarStrings(strValue);
                    break;
                }
            }
            if (!defaultFontFamilyNames.empty()) {
                GlobalManager::Instance().Font().SetDefaultFontFamilyNames(defaultFontFamilyNames);
            }
        }
        else if (strClass == _T("FallbackFontFamilyNames")) {
            DString fallbackFontFamilyNames;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == fontPropName) {
                    fallbackFontFamilyNames = GlobalManager::Instance().ExpandVarStrings(strValue);
                    break;
                }
            }
            if (!fallbackFontFamilyNames.empty()) {
                GlobalManager::Instance().Font().SetFallbackFontFamilyNames(fallbackFontFamilyNames);
            }
        }
        else if (strClass == _T("FontFile")) {
            //字体文件
            DString strFontFile;
            DString strFontDesc;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == _T("file")) {
                    strFontFile = strValue;
                }
                else if (strName == _T("desc")) {
                    strFontDesc = strValue;
                }
            }
            if (!strFontFile.empty()) {
                strFontFile = GlobalManager::Instance().ExpandVarStrings(strFontFile);
                GlobalManager::Instance().Font().AddFontFile(strFontFile, strFontDesc);
            }
        }
        else if (strClass == _T("Font")) {
            ParseFontXmlNode(node);
        }
        else if (strClass == _T("Class")) {
            DString strClassName;
            DString strAttribute;
            for (pugi::xml_attribute attr : node.attributes()) {
                strName = attr.name();
                strValue = attr.value();
                if (strName == _T("name")) {
                    strClassName = strValue;
                }
                else {
                    GlobalManager::Instance().ExpandVarStrings(strValue);
                    strAttribute.append(StringUtil::Printf(_T(" %s=\"%s\""), strName.c_str(), strValue.c_str()));
                }
            }
            if (!strClassName.empty()) {
                StringUtil::TrimLeft(strAttribute);
                GlobalManager::Instance().AddClass(strClassName, strAttribute);
            }
        }
        else if ((strClass == _T("ThemeColor")) || (strClass == _T("TextColor"))) {
            DString colorName = node.attribute(_T("name")).as_string();
            DString colorValue = node.attribute(_T("value")).as_string();
            if (!colorName.empty() && !colorValue.empty()) {
                GlobalManager::Instance().ExpandVarStrings(colorValue);
                ColorManager& colorManager = GlobalManager::Instance().Color();
                colorManager.AddColor(colorName, colorValue);
                if ((colorName == _T("text_default")) || (colorName == _T("default_font_color"))) {
                    colorManager.SetDefaultTextColor(colorName);
                }
                else if ((colorName == _T("text_disabled")) || (colorName == _T("disabled_font_color"))) {
                    colorManager.SetDefaultDisabledTextColor(colorName);
                }
            }
        }
        else if (strClass == _T("Alias")) {
            DString aliasName = node.attribute(_T("name")).as_string();
            DString aliasValue = node.attribute(_T("value")).as_string();
            GlobalManager::Instance().AddAlias(aliasName, aliasValue);
        }
        else if (strClass == _T("Var")) {
            DString defineName = node.attribute(_T("name")).as_string();
            DString defineValue = node.attribute(_T("value")).as_string();
            GlobalManager::Instance().AddVar(defineName, defineValue);
        }
        else if (strClass == _T("Theme")) {
            //跳过(主题名称，在其他地方解析)
        }
    }
}

bool WindowBuilder::IsIgnoreNodeName(const DString& nodeName) const
{
    //这些是公共资源的标签名字，有些是旧名（已废弃）
    if ((nodeName == _T("DefaultFontFamilyNames")) ||
        (nodeName == _T("FallbackFontFamilyNames")) ||
        (nodeName == _T("Font")) ||
        (nodeName == _T("FontFile")) ||
        (nodeName == _T("FontResource")) ||
        (nodeName == _T("Class")) ||
        (nodeName == _T("TextColor")) ||
        (nodeName == _T("ThemeColor")) ||
        (nodeName == _T("ThemeMeta")) ||
        (nodeName == _T("Theme")) ||
        (nodeName == _T("Alias")) ||
        (nodeName == _T("Var"))) {
        return true;
    }
    return false;
}

bool WindowBuilder::ParseThemeInfo(DString& themeName, DString& themeType, DString& themeStyle) const
{
    pugi::xml_node root = m_xml->root().first_child();
    if (root.empty()) {
        return false;
    }
    DString strClass = root.name();
    if (strClass != _T("Global")) {
        return false;
    }
    for (pugi::xml_node node : root.children()) {
        strClass = node.name();
        if (strClass == _T("Theme")) {
            themeName = node.attribute(_T("name")).as_string();
            themeType = node.attribute(_T("type")).as_string();
            themeStyle = node.attribute(_T("style")).as_string();
            return true;
        }
    }
    return false;
}

bool WindowBuilder::ParseThemeColor(ColorManager& colorManager) const
{
    pugi::xml_node root = m_xml->root().first_child();
    if (root.empty()) {
        return false;
    }
    DString strClass = root.name();
    if (strClass != _T("Global")) {
        return false;
    }
    bool bRet = false;
    for (pugi::xml_node node : root.children()) {
        strClass = node.name();
        if (strClass == _T("ThemeColor")) {
            DString colorName = node.attribute(_T("name")).as_string();
            DString colorValue = node.attribute(_T("value")).as_string();
            if (!colorName.empty() && !colorValue.empty()) {
                GlobalManager::Instance().ExpandVarStrings(colorValue);
                colorManager.AddColor(colorName, colorValue);
                if ((colorName == _T("text_default")) || (colorName == _T("default_font_color"))) {
                    colorManager.SetDefaultTextColor(colorName);
                }
                else if ((colorName == _T("text_disabled")) || (colorName == _T("disabled_font_color"))) {
                    colorManager.SetDefaultDisabledTextColor(colorName);
                }
                bRet = true;
            }            
        }
    }
    return bRet;
}

void WindowBuilder::ParseFontXmlNode(const pugi::xml_node& xmlNode)
{
    DString strName;
    DString strValue;

    DString strFontId;
    DString strFontName;
    int size = 12;
    bool bold = false;
    bool underline = false;
    bool strikeout = false;
    bool italic = false;
    bool isDefault = false;
    for (pugi::xml_attribute attr : xmlNode.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        if (strName == _T("id")) {
            strFontId = strValue;
        }
        else if (strName == _T("name")) {
            strFontName = strValue;
        }
        else if (strName == _T("size")) {
            size = StringUtil::StringToInt32(strValue);
        }
        else if (strName == _T("bold")) {
            bold = (StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("underline")) {
            underline = (StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("strikeout")) {
            strikeout = (StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("italic")) {
            italic = (StringUtil::IsValueTrue(strValue));
        }
        else if (strName == _T("default")) {
            isDefault = (StringUtil::IsValueTrue(strValue));
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
        if (GlobalManager::Instance().Font().AddFont(strFontId, fontInfo, isDefault)) {
            m_globalFontIdList.push_back(strFontId);
        }
    }
}

Control* WindowBuilder::ParseXmlNodeChildren(const pugi::xml_node& xmlNode, Control* pParent, Window* pWindow)
{
    if (xmlNode.empty()) {
        return nullptr;
    }
    Control* pReturn = nullptr;
    for (pugi::xml_node node : xmlNode.children()) {
        DString strClass = node.name();
        if(IsIgnoreNodeName(strClass)) {
            //需要忽略的节点名称（一些全局属性等）
            continue;
        }
        else if (strClass == _T("Include")) {
            //Include节点
            Control* pNewControl = ParseIncludeXmlNode(node, pParent, pWindow);
            if ((pNewControl != nullptr) && (pReturn == nullptr)) {
                pReturn = pNewControl;
            }
            continue;
        }
        else if (strClass == DUI_CTR_MENU_BAR_ITEM) {
            //MenuBarItem节点
            ParseMenuBarItemXmlNode(node, pParent, pWindow);
            continue;
        }
        else if (strClass == _T("PropertyGridGroup")) {
            //PropertyGridGroup节点
            ParsePropertyGridGroupXmlNode(node, pParent, pWindow);
            continue;
        }
        else if (strClass == _T("CheckComboText")) {
            //CheckComboText节点
            ParseCheckComboTextXmlNode(node, pParent, pWindow);
            continue;
        }
        else if ((strClass == _T("ListCtrlHeaderItem")) || (strClass == _T("ListCtrlItem")) || (strClass == _T("ListCtrlSubItem"))) {
            //ListCtrlHeaderItem/ListCtrlItem/ListCtrlSubItem节点
            ParseListCtrlXmlNode(node, pParent, pWindow);
            continue;
        }
 
        //根据Class名称直接窗口标准控件
        Control* pControl = CreateControlByClass(strClass, pWindow);
        if (pControl == nullptr) {
            if ((strClass == _T("Event")) || 
                (strClass == _T("BubbledEvent"))) {
                //挂载XML事件
                bool bBubbled = (strClass == _T("BubbledEvent"));
                AttachXmlEvent(bBubbled, node, pParent);
                continue;
            }
        }
        if( pControl == nullptr) {
            //通过注册的控件函数来创建控件
            pControl = GlobalManager::Instance().CreateControl(strClass);
            if (pControl != nullptr) {
                pControl->SetWindow(pWindow);
            }
        }

        if( pControl == nullptr && m_createControlCallback ) {
            //通过外部回调函数创建控件
            pControl = m_createControlCallback(strClass);
            if (pControl != nullptr) {
                pControl->SetWindow(pWindow);
            }
        }

        if ((pControl == nullptr) && (strClass == DUI_CTR_MENU)) {
            //菜单容器
            pControl = new MenuListBox(pWindow);
        }

        if(pControl == nullptr) {
            DString nodeName = strClass;
            ASSERT(!"Found unknown node name, can't create control!");
            continue;
        }

        // TreeView相关必须先添加到容器，然后再解析子节点
        bool bAddedToParentBox = false;
        if (strClass == DUI_CTR_TREENODE) {
            TreeNode* pNode = dynamic_cast<TreeNode*>(pControl);
            ASSERT(pNode != nullptr);
            TreeView* pTreeView = dynamic_cast<TreeView*>(pParent);
            if (pTreeView != nullptr) {
                //一级子节点
                pTreeView->GetRootNode()->AddChildNode(pNode);
                bAddedToParentBox = true;
            }
            else {
                //多级子节点
                TreeNode* pTreeNode = dynamic_cast<TreeNode*>(pParent);
                if (pTreeNode != nullptr) {
                    pTreeNode->AddChildNode(pNode);
                    bAddedToParentBox = true;
                }
            }
            if (!bAddedToParentBox) {
                //尝试Combo控件
                Combo* pCombo = dynamic_cast<Combo*>(pParent);
                if (pCombo != nullptr) {
                    pCombo->GetTreeView()->GetRootNode()->AddChildNode(pNode);
                    bAddedToParentBox = true;
                }
            }
            ASSERT(bAddedToParentBox);
        }
        else if ((strClass == DUI_CTR_PROPERTY_GRID) || (strClass == DUI_CTR_LISTCTRL)) {
            //PropertyGrid/ListCtrl控件必须先添加到容器，然后再解析子节点
            if (pParent != nullptr) {
                Box* pContainer = dynamic_cast<Box*>(pParent);
                ASSERT(pContainer != nullptr);
                if (pContainer != nullptr) {
                    bAddedToParentBox = pContainer->AddItem(pControl);
                }
            }
        }
        pControl->SetWindow(pWindow);
        
        // Process attributes
        if(!node.attributes().empty()) {
            //读取节点的属性，设置控件的属性
            int32_t i = 0;
            for (pugi::xml_attribute attr : node.attributes()) {
                ASSERT_UNUSED_VARIABLE(i == 0 || StringUtil::StringCompare(attr.name(), _T("class")) != 0);    //class必须是第一个属性
                ++i;
                pControl->SetAttribute(attr.name(), attr.value());
            }
        }
        if (strClass == DUI_CTR_RICHTEXT) {
            //节点为：<RichText></RichText>，解析其子节点为RichText内容
            ParseRichTextXmlNode(node, pControl);
        }
        else if (!node.children().empty()) {
            //递归该节点的所有子节点，继续添加
            ParseXmlNodeChildren(node, pControl, pWindow);
        }
        //普通的控件或者容器，需要再最后添加到容器（不可以先添加到容器，否则部分控件的实现逻辑会存在问题，比如TabBox等带select功能的容器）
        if ((pParent != nullptr) && !bAddedToParentBox) {
            Box* pContainer = dynamic_cast<Box*>(pParent);
            ASSERT(pContainer != nullptr);
            if (pContainer != nullptr) {
                if (!pContainer->AddItem(pControl)) {
                    ASSERT(0);
                    delete pControl;
                    pControl = nullptr;
                    continue;
                }
                else if ((strClass == DUI_CTR_LISTBOX_ITEM) ||
                         (strClass == DUI_CTR_LISTBOX_ITEM_HBOX) ||
                         (strClass == DUI_CTR_LISTBOX_ITEM_VBOX)) {
                    //检查ListBoxItem
                    if (!node.attributes().empty() && StringUtil::IsValueTrue(node.attribute(_T("selected")).as_string())) {
                        //该列表项设置了选择selected属性
                        ListBox* pListBox = dynamic_cast<ListBox*>(pContainer);
                        if (pListBox != nullptr) {
                            size_t nItemIndex = pListBox->GetItemIndex(pControl);
                            if (nItemIndex != Box::InvalidIndex) {
                                pListBox->SelectItem(nItemIndex, false, false);
                            }
                        }
                    }
                 }
            }
        }
        if (pReturn == nullptr) {
            pReturn = pControl;
        }
    }
    return pReturn;
}

Control* WindowBuilder::ParseIncludeXmlNode(const pugi::xml_node& node, Control* pParent, Window* pWindow) const
{
    if (node.attributes().empty()) {
        return nullptr;
    }
    pugi::xml_attribute countAttr = node.attribute(_T("count"));
    int32_t nCount = countAttr.as_int();
    if (nCount <= 0) {
        //默认值设置为1，count这个属性参数为可选
        nCount = 1;
    }
    pugi::xml_attribute sourceAttr = node.attribute(_T("src"));
    DString sourceValue = sourceAttr.as_string();
    if (sourceValue.empty()) {
        sourceAttr = node.attribute(_T("source"));
        sourceValue = sourceAttr.as_string();
    }
    const FilePath windowResPath = (pWindow != nullptr) ? pWindow->GetResourcePath() : FilePath();
    FilePath sourceXmlFilePath(sourceValue);
    if (!sourceValue.empty()) {
        StringUtil::ReplaceAll(_T("/"), m_xmlFilePath.GetPathSeparatorStr(), sourceValue);
        StringUtil::ReplaceAll(_T("\\"), m_xmlFilePath.GetPathSeparatorStr(), sourceValue);
        if (!m_xmlFilePath.IsEmpty()) {
            //优先尝试在原XML文件相同目录加载
            DString xmlFilePath = m_xmlFilePath.ToString();
            size_t pos = xmlFilePath.find_last_of(_T("\\/"));
            if (pos != DString::npos) {
                FilePath srcFilePath(xmlFilePath.substr(0, pos));
                srcFilePath.JoinFilePath(FilePath(sourceValue));
                if (GlobalManager::Instance().Theme().IsResFileExists(srcFilePath, windowResPath)) {
                    sourceXmlFilePath = srcFilePath;
                }
            }
        }
    }
    ASSERT(!sourceXmlFilePath.IsEmpty());
    if (sourceXmlFilePath.IsEmpty()) {
        return nullptr;
    }
    Control* pReturn = nullptr;
    Box* pParentBox = ToBox(pParent);
    for (int32_t i = 0; i < nCount; i++) {
        WindowBuilder builder;
        if (builder.ParseXmlFile(sourceXmlFilePath, windowResPath)) {
            Control* pControl = builder.CreateControls(pWindow, m_createControlCallback, pParentBox, nullptr);
            if ((pParentBox != nullptr) && (pControl != nullptr)) {
                ASSERT(pParentBox->GetItemIndex(pControl) != Box::InvalidIndex);
            }
            if (pReturn == nullptr) {
                pReturn = pControl;
            }
        }
    }
    return pReturn;
}

void WindowBuilder::ParseMenuBarItemXmlNode(const pugi::xml_node& node, Control* pParent, Window* /*pWindow*/) const
{
    MenuBar* pMenuBar = dynamic_cast<MenuBar*>(pParent);
    ASSERT((pMenuBar != nullptr) && !node.attributes().empty());
    if ((pMenuBar == nullptr) || node.attributes().empty()) {
        return;
    }

    DString menuItemId = node.attribute(_T("id")).as_string();
    DString menuText = node.attribute(_T("text")).as_string();
    DString menuTextId = node.attribute(_T("text_id")).as_string();
    DString menuXmlPath = node.attribute(_T("xml_path")).as_string();
    ASSERT(!menuText.empty() || !menuTextId.empty());
    if (menuText.empty() && menuTextId.empty()) {
        return;
    }

    if (menuXmlPath.empty()) {
        pugi::xml_node menuNode = node.child(_T("Menu"));
        if (!menuNode.empty()) {
            struct xml_string_writer : pugi::xml_writer {
                DString result;
                virtual void write(const void* data, size_t size) override {
                    std::string utf8(static_cast<const char*>(data), size);
                    result.append(StringConvert::UTF8ToT(utf8));
                }
            };
            xml_string_writer writer;
            menuNode.print(writer, _T("    "), pugi::format_default);
            menuXmlPath = _T("<Window shadow_type = \"menu_round\" shadow_border_size = \"1\" shadow_border_color = \"border_window\">");
            menuXmlPath += writer.result;
            menuXmlPath += _T("</Window>");
        }
    }
    pMenuBar->AddTopMenu(menuItemId, menuText, menuTextId, menuXmlPath);
}

void WindowBuilder::ParsePropertyGridGroupXmlNode(const pugi::xml_node& node, Control* pParent, Window* /*pWindow*/) const
{
    PropertyGrid* pPropertyGrid = dynamic_cast<PropertyGrid*>(pParent);
    ASSERT((pPropertyGrid != nullptr) && !node.attributes().empty());
    if ((pPropertyGrid == nullptr) || node.attributes().empty()) {
        return;
    }
    PropertyGridGroup* pPropertyGridGroup = nullptr;
    DString name = node.attribute(_T("name")).as_string();
    DString groupName = node.attribute(_T("group_name")).as_string();
    DString groupNameId = node.attribute(_T("group_name_id")).as_string();
    DString groupDescription = node.attribute(_T("description")).as_string();
    if (!groupNameId.empty()) {
        pPropertyGridGroup = pPropertyGrid->AddGroupById(groupNameId, groupDescription);
    }
    else {
        pPropertyGridGroup = pPropertyGrid->AddGroup(groupName, groupDescription);
    }
    ASSERT(pPropertyGridGroup != nullptr);
    if (pPropertyGridGroup == nullptr) {
        return;
    }
    if (!name.empty()) {
        pPropertyGridGroup->SetName(name);
    }

    for (pugi::xml_node childNode : node.children()) {
        DString strClass = childNode.name();
        DString propName = childNode.attribute(_T("name")).as_string();
        DString propNameId = childNode.attribute(_T("name_id")).as_string();
        DString propValue = childNode.attribute(_T("value")).as_string();
        DString propValueId = childNode.attribute(_T("value_id")).as_string();
        DString propDescription = childNode.attribute(_T("description")).as_string();
        DString propMargin = childNode.attribute(_T("margin")).as_string();
        DString propPadding = childNode.attribute(_T("padding")).as_string();
        PropertyGridProperty* pProperty = nullptr;
        if (strClass == _T("PropertyGridTextProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddTextPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddTextProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridComboProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddComboPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddComboProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridFontProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddFontPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddFontProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridFontSizeProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddFontSizePropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddFontSizeProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridColorProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddColorPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddColorProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridDateTimeProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddDateTimePropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddDateTimeProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridIPAddressProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddIPAddressPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddIPAddressProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridHotKeyProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddHotKeyPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddHotKeyProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridFileProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddFilePropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddFileProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        else if (strClass == _T("PropertyGridDirectoryProperty")) {
            if (!propNameId.empty()) {
                pProperty = pPropertyGrid->AddDirectoryPropertyById(pPropertyGridGroup, propNameId, propValueId.empty() ? propValue : propValueId, propDescription, 0, !propValueId.empty());
            }
            else {
                pProperty = pPropertyGrid->AddDirectoryProperty(pPropertyGridGroup, propName, propValue, propDescription);
            }
        }
        if (pProperty != nullptr) {
            if (!propName.empty()) {
                pProperty->SetName(propName);
            }
            if (!propMargin.empty()) {
                pProperty->SetAttribute(_T("margin"), propMargin);
            }
            if (!propPadding.empty()) {
                pProperty->SetAttribute(_T("padding"), propPadding);
            }
        }
    }
}

void WindowBuilder::ParseCheckComboTextXmlNode(const pugi::xml_node& node, Control* pParent, Window* /*pWindow*/) const
{
    CheckCombo* pCheckCombo = dynamic_cast<CheckCombo*>(pParent);
    ASSERT((pCheckCombo != nullptr) && !node.attributes().empty());
    if ((pCheckCombo == nullptr) || node.attributes().empty()) {
        return;
    }

    DString text = node.attribute(_T("text")).as_string();
    DString textId = node.attribute(_T("text_id")).as_string();
    DString selected = node.attribute(_T("selected")).as_string();
    if (text.empty() && textId.empty()) {
        return;
    }
    if (!textId.empty()) {
        pCheckCombo->AddTextIdItem(textId);
        if (StringUtil::IsValueTrue(selected)) {
            pCheckCombo->SelectTextIdItem(textId, true);
        }
    }
    else {
        pCheckCombo->AddTextItem(text);
        if (StringUtil::IsValueTrue(selected)) {
            pCheckCombo->SelectTextItem(text, true);
        }
    }
}

void WindowBuilder::ParseListCtrlXmlNode(const pugi::xml_node& node, Control* pParent, Window* /*pWindow*/) const
{
    ListCtrl* pListCtrl = dynamic_cast<ListCtrl*>(pParent);
    ASSERT((pListCtrl != nullptr) && !node.attributes().empty());
    if ((pListCtrl == nullptr) || node.attributes().empty()) {
        return;
    }

    DString strClass = node.name();
    if (strClass == _T("ListCtrlHeaderItem"))  {
        //ListCtrlHeaderItem节点
        ListCtrlColumn column;
        column.text = node.attribute(_T("text")).as_string();
        column.textId = node.attribute(_T("text_id")).as_string();
        column.nColumnWidth = node.attribute(_T("width")).as_int();
        if (column.nColumnWidth <= 0) {
            column.nColumnWidth = 100;
        }
        pListCtrl->InsertColumn(-1, column);
    }
    else if (strClass == _T("ListCtrlItem")) {
        //ListCtrlItem节点: 添加一行
        ListCtrlSubItemData dataItem;
        dataItem.text = node.attribute(_T("text")).as_string();
        pListCtrl->AddDataItem(dataItem);
    }
    else if (strClass == _T("ListCtrlSubItem")) {
        //ListCtrlSubItem节点
        int32_t nItemIndex = node.attribute(_T("item_index")).as_int();
        int32_t nColumnIndex = node.attribute(_T("column_index")).as_int();
        ListCtrlSubItemData subItem;
        subItem.text = node.attribute(_T("text")).as_string();
        pListCtrl->SetSubItemData((size_t)nItemIndex, (size_t)nColumnIndex, subItem);
    }
}

bool WindowBuilder::ParseRichTextXmlText(const DString& xmlText, Control* pControl)
{
#ifdef DUILIB_UNICODE
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf16;
#else
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf8;
#endif
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xmlText.c_str(),
                                                    xmlText.size() * sizeof(DString::value_type),
                                                    pugi::parse_default,
                                                    encoding);
    if (result.status != pugi::status_ok) {
        ASSERT(!_T("WindowBuilder::ParseRichTextXmlText load xml text failed!"));
        return false;
    }
    pugi::xml_node root = doc.root();
    DString rootName = root.name();
    DString rootValue = root.value();
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
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    //获取实现接口
    RichTextImpl* pRichTextImpl = nullptr;
    RichText* pRichText = dynamic_cast<RichText*>(pControl);
    if (pRichText != nullptr) {
        pRichTextImpl = pRichText->GetRichTextImpl();
    }
    if (pRichTextImpl == nullptr) {
        RichTextBox* pRichTextBox = dynamic_cast<RichTextBox*>(pControl);
        if (pRichTextBox != nullptr) {
            pRichTextImpl = pRichTextBox->GetRichTextImpl();
        }
    }
    if (pRichTextImpl == nullptr) {
        RichTextHBox* pRichTextBox = dynamic_cast<RichTextHBox*>(pControl);
        if (pRichTextBox != nullptr) {
            pRichTextImpl = pRichTextBox->GetRichTextImpl();
        }
    }
    if (pRichTextImpl == nullptr) {
        RichTextVBox* pRichTextBox = dynamic_cast<RichTextVBox*>(pControl);
        if (pRichTextBox != nullptr) {
            pRichTextImpl = pRichTextBox->GetRichTextImpl();
        }
    }
    ASSERT(pRichTextImpl != nullptr);
    return ParseRichTextXmlNode(xmlNode, pRichTextImpl, pTextSlice);
}

bool WindowBuilder::ParseRichTextXmlNode(const pugi::xml_node& xmlNode, RichTextImpl* pRichTextImpl, RichTextSlice* pTextSlice)
{
    ASSERT(pRichTextImpl != nullptr);
    if (pRichTextImpl == nullptr) {
        return false;
    }

    DString nodeName;
    for (pugi::xml_node node : xmlNode.children()) {
        RichTextSlice textSlice;
        textSlice.m_nodeName = node.name();
        nodeName = textSlice.m_nodeName.c_str();

        bool bParseChildren = true;
        if (nodeName.empty()) {            
            //无节点名称，只读取文本内容, 不需要递归遍历子节点
#ifdef DUILIB_UNICODE
            textSlice.m_text = pRichTextImpl->TrimText(node.value());
#else
            textSlice.m_text = StringConvert::UTF8ToWString(pRichTextImpl->TrimText(node.value()));
#endif
            bParseChildren = false;
        }        
        else if (nodeName == _T("a")) {
#ifdef DUILIB_UNICODE
            textSlice.m_text = pRichTextImpl->TrimText(node.first_child().value());
#else
            textSlice.m_text = StringConvert::UTF8ToWString(pRichTextImpl->TrimText(node.first_child().value()));
#endif
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
            textSlice.m_text = L"\n";
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
            ParseRichTextXmlNode(node, pRichTextImpl, &textSlice);
        }
        //将子节点添加到Control或者父节点(注意：std::move以后，textSlice对象失效)
        if (pTextSlice != nullptr) {
            pTextSlice->m_children.emplace_back(std::move(textSlice));
        }
        else {
            pRichTextImpl->AppendTextSlice(std::move(textSlice));
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
    DString strType;
    DString strReceiver;
    DString strApplyAttribute;
    DString strName;
    DString strValue;
    int i = 0;
    for (pugi::xml_attribute attr : node.attributes()) {
        strName = attr.name();
        strValue = attr.value();
        ASSERT_UNUSED_VARIABLE(i != 0 || strName == _T("type"));
        ASSERT_UNUSED_VARIABLE(i != 1 || strName == _T("receiver"));
        ASSERT_UNUSED_VARIABLE(i != 2 || ((strName == _T("applyattribute")) || (strName == _T("apply_attribute"))));
        ++i;
        if (strName == _T("type") ) {
            strType = strValue;
        }
        else if (strName == _T("receiver") ) {
            strReceiver = strValue;
        }
        else if ((strName == _T("apply_attribute")) || (strName == _T("applyattribute"))) {
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
            EventType eventType = EventUtils::StringToEventType(*itType);
            ASSERT(eventType != EventType::kEventNone);//如果有断言，说明XML中配置的消息名称不正确
            if (eventType == EventType::kEventNone) {
                continue;
            }
            auto callback = UiBind(&Control::OnApplyAttributeList, pParent, *itReceiver, strApplyAttribute, std::placeholders::_1);
            if (!bBubbled) {
                pParent->AttachXmlEvent(eventType, callback, 0);
            }
            else {
                pParent->AttachXmlBubbledEvent(eventType, callback, 0);
            }
        }
    }
}

Box* WindowBuilder::ToBox(Control* pControl) const
{
    if (pControl == nullptr) {
        return nullptr;
    }
    Box* pBox = dynamic_cast<Box*>(pControl);
    ASSERT(pBox != nullptr);
    return pBox;
}

bool WindowBuilder::ParseWindowAttributes(std::map<DString, DString>& windowAttributes) const
{
    if (m_xml == nullptr) {
        return false;
    }
    pugi::xml_node root = m_xml->root().first_child();
    ASSERT(!root.empty());
    if (root.empty()) {
        return false;
    }

    DString strClass = root.name();
    if (strClass == _T("Window")) {
        for (pugi::xml_attribute attr : root.attributes()) {
            windowAttributes[attr.name()] = attr.value();            
        }
        return true;
    }
    return false;
}

const std::vector<DString>& WindowBuilder::GetWindowClassList() const
{
    return m_windowClassList;
}

const std::vector<DString>& WindowBuilder::GetWindowThemeColorList() const
{
    return m_windowThemeColorList;
}

const std::vector<DString>& WindowBuilder::GetGlobalFontIdList() const
{
    return m_globalFontIdList;
}

} // namespace ui
