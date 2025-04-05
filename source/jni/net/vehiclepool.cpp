#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../chatwindow.h"
#include "../keyboard.h"

#include "../util/CJavaWrapper.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CKeyBoard *pKeyBoard;

CVehiclePool::CVehiclePool()
{
	for(VEHICLEID VehicleID = 0; VehicleID < ARRAY_SIZE(m_pVehicles); VehicleID++)
	{
		m_bVehicleSlotState[VehicleID] = false;
		m_pVehicles[VehicleID] = nullptr;
		m_pGTAVehicles[VehicleID] = nullptr;
		m_bIsActive[VehicleID] = false;
		m_bIsMarker[VehicleID] = 0;
	}
}

CVehiclePool::~CVehiclePool()
{
	for(VEHICLEID VehicleID = 0; VehicleID < ARRAY_SIZE(m_pVehicles); VehicleID++) {
		Delete(VehicleID);
	}
}

void CVehiclePool::UpdateSpeed()
{
	if (!pGame) {
		return;
	}

	if (!pGame->FindPlayerPed()) {
		return;
	}

	CVehicle *pVehicle = GetAt(FindIDFromGtaPtr(pGame->FindPlayerPed()->GetGtaVehicle()));
	if (pGame->FindPlayerPed()->GetVehicleSeatID() == 0)
	{
	 	if(pVehicle)
	 	{
	 		int vehId = pVehicle->GetModelIndex();
	 		if (vehId != 481 && vehId != 509 && vehId != 510 && !pKeyBoard->IsOpen() && *(uint8_t*)(g_libGTASA + 0x008C9BA3) == 0)
	 		{
	 			if (!g_pJavaWrapper->isGlobalShowSpeedometer)
	 				g_pJavaWrapper->isGlobalShowSpeedometer = true;

	 			g_pJavaWrapper->ShowSpeedometer();
	 		}
	 		else
	 		{
	 			g_pJavaWrapper->HideSpeedometer();
	 			g_pJavaWrapper->isGlobalShowSpeedometer = false;
	 		}

	 		VECTOR vecMoveSpeed = {1, 1, 1};
	 		pVehicle->GetMoveSpeedVector(&vecMoveSpeed);

	 		int speed = sqrt((vecMoveSpeed.X * vecMoveSpeed.X) + (vecMoveSpeed.Y * vecMoveSpeed.Y) + (vecMoveSpeed.Z * vecMoveSpeed.Z)) * 183;

	 		g_pJavaWrapper->SetSpeedometerSpeed(speed);
	 		g_pJavaWrapper->SetSpeedometerCarHP((int) pVehicle->GetHealth());

	 		g_pJavaWrapper->SetEngineState(pVehicle->GetEngineState());
	 		//g_pJavaWrapper->SetLightState(pVehicle->m_byteLightStatus);
	 		g_pJavaWrapper->SetLockState(pVehicle->m_bDoorsLocked);
	 	}
	 }
	 else
	 {
	 	if (g_pJavaWrapper->isGlobalShowSpeedometer)
	 	{
	 		g_pJavaWrapper->HideSpeedometer();
	 		g_pJavaWrapper->isGlobalShowSpeedometer = false;
	 	}
	 }
}

void CVehiclePool::Process()
{
	CVehicle* pVehicle = NULL;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if(!pPlayerPool) {
		return;
	}

	CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();
	if(!pLocalPlayer) {
		return;
	}

	for (VEHICLEID x = 0; x < ARRAY_SIZE(m_pVehicles); x++)
	{
		if (GetSlotState(x))
		{
			pVehicle = m_pVehicles[x];

			if (m_bIsActive[x] && pVehicle)
			{
				float vehicleDistance;
				/*if (pVehicle->IsDriverLocalPlayer()) vehicleDistance = 200.0f;
				else vehicleDistance = 100.0f;*/
				vehicleDistance = 200.0f;
				
				if (pVehicle->GetDistanceFromLocalPlayerPed() < vehicleDistance)
				{
					if (!pVehicle->IsAdded())
					{
						pVehicle->Add();
						pVehicle->SetColor(pVehicle->m_byteColor1, pVehicle->m_byteColor2);
					}

					CVehicle* pTrailer = pVehicle->GetTrailer();
					if (pTrailer && !pTrailer->IsAdded())
					{
						MATRIX4X4 matPos;
						pVehicle->GetMatrix(&matPos);
						pTrailer->TeleportTo(matPos.pos.X, matPos.pos.Y, matPos.pos.Z);
						pTrailer->Add();
					}
					
					pVehicle->ProcessDamage();
					
					if(pNetGame->m_bManualVehicleEngineAndLight)
					{
						pVehicle->EnableEngine(pVehicle->GetEngineState() == 1);
					}
					else
					{
						if(pVehicle->GetEngineState() == -1)
						{
							if(!pVehicle->IsDriverLocalPlayer())
								pVehicle->EnableEngine(false);
							
							else
								pVehicle->EnableEngine(true);
						}
						else if(pVehicle->GetEngineState() != -1)
						{
							if(pVehicle->GetEngineState() == 0)
								pVehicle->EnableEngine(false);
							
							if(pVehicle->GetEngineState() == 1)
								pVehicle->EnableEngine(true);
						}
					}
					
					if (pVehicle->GetDoorState())
						pVehicle->SetDoorState(1);
					else
						pVehicle->SetDoorState(0);
					
					if(pVehicle->IsDriverLocalPlayer()) 
					{
						pVehicle->SetInvulnerable(false);
						if (m_bIsMarker[x] != 1) pVehicle->ProcessMarkers(vehicleDistance), m_bIsMarker[x] = 1;
					}
					else
					{
						pVehicle->SetInvulnerable(true);
						if (m_bIsMarker[x] != 2) pVehicle->ProcessMarkers(vehicleDistance), m_bIsMarker[x] = 2;
					}
				}
				else 
				{
					if (pVehicle->IsAdded())
					{
						pVehicle->Remove();
					}
					
					if (m_bIsMarker[x])
					{
						pVehicle->ProcessMarkers(vehicleDistance - 1.0f);
						m_bIsMarker[x] = 0;
					}
				}
			}

				/*if(pVehicle->GetHealth() == 0.0f)
				{
						NotifyVehicleDeath(x);
						continue;
				}

				if(pVehicle->GetVehicleSubtype() != VEHICLE_SUBTYPE_BOAT &&
					pVehicle->GetDistanceFromLocalPlayerPed() < 200.0f &&
					pVehicle->HasSunk())
				{
					NotifyVehicleDeath(x);
					continue;
				}


				if(pVehicle->m_pVehicle != m_pGTAVehicles[x])
					m_pGTAVehicles[x] = pVehicle->m_pVehicle;*/
		}
	}
}
#include "..//game/CCustomPlateManager.h"
bool CVehiclePool::New(NEW_VEHICLE *pNewVehicle)
{
#ifdef _CDEBUG
	if(pChatWindow) {
		pChatWindow->AddDebugMessage(OBFUSCATE("Added veh %d %d"), pNewVehicle->VehicleID, pNewVehicle->iVehicleType);
	}
#endif

	if(!WORLD_BOUNDS_VALIDATE(pNewVehicle->vecPos.X, pNewVehicle->vecPos.Y, pNewVehicle->vecPos.Z)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid position (CVehiclePool::New)", pNewVehicle->VehicleID);
		return false;
	}

	if(!ARRAY_VALIDATE(m_pVehicles, pNewVehicle->VehicleID)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid ID (CVehiclePool::New)", pNewVehicle->VehicleID);
		return false;
	}

	if(m_pVehicles[pNewVehicle->VehicleID] != nullptr)
	{
		if(pChatWindow) {
			pChatWindow->AddDebugMessage(OBFUSCATE("Warning: vehicle %u was not deleted"), pNewVehicle->VehicleID);
		}

		Delete(pNewVehicle->VehicleID);
	}

	m_pVehicles[pNewVehicle->VehicleID] = pGame->NewVehicle(pNewVehicle->iVehicleType,
		pNewVehicle->vecPos.X, pNewVehicle->vecPos.Y, pNewVehicle->vecPos.Z, 
		pNewVehicle->fRotation, pNewVehicle->byteAddSiren);

	if(m_pVehicles[pNewVehicle->VehicleID])
	{
		// colors
		if( pNewVehicle->aColor1 >= 0 || pNewVehicle->aColor2 >= 0 ||
			pNewVehicle->aColor1 <= 0xFF || pNewVehicle->aColor2 <= 0xFF) // 0-255
		{
			m_pVehicles[pNewVehicle->VehicleID]->SetColor(
				pNewVehicle->aColor1, pNewVehicle->aColor2 );
		}

		// health
		m_pVehicles[pNewVehicle->VehicleID]->SetHealth(pNewVehicle->fHealth);

		// gta handle
		m_pGTAVehicles[pNewVehicle->VehicleID] = m_pVehicles[pNewVehicle->VehicleID]->m_pVehicle;
		m_bVehicleSlotState[pNewVehicle->VehicleID] = true;

		// interior
		if(pNewVehicle->byteInterior > 0) {
			LinkToInterior(pNewVehicle->VehicleID, pNewVehicle->byteInterior);
		}

		// damage status
		if(pNewVehicle->dwPanelDamageStatus || 
			pNewVehicle->dwDoorDamageStatus || 
			pNewVehicle->byteLightDamageStatus)
		{
			m_pVehicles[pNewVehicle->VehicleID]->UpdateDamageStatus(
				pNewVehicle->dwPanelDamageStatus, 
				pNewVehicle->dwDoorDamageStatus,
				pNewVehicle->byteLightDamageStatus, pNewVehicle->byteTireDamageStatus);
		}

		m_pVehicles[pNewVehicle->VehicleID]->SetWheelPopped(pNewVehicle->byteTireDamageStatus);

		m_bIsActive[pNewVehicle->VehicleID] = true;
		m_bIsWasted[pNewVehicle->VehicleID] = false;

		m_bIsMarker[pNewVehicle->VehicleID] = 0;
		return true;
	}

	return false;
}

bool CVehiclePool::Delete(VEHICLEID VehicleID)
{
	if(!ARRAY_VALIDATE(m_pVehicles, VehicleID)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid ID (CVehiclePool::Delete)", VehicleID);
		return false;
	}

	if(!GetSlotState(VehicleID) || !m_pVehicles[VehicleID]) {
		return false;
	}

	m_bIsActive[VehicleID] = false;
	m_bIsMarker[VehicleID] = 0;
	m_bVehicleSlotState[VehicleID] = false;
	SAFE_DELETE(m_pVehicles[VehicleID]);
	m_pGTAVehicles[VehicleID] = nullptr;

	return true;
}

VEHICLEID CVehiclePool::FindIDFromGtaPtr(VEHICLE_TYPE *pGtaVehicle)
{
	int x=1;

	while(x != MAX_VEHICLES) 
	{
		if(pGtaVehicle == m_pGTAVehicles[x]) return x;
		x++;
	}

	return INVALID_VEHICLE_ID;
}

VEHICLEID CVehiclePool::FindIDFromRwObject(RwObject* pRWObject)
{
	int x = 1;

	while (x != MAX_VEHICLES)
	{
		if (m_pGTAVehicles[x])
		{
			if (pRWObject == (RwObject*)(m_pGTAVehicles[x]->entity.m_RwObject)) return x;
		}
		x++;
	}

	return INVALID_VEHICLE_ID;
}

int CVehiclePool::FindGtaIDFromID(int iID)
{
	if(!ARRAY_VALIDATE(m_pGTAVehicles, iID)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid ID (CVehiclePool::FindGtaIDFromID)", iID);
		return INVALID_VEHICLE_ID;
	}

	if(m_pGTAVehicles[iID]) {
		return GamePool_Vehicle_GetIndex(m_pGTAVehicles[iID]);
	}
	
	return INVALID_VEHICLE_ID;
}

int CVehiclePool::FindNearestToLocalPlayerPed()
{
	float fLeastDistance = 10000.0f;
	float fThisDistance = 0.0f;
	VEHICLEID ClosetSoFar = INVALID_VEHICLE_ID;

	VEHICLEID x = 0;
	while(x < ARRAY_SIZE(m_pVehicles))
	{
		if(GetSlotState(x) && m_bIsActive[x])
		{
			fThisDistance = m_pVehicles[x]->GetDistanceFromLocalPlayerPed();
			if(fThisDistance < fLeastDistance)
			{
				fLeastDistance = fThisDistance;
				ClosetSoFar = x;
			}
		}

		x++;
	}

	return ClosetSoFar;
}

void CVehiclePool::LinkToInterior(VEHICLEID VehicleID, int iInterior)
{
	if(!ARRAY_VALIDATE(m_pVehicles, VehicleID)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid ID (CVehiclePool::LinkToInterior)", VehicleID);
		return;
	}

	if(m_bVehicleSlotState[VehicleID]) {
		m_pVehicles[VehicleID]->LinkToInterior(iInterior);
	}
}

void CVehiclePool::NotifyVehicleDeath(VEHICLEID VehicleID)
{
	if(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle != VehicleID) return;
	Log(OBFUSCATE("CVehiclePool::NotifyVehicleDeath"));

	RakNet::BitStream bsDeath;
	bsDeath.Write(VehicleID);
	pNetGame->GetRakClient()->RPC(&RPC_VehicleDestroyed, &bsDeath, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle = INVALID_VEHICLE_ID;
}

void CVehiclePool::AssignSpecialParamsToVehicle(VEHICLEID VehicleID, uint8_t byteObjective, uint8_t byteDoorsLocked)
{
	if(!ARRAY_VALIDATE(m_pVehicles, VehicleID)) {
		SAMP_DUMP("VehiclePool", "Warning: vehicle %u has invalid ID (CVehiclePool::AssignSpecialParamsToVehicle)", VehicleID);
		return;
	}

	if(!GetSlotState(VehicleID)) return;
	CVehicle *pVehicle = m_pVehicles[VehicleID];

	if(pVehicle && m_bIsActive[VehicleID])
	{
		if(byteObjective)
		{
			pVehicle->m_byteObjectiveVehicle = 1;
			pVehicle->m_bSpecialMarkerEnabled = false;
		}

		pVehicle->SetDoorState(byteDoorsLocked);
	}
}