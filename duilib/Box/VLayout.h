#ifndef UI_BOX_VLAYOUT_H_
#define UI_BOX_VLAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** ��ֱ���֣������ӿؼ����������left����ͬ����ֱ������������
*/
class UILIB_API VLayout : public Layout
{
public:
	VLayout();

	/** ��������
	*/
	virtual LayoutType GetLayoutType() const override { return LayoutType::VLayout; }
	
	/** �����ڲ����пؼ���λ����Ϣ
	 * @param[in] items �ؼ��б�
	 * @param[in] rc ��ǰ����λ����Ϣ, �����ڱ߾࣬����������߾�
	 * @return �������к����պ��ӵĿ��Ⱥ͸߶���Ϣ
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

	/** �����ڲ��ӿؼ���С��������������С���������͵��ӿؼ������ԣ��������С����
	 * @param[in] items �ӿؼ��б�
	 * @param [in] szAvailable ���ô�С������������ÿؼ����ڱ߾࣬��������������ؼ�����߾�
	 * @return �������к����ղ��ֵĴ�С��Ϣ�����Ⱥ͸߶ȣ���
			   ����items���ӿؼ�����߾࣬����items���ӿؼ����ڱ߾ࣻ
			   ����Box�ؼ��������ڱ߾ࣻ
			   ������Box�ؼ���������߾ࣻ
			   ����ֵ�в������������͵��ӿؼ���С��
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable);
};

} // namespace ui

#endif // UI_BOX_VLAYOUT_H_