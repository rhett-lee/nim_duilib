#ifndef UI_ANIMATION_ANIMATIONMANAGER_H_
#define UI_ANIMATION_ANIMATIONMANAGER_H_

#pragma once

#include "duilib/Animation/AnimationPlayer.h"
#include <map>
#include <memory>

namespace ui 
{
class Control;

/** 控件的动画播放管理器
*/
class UILIB_API AnimationManager
{
public:
	AnimationManager();
	AnimationManager(const AnimationManager& r) = delete;
	AnimationManager& operator=(const AnimationManager& r) = delete;

	/** 初始化
	* @param [in] pControl 动画关联控件的接口
	*/
	void Init(Control* pControl)	{ m_pControl = pControl;	}

	/** 获取指定动画类型的播放接口
	* @param [in] animationType 播放类型
	* @return 返回该类型的动画播放接口，如果匹配不到，返回nullptr
	*/
	AnimationPlayer* GetAnimationPlayer(AnimationType animationType) const;

	/** 设置或清除播放动画，对应动画类型为：kAnimationAlpha
	* @param [in] bFadeHot true表示设置动画，false表示清除动画
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeHot(bool bFadeHot);

	/** 设置或清除播放动画，对应动画类型为：kAnimationHot
	* @param [in] bFadeVisible true表示设置动画，false表示清除动画
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeAlpha(bool bFadeVisible);

	/** 设置或清除播放动画，对应动画类型为：kAnimationWidth
	* @param [in] bFadeWidth true表示设置动画，false表示清除动画
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeWidth(bool bFadeWidth);

	/** 设置或清除播放动画，对应动画类型为：kAnimationHeight
	* @param [in] bFadeHeight true表示设置动画，false表示清除动画
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeHeight(bool bFadeHeight);

	/** 设置或清除播放动画
	* @param [in] bFade true表示设置动画，false表示清除动画
	* @param [in] bIsFromRight true表示从右侧动画（kAnimationInoutXFromRight），false表示从左侧动画（kAnimationInoutXFromLeft）
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeInOutX(bool bFade, bool bIsFromRight);

	/** 设置或清除播放动画
	* @param [in] bFade true表示设置动画，false表示清除动画
	* @param [in] bIsFromBottom true表示从下侧动画（kAnimationInoutYFromBottom），false表示从上侧动画（kAnimationInoutYFromTop）
	* @return 设置时返回动画播放接口，清除时返回nullptr
	*/
	AnimationPlayer* SetFadeInOutY(bool bFade, bool bIsFromBottom);

	/** 按设置的动画，显示控件
	*/
	void Appear();

	/** 按设置的动画，隐藏控件
	*/
	void Disappear();

	/** 鼠标移入时，按设置执行动画
	*/
	void MouseEnter();

	/** 鼠标移出时，按设置执行动画
	*/
	void MouseLeave();

	/** 清理所有动画资源
	*/
	void Clear(Control* control);

private:
	/** 动画所属控件的接口
	*/
	Control* m_pControl;

	/** 每个动画类型下的动画播放接口
	*/
	std::map<AnimationType, std::unique_ptr<AnimationPlayer>> m_animationMap;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONMANAGER_H_
