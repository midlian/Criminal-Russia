#include "../main.h"
#include "game.h"
#include "../net/netgame.h"
#include <cmath>
#include "materialtext.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CMaterialText *pMaterialText;

float fixAngle(float angle)
{
	if (angle > 180.0f) angle -= 360.0f;
	if (angle < -180.0f) angle += 360.0f;

	return angle;
}

float subAngle(float a1, float a2)
{
	return fixAngle(fixAngle(a2) - a1);
}

CObject::CObject(int iModel, float fPosX, float fPosY, float fPosZ, VECTOR vecRot, float fDrawDistance)
{
	uint32_t dwRetID 	= 0;
	m_pEntity 			= nullptr;
	m_dwGTAId 			= 0;
	m_ObjectModel		= iModel;

	ScriptCommand(&create_object, iModel, fPosX, fPosY, fPosZ, &dwRetID);
	ScriptCommand(&put_object_at, dwRetID, fPosX, fPosY, fPosZ);

	m_pEntity = GamePool_Object_GetAt(dwRetID);
	m_dwGTAId = dwRetID;
	m_byteMoving = 0;
	m_fMoveSpeed = 0.0;

	m_vecRot = vecRot;
	m_vecTargetRotTar = vecRot;
	
	m_bIsPlayerSurfing = false;
	m_bNeedRotate = false;
	
	m_bAttached = false;
	m_bAttachedType = 0;
	m_usAttachedVehicle = 0xFFFF;

	m_bMaterials = false;
	for (auto & m_pMaterial : m_pMaterials)
	{
		m_pMaterial.m_bCreated = 0;
		m_pMaterial.pTex = nullptr;
	}
	
    m_bHasMaterialText = false;
	for(int i = 0; i <= MAX_MATERIALS_PER_MODEL; i++)
    {
        m_MaterialTextTexture[i] = 0;
    }

	InstantRotate(vecRot.X, vecRot.Y, vecRot.Z);
}
// todo
CObject::~CObject()
{
	m_bMaterials = false;
	for (auto & m_pMaterial : m_pMaterials)
	{
		if (m_pMaterial.m_bCreated && m_pMaterial.pTex)
		{
			m_pMaterial.m_bCreated = 0;
			RwTextureDestroy(m_pMaterial.pTex);
			m_pMaterial.pTex = nullptr;
		}
	}
	
	m_bHasMaterialText = false;
	for(int i = 0; i <= MAX_MATERIALS_PER_MODEL; i++)
    {
		if (m_MaterialTextTexture[i])
		{
			RwTextureDestroy(m_MaterialTextTexture[i]);
		}
        m_MaterialTextTexture[i] = 0;
    }
	
	m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
	if(m_pEntity)
		ScriptCommand(&destroy_object, m_dwGTAId);
}

void CObject::Process(float fElapsedTime)
{
	if (m_bAttachedType == 1 && !m_bAttached)
	{
		CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(m_usAttachedVehicle);
		if (pVehicle)
		{
			if (pVehicle->IsAdded())
			{
				if (m_vecAttachedOffset.X > 10000.0f || m_vecAttachedOffset.Y > 10000.0f || m_vecAttachedOffset.Z > 10000.0f ||
					m_vecAttachedOffset.X < -10000.0f || m_vecAttachedOffset.Y < -10000.0f || m_vecAttachedOffset.Z < -10000.0f)
				{ 
					// пропускаем действие
				}
				else
				{	
					m_bAttached = true;
					ProcessAttachToVehicle(pVehicle);
				}
			}
		}
	}
	m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
	if (!m_pEntity) return;
	if (!(m_pEntity->mat)) return;
	if (m_byteMoving & 1)
	{
		MATRIX4X4 matPos;
		GetMatrix(&matPos);


		VECTOR matRot = m_vecRot;

		float distance = fElapsedTime * m_fMoveSpeed;
		float remaining = DistanceRemaining(&matPos, &m_matTarget);
		float remainingRot = RotaionRemaining(m_vecTargetRotTar, m_vecTargetRot);

		if (distance >= remaining)
		{
			m_byteMoving &= ~1;

			m_vecTargetRotTar = m_vecTargetRot;

			TeleportTo(m_matTarget.pos.X, m_matTarget.pos.Y, m_matTarget.pos.Z);
			InstantRotate(m_vecTargetRot.X, m_vecTargetRot.Y, m_vecTargetRot.Z);
		}
		else
		{
			remaining /= distance;
			remainingRot /= distance;

			matPos.pos.X += (m_matTarget.pos.X - matPos.pos.X) / remaining;
			matPos.pos.Y += (m_matTarget.pos.Y - matPos.pos.Y) / remaining;
			matPos.pos.Z += (m_matTarget.pos.Z - matPos.pos.Z) / remaining;
			
			m_vecTargetRotTar.X += (m_vecTargetRot.X - m_vecTargetRotTar.X) / remaining;
			m_vecTargetRotTar.Y += (m_vecTargetRot.Y - m_vecTargetRotTar.Y) / remaining;
			m_vecTargetRotTar.Z += (m_vecTargetRot.Z - m_vecTargetRotTar.Z) / remaining;

			TeleportTo(matPos.pos.X, matPos.pos.Y, matPos.pos.Z);
			InstantRotate(m_vecTargetRotTar.X, m_vecTargetRotTar.Y, m_vecTargetRotTar.Z);
		}
		
		if (m_ObjectModel >= 19332 && m_ObjectModel <= 19338)
		{
			if (pNetGame) 
			{
				CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
				if (pPlayerPool) 
				{
					pPlayerPool->GetLocalPlayer()->ProcessSurfing();
				}
			}
		}
	}
}

float CObject::DistanceRemaining(MATRIX4X4 *matPos, MATRIX4X4 *m_matPositionTarget)
{
	float	fSX,fSY,fSZ;
	fSX = (matPos->pos.X - m_matPositionTarget->pos.X) * (matPos->pos.X - m_matPositionTarget->pos.X);
	fSY = (matPos->pos.Y - m_matPositionTarget->pos.Y) * (matPos->pos.Y - m_matPositionTarget->pos.Y);
	fSZ = (matPos->pos.Z - m_matPositionTarget->pos.Z) * (matPos->pos.Z - m_matPositionTarget->pos.Z);
	return (float)sqrt(fSX + fSY + fSZ);
}

float CObject::RotaionRemaining(VECTOR matPos, VECTOR m_vecRot)
{
	float fSX,fSY,fSZ;
	fSX = (matPos.X - m_vecRot.X) * (matPos.X - m_vecRot.X);
	fSY = (matPos.Y - m_vecRot.Y) * (matPos.Y - m_vecRot.Y);
	fSZ = (matPos.Z - m_vecRot.Z) * (matPos.Z - m_vecRot.Z);
	return (float)sqrt(fSX + fSY + fSZ);
}

void CObject::SetPos(float x, float y, float z)
{
	if (GamePool_Object_GetAt(m_dwGTAId))
		ScriptCommand(&put_object_at, m_dwGTAId, x, y, z);
}

void CObject::StopMoving()
{
	m_byteMoving = 0;
}

void CObject::MoveTo(float fX, float fY, float fZ, float fSpeed, float fRotX, float fRotY, float fRotZ)
{
	m_matTarget.pos.X = fX;
	m_matTarget.pos.Y = fY;
	m_matTarget.pos.Z = fZ;

	m_vecTargetRot.X = fRotX;
	m_vecTargetRot.Y = fRotY;
	m_vecTargetRot.Z = fRotZ;
	
	m_fMoveSpeed = fSpeed;
	m_byteMoving |= 1;
}

void CObject::AttachToVehicle(uint16_t usVehID, VECTOR* pVecOffset, VECTOR* pVecRot)
{
	m_bAttached = false;
	m_bAttachedType = 1;
	m_usAttachedVehicle = usVehID;
	m_vecAttachedOffset.X = pVecOffset->X;
	m_vecAttachedOffset.Y = pVecOffset->Y;
	m_vecAttachedOffset.Z = pVecOffset->Z;

	m_vecAttachedRotation.X = pVecRot->X;
	m_vecAttachedRotation.Y = pVecRot->Y;
	m_vecAttachedRotation.Z = pVecRot->Z;
}

void CObject::ProcessAttachToVehicle(CVehicle* pVehicle)
{
	if (GamePool_Object_GetAt(m_dwGTAId))
	{
		m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
		*(uint32_t*)((uintptr_t)m_pEntity + 28) &= 0xFFFFFFFE;

		if (!ScriptCommand(&is_object_attached, m_dwGTAId))
		{
			ScriptCommand(&attach_object_to_car, m_dwGTAId, pVehicle->m_dwGTAId, m_vecAttachedOffset.X,
				m_vecAttachedOffset.Y, m_vecAttachedOffset.Z, m_vecAttachedRotation.X, m_vecAttachedRotation.Y, m_vecAttachedRotation.Z);
		}
	}
}

void CObject::InstantRotate(float x, float y, float z)
{
	if (GamePool_Object_GetAt(m_dwGTAId))
	{
		ScriptCommand(&set_object_rotation, m_dwGTAId, x, y, z);
	}
}

void CObject::SetMaterialText(int iMaterialIndex, uint8_t byteMaterialSize, const char *szFontName, uint8_t byteFontSize, uint8_t byteFontBold, uint32_t dwFontColor, uint32_t dwBackgroundColor, uint8_t byteAlign, const char *szText)
{
    if (iMaterialIndex < 16)
	{
		if(m_MaterialTextTexture[iMaterialIndex])
    	{
        	RwTextureDestroy(m_MaterialTextTexture[iMaterialIndex]);
        	m_MaterialTextTexture[iMaterialIndex] = 0;
    	}
    	m_MaterialTextTexture[iMaterialIndex] = pMaterialText->Generate(byteMaterialSize, szFontName, byteFontSize, byteFontBold, dwFontColor, dwBackgroundColor, byteAlign, szText);
    	//m_dwMaterialTextColor[iMaterialIndex] = 0;
    	m_bHasMaterialText = true;
	}
}