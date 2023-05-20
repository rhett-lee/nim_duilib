#include "IRender.h"

namespace ui 
{

AutoClip::AutoClip(IRender* pRender, const UiRect& rc, bool bClip)
{
	m_pRender = NULL;
	m_bClip = false;

	if (bClip)
	{
		m_bClip = bClip;
		ASSERT(pRender);
		m_pRender = pRender;
		m_pRender->SetClip(rc);
	}
}

AutoClip::AutoClip(IRender* pRender, const UiRect& rcRound, int width, int height, bool bClip)
{
	m_pRender = NULL;
	m_bClip = false;

	if (bClip)
	{
		m_bClip = bClip;
		ASSERT(pRender);
		m_pRender = pRender;
		m_pRender->SetRoundClip(rcRound, width, height);
	}
}

AutoClip::~AutoClip()
{
	if (m_bClip && m_pRender)
		m_pRender->ClearClip();
}

}