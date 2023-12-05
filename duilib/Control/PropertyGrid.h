#ifndef UI_CONTROL_PROPERTY_GRID_H_
#define UI_CONTROL_PROPERTY_GRID_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/TreeView.h"

namespace ui
{
/** 属性表控件
*/
class PropertyGridGroup;
class PropertyGridProperty;
class PropertyGrid : public VBox
{
public:
	PropertyGrid();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
	/** 设置是否显示表头
	*/
	void SetEnableHeaderCtrl(bool bEnable,
							 const std::wstring& sLeftColumn = L"", 
							 const std::wstring& sRightColumn = L"");
	/** 判断当前是否显示表头
	*/
	bool IsEnableHeaderCtrl() const { return m_bHeaderCtrl; }

	/** 获取Header接口
	*/
	Control* GetHeaderCtrl() const { return m_pHeaderCtrl; }

	/** 获取Header中的左侧一列
	*/
	Label* GetHeaderLeft() const { return m_pHeaderLeft; }

	/** 获取Header中的右侧一列
	*/
	Label* GetHeaderRight() const { return m_pHeaderRight; }

	/** 获取Header中的分割条
	*/
	Split* GetHeaderSplit() const { return m_pHeaderSplit; }

public:
	/** 增加一个分组
	* @param [in] groupName 分组的名称
	* @param [in] description 分组的描述信息
	* @return 返回该分组的接口，可用于添加属性
	*/
	PropertyGridGroup* AddGroup(const std::wstring& groupName, 
							    const std::wstring& description = L"");

	/** 获取所有的分组
	* @param [out] groups 返回当前所有的分组列表
	*/
	void GetGroups(std::vector<PropertyGridGroup*>& groups) const;

	/** 删除分组
	* @param [in] pGroup 待删除的分组
	*/
	bool RemoveGroup(PropertyGridGroup* pGroup);

	/** 删除所有分组
	*/
	void RemoveAllGroups();

	/** 增加一个属性
	* @param [in] pGroup 该属性所属的分组
	* @param [in] propertyName 属性的名称
	* @param [in] propertyValue 属性的值
	* @param [in] description 属性的描述信息
	* @return 返回该属性的接口
	*/
	PropertyGridProperty* AddProperty(PropertyGridGroup* pGroup,
									  const std::wstring& propertyName, 
									  const std::wstring& propertyValue,
									  const std::wstring& description = L"");

	/** 设置左侧一列的宽度
	* @param [in] nLeftColumnWidth 左侧一列的宽度
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
	*/
	void SetLeftColumnWidth(int32_t nLeftColumnWidth, bool bNeedDpiScale);

	/** 获取左侧一列的宽度值
	*/
	int32_t GetLeftColumnWidth() const;

public:
	/** 设置是否显示描述区域
	*/
	void SetEnableDescriptionArea(bool bEnable);

	/** 判断是否显示描述区域
	*/
	bool IsEnableDescriptionArea() const { return m_bDescriptionArea; }

	/** 设置描述区域的高度值
	* @param [in] nHeight 高度值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
	*/
	void SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale);

	/** 获取当前描述区域的高度值
	*/
	int32_t GetDescriptionAreaHeight() const;

	/** 获取描述控件的接口
	*/
	RichText* GetDescriptionArea() const { return m_pDescriptionArea; }

	/** 描述控件的分割条接口
	*/
	Split* GetDescriptionAreaSplit() const { return m_pDescriptionAreaSplit; }

public:
	/** 获取属性表的树控件接口（用于管理数据）
	*/
	TreeView* GetTreeView() const { return m_pTreeView; }

	/** 横向网格线的宽度
	* @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
	* @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
	*/
	void SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetRowGridLineWidth() const;

	/** 横向网格线的颜色
	* @param [in] color 横向网格线的颜色
	*/
	void SetRowGridLineColor(const std::wstring& color);
	std::wstring GetRowGridLineColor() const;

	/** 纵向网格线的宽度
	* @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
	* @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
	*/
	void SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale);
	int32_t GetColumnGridLineWidth() const;

	/** 纵向网格线的颜色
	* @param [in] color 纵向网格线的颜色
	*/
	void SetColumnGridLineColor(const std::wstring& color);
	std::wstring GetColumnGridLineColor() const;

	/** 表头的Class
	*/
	void SetHeaderClass(const std::wstring& headerClass);
	std::wstring GetHeaderClass() const;

	/** 分组的Class
	*/
	void SetGroupClass(const std::wstring& groupClass);
	std::wstring GetGroupClass() const;

	/** 属性的Class
	*/
	void SetPropertyClass(const std::wstring& propertyClass);
	std::wstring GetPropertyClass() const;

protected:
	/** 初始化函数
	 */
	virtual void DoInit() override;

	/** 绘制子控件
	*/
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

	/** 绘制网格线
	*/
	void PaintGridLines(IRender* pRender);

	/** 拖动列表头改变列宽的事件响应函数
	*/
	void OnHeaderColumnResized();

	/** 调整一个属性节点的列宽
	*/
	void ResizePropertyColumn(TreeNode* pPropertyNode, int32_t nLeftColumnWidth);

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 配置XML文件
	*/
	UiString m_configXml;

private:
	/** 是否显示Header
	*/
	bool m_bHeaderCtrl;

	/** Header接口
	*/
	Control* m_pHeaderCtrl;

	/** Header中的左侧一列
	*/
	Label* m_pHeaderLeft;

	/** Header中的右侧一列
	*/
	Label* m_pHeaderRight;

	/** Header中的分割条
	*/
	Split* m_pHeaderSplit;

	/** 左侧一列的宽度
	*/
	int32_t m_nLeftColumnWidth;

private:
	/** 描述控件的分割条
	*/
	Split* m_pDescriptionAreaSplit;

	/** 描述控件
	*/
	RichText* m_pDescriptionArea;

	/** 是否显示描述区域
	*/
	bool m_bDescriptionArea;

private:
	/** 属性表的树控件接口
	*/
	TreeView* m_pTreeView;

	/** 表头的Class
	*/
	UiString m_headerClass;

	/** 分组的Class
	*/
	UiString m_groupClass;

	/** 属性的Class
	*/
	UiString m_propertyClass;

private:
	/** 横向网格线的宽度
	*/
	int32_t m_nRowGridLineWidth;

	/** 横向网格线的颜色
	*/
	UiString m_rowGridLineColor;

	/** 纵向网格线的宽度
	*/
	int32_t m_nColumnGridLineWidth;

	/** 纵向网格线的颜色
	*/
	UiString m_columnGridLineColor;
};

/** 属性表的分组, 基本结构
*   <PropertyGridGroup>
*		<HBox>
*			<LabelBox/>
*		</HBox>
*   </PropertyGridGroup>
*/
class PropertyGridGroup : public TreeNode
{
public:
	/** 构造一个组
	@param [in] groupName 组的名称
	@param [in] description 组的描述信息
	*/
	explicit PropertyGridGroup(const std::wstring& groupName, 
							   const std::wstring& description = L"");

public:
	/** 获取属性名称
	*/
	std::wstring GetGroupName() const { return m_groupName.c_str(); }

	/** 获取组的描述信息
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** 获取属性名称的显示控件
	*/
	LabelBox* GetLabelBox() const { return m_pLabelBox; }

	/** 获取分组下的所有属性
	* @param [out] properties 返回当前所有的分组列表
	*/
	void GetProperties(std::vector<PropertyGridProperty*>& properties) const;

	/** 删除该分组下的属性
	* @param [in] pProperty 需要删除的属性接口
	*/
	bool RemoveProperty(PropertyGridProperty* pProperty);

	/** 删除该分组下的所有属性
	*/
	void RemoveAllProperties();

protected:
	/** 初始化函数
	 */
	virtual void DoInit() override;

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 分组的名称
	*/
	UiString m_groupName;

	/** 组的描述信息
	*/
	UiString m_description;

	/** 属性名称的显示控件
	*/
	LabelBox* m_pLabelBox;
};

/** 属性表的属性, 基本结构
*   <PropertyGridProperty>
*		<HBox>
*			<LabelBox/>
*			<LabelBox/>
*		</HBox>
*   </PropertyGridProperty>
*/
class PropertyGridProperty: public TreeNode
{
public:
	/** 构造一个属性
	@param [in] propertyName 属性的名称
	@param [in] propertyValue 属性的值
	@param [in] description 属性的描述信息
	*/
	PropertyGridProperty(const std::wstring& propertyName, 
					     const std::wstring& propertyValue,
					     const std::wstring& description = L"");

public:
	/** 获取属性名称
	*/
	std::wstring GetPropertyName() const { return m_propertyName.c_str(); }

	/** 获取属性值
	*/
	std::wstring GetPropertyValue() const { return m_propertyValue.c_str(); }

	/** 获取属性的描述信息
	*/
	std::wstring GetDescriptiion() const { return m_description.c_str(); }

	/** 获取属性名称的显示控件
	*/
	LabelBox* GetLabelBoxLeft() const { return m_pLabelBoxLeft; }

	/** 获取属性值的显示控件
	*/
	LabelBox* GetLabelBoxRight() const { return m_pLabelBoxRight; }

protected:
	/** 初始化函数
	 */
	virtual void DoInit() override;

private:
	/** 是否已经完成初始化
	*/
	bool m_bInited;

	/** 属性的名称
	*/
	UiString m_propertyName;

	/** 属性的值
	*/
	UiString m_propertyValue;

	/** 属性的描述信息
	*/
	UiString m_description;

	/** 属性名称的显示控件
	*/
	LabelBox* m_pLabelBoxLeft;

	/** 属性值的显示控件
	*/
	LabelBox* m_pLabelBoxRight;
};

}//namespace ui

#endif //UI_CONTROL_PROPERTY_GRID_H_
