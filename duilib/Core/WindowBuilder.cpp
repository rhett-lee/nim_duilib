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

bool WindowBuilder::IsXmlFileExists(const std::wstring& xml) const
{
	if (xml.empty()) {
		return false;
	}
	if (GlobalManager::Instance().Zip().IsUseZip()) {
		std::wstring sFile = StringHelper::JoinFilePath(GlobalManager::Instance().GetResourcePath(), xml);
		if (GlobalManager::Instance().Zip().IsZipResExist(sFile)) {
			return true;
		}
	}
	std::wstring xmlFilePath = GlobalManager::Instance().GetResourcePath();
	if (StringHelper::IsRelativePath(xml)) {
		xmlFilePath = StringHelper::JoinFilePath(xmlFilePath, xml);
	}
	else {
		xmlFilePath = xml;
	}
	return StringHelper::IsExistsPath(xmlFilePath);
}

Box* WindowBuilder::Create(const std::wstring& xml, 
	                       CreateControlCallback pCallback,
						   Window* pWindow, 
	                       Box* pParent, 
	                       Box* pUserDefinedBox)
{
	ASSERT(!xml.empty() && L"xml 参数为空！");
	if (xml.empty()) {
		return nullptr;
	}
	bool isLoaded = false;
	//字符串以<开头认为是XML字符串，否则认为是XML文件
	//如果使用了 zip 压缩包，则从内存中读取
	if (xml.front() == L'<') {
		pugi::xml_parse_result result = m_xml->load_buffer(xml.c_str(), 
														   xml.size() * sizeof(std::wstring::value_type), 
														   pugi::parse_default, 
			                                               pugi::xml_encoding::encoding_utf16);
		if (result.status != pugi::status_ok) {
			ASSERT(!L"WindowBuilder::Create load xml from string data failed!");
			return nullptr;
		}
		isLoaded = true;
	}
	if (!isLoaded && GlobalManager::Instance().Zip().IsUseZip()) {
		std::wstring sFile = StringHelper::JoinFilePath(GlobalManager::Instance().GetResourcePath(), xml);
		std::vector<unsigned char> file_data;
		if (GlobalManager::Instance().Zip().GetZipData(sFile, file_data)) {
			pugi::xml_parse_result result = m_xml->load_buffer(file_data.data(), file_data.size());
			if (result.status != pugi::status_ok) {
				ASSERT(!L"WindowBuilder::Create load xml from zip data failed!");
				return nullptr;
			}
			isLoaded = true;
		}
	}
	if(!isLoaded) {
		std::wstring xmlFilePath = GlobalManager::Instance().GetResourcePath();
		if (StringHelper::IsRelativePath(xml)) {
			xmlFilePath = StringHelper::JoinFilePath(xmlFilePath, xml);
		}
		else {
			xmlFilePath = xml;
		}
		pugi::xml_parse_result result = m_xml->load_file(xmlFilePath.c_str());
		if (result.status != pugi::status_ok) {
			ASSERT(!L"WindowBuilder::Create load xml file failed!");
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

		if( strClass == L"Window" ) {
			if( pWindow->GetHWND() ) {
				//首先处理mininfo和maxinfo，因为其他属性有用到这两个属性的
				for (pugi::xml_attribute attr : root.attributes()) {
					strName = attr.name();
					strValue = attr.value();
					if (strName == L"mininfo") {
						UiSize size;
						AttributeUtil::ParseSizeValue(strValue.c_str(), size);
						pWindow->SetMinInfo(size.cx, size.cy);
					}
					else if (strName == L"maxinfo") {
						UiSize size;
						AttributeUtil::ParseSizeValue(strValue.c_str(), size);
						pWindow->SetMaxInfo(size.cx, size.cy);
					}
				}
				for (pugi::xml_attribute attr : root.attributes()) {
					strName = attr.name();
					strValue = attr.value();
					if (strName == L"size") {
						UiSize windowSize;
						AttributeUtil::ParseWindowSize(pWindow, strValue.c_str(), windowSize);
						int32_t cx = windowSize.cx;
						int32_t cy = windowSize.cy;
						UiSize minSize = pWindow->GetMinInfo();
						UiSize maxSize = pWindow->GetMaxInfo();
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
					else if( strName == L"sizebox" ) {
						UiRect rcSizeBox;
						AttributeUtil::ParseRectValue(strValue.c_str(), rcSizeBox);
						pWindow->SetSizeBox(rcSizeBox);
					}
					else if( strName == L"caption" ) {
						UiRect rcCaption;
						AttributeUtil::ParseRectValue(strValue.c_str(), rcCaption);
						pWindow->SetCaptionRect(rcCaption);
					}
					else if (strName == L"use_system_caption") {
						pWindow->SetUseSystemCaption(strValue == L"true");
					}
					else if( strName == L"text" ) {
						pWindow->SetText(strValue);
					}
					else if (strName == L"textid") {
						pWindow->SetTextId(strValue);
					}
					else if (strName == L"round_corner" || strName == L"roundcorner" ) {
						UiSize size;
						AttributeUtil::ParseSizeValue(strValue.c_str(), size);
						pWindow->SetRoundCorner(size.cx, size.cy);
					}								
					else if (strName == L"alpha_fix_corner" || strName == L"alphafixcorner") {
						UiRect rc;
						AttributeUtil::ParseRectValue(strValue.c_str(), rc);
						pWindow->SetAlphaFixCorner(rc);
					}
					else if (strName == L"render_transparent") {
						pWindow->SetRenderTransparent(strValue == L"true");
					}
					else if ((strName == L"shadow_attached") || (strName == L"shadowattached")) {
						//设置是否支持窗口阴影（阴影实现有两种：层窗口和普通窗口）
						pWindow->SetShadowAttached(strValue == L"true");
					}
					else if ((strName == L"shadow_image") || (strName == L"shadowimage")) {
						//设置阴影图片
						pWindow->SetShadowImage(strValue);
					}
					else if ((strName == L"shadow_corner") || (strName == L"shadowcorner")) {
						//设置窗口阴影的九宫格属性
						UiPadding padding;
						AttributeUtil::ParsePaddingValue(strValue.c_str(), padding);
						pWindow->SetShadowCorner(padding);
					}
					else if ((strName == L"layered_window") || (strName == L"layeredwindow")) {
						//设置是否设置层窗口属性（层窗口还是普通窗口）
						pWindow->SetLayeredWindow(strValue == L"true");
					}
					else if (strName == L"alpha") {
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

		if( strClass == L"Global" ) {
			for(pugi::xml_node node : root.children()) {
				strClass = node.name();
				if( strClass == L"Image" ) {
					ASSERT(FALSE);	//废弃
				}
				else if (strClass == L"FontResource") {
					std::wstring strFontFile;
					std::wstring strFontName;
					for (pugi::xml_attribute attr : node.attributes()) {
						strName = attr.name();
						strValue = attr.value();
						if (strName == L"file") {
							strFontFile = strValue;
						}
						else if (strName == L"name") {
							strFontName = strValue;
						}
					}
					if (!strFontFile.empty() && !strFontName.empty()) {
						GlobalManager::Instance().Font().AddFontFile(strFontFile, strFontName);
					}
				}
				else if( strClass == L"Font") {
					ParseFontXmlNode(node);
				}
				else if( strClass == L"Class" ) {
					std::wstring strClassName;
					std::wstring strAttribute;
					for (pugi::xml_attribute attr : node.attributes()) {
						strName = attr.name();
						strValue = attr.value();
						if( strName == L"name" ) {
							strClassName = strValue;
						}
						else {
							strAttribute.append(StringHelper::Printf(L" %s=\"%s\"",
												strName.c_str(), strValue.c_str()));
						}
					}
					if( !strClassName.empty() ) {
						StringHelper::TrimLeft(strAttribute);
						GlobalManager::Instance().AddClass(strClassName, strAttribute);
					}
				}
				else if( strClass == L"TextColor" ) {
					std::wstring colorName = node.attribute(L"name").as_string();
					std::wstring colorValue = node.attribute(L"value").as_string();
					if(!colorName.empty() && !colorValue.empty()) {
						ColorManager& colorManager = GlobalManager::Instance().Color();
						colorManager.AddColor(colorName, colorValue);
						if (colorName == L"default_font_color") {
							colorManager.SetDefaultTextColor(colorName);
						}
						else if (colorName == L"disabled_font_color") {
							colorManager.SetDefaultDisabledTextColor(colorName);
						}
					}
				}
			}
		}
		else if (strClass == L"Window")
		{
			for (pugi::xml_node node : root.children()) {
				strClass = node.name();
				if (strClass == L"Class") {
					std::wstring strClassName;
					std::wstring strAttribute;
					for (pugi::xml_attribute attr : node.attributes()) {
						strName = attr.name();
						strValue = attr.value();
						if (strName == L"name") {
							strClassName = strValue;
						}
						else {
							strAttribute.append(StringHelper::Printf(L" %s=\"%s\"",
												strName.c_str(), strValue.c_str()));
						}
					}
					if (!strClassName.empty()) {
						ASSERT(GlobalManager::Instance().GetClassAttributes(strClassName).empty());	//窗口中的Class不能与全局的重名
						StringHelper::TrimLeft(strAttribute);
						pWindow->AddClass(strClassName, strAttribute);
					}
				}
				else if (strClass == L"TextColor") {
					std::wstring strColorName;
					std::wstring strColor;
					for (pugi::xml_attribute attr : node.attributes()) {
						strName = attr.name();
						strValue = attr.value();
						if (strName == L"name") {
							strColorName = strValue;
						}
						else if (strName == L"value") {
							strColor = strValue;
						}
					}
					if (!strColorName.empty()) {
						pWindow->AddTextColor(strColorName, strColor);
					}
				}
				else if (strClass == L"Font") {
					//Window节点下，允许定义字体
					ParseFontXmlNode(node);
				}
			}
		}
	}

	for (pugi::xml_node node : root.children()) {
		std::wstring strClass = node.name();
		if (strClass == L"Image" || strClass == L"FontResource" || strClass == L"Font"
			|| strClass == L"Class" || strClass == L"TextColor") {

		}
		else {
			if (!pUserDefinedBox) {
				return (Box*)ParseXmlNode(root, pParent, pWindow);
			}
			else {
				ParseXmlNode(node, pUserDefinedBox, pWindow);
				int i = 0;
				for (pugi::xml_attribute attr : node.attributes()) {
					if (wcscmp(attr.name(), L"class") == 0) {
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
		if (strName == L"id")
		{
			strFontId = strValue;
		}
		else if (strName == L"name") {
			strFontName = strValue;
		}
		else if (strName == L"size") {
			size = wcstol(strValue.c_str(), nullptr, 10);
		}
		else if (strName == L"bold") {
			bold = (strValue == L"true");
		}
		else if (strName == L"underline") {
			underline = (strValue == L"true");
		}
		else if (strName == L"strikeout") {
			strikeout = (strValue == L"true");
		}
		else if (strName == L"italic") {
			italic = (strValue == L"true");
		}
		else if (strName == L"default") {
			isDefault = (strValue == L"true");
		}
	}
	if (!strFontName.empty() && !strFontId.empty()) {
		UiFont fontInfo;
		fontInfo.m_fontName = strFontName;
		fontInfo.m_fontSize = GlobalManager::Instance().Dpi().GetScaleInt(size);
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
		if( (strClass == L"Image") || 
			(strClass == L"Font")  ||
			(strClass == L"Class") || 
			(strClass == L"TextColor") ) {
				continue;
		}

        Control* pControl = nullptr;
        if (strClass == L"Include") {
			if (node.attributes().empty()) {
				continue;
			}
			pugi::xml_attribute countAttr = node.attribute(L"count");
			int nCount = countAttr.as_int();
			if (nCount <= 0) {
				//默认值设置为1，count这个属性参数为可选
				nCount = 1;
			}
			pugi::xml_attribute sourceAttr = node.attribute(L"src");
			std::wstring sourceValue = sourceAttr.as_string();
			if (sourceValue.empty()) {
				sourceAttr = node.attribute(L"source");
				sourceValue = sourceAttr.as_string();				
			}
			if (!sourceValue.empty()) {
				StringHelper::ReplaceAll(L"/", L"\\", sourceValue);
				if (!m_xmlFilePath.empty()) {
					//优先尝试在原XML文件相同目录加载
					size_t pos = m_xmlFilePath.find_last_of(L"\\/");
					if (pos != std::wstring::npos) {
						std::wstring filePath = m_xmlFilePath.substr(0, pos);
						filePath = StringHelper::JoinFilePath(filePath, sourceValue);
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
			pControl = CreateControlByClass(strClass);
			if (pControl == nullptr) {
				if ((strClass == L"Event") || 
					(strClass == L"BubbledEvent")) {
					bool bBubbled = (strClass == L"BubbledEvent");
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
				ASSERT(i == 0 || wcscmp(attr.name(), L"class") != 0);	//class必须是第一个属性
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
		ASSERT(!L"WindowBuilder::ParseRichTextXmlText load xml text failed!");
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
				textSlice.m_text = StringHelper::Trim(nodeValue);
			}
			//无节点名称，只读取文本内容, 不需要递归遍历子节点
			bParseChildren = false;
		}		
		else if (nodeName == L"a") {			
			textSlice.m_text = StringHelper::Trim(node.first_child().value());
			textSlice.m_linkUrl = StringHelper::Trim(node.attribute(L"href").as_string());
			//超级链接节点, 不需要递归遍历子节点
			bParseChildren = false;
		}
		else if (nodeName == L"b") {
			//粗体字
			textSlice.m_fontInfo.m_bBold = true;
		}
		else if (nodeName == L"i") {
			//斜体字
			textSlice.m_fontInfo.m_bItalic = true;
		}
		else if ((nodeName == L"del") || (nodeName == L"s") || (nodeName == L"strike")) {
			//删除字
			textSlice.m_fontInfo.m_bStrikeOut = true;
		}
		else if ( (nodeName == L"ins") || (nodeName == L"u") ){
			//下划线
			textSlice.m_fontInfo.m_bUnderline = true;
		}
		else if (nodeName == L"bgcolor") {
			//背景颜色
			textSlice.m_bgColor = StringHelper::Trim(node.attribute(L"color").as_string());
		}
		else if (nodeName == L"font") {
			//字体设置：文本颜色
			textSlice.m_textColor = node.attribute(L"color").as_string();
			textSlice.m_fontInfo.m_fontName = node.attribute(L"face").as_string();
			textSlice.m_fontInfo.m_fontSize = node.attribute(L"size").as_int();
			//字号进行DPI自适应
			GlobalManager::Instance().Dpi().ScaleInt(textSlice.m_fontInfo.m_fontSize);
		}
		else if (nodeName == L"br") {
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

Control* WindowBuilder::CreateControlByClass(const std::wstring& strControlClass)
{
	typedef std::function<Control* (void)> CreateControlFunction;
	static std::map<std::wstring, CreateControlFunction> createControlMap = 
	{ 
		{DUI_CTR_BOX,  []() { return new Box; }},
		{DUI_CTR_HBOX, []() { return new HBox; }},
		{DUI_CTR_VBOX, []() { return new VBox; }},
		{DUI_CTR_VTILE_BOX, []() { return new VTileBox; }},
		{DUI_CTR_HTILE_BOX, []() { return new HTileBox; }},
		{DUI_CTR_TABBOX, []() { return new TabBox; }},

		{DUI_CTR_SCROLLBOX, []() { return new ScrollBox; }},
		{DUI_CTR_HSCROLLBOX, []() { return new HScrollBox; }},
		{DUI_CTR_VSCROLLBOX, []() { return new VScrollBox; }},
		{DUI_CTR_HTILE_SCROLLBOX, []() { return new HTileScrollBox; }},
		{DUI_CTR_VTILE_SCROLLBOX, []() { return new VTileScrollBox; }},
	
		{DUI_CTR_LISTBOX_ITEM, []() { return new ListBoxItem; }},
		{DUI_CTR_HLISTBOX, []() { return new HListBox; }},
		{DUI_CTR_VLISTBOX, []() { return new VListBox; }},
		{DUI_CTR_HTILE_LISTBOX, []() { return new HTileListBox; }},
		{DUI_CTR_VTILE_LISTBOX, []() { return new VTileListBox; }},
		{DUI_CTR_LISTCTRL, []() { return new ListCtrl; }},
		{DUI_CTR_PROPERTY_GRID, []() { return new PropertyGrid; }},

		{DUI_CTR_VIRTUAL_HTILE_LISTBOX, []() { return new VirtualHTileListBox; }},
		{DUI_CTR_VIRTUAL_VTILE_LISTBOX, []() { return new VirtualVTileListBox; }},
		{DUI_CTR_VIRTUAL_VLISTBOX, []() { return new VirtualVListBox; }},
		{DUI_CTR_VIRTUAL_HLISTBOX, []() { return new VirtualHListBox; }},

		{DUI_CTR_CONTROL, []() { return new Control; }},
		{DUI_CTR_CONTROL_DRAGABLE, []() { return new ControlDragable; }},
		{DUI_CTR_SCROLLBAR, []() { return new ScrollBar; }},
		{DUI_CTR_LABEL, []() { return new Label; }},
		{DUI_CTR_LABELBOX, []() { return new LabelBox; }},
		{DUI_CTR_BUTTON, []() { return new Button; }},
		{DUI_CTR_BUTTONBOX, []() { return new ButtonBox; }},
		{DUI_CTR_OPTION, []() { return new Option; }},
		{DUI_CTR_OPTIONBOX, []() { return new OptionBox; }},
		{DUI_CTR_CHECKBOX, []() { return new CheckBox; }},
		{DUI_CTR_CHECKBOXBOX, []() { return new CheckBoxBox; }},
		{DUI_CTR_TREEVIEW, []() { return new TreeView; }},
		{DUI_CTR_TREENODE, []() { return new TreeNode; }},
		{DUI_CTR_COMBO, []() { return new Combo; }},
		{DUI_CTR_COMBO_BUTTON, []() { return new ComboButton; }},
		{DUI_CTR_FILTER_COMBO, []() { return new FilterCombo; }},
		{DUI_CTR_CHECK_COMBO, []() { return new CheckCombo; }},
		{DUI_CTR_SLIDER, []() { return new Slider; }},
		{DUI_CTR_PROGRESS, []() { return new Progress; }},
		{DUI_CTR_CIRCLEPROGRESS, []() { return new CircleProgress; }},
		{DUI_CTR_RICHTEXT, []() { return new RichText; }},
		{DUI_CTR_RICHEDIT, []() { return new RichEdit; }},
		{DUI_CTR_DATETIME, []() { return new DateTime; }},
		{DUI_CTR_COLOR_CONTROL, []() { return new ColorControl; }},
		{DUI_CTR_COLOR_SLIDER, []() { return new ColorSlider; }},
		{DUI_CTR_COLOR_PICKER_REGULAR, []() { return new ColorPickerRegular; }},
		{DUI_CTR_COLOR_PICKER_STANDARD, []() { return new ColorPickerStatard; }},
		{DUI_CTR_COLOR_PICKER_STANDARD_GRAY, []() { return new ColorPickerStatardGray; }},
		{DUI_CTR_COLOR_PICKER_CUSTOM, []() { return new ColorPickerCustom; }},
		{DUI_CTR_LINE, []() { return new Line; }},
		{DUI_CTR_IPADDRESS, []() { return new IPAddress; }},
		{DUI_CTR_HOTKEY, []() { return new HotKey; }},
		{DUI_CTR_HYPER_LINK, []() { return new HyperLink; }},
		{DUI_CTR_TAB_CTRL, []() { return new TabCtrl; }},
		{DUI_CTR_TAB_CTRL_ITEM, []() { return new TabCtrlItem; }},

		{DUI_CTR_SPLIT, []() { return new Split; }},
		{DUI_CTR_SPLITBOX, []() { return new SplitBox; }},
		{DUI_CTR_GROUP_BOX, []() { return new GroupBox; }},
		{DUI_CTR_GROUP_HBOX, []() { return new GroupHBox; }},
		{DUI_CTR_GROUP_VBOX, []() { return new GroupVBox; }},

		{DUI_CTR_BOX_DRAGABLE, []() { return new BoxDragable; }},
		{DUI_CTR_HBOX_DRAGABLE, []() { return new HBoxDragable; }},
		{DUI_CTR_VBOX_DRAGABLE, []() { return new VBoxDragable; }},
	};
	Control* pControl = nullptr;
	auto iter = createControlMap.find(strControlClass);
	if (iter != createControlMap.end()) {
		pControl = iter->second();
	}
	return pControl;
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
		ASSERT(i != 0 || strName == L"type");
		ASSERT(i != 1 || strName == L"receiver");
		ASSERT(i != 2 || strName == L"applyattribute");
		++i;
		if( strName == L"type" ) {
			strType = strValue;
		}
		else if( strName == L"receiver" ) {
			strReceiver = strValue;
		}
		else if( strName == L"applyattribute" ) {
			strApplyAttribute = strValue;
		}
	}

	auto typeList = StringHelper::Split(strType, L" ");
	auto receiverList = StringHelper::Split(strReceiver, L" ");
	for (auto itType = typeList.begin(); itType != typeList.end(); itType++) {
		if (receiverList.empty()) {
			receiverList.push_back(L"");
		}
		for (auto itReceiver = receiverList.begin(); itReceiver != receiverList.end(); itReceiver++) {
			EventType eventType = StringToEventType(*itType);
			if (eventType == EventType::kEventNone) {
				continue;
			}
			auto callback = nbase::Bind(&Control::OnApplyAttributeList, pParent, *itReceiver, strApplyAttribute, std::placeholders::_1);
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
