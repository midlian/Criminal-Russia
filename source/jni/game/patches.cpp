#include "../main.h"
#include "../util/armhook.h"
char* PLAYERS_REALLOC = nullptr;
#include "../CSettings.h"
#include "..//CDebugInfo.h"
#include "game.h"
#include "CExtendedCarColors.h"
extern CSettings* pSettings;

void WriteUnVerified0();
void InitInGame();

VehicleAudioPropertiesStruct VehicleAudioProperties[20000];

void readVehiclesAudioSettings() 
{
	char vehicleModel[50];
	int16_t pIndex = 0;

	FILE* pFile;

	char line[300];
	memset(VehicleAudioProperties, 0x00, sizeof(VehicleAudioProperties));

	VehicleAudioPropertiesStruct CurrentVehicleAudioProperties;
	memset(&CurrentVehicleAudioProperties, 0x0, sizeof(VehicleAudioPropertiesStruct));

	char buffer[0xFF];
	sprintf(buffer, "%sSAMP/vehicleAudioSettings.cfg", g_pszStorage);
	pFile = fopen(buffer, "r");
	if (!pFile)
	{
		Log("Cannot read vehicleAudioSettings.cfg");
		return;
	}

	while (fgets(line, sizeof(line), pFile))
	{
		if (strncmp(line, ";the end", 8) == 0)
			break;

		if (line[0] == ';')
			continue;

		sscanf(line, "%s %d %d %d %d %f %f %d %f %d %d %d %d %f",
			vehicleModel,
			&CurrentVehicleAudioProperties.VehicleType,
			&CurrentVehicleAudioProperties.EngineOnSound,
			&CurrentVehicleAudioProperties.EngineOffSound,
			&CurrentVehicleAudioProperties.field_4,
			&CurrentVehicleAudioProperties.field_5,
			&CurrentVehicleAudioProperties.field_6,
			&CurrentVehicleAudioProperties.HornTon,
			&CurrentVehicleAudioProperties.HornHigh,
			&CurrentVehicleAudioProperties.DoorSound,
			&CurrentVehicleAudioProperties.RadioNum,
			&CurrentVehicleAudioProperties.RadioType,
			&CurrentVehicleAudioProperties.field_14,
			&CurrentVehicleAudioProperties.field_16);

		int result = ((int (*)(const char* thiz, int16_t* a2))(g_libGTASA + 0x00336110 + 1))(vehicleModel, &pIndex);
		memcpy(&VehicleAudioProperties[pIndex-400], &CurrentVehicleAudioProperties, sizeof(VehicleAudioPropertiesStruct));
	}

	fclose(pFile);
}

int test_pointsArray[4000];
int test_pointersLibArray[4000];

void ApplyPatches_level0()
{
	// unProtect(g_libGTASA + 0x005CE80C);
  	// *(uintptr_t*)(g_libGTASA + 0x005CE80C) = (uintptr_t)CStreaming_ms_files_ptr;

	// unProtect(g_libGTASA + 0x006702FC);
  	// *(uintptr_t*)(g_libGTASA + 0x006702FC) = (uintptr_t)CStreaming_ms_files_ptr;

	PLAYERS_REALLOC = ((char* (*)(uint32_t))(g_libGTASA + 0x179B40))(404 * 257 * sizeof(char));
	memset(PLAYERS_REALLOC, 0, 404 * 257);
	unProtect(g_libGTASA + 0x5D021C);
	*(char**)(g_libGTASA + 0x5D021C) = PLAYERS_REALLOC;

                  // .. 3TOUCH	
	unProtect(g_libGTASA + 0x5D1E8C);
  	*(uintptr_t*)(g_libGTASA + 0x5D1E8C) = (uintptr_t)test_pointsArray;
	
	unProtect(g_libGTASA + 0x5CEA8C);
  	*(uintptr_t*)(g_libGTASA + 0x5CEA8C) = (uintptr_t)test_pointersLibArray;

	WriteMemory(g_libGTASA + 0x00238232, (uintptr_t)"\x03\x20", 2);
	WriteMemory(g_libGTASA + 0x0025C522, (uintptr_t)"\x02\x2C", 2);
	// .. 

	// ... COL
	WriteMemory(g_libGTASA + 0x1859FC, (uintptr_t)"\x01\x22", 2);

	WriteMemory(g_libGTASA + 0x0029554A, (uintptr_t)"\x4f\xf4\x61\x60", 4); // allocate memory to 300 * sizeof(CCollisionLink)
	WriteMemory(g_libGTASA + 0x00295556, (uintptr_t)"\x4f\xf4\x5b\x62", 4); // BUT MAKE INITIALIZED ONLY 292 DUE TO SHIT ARM ASM \ (292 * sizeof(CCollisionLink)

	// ...

	CExtendedCarColors::ApplyPatches_level0();

	uint8_t fps = 120;
	//if (pSettings)
	//{
	//	fps = pSettings->GetReadOnly().iFPS;
	//}
	WriteMemory(g_libGTASA + 0x463FE8, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C1F6, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C126, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x95B074, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C1A2, (uintptr_t)& fps, 1);

	// NOP RANDOM PICKUP
	makeNOP(g_libGTASA + 0x00402472, 2);
	makeNOP(g_libGTASA + 0x003E1AF0, 2);

	// osMutexStuff
	WriteMemory(g_libGTASA + 0x001A7ECE, (uintptr_t)"\x4F\xF0\x01\x00\x00\x46", 6);

	// CdStreamInit(6);
	WriteMemory(g_libGTASA + 0x3981EC, (uintptr_t)"\x06\x20", 2);

	uintptr_t g_libSCAnd = FindLibrary(OBFUSCATE("libSCAnd.so"));
	if (g_libSCAnd)
	{
		unProtect(g_libSCAnd + 0x001E1738);
		unProtect(g_libSCAnd + 0x001E16DC);
		unProtect(g_libSCAnd + 0x001E080C);
		strcpy((char*)(g_libSCAnd + 0x001E16DC), OBFUSCATE("com/rockstargames/hal/andViewManager"));
		strcpy((char*)(g_libSCAnd + 0x001E1738), OBFUSCATE("staticExitSocialClub"));
		strcpy((char*)(g_libSCAnd + 0x001E080C), OBFUSCATE("()V"));
	}

	// HOOK CRASH
	WriteMemory(g_libGTASA + 0x00336618, (uintptr_t)"\x4F\xF4\x6A\x71", 4);
	WriteMemory(g_libGTASA + 0x33661C, (uintptr_t)"\x1A\x4B", 2);
	WriteMemory(g_libGTASA + 0x00274AB4, (uintptr_t)"\x00\x46", 2);

	// getmaxstorage
	WriteMemory(g_libGTASA + 0x001BD8DC, (uintptr_t)"\x4F\xF0\x48\x70", 2); // mov.w r0, #0x3200000

	WriteUnVerified0();

	// BLACK ROADS (когда дороги становятся при повороте черными / может быть в любой момент)
	WriteMemory(g_libGTASA + 0x1A69F8, (uintptr_t)"\x42\xF2\x05\x72", 4); // movw r2, #0x2705	
	WriteMemory(g_libGTASA + 0x1A6A1E, (uintptr_t)"\x42\xF2\x05\x72", 4); // movw r2, #0x2705	
	WriteMemory(g_libGTASA + 0x1A6A10, (uintptr_t)"\x42\xF2\x05\x72", 4); // movw r2, #0x2705	
	WriteMemory(g_libGTASA + 0x1A69E8, (uintptr_t)"\x42\xF2\x05\x72", 4); // movw r2, #0x2705	
}

struct _ATOMIC_MODEL
{
	uintptr_t func_tbl;
	char data[56];
};
struct _ATOMIC_MODEL *ATOMIC_MODELS;

void ApplyPatches()
{
	Log(OBFUSCATE("Installing patches.."));

	CDebugInfo::ApplyDebugPatches();

	ATOMIC_MODELS = new _ATOMIC_MODEL[20000];
	for (int i = 0; i < 20000; i++)
	{
		((void(*)(_ATOMIC_MODEL*))(g_libGTASA + 0x33559C + 1))(&ATOMIC_MODELS[i]);
		ATOMIC_MODELS[i].func_tbl = g_libGTASA + 0x5C6C68;
		memset(ATOMIC_MODELS[i].data, 0, sizeof(ATOMIC_MODELS->data));
	}

	// TEXTURES SIZE
	WriteMemory(g_libGTASA + 0x573640, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x573668, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x57367C, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x573690, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x5736C8, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x5736D8, (uintptr_t)"dxt", 3);
	WriteMemory(g_libGTASA + 0x5736E8, (uintptr_t)"dxt", 3);
	
	// Settings
	makeNOP(g_libGTASA + 0x266460, 2); // Game - TrafficMode
                  makeNOP(g_libGTASA + 0x2661DA, 2); // Display - Shadows (Фикс фпс на фоне теней)
                  makeNOP(g_libGTASA + 0x2665EE, 2); // Game - SocialClub

	// CWorld::FindPlayerSlotWithPedPointer
	makeNOP(g_libGTASA + 0x0045A4C8, 11);
	makeNOP(g_libGTASA + 0x0045A4E0, 3);

	// CAudioEngine::StartLoadingTune
	makeNOP(g_libGTASA + 0x56C150, 2);

	makeNOP(g_libGTASA + 0x402472, 2);

	// CRealTimeShadowManager::Update(void)
	makeNOP(g_libGTASA + 0x0039B2C0, 2);

	// DefaultPCSaveFileName
	char* DefaultPCSaveFileName = (char*)(g_libGTASA + 0x60EAE8);
	memcpy((char*)DefaultPCSaveFileName, OBFUSCATE("GTASAMP"), 8);

	// uint8_t fps = pSettings->GetReadOnly().iFPS;
                  uint8_t fps = 120;
	WriteMemory(g_libGTASA + 0x463FE8, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C1F6, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C126, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x95B074, (uintptr_t)& fps, 1);
	WriteMemory(g_libGTASA + 0x56C1A2, (uintptr_t)& fps, 1);

	// CWidgetRegionSteeringSelection::Draw
	WriteMemory(g_libGTASA + 0x284BB8, (uintptr_t)"\xF7\x46", 2);

	// CHud::SetHelpMessage
	WriteMemory(g_libGTASA + 0x3D4244, (uintptr_t)"\xF7\x46", 2);
	// CHud::SetHelpMessageStatUpdate
	WriteMemory(g_libGTASA + 0x3D42A8, (uintptr_t)"\xF7\x46", 2);

	// CVehicleRecording::Load
	WriteMemory(g_libGTASA + 0x2DC8E0, (uintptr_t)"\xF7\x46", 2);

                  // cFire::extinguish
	WriteMemory(g_libGTASA + 0x00458D68, (uintptr_t)"\x00\x46\x00\x46", 4);

	// CRealTimeShadowManager::ReturnRealTimeShadow from ~CPhysical
	makeNOP(g_libGTASA + 0x3A019C, 2);

	// Fix shadows crash (thx War Drum Studios)
	unProtect(g_libGTASA + 0x39B2C4);
	makeNOP(g_libGTASA + 0x39B2C4, 2);
	
	WriteMemory(g_libGTASA + 0x3DA500, (uintptr_t)"\xF7\x46", 2); // CRadar::DrawLgegend
	WriteMemory(g_libGTASA + 0x3DBB30, (uintptr_t)"\xF7\x46", 2); // CRadar::AddBlipToLegendList
	WriteMemory(g_libGTASA + 0x3D541C, (uintptr_t)"\xF7\x46", 2); // CHud::DrawVehicleName
	WriteMemory(g_libGTASA + 0x2BCD0C, (uintptr_t)"\xF7\x46", 2); // CCheat::ProcessCheatMenu
	WriteMemory(g_libGTASA + 0x2BCDB4, (uintptr_t)"\xF7\x46", 2); // CCheat::ProcessCheats
	WriteMemory(g_libGTASA + 0x2BC08C, (uintptr_t)"\xF7\x46", 2); // CCheat::AddToCheatString
	WriteMemory(g_libGTASA + 0x2BC24C, (uintptr_t)"\xF7\x46", 2); // CCheat::DoCheats 
	WriteMemory(g_libGTASA + 0x2BA710, (uintptr_t)"\xF7\x46", 2); // CCheat::WeaponCheat1
	WriteMemory(g_libGTASA + 0x2BA92C, (uintptr_t)"\xF7\x46", 2); // CCheat::WeaponCheat2
	WriteMemory(g_libGTASA + 0x2BAB20, (uintptr_t)"\xF7\x46", 2); // CCheat::WeaponCheat3
	WriteMemory(g_libGTASA + 0x2BA68C, (uintptr_t)"\xF7\x46", 2); // CCheat::WeaponCheat4
	
	WriteMemory(g_libGTASA + 0x3E17F0, (uintptr_t)"\xF7\x46", 2); // Interior_c::AddPickups
	WriteMemory(g_libGTASA + 0x3E42E0, (uintptr_t)"\xF7\x46", 2); // InteriorGroup_c::Exit
	WriteMemory(g_libGTASA + 0x3E4490, (uintptr_t)"\xF7\x46", 2); // InteriorGroup_c::Setup
	WriteMemory(g_libGTASA + 0x3E3F38, (uintptr_t)"\xF7\x46", 2); // InteriorGroup_c::SetupPeds
	
	WriteMemory(g_libGTASA + 0x5366D0, (uintptr_t)"\xF7\x46", 2); // CGlass::Render
	WriteMemory(g_libGTASA + 0x5286EC, (uintptr_t)"\xF7\x46", 2); // CBirds::Render
	WriteMemory(g_libGTASA + 0x392A98, (uintptr_t)"\xF7\x46", 2); // CEntity::PreRenderForGlassWindow
	WriteMemory(g_libGTASA + 0x54DCF4, (uintptr_t)"\xF7\x46", 2); // CMirrors::RenderReflBuffer
	WriteMemory(g_libGTASA + 0x3B67F8, (uintptr_t)"\xF7\x46", 2); // CRopes::Update
	
	WriteMemory(g_libGTASA + 0x463870, (uintptr_t)"\xF7\x46", 2); // C_PcSave::SaveSlot
	WriteMemory(g_libGTASA + 0x2DE734, (uintptr_t)"\xF7\x46", 2); // CRoadBlocks::GenerateRoadBlocks
                  WriteMemory(g_libGTASA + 0x4F90AC, (uintptr_t)"\xF7\x46", 2); // CTheCarGenerators::Process
  	WriteMemory(g_libGTASA + 0x2E82CC, (uintptr_t)"\xF7\x46", 2); // CCarCtrl::GenerateRandomCars
                  WriteMemory(g_libGTASA + 0x504DB8, (uintptr_t)"\xF7\x46", 2); // CPlane::DoPlaneGenerationAndRemoval
                  WriteMemory(g_libGTASA + 0x2C1CB0, (uintptr_t)"\xF7\x46", 2); // CEntryExit::GenerateAmbientPeds
	WriteMemory(g_libGTASA + 0x2B055C, (uintptr_t)"\xF7\x46", 2); // CCarCtrl::GenerateOneEmergencyServicesCar
	WriteMemory(g_libGTASA + 0x351700, (uintptr_t)"\xF7\x46", 2); // CAERadioTrackManager::StartRadio
	
	WriteMemory(g_libGTASA + 0x3474E0, (uintptr_t)"\xF7\x46", 2); // CAEGlobalWeaponAudioEntity::ServiceAmbientGunFire
  	WriteMemory(g_libGTASA + 0x3BF8B4, (uintptr_t)"\xF7\x46", 2); // CPlaceName::Process
  	WriteMemory(g_libGTASA + 0x494FE4, (uintptr_t)"\xF7\x46", 2); // CCarEnterExit::SetPedInCarDirect
	
	WriteMemory(g_libGTASA + 0x4FDC78, (uintptr_t)"\xF7\x46", 2); // CHeli::UpdateHelis
	
	makeNOP(g_libGTASA + 0x408AAA, 2); 	// RpWorldAddLight from ~LightsCreate
                  makeNOP(g_libGTASA + 0x3D6FDC, 2); // CHud::DrawVehicleName from CHud::Draw
	makeNOP(g_libGTASA + 0x2E1EDC, 2); // CUpsideDownCarCheck::UpdateTimers from CTheScripts::Process

	WriteMemory(g_libGTASA + 0x2C3868, (uintptr_t)"\x00\x20\x70\x47", 4); // CGameLogic::IsCoopGameGoingOn
	WriteMemory(g_libGTASA + 0x45FC20, (uintptr_t)"\x4F\xF0\x00\x00\xF7\x46", 6); 	// CPopulation::AddToPopulation
	
	// CCamera::ClearPlayerWeaponMode from CPlayerPed::ClearWeaponTarget (Sniper)
	makeNOP(g_libGTASA + 0x454A88, 2);
	
	// CMessages::AddBigMessage from CPlayerInfo::KillPlayer
	makeNOP(g_libGTASA + 0x3AC8B2, 2);

	unProtect(g_libGTASA + 0x00454950);
	makeNOP(g_libGTASA + 0x00454950, 17); // CAnimManager::RemoveAnimBlockRef

	// CPed pool (old: 140, new: 210)
	WriteMemory(g_libGTASA + 0x3AF2D0, (uintptr_t)"\x45\xF6\xC8\x60\xC0\xF2\x06\x00", 8);
	WriteMemory(g_libGTASA + 0x3AF2DE, (uintptr_t)"\xD2\x20", 2); 
	WriteMemory(g_libGTASA + 0x3AF2E4, (uintptr_t)"\xD2\x22", 2); 
	WriteMemory(g_libGTASA + 0x3AF310, (uintptr_t)"\xD2\x2B", 2);

	// CPedIntelligence pool (old: 140, new: 210)
	WriteMemory(g_libGTASA + 0x3AF7E6, (uintptr_t)"\x42\xF2\xB0\x00\xC0\xF2\x02\x00\x00\x46", 10);
	WriteMemory(g_libGTASA + 0x3AF7F6, (uintptr_t)"\xD2\x20", 2); 
	WriteMemory(g_libGTASA + 0x3AF7FC, (uintptr_t)"\xD2\x22", 2); 
	WriteMemory(g_libGTASA + 0x3AF824, (uintptr_t)"\xD2\x2B", 2);

	// Task pool (old: 500, new: 1524 (1536))
	WriteMemory(g_libGTASA + 0x3AF4EA, (uintptr_t)"\x4F\xF4\x40\x30", 4); 
	WriteMemory(g_libGTASA + 0x3AF4F4, (uintptr_t)"\x4F\xF4\xC0\x60", 4); 
	WriteMemory(g_libGTASA + 0x3AF4FC, (uintptr_t)"\x4F\xF4\xC0\x62", 4); 
	WriteMemory(g_libGTASA + 0x3AF52A, (uintptr_t)"\xB3\xF5\xC0\x6F", 4);

	// TxdStore pool (old: 5000, new: 20000)
	WriteMemory(g_libGTASA + 0x0055BA9A, (uintptr_t)"\x4f\xf4\xb8\x50\xc0\xf2\x11\x00", 8);
	WriteMemory(g_libGTASA + 0x0055BAA8, (uintptr_t)"\x44\xf6\x20\x60", 4); 
	WriteMemory(g_libGTASA + 0x0055BAB0, (uintptr_t)"\x44\xf6\x20\x62", 4); 

	// Event pool (old: 200, new: 512)

	// Object pool (old: 350, new: 2992)
	WriteMemory(g_libGTASA + 0x003AF3D6, (uintptr_t)"\x4f\xf4\x7e\x40\xc0\xf2\x12\x00", 8);
	WriteMemory(g_libGTASA + 0x003AF3E4, (uintptr_t)"\x4f\xf4\x3b\x60", 4); 
	WriteMemory(g_libGTASA + 0x003AF3EC, (uintptr_t)"\x4f\xf4\x3b\x62", 4); 
	WriteMemory(g_libGTASA + 0x003AF41A, (uintptr_t)"\xb3\xf5\x3b\x6f", 4); 

	// Building pool (old: 14000, new: 29585)
	WriteMemory(g_libGTASA + 0x003AF378, (uintptr_t)"\x4F\xF4\x90\x40\xC0\xF2\x19\x00", 8);
	WriteMemory(g_libGTASA + 0x003AF386, (uintptr_t)"\x47\xF2\x91\x30", 4);
	WriteMemory(g_libGTASA + 0x003AF38E, (uintptr_t)"\x47\xF2\x91\x32", 4); 

	// Dummys pool (old: 3500, new: 4000)
	WriteMemory(g_libGTASA + 0x003AF430, (uintptr_t)"\x4f\xf4\xd6\x40\xc0\xf2\x03\x00", 8); 
	WriteMemory(g_libGTASA + 0x003AF43E, (uintptr_t)"\x40\xf6\xa0\x70", 4);
	WriteMemory(g_libGTASA + 0x003AF446, (uintptr_t)"\x40\xf6\xa0\x72", 4); 

	// PtrNode Single pool (old: 75000, new: 100000)
	WriteMemory(g_libGTASA + 0x003AF1BC, (uintptr_t)"\x4f\xf4\x54\x50\xc0\xf2\x0c\x00", 8);
	WriteMemory(g_libGTASA + 0x003AF1D6, (uintptr_t)"\x48\xf2\xa0\x62", 4); 
	WriteMemory(g_libGTASA + 0x003AF1B0, (uintptr_t)"\x48\xf2\xa0\x66", 4); 
	WriteMemory(g_libGTASA + 0x003AF1CA, (uintptr_t)"\x48\xf2\xa0\x60\xc0\xf2\x01\x00", 8); 

	//PtrNode Double pool (old: 6000, new: 8000)
	WriteMemory(g_libGTASA + 0x003AF21C, (uintptr_t)"\x4f\xf4\xee\x40\xc0\xf2\x01\x00", 8);
	WriteMemory(g_libGTASA + 0x003AF22A, (uintptr_t)"\x41\xf6\x40\x70", 4); 
	WriteMemory(g_libGTASA + 0x003AF232, (uintptr_t)"\x41\xf6\x40\x72", 4); 

	// Entry Node Info pool (old: 500, new: 5120)
	WriteMemory(g_libGTASA + 0x003AF27A, (uintptr_t)"\x4f\xf4\xc8\x30", 4); 
	WriteMemory(g_libGTASA + 0x003AF284, (uintptr_t)"\x4f\xf4\xa0\x50", 4); 
	WriteMemory(g_libGTASA + 0x003AF28C, (uintptr_t)"\x4f\xf4\xa0\x52", 4); 
	WriteMemory(g_libGTASA + 0x003AF2BA, (uintptr_t)"\xb3\xf5\xa0\x5f", 4);

	// ColModel pool (old:10150, new: 32511)
	WriteMemory(g_libGTASA + 0x3AF48E, (uintptr_t)"\x4C\xF6\xD0\x70\xC0\xF2\x17\x00", 8);
	WriteMemory(g_libGTASA + 0x3AF49C, (uintptr_t)"\x47\xF6\xFF\x60", 4); 
	WriteMemory(g_libGTASA + 0x3AF4A4, (uintptr_t)"\x47\xF6\xFF\x62", 4);

	// VehicleStruct increase (0x32C*0x50 = 0xFDC0)
	WriteMemory(g_libGTASA + 0x405338, (uintptr_t)"\x4F\xF6\xC0\x50", 4);
	WriteMemory(g_libGTASA + 0x405342, (uintptr_t)"\x50\x20", 2);			
	WriteMemory(g_libGTASA + 0x405348, (uintptr_t)"\x50\x22", 2);			
	WriteMemory(g_libGTASA + 0x405374, (uintptr_t)"\x50\x2B", 2);		

	// Increase matrix count in CPlaceable::InitMatrixArray
	WriteMemory(g_libGTASA + 0x3ABB0A, (uintptr_t)"\x4F\xF4\x7A\x61", 4);

	// stop assign 0xFFFFFF to beam color!
	makeNOP(g_libGTASA + 0x00519116, 2);
	makeNOP(g_libGTASA + 0x0051911C, 4);
	makeNOP(g_libGTASA + 0x00519198, 4);
	makeNOP(g_libGTASA + 0x00519198, 2);
	makeNOP(g_libGTASA + 0x005191D0, 4);
	makeNOP(g_libGTASA + 0x005191DC, 2);

	// Prevent another ped from answering when collision happens (or damage)
	makeNOP(g_libGTASA + 0x327730, 2);
	makeNOP(g_libGTASA + 0x32C488, 2);
	makeNOP(g_libGTASA + 0x32DF94, 2);
	makeNOP(g_libGTASA + 0x32E05E, 2);
	makeNOP(g_libGTASA + 0x32E160, 2);
	makeNOP(g_libGTASA + 0x32E1F2, 2);
	makeNOP(g_libGTASA + 0x32E292, 2);
	makeNOP(g_libGTASA + 0x32EFD0, 2);

	// CPed::Say
	makeNOP(g_libGTASA + 0x43E288, 2);

	// Prevent double vehicle sound service
	makeNOP(g_libGTASA + 0x4E31A6, 2);
	makeNOP(g_libGTASA + 0x4EE7D2, 2);
	makeNOP(g_libGTASA + 0x4F741E, 2);
	makeNOP(g_libGTASA + 0x50AB4A, 2);

	// Disable in-game radio
	makeNOP(g_libGTASA + 0x3688DA, 2);
	makeNOP(g_libGTASA + 0x368DF0, 2);
	makeNOP(g_libGTASA + 0x369072, 2);
	makeNOP(g_libGTASA + 0x369168, 2);

	// Stop it trying to load tracks2.dat
	makeNOP(g_libGTASA + 0x508F36, 2);

	// Stop it trying to load tracks4.dat
	makeNOP(g_libGTASA + 0x508F54, 2);

	// Prevent cheats processing
	makeNOP(g_libGTASA + 0x3987BA, 2);

	// gamepad lags
                  // FuckCode(g_libGTASA + 0x24D3D4); // _ZN4CHID12GetInputTypeEv
}

void ApplyInGamePatches()
{
	Log(OBFUSCATE("Installing patches (ingame).."));

	// CTheZones::ZonesVisited[100]
	memset((void*)(g_libGTASA + 0x8EA7B0), 1, 100);
	// CTheZones::ZonesRevealed
	*(uint32_t*)(g_libGTASA + 0x8EA7A8) = 100;

	// CTaskSimplePlayerOnFoot::PlayIdleAnimations
	WriteMemory(g_libGTASA + 0x4BDB18, (uintptr_t)"\x70\x47", 2);

	// CarCtl::GenerateRandomCars nulled from CGame::Process
	unProtect(g_libGTASA + 0x398A3A);
	makeNOP(g_libGTASA + 0x398A3A, 2);

	// CTheCarGenerators::Process nulled from CGame::Process
	unProtect(g_libGTASA + 0x398A34);
	makeNOP(g_libGTASA + 0x398A34, 2);

	// MAXHealth
	unProtect(g_libGTASA + 0x3BAC68);
	*(float*)(g_libGTASA + 0x3BAC68) = 176.0f;
	// MAXArmour
	unProtect(g_libGTASA + 0x27D884);
	*(float*)(g_libGTASA + 0x27D884) = 176.0f;

	// CEntryExit::GenerateAmbientPeds nulled from CEntryExit::TransitionFinished
	unProtect(g_libGTASA + 0x2C2C22);
	makeNOP(g_libGTASA + 0x2C2C22, 4);

	makeNOP(g_libGTASA + 0x003D6FDC, 2);
	makeNOP(g_libGTASA + 0x0026B214, 2);

	WriteMemory(g_libGTASA + 0x3D62FC, (uintptr_t)"\xF7\x46", 2);

	// CPlayerPed::CPlayerPed task fix
	WriteMemory(g_libGTASA + 0x458ED1, (uintptr_t)"\xE0", 1);

	// ReapplyPlayerAnimation
	makeNOP(g_libGTASA + 0x45477E, 5);

	// radar draw blips
	unProtect(g_libGTASA + 0x3DCA90);
	makeNOP(g_libGTASA + 0x3DCA90, 2);
	unProtect(g_libGTASA + 0x3DD4A4);
	makeNOP(g_libGTASA + 0x3DD4A4, 2);

	// CCamera::CamShake from CExplosion::AddExplosion
	makeNOP(g_libGTASA + 0x55EFB8, 2);
	makeNOP(g_libGTASA + 0x55F8F8, 2);

	// camera_on_actor path
	unProtect(g_libGTASA + 0x2F7B68);
	*(uint8_t*)(g_libGTASA + 0x2F7B6B) = 0xBE;

	// CPed::RemoveWeaponWhenEnteringVehicle (GetPlayerInfoForThisPlayerPed)
	unProtect(g_libGTASA + 0x434D94);
	makeNOP(g_libGTASA + 0x434D94, 6);

	// CBike::ProcessAI
	unProtect(g_libGTASA + 0x4EE200);
	*(uint8_t*)(g_libGTASA + 0x4EE200) = 0x9B;

	// CWidgetPlayerInfo::DrawWanted
	WriteMemory(g_libGTASA + 0x27D8D0, (uintptr_t)"\x4F\xF0\x00\x08", 4);

	// no vehicle audio processing
	unProtect(g_libGTASA + 0x4E31A6);
	makeNOP(g_libGTASA + 0x4E31A6, 2);
	unProtect(g_libGTASA + 0x4EE7D2);
	makeNOP(g_libGTASA + 0x4EE7D2, 2);
	unProtect(g_libGTASA + 0x4F741E);
	makeNOP(g_libGTASA + 0x4F741E, 2);
	unProtect(g_libGTASA + 0x50AB4A);
	makeNOP(g_libGTASA + 0x50AB4A, 2);

	makeNOP(g_libGTASA + 0x00398768, 2); // nop police helis
	makeNOP(g_libGTASA + 0x003987DC, 2);

	makeNOP(g_libGTASA + 0x003688EC, 2); // nop ServiceAmbientGunFire

	// Artifacts
	*(float*)(SA_ADDR(0x608558)) = 200.0f;

	Log(OBFUSCATE("InGame patches successfully installed!"));
}