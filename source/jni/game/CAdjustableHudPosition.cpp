#include "../main.h"

#include "game.h"

CVector2D CAdjustableHudPosition::m_aPosition[E_HUD_ELEMENT::HUD_SIZE];

CVector2D CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT id)
{
	if (id < E_HUD_ELEMENT::HUD_HP || id >= E_HUD_ELEMENT::HUD_SIZE)
		return CVector2D();

	CVector2D ret = m_aPosition[id];
	return ret;
}

void CAdjustableHudPosition::SetElementPosition(E_HUD_ELEMENT id, int x, int y)
{
	if (id < E_HUD_ELEMENT::HUD_HP || id >= E_HUD_ELEMENT::HUD_SIZE)
		return;

	m_aPosition[id].X = x;
	m_aPosition[id].Y = y;

	if (x <= 5)
		m_aPosition[id].X = -1;

	if (y <= 5)
		m_aPosition[id].Y = -1;
}
