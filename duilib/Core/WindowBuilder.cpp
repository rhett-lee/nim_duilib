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
#include "duilib/Core/WindowAttributes.h"
#include "duilib/Utils/LogUtil.h"

#include "duilib/Control/TreeView.h"
#include "duilib/Control/DirectoryTree.h"
#include "duilib/Control/Combo.h"
#include "duilib/Control/ComboButton.h"
#include "duilib/Control/FilterCombo.h"
#include "duilib/Control/CheckCombo.h"
#include "duilib/Control/Slider.h"
#include "duilib/Control/RangeSlider.h"
#include "duilib/Control/Progress.h"
#include "duilib/Control/CircleProgress.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/SpinBox.h"
#include "duilib/Control/SearchBox.h"
#include "duilib/Control/UrlBar.h"
#include "duilib/Control/Switch.h"
#include "duilib/Control/Badge.h"
#include "duilib/Control/Avatar.h"
#include "duilib/Control/TagInput.h"
#include "duilib/Control/AutoComplete.h"
#include "duilib/Control/Mask.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/DateTime.h"
#include "duilib/Control/Calendar.h"
#include "duilib/Control/CalendarFlyout.h"
#include "duilib/Control/NavigationView.h"
#include "duilib/Control/Chart.h"
#include "duilib/Control/Marquee.h"
#include "duilib/Control/Timeline.h"
#include "duilib/Control/ChatMessageList.h"
#include "duilib/Control/Stepper.h"
#include "duilib/Control/Rating.h"
#include "duilib/Control/Carousel.h"
#include "duilib/Control/Pagination.h"

#include <algorithm>
#include "duilib/Control/Breadcrumb.h"
#include "duilib/Control/CascadingSelect.h"
#include "duilib/Control/Popover.h"
#include "duilib/Control/Popconfirm.h"
#include "duilib/Control/TreeGrid.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/GroupBox.h"
#include "duilib/Control/StatusBar.h"
#include "duilib/Control/ToolBar.h"

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
#include "duilib/Box/Panel.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/LogUtil.h"

#include "duilib/third_party/xml/pugixml.hpp"

namespace ui 
{

namespace
{
    /** 计算XML数据中某个字节偏移对应的行号和列号（行号、列号均从1开始计数）
    * @param [in] data XML数据（UTF-8 或者 UTF-16 编码）
    * @param [in] dataSize XML数据的字节数
    * @param [in] offsetBytes 解析失败位置的字节偏移
    * @param [out] nLine 行号
    * @param [out] nColumn 列号
    */
    template<typename CharType>
    void CalcXmlParseErrorPosition(const CharType* data, size_t dataSize, size_t offsetBytes, uint32_t& nLine, uint32_t& nColumn)
    {
        nLine = 1;
        nColumn = 1;
        if ((data == nullptr) || (dataSize == 0)) {
            return;
        }
        const size_t charCount = dataSize / sizeof(CharType);
        size_t nIndex = offsetBytes / sizeof(CharType);
        if (nIndex > charCount) {
            nIndex = charCount;
        }
        for (size_t i = 0; i < nIndex; ++i) {
            const CharType ch = data[i];
            if (ch == static_cast<CharType>('\n')) {
                ++nLine;
                nColumn = 1;
            }
            else if (ch == static_cast<CharType>('\r')) {
                //换行符(\r\n)中的'\r'，不单独计数
            }
            else if constexpr (sizeof(CharType) == 1) {
                if ((static_cast<unsigned char>(ch) & 0xC0) == 0x80) {
                    //UTF-8 编码中的后续字节，不单独计数
                    continue;
                }
                ++nColumn;
            }
            else {
                ++nColumn;
            }
        }
    }

    /** 生成XML解析失败的错误信息（包含文件路径、行号、列号和失败原因）
    */
    DString MakeXmlParseError(const FilePath& xmlFilePath, const pugi::xml_parse_result& result, uint32_t nLine, uint32_t nColumn)
    {
        const DString strDescription = StringConvert::UTF8ToT(std::string(result.description()));
        return StringUtil::Printf(_T("XML parse failed: %s, line %u, column %u: %s"),
                                  xmlFilePath.ToString().c_str(), nLine, nColumn, strDescription.c_str());
    }
}


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
        {DUI_CTR_RANGE_SLIDER, [](Window* pWindow) { return new RangeSlider(pWindow); }},
        {DUI_CTR_PROGRESS, [](Window* pWindow) { return new Progress(pWindow); }},
        {DUI_CTR_CIRCLEPROGRESS, [](Window* pWindow) { return new CircleProgress(pWindow); }},
        {DUI_CTR_RICHTEXT, [](Window* pWindow) { return new RichText(pWindow); }},
        {DUI_CTR_RICHTEXT_BOX, [](Window* pWindow) { return new RichTextBox(pWindow); }},
        {DUI_CTR_RICHTEXT_HBOX, [](Window* pWindow) { return new RichTextHBox(pWindow); }},
        {DUI_CTR_RICHTEXT_VBOX, [](Window* pWindow) { return new RichTextVBox(pWindow); }},
        {DUI_CTR_RICHEDIT, [](Window* pWindow) { return new RichEdit(pWindow); }},
        {DUI_CTR_RICHEDIT2, [](Window* pWindow) { return new RichEdit2(pWindow); }},
        {DUI_CTR_SPIN_BOX, [](Window* pWindow) { return new SpinBox(pWindow); }},
        {DUI_CTR_SEARCH_BOX, [](Window* pWindow) { return new SearchBox(pWindow); }},
        {DUI_CTR_URL_BAR, [](Window* pWindow) { return new UrlBar(pWindow); }},
        {DUI_CTR_SWITCH, [](Window* pWindow) { return new Switch(pWindow); }},
        {DUI_CTR_BADGE, [](Window* pWindow) { return new Badge(pWindow); }},
        {DUI_CTR_AVATAR, [](Window* pWindow) { return new Avatar(pWindow); }},
        {DUI_CTR_TAG_INPUT, [](Window* pWindow) { return new TagInput(pWindow); }},
        {DUI_CTR_AUTO_COMPLETE, [](Window* pWindow) { return new AutoComplete(pWindow); }},
        {DUI_CTR_MASK, [](Window* pWindow) { return new Mask(pWindow); }},
        {DUI_CTR_DATETIME, [](Window* pWindow) { return new DateTime(pWindow); }},
        {DUI_CTR_CALENDAR, [](Window* pWindow) { return new Calendar(pWindow); }},
        {DUI_CTR_NAVIGATIONVIEW, [](Window* pWindow) { return new NavigationView(pWindow); }},
        {DUI_CTR_NAVIGATION_VIEW_ITEM, [](Window* pWindow) { return new NavigationViewItem(pWindow); }},
        {DUI_CTR_CHART, [](Window* pWindow) { return new Chart(pWindow); }},
        {DUI_CTR_MARQUEE, [](Window* pWindow) { return new Marquee(pWindow); }},
        {DUI_CTR_TIMELINE, [](Window* pWindow) { return new Timeline(pWindow); }},
        {DUI_CTR_CHATMESSAGELIST, [](Window* pWindow) { return new ChatMessageList(pWindow); }},
        {DUI_CTR_STEPPER, [](Window* pWindow) { return new Stepper(pWindow); }},
        {DUI_CTR_RATING, [](Window* pWindow) { return new Rating(pWindow); }},
        {DUI_CTR_CAROUSEL, [](Window* pWindow) { return new Carousel(pWindow); }},
        {DUI_CTR_PAGINATION, [](Window* pWindow) { return new Pagination(pWindow); }},
        {DUI_CTR_BREADCRUMB, [](Window* pWindow) { return new Breadcrumb(pWindow); }},
        {DUI_CTR_CASCADING_SELECT, [](Window* pWindow) { return new CascadingSelect(pWindow); }},
        {DUI_CTR_POPOVER, [](Window* pWindow) { return new Popover(pWindow); }},
        {DUI_CTR_POPCONFIRM, [](Window* pWindow) { return new Popconfirm(pWindow); }},
        {DUI_CTR_TREE_GRID, [](Window* pWindow) { return new TreeGrid(pWindow); }},
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
        {DUI_CTR_STATUS_BAR, [](Window* pWindow) { return new StatusBar(pWindow); }},
        {DUI_CTR_TOOL_BAR, [](Window* pWindow) { return new ToolBar(pWindow); }},
        {DUI_CTR_CHILD_WINDOW, [](Window* pWindow) { return new ChildWindow(pWindow); }},

        {DUI_CTR_SPLIT, [](Window* pWindow) { return new Split(pWindow); }},
        {DUI_CTR_SPLITBOX, [](Window* pWindow) { return new SplitBox(pWindow); }},
        {DUI_CTR_GROUP_BOX, [](Window* pWindow) { return new GroupBox(pWindow); }},
        {DUI_CTR_GROUP_HBOX, [](Window* pWindow) { return new GroupHBox(pWindow); }},
        {DUI_CTR_GROUP_VBOX, [](Window* pWindow) { return new GroupVBox(pWindow); }},

        {DUI_CTR_PANEL, [](Window* pWindow) { return new Panel(pWindow); }},
        {DUI_CTR_PANEL_HBOX, [](Window* pWindow) { return new PanelHBox(pWindow); }},
        {DUI_CTR_PANEL_VBOX, [](Window* pWindow) { return new PanelVBox(pWindow); }},

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
    m_lastError.clear();
    ASSERT(!xmlFileData.empty() && _T("xmlFileData is empty!"));
    if (xmlFileData.empty()) {
        m_lastError = StringUtil::Printf(_T("XML data is empty, file: %s"), xmlFilePath.ToString().c_str());
        LogUtil::OutputLine(m_lastError, false);
        return false;
    }
    bool isLoaded = false;
    //字符串以<开头认为是XML字符串，否则认为是XML文件
    //如果使用了 zip 压缩包，则从内存中读取
    if (xmlFileData.front() == _T('<')) {
#ifdef DUILIB_UNICODE
        //自动适配wchar_t的编码：Windows为UTF16，Linux/macOS为UTF32
        pugi::xml_encoding encoding = pugi::xml_encoding::encoding_wchar;
#else
        pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf8;
#endif
        pugi::xml_parse_result result = m_xml->load_buffer(xmlFileData.c_str(),
                                                           xmlFileData.size() * sizeof(DString::value_type),
                                                           pugi::parse_default, encoding);
        isLoaded = result.status == pugi::status_ok;
        if (!isLoaded) {
            uint32_t nLine = 1;
            uint32_t nColumn = 1;
            CalcXmlParseErrorPosition(xmlFileData.data(), xmlFileData.size() * sizeof(DString::value_type),
                                      (size_t)result.offset, nLine, nColumn);
            m_lastError = MakeXmlParseError(xmlFilePath, result, nLine, nColumn);
            LogUtil::OutputLine(m_lastError, false);
        }
    }
    else {
        m_lastError = StringUtil::Printf(_T("XML data does not start with '<', file: %s"), xmlFilePath.ToString().c_str());
        LogUtil::OutputLine(m_lastError, false);
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
    m_lastError.clear();
    ASSERT(!xmlFileData.empty() && _T("xmlFileData is empty!"));
    if (xmlFileData.empty()) {
        m_lastError = StringUtil::Printf(_T("XML data is empty, file: %s"), xmlFilePath.ToString().c_str());
        LogUtil::OutputLine(m_lastError, false);
        return false;
    }
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_auto;
    pugi::xml_parse_result result = m_xml->load_buffer(xmlFileData.data(),
                                                       xmlFileData.size(),
                                                       pugi::parse_default, encoding);
    bool isLoaded = result.status == pugi::status_ok;
    if (!isLoaded) {
        uint32_t nLine = 1;
        uint32_t nColumn = 1;
        CalcXmlParseErrorPosition(xmlFileData.data(), xmlFileData.size(), (size_t)result.offset, nLine, nColumn);
        m_lastError = MakeXmlParseError(xmlFilePath, result, nLine, nColumn);
        LogUtil::OutputLine(m_lastError, false);
        ASSERT(!_T("WindowBuilder::ParseXmlData load xmlFileData failed!"));
        return false;
    }
    m_xmlFilePath = xmlFilePath;
    return true;
}

bool WindowBuilder::ParseXmlFile(const FilePath& xmlFilePath, const FilePath& windowResPath)
{
    m_lastError.clear();
    ASSERT(!xmlFilePath.IsEmpty() && _T("xmlFilePath is empty!"));
    if (xmlFilePath.IsEmpty()) {
        m_lastError = _T("XML file path is empty");
        LogUtil::OutputLine(m_lastError, false);
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
                uint32_t nLine = 1;
                uint32_t nColumn = 1;
                CalcXmlParseErrorPosition(xmlFileData.data(), xmlFileData.size(), (size_t)result.offset, nLine, nColumn);
                m_lastError = MakeXmlParseError(xmlFilePath, result, nLine, nColumn);
                LogUtil::OutputLine(m_lastError, false);
                ASSERT(!_T("WindowBuilder::ParseXmlFile load xml from zip data failed!"));
                return false;
            }
            isLoaded = true;
        }
        else if (!xmlFileFullPath.IsEmpty()) {
            pugi::xml_parse_result result = m_xml->load_file(xmlFileFullPath.NativePathA().c_str());
            if (result.status != pugi::status_ok) {
                //解析磁盘文件失败：重新读取文件内容，用于计算错误位置的行号和列号
                uint32_t nLine = 1;
                uint32_t nColumn = 1;
                std::vector<uint8_t> fileData;
                if (FileUtil::ReadFileData(xmlFileFullPath, fileData) && !fileData.empty()) {
                    CalcXmlParseErrorPosition(fileData.data(), fileData.size(), (size_t)result.offset, nLine, nColumn);
                }
                m_lastError = MakeXmlParseError(xmlFileFullPath, result, nLine, nColumn);
                LogUtil::OutputLine(m_lastError, false);
                ASSERT(!_T("WindowBuilder::ParseXmlFile load xml file failed!"));
                return false;
            }
            isLoaded = true;
        }
    }
    if (!isLoaded) {
        m_lastError = StringUtil::Printf(_T("XML file not found: %s, windowResPath: %s"),
                                         xmlFilePath.ToString().c_str(), windowResPath.ToString().c_str());
        LogUtil::OutputLine(m_lastError, false);
        ASSERT(!_T("WindowBuilder::ParseXmlFile load xmlFilePath failed!"));
        return false;
    }
    m_xmlFilePath = xmlFilePath;
    return true;
}

const DString& WindowBuilder::GetLastError() const
{
    return m_lastError;
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
        else if ((strClass == _T("Include")) && (StringUtil::StringCompare(root.name(), _T("Global")) == 0)) {
            //全局资源中的Include节点，已在ParseGlobalAttributes中处理，这里跳过
            continue;
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

namespace
{
    /** 输出窗口配置的告警信息（属性值非法或配置冲突时的纠正说明），输出后清空列表
    */
    void OutputWindowConfigWarnings(std::vector<DString>& warnings)
    {
        for (const DString& warning : warnings) {
            LogUtil::OutputLine(StringUtil::Printf(_T("WindowBuilder: %s"), warning.c_str()));
        }
        warnings.clear();
    }

    /** 构造渲染后端的默认配置（窗口当前配置与全局配置，后者优先作为默认值）
    */
    RenderBackendResolveParam MakeRenderBackendResolveParam(Window* pWindow)
    {
        RenderBackendResolveParam renderParam;
        if (pWindow != nullptr) {
            renderParam.m_defaultBackendType = pWindow->GetRenderBackendType();
            renderParam.m_nDefaultMsaaSampleCount = pWindow->GetRenderMsaaSampleCount();
            renderParam.m_bDefaultDisableVsync = pWindow->IsRenderDisableVsync();
        }
        const GlobalManager& globalManager = GlobalManager::Instance();
        if (globalManager.IsGlobalRenderBackendTypeEnabled()) {
            renderParam.m_defaultBackendType = globalManager.GetGlobalRenderBackendType();
            renderParam.m_nDefaultMsaaSampleCount = globalManager.GetGlobalMsaaSampleCount();
            renderParam.m_bDefaultDisableVsync = globalManager.IsGlobalDisableVsync();
        }
        return renderParam;
    }

    /** 构造阴影/分层窗口推导的平台参数
    */
    ShadowResolveParam MakeShadowResolveParam(Window* pWindow, bool bLayeredWindow)
    {
        ShadowResolveParam param;
        if (pWindow != nullptr) {
            param.m_bSystemShadowSupported = pWindow->NativeWnd()->IsSystemShadowSupported();
        }
        param.m_bLayeredWindow = bLayeredWindow;
        return param;
    }
}

bool WindowBuilder::ParseWindowCreateAttributes(Window* pWindow, WindowCreateAttributes& createAttributes)
{
    //该函数应在窗口创建前调用
    if (pWindow != nullptr) {
        ASSERT(!pWindow->IsWindow());
    }
    pugi::xml_node root = m_xml->root().first_child();
    ASSERT(!root.empty());
    if (root.empty()) {
        return false;
    }
    ASSERT(StringUtil::StringCompare(root.name(), _T("Window")) == 0);

    //单趟解析XML中窗口的全部属性（属性的书写顺序不影响解析结果）
    WindowAttributeValues values;
    if (!ParseWindowAttributeValues(root, values, nullptr)) {
        return false;
    }

    //窗口创建依赖的属性：直接映射解析结果
    createAttributes.m_bUseSystemCaption = values.m_bUseSystemCaption;
    createAttributes.m_bUseSystemCaptionDefined = values.m_bUseSystemCaptionDefined;

    createAttributes.m_rcSizeBox = values.m_rcSizeBox;
    createAttributes.m_bSizeBoxDefined = values.m_bSizeBoxDefined;

    createAttributes.m_rcCaption = values.m_rcCaption;
    createAttributes.m_bCaptionDefined = values.m_bCaptionDefined;

    createAttributes.m_sdlRenderName = values.m_sdlRenderName;

    //创建前统一Resolve：与创建后使用同一套规则，避免两套推导结果不一致
    const RenderBackendResolveParam renderParam = MakeRenderBackendResolveParam(pWindow);
    std::vector<DString> warnings = values.m_warnings;
    const ShadowResolveParam resolveParam = MakeShadowResolveParam(pWindow, false);
    const ResolvedWindowAttributes resolved = ResolveWindowAttributes(values, renderParam, resolveParam, &warnings);
    OutputWindowConfigWarnings(warnings);

    createAttributes.m_bShadowAttached = resolved.m_shadow.m_bShadowAttached;
    createAttributes.m_bShadowAttachedDefined = values.m_bShadowAttachedDefined;

    createAttributes.m_bIsLayeredWindow = resolved.m_layered.m_bIsLayeredWindow;
    createAttributes.m_bIsLayeredWindowDefined = resolved.m_layered.m_bIsLayeredWindowDefined;
    createAttributes.m_nLayeredWindowAlpha = (uint8_t)resolved.m_layered.m_nAlpha;
    createAttributes.m_bLayeredWindowAlphaDefined = resolved.m_layered.m_bAlphaDefined;
    createAttributes.m_nLayeredWindowOpacity = (uint8_t)resolved.m_layered.m_nOpacity;
    createAttributes.m_bLayeredWindowOpacityDefined = resolved.m_layered.m_bOpacityDefined;
    createAttributes.m_rcShadowCorner = resolved.m_shadow.m_rcShadowCorner;

    //评估窗口的初始大小（必须依据最终的阴影九宫格与min/max属性）
    if (values.m_bSizeDefined) {
        WindowInitSizeResolveParam sizeParam;
        sizeParam.m_szMinSize = values.m_szMinSize;
        sizeParam.m_szMaxSize = values.m_szMaxSize;
        sizeParam.m_rcShadowCorner = resolved.m_shadow.m_rcShadowCorner;
        sizeParam.m_bShadowAttached = resolved.m_shadow.m_bShadowAttached;
        sizeParam.m_bSizeContainShadow = values.m_bSizeContainShadow;
        sizeParam.m_pWindow = nullptr;
        createAttributes.m_szInitSize = ResolveWindowInitSize(values, sizeParam, GlobalManager::Instance().Dpi());
        createAttributes.m_bInitSizeDefined = true;
    }
    return true;
}

namespace
{
    /** 将解析后的窗口属性应用到窗口
    *   按固定阶段执行，每个属性只出现一处，不依赖XML中属性的书写顺序：
    *   1. 渲染后端（后续属性的设置依赖Render对象）；
    *   2. 结构性属性（min/max大小、系统标题栏、标题栏区域等）；
    *   3. 阴影（先阴影类型，再显式配置，最后设置是否支持阴影）；
    *   4. 层窗口与透明度；
    *   5. 无依赖的属性；
    *   6. 窗口的初始大小（最后设置）。
    */
    void ApplyWindowAttributes(Window* pWindow, const WindowAttributeValues& values)
    {
        ASSERT(pWindow != nullptr);
        if (pWindow == nullptr) {
            return;
        }

        //统一Resolve：渲染后端 → 阴影 → 分层窗口（与创建前使用同一套规则）
        const RenderBackendResolveParam renderParam = MakeRenderBackendResolveParam(pWindow);
        const ShadowResolveParam resolveParam = MakeShadowResolveParam(pWindow, pWindow->IsLayeredWindow());
        std::vector<DString> warnings = values.m_warnings;
        const ResolvedWindowAttributes resolved = ResolveWindowAttributes(values, renderParam, resolveParam, &warnings);
        OutputWindowConfigWarnings(warnings);

        //【阶段1】渲染后端：必须最先设置，后续属性依赖Render对象
        pWindow->SetRenderBackendType(resolved.m_render.m_backendType,
                                      resolved.m_render.m_nMsaaSampleCount,
                                      resolved.m_render.m_bDisableVsync);

        //【阶段2】结构性属性：其他属性依赖这些属性（尤其是use_system_caption）
        if (values.m_bMinSizeDefined) {
            pWindow->SetWindowMinimumSize(values.m_szMinSize, true);
        }
        if (values.m_bMaxSizeDefined) {
            pWindow->SetWindowMaximumSize(values.m_szMaxSize, true);
        }
        if (values.m_bUseSystemCaptionDefined) {
            pWindow->SetUseSystemCaption(values.m_bUseSystemCaption);
        }
        if (values.m_bSizeBoxDefined) {
            pWindow->SetSizeBox(values.m_rcSizeBox, true);
        }
        if (values.m_bCaptionDefined) {
            pWindow->SetCaptionRect(values.m_rcCaption, true);
        }
        if (values.m_bEnableSnapLayoutMenuDefined) {
            pWindow->SetEnableSnapLayoutMenu(values.m_bEnableSnapLayoutMenu);
        }
        if (values.m_bEnableSysMenuDefined) {
            pWindow->SetEnableSysMenu(values.m_bEnableSysMenu);
        }
        if (values.m_bSysMenuRectDefined) {
            pWindow->SetSysMenuRect(values.m_rcSysMenuRect, true);
        }
        if (values.m_bWindowIconDefined && !values.m_windowIcon.empty()) {
            pWindow->SetWindowIcon(values.m_windowIcon);
        }
        if (values.m_bWindowTextDefined) {
            pWindow->SetText(values.m_windowText);
        }
        if (values.m_bWindowTextIdDefined) {
            pWindow->SetTextId(values.m_windowTextId);
        }
        if (values.m_bRoundCornerDefined) {
            pWindow->SetRoundCorner(values.m_szRoundCorner.cx, values.m_szRoundCorner.cy);
        }

        //【阶段3】阴影：使用Resolve后的有效类型，再应用显式配置，最后应用有效开关
        pWindow->SetShadowType(resolved.m_shadow.m_nShadowType);
        if (values.m_bShadowImageDefined) {
            pWindow->SetShadowImage(values.m_shadowImage);
        }
        if (values.m_bShadowCornerDefined) {
            pWindow->SetShadowCorner(values.m_rcShadowCorner);
        }
        if (values.m_bShadowBorderRoundDefined) {
            pWindow->SetShadowBorderRound(values.m_szShadowBorderRound);
        }
        if (values.m_bShadowBorderSizeDefined) {
            pWindow->SetShadowBorderSize(values.m_nShadowBorderSize);
        }
        if (values.m_bShadowBorderColorDefined) {
            pWindow->SetShadowBorderColor(values.m_shadowBorderColor);
        }
        if (values.m_bEnableShadowSnapDefined) {
            pWindow->SetEnableShadowSnap(values.m_bEnableShadowSnap);
        }
        //最后设置，避免受到"shadow_type"自动开启阴影的影响
        if (pWindow->IsShadowAttached() != resolved.m_shadow.m_bShadowAttached) {
            pWindow->SetShadowAttached(resolved.m_shadow.m_bShadowAttached);
        }

        //【阶段4】层窗口与透明度（互斥与降级规则已在统一Resolve中处理）
        if (!pWindow->IsLayeredWindowSetByUser() &&
            resolved.m_layered.m_bIsLayeredWindowDefined &&
            (resolved.m_layered.m_bIsLayeredWindow != pWindow->IsLayeredWindow())) {
            //仅当推导结果与窗口当前状态不同时才设置（窗口创建时已按相同规则设置过）
            LogUtil::OutputLine(StringUtil::Printf(_T("WindowBuilder: apply layered window: %s, current: %s"),
                                                   resolved.m_layered.m_bIsLayeredWindow ? _T("true") : _T("false"),
                                                   pWindow->IsLayeredWindow() ? _T("true") : _T("false")));
            //窗口配置应用不标记为"外部设置"，后续内部仍可按阴影、系统标题栏等需求自动调整
            pWindow->SetLayeredWindowForConfig(resolved.m_layered.m_bIsLayeredWindow, false);
        }
        if (resolved.m_layered.m_bAlphaDefined) {
            pWindow->SetLayeredWindowAlpha(resolved.m_layered.m_nAlpha);
        }
        if (resolved.m_layered.m_bOpacityDefined) {
            //SDL下opacity不依赖透明窗口；Windows下Resolve已保证只在分层窗口有效时才标记为Defined
            pWindow->SetLayeredWindowOpacity(resolved.m_layered.m_nOpacity);
        }

        //【阶段5】无依赖的属性
        if (values.m_bEnableDragDropDefined) {
            pWindow->SetEnableDragDrop(values.m_bEnableDragDrop);
        }

        //【阶段6】窗口的初始化大小：最后设置，因为初始化大小与阴影、最小/最大尺寸相关
        if (values.m_bSizeDefined) {
            WindowInitSizeResolveParam sizeParam;
            sizeParam.m_szMinSize = pWindow->GetWindowMinimumSize();
            sizeParam.m_szMaxSize = pWindow->GetWindowMaximumSize();
            sizeParam.m_bMinMaxScaled = true;//窗口中的min/max已经是DPI缩放后的值
            sizeParam.m_rcShadowCorner = pWindow->GetShadowCorner();
            sizeParam.m_bShadowAttached = pWindow->IsShadowAttached();
            sizeParam.m_bSizeContainShadow = values.m_bSizeContainShadow;
            sizeParam.m_bWindowMaximized = pWindow->IsWindowMaximized();
            sizeParam.m_pWindow = pWindow;
            const UiSize initSize = ResolveWindowInitSize(values, sizeParam, pWindow->Dpi(), &warnings);
            OutputWindowConfigWarnings(warnings);
            pWindow->SetInitSize(initSize.cx, initSize.cy);
        }
    }
}

void WindowBuilder::ParseWindowAttributes(Window* pWindow, const pugi::xml_node& root) const
{
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return;
    }

    //1. 单趟解析XML中的窗口属性（无副作用）
    WindowAttributeValues values;
    if (!ParseWindowAttributeValues(root, values, pWindow)) {
        return;
    }

    //2. 预警不认识的属性（减少配置错误问题）
#ifdef _DEBUG
    ASSERT_UNUSED_VARIABLE(values.m_unknownAttributes.empty() && "Found unknown window attributes in xml!");
#endif
    for (const DString& unknownName : values.m_unknownAttributes) {
        LogUtil::OutputLine(StringUtil::Printf(_T("WindowBuilder: unknown window attribute: %s"), unknownName.c_str()));
    }

    //3. 输出属性值非法或配置冲突时的纠正信息
    OutputWindowConfigWarnings(values.m_warnings);

    //4. 按固定阶段应用到窗口
    ApplyWindowAttributes(pWindow, values);

    //5. 保存请求值：动态切换（GL↔CPU、标题栏开关等）需要按请求值重新Resolve并恢复冲突属性
    pWindow->SetRequestedWindowAttributes(values);
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
    //全局渲染配置（可选）：在Global节点上设置render_backend_type后，所有窗口统一使用该配置
    //支持属性：render_backend_type（"CPU"/"GL"/"GPU"）、vsync（"true"/"false"）、msaa（整数）
    //注意："GL"/"GPU"仅在Windows平台且非SDL构建模式下支持，其他情况会自动回退为CPU绘制
    {
        DString renderBackendType;
        int32_t nMsaaSampleCount = 0;
        bool bDisableVsync = kDefaultDisableVsync;
        for (pugi::xml_attribute attr : root.attributes()) {
            const DString strAttrName = attr.name();
            const DString strAttrValue = attr.value();
            if (strAttrName == _T("render_backend_type")) {
                renderBackendType = strAttrValue;
            }
            else if (strAttrName == _T("vsync")) {
                bDisableVsync = !StringUtil::IsValueTrue(strAttrValue);
            }
            else if (strAttrName == _T("msaa")) {
                nMsaaSampleCount = StringUtil::StringToInt32(strAttrValue);
                if (nMsaaSampleCount < 0) {
                    nMsaaSampleCount = 0;
                }
            }
        }
        if (!renderBackendType.empty()) {
            RenderBackendType backendType = RenderBackendType::kRaster_BackendType;
            if (StringUtil::IsEqualNoCase(renderBackendType, _T("GL")) ||
                StringUtil::IsEqualNoCase(renderBackendType, _T("GPU"))) {
                backendType = RenderBackendType::kNativeGL_BackendType;
            }
            else if (!StringUtil::IsEqualNoCase(renderBackendType, _T("CPU"))) {
                ASSERT(0);
            }
            //启用全局开关，并应用到当前已经存在的所有窗口
            GlobalManager::Instance().SetGlobalRenderBackendType(backendType, nMsaaSampleCount, bDisableVsync, true, true);
        }
    }

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
        else if (strClass == _T("Include")) {
            //Include节点：引用同目录的XML文件，注册其中的全局资源
            ParseGlobalIncludeXmlNode(node);
        }
    }
}

void WindowBuilder::ParseGlobalIncludeXmlNode(const pugi::xml_node& node)
{
    if (node.attributes().empty()) {
        return;
    }
    pugi::xml_attribute sourceAttr = node.attribute(_T("src"));
    DString sourceValue = sourceAttr.as_string();
    if (sourceValue.empty()) {
        sourceAttr = node.attribute(_T("source"));
        sourceValue = sourceAttr.as_string();
    }
    if (sourceValue.empty()) {
        return;
    }

    //统一路径分隔符
    StringUtil::ReplaceAll(_T("/"), m_xmlFilePath.GetPathSeparatorStr(), sourceValue);
    StringUtil::ReplaceAll(_T("\\"), m_xmlFilePath.GetPathSeparatorStr(), sourceValue);

    FilePath sourceXmlFilePath(sourceValue);
    if (!m_xmlFilePath.IsEmpty()) {
        //优先尝试在原XML文件相同目录加载被引用的XML文件
        DString xmlFilePath = m_xmlFilePath.ToString();
        size_t pos = xmlFilePath.find_last_of(_T("\\/"));
        if (pos != DString::npos) {
            FilePath srcFilePath(xmlFilePath.substr(0, pos));
            srcFilePath.JoinFilePath(FilePath(sourceValue));
            if (GlobalManager::Instance().Theme().IsResFileExists(srcFilePath, FilePath())) {
                sourceXmlFilePath = srcFilePath;
            }
        }
    }

    //使用独立的WindowBuilder解析被引用的XML文件，避免覆盖当前正在遍历的文档
    WindowBuilder builder;
    if (builder.ParseXmlFile(sourceXmlFilePath, FilePath())) {
        pugi::xml_node root = builder.m_xml->root().first_child();
        if (!root.empty() && (StringUtil::StringCompare(root.name(), _T("Global")) == 0)) {
            //被引用的文件根节点为"Global"，递归解析其中的全局资源声明
            builder.ParseGlobalAttributes(root);
        }
        else {
            ASSERT(!"ParseGlobalIncludeXmlNode: the included file root node must be 'Global'");
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
#ifdef _DEBUG
            int32_t i = 0;
#endif
            for (pugi::xml_attribute attr : node.attributes()) {
#ifdef _DEBUG
                // 约束class必须是第一个属性,否则可能会出现属性的值被class中的同名属性覆盖的问题
                // 仅在Debug版本进行校验
                if ((StringUtil::StringCompare(attr.name(), _T("class")) == 0) && (i != 0)) {
                    ASSERT((i == 0) && "The class attribute ‌must be the first attribute‌ in the element.");
                }
                ++i;
#endif
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
        LogUtil::OutputLine(StringUtil::Printf(_T("Include node has no 'src' attribute, included from: %s"),
                                               m_xmlFilePath.ToString().c_str()), false);
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
        else {
            LogUtil::OutputLine(StringUtil::Printf(_T("Include XML load failed: %s, included from: %s"),
                                                   sourceXmlFilePath.ToString().c_str(),
                                                   m_xmlFilePath.ToString().c_str()), false);
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
    DString menuTextButtonClass = node.attribute(_T("class")).as_string();
    DString menuTextButtonAttributes = node.attribute(_T("attributes")).as_string();
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
            menuXmlPath = _T("<Window class = \"menu_window\">");
            menuXmlPath += writer.result;
            menuXmlPath += _T("</Window>");
        }
    }
    pMenuBar->AddTopMenu(menuItemId, menuText, menuTextId, menuXmlPath, menuTextButtonClass, menuTextButtonAttributes);
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
    //自动适配wchar_t的编码：Windows为UTF16，Linux/macOS为UTF32
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_wchar;
#else
    pugi::xml_encoding encoding = pugi::xml_encoding::encoding_utf8;
#endif
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xmlText.c_str(),
                                                    xmlText.size() * sizeof(DString::value_type),
                                                    pugi::parse_default | pugi::parse_ws_pcdata,
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
    //根标签名称不区分大小写（兼容 <RichText>、<richtext> 等写法）
    const DString richtextTagName = StringUtil::MakeLowerString(DString(DUI_CTR_RICHTEXT));
    const DString rootNameLower = StringUtil::MakeLowerString(rootName);
    ASSERT(rootNameLower == richtextTagName);
    if (rootNameLower != richtextTagName) {
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
    bool bFirstNode = true;
    //上一个节点是否为块级节点：块级节点结束后，后续的行内内容也需要另起一段
    bool bPrevBlockNode = false;
    for (pugi::xml_node node : xmlNode.children()) {
        RichTextSlice textSlice;
        textSlice.m_nodeName = node.name();
        nodeName = textSlice.m_nodeName.c_str();
        //标签名称不区分大小写（兼容HTML的写法，如 <B>、<Font>）
        const DString lowerNodeName = StringUtil::MakeLowerString(nodeName);

        const bool bBlockNode = IsRichTextBlockTagName(lowerNodeName);
        //文本节点只有包含可见内容时才参与段落判断，避免格式化文本的换行/缩进产生空行
        const bool bHasVisibleText = !nodeName.empty() || HasRichTextVisibleText(node.value());
        if (!bFirstNode && (bBlockNode ||
                            (bPrevBlockNode && bHasVisibleText && (lowerNodeName != _T("br"))))) {
            //块级节点：先插入换行，保证另起一段（段间距附加在新行的行首元素上）
            RichTextSlice blockBreakSlice;
            blockBreakSlice.m_text = L"\n";
            blockBreakSlice.m_fRowSpacingAdd = (float)pRichTextImpl->GetParagraphSpacing();
            if (pTextSlice != nullptr) {
                pTextSlice->m_children.emplace_back(std::move(blockBreakSlice));
            }
            else {
                pRichTextImpl->AppendTextSlice(std::move(blockBreakSlice), false);
            }
        }
        if (!nodeName.empty() || bHasVisibleText) {
            bFirstNode = false;
        }
        bPrevBlockNode = bBlockNode;

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
        else if (lowerNodeName == _T("a")) {
            //超级链接节点：href属性作用于所有子节点，子节点可以包含格式（如粗体、斜体等）
            textSlice.m_linkUrl = StringUtil::Trim(node.attribute(_T("href")).as_string());
            //自定义的链接提示文本（为空时使用URL作为提示）
            textSlice.m_linkTitle = StringUtil::Trim(node.attribute(_T("title")).as_string());
        }
        else if (lowerNodeName == _T("span")) {
            //行内样式容器（别名）：仅承载颜色属性，文本格式由子节点决定
            textSlice.m_textColor = StringUtil::Trim(node.attribute(_T("color")).as_string());
        }
        else if (lowerNodeName == _T("p")) {
            //段落：支持 align 属性（left/center/right/justify）
            textSlice.m_nHAlign = ParseRichTextHAlignValue(node.attribute(_T("align")).as_string());
        }
        else if ((lowerNodeName == _T("h1")) || (lowerNodeName == _T("h2")) || (lowerNodeName == _T("h3")) ||
                 (lowerNodeName == _T("h4")) || (lowerNodeName == _T("h5")) || (lowerNodeName == _T("h6"))) {
            //标题：按级别设置字号缩放比例，并使用粗体
            const DString::value_type chLevel = lowerNodeName[1];
            static const float kHeadingScale[] = { 2.0f, 1.6f, 1.3f, 1.1f, 1.0f, 0.9f };
            const int32_t nLevelIndex = (int32_t)(chLevel - _T('1'));
            if ((nLevelIndex >= 0) && (nLevelIndex < (int32_t)(sizeof(kHeadingScale) / sizeof(kHeadingScale[0])))) {
                textSlice.m_fFontScale = kHeadingScale[nLevelIndex];
            }
            textSlice.m_fontInfo.m_bBold = true;
            textSlice.m_nHAlign = ParseRichTextHAlignValue(node.attribute(_T("align")).as_string());
        }
        else if ((lowerNodeName == _T("ul")) || (lowerNodeName == _T("ol"))) {
            //列表容器：仅作为块级容器，不产生额外的显示内容
        }
        else if (lowerNodeName == _T("li")) {
            //列表项：在内容前加项目符号（无序列表用实心圆点）
            textSlice.m_text = L"\u2022 ";
        }
        else if (lowerNodeName == _T("sup")) {
            //上标：字号缩小，并按父节点字号的比例向上偏移基线
            textSlice.m_fFontScale = 0.8f;
            textSlice.m_fBaselineScale = -0.15f;
        }
        else if (lowerNodeName == _T("sub")) {
            //下标：字号缩小，并按父节点字号的比例向下偏移基线（需抵消字号缩小时基线被抬高部分）
            textSlice.m_fFontScale = 0.8f;
            textSlice.m_fBaselineScale = 0.45f;
        }
        else if (lowerNodeName == _T("hr")) {
            //水平分割线：没有文本内容，独占一行绘制
            textSlice.m_bHorizontalRule = true;
            textSlice.m_textColor = StringUtil::Trim(node.attribute(_T("color")).as_string());
            bParseChildren = false;
        }
        else if (lowerNodeName == _T("img")) {
            //内联图片：图片底部与当前文本的基线对齐
            textSlice.m_imageSrc = StringUtil::Trim(node.attribute(_T("src")).as_string());
            //显示尺寸(96 DPI)，未指定时使用图片的原始尺寸；限制有效范围，避免异常值
            const int32_t nImageWidth = node.attribute(_T("width")).as_int();
            const int32_t nImageHeight = node.attribute(_T("height")).as_int();
            if (nImageWidth > 0) {
                textSlice.m_nImageWidth = std::min(nImageWidth, 10000);
            }
            if (nImageHeight > 0) {
                textSlice.m_nImageHeight = std::min(nImageHeight, 10000);
            }
            bParseChildren = false;
        }
        else if (lowerNodeName == _T("b")) {
            //粗体字
            textSlice.m_fontInfo.m_bBold = true;
        }
        else if (lowerNodeName == _T("i")) {
            //斜体字
            textSlice.m_fontInfo.m_bItalic = true;
        }
        else if ((lowerNodeName == _T("del")) || (lowerNodeName == _T("s")) || (lowerNodeName == _T("strike"))) {
            //删除字
            textSlice.m_fontInfo.m_bStrikeOut = true;
        }
        else if ( (lowerNodeName == _T("ins")) || (lowerNodeName == _T("u")) ){
            //下划线
            textSlice.m_fontInfo.m_bUnderline = true;
        }
        else if (lowerNodeName == _T("bgcolor")) {
            //背景颜色
            textSlice.m_bgColor = StringUtil::Trim(node.attribute(_T("color")).as_string());
        }
        else if (lowerNodeName == _T("font")) {
            //字体设置：文本颜色
            textSlice.m_textColor = node.attribute(_T("color")).as_string();
            textSlice.m_fontInfo.m_fontName = node.attribute(_T("face")).as_string();
            //字号在解析时进行DPI缩放（见RichTextImpl::ParseTextSlice）；此处限制有效范围，避免异常值
            int32_t nFontSize = node.attribute(_T("size")).as_int();
            if (nFontSize > 0) {
                textSlice.m_fontInfo.m_fontSize = std::clamp(nFontSize, 1, 512);
            }
        }
        else if (lowerNodeName == _T("br")) {
            textSlice.m_text = L"\n";
            //换行节点, 不需要递归遍历子节点
            bParseChildren = false;
        }
        else {
            //遇到不认识的节点：忽略标签本身，但保留其子内容，避免文本丢失
            ASSERT(!"Found unknown xml node name!");
            ParseRichTextXmlNode(node, pRichTextImpl, pTextSlice);
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
            //解析过程中不触发重绘，解析完成后由SetText统一重绘，避免逐节点重绘
            pRichTextImpl->AppendTextSlice(std::move(textSlice), false);
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
