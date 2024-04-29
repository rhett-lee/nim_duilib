#include "AnimationManager.h"
#include "duilib/Core/Control.h"

namespace ui 
{

AnimationManager::AnimationManager() :
	m_pControl(nullptr)
{
}

AnimationPlayer* AnimationManager::GetAnimationPlayer(AnimationType animationType) const
{
	auto it = m_animationMap.find(animationType);
	if (it != m_animationMap.end()) {
		return it->second.get();
	}
	else {
		return nullptr;
	}
}

AnimationPlayer* AnimationManager::SetFadeHot(bool bFadeHot)
{
	AnimationPlayer* animationArgs = nullptr;
	if (bFadeHot) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetAnimationType(AnimationType::kAnimationHot);
		animationArgs->SetStartValue(0);
		animationArgs->SetEndValue(255);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedDownRatio(0.7);
		animationArgs->SetTotalMillSeconds(50);//控件处于焦点状态时，动画效果的总时长
		auto playCallback = nbase::Bind(&Control::SetHotAlpha, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);
		m_animationMap[AnimationType::kAnimationHot].reset(animationArgs);
	}
	else {
		m_animationMap.erase(AnimationType::kAnimationHot);
	}

	return animationArgs;
}

AnimationPlayer* AnimationManager::SetFadeAlpha(bool bFadeVisible)
{
	AnimationPlayer* animationArgs = nullptr;
	if (bFadeVisible) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetAnimationType(AnimationType::kAnimationAlpha);
		animationArgs->SetStartValue(0);
		animationArgs->SetEndValue(255);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedDownRatio(0.7);
		animationArgs->SetTotalMillSeconds(250);
		auto playCallback = nbase::Bind(&Control::SetAlpha, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);
		m_animationMap[AnimationType::kAnimationAlpha].reset(animationArgs);
		m_pControl->SetAlpha(0);
	}
	else {
		m_animationMap.erase(AnimationType::kAnimationAlpha);
		m_pControl->SetAlpha(255);
	}

	return animationArgs;
}

AnimationPlayer* AnimationManager::SetFadeWidth(bool bFadeWidth)
{
	AnimationPlayer* animationArgs = nullptr;
	int32_t cx = 0;
	if (bFadeWidth) {
		UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
		cx = estSize.cx.GetInt32();
		ASSERT(cx > 0);
	}
	if (bFadeWidth && (cx > 0)) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetAnimationType(AnimationType::kAnimationWidth);
		animationArgs->SetStartValue(0);
		animationArgs->SetEndValue(cx);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedUpfactorA(0.00084);
		animationArgs->SetSpeedDownRatio(0.7);
		auto playCallback = nbase::Bind(&Control::SetFixedWidth64, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);
		m_animationMap[AnimationType::kAnimationWidth].reset(animationArgs);
	}
	else {
		m_animationMap.erase(AnimationType::kAnimationWidth);
	}

	return animationArgs;
}

AnimationPlayer* AnimationManager::SetFadeHeight(bool bFadeHeight)
{
	AnimationPlayer* animationArgs = nullptr;
	int32_t cy = 0;
	if (bFadeHeight) {
		UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
		cy = estSize.cy.GetInt32();
		ASSERT(cy > 0);
	}
	if (bFadeHeight && (cy > 0)) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetAnimationType(AnimationType::kAnimationHeight);
		animationArgs->SetStartValue(0);
		animationArgs->SetEndValue(cy);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedUpfactorA(0.00084);
		animationArgs->SetSpeedDownRatio(0.7);
		auto playCallback = nbase::Bind(&Control::SetFixedHeight64, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);
		m_animationMap[AnimationType::kAnimationHeight].reset(animationArgs);
	}
	else {
		m_animationMap.erase(AnimationType::kAnimationHeight);
	}

	return animationArgs;
}

AnimationPlayer* AnimationManager::SetFadeInOutX(bool bFade, bool bIsFromRight)
{
	AnimationPlayer* animationArgs = nullptr;
	int32_t cx = 0;
	if (bFade) {
		UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
		cx = estSize.cx.GetInt32();
		if (cx <= 0) {
			cx = 100;
		}
	}
	if (bFade) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetEndValue(0);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedUpfactorA(0.006);
		animationArgs->SetSpeedDownRatio(0.7);
		auto playCallback = nbase::Bind(&Control::SetRenderOffsetX, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);

		if (bIsFromRight) {
			animationArgs->SetStartValue(-cx);
			animationArgs->SetAnimationType(AnimationType::kAnimationInoutXFromRight);
			m_animationMap[AnimationType::kAnimationInoutXFromRight].reset(animationArgs);
		}
		else {
			animationArgs->SetStartValue(cx);
			animationArgs->SetAnimationType(AnimationType::kAnimationInoutXFromLeft);
			m_animationMap[AnimationType::kAnimationInoutXFromLeft].reset(animationArgs);
		}
	}
	else{
		if (bIsFromRight) {
			m_animationMap.erase(AnimationType::kAnimationInoutXFromRight);
		}
		else {
			m_animationMap.erase(AnimationType::kAnimationInoutXFromLeft);
		}
	}

	return animationArgs;
}

AnimationPlayer* AnimationManager::SetFadeInOutY(bool bFade, bool bIsFromBottom)
{
	AnimationPlayer* animationArgs = nullptr;
	int32_t cy = 0;
	if (bFade) {
		UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
		cy = estSize.cy.GetInt32();
		if (cy <= 0) {
			cy = 100;
		}
	}
	if (bFade) {
		animationArgs = new AnimationPlayer();
		animationArgs->SetEndValue(0);
		animationArgs->SetSpeedUpRatio(0.3);
		animationArgs->SetSpeedUpfactorA(0.006);
		animationArgs->SetSpeedDownRatio(0.7);
		auto playCallback = nbase::Bind(&Control::SetRenderOffsetY, m_pControl, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);

		if (bIsFromBottom) {
			animationArgs->SetStartValue(-cy);
			animationArgs->SetAnimationType(AnimationType::kAnimationInoutYFromBottom);
			m_animationMap[AnimationType::kAnimationInoutYFromBottom].reset(animationArgs);
		}
		else {
			animationArgs->SetStartValue(cy);
			animationArgs->SetAnimationType(AnimationType::kAnimationInoutYFromTop);
			m_animationMap[AnimationType::kAnimationInoutYFromTop].reset(animationArgs);
		}
	}
	else{
		if (bIsFromBottom) {
			m_animationMap.erase(AnimationType::kAnimationInoutYFromBottom);
		}
		else {
			m_animationMap.erase(AnimationType::kAnimationInoutYFromTop);
		}
	}

	return animationArgs;
}

void AnimationManager::Appear()
{
	ASSERT(m_pControl != nullptr);
	if (m_pControl != nullptr) {
		m_pControl->SetVisible(true);
	}	
	if (GetAnimationPlayer(AnimationType::kAnimationAlpha)) {
		m_animationMap[AnimationType::kAnimationAlpha]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationAlpha]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationWidth)) {
		m_animationMap[AnimationType::kAnimationWidth]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationWidth]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationHeight)) {
		m_animationMap[AnimationType::kAnimationHeight]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationHeight]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutXFromLeft)) {
		m_animationMap[AnimationType::kAnimationInoutXFromLeft]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationInoutXFromLeft]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutXFromRight)) {
		m_animationMap[AnimationType::kAnimationInoutXFromRight]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationInoutXFromRight]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutYFromTop)) {
		m_animationMap[AnimationType::kAnimationInoutYFromTop]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationInoutYFromTop]->Continue();
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutYFromBottom)) {
		m_animationMap[AnimationType::kAnimationInoutYFromBottom]->SetCompleteCallback(CompleteCallback());
		m_animationMap[AnimationType::kAnimationInoutYFromBottom]->Continue();
	}
}

void AnimationManager::Disappear()
{
	bool handled = false;
	ASSERT(m_pControl != nullptr);
	if (m_pControl == nullptr) {
		return;
	}

	CompleteCallback completeCallback = nbase::Bind(&Control::SetVisible, m_pControl, false);
	if (GetAnimationPlayer(AnimationType::kAnimationAlpha)) {
		m_animationMap[AnimationType::kAnimationAlpha]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationAlpha]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationWidth)) {
		m_animationMap[AnimationType::kAnimationWidth]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationWidth]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationHeight)) {
		m_animationMap[AnimationType::kAnimationHeight]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationHeight]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutXFromLeft)) {
		m_animationMap[AnimationType::kAnimationInoutXFromLeft]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationInoutXFromLeft]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutXFromRight)) {
		m_animationMap[AnimationType::kAnimationInoutXFromRight]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationInoutXFromRight]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutYFromTop)) {
		m_animationMap[AnimationType::kAnimationInoutYFromTop]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationInoutYFromTop]->ReverseContinue();
		handled = true;
	}
	if (GetAnimationPlayer(AnimationType::kAnimationInoutYFromBottom)) {
		m_animationMap[AnimationType::kAnimationInoutYFromBottom]->SetCompleteCallback(completeCallback);
		m_animationMap[AnimationType::kAnimationInoutYFromBottom]->ReverseContinue();
		handled = true;
	}

	if (!handled) {
		m_pControl->SetVisible(false);
	}
}

void AnimationManager::MouseEnter()
{
	AnimationPlayer* pPlayer = GetAnimationPlayer(AnimationType::kAnimationHot);
	if (pPlayer) {
		pPlayer->Continue();
	}
}

void AnimationManager::MouseLeave()
{
	AnimationPlayer* pPlayer = GetAnimationPlayer(AnimationType::kAnimationHot);
	if (pPlayer) {
		pPlayer->ReverseContinue();
	}
}

void AnimationManager::Clear(Control* control)
{
	if (m_pControl != nullptr) {
		ASSERT_UNUSED_VARIABLE(control == m_pControl);
	}

	for (auto& iter : m_animationMap) {
		if (iter.second != nullptr) {
			iter.second->Clear();
		}
	}

	m_pControl = nullptr;
	m_animationMap.clear();
}

}