#pragma once

#define INVALID_OBJECT_ID	0xFFF9
#define MAX_OBJECTS			1000

class CObjectPool
{
private:
	bool m_bObjectSlotState[MAX_OBJECTS];
	CObject *m_pObjects[MAX_OBJECTS];

public:
	CObjectPool();
	~CObjectPool();

	bool New(uint16_t ObjectID, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDistance = 0);
	bool Delete(uint16_t ObjectID);

	bool GetSlotState(uint16_t ObjectID)
	{
		if (!ARRAY_VALIDATE(m_bObjectSlotState, ObjectID)) return false;
		return m_bObjectSlotState[ObjectID];
	};

	CObject *GetAt(uint16_t ObjectID)
	{
		if(!GetSlotState(ObjectID)) return (CObject *)0;
		return m_pObjects[ObjectID];
	};

	uint16_t FindIDFromGtaPtr(ENTITY_TYPE *pGtaObject);

	CObject *GetObjectFromGtaPtr(ENTITY_TYPE *pGtaObject);

	void Process();
};