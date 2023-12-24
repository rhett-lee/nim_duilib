#ifndef UI_BOX_LAYOUT_H_
#define UI_BOX_LAYOUT_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <string>
#include <vector>

namespace ui 
{
class Box;
class Control;

/** 控件的布局类型
*/
enum class LayoutType
{
	FloatLayout,			//浮动布局
	HLayout,				//横向布局
	VLayout,				//纵向布局
	HTileLayout,			//横向瓦片布局
	VTileLayout,			//纵向瓦片布局
	VirtualHLayout,			//虚表横向布局
	VirtualVLayout,			//虚表纵向布局
	VirtualHTileLayout,		//虚表横向瓦片布局
	VirtualVTileLayout,		//虚表纵向瓦片布局
	ListCtrlReportLayout	//ListCtrl控件的Report模式布局
};

/** 控件布局(Float方式布局)：
*    各个子控件顶点坐标(left,top)都相同，各个控件堆叠排列（相当于都看成是Float控件）
*    子控件可用指定横向对齐方式和纵向对齐方式，排列的时候会按照子控件指定的对齐方式排列
*/
class UILIB_API Layout
{
public:
	Layout();
	Layout(const Layout& r) = delete;
	Layout& operator=(const Layout& r) = delete;
	virtual ~Layout() = default;

	/** 布局类型
	*/
	virtual LayoutType GetLayoutType() const { return LayoutType::FloatLayout; }

	/** 是否为纵向布局
	*/
	virtual bool IsVLayout() const 
	{
		LayoutType type = GetLayoutType();
		return (type == LayoutType::VLayout) ||
			   (type == LayoutType::VTileLayout) ||
			   (type == LayoutType::VirtualVLayout) ||
			   (type == LayoutType::VirtualVTileLayout) ||
			   (type == LayoutType::ListCtrlReportLayout);
	}

	/** 是否为横向布局
	*/
	virtual bool IsHLayout() const 
	{
		LayoutType type = GetLayoutType();
		return (type == LayoutType::HLayout) ||
			   (type == LayoutType::HTileLayout) ||
			   (type == LayoutType::VirtualHLayout) ||
			   (type == LayoutType::VirtualHTileLayout);
	}

	/** 是否为瓦片布局(可能为纵向或者横向布局)
	*/
	virtual bool IsTileLayout() const
	{
		LayoutType type = GetLayoutType();
		return (type == LayoutType::VTileLayout) ||
			   (type == LayoutType::HTileLayout) ||
			   (type == LayoutType::VirtualVTileLayout) ||
			   (type == LayoutType::VirtualHTileLayout);
	}

	/** 设置所有者容器接口
	 */
	void SetOwner(Box* pOwner);

	/** 获取所有者容器接口
	*/
	Box* GetOwner() const { return m_pOwner; }

	/** 设置浮动状态下的坐标信息
	 * @param[in] pControl 控件句柄
	 * @param[in] rcContainer 要设置的矩形区域，包含内边距，包含外边距
	 * @return 返回控件最终的大小信息（宽度和高度）
	 */
	static UiSize64 SetFloatPos(Control* pControl, const UiRect& rcContainer);

	/** 设置布局属性
	 * @param[in] strName 要设置的属性名
	 * @param[in] strValue 要设置的属性值
	 * @return true 设置成功，false 属性不存在
	 */
	virtual bool SetAttribute(const std::wstring& strName, const std::wstring& strValue);

	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
	 * @return 返回排列后最终盒子的宽度和高度信息，包含Owner Box的内边距，不包含外边距
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc);

	/** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
	 * @param[in] items 子控件列表
	 * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
	 * @return 返回排列后最终布局的大小信息（宽度和高度）；
	           包含items中子控件的外边距，包含items中子控件的内边距；
			   包含Box控件本身的内边距；
			   不包含Box控件本身的外边距；
	           返回值中不包含拉伸类型的子控件大小。
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable);

public:
	/** 获取子控件之间的额外边距（X轴方向）
	 * @return 返回额外间距的数值
	 */
	int32_t GetChildMarginX() const { return m_iChildMarginX; }

	/** 获取子控件之间的额外边距（Y轴方向）
	 * @return 返回额外间距的数值
	 */
	int32_t GetChildMarginY() const { return m_iChildMarginY; }

	/** 设置子控件之间的额外边距（X轴方向）
	 * @param[in] iMarginX 要设置的边距数值
	 */
	void SetChildMarginX(int32_t iMarginX);

	/** 设置子控件之间的额外边距（Y轴方向）
	 * @param[in] iMarginY 要设置的边距数值
	 */
	void SetChildMarginY(int32_t iMarginY);

	/** 设置子控件之间的额外边距（X轴方向和Y轴方向，均设置为同一个值）
	 * @param[in] iMargin 要设置的边距数值
	 */
	void SetChildMargin(int32_t iMargin);

	/** 将区域去掉内边距, 并确保rc区域有效
	*/
	void DeflatePadding(UiRect& rc) const;

protected:
	/** 检查配置的宽和高是否正确, 如果发现错误，给予断言
	*/
	void CheckConfig(const std::vector<Control*>& items);

	/** 按照控件指定的对齐方式，计算控件的布局位置
	* @param [in] pControl 控件的接口
	* @param [in] rcContainer 目标容器的矩形，包含控件的外边距和内边距
	* @param [in] childSize 控件pControl的大小（宽和高）, 包含内边距，内部不会再计算控件的大小
	* @return 返回控件的位置和大小，不包含外边距，包含内边距
	          这个返回值，可用pControl->SetPos(rect)来调整控件位置;
	*/
	static UiRect GetFloatPos(Control* pControl, UiRect rcContainer, UiSize childSize);

private:
	//所属Box对象
	Box* m_pOwner;

	//子控件之间的额外边距: X 轴方向
	uint16_t m_iChildMarginX;

	//子控件之间的额外边距: Y 轴方向
	uint16_t m_iChildMarginY;
};

} // namespace ui

#endif // UI_BOX_LAYOUT_H_
