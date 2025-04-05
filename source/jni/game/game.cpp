#include "../main.h"
#include "game.h"
#include "../chatwindow.h"

#include "../util/armhook.h"
#include "../util/CJavaWrapper.h"

#include "../graphics/CSkyBox.h"

void ApplyPatches();
void ApplyInGamePatches();
void InstallHooks();
void LoadSplashTexture();
void InitScripting();

uint16_t *szGameTextMessage;
bool bUsedPlayerSlots[PLAYER_PED_SLOTS];
extern CChatWindow* pChatWindow;

void CGame::RemoveModel(int iModel, bool bFromStreaming)
{
	if (iModel >= 0 && iModel < 20000)
	{
		if (bFromStreaming)
		{
			if (ScriptCommand(&is_model_available, iModel))
				// CStreaming::RemoveModel
				((void(*)(int))(SA_ADDR(0x290C4C + 1)))(iModel);
		}
		else
		{
			if (ScriptCommand(&is_model_available, iModel))
				ScriptCommand(&release_model);
		}
	}
}

extern char* PLAYERS_REALLOC;
CGame::CGame()
{
	for (int i = 0; i < HUD_MAX; i++)
	{
		aToggleStatusHUD[i] = true;
	}
	m_pGameCamera = new CCamera();
	m_pGamePlayer = nullptr;

	m_bClockEnabled = true;
	memset(&m_checkpointData, 0, sizeof(m_checkpointData));

	m_bRaceCheckpointsEnabled = 0;
	m_dwRaceCheckpointHandle = 0;
	m_dwRaceCheckpointMarker = 0;

	memset(&bUsedPlayerSlots[0], 0, PLAYER_PED_SLOTS);
}

// 0.3.7
uint8_t CGame::FindFirstFreePlayerPedSlot()
{
	uint8_t x = 2;
	while(x != PLAYER_PED_SLOTS)
	{
		if(!bUsedPlayerSlots[x]) return x;
		x++;
	}

	return 0;
}

// 0.3.7
CPlayerPed* CGame::NewPlayer(int iSkin, float fX, float fY, float fZ, float fRotation, uint8_t byteCreateMarker)
{
	uint8_t bytePlayerNum = FindFirstFreePlayerPedSlot();
	if(!bytePlayerNum) return nullptr;

	CPlayerPed* pPlayerNew = new CPlayerPed(bytePlayerNum, iSkin, fX, fY, fZ, fRotation);
	if(pPlayerNew && pPlayerNew->m_pPed)
		bUsedPlayerSlots[bytePlayerNum] = true;

	//if(byteCreateMarker) (no xrefs ;( )
	return pPlayerNew;
}

// 0.3.7
void CGame::RemovePlayer(CPlayerPed* pPlayer)
{
	if(pPlayer)
	{
		bUsedPlayerSlots[pPlayer->m_bytePlayerNumber] = false;
		delete pPlayer;
	}
}

// 0.3.7
CVehicle* CGame::NewVehicle(int iType, float fPosX, float fPosY, float fPosZ, float fRotation, bool bAddSiren)
{
	CVehicle *pVehicleNew = new	CVehicle(iType, fPosX, fPosY, fPosZ, fRotation, bAddSiren);
	return pVehicleNew;
}

CObject *CGame::NewObject(int iModel, float fPosX, float fPosY, float fPosZ, VECTOR vecRot, float fDrawDistance)
{
	CObject *pObjectNew = new CObject(iModel, fPosX, fPosY, fPosZ, vecRot, fDrawDistance);
	return pObjectNew;
}

uint32_t CGame::CreatePickup(int iModel, int iType, float fX, float fY, float fZ, int* unk)
{
	Log(OBFUSCATE("CreatePickup(%d, %d, %4.f, %4.f, %4.f)"), iModel, iType, fX, fY, fZ);

	uint32_t hnd;

	if(iModel > 0 && iModel < 20000)
	{
		uintptr_t *dwModelArray = (uintptr_t*)(SA_ADDR(0x87BF48));
    	if(dwModelArray[iModel] == 0)
    		iModel = 18631;
	}
	else iModel = 18631;

	if(!ScriptCommand(&is_model_available, iModel))
	{
		ScriptCommand(&request_model, iModel);
		ScriptCommand(&load_requested_models);
		while(!ScriptCommand(&is_model_available, iModel))
			usleep(1000);
	}

	ScriptCommand(&create_pickup, iModel, iType, fX, fY, fZ, &hnd);

	int lol = 32 * (uint16_t)hnd;
	if(lol) lol /= 32;
	if(unk) *unk = lol;

	return hnd;
}

void CGame::InitInMenu()
{
	//Log(OBFUSCATE("CGame: InitInMenu"));

                  Log(OBFUSCATE("Inject new graphics settings[5].."));
                  Log(OBFUSCATE("Inject shaders.."));
	*(int32_t *)(g_libGTASA + 0x63E384) = 1;

                  Log(OBFUSCATE("Inject pixels.."));
	*(int32_t *)(g_libGTASA + 0x63E3A4) = 100;

                  Log(OBFUSCATE("Inject draw distance.."));
	*(int32_t *)(g_libGTASA + 0x63E3C4) = 0; 

                  Log(OBFUSCATE("Inject shadows.."));
	*(int32_t *)(g_libGTASA + 0x63E424) = 0;

                  Log(OBFUSCATE("Inject reflection.."));
	*(int32_t *)(g_libGTASA + 0x63E464) = 3; 


                  /* reversing by t.me/weikton
                    dword_63E384 = 3; // shaders (������ - 0, ������� - 1, ������� - 2, ������������ - 3)
                    dword_63E3A4 = 80; // pixels (max: 100)
                    dword_63E3C4 = 100; // draw distance (max: 100)
                    dword_63E424 = 2; // shadow (max: 3)
                    dword_63E464 = 2; // reflect (max: 3)
                  */
	
	ApplyPatches();
	InstallHooks();
	GameAimSyncInit();
	LoadSplashTexture();

	szGameTextMessage = new uint16_t[1076];
}

void CGame::InitInGame()
{
	Log(OBFUSCATE("CGame: InitInGame"));

	ApplyInGamePatches();
	InitScripting();
	
	GameResetRadarColors();
}

void CGame::ToggleHUDElement(int iID, bool bToggle)
{
	if (iID < 0 || iID >= HUD_MAX)
	{
		return;
	}
	aToggleStatusHUD[iID] = bToggle;
}

bool CGame::IsToggledHUDElement(int iID)
{
	if (iID < 0 || iID >= HUD_MAX)
	{
		return 1;
	}
	return aToggleStatusHUD[iID];
}

void CGame::HandleChangedHUDStatus()
{
	DisplayHUD(aToggleStatusHUD[HUD_ELEMENT_HUD]);
	ToggleRadar(aToggleStatusHUD[HUD_ELEMENT_MAP]);
	DisplayWidgets(aToggleStatusHUD[HUD_ELEMENT_BUTTONS]);
}

void CGame::SetEnabledPCMoney(bool bEnabled)
{
	if (bEnabled)
		WriteMemory(SA_ADDR(0x599510), (uintptr_t)"$%08d", 6);
	else 
                                    WriteMemory(SA_ADDR(0x599510), (uintptr_t)"$%d", 6);
}

float CGame::FindGroundZForCoord(float x, float y, float z)
{
	float fGroundZ;
	ScriptCommand(&get_ground_z, x, y, z, &fGroundZ);
	return fGroundZ;
}

// 0.3.7
void CGame::SetCheckpoint(VECTOR *pos, VECTOR *extent)
{
	if (!pos || !extent) return;

	if (m_checkpointData.m_bIsActive)
	{
		DisableCheckpoint();
	}

	m_checkpointData.m_vecPosition.X = pos->X;
	m_checkpointData.m_vecPosition.Y = pos->Y;
	m_checkpointData.m_vecPosition.Z = pos->Z;

	m_checkpointData.m_vecExtent.X = extent->X;
	m_checkpointData.m_vecExtent.Y = extent->Y;
	m_checkpointData.m_vecExtent.Z = extent->Z;

	m_checkpointData.m_dwMarkerId = CreateRadarMarkerIcon(0, pos->X,
		pos->Y, pos->Z, 1005, 0);

	m_checkpointData.m_bIsActive = true;
}

void CGame::DisableCheckpoint()
{
	if (m_checkpointData.m_bIsActive)
	{
		DisableMarker(m_checkpointData.m_dwMarkerId);
		m_checkpointData.m_bIsActive = false;
	}
}

// 0.3.7
void CGame::SetRaceCheckpointInformation(uint8_t byteType, VECTOR *pos, VECTOR *next, float fSize)
{
	memcpy(&m_vecRaceCheckpointPos,pos,sizeof(VECTOR));
	memcpy(&m_vecRaceCheckpointNext,next,sizeof(VECTOR));
	m_fRaceCheckpointSize = fSize;
	m_byteRaceType = byteType;

	if(m_dwRaceCheckpointMarker)
	{
		DisableMarker(m_dwRaceCheckpointMarker);

		m_dwRaceCheckpointMarker = NULL;

		m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X,
			m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, 1005, 0);
	}

	MakeRaceCheckpoint();
}

// 0.3.7
void CGame::MakeRaceCheckpoint()
{
	DisableRaceCheckpoint();

	ScriptCommand(&create_racing_checkpoint, (int)m_byteRaceType,
				m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z,
				m_vecRaceCheckpointNext.X, m_vecRaceCheckpointNext.Y, m_vecRaceCheckpointNext.Z,
				m_fRaceCheckpointSize, &m_dwRaceCheckpointHandle);

	m_bRaceCheckpointsEnabled = true;
}

// 0.3.7
void CGame::DisableRaceCheckpoint()
{
	if (m_dwRaceCheckpointHandle)
	{
		ScriptCommand(&destroy_racing_checkpoint, m_dwRaceCheckpointHandle);
		m_dwRaceCheckpointHandle = NULL;
	}
	m_bRaceCheckpointsEnabled = false;
}

// 0.3.7
void CGame::UpdateCheckpoints()
{
	if(m_checkpointData.m_bIsActive) 
	{
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed) 
		{
			if (ScriptCommand(&is_actor_near_point_3d, pPlayerPed->m_dwGTAId,
				m_checkpointData.m_vecPosition.X, m_checkpointData.m_vecPosition.Y, m_checkpointData.m_vecPosition.Z,
				m_checkpointData.m_vecExtent.X, m_checkpointData.m_vecExtent.Y, m_checkpointData.m_vecExtent.Z, 1))
			{
				DisableCheckpoint();
			}
		}
	}
	
	if(m_bRaceCheckpointsEnabled) 
	{
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed)
		{
			if (!m_dwRaceCheckpointMarker)
			{
				m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X,
					m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, 1005, 0);
			}
		}
	}
	else if(m_dwRaceCheckpointMarker) 
	{
		DisableMarker(m_dwRaceCheckpointMarker);
		DisableRaceCheckpoint();
		m_dwRaceCheckpointMarker = 0;
	}
}


// 0.3.7
uint32_t CGame::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle)
{
	uint32_t dwMarkerID = 0;

	if(iStyle == 1) 
		ScriptCommand(&create_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
	else if(iStyle == 2) 
		ScriptCommand(&create_radar_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
	else if(iStyle == 3) 
		ScriptCommand(&create_icon_marker_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	else 
		ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);

	if(iMarkerType == 0)
	{
		if(iColor >= 1004)
		{
			ScriptCommand(&set_marker_color, dwMarkerID, iColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 3);
		}
		else
		{
			ScriptCommand(&set_marker_color, dwMarkerID, iColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 2);
		}
	}

	return dwMarkerID;
}

// 0.3.7
uint8_t CGame::GetActiveInterior()
{
	uint32_t dwRet;
	ScriptCommand(&get_active_interior, &dwRet);
	return (uint8_t)dwRet;
}

// 0.3.7
void CGame::SetWorldTime(int iHour, int iMinute)
{
	*(uint8_t*)(SA_ADDR(0x8B18A4)) = (uint8_t)iMinute;
	*(uint8_t*)(SA_ADDR(0x8B18A5)) = (uint8_t)iHour;

	ScriptCommand(&set_current_time, iHour, iMinute);
}

// 0.3.7
void CGame::SetWorldWeather(unsigned char byteWeatherID)
{
	*(unsigned char*)(SA_ADDR(0x9DB98E)) = byteWeatherID;

	if(!m_bClockEnabled)
	{
		*(uint16_t*)(SA_ADDR(0x9DB990)) = byteWeatherID;
		*(uint16_t*)(SA_ADDR(0x9DB992)) = byteWeatherID;
	}
}

void CGame::ToggleThePassingOfTime(bool bOnOff)
{
	if(bOnOff)
		WriteMemory(SA_ADDR(0x38C154), (uintptr_t)"\x2D\xE9", 2);
	else 
                                    WriteMemory(SA_ADDR(0x38C154), (uintptr_t)"\xF7\x46", 2);
}

// 0.3.7
void CGame::EnableClock(bool bEnable)
{
	char byteClockData[] = { '%', '0', '2', 'd', ':', '%', '0', '2', 'd', 0 };
	unProtect(SA_ADDR(0x599504));

	if(bEnable)
	{
		ToggleThePassingOfTime(true);
		m_bClockEnabled = true;
		memcpy((void*)(SA_ADDR(0x599504)), byteClockData, 10);
	}
	else
	{
		ToggleThePassingOfTime(false);
		m_bClockEnabled = false;
		memset((void*)(SA_ADDR(0x599504)), 0, 10);
	}
}

// 0.3.7
void CGame::EnableZoneNames(bool bEnable)
{
	ScriptCommand(&enable_zone_names, bEnable);
}

void CGame::DisplayWidgets(bool bDisp)
{
	if(bDisp)
		*(uint16_t*)(g_libGTASA + 0x8B82A0 + 0x10C) = 0;
	else 
                                    *(uint16_t*)(g_libGTASA + 0x8B82A0 + 0x10C) = 1;

	if(!bDisp)
	{
		if (g_pJavaWrapper->isGlobalShowSpeedometer)
			g_pJavaWrapper->ShowSpeedometer();

		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->ShowHUD(false);
	}
	else
	{
		if (g_pJavaWrapper->isGlobalShowSpeedometer)
			g_pJavaWrapper->HideSpeedometer();

		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->HideHUD(false);
	}
}

// ��������
void CGame::PlaySound(int iSound, float fX, float fY, float fZ)
{
	ScriptCommand(&play_sound, fX, fY, fZ, iSound);
}

void CGame::ToggleRadar(bool iToggle)
{
	*(uint8_t*)(SA_ADDR(0x8EF36B)) = (uint8_t)!iToggle;
}

void CGame::DisplayHUD(bool bDisp)
{
	if(bDisp)
	{	
		// CTheScripts11bDisplayHud
		*(uint8_t*)(SA_ADDR(0x7165E8)) = 1;
		ToggleRadar(true);
	}
	else
	{
		*(uint8_t*)(SA_ADDR(0x7165E8)) = 0;
		ToggleRadar(false);
	}
}

// 0.3.7
void CGame::RequestModel(unsigned int iModelID, int iLoadingStream)
{
	ScriptCommand(&request_model, iModelID);
}

// 0.3.7
void CGame::LoadRequestedModels()
{
	ScriptCommand(&load_requested_models);
}

// 0.3.7
uint8_t CGame::IsModelLoaded(unsigned int iModelID)
{
	return ScriptCommand(&is_model_available, iModelID);
}

// 0.3.7
void CGame::RefreshStreamingAt(float x, float y)
{
	ScriptCommand(&refresh_streaming_at, x, y);
}

// 0.3.7
void CGame::DisableTrainTraffic()
{
	ScriptCommand(&enable_train_traffic,0);
}

// 0.3.7
void CGame::SetMaxStats()
{
	// CCheat::VehicleSkillsCheat
	(( int (*)())(SA_ADDR(0x2BAED0 + 1)))();

	// CCheat::WeaponSkillsCheat
	(( int (*)())(SA_ADDR(0x2BAE68 + 1)))();

	// CStats::SetStatValue nop
	WriteMemory(SA_ADDR(0x3B9074), (uintptr_t)"\xF7\x46", 2);
}

void CGame::SetWantedLevel(uint8_t byteLevel)
{
	WriteMemory(SA_ADDR(0x27D8D2), (uintptr_t)&byteLevel, 1);
	g_pJavaWrapper->SetWantedLevel((int)byteLevel);
}

bool CGame::IsAnimationLoaded(char *szAnimFile)
{
	return ScriptCommand(&is_animation_loaded, szAnimFile);
}

void CGame::RequestAnimation(char *szAnimFile)
{
	ScriptCommand(&request_animation, szAnimFile);
}

// 0.3.7
void CGame::DisplayGameText(char* szStr, int iTime, int iType)
{
	ScriptCommand(&text_clear_all);
	/*if(strcmp(szStr,"LIGHT OFF")) {
		g_pJavaWrapper->SetLightState(0);
		Log(OBFUSCATE("DisplayGameText OFF"));
	}	
	//Log(OBFUSCATE("MIAHIL DisplayGameText  light OFF"));
	if(strcmp(szStr,"LIGHT ON")) {
		g_pJavaWrapper->SetLightState(1);
		Log(OBFUSCATE("DisplayGameText ON"));
	
	}	*/
	CFont::AsciiToGxtChar(szStr, szGameTextMessage);
	// CMessages::AddBigMesssage
	(( void (*)(uint16_t*, int, int))(SA_ADDR(0x4D18C0 + 1)))(szGameTextMessage, iTime, iType);
	g_pJavaWrapper->CallLauncherActivity(1234);
}

// 0.3.7
void CGame::SetGravity(float fGravity)
{
	unProtect(SA_ADDR(0x3A0B64));
	*(float*)(SA_ADDR(0x3A0B64)) = fGravity;
}

void CGame::ToggleCJWalk(bool bUseCJWalk)
{
	if(bUseCJWalk)
		WriteMemory(SA_ADDR(0x45477E), (uintptr_t)"\xC4\xF8\xDC\x64", 4);
	else makeNOP(SA_ADDR(0x45477E), 2);
}

void CGame::DisableMarker(uint32_t dwMarkerID)
{
	ScriptCommand(&disable_marker, dwMarkerID);
}

// 0.3.7
int CGame::GetLocalMoney()
{
	return *(int*)(PLAYERS_REALLOC+0xB8);
}

// 0.3.7
void CGame::AddToLocalMoney(int iAmmount)
{
	ScriptCommand(&add_to_player_money, 0, iAmmount);
}

// 0.3.7
void CGame::ResetLocalMoney()
{
	int iMoney = GetLocalMoney();
	if(!iMoney) return;

	if(iMoney < 0)
		AddToLocalMoney(abs(iMoney));
	else
		AddToLocalMoney(-(iMoney));
}
// 0.3.7
void CGame::DisableInteriorEnterExits()
{
	uintptr_t addr = *(uintptr_t*)(SA_ADDR(0x700120));
	int count = *(uint32_t*)(addr + 8);
	Log(OBFUSCATE("Count = %d"), count);

	addr = *(uintptr_t*)addr;

	for(int i=0; i<count; i++)
	{
		*(uint16_t*)(addr + 0x30) = 0;
		addr += 0x3C;
	}
}

extern uint8_t bGZ;
void CGame::DrawGangZone(float fPos[], uint32_t dwColor)
{
    (( void (*)(float*, uint32_t*, uint8_t))(SA_ADDR(0x3DE7F8 + 1)))(fPos, &dwColor, bGZ);
}

uint8_t CGame::IsGamePaused()
{
	return *(uint8_t*)(SA_ADDR(0x8C9BA3));
}