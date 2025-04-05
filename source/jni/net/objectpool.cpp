#include "../main.h"
#include "../game/game.h"
#include "netgame.h"

extern CGame *pGame;

CObjectPool::CObjectPool()
{
	for(uint16_t ObjectID = 0; ObjectID < ARRAY_SIZE(m_pObjects); ObjectID++)
	{
		m_bObjectSlotState[ObjectID] = false;
		m_pObjects[ObjectID] = (CObject *)0;
	}
}

CObjectPool::~CObjectPool()
{
	for(uint16_t i = 0; i < ARRAY_SIZE(m_pObjects); i++)
	{
		Delete(i);
	}
}

bool CObjectPool::Delete(uint16_t ObjectID)
{
	if (!ARRAY_VALIDATE(m_pObjects, ObjectID)) { 
		SAMP_DUMP("ObjectPool", "Warning: object %u has invalid ID (CObjectPool::Delete)", ObjectID);
		return false;
	}

	if(!GetSlotState(ObjectID) || !m_pObjects[ObjectID]) {
		return false;
	}

	m_bObjectSlotState[ObjectID] = false;
		
	if(m_pObjects[ObjectID]) {
		delete m_pObjects[ObjectID];
		m_pObjects[ObjectID] = (CObject *)0;
	}

	return true;
}

bool CObjectPool::New(uint16_t ObjectID, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDistance)
{
	if (!ARRAY_VALIDATE(m_pObjects, ObjectID)) { 
		SAMP_DUMP("ObjectPool", "Warning: object %u has invalid ID (CObjectPool::New)", ObjectID);
		return false;
	}

	if(m_pObjects[ObjectID] != (CObject *)0) {
		Delete(ObjectID);
	}

	m_pObjects[ObjectID] = pGame->NewObject(iModel, vecPos.X, vecPos.Y, vecPos.Z, vecRot, fDrawDistance);

	if(m_pObjects[ObjectID])
	{
		m_bObjectSlotState[ObjectID] = true;

		return true;
	}

	return false;
}

CObject *CObjectPool::GetObjectFromGtaPtr(ENTITY_TYPE *pGtaObject)
{
	uint16_t x=1;

	while(x!=ARRAY_SIZE(m_pObjects))
	{
		if(m_pObjects[x])
			if(pGtaObject == m_pObjects[x]->m_pEntity) return m_pObjects[x];
		
		x++;
	}

	return 0;
}

uint16_t CObjectPool::FindIDFromGtaPtr(ENTITY_TYPE* pGtaObject)
{
	uint16_t x=1;

	while(x!=ARRAY_SIZE(m_pObjects))
	{
		if(m_pObjects[x])
			if(pGtaObject == m_pObjects[x]->m_pEntity) return x;

		x++;
	}

	return INVALID_OBJECT_ID;
}

void CObjectPool::Process()
{
	static unsigned long s_ulongLastCall = 0;
	if (!s_ulongLastCall) s_ulongLastCall = GetTickCount();
	unsigned long ulongTick = GetTickCount();
	float fElapsedTime = ((float)(ulongTick - s_ulongLastCall)) / 1000.0f;
	// Get elapsed time in seconds
	for (int i = 0; i < ARRAY_SIZE(m_pObjects); i++)
	{
		if (m_bObjectSlotState[i]) {
			if(m_pObjects[i]) {
				m_pObjects[i]->Process(fElapsedTime);
			}
		}
	}
	s_ulongLastCall = ulongTick;
}
