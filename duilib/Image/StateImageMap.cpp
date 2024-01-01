#include "StateImageMap.h"
#include "duilib/Core/Control.h"

namespace ui 
{
StateImageMap::StateImageMap():
	m_pControl(nullptr)
{
}

void StateImageMap::SetControl(Control* control)
{
	m_pControl = control;
	for (auto& it : m_stateImageMap) {
		it.second.SetControl(control);
	}
}

void StateImageMap::SetImageString(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath)
{
	StateImage& stateImage = m_stateImageMap[stateImageType];
	stateImage.SetControl(m_pControl);
	stateImage.SetImageString(stateType, strImagePath);
}

std::wstring StateImageMap::GetImageString(StateImageType stateImageType, ControlStateType stateType) const
{
	std::wstring imageString;
	auto iter = m_stateImageMap.find(stateImageType);
	if (iter != m_stateImageMap.end()) {
		imageString = iter->second.GetImageString(stateType);
	}
	return imageString;
}

bool StateImageMap::HasHotImage() const
{
	for (auto& it : m_stateImageMap) {
		if (it.second.HasHotImage()) {
			return true;
		}
	}
	return false;
}

bool StateImageMap::HasStateImages(void) const
{
	for (auto& it : m_stateImageMap) {
		if (it.second.HasImage()) {
			return true;
		}
	}
	return false;
}

bool StateImageMap::HasStateImage(StateImageType stateImageType) const
{
	bool bHasImage = false;
	auto iter = m_stateImageMap.find(stateImageType);
	if (iter != m_stateImageMap.end()) {
		bHasImage = iter->second.HasImage();
	}
	return bHasImage;
}

bool StateImageMap::PaintStateImage(IRender* pRender, 
								    StateImageType stateImageType, 
								    ControlStateType stateType, 
									const std::wstring& sImageModify,
	                                UiRect* pDestRect)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.PaintStateImage(pRender, stateType, sImageModify, pDestRect);
	}
	return false;
}

Image* StateImageMap::GetEstimateImage(StateImageType stateImageType) const
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.GetEstimateImage();
	}
	return nullptr;
}

Image* StateImageMap::GetStateImage(StateImageType stateImageType, ControlStateType stateType) const
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		const StateImage& stateImage = it->second;
		return stateImage.GetStateImage(stateType);
	}
	return nullptr;
}

void StateImageMap::GetAllImages(std::vector<Image*>& allImages) const
{
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
		iter->second.GetAllImages(allImages);
	}
}

void StateImageMap::ClearImageCache()
{
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
		iter->second.ClearImageCache();
	}
}

}