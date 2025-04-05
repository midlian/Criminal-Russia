#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../dialog.h"

extern CDialogWindow* pDialogWindow;
extern CGame *pGame;

CGangZonePool::CGangZonePool()
{
	for(uint16_t wZone = 0; wZone < ARRAY_SIZE(m_pGangZone); wZone++)
	{
		m_pGangZone[wZone] = nullptr;
		m_bSlotState[wZone] = false;
	}
}

CGangZonePool::~CGangZonePool()
{
	for(uint16_t wZone = 0; wZone < ARRAY_SIZE(m_pGangZone); wZone++)
	{
		SAFE_DELETE(m_pGangZone[wZone]);
	}
}

void CGangZonePool::New(uint16_t wZone, float fMinX, float fMinY, float fMaxX, float fMaxY, uint32_t dwColor)
{
	if(!ARRAY_VALIDATE(m_pGangZone, wZone)) {
		SAMP_DUMP("GangZonePool", "Warning: zone %u has invalid ID (CGangZonePool::New)", wZone);
		return;
	}

	if(m_pGangZone[wZone])
	{
		delete m_pGangZone[wZone];
		m_pGangZone[wZone] = nullptr;
		m_bSlotState[wZone] = false;
	}

	auto *pGangZone = new GANG_ZONE;
	if(pGangZone)
	{
		pGangZone->fPos[0] = fMinX;
		pGangZone->fPos[1] = fMaxY;
		pGangZone->fPos[2] = fMaxX;
		pGangZone->fPos[3] = fMinY;
		pGangZone->dwColor = dwColor;
		pGangZone->dwAltColor = dwColor;

		m_pGangZone[wZone] = pGangZone;
		m_bSlotState[wZone] = true;
	}
}

void CGangZonePool::Delete(uint16_t wZone)
{
	if(!ARRAY_VALIDATE(m_pGangZone, wZone)) {
		SAMP_DUMP("GangZonePool", "Warning: zone %u has invalid ID (CGangZonePool::Delete)", wZone);
		return;
	}

	SAFE_DELETE(m_pGangZone[wZone]);
	m_bSlotState[wZone] = false;
}

void CGangZonePool::Draw()
{
	static uint32_t dwLastTick = 0;
	static uint8_t alt = 0;

	uint32_t dwTick = GetTickCount();

	bool bDraw = true;

	if(dwTick - dwLastTick >= 500)
	{
		alt = ~alt;
		dwLastTick = dwTick;
	}

	if (pDialogWindow)
	{
		if (pDialogWindow->m_bRendered)
			bDraw = false;
	}

	if (!bDraw)
		return;

	for(uint16_t wZone = 0; wZone < MAX_GANG_ZONES; wZone++)
	{
		if(m_bSlotState[wZone])
			pGame->DrawGangZone(m_pGangZone[wZone]->fPos, alt ? m_pGangZone[wZone]->dwAltColor : m_pGangZone[wZone]->dwColor);
	}
}

void CGangZonePool::Flash(uint16_t wZone, uint32_t dwColor)
{
	if(!ARRAY_VALIDATE(m_pGangZone, wZone)) {
		SAMP_DUMP("GangZonePool", "Warning: zone %u has invalid ID (CGangZonePool::Flash)", wZone);
		return;
	}

	if(m_pGangZone[wZone]) {
		m_pGangZone[wZone]->dwAltColor = dwColor;
	}
}

void CGangZonePool::StopFlash(uint16_t wZone)
{
	if(!ARRAY_VALIDATE(m_pGangZone, wZone)) {
		SAMP_DUMP("GangZonePool", "Warning: zone %u has invalid ID (CGangZonePool::StopFlash)", wZone);
		return;
	}

	if(m_pGangZone[wZone]) {
		m_pGangZone[wZone]->dwAltColor = m_pGangZone[wZone]->dwColor;
	}
}