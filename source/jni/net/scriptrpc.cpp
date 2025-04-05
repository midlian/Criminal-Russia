#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../chatwindow.h"
#include "../CAudioStream.h"
#include "../KillList.h"

#include "../util/CJavaWrapper.h"

#include <algorithm>

#include "../gui/gui.h"
extern CGUI* pGUI;

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CAudioStream* pAudioStream;
extern KillList *pKillList;

void ScrDisplayGameText(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrDisplayGameText"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	char szMessage[512];
	int iType;
	int iTime;
	int iLength;

	bsData.Read(iType);
	bsData.Read(iTime);
	bsData.Read(iLength);

	if(iLength > 512) return;

	bsData.Read(szMessage,iLength);
	szMessage[iLength] = '\0';

	//Log(OBFUSCATE("MIAHIL DisplayGameText"), szMessage);

	pGame->DisplayGameText(szMessage, iTime, iType);
}

void ScrSetGravity(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetGravity"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	float fGravity;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(fGravity);

	pGame->SetGravity(fGravity);
}

void ScrForceSpawnSelection(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrForceSpawnSelection"));
	pNetGame->GetPlayerPool()->GetLocalPlayer()->ReturnToClassSelection();
}

void ScrSetPlayerPos(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerPos"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	if(pLocalPlayer) pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X,vecPos.Y,vecPos.Z);
}

void ScrSetCameraPos(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetCameraPos"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VECTOR vecPos;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	pGame->GetCamera()->SetPosition(vecPos.X, vecPos.Y, vecPos.Z, 0.0f, 0.0f, 0.0f);
}

void ScrSetCameraLookAt(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetCameraLookAt"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VECTOR vecPos;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	pGame->GetCamera()->LookAtPoint(vecPos.X,vecPos.Y,vecPos.Z,2);	
}

void ScrSetPlayerFacingAngle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerFacingAngle"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	float fAngle;
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(fAngle);
	pGame->FindPlayerPed()->ForceTargetRotation(fAngle);
}

void ScrSetFightingStyle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetFightingStyle"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID playerId;
	uint8_t byteFightingStyle = 0;
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(playerId);
	bsData.Read(byteFightingStyle);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CPlayerPed *pPlayerPed = nullptr;

	if(pPlayerPool)
	{
		if(playerId == pPlayerPool->GetLocalPlayerID())
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		else if(pPlayerPool->GetSlotState(playerId)) 
			pPlayerPed = pPlayerPool->GetAt(playerId)->GetPlayerPed();

		if(pPlayerPed)				
				pPlayerPed->SetFightingStyle(byteFightingStyle);
	}
}

void ScrSetPlayerSkin(RPCParameters *rpcParams)
{
	//Log("RPC: ScrSetPlayerSkin");
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	int iPlayerID;
	unsigned int uiSkin;
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(iPlayerID);
	bsData.Read(uiSkin);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(iPlayerID == pPlayerPool->GetLocalPlayerID())
		pPlayerPool->GetLocalPlayer()->GetPlayerPed()->SetModelIndex(uiSkin);
	else
	{
		if(pPlayerPool->GetSlotState(iPlayerID) && pPlayerPool->GetAt(iPlayerID)->GetPlayerPed())
			pPlayerPool->GetAt(iPlayerID)->GetPlayerPed()->SetModelIndex(uiSkin);
	}
}

void ScrApplyPlayerAnimation(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrApplyPlayerAnimation"));
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID playerId;
	uint8_t byteAnimLibLen;
	uint8_t byteAnimNameLen;
	char szAnimLib[256];
	char szAnimName[256];
	float fS;
	bool opt1, opt2, opt3, opt4;
	int opt5;
	CPlayerPool *pPlayerPool = nullptr;
	CPlayerPed *pPlayerPed = nullptr;

	memset(szAnimLib, 0, 256);
	memset(szAnimName, 0, 256);

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(playerId);
	bsData.Read(byteAnimLibLen);
	bsData.Read(szAnimLib, byteAnimLibLen);
	bsData.Read(byteAnimNameLen);
	bsData.Read(szAnimName, byteAnimNameLen);
	bsData.Read(fS);
	bsData.Read(opt1);
	bsData.Read(opt2);
	bsData.Read(opt3);
	bsData.Read(opt4);
	bsData.Read(opt5);

	szAnimLib[byteAnimLibLen] = '\0';
	szAnimName[byteAnimNameLen] = '\0';

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool)
	{
		if(pPlayerPool->GetLocalPlayerID() == playerId)
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		else if(pPlayerPool->GetSlotState(playerId))
			pPlayerPed = pPlayerPool->GetAt(playerId)->GetPlayerPed();

		// Log("%s, %s", szAnimLib, szAnimName);
                                    // pChatWindow->AddDebugMessage("ApplyAnimation: {ffff00}%s{ffffff}(AnimLib), {ffff00}%s{ffffff}(AnimName)", szAnimLib, szAnimName);

		if(pPlayerPed)
			pPlayerPed->ApplyAnimation(szAnimName, szAnimLib, fS, (int)opt1, (int)opt2, (int)opt3, (int)opt4, (int)opt5);
	}
}

void ScrClearPlayerAnimations(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrClearPlayerAnimations"));

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	PLAYERID playerId;
	bsData.Read(playerId);
	MATRIX4X4 mat;

	CPlayerPool *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(playerId == pPlayerPool->GetLocalPlayerID()) 
		{
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else 
		{
			if(pPlayerPool->GetSlotState(playerId))
				pPlayerPed = pPlayerPool->GetAt(playerId)->GetPlayerPed();
		}
		
		if(pPlayerPed) 
		{
			pPlayerPed->GetMatrix(&mat);
			pPlayerPed->TeleportTo(mat.pos.X, mat.pos.Y, mat.pos.Z);
		}
	}
}

void ScrSetSpawnInfo(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetSpawnInfo"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYER_SPAWN_INFO SpawnInfo;

	RakNet::BitStream bsData(Data, (iBitLength/8)+1, false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read((char*)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	pPlayerPool->GetLocalPlayer()->SetSpawnInfo(&SpawnInfo);

	if (g_pJavaWrapper->isGlobalShowHUD)
		g_pJavaWrapper->ShowHUD(true);
}

void ScrCreateExplosion(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrCreateExplosion"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	float X, Y, Z, Radius;
	int   iType;

	bsData.Read(X);
	bsData.Read(Y);
	bsData.Read(Z);
	bsData.Read(iType);
	bsData.Read(Radius);

	ScriptCommand(&create_explosion_with_radius, X, Y, Z, iType, Radius);
}

void ScrSetPlayerHealth(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerHealth"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fHealth;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(fHealth);
	pLocalPlayer->GetPlayerPed()->SetHealth(fHealth);
}

void ScrSetPlayerArmour(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerArmour"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fHealth;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(fHealth);

	pLocalPlayer->GetPlayerPed()->SetArmour(fHealth);
}

void ScrSetPlayerColor(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerColor"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PLAYERID playerId;
	uint32_t dwColor;

	bsData.Read(playerId);
	bsData.Read(dwColor);

	if(playerId == pPlayerPool->GetLocalPlayerID()) 
	{
		pPlayerPool->GetLocalPlayer()->SetPlayerColor(dwColor);
	} 
	else 
	{
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(playerId);
		if(pPlayer)	pPlayer->SetPlayerColor(dwColor);
	}
}

void ScrSetPlayerName(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerName"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID playerId;
	uint8_t byteNickLen;
	char szNewName[MAX_PLAYER_NAME+1];
	uint8_t byteSuccess;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(playerId);
	bsData.Read(byteNickLen);

	if(byteNickLen > MAX_PLAYER_NAME) return;

	bsData.Read(szNewName, byteNickLen);
	bsData.Read(byteSuccess);

	szNewName[byteNickLen] = '\0';

	Log(OBFUSCATE("byteSuccess = %d"), byteSuccess);
	if (byteSuccess == 1) pPlayerPool->SetPlayerName(playerId, szNewName);
	
	// Extra addition which we need to do if this is the local player;
	if( pPlayerPool->GetLocalPlayerID() == playerId )
		pPlayerPool->SetLocalPlayerName( szNewName );
}

void ScrSetPlayerPosFindZ(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerPosFindZ"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	vecPos.Z = pGame->FindGroundZForCoord(vecPos.X, vecPos.Y, vecPos.Z);
	vecPos.Z += 1.5f;

	pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X, vecPos.Y, vecPos.Z);
}

void ScrSetPlayerInterior(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerInterior"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;


	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	uint8_t byteInterior;
	bsData.Read(byteInterior);

	pGame->FindPlayerPed()->SetInterior(byteInterior);	
}

void ScrSetMapIcon(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetMapIcon"));
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	uint8_t byteIndex;
	uint8_t byteIcon;
	uint32_t dwColor;
	float fPos[3];
	uint8_t byteStyle;

	bsData.Read(byteIndex);
	bsData.Read(fPos[0]);
	bsData.Read(fPos[1]);
	bsData.Read(fPos[2]);
	bsData.Read(byteIcon);
	bsData.Read(dwColor);
	bsData.Read(byteStyle);

	pNetGame->SetMapIcon(byteIndex, fPos[0], fPos[1], fPos[2], byteIcon, dwColor, byteStyle);
}

void ScrDisableMapIcon(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrDisableMapIcon"));
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	uint8_t byteIndex;

	bsData.Read(byteIndex);

	pNetGame->DisableMapIcon(byteIndex);
}

void ScrSetCameraBehindPlayer(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetCameraBehindPlayer"));

	pGame->GetCamera()->SetBehindPlayer();	
}

void ScrSetPlayerSpecialAction(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerSpecialAction"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	uint8_t byteSpecialAction;
	bsData.Read(byteSpecialAction);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) pPlayerPool->GetLocalPlayer()->ApplySpecialAction(byteSpecialAction);
}

void ScrTogglePlayerSpectating(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrTogglePlayerSpectating"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	uint32_t bToggle;
	bsData.Read(bToggle);
	pPlayerPool->GetLocalPlayer()->ToggleSpectating(bToggle);
	Log(OBFUSCATE("toggle: %d"), bToggle);
}

void ScrSetPlayerSpectating(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerSpectating"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID playerId;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(playerId);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool->GetSlotState(playerId))
		pPlayerPool->GetAt(playerId)->SetState(PLAYER_STATE_SPECTATING);
}

#define SPECTATE_TYPE_NORMAL	1
#define SPECTATE_TYPE_FIXED		2
#define SPECTATE_TYPE_SIDE		3

void ScrPlayerSpectatePlayer(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrPlayerSpectatePlayer"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	
	PLAYERID playerId;
    uint8_t byteMode;
	
	bsData.Read(playerId);
	bsData.Read(byteMode);

	switch (byteMode) 
	{
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 4;
	}

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectatePlayer(playerId);
}

void ScrPlayerSpectateVehicle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrPlayerSpectateVehicle"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	VEHICLEID VehicleID;
	uint8_t byteMode;

	bsData.Read(VehicleID);
	bsData.Read(byteMode);

	switch (byteMode) 
	{
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 3;
	}

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectateVehicle(VehicleID);
}

void ScrPutPlayerInVehicle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrPutPlayerInVehicle"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VEHICLEID vehicleid;
	uint8_t seatid;
	bsData.Read(vehicleid);
	bsData.Read(seatid);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	int iVehicleIndex = pNetGame->GetVehiclePool()->FindGtaIDFromID(vehicleid);
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(vehicleid);

	if(iVehicleIndex && pVehicle)
		 pGame->FindPlayerPed()->PutDirectlyInVehicle(iVehicleIndex, seatid);
}

void ScrVehicleParams(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetVehicleParams"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleID;
	uint8_t byteObjectiveVehicle;
	uint8_t byteDoorsLocked;

	bsData.Read(VehicleID);
	bsData.Read(byteObjectiveVehicle);
	bsData.Read(byteDoorsLocked);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->AssignSpecialParamsToVehicle(VehicleID,byteObjectiveVehicle,byteDoorsLocked);
}

#pragma pack(1)
typedef struct _VEHICLE_PARAMS_STATUS_EX
{
	uint8_t engine, lights, alarm, doors, bonnet, boot, objective;
	uint8_t unk1;
	uint8_t door_driver;
	uint8_t unk[3];
	uint8_t windor_driver;
} VEHICLE_PARAMS_STATUS_EX;

void ScrVehicleParamsEx(RPCParameters* rpcParams)
{
	//Log("RPC: ScrVehicleParamsEx");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID VehicleId;
	uint8_t engine, lights, alarm, doors, bonnet, boot, objective;
	bsData.Read(VehicleId);
	bsData.Read(engine);
	bsData.Read(lights);
	bsData.Read(alarm);
	bsData.Read(doors);
	bsData.Read(bonnet);
	bsData.Read(boot);
	bsData.Read(objective);
	//Log("VehicleId: %d", VehicleId);
	//Log("engine: %d, lights: %d, alarm: %d, doors: %d, bonnet: %d, boot: %d, obj: %d",
//		engine, lights, alarm, doors, bonnet, boot, objective);

	if (pNetGame && pNetGame->GetVehiclePool())
	{
		if (pNetGame->GetVehiclePool()->GetSlotState(VehicleId))
		{
			// -- doors
			pNetGame->GetVehiclePool()->GetAt(VehicleId)->SetDoorState(doors);
			
                                                      // -- engine
			pNetGame->GetVehiclePool()->GetAt(VehicleId)->SetEngineState(engine);
			
			// -- lights
			pNetGame->GetVehiclePool()->GetAt(VehicleId)->SetLightsState(lights);			

                                                      // -- boot && bonnet
			// pNetGame->GetVehiclePool()->GetAt(VehicleId)->SetBonnetAndBootStatus((int)bonnet, (int)boot);
		}
	}
}


void ScrHaveSomeMoney(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrHaveSomeMoney"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	int iAmmount;
	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bsData.Read(iAmmount);
	pGame->AddToLocalMoney(iAmmount);
}

void ScrResetMoney(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrResetMoney"));

	pGame->ResetLocalMoney();
}

void ScrLinkVehicle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrLinkVehicle"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleID;
	uint8_t byteInterior;

	bsData.Read(VehicleID);
	bsData.Read(byteInterior);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->LinkToInterior(VehicleID, (int)byteInterior);
}

void ScrRemovePlayerFromVehicle(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrRemovePlayerFromVehicle"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	pPlayerPool->GetLocalPlayer()->GetPlayerPed()->ExitCurrentVehicle();
}

void ScrSetVehicleHealth(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetVehicleHealth"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	float fHealth;
	VEHICLEID VehicleID;

	bsData.Read(VehicleID);
	bsData.Read(fHealth);

	if(pNetGame->GetVehiclePool()->GetSlotState(VehicleID))
		pNetGame->GetVehiclePool()->GetAt(VehicleID)->SetHealth(fHealth);
}

void ScrSetVehiclePos(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetVehiclePos"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleId;
	float fX, fY, fZ;
	bsData.Read(VehicleId);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	if(pNetGame && pNetGame->GetVehiclePool())
	{
		if(pNetGame->GetVehiclePool()->GetSlotState(VehicleId))
			pNetGame->GetVehiclePool()->GetAt(VehicleId)->TeleportTo(fX, fY, fZ);
	}
}

void ScrSetVehicleVelocity(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetVehicleVelocity"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	uint8_t turn = false;
	VECTOR vecMoveSpeed;
	bsData.Read(turn);
	bsData.Read(vecMoveSpeed.X);
	bsData.Read(vecMoveSpeed.Y);
	bsData.Read(vecMoveSpeed.Z);
	Log(OBFUSCATE("X: %f, Y: %f, Z: %f"), vecMoveSpeed.X, vecMoveSpeed.Y, vecMoveSpeed.Z);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();

	if(pPlayerPed)
	{
		CVehicle *pVehicle = pVehiclePool->GetAt( pVehiclePool->FindIDFromGtaPtr(pPlayerPed->GetGtaVehicle()));
		if(pVehicle)
			pVehicle->SetMoveSpeedVector(vecMoveSpeed);
	}
}

void ScrNumberPlate(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrNumberPlate"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	VEHICLEID VehicleID;
	char len;
	char szNumberPlate[32+1];

	/*bsData.Read(VehicleID);
	bsData.Read(len);
	bsData.Read(szNumberPlate, len);
	szNumberPlate[len] = '\0';*/
}

void ScrInterpolateCamera(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("ScrInterpolateCamera"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	bool bSetPos = true;
	VECTOR vecFrom, vecDest;
	int time;
	uint8_t mode;

	bsData.Read(bSetPos);
	bsData.Read(vecFrom.X);
	bsData.Read(vecFrom.Y);
	bsData.Read(vecFrom.Z);
	bsData.Read(vecDest.X);
	bsData.Read(vecDest.Y);
	bsData.Read(vecDest.Z);
	bsData.Read(time);
	bsData.Read(mode);

	if(mode < 1 || mode > 2)
		mode = 2;

	if(time > 0)
	{
		pNetGame->GetPlayerPool()->GetLocalPlayer()->m_bSpectateProcessed = true;
		if(bSetPos)
		{
			pGame->GetCamera()->InterpolateCameraPos(&vecFrom, &vecDest, time, mode);
		}
		else
			pGame->GetCamera()->InterpolateCameraLookAt(&vecFrom, &vecDest, time, mode);
	}
}

void ScrAddGangZone(RPCParameters *rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		uint16_t wZoneID;
		float minx, miny, maxx, maxy;
		uint32_t dwColor;
		bsData.Read(wZoneID);
		bsData.Read(minx);
		bsData.Read(miny);
		bsData.Read(maxx);
		bsData.Read(maxy);
		bsData.Read(dwColor);
		pGangZonePool->New(wZoneID, minx, miny, maxx, maxy, dwColor);
	}
}

void ScrRemoveGangZone(RPCParameters *rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();

	if (pGangZonePool)
	{
		uint16_t wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->Delete(wZoneID);
	}
}

void ScrFlashGangZone(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrFlashGangZone"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		uint16_t wZoneID;
		uint32_t dwColor;
		bsData.Read(wZoneID);
		bsData.Read(dwColor);
		pGangZonePool->Flash(wZoneID, dwColor);
	}
}

void ScrStopFlashGangZone(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrStopFlashGangZone"));

	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		uint16_t wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->StopFlash(wZoneID);
	}
}

int iTotalObjects = 0;

RwTexture* LoadTextureForReTexture(const char* szTexture)
{
	RwTexture* tex;

	std::vector<const char *> badTextures = { OBFUSCATE("none"), "", " " };

	for (auto & badTexture : badTextures)
	{
		if (strncmp(szTexture, badTexture, 5u) == 0)
			return nullptr;
	}

	tex = (RwTexture*)LoadTexture(szTexture);

	// -- Converting texture name to lower register and loading it
	if (!tex)
	{
		std::string str = szTexture;
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		tex = (RwTexture*)LoadTexture(str.c_str());
	}

	if (!tex)
		return nullptr;

	return tex;
}

void ScrCreateObject(RPCParameters* rpcParams)
{
    unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
    int iBitLength = rpcParams->numberOfBitsOfData;

    uint16_t wObjectID;
    unsigned long ModelID;
    float fDrawDistance;
    VECTOR vecPos, vecRot;
    uint8_t byteMatType;
    uint8_t bNoCameraCol;
    int16_t attachedVehicleID;
    int16_t attachedObjectID;
    VECTOR vecAttachedOffset;
    VECTOR vecAttachedRotation;
    uint8_t bSyncRot;
    uint8_t iMaterialCount;
    uint8_t id;
    // Material Text
    uint8_t byteMaterialSize;
    uint8_t byteFontNameLength;
    char szFontName[32];
    uint8_t byteFontSize;
    uint8_t byteFontBold;
    uint32_t dwFontColor;
    uint32_t dwBackgroundColor;
    uint8_t byteAlign;
    char szText[2048];

    RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
    bsData.Read(wObjectID);
    bsData.Read(ModelID);
    bsData.Read(vecPos.X);
    bsData.Read(vecPos.Y);
    bsData.Read(vecPos.Z);

    Log(OBFUSCATE("RPC_SCRCREATEOBJECT(%d) %d"), iTotalObjects, ModelID);

    bsData.Read(vecRot.X);
    bsData.Read(vecRot.Y);
    bsData.Read(vecRot.Z);

    bsData.Read(fDrawDistance);

    bsData.Read(bNoCameraCol);
    bsData.Read(attachedVehicleID);
    bsData.Read(attachedObjectID);
    if (attachedObjectID != -1 || attachedVehicleID != -1)
    {
        bsData.Read(vecAttachedOffset);
        bsData.Read(vecAttachedRotation);
        bsData.Read(bSyncRot);
    }
    bsData.Read(iMaterialCount);

    iTotalObjects++;
    //Log("ID: %d, model: %d. iTotalObjects = %d", wObjectID, ModelID, iTotalObjects);

    CObjectPool* pObjectPool = pNetGame->GetObjectPool();
    pObjectPool->New(wObjectID, ModelID, vecPos, vecRot, fDrawDistance);

    CObject* pObject = pObjectPool->GetAt(wObjectID);
    if (!pObject) return;
    if (attachedVehicleID != -1)
    {
        pObject->AttachToVehicle(attachedVehicleID, &vecAttachedOffset, &vecAttachedRotation);
    }
    if (iMaterialCount > 0)
    {
        for (int i = 0; i < iMaterialCount; i++)
        {
            memset(szFontName, 0, sizeof(szFontName));
            memset(szText, 0, sizeof(szText));
            bsData.Read(byteMatType);

            if(byteMatType == 1) {
                uint16_t modelId;
                uint8_t libLength, texLength;

                bsData.Read(id);
                if (id == 2) continue;
                uint8_t slot;
                bsData.Read(slot);
                bsData.Read(modelId);

                bsData.Read(libLength);
                char *str = new char[libLength + 1];
                bsData.Read(str, libLength);

                bsData.Read(texLength);
                char *tex = new char[texLength + 1];
                bsData.Read(tex, texLength);

                tex[texLength] = 0;
                str[libLength] = 0;

                int oldmodel = modelId;

                if (modelId < 0 || modelId > 20000)
                    modelId = 18631;

                if (!pGame->IsModelLoaded(modelId))
                {
                    int tries = 0;

					pGame->RequestModel(modelId);
					pGame->LoadRequestedModels();

                    while (!pGame->IsModelLoaded(modelId))
                    {
                        if (tries >= 5)
                        {
                            Log(OBFUSCATE("Error on object %d original %d"), modelId, oldmodel);
                            return;
                        }

                        tries++;
                        usleep(100);
                    }
                }

				if (!modelId)
					return;

				if (slot > MAX_MATERIALS)
					return;

                // Material Object
                pObject->m_bMaterials = true;
                pObject->m_pMaterials[slot].wModelID = modelId;
                pObject->m_pMaterials[slot].pTex = LoadTextureForReTexture(tex);
                pObject->m_pMaterials[slot].m_bCreated = 1;

                if (!strncmp(tex, OBFUSCATE("materialtext1"), sizeof(("materialtext1"))))
                    tex = OBFUSCATE("MaterialText1");

                if (!strncmp(tex, OBFUSCATE("sampblack"), sizeof(("sampblack"))))
                    tex = OBFUSCATE("SAMPBlack");

                if (!strncmp(tex, OBFUSCATE("carpet19-128x128"), sizeof(("carpet19-128x128"))))
                    tex = OBFUSCATE("Carpet19-128x128");

                if (!pObject->m_pMaterials[slot].pTex && strncmp(tex, OBFUSCATE("none"), sizeof(("none"))))
                    pObject->m_pMaterials[slot].pTex = LoadTextureForReTexture(tex);

                bsData.Read(pObject->m_pMaterials[i].dwColor);

                if (!GetModelReferenceCount(modelId) && pGame->IsModelLoaded(modelId))
                    ScriptCommand(&release_model, modelId);
            }
            if(byteMatType == 2)
			{
				uint8_t byteMaterialIndex = 0;
				uint8_t byteMaterialSize;
				uint8_t byteFontNameLength;
				char szFontName[32];
				uint8_t byteFontSize;
				uint8_t byteFontBold;
				uint32_t dwFontColor;
				uint32_t dwBackgroundColor;
				uint8_t byteAlign;
				char szText[2048];
	
                bsData.Read(byteMaterialIndex);
                bsData.Read(byteMaterialSize);
                bsData.Read(byteFontNameLength);
                bsData.Read(szFontName, byteFontNameLength);
                szFontName[byteFontNameLength] = '\0';
                bsData.Read(byteFontSize);
                bsData.Read(byteFontBold);
                bsData.Read(dwFontColor);
                bsData.Read(dwBackgroundColor);
                bsData.Read(byteAlign);
                stringCompressor->DecodeString(szText, 2048, &bsData);

				Log("dwFontColor: %d, dwBackgroundColor: %d", dwFontColor, dwBackgroundColor);
                if(strlen(szFontName) <= 32)
                {
                    if(pObject)
                    {
                        pObject->SetMaterialText(byteMaterialIndex, byteMaterialSize, szFontName, byteFontSize, byteFontBold, dwFontColor, dwBackgroundColor, byteAlign, szText);
                    }
                }
            }
        }
    }
}

void ScrDestroyObject(RPCParameters *rpcParams)
{

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint16_t wObjectID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(wObjectID);

	//LOGI("id: %d", wObjectID);
	iTotalObjects--;

	CObjectPool* pObjectPool =	pNetGame->GetObjectPool();
	if(pObjectPool->GetAt(wObjectID))
		pObjectPool->Delete(wObjectID);
}

void ScrSetObjectPos(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC_SCRSETOBJECTPOS"));

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint16_t wObjectID;
	float fRotation;
	VECTOR vecPos, vecRot;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(wObjectID);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fRotation);

	//LOGI("id: %d x: %.2f y: %.2f z: %.2f", wObjectID, vecPos.X, vecPos.Y, vecPos.Z);
	//LOGI("VecRot x: %.2f y: %.2f z: %.2f", vecRot.X, vecRot.Y, vecRot.Z);

	CObjectPool*	pObjectPool =	pNetGame->GetObjectPool();
	CObject*		pObject		=	pObjectPool->GetAt(wObjectID);
	if(pObject)
	{
		pObject->SetPos(vecPos.X, vecPos.Y, vecPos.Z);
	}
}

void ScrAttachObjectToPlayer(RPCParameters* rpcParams)
{
	Log(OBFUSCATE("RPC_SCRATTACHOBJECTTOPLAYER"));

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	PLAYERID wObjectID, wPlayerID;
	float OffsetX, OffsetY, OffsetZ, rX, rY, rZ;

	bsData.Read(wObjectID);
	bsData.Read(wPlayerID);

	bsData.Read(OffsetX);
	bsData.Read(OffsetY);
	bsData.Read(OffsetZ);

	bsData.Read(rX);
	bsData.Read(rY);
	bsData.Read(rZ);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(wObjectID);
	if (!pObject) return;

	if (wPlayerID == pNetGame->GetPlayerPool()->GetLocalPlayerID())
	{
		CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		ScriptCommand(&attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
			OffsetX, OffsetY, OffsetZ, rX, rY, rZ);
	}
	else {
		CRemotePlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(wPlayerID);

		if (!pPlayer)
			return;

		ScriptCommand(&attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
			OffsetX, OffsetY, OffsetZ, rX, rY, rZ);
	}
}


void ScrPlaySound(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrPlaySound"));

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);

	int iSound;
	float fX, fY, fZ;
	bsData.Read(iSound);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	pGame->PlaySound(iSound, fX, fY, fZ);
}

void ScrSetPlayerWantedLevel(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetPlayerWantedLevel"));

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data,(iBitLength/8)+1,false);
	
	if(!pGame) return;

	uint8_t byteLevel;
	bsData.Read(byteLevel);
	pGame->SetWantedLevel(byteLevel);
}

void ScrGivePlayerWeapon(RPCParameters* rpcParams)
{
	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	PLAYERID wPlayerID;
	int iWeaponID;
	int iAmmo;
	bsData.Read(iWeaponID);
	bsData.Read(iAmmo);

	CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
                  if(!pGUI->m_bCarryStuff) pPlayer->GetPlayerPed()->GiveWeapon(iWeaponID, iAmmo);
}

void ScrTogglePlayerControllable(RPCParameters *rpcParams)
{
	Log(OBFUSCATE("RPC: TogglePlayerControllable"));

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	uint8_t byteControllable;
	bsData.Read(byteControllable);
	//Log("controllable = %d", byteControllable);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->LockControllable(!byteControllable);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->TogglePlayerControllable((int)byteControllable);
}

void ScrSetPlayerSkillLevel(RPCParameters *rpcParams)
{
	int iBitLength = rpcParams->numberOfBitsOfData;

	CPlayerPool * pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool) return;

	RakNet::BitStream bsData(rpcParams->input, (iBitLength / 8) + 1, false);

	PLAYERID bytePlayerID;
	unsigned int ucSkillType;
	unsigned short uiSkillLevel;

	bsData.Read(bytePlayerID);
	bsData.Read(ucSkillType);
	bsData.Read(uiSkillLevel);

	if (ucSkillType < 0 || ucSkillType > 10) return;
	if (uiSkillLevel < 0 || uiSkillLevel > 1000) return;

	switch (ucSkillType)
	{
	case 0:
		ucSkillType = WEAPONTYPE_PISTOL_SKILL;
		break;
	case 1:
		ucSkillType = WEAPONTYPE_PISTOL_SILENCED_SKILL;
		break;
	case 2:
		ucSkillType = WEAPONTYPE_DESERT_EAGLE_SKILL;
		break;
	case 3:
		ucSkillType = WEAPONTYPE_SHOTGUN_SKILL;
		break;
	case 4:
		ucSkillType = WEAPONTYPE_SAWNOFF_SHOTGUN_SKILL;
		break;
	case 5:
		ucSkillType = WEAPONTYPE_SPAS12_SHOTGUN_SKILL;
		break;
	case 6:
		ucSkillType = WEAPONTYPE_MICRO_UZI_SKILL;
		break;
	case 7:
		ucSkillType = WEAPONTYPE_MP5_SKILL;
		break;
	case 8:
		ucSkillType = WEAPONTYPE_AK47_SKILL;
		break;
	case 9:
		ucSkillType = WEAPONTYPE_M4_SKILL;
		break;
	case 10:
		ucSkillType = WEAPONTYPE_SNIPERRIFLE_SKILL;
		break;
	default:
		return;
	}

	if (bytePlayerID == pPlayerPool->GetLocalPlayerID())
	{
		float* StatsTypesFloat = (float*)(SA_ADDR(0x8C41A0));
		StatsTypesFloat[ucSkillType] = uiSkillLevel;
	}
	else
	{
		CRemotePlayer* pRemotePlayer = pPlayerPool->GetAt(bytePlayerID);
		if (!pRemotePlayer) return;

		CPlayerPed* pPlayerPed = pRemotePlayer->GetPlayerPed();
		if (!pPlayerPed) return;

		pPlayerPed->SetFloatStat(ucSkillType, uiSkillLevel);
	}
}

void ScrResetPlayerWeapons(RPCParameters* rpcParams)
{
	uint8_t* Data = reinterpret_cast<uint8_t*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CPlayerPed* pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->ClearAllWeapons();
}

void ScrShowTextDraw(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		uint16_t wTextID;
		uint16_t wTextSize;
		TEXT_DRAW_TRANSMIT TextDrawTransmit;
		char cText[MAX_TEXT_DRAW_LINE];

		bsData.Read(wTextID);
		bsData.Read((char*)& TextDrawTransmit, sizeof(TEXT_DRAW_TRANSMIT));
		bsData.Read(wTextSize);
		bsData.Read(cText, wTextSize);
		cText[wTextSize] = 0;
		pTextDrawPool->New(wTextID, &TextDrawTransmit, cText);
	}
}

void ScrHideTextDraw(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		uint16_t wTextID;
		bsData.Read(wTextID);
		pTextDrawPool->Delete(wTextID);
	}
}

void ScrEditTextDraw(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint16_t wTextID;
	uint16_t wLen;
	bsData.Read(wTextID);
	bsData.Read(wLen);
	uint8_t pStr[256];
	if (wLen >= 255) return;

	bsData.Read((char*)pStr, wLen);
	pStr[wLen] = 0;
	CTextDraw* pTextDraw = pNetGame->GetTextDrawPool()->GetAt(wTextID);
	if (pTextDraw)
	{
		pTextDraw->SetText((const char*)pStr);
	}
}

#define ATTACH_BONE_SPINE	1
#define ATTACH_BONE_HEAD	2
#define ATTACH_BONE_LUPPER	3
#define ATTACH_BONE_RUPPER	4
#define ATTACH_BONE_LHAND	5
#define ATTACH_BONE_RHAND	6
#define ATTACH_BONE_LTHIGH	7
#define ATTACH_BONE_RTHIGH	8
#define ATTACH_BONE_LFOOT	9
#define ATTACH_BONE_RFOOT	10
#define ATTACH_BONE_RCALF	11
#define ATTACH_BONE_LCALF	12
#define ATTACH_BONE_LFARM	13
#define ATTACH_BONE_RFARM	14
#define ATTACH_BONE_LSHOULDER	15
#define ATTACH_BONE_RSHOULDER	16
#define ATTACH_BONE_NECK	17
#define ATTACH_BONE_JAW		18

int GetInternalBoneIDFromSampID(int sampid)
{
	switch (sampid)
	{
	case ATTACH_BONE_SPINE: // 3 or 2
		return 3;
	case ATTACH_BONE_HEAD: // ?
		return 5;
	case ATTACH_BONE_LUPPER: // left upper arm
		return 22;
	case ATTACH_BONE_RUPPER: // right upper arm
		return 32;
	case ATTACH_BONE_LHAND: // left hand
		return 34;
	case ATTACH_BONE_RHAND: // right hand
		return 24;
	case ATTACH_BONE_LTHIGH: // left thigh
		return 41;
	case ATTACH_BONE_RTHIGH: // right thigh
		return 51;
	case ATTACH_BONE_LFOOT: // left foot
		return 43;
	case ATTACH_BONE_RFOOT: // right foot
		return 53;
	case ATTACH_BONE_RCALF: // right calf
		return 52;
	case ATTACH_BONE_LCALF: // left calf
		return 42;
	case ATTACH_BONE_LFARM: // left forearm
		return 33;
	case ATTACH_BONE_RFARM: // right forearm
		return 23;
	case ATTACH_BONE_LSHOULDER: // left shoulder (claviacle)
		return 31;
	case ATTACH_BONE_RSHOULDER: // right shoulder (claviacle)
		return 21;
	case ATTACH_BONE_NECK: // neck
		return 4;
	case ATTACH_BONE_JAW: // jaw ???
		return 8; // i dont know
	}
	return 0;
}

void ScrSetPlayerAttachedObject(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	PLAYERID id;
	uint32_t slot;
	bool create;
	ATTACHED_OBJECT_INFO info;

	bsData.Read(id);
	bsData.Read(slot);
	bsData.Read(create);
	CPlayerPed* pPed = nullptr;
	if (id == pNetGame->GetPlayerPool()->GetLocalPlayerID())
	{
		pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	}
	else
	{
		if (pNetGame->GetPlayerPool()->GetSlotState(id))
		{
			pPed = pNetGame->GetPlayerPool()->GetAt(id)->GetPlayerPed();
		}
	}
	if (!pPed) return;
	if (!create)
	{
		pPed->DeattachObject(slot);
		return;
	}
	bsData.Read((char*)& info, sizeof(ATTACHED_OBJECT_INFO));
	info.dwBone = GetInternalBoneIDFromSampID(info.dwBone);
	pPed->AttachObject(&info, slot);
}


void ScrSetPlayerObjectMaterial(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint16_t wObjectID;
	int16_t modelId;
	uint8_t materialType, matId, libLength, texLength;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(wObjectID);
	bsData.Read(materialType);
	
	CObject* pObj = pNetGame->GetObjectPool()->GetAt(wObjectID);
	if (!pObj) return;
	
	if (materialType == 2)
	{
		uint8_t byteMaterialIndex = 0;
		uint8_t byteMaterialSize;
		uint8_t byteFontNameLength;
		char szFontName[32];
		uint8_t byteFontSize;
		uint8_t byteFontBold;
		uint32_t dwFontColor;
		uint32_t dwBackgroundColor;
		uint8_t byteAlign;
		char szText[2048];
	
        bsData.Read(byteMaterialIndex);
        bsData.Read(byteMaterialSize);
        bsData.Read(byteFontNameLength);
        bsData.Read(szFontName, byteFontNameLength);
        szFontName[byteFontNameLength] = '\0';
        bsData.Read(byteFontSize);
        bsData.Read(byteFontBold);
		bsData.Read(dwFontColor);
        bsData.Read(dwBackgroundColor);
        bsData.Read(byteAlign);
        stringCompressor->DecodeString(szText, 2048, &bsData);

		Log("dwFontColor: %d, dwBackgroundColor: %d", dwFontColor, dwBackgroundColor);
        if(strlen(szFontName) <= 32)
        {
			if(pObj)
            {
                pObj->SetMaterialText(byteMaterialIndex, byteMaterialSize, szFontName, byteFontSize, byteFontBold, dwFontColor, dwBackgroundColor, byteAlign, szText);
            }
        }
		return;
	}
	
	bsData.Read(matId);
	bsData.Read(modelId);
	bsData.Read(libLength);
	char* str = new char[libLength + 1];
	bsData.Read(str, libLength);
	str[libLength] = 0;
	bsData.Read(texLength);
	char* tex = new char[texLength + 1];
	bsData.Read(tex, texLength);
	uint32_t col;
	bsData.Read(col);
	tex[texLength] = 0;

	union color
	{
		uint32_t dwColor;
		uint8_t cols[4];
	};

	color rightColor;
	rightColor.dwColor = col;
	uint8_t temp = rightColor.cols[0];
	rightColor.cols[0] = rightColor.cols[2];
	rightColor.cols[2] = temp;
	col = rightColor.dwColor;

	if (modelId == -1)
	{
		pObj->m_bMaterials = true;
		pObj->m_pMaterials[matId].m_bCreated = true;
		pObj->m_pMaterials[matId].wModelID = 0xFFFF;
		pObj->m_pMaterials[matId].pTex = nullptr;
		pObj->m_pMaterials[matId].dwColor = col;
		return;
	}

	int oldmodel = modelId;

	if (modelId < 0 || modelId > 20000)
		modelId = 18631;

	if (!pGame->IsModelLoaded(modelId))
	{
		int tries = 0;

		pGame->RequestModel(modelId);
		pGame->LoadRequestedModels();

		while (!pGame->IsModelLoaded(modelId))
		{
			if (tries >= 5)
			{
				Log(OBFUSCATE("Error on object %d original %d"), modelId, oldmodel);
				return;
			}

			tries++;
			usleep(100);
		}
	}

	if (!modelId)
		return;

	if (matId > MAX_MATERIALS)
		return;

	pObj->m_bMaterials = true;
	pObj->m_pMaterials[matId].m_bCreated = true;
	pObj->m_pMaterials[matId].wModelID = modelId;
	pObj->m_pMaterials[matId].pTex = LoadTextureForReTexture(tex);
	pObj->m_pMaterials[matId].dwColor = col;

	if (!strncmp(tex, OBFUSCATE("materialtext1"), sizeof(("materialtext1"))))
		tex = OBFUSCATE("MaterialText1");

	if (!strncmp(tex, OBFUSCATE("sampblack"), sizeof(("sampblack"))))
		tex = OBFUSCATE("SAMPBlack");

	if (!strncmp(tex, OBFUSCATE("carpet19-128x128"), sizeof(("carpet19-128x128"))))
		tex = OBFUSCATE("Carpet19-128x128");

	if (!pObj->m_pMaterials[matId].pTex && strncmp(tex, OBFUSCATE("none"), sizeof(("none"))) && strncmp(tex, OBFUSCATE("wall8"), sizeof(("wall8"))))
		pObj->m_pMaterials[matId].pTex = LoadTextureForReTexture(tex);

	if (!GetModelReferenceCount(modelId) && pGame->IsModelLoaded(modelId))
		ScriptCommand(&release_model, modelId);
}

void ScrSetVehicleZAngle(RPCParameters* rpcParams)
{
	Log(OBFUSCATE("RPC: ScrSetVehicleZAngle"));

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	VEHICLEID VehicleId;
	float fZAngle;
	bsData.Read(VehicleId);
	bsData.Read(fZAngle);
	CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(VehicleId);
	if (!pVeh) return;
	if (GamePool_Vehicle_GetAt(pVeh->m_dwGTAId))
	{
		ScriptCommand(&set_car_z_angle, pVeh->m_dwGTAId, fZAngle);
	}
}

void ScrAttachTrailerToVehicle(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID TrailerID, VehicleID;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(TrailerID);
	bsData.Read(VehicleID);
	CVehicle* pTrailer = pNetGame->GetVehiclePool()->GetAt(TrailerID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	if (!pVehicle) return;
	if (!pTrailer) return;
	pVehicle->SetTrailer(pTrailer);
	pVehicle->AttachTrailer();
}

//----------------------------------------------------

void ScrDetachTrailerFromVehicle(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID VehicleID;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(VehicleID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	if (!pVehicle) return;
	pVehicle->DetachTrailer();
	pVehicle->SetTrailer(NULL);
}

void ScrRemoveComponent(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	
	VEHICLEID vehicleId;
	uint16_t component;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(vehicleId);
	bsData.Read(component);

	if (pNetGame->GetVehiclePool())
	{
		if (pNetGame->GetVehiclePool()->GetAt(vehicleId))
		{
			pNetGame->GetVehiclePool()->GetAt(vehicleId)->RemoveComponent(component);
		}
	}
}

void ScrMoveObject(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	unsigned short byteObjectID;
	float curx, cury, curz, newx, newy, newz, speed, rotx, roty, rotz;

	bsData.Read(byteObjectID);
	bsData.Read(curx);
	bsData.Read(cury);
	bsData.Read(curz);
	bsData.Read(newx);
	bsData.Read(newy);
	bsData.Read(newz);
	bsData.Read(speed);
	bsData.Read(rotx);
	bsData.Read(roty);
	bsData.Read(rotz);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->MoveTo(newx, newy, newz, speed, rotx, roty, rotz);
	}
}

void ScrStopObject(RPCParameters *rpcParams) 
{
	Log("RPC: StopObject");
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	int16_t ObjectID;
	bsData.Read(ObjectID);
	
	CObject* pObject = pNetGame->GetObjectPool()->GetAt(ObjectID);
	if(pObject) pObject->StopMoving();
}

void ScrSetObjectRotation(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	uint16_t objectId;
	VECTOR vecRot;
	
	bsData.Read(objectId);
	bsData.Read((char*)&vecRot, sizeof(VECTOR));

	if (pNetGame->GetObjectPool()->GetAt(objectId))
	{
		pNetGame->GetObjectPool()->GetAt(objectId)->InstantRotate(vecRot.X, vecRot.Y, vecRot.Z);
	}
}

void ScrSelectTextDraw(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	bool bEnable = false;
	uint32_t dwColor = 0;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(bEnable);
	bsData.Read(dwColor);

	pNetGame->GetTextDrawPool()->SetSelectState(bEnable ? true : false, dwColor);
}

void ScrPlayAudioStream(RPCParameters* rpcParams)
{
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteTextLen;
	char szURL[1024];

	float X, Y, Z;
	float fRadius;

	bool bUsePos;

	bsData.Read(byteTextLen);
	bsData.Read(szURL, byteTextLen);

	bsData.Read(X);
	bsData.Read(Y);
	bsData.Read(Z);

	bsData.Read(fRadius);

	bsData.Read(bUsePos);

	szURL[byteTextLen] = '\0';

	if (pAudioStream)
	{
		pAudioStream->PlayByURL(szURL, X, Y, Z, fRadius, bUsePos);
		if (pChatWindow) {
			pChatWindow->AddInfoMessage(OBFUSCATE("Audio Stream: {ffff00}%s"), szURL);
		}
	}
	else
	{
		if (pChatWindow) {
			pChatWindow->AddInfoMessage(OBFUSCATE("Failed to play Sound! Audio Stream is not initialized!"));
		}
	}
}

void ScrStopAudioStream(RPCParameters* rpcParams)
{
	if (pAudioStream) pAudioStream->Stop();
}

void ScrSetPlayerArmed(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	uint32_t weaponID;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(weaponID);

	pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetArmedWeapon(weaponID);
}
void ScrDeathMessage(RPCParameters* rpcParams)
{
    Log(OBFUSCATE("RPC: DeathMessage"));
    unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
    int iBitLength = rpcParams->numberOfBitsOfData;
    RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

    PLAYERID playerId, killerId;
    uint8_t reason; 

    bsData.Read(killerId);
    bsData.Read(playerId);
    bsData.Read(reason);

    std::string killername, playername;
    uint32_t killercolor, playercolor;
    killername.clear();
    playername.clear();

    CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
    if(pPlayerPool)
    {
        if(pPlayerPool->GetLocalPlayerID() == playerId)
        {
            playername = pPlayerPool->GetLocalPlayerName();
            playercolor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
        }
        else
        {
            if(pPlayerPool->GetSlotState(playerId))
            {
                playername = pPlayerPool->GetPlayerName(playerId);
                playercolor = pPlayerPool->GetAt(playerId)->GetPlayerColorAsARGB();
            }
			else
			{
				playername = "  ";
				
				if(pPlayerPool->GetLocalPlayerID() != killerId && !pPlayerPool->GetSlotState(killerId))
				{
					killername = "  ";
					reason = 255;
				}
			}
        }

        if(pPlayerPool->GetLocalPlayerID() == killerId)
        {
            killername = pPlayerPool->GetLocalPlayerName();
            killercolor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
        }
        else
        {
            if(pPlayerPool->GetSlotState(killerId))
            {
                killername = pPlayerPool->GetPlayerName(killerId);
                killercolor = pPlayerPool->GetAt(killerId)->GetPlayerColorAsARGB();
            }
			else
			{
				killername = "  ";
			}
        }
    }

   pKillList->MakeRecord(playername.c_str(), playercolor, killername.c_str(), killercolor, reason);
}

void ScrSetPlayerDrunkLevel(RPCParameters * rpcParams)
{
	Log("RPC: ScrSetPlayerDrunkLevel");
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	uint32_t dwLevel;

	bsData.Read(dwLevel);

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed)
		pPlayerPed->SetDrunkLevel(dwLevel);
}

void ScrSetPlayerDrunkVisuals(RPCParameters* rpcParams)
{
	Log("RPC: ScrSetPlayerDrunkVisuals");
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	uint32_t dwVisuals;

	bsData.Read(dwVisuals);

	ScriptCommand(&set_player_drunk_visuals, 0, dwVisuals);
}

void ScrSetPlayerDrunkHandling(RPCParameters* rpcParams)
{
	Log("RPC: ScrSetPlayerDrunkHandling");
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	uint32_t dwHandling;

	bsData.Read(dwHandling);

	ScriptCommand(&set_player_drunk_handling, 0, dwHandling);
}

void UpdateSpeedometer(RPCParameters *rpcParams)
{
	auto *Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = (int)rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	uint8_t element = -1;

	bool beltState = false;

	float mileage = 0;
	float fuel = 0;

	bsData.Read(element);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	switch (element) {
		case 0:
			bsData.Read(beltState);
			g_pJavaWrapper->SetBeltState(beltState ? 1 : 0);

			Log("RPC: UpdateSpeedometer %d %d", element, beltState);
			break;
		case 1:
			bsData.Read(mileage);
			g_pJavaWrapper->SetSpeedometerMileage((int)mileage);

			Log("RPC: UpdateSpeedometer %d %d", element, mileage);
			break;
		case 2:
			bsData.Read(fuel);
			if (pLocalPlayer)
			{
				if (pVehiclePool)
				{
					CVehicle *pVehicle = pVehiclePool->GetAt(pVehiclePool->FindIDFromGtaPtr(pGame->FindPlayerPed()->GetGtaVehicle()));
					if (pVehicle)
					{
						int vehId = pVehicle->GetModelIndex();

						if (vehId == 466 || vehId == 576)
							fuel += 1000;
					}
				}
			}

			g_pJavaWrapper->SetSpeedometerFuel((int)fuel);

			Log("RPC: UpdateSpeedometer %d %d", element, fuel);
			break;
		default:
			Log("RPC: UpdateSpeedometer ����� (%d)", element);
	}
}

void RegisterScriptRPCs(RakClientInterface* pRakClient)
{
	Log(OBFUSCATE("Registering ScriptRPC's.."));
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDisplayGameText, ScrDisplayGameText);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetGravity, ScrSetGravity);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrForceSpawnSelection,ScrForceSpawnSelection);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPos, ScrSetPlayerPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraPos, ScrSetCameraPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraLookAt, ScrSetCameraLookAt);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerFacingAngle, ScrSetPlayerFacingAngle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetFightingStyle, ScrSetFightingStyle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkin, ScrSetPlayerSkin);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrApplyPlayerAnimation, ScrApplyPlayerAnimation);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrClearPlayerAnimations, ScrClearPlayerAnimations);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetSpawnInfo, ScrSetSpawnInfo);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrCreateExplosion, ScrCreateExplosion);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerHealth, ScrSetPlayerHealth);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerArmour, ScrSetPlayerArmour);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerColor, ScrSetPlayerColor);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerName, ScrSetPlayerName);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPosFindZ, ScrSetPlayerPosFindZ);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetInterior, ScrSetPlayerInterior);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetMapIcon, ScrSetMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDisableMapIcon, ScrDisableMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraBehindPlayer, ScrSetCameraBehindPlayer);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetSpecialAction, ScrSetPlayerSpecialAction);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerSpectating, ScrTogglePlayerSpectating);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSpectating, ScrSetPlayerSpectating);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectatePlayer, ScrPlayerSpectatePlayer);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectateVehicle, ScrPlayerSpectateVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPutPlayerInVehicle, ScrPutPlayerInVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrVehicleParams, ScrVehicleParams);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrVehicleParamsEx, ScrVehicleParamsEx);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrHaveSomeMoney, ScrHaveSomeMoney);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrResetMoney, ScrResetMoney);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrLinkVehicle, ScrLinkVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrRemovePlayerFromVehicle, ScrRemovePlayerFromVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleHealth, ScrSetVehicleHealth);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehiclePos, ScrSetVehiclePos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleVelocity, ScrSetVehicleVelocity);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrNumberPlate, ScrNumberPlate);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrInterpolateCamera, ScrInterpolateCamera);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrAddGangZone,ScrAddGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrRemoveGangZone,ScrRemoveGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrFlashGangZone,ScrFlashGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrStopFlashGangZone,ScrStopFlashGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrCreateObject, ScrCreateObject);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetObjectPos, ScrSetObjectPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDestroyObject, ScrDestroyObject);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlaySound, ScrPlaySound);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerWantedLevel, ScrSetPlayerWantedLevel);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrGivePlayerWeapon, ScrGivePlayerWeapon);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerControllable, ScrTogglePlayerControllable);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrAttachObjectToPlayer, ScrAttachObjectToPlayer);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrResetPlayerWeapons, ScrResetPlayerWeapons);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkillLevel, ScrSetPlayerSkillLevel);
	//Kill list
    pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDeathMessage, ScrDeathMessage);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrShowTextDraw, ScrShowTextDraw);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrHideTextDraw, ScrHideTextDraw);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrEditTextDraw, ScrEditTextDraw);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerAttachedObject, ScrSetPlayerAttachedObject);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerObjectMaterial, ScrSetPlayerObjectMaterial);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleZAngle, ScrSetVehicleZAngle);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrAttachTrailerToVehicle, ScrAttachTrailerToVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDetachTrailerFromVehicle, ScrDetachTrailerFromVehicle);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrRemoveComponent, ScrRemoveComponent);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ClickTextDraw, ScrSelectTextDraw);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrStopObject, ScrStopObject);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetObjectRotation, ScrSetObjectRotation);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_PlayAudioStream, ScrPlayAudioStream);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_StopAudioStream, ScrStopAudioStream);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_SetArmedWeapon, ScrSetPlayerArmed);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrMoveObject, ScrMoveObject);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkLevel, ScrSetPlayerDrunkLevel);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkVisuals, ScrSetPlayerDrunkVisuals);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkHandling, ScrSetPlayerDrunkHandling);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_UpdateSpeedometer, UpdateSpeedometer);
}

void UnRegisterScriptRPCs(RakClientInterface* pRakClient)
{
	Log(OBFUSCATE("Unregistering ScriptRPC's.."));

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ClickTextDraw);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrAttachTrailerToVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrDetachTrailerFromVehicle);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetVehicleZAngle);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerObjectMaterial);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerAttachedObject);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrEditTextDraw);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrShowTextDraw);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrHideTextDraw);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkillLevel);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrResetPlayerWeapons);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrGivePlayerWeapon);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrDisplayGameText);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetGravity);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrForceSpawnSelection);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPos);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetCameraPos);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetCameraLookAt);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerFacingAngle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetFightingStyle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkin);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrApplyPlayerAnimation);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrClearPlayerAnimations);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetSpawnInfo);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrCreateExplosion);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerHealth);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerArmour);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerColor);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerName);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPosFindZ);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetInterior);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetMapIcon);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrDisableMapIcon);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetCameraBehindPlayer);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetSpecialAction);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerSpectating);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSpectating);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectatePlayer);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectateVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrPutPlayerInVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrVehicleParams);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrVehicleParamsEx);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrHaveSomeMoney);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrResetMoney);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrLinkVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrRemovePlayerFromVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetVehicleHealth);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetVehiclePos);
	//KillList
    pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrDeathMessage);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetVehicleVelocity);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrNumberPlate);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrInterpolateCamera);
	
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrAddGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrRemoveGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrFlashGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrStopFlashGangZone);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrCreateObject);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrDestroyObject);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetObjectPos);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrPlaySound);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerWantedLevel);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrRemoveComponent);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrStopObject);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetObjectRotation);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_PlayAudioStream);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_StopAudioStream);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_SetArmedWeapon);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrMoveObject);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkLevel);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkVisuals);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkHandling);

	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_UpdateSpeedometer);
}