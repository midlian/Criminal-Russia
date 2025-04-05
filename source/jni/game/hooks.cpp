#include "../main.h"

#include "RW/RenderWare.h"
#include "game.h"
#include "crosshair.h"
#include "util.h"
#include "CRenderTarget.h"
#include "CCustomPlateManager.h"

#include "../net/netgame.h"

static bool mNeedNewLoader = false;

#include "../gui/CFontInstance.h"
#include "../gui/CFontRenderer.h"
#include "../gui/CBinder.h"
#include "../gui/gui.h"

#ifdef USE_SANTROPE_BULLSHIT
	#include "../../santrope-tea-gtasa/encryption/CTinyEncrypt.h"
	#include "../../santrope-tea-gtasa/encryption/encrypt.h"
	extern "C" {
		#include "../../santrope-tea-gtasa/encryption/aes.h"
	}
	
	#include "../cryptors/MODELINFO_EDITABLE_result.h"
	#include "../cryptors/RESET_AFTER_RENDER_result.h"
	#include "../cryptors/AUTOMOBILE_COLLISION_result.h"
	#include "../cryptors/INSTALLHOOKS_result.h"
#endif 

#include "../util/CJavaWrapper.h"
#include "../util/armhook.h"

#include "../voice/CVoiceChatClient.h"

#include "../CAudioStream.h"
#include "../chatwindow.h"
#include "../keyboard.h"
#include "../CSettings.h"
#include "../nv_event.h"
#include "../crashlytics.h"
#include "../str_obfuscator_no_template.hpp"
#include "../CFPSFix.h"

#include "../graphics/CRQ_Commands.h"
#include "../graphics/CSkyBox.h"

extern CChatWindow* pChatWindow;
extern CCrossHair* pCrossHair;
extern CVoiceChatClient* pVoice;
extern CAudioStream* pAudioStream;
extern CNetGame *pNetGame;
extern CGUI *pGUI;
extern CKeyBoard* pKeyBoard;
extern CGame* pGame;
extern CSettings* pSettings;

CFPSFix g_fps;

extern CPlayerPed* g_pCurrentFiredPed;
extern BULLET_DATA* g_pCurrentBulletData;
extern CAMERA_AIM * pcaInternalAim;

#ifdef USE_SANTROPE_BULLSHIT
	char(*CStreaming__ConvertBufferToObject)(int, int, int);
	int __attribute__((noinline)) g_unobfuscate(int a)
	{
		return UNOBFUSCATE_DATA(a);
	}
#endif

#define MAX_ENCRYPTED_TXD 2
const cryptor::string_encryptor encrArch[MAX_ENCRYPTED_TXD] = {
		cryptor::create(OBFUSCATE("texdb/samp/samp.txt"), 21),
		cryptor::create(OBFUSCATE("texdb/gui/gui.txt"), 19)
};
static int lastOpenedFile = 0;

bool g_bPlaySAMP = false;

void InitInMenu();
void MainLoop();
void HookCPad();

extern "C" uintptr_t get_lib() 
{
 	return g_libGTASA;
}

struct stFile
{
	int isFileExist;
	FILE *f;
};

stFile* (*NvFOpen)(const char*, const char*, int, int);
stFile* NvFOpen_hook(const char* r0, const char* r1, int r2, int r3)
{
	char path[0xFF] = { 0 };
	
	if(!strncmp(r1+12, "mainV1.scm", 10))
	{
		sprintf(path, "%sSAMP/main.scm", g_pszStorage);
		Log("Loading mainV1.scm..");
		goto open;
	}
	
	if(!strncmp(r1+12, "SCRIPTV1.IMG", 12))
	{
		sprintf(path, "%sSAMP/script.img", g_pszStorage);
		Log("Loading script.img..");
		goto open;
	}
	
	if(!strncmp(r1, "DATA/PEDS.IDE", 13))
	{
		sprintf(path, "%s/SAMP/peds.ide", g_pszStorage);
		Log("Loading peds.ide..");
		goto open;
	}
	
	if(!strncmp(r1, "DATA/VEHICLES.IDE", 17))
	{
		sprintf(path, "%s/SAMP/vehicles.ide", g_pszStorage);
		Log("Loading vehicles.ide..");
		goto open;
	}

	if (!strncmp(r1, "DATA/GTA.DAT", 12))
	{
		sprintf(path, "%s/SAMP/gta.dat", g_pszStorage);
		Log("Loading gta.dat..");
		goto open;
	}

	/*if (!strncmp(r1, "DATA/HANDLING.CFG", 17))
	{
		sprintf(path, "%sdata/handling.cfg", g_pszStorage);
		Log("Loading handling.cfg..");
		goto open;
	}

	if (!strncmp(r1, "DATA/WEAPON.DAT", 15))
	{
		sprintf(path, "%s/SAMP/weapon.dat", g_pszStorage);
		Log("Loading weapon.dat..");
		goto open;
	}*/

orig:
	return NvFOpen(r0, r1, r2, r3);

open:
	stFile *st = (stFile*)malloc(8);
	st->isFileExist = false;

	FILE *f  = fopen(path, "rb");
	if(f)
	{
		st->isFileExist = true;
		st->f = f;
		return st;
	}
	else
	{
		Log("Error: file in %s not found", path);
		free(st);
		st = nullptr;
		return 0;
	}
}

// -----------------------------------------------------------------------
void InjectStuff() 
{
	if (pGUI->m_bCarryStuff) {
                                    g_pJavaWrapper->UpdateHudIcon(0);
                                    pGame->FindPlayerPed()->SetArmedWeapon(0);
                  }
}

void InjectHud() 
{
                                    CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		if(pPlayerPool) 
		{                                                    
			g_pJavaWrapper->SetHP((int)pGame->FindPlayerPed()->GetHealth());
			g_pJavaWrapper->SetArmour((int)pGame->FindPlayerPed()->GetArmour());
			g_pJavaWrapper->SetMoney(pGame->GetLocalMoney());
	
			int patrons = (int)pGame->FindPlayerPed()->GetCurrentWeaponSlot()->dwAmmo;
			int patronsInClip = (int)pGame->FindPlayerPed()->GetCurrentWeaponSlot()->dwAmmoInClip;
			g_pJavaWrapper->SetAmmo(
                                                                            patrons,
                                                                            patronsInClip
                                                      );
			g_pJavaWrapper->UpdateHudIcon((int) pGame->FindPlayerPed()->GetCurrentWeaponSlot()->dwType);

			g_pJavaWrapper->UpdateTime();
			g_pJavaWrapper->SetOnline((int)pPlayerPool->GetLocalPlayerID());
	
	
                                    }
}
// -----------------------------------------------------------------------

bool bGameStarted = false;
uint32_t bProcessedRender2dstuff = 0;

void (*Render2dStuff)();
void Render2dStuff_hook()
{
	bGameStarted = true;
	MAKE_PROFILE(test, test_time);
	MainLoop();
	LOG_PROFILE(test, test_time);
	bProcessedRender2dstuff = GetTickCount();
	Render2dStuff();

                  InjectHud();
                  // ... InjectStuff();

	if (pNetGame)
	{
		CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->GetPlayerPed())
				pNetGame->GetTextDrawPool()->Draw();
		}
		if (pNetGame->GetPlayerPool())
		{
			if (pNetGame->GetPlayerPool()->GetLocalPlayer())
			{
				CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
				if (pPed)
					CInfoBarText::Draw(pPed->GetHealth(), pPed->GetArmour());
			}
		}
	}
}

#ifdef USE_SANTROPE_BULLSHIT
void InitCTX(AES_ctx&, unsigned char const*)
{
	
}
#endif

void (*Render2dStuffAfterFade)();
void Render2dStuffAfterFade_hook()
{
	Render2dStuffAfterFade();

	if (pGUI && bGameStarted)
		pGUI->Render();
}

void (*CWidget__Update)(WIDGET_TYPE*);
void CWidget__Update_hook(WIDGET_TYPE* pWidget)
{
	if(pNetGame)
	{
		switch (ProcessFixed(pWidget))
		{
			case STATE_NONE: break;
			case STATE_FIXED: return;
		}
	}
	return CWidget__Update(pWidget);
}

void (*CWidget__SetEnabled)(WIDGET_TYPE* pWidget, bool bEnabled);
void CWidget__SetEnabled_hook(WIDGET_TYPE* pWidget, bool bEnabled)
{	
	if(pNetGame)
	{
		switch (ProcessFixed(pWidget))
		{
			case STATE_NONE: break;
			case STATE_FIXED:
				bEnabled = false;
			break;
		}
	} 
	return CWidget__SetEnabled(pWidget, bEnabled);	
}

uintptr_t (*CWidgets)(WIDGET_TYPE*, const char*, int, int, int, int);
uintptr_t CWidget_hook(WIDGET_TYPE* thiz, const char* name, int a3, int a4, int a5, int a6)
{
	SetByName(name, thiz);
	return CWidgets(thiz, name, a3, a4, a5, a6);
}

uint16_t gxt_string[0x7F];
uint16_t* (*CText_Get)(uintptr_t thiz, const char* text);
uint16_t* CText_Get_hook(uintptr_t thiz, const char* text)
{
	if(text[0] == 'S' && text[1] == 'A' && text[2] == 'M' && text[3] == 'P')
	{
		const char* code = &text[5];
		if(!strcmp(code, OBFUSCATE("MP"))) CFont::AsciiToGxtChar(OBFUSCATE("MultiPlayer"), gxt_string);

    	                  return gxt_string;
	}

	return CText_Get(thiz, text);
}

static uint32_t dwLastSelectedLang = 0;
void MainMenu_OnStartSAMP()
{
	if(g_bPlaySAMP) return;

	InitInMenu();

	dwLastSelectedLang = *(uint32_t*)(SA_ADDR(0x63E5A4));

	// StartGameScreen::OnNewGameCheck()
	(( void (*)())(SA_ADDR(0x261C8C + 1)))();

	g_bPlaySAMP = true;
}

void (*MenuItem_add)(int r0, uintptr_t r1);
void MenuItem_add_hook(int r0, uintptr_t r1)
{
	static bool bMenuInited = false;
	char* name = *(char**)(r1+4);

	if(!strcmp(name, OBFUSCATE("FEP_STG")) && !bMenuInited)
	{
		Log(OBFUSCATE("Creating \"MultiPlayer\" button.. (struct: 0x%X)"), r1);
		MenuItem_add(r0, r1);

		*(char**)(r1+4) = OBFUSCATE("SAMP_MP");
		*(uintptr_t*)r1 = LoadTextureFromDB(OBFUSCATE("samp"), OBFUSCATE("menu_mainmp"));
		*(uintptr_t*)(r1+8) = (uintptr_t)MainMenu_OnStartSAMP;

		bMenuInited = true;
		goto ret;
	}

	if(g_bPlaySAMP && (
		!strcmp(name, OBFUSCATE("FEP_STG")) ||
		!strcmp(name, OBFUSCATE("FEH_STA")) ||
		!strcmp(name, OBFUSCATE("FEH_BRI")) ))
		return;

ret:
	return MenuItem_add(r0, r1);
}

enum TextureDatabaseFormat {
	DF_NONE 	= 0,		// DF_UNC
	DF_DXT 		= 1,
	DF_UNC_2 	= 2,
	DF_PVR 		= 3,
	DF_ETC	 	= 4,
	DF_Default	= 5
};	

/*
FFFFFFFF ; enum TextureDatabaseFormat, copyof_268, width 4 bytes
FFFFFFFF DF_UNC           EQU 0
FFFFFFFF DF_DXT           EQU 1
FFFFFFFF DF_360           EQU 2
FFFFFFFF DF_PS3           EQU 3
FFFFFFFF DF_PVR           EQU 4
FFFFFFFF DF_ETC           EQU 5
FFFFFFFF DF_Default       EQU 6
FFFFFFFF DF_ALL           EQU 7
FFFFFFFF
*/

uintptr_t* (*TextureDatabaseRuntime_Load)(char *a1, int a2, int a3);
uintptr_t* TextureDatabaseRuntime_Load_hook(char *a1, int a2, int a3){

	a3 = DF_DXT;

	if (std::strcmp(a1, "player") == 0 || std::strcmp(a1, "playerhi") == 0) {
		a3 = DF_PVR;
	}

	return TextureDatabaseRuntime_Load(a1, a2, a3);
}

/* Formats */
class TextureCategory {
public:
	std::uint8_t * name;
	std::uint32_t onFootPriority;
	std::uint32_t slowCarPriority;
	std::uint32_t fastCarPriority;
	std::uint16_t defaultFormat;
	std::uint16_t defaultStreamMode;
};
static_assert(sizeof(TextureCategory) == 0x14, "TextureCategory not 0x14");

class TextureDatabaseEntry {
public:
	std::uint8_t * name;
	std::uint8_t pad0[0x13];
};
static_assert(sizeof(TextureDatabaseEntry) == 0x17, "TextureDatabaseEntry not 0x17");

/* Arrays */
template<typename T>
class TDBArray {
public:
	std::uint32_t numAlloced;
	std::uint32_t numEntries;
	T * dataPtr;
};
static_assert(sizeof(TDBArray<void>) == 0xC, "TextureDatabaseRuntime not 0xC8");


class TextureDatabase {
public:
	void * _vptrTextureDatabase;
	uint32_t name;
	TDBArray<TextureCategory> categories; // categories
	TDBArray<TextureDatabaseEntry> entries; // entries
	uint8_t pad2[0x48]; // thumbs
	uint8_t pad3[0xC]; // toFree
	uint32_t loadedFormat;
};
static_assert(sizeof(TextureDatabase) == 0x78, "TextureDatabase not 78");

class TextureDatabaseRuntime : public TextureDatabase {
public:
	uint8_t pad4[0xC]; // priorityStreamingQueue
	uint8_t pad5[0xC]; // renderedStreamingQueue
	uint8_t pad6[0xC]; // unrenderedStreamingQueue
	uint8_t pad7[0xC]; // deletionQueue
	uint32_t streamFile;
	uint8_t pad8[0xC]; // fullDataOffsets
	uint8_t pad9[0xC]; // hashOffsets
	uint32_t numHashes;
};
static_assert(sizeof(TextureDatabaseRuntime) == 0xC8, "TextureDatabaseRuntime not 0xC8");

void (*TextureDatabaseRuntime_SortEntries)(uintptr_t thiz, int a2);
void TextureDatabaseRuntime_SortEntries_hook(uintptr_t thiz, int a2) {
	TextureDatabaseRuntime * database = (TextureDatabaseRuntime*)thiz;

	Log("TextureDatabaseRuntime::SortEntries_hook: (database)%s | (texture)%s", (const char*)database->name, database->entries.dataPtr->name);
	return TextureDatabaseRuntime_SortEntries(thiz, a2);
}


// CGame::InitialiseRenderWare
void (*InitialiseRenderWare)();
void InitialiseRenderWare_hook()
{
	Log(OBFUSCATE("Loading textures.."));

	InitialiseRenderWare();

	(( void (*)(const char*, int, int))(SA_ADDR(0x1BF244 + 1)))("samp", 0, 5);	

                  if(mNeedNewLoader) {
                                    Log(OBFUSCATE("Inject new textures.."));

                                    // -- Peds
		(( void (*)(const char*, int, int))(SA_ADDR(0x1BF244 + 1)))("peds", 0, 5);	

                                    // -- Tuning
		(( void (*)(const char*, int, int))(SA_ADDR(0x1BF244 + 1)))("tuning", 0, 5);	

                                    // -- Vehicles
		(( void (*)(const char*, int, int))(SA_ADDR(0x1BF244 + 1)))("vehf", 0, 5);	

                                    // -- Accesories
		(( void (*)(const char*, int, int))(SA_ADDR(0x1BF244 + 1)))("accesories", 0, 5);	
                  }
}

void RenderSplashScreen();

void (*CLoadingScreen_DisplayPCScreen)();
void CLoadingScreen_DisplayPCScreen_hook()
{
	RwCamera* camera = *(RwCamera**)(SA_ADDR(0x95B064));

	if(RwCameraBeginUpdate(camera))
	{
		DefinedState2d();

		(( void (*)())(SA_ADDR(0x5519C8 + 1)))(); // CSprite2d::InitPerFrame()
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
		(( void (*)(bool))(SA_ADDR(0x198010 + 1)))(false); // emu_GammaSet()

		RenderSplashScreen();

		RwCameraEndUpdate(camera);
		RwCameraShowRaster(camera, nullptr, 0);
	}
}

int bBlockCWidgetRegionLookUpdate = 0;

void (*TouchEvent)(int, int, int posX, int posY);
void TouchEvent_hook(int type, int num, int posX, int posY)
{
	if (*(uint8_t*)(SA_ADDR(0x8C9BA3)) == 1)
		return TouchEvent(type, num, posX, posY);

	if (g_pWidgetManager)
	{
		g_pWidgetManager->OnTouchEventSingle(WIDGET_CHATHISTORY_UP, type, num, posX, posY);
		g_pWidgetManager->OnTouchEventSingle(WIDGET_CHATHISTORY_DOWN, type, num, posX, posY);
	}

	bool bRet = pGUI->OnTouchEvent(type, num, posX, posY);
	if (!bRet)
		return;

	ImGuiIO& io = ImGui::GetIO();

	if (pKeyBoard && pKeyBoard->IsOpen())
	{
		if (posX >= io.DisplaySize.x - ImGui::GetFontSize() * 4 && posY >= io.DisplaySize.y / 2 - (ImGui::GetFontSize() * 2.5) * 3 && posY <= io.DisplaySize.y / 2) // keys
			return;
	}

	if (g_pWidgetManager)
	{
		g_pWidgetManager->OnTouchEvent(type, num, posX, posY);
		if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE))
		{
			if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->GetState() == 1)
				bBlockCWidgetRegionLookUpdate = 1;

			if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->GetState() == 2)
				bBlockCWidgetRegionLookUpdate = 0;
		}
	}

	if(bRet) 
		return TouchEvent(type, num, posX, posY);
}

void (*CStreaming_InitImageList)();
void CStreaming_InitImageList_hook()
{
	char* ms_files = (char*)(SA_ADDR(0x6702FC));
	ms_files[0] = 0;
	*(uint32_t*)&ms_files[44] = 0;
	ms_files[48] = 0;
	*(uint32_t*)&ms_files[92] = 0;
	ms_files[96] = 0;
	*(uint32_t*)&ms_files[140] = 0;
	ms_files[144] = 0;
	*(uint32_t*)&ms_files[188] = 0;
	ms_files[192] = 0;
	*(uint32_t*)&ms_files[236] = 0;
	ms_files[240] = 0;
	*(uint32_t*)&ms_files[284] = 0;
	ms_files[288] = 0;
	*(uint32_t*)&ms_files[332] = 0;
	ms_files[336] = 0;
	*(uint32_t*)&ms_files[380] = 0;

                  // CStreaming::AddImageToList()
                  Log(OBFUSCATE("Loading imgs.."));
	(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\GTA3.IMG"), 1); 
	(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\GTA_INT.IMG"), 1);
	(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\SAMP.IMG"), 1); 
	(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\SAMPCOL.IMG"), 1);

                  if(mNeedNewLoader) {
                                    Log(OBFUSCATE("Inject new models.."));

                                    // -- Peds
		(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\PEDS.IMG"), 1);

                                    // -- Tuning
		(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\TUNING.IMG"), 1);

                                    // -- Vehicles
		(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\VEHF.IMG"), 1);

                                    // -- Accesories
		(( uint32_t (*)(char*, uint32_t))(SA_ADDR(0x28E7B0 + 1)))(OBFUSCATE("TEXDB\\ACCESORIES.IMG"), 1);
                  }
}

/* ====================================================== */
typedef struct _PED_MODEL
{
	uintptr_t 	vtable;
	uint8_t		data[88];
} PED_MODEL; // SIZE = 92

PED_MODEL PedsModels[1000];
int PedsModelsCount = 0;

PED_MODEL* (*CModelInfo_AddPedModel)(int id);
PED_MODEL* CModelInfo_AddPedModel_hook(int id)
{
	PED_MODEL* model = &PedsModels[PedsModelsCount];
	memset(model, 0, sizeof(PED_MODEL));

	((void(*)(void* thiz))(SA_ADDR(0x33559C + 1)))((void*)model); // CBaseModelInfo::CBaseModelInfo();
                  model->vtable = (uintptr_t)(SA_ADDR(0x5C6E90));

                  ((uintptr_t (*)(PED_MODEL*))(*(void**)(model->vtable+0x1C)))(model);  // CClumpModelInfo::Initialise()

                  *(PED_MODEL**)(SA_ADDR(0x87BF48 + (id * 4))) = model; // CModelInfo::ms_modelInfoPtrs

	PedsModelsCount++;
	Log("Loading skin(%d)...", PedsModelsCount);
	return model;
}

uint32_t (*CRadar__GetRadarTraceColor)(uint32_t color, uint8_t bright, uint8_t friendly);
uint32_t CRadar__GetRadarTraceColor_hook(uint32_t color, uint8_t bright, uint8_t friendly)
{
	return TranslateColorCodeToRGBA(color);
}

int (*CRadar__SetCoordBlip)(int r0, float X, float Y, float Z, int r4, int r5, char* name);
int CRadar__SetCoordBlip_hook(int r0, float X, float Y, float Z, int r4, int r5, char* name)
{
	if(pNetGame && !strncmp(name, OBFUSCATE("CODEWAY"), 7))
	{
		float findZ = (( float (*)(float, float))(SA_ADDR(0x3C3DD8 + 1)))(X, Y);
		findZ += 1.5f;

		// .. Log(OBFUSCATE("OnPlayerClickMap: %f, %f, %f"), X, Y, Z);
		RakNet::BitStream bsSend;

		bsSend.Write(X);
		bsSend.Write(Y);
		bsSend.Write(findZ);

		pNetGame->GetRakClient()->RPC(&RPC_MapMarker, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
	}

	return CRadar__SetCoordBlip(r0, X, Y, Z, r4, r5, name);
}

uint8_t bGZ = 0;

void (*CRadar__DrawRadarGangOverlay)(uint8_t v1);
void CRadar__DrawRadarGangOverlay_hook(uint8_t v1)
{
	bGZ = v1;
	if (pNetGame && pNetGame->GetGangZonePool()) 
		pNetGame->GetGangZonePool()->Draw();
}

uint32_t dwParam1, dwParam2;
extern "C" void pickup_ololo()
{
	if(pNetGame && pNetGame->GetPickupPool())
	{
		CPickupPool *pPickups = pNetGame->GetPickupPool();
		pPickups->PickedUp( ((dwParam1 - (SA_ADDR(0x70E264))) / 0x20));
	}
}

__attribute__((naked)) void PickupPickUp_hook()
{
	__asm__ volatile("push {lr}\n\t"
					"push {r0}\n\t"
					"blx get_lib\n\t"
					"add r0, #0x2D0000\n\t"
					"add r0, #0x009A00\n\t"
					"add r0, #1\n\t"
					"mov r1, r0\n\t"
					"pop {r0}\n\t"
					"pop {lr}\n\t"
					"push {r1}\n\t");
	
	__asm__ volatile("push {r0-r11, lr}\n\t"
					"mov %0, r4" : "=r" (dwParam1));

	__asm__ volatile("blx pickup_ololo\n\t");


	__asm__ volatile("pop {r0-r11, lr}\n\t");

	__asm__ volatile("ldrb r1, [r4, #0x1C]\n\t"
					"sub.w r2, r1, #0xD\n\t"
					"sub.w r2, r1, #8\n\t"
					"cmp r1, #6\n\t"
					"pop {pc}\n\t");
}

extern "C" bool NotifyEnterVehicle(VEHICLE_TYPE *_pVehicle)
{
     if(!pNetGame)
    	return false;
 
    CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
    CVehicle *pVehicle;
    VEHICLEID VehicleID = pVehiclePool->FindIDFromGtaPtr(_pVehicle);
 
    if(VehicleID == INVALID_VEHICLE_ID)
    	return false;

    if(!pVehiclePool->GetSlotState(VehicleID))
    	return false;

    pVehicle = pVehiclePool->GetAt(VehicleID);
    if(pVehicle->m_pVehicle->entity.nModelIndex == TRAIN_PASSENGER)
    	return false;
 
    if(pVehicle->m_pVehicle->pDriver && pVehicle->m_pVehicle->pDriver->dwPedType != 0)
                  return false;
 
    CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
    pLocalPlayer->SendEnterVehicleNotification(VehicleID, false);
 
    return true;
}

void (*CTaskComplexEnterCarAsDriver)(uint32_t thiz, uint32_t pVehicle);
extern "C" void call_taskEnterCarAsDriver(uintptr_t a, uint32_t b)
{
	CTaskComplexEnterCarAsDriver(a, b);
}
void __attribute__((naked)) CTaskComplexEnterCarAsDriver_hook(uint32_t thiz, uint32_t pVehicle)
{
    __asm__ volatile("push {r0-r11, lr}\n\t"
                    "mov r2, lr\n\t"
                    "blx get_lib\n\t"
                    "add r0, #0x3A0000\n\t"
                    "add r0, #0xEE00\n\t"
                    "add r0, #0xF7\n\t"
                    "cmp r2, r0\n\t"
                    "bne 1f\n\t" // !=
                    "mov r0, r1\n\t"
                    "blx NotifyEnterVehicle\n\t"
                    "1:\n\t"  // call orig
                    "pop {r0-r11, lr}\n\t"
    				"push {r0-r11, lr}\n\t"
    				"blx call_taskEnterCarAsDriver\n\t"
    				"pop {r0-r11, pc}");
}

void (*CTaskComplexLeaveCar)(uintptr_t** thiz, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut);
void CTaskComplexLeaveCar_hook(uintptr_t** thiz, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut) 
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;
 
 	if (dwRetAddr == 0x3AE905 || dwRetAddr == 0x3AE9CF) 
 	{
 		if (pNetGame) 
 		{
 			if (GamePool_FindPlayerPed()->pVehicle == (uint32_t)pVehicle) 
 			{
 				CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
 				VEHICLEID VehicleID = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)GamePool_FindPlayerPed()->pVehicle);
 				CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

 				pLocalPlayer->SendExitVehicleNotification(VehicleID);
 			}
 		}
 	}
 
 	(*CTaskComplexLeaveCar)(thiz, pVehicle, iTargetDoor, iDelayTime, bSensibleLeaveCar, bForceGetOut);
}

unsigned int (*MainMenuScreen__Update)(uintptr_t thiz, float a2);
unsigned int MainMenuScreen__Update_hook(uintptr_t thiz, float a2)
{
	unsigned int ret = MainMenuScreen__Update(thiz, a2);

	MainMenu_OnStartSAMP();

	return ret;
}

extern signed int (*OS_FileOpen)(unsigned int a1, int* a2, const char* a3, int a4);
signed int OS_FileOpen_hook(unsigned int a1, int* a2, const char* a3, int a4);

extern int(*OS_FileRead)(void* a1, void* a2, int a3);
int OS_FileRead_hook(void* a1, void* a2, int a33);
extern char(*CStreaming__ConvertBufferToObject)(int, int, int);
char CStreaming__ConvertBufferToObject_hook(int a1, int a2, int a3);
extern char(*CFileMgr__ReadLine)(int, int, int);
char CFileMgr__ReadLine_hook(int a1, int a2, int a3);

void RedirectCall(uintptr_t addr, uintptr_t func);

static char szLastBufferedName[40];
int (*cHandlingDataMgr__FindExactWord)(uintptr_t thiz, char* line, char* nameTable, int entrySize, int entryCount);
int cHandlingDataMgr__FindExactWord_hook(uintptr_t thiz, char* line, char* nameTable, int entrySize, int entryCount)
{
	strncpy(&szLastBufferedName[0], line, entrySize);
	return cHandlingDataMgr__FindExactWord(thiz, line, nameTable, entrySize, entryCount);
}

void (*cHandlingDataMgr__ConvertDataToGameUnits)(uintptr_t thiz, tHandlingData* handling);
void cHandlingDataMgr__ConvertDataToGameUnits_hook(uintptr_t thiz, tHandlingData* handling)
{
	int iHandling = ((int(*)(uintptr_t, char*))(SA_ADDR(0x4FBC4C + 1)))(thiz, &szLastBufferedName[0]);

	CHandlingDefault::FillDefaultHandling((uint16_t)iHandling, handling);

	return cHandlingDataMgr__ConvertDataToGameUnits(thiz, handling);
}

int32_t(*NVEventGetNextEvent_hooked)(NVEvent* ev, int waitMSecs);
int32_t NVEventGetNextEvent_hook(NVEvent* ev, int waitMSecs)
{
	int32_t ret = NVEventGetNextEvent_hooked(ev, waitMSecs);

	if (ret)
	{
		if (ev->m_type == NV_EVENT_MULTITOUCH)
		{
			// process manually
			ev->m_type = (NVEventType)228;
		}

	}

	NVEvent event;
	NVEventGetNextEvent(&event);

	if (event.m_type == NV_EVENT_MULTITOUCH)
	{
		int type = event.m_data.m_multi.m_action & NV_MULTITOUCH_ACTION_MASK;
		int num = (event.m_data.m_multi.m_action & NV_MULTITOUCH_POINTER_MASK) >> NV_MULTITOUCH_POINTER_SHIFT;

		int x1 = event.m_data.m_multi.m_x1;
		int y1 = event.m_data.m_multi.m_y1;

		int x2 = event.m_data.m_multi.m_x2;
		int y2 = event.m_data.m_multi.m_y2;

		int x3 = event.m_data.m_multi.m_x3;
		int y3 = event.m_data.m_multi.m_y3;

		if (type == NV_MULTITOUCH_CANCEL)
		{
			type = NV_MULTITOUCH_UP;
		}

		if ((x1 || y1) || num == 0)
		{
			if (num == 0 && type != NV_MULTITOUCH_MOVE)
				((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(type, 0, x1, y1); // AND_TouchEvent
			else ((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(NV_MULTITOUCH_MOVE, 0, x1, y1); // AND_TouchEvent
		}

		if ((x2 || y2) || num == 1)
		{
			if (num == 1 && type != NV_MULTITOUCH_MOVE)
				((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(type, 1, x2, y2); // AND_TouchEvent
			else ((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(NV_MULTITOUCH_MOVE, 1, x2, y2); // AND_TouchEvent
		}

		if ((x3 || y3) || num == 2)
		{
			if (num == 2 && type != NV_MULTITOUCH_MOVE)
				((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(type, 2, x3, y3); // AND_TouchEvent
			else ((void(*)(int, int, int posX, int posY))(SA_ADDR(0x239D5C + 1)))(NV_MULTITOUCH_MOVE, 2, x3, y3); // AND_TouchEvent
		}
	}

	return ret;
}

void(*CStreaming__Init2)();
void CStreaming__Init2_hook()
{
	CStreaming__Init2();
	*(uint32_t*)(SA_ADDR(0x5DE734)) = 536870912;
}

void (*ANDRunThread)(void* a1);
void ANDRunThread_hook(void* a1)
{
	g_fps.PushThread(gettid());

	ANDRunThread(a1);
}

void (*MainMenuScreen__OnExit)();
void MainMenuScreen__OnExit_hook()
{
	Log(OBFUSCATE("MainMenuScreen::OnExit: Exiting game..."));

	makeNOP(SA_ADDR(0x341FCC), 2); // nop PauseOpenSLES
	makeNOP(SA_ADDR(0x46389E), 2); // nop saving

	if (pNetGame)
	{
		if (pNetGame->GetRakClient())
			pNetGame->GetRakClient()->Disconnect(500, 0);
	}

	// std::this_thread::sleep_for(std::chrono::milliseconds(500));

	g_pJavaWrapper->FinishActivity();
	exit(EXIT_SUCCESS);
}

void CVehicleModelInfo__CLinkedUpgradeList__AddUpgradeLink_HOOK(int result, short int a2, short int a3)
{
	auto v1 = *(uint32_t *)(result + 400);
	if (v1 <= 99)
	{
		*(unsigned short *)(result + 2 + v1) = a2;

		auto v2 = *(uint32_t *)(result + 400);
		*(uint32_t *)(result + 400) = v2 + 1;

		result += 2 * v2;
		*(unsigned short *)(result + 200) = a3;
	}
}

void CVehicleModelInfo__CLinkedUpgradeList__FindOtherUpgrade_HOOK() { }

std::int8_t (__fastcall *CRunningScript_ProcessCommands200To299)(void *_this, std::int32_t CurrCommand);
std::int8_t __fastcall CRunningScript_ProcessCommands200To299_hook(void *_this, std::int32_t CurrCommand)
{
	if (CurrCommand == 263) {

		((void(*)(void *, int))(SA_ADDR(0x002E1254 + 1)))(_this, 4); // CRunningScript::CollectParameters

		int * ScriptParams = (int*)(SA_ADDR(0x007159E8));

		std::int32_t index = ScriptParams[0];

		Log("Current command 263: (%d)", index);
		
		return 0;
	}
	
	return CRunningScript_ProcessCommands200To299(_this, CurrCommand);
}

std::int8_t (__fastcall *CRunningScript_ProcessCommands400To499)(void *_this, std::int32_t CurrCommand);
std::int8_t __fastcall CRunningScript_ProcessCommands400To499_hook(void *_this, std::int32_t CurrCommand)
{
	if (CurrCommand == 444) {

		((void(*)(void *, int))(SA_ADDR(0x002E1254 + 1)))(_this, 4); // CRunningScript::CollectParameters

		// int * ScriptParams = (int*)(SA_ADDR(0x007159E8));

		// std::int32_t index = ScriptParams[0];

		Log("Current Command 444");
		
		return 0;
	}
	
	return CRunningScript_ProcessCommands400To499(_this, CurrCommand);
}

void (__fastcall *CStreaming_RequestModel)(int32_t index, int32_t flags);
void __fastcall CStreaming_RequestModel_hook(int32_t index, int32_t flags)
{
	// CStreaming::IsObjectInCdImage
	bool result = ((bool(*)(int32_t))(SA_ADDR(0x0028EAE8 + 1)))(index);

	// Log("Request Model: %d | %d", index, result);

	//if (!result) index = 14544;

	CStreaming_RequestModel(index, flags);
}

void InstallHookFixes();
void InstallSpecialHooks()
{
	WriteMemory(SA_ADDR(0x23BEDC), (uintptr_t)"\xF8\xB5", 2);
	WriteMemory(SA_ADDR(0x23BEDE), (uintptr_t)"\x00\x46\x00\x46", 4);

	installHook(SA_ADDR(0x0028EB10), (uintptr_t) CStreaming_RequestModel_hook, (uintptr_t *) &CStreaming_RequestModel);

	installHook(SA_ADDR(0x001BF244), (uintptr_t) TextureDatabaseRuntime_Load_hook, (uintptr_t *) &TextureDatabaseRuntime_Load);
	installHook(SA_ADDR(0x001BEB9C), (uintptr_t) TextureDatabaseRuntime_SortEntries_hook, (uintptr_t *) &TextureDatabaseRuntime_SortEntries);

	installHook(SA_ADDR(0x4FBBB0), (uintptr_t) cHandlingDataMgr__FindExactWord_hook, (uintptr_t *) &cHandlingDataMgr__FindExactWord);
	installHook(SA_ADDR(0x4FBCF4), (uintptr_t) cHandlingDataMgr__ConvertDataToGameUnits_hook, (uintptr_t *) &cHandlingDataMgr__ConvertDataToGameUnits);
	installHook(SA_ADDR(0x23ACC4), (uintptr_t) NVEventGetNextEvent_hook, (uintptr_t *) &NVEventGetNextEvent_hooked);
	installHook(SA_ADDR(0x4042A8), (uintptr_t) CStreaming__Init2_hook, (uintptr_t *) &CStreaming__Init2);

	installHook(SA_ADDR(0x269974), (uintptr_t) MenuItem_add_hook, (uintptr_t *) &MenuItem_add);
	installHook(SA_ADDR(0x4D3864), (uintptr_t) CText_Get_hook, (uintptr_t *) &CText_Get);
	installHook(SA_ADDR(0x40C530), (uintptr_t) InitialiseRenderWare_hook, (uintptr_t *) &InitialiseRenderWare);

                  installHook(SA_ADDR(0x23BEDC), (uintptr_t) OS_FileRead_hook, (uintptr_t *) &OS_FileRead);
	installHook(SA_ADDR(0x23B3DC), (uintptr_t) NvFOpen_hook, (uintptr_t *) &NvFOpen);

	installHook(SA_ADDR(0x25E660), (uintptr_t) MainMenuScreen__Update_hook, (uintptr_t *) &MainMenuScreen__Update);
	installHook(SA_ADDR(0x25CB8C), (uintptr_t) MainMenuScreen__OnExit_hook, (uintptr_t *) &MainMenuScreen__OnExit);
	installHook(SA_ADDR(0x23BB84), (uintptr_t) OS_FileOpen_hook, (uintptr_t *) &OS_FileOpen);

	// ... Huawei 
	if (!*(uintptr_t*)(SA_ADDR(0x61B298)))
		*(uintptr_t*)(SA_ADDR(0x61B298)) = ((uintptr_t(*)(const char*))(SA_ADDR(0x179A20)))(OBFUSCATE("glAlphaFuncQCOM"));

	if (!*(uintptr_t*)(SA_ADDR(0x61B298)))
		*(uintptr_t*)(SA_ADDR(0x61B298)) = ((uintptr_t(*)(const char*))(SA_ADDR(0x179A20)))(OBFUSCATE("glAlphaFunc"));

	if (!*(uintptr_t*)(SA_ADDR(0x61B298)))
		Log(OBFUSCATE("Detected crash! (glAlphaFuncQCOM + glAlphaFunc)"));
	// ...

	WriteMemory(SA_ADDR(0x1BDD4A), (uintptr_t)"\x10\x46\xA2\xF1\x04\x0B", 6);
	WriteMemory(SA_ADDR(0x3E1A2C), (uintptr_t)"\x67\xE0", 2);

	installHook(SA_ADDR(0x23768C), (uintptr_t) ANDRunThread_hook, (uintptr_t *)&ANDRunThread);

                  // .. PIXELS
	installJMPHook(SA_ADDR(0x1A1ED8), (uintptr_t) CRQ_Commands::rqVertexBufferSelect_HOOK);
	installJMPHook(SA_ADDR(0x1A1F6C), (uintptr_t) CRQ_Commands::rqVertexBufferDelete_HOOK);

	// ... CARS
	// installJMPHook(SA_ADDR(0x3379F8), (uintptr_t) CVehicleModelInfo__CLinkedUpgradeList__AddUpgradeLink_HOOK);
	// installJMPHook(SA_ADDR(0x337A14), (uintptr_t) CVehicleModelInfo__CLinkedUpgradeList__FindOtherUpgrade_HOOK);

	InstallHookFixes();
}

int(*RwFrameAddChild)(int, int);
int RwFrameAddChild_hook(int a1, int a2)
{
	if (a2 && a1)
		return RwFrameAddChild(a1, a2);

	else return 0;
}

void (*CAnimManager__UncompressAnimation)(int, int);
void CAnimManager__UncompressAnimation_hook(int a1, int a2)
{
	if (a1)
		CAnimManager__UncompressAnimation(a1, a2);
}

void(*CCustomRoadsignMgr__RenderRoadsignAtomic)(int, int);
void CCustomRoadsignMgr__RenderRoadsignAtomic_hook(int a1, int a2)
{
	if (a1)
		CCustomRoadsignMgr__RenderRoadsignAtomic(a1, a2);
}

int(*CUpsideDownCarCheck__IsCarUpsideDown)(int, int);
int CUpsideDownCarCheck__IsCarUpsideDown_hook(int a1, int a2)
{
	if (*(uintptr_t*)(a2 + 20))
		return CUpsideDownCarCheck__IsCarUpsideDown(a1, a2);

	return 0;
}

int(*CAnimBlendNode__FindKeyFrame)(int, float, int, int);
int CAnimBlendNode__FindKeyFrame_hook(int a1, float a2, int a3, int a4)
{
	if (*(uintptr_t*)(a1 + 16))
		return CAnimBlendNode__FindKeyFrame(a1, a2, a3, a4);

	else return 0;
}


/* ====================================================== */

typedef struct _ATOMIC_MODEL
{
	uintptr_t 	vtable;
	uint8_t		data[52];
} ATOMIC_MODEL; // SIZE = 56

ATOMIC_MODEL AtomicModels[30000];
int AtomicModelsCount = 0;

ATOMIC_MODEL* (*CModelInfo_AddAtomicModel)(int id);
ATOMIC_MODEL* CModelInfo_AddAtomicModel_hook(int id)
{
	ATOMIC_MODEL* model = &AtomicModels[AtomicModelsCount];
	memset(model, 0, sizeof(ATOMIC_MODEL));

	((void(*)(void* thiz))(SA_ADDR(0x33559C + 1)))((void*)model); // CBaseModelInfo::CBaseModelInfo();

	model->vtable = (uintptr_t)(SA_ADDR(0x5C6C68)); // assign CAtomicModelInfo vmt

	((uintptr_t(*)(ATOMIC_MODEL*))(*(void**)(model->vtable + 0x1C)))(model); // CClumpModelInfo::Initialise()

	*(ATOMIC_MODEL * *)(SA_ADDR(0x87BF48 + (id * 4))) = model; // CModelInfo::ms_modelInfoPtrs

	AtomicModelsCount++;
	return model;
}

/* ====================================================== */
typedef struct _VEHICLE_MODEL
{
	uintptr_t 	vtable;
	uint8_t		data[932];
} VEHICLE_MODEL; // SIZE = 936

VEHICLE_MODEL VehicleModels[1000];
int VehicleModelsCount = 0;

VEHICLE_MODEL* (*CModelInfo_AddVehicleModel)(int id);
VEHICLE_MODEL* CModelInfo_AddVehicleModel_hook(int id)
{
	VEHICLE_MODEL* model = &VehicleModels[VehicleModelsCount];
	memset(model, 0, sizeof(VEHICLE_MODEL));

	((void(*)(void* thiz))(SA_ADDR(0x337AA0 + 1)))((void*)model); // CVehicleModelInfo::CVehicleModelInfo();

	model->vtable = (uintptr_t)(SA_ADDR(0x5C6EE0));			// assign CVehicleModelInfo vmt

	((uintptr_t(*)(VEHICLE_MODEL*))(*(void**)(model->vtable + 0x1C)))(model); // CVehicleModelInfo::Initialise()

	*(VEHICLE_MODEL * *)(SA_ADDR(0x87BF48 + (id * 4))) = model; // CModelInfo::ms_modelInfoPtrs

	VehicleModelsCount++;
	Log("Loading vehicle(%d)...", VehicleModelsCount);
	return model;
}
/* ====================================================== */

void(*CHud__DrawScriptText)(uintptr_t, uint8_t);

float g_fMicrophoneButtonPosX;
float g_fMicrophoneButtonPosY;
bool g_IsVoiceServer();
bool isVoiceHidden();
void CHud__DrawScriptText_hook(uintptr_t thiz, uint8_t unk)
{
	CHud__DrawScriptText(thiz, unk);
	if (pGame && pNetGame)
	{
	

		if (g_pWidgetManager)
		{
			ImGuiIO& io = ImGui::GetIO();

			if (!g_pWidgetManager->GetSlotState(WIDGET_CHATHISTORY_UP))
			{
				g_pWidgetManager->New(WIDGET_CHATHISTORY_UP, 1700.0f, (io.DisplaySize.y * 0.3) - 180.0f, 130.0f, 130.0f, OBFUSCATE("menu_up"));
				g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_UP)->SetPosWithoutScale(pGUI->ScaleX(1325.0f), io.DisplaySize.y * 0.3);
			}

			if (!g_pWidgetManager->GetSlotState(WIDGET_CHATHISTORY_DOWN))
			{
				g_pWidgetManager->New(WIDGET_CHATHISTORY_DOWN, 1700.0f, (io.DisplaySize.y * 0.3) - 30, 130.0f, 130.0f, OBFUSCATE("menu_down"));
				g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_DOWN)->SetPosWithoutScale(pGUI->ScaleX(1515.0f), io.DisplaySize.y * 0.3);
			}

			if (!g_pWidgetManager->GetSlotState(WIDGET_MICROPHONE))
			{
				if (g_IsVoiceServer())
				{
					// pVoice
					g_fMicrophoneButtonPosX = pSettings->GetReadOnly().fButtonMicrophoneX;
					g_fMicrophoneButtonPosY = pSettings->GetReadOnly().fButtonMicrophoneY;

					g_pWidgetManager->New(WIDGET_MICROPHONE, pSettings->GetReadOnly().fButtonMicrophoneX, pSettings->GetReadOnly().fButtonMicrophoneY, pSettings->GetReadOnly().fButtonMicrophoneSize, pSettings->GetReadOnly().fButtonMicrophoneSize, OBFUSCATE("hud_dance"));
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetTexture(OBFUSCATE("samp"), OBFUSCATE("voice_button"));
				}
			}

			if (!g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
				g_pWidgetManager->New(WIDGET_CAMERA_CYCLE, pSettings->GetReadOnly().fButtonCameraCycleX, pSettings->GetReadOnly().fButtonCameraCycleY, pSettings->GetReadOnly().fButtonCameraCycleSize, pSettings->GetReadOnly().fButtonCameraCycleSize, OBFUSCATE("cam-toggle"));

			if (!g_pWidgetManager->GetSlotState(WIDGET_PASSENGER)) {
				g_pWidgetManager->New(WIDGET_PASSENGER, 100, 500, 128, 128, "hud_dance");
				g_pWidgetManager->GetWidget(WIDGET_PASSENGER)->SetTexture(OBFUSCATE("samp"), OBFUSCATE("passenger_button"));
			}
		}
	}
}

int(*CWidgetButtonEnterCar_Draw)(uintptr_t);

uint32_t g_uiLastTickVoice = 0;
int CWidgetButtonEnterCar_Draw_hook(uintptr_t thiz)
{
	if (g_pWidgetManager)
	{
		CWidget* pWidget = g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_UP);
		if (pWidget)
			pWidget->SetDrawState(false);

		pWidget = g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_DOWN);
		if (pWidget)
			pWidget->SetDrawState(false);

		pWidget = g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE);
		if (pWidget)
		{
			if (pKeyBoard && pKeyBoard->IsOpen())
				pWidget->SetDrawState(false);
			
			if (pKeyBoard && !pKeyBoard->IsOpen())
				pWidget->SetDrawState(true);
		}

		pWidget = g_pWidgetManager->GetWidget(WIDGET_PASSENGER);
		if (pWidget)
			pWidget->SetDrawState(false);

		auto pVehiclePool = pNetGame->GetVehiclePool();
		if (pVehiclePool)
		{
			VEHICLEID ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
			if (ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID))
			{
				auto pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
				if (pVehicle)
				{
					if (pVehicle->HasFreePassengerSeats() && pVehicle->GetDistanceFromLocalPlayerPed() < 5.0f)
					{
						auto pPlayerPool = pNetGame->GetPlayerPool();
						if (pPlayerPool)
						{
							if (!pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
							{
								pWidget = g_pWidgetManager->GetWidget(WIDGET_PASSENGER);
								if (pWidget)
									pWidget->SetDrawState(true);
							}
						}
					}
				}
			}
		}

		pWidget = g_pWidgetManager->GetWidget(WIDGET_MICROPHONE);
		if (pWidget)
		{
			if (pVoice)
			{
				pWidget->SetDrawState(true);
				static uint32_t lastTick = GetTickCount();
				if (pWidget->GetState() == 2 && GetTickCount() - lastTick >= 250)
				{
					pVoice->TurnRecording();
					if (pVoice->IsRecording())
					{
						g_uiLastTickVoice = GetTickCount();
						if (pVoice->IsDisconnected())
							pVoice->DisableInput();
					}

					lastTick = GetTickCount();
				}

				if (pVoice->IsRecording() && GetTickCount() - g_uiLastTickVoice >= 30000)
					pVoice->DisableInput();

				if (pVoice->IsRecording())
				{
					if (pVoice->IsDisconnected())
						pVoice->DisableInput();

						ImGuiIO &io = ImGui::GetIO();
						char bufferNotify[0xFF+1] = { '\0' };
						//pChatWindow->AddDebugMessage("IsRecording");
					//	sprintf(bufferNotify, "[%d] %s", pNetGame->GetPlayerPool()->GetLocalPlayerID(), pNetGame->GetPlayerPool()->GetLocalPlayerName());
					//	pNetGame->GetNotificationPool()->Add(0, 0, 0, ImVec2(pGUI->GetFontSize() * 5.5f, io.DisplaySize.y * 0.58f), bufferNotify, pGUI->GetFontSize(), 0xFFFAFAFA, true, 0xFF000000, 2, true, ImVec2(32, 32), pNetGame->GetNotificationPool()->GetCachedIconTexture(0), false);		
					
					//pWidget->SetColor(255, 0x9C, 0xCF, 0x9C);
					pWidget->SetTexture("samp", "voice_button2");
				}
				else {
					pWidget->ResetColor();
					pWidget->SetTexture("samp", "voice_button");
					pNetGame->GetNotificationPool()->Remove(0);
				}

				//if ((pWidget->GetState() == 2 || pWidget->GetState() == 0) && GetTickCount() - lastTick >= 250)
				//{
				//	pVoice->DisableInput();
				//	lastTick = GetTickCount();
				//}
			}
		}

		if (!pGame->IsToggledHUDElement(HUD_ELEMENT_BUTTONS))
		{
			for (int i = 0; i < MAX_WIDGETS; i++)
			{
				CWidget* pWidget = g_pWidgetManager->GetWidget(i);
				if (pWidget)
					pWidget->SetDrawState(false);
			}
		}

		g_pWidgetManager->Draw();
	}

	return CWidgetButtonEnterCar_Draw(thiz);
}

/*int CWidgetButtonEnterCar_Draw_hook(uintptr_t thiz)
{
	if (g_pWidgetManager)
	{
		CWidget* pWidget = g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_UP);
		if (pWidget)
		{
			pWidget->SetDrawState(false);
		}
		pWidget = g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_DOWN);
		if (pWidget)
		{
			pWidget->SetDrawState(false);
		}

		pWidget = g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE);
		if (pWidget)
		{
			pWidget->SetDrawState(true);
		}

		pWidget = g_pWidgetManager->GetWidget(WIDGET_MICROPHONE);
		if (pWidget)
		{
			if (pVoice)
			{
				pWidget->SetDrawState(true);
				static uint32_t lastTick = GetTickCount();
				if (pWidget->GetState() == 2 && GetTickCount() - lastTick >= 250)
				{
					pVoice->TurnRecording();
					if (pVoice->IsRecording())
					{
						g_uiLastTickVoice = GetTickCount();
						if (pVoice->IsDisconnected())
							pVoice->DisableInput();
					}

					lastTick = GetTickCount();
				}

				if (pVoice->IsRecording() && GetTickCount() - g_uiLastTickVoice >= 30000)
					pVoice->DisableInput();

				if (pVoice->IsRecording())
				{
					if (pVoice->IsDisconnected())
						pVoice->DisableInput();

						ImGuiIO &io = ImGui::GetIO();
						char bufferNotify[0xFF+1] = { '\0' };
						//pChatWindow->AddDebugMessage("IsRecording");
					//	sprintf(bufferNotify, "[%d] %s", pNetGame->GetPlayerPool()->GetLocalPlayerID(), pNetGame->GetPlayerPool()->GetLocalPlayerName());
					//	pNetGame->GetNotificationPool()->Add(0, 0, 0, ImVec2(pGUI->GetFontSize() * 5.5f, io.DisplaySize.y * 0.58f), bufferNotify, pGUI->GetFontSize(), 0xFFFAFAFA, true, 0xFF000000, 2, true, ImVec2(32, 32), pNetGame->GetNotificationPool()->GetCachedIconTexture(0), false);		
					
					//pWidget->SetColor(255, 0x9C, 0xCF, 0x9C);
					pWidget->SetTexture("samp", "voice_button2");
				}
				else {
					pWidget->ResetColor();
					pWidget->SetTexture("samp", "voice_button");
					pNetGame->GetNotificationPool()->Remove(0);
				}
			}
		}

		if (!pGame->IsToggledHUDElement(HUD_ELEMENT_BUTTONS))
		{
			for (int i = 0; i < MAX_WIDGETS; i++)
			{
				CWidget* pWidget = g_pWidgetManager->GetWidget(i);
				if (pWidget)
				{
					pWidget->SetDrawState(false);
				}
			}
		}

		g_pWidgetManager->Draw();
	}
	return CWidgetButtonEnterCar_Draw(thiz);
}*/

uint64_t(*CWorld_ProcessPedsAfterPreRender)();
uint64_t CWorld_ProcessPedsAfterPreRender_hook()
{
	uint64_t res = CWorld_ProcessPedsAfterPreRender();
	if (pNetGame && pNetGame->GetPlayerPool())
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayerPed* pPed = nullptr;
			if (pNetGame->GetPlayerPool()->GetLocalPlayerID() == i)
				pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
			else
			{
				if (pNetGame->GetPlayerPool()->GetSlotState(i))
					pPed = pNetGame->GetPlayerPool()->GetAt(i)->GetPlayerPed();
			}

			if (!pPed)
				continue;
			else pPed->ProcessAttach();
		}
	}

	return res;
}

int RemoveModelIDs[MAX_REMOVE_MODELS];
VECTOR RemovePos[MAX_REMOVE_MODELS];
float RemoveRad[MAX_REMOVE_MODELS];
int iTotalRemovedObjects = 0;

int(*CFileLoader__LoadObjectInstance)(uintptr_t, uintptr_t);
int CFileLoader__LoadObjectInstance_hook(uintptr_t thiz, uintptr_t name)
{
	for (int i = 0; i < iTotalRemovedObjects; i++)
	{
		if (RemoveModelIDs[i] == *(uint32_t*)(thiz + 28))
		{
			VECTOR pos;

			pos.X = *(float*)(thiz);
			pos.Y = *(float*)(thiz + 4);
			pos.Z = *(float*)(thiz + 8);

			if (GetDistanceBetween3DPoints(&pos, &RemovePos[i]) <= RemoveRad[i])
				*(int*)(thiz + 28) = 19300;
		}
	}
	return CFileLoader__LoadObjectInstance(thiz, name);
}

std::list<std::pair<unsigned int*, unsigned int>> resetEntries;

static uint32_t Color32Reverse(uint32_t x)
{
	return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
}

static RwRGBA DWORD2RGBAinternal(uint32_t dwColor)
{
	RwRGBA tmp;

	tmp.blue = dwColor & 0xFF; dwColor >>= 8;
	tmp.green = dwColor & 0xFF; dwColor >>= 8;
	tmp.red = dwColor & 0xFF; dwColor >>= 8;
	tmp.alpha = dwColor & 0xFF;

	/*tmp.blue = (dwColor) & 0xFF;
	tmp.green = (dwColor >> 8) & 0xFF;
	tmp.red = (dwColor >> 16) & 0xFF;
	tmp.alpha = (dwColor >> 24) & 0xFF;*/

	return tmp;
}

uintptr_t RwFrameForAllObjectsCALLBACK1(uintptr_t object, CObject* pObject)
{
	if (*(uint8_t*)object != 1)
		return object;

	uintptr_t pAtomic = object;
	RpGeometry* pGeom = *(RpGeometry * *)(pAtomic + 24);
	if (!pGeom)
		return object;

	int numMats = pGeom->matList.numMaterials;
	if (numMats > 16)
		numMats = 16;

	for (int i = 0; i < numMats; i++)
	{
		RpMaterial* pMat = pGeom->matList.materials[i];
		if (!pMat)
			continue;

		if (pObject->m_pMaterials[i].m_bCreated)
		{
			RpMaterial* pMat = pGeom->matList.materials[i];
			if (!pMat)
				continue;

			if (pObject->m_pMaterials[i].pTex)
			{
				resetEntries.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&pMat->texture), *reinterpret_cast<unsigned int*>(&pMat->texture)));
				pMat->texture = pObject->m_pMaterials[i].pTex;
			}
		}
	}

	return object;
}

int g_iLastRenderedObject;
void(*CObject__Render)(ENTITY_TYPE*);
void CObject__Render_hook(ENTITY_TYPE* thiz)
{
	// 0051ABB0 + 1
	// 004353FE + 1
	// 004352C4 + 1

	if (CSkyBox::GetSkyObject())
	{
		if (CSkyBox::GetSkyObject()->m_pEntity == thiz && !CSkyBox::IsNeedRender())
			return;
	}

	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (dwRetAddr == 0x0051ABB0 + 1 || dwRetAddr == 0x004353FE + 1 || dwRetAddr == 0x004352C4 + 1)
		return CObject__Render(thiz);

	uintptr_t pAtomic = thiz->m_RwObject;
	if (!pAtomic)
		return CObject__Render(thiz); // CObject::Render(CObject*)

	if (!*(uintptr_t*)(pAtomic + 4))
		return CObject__Render(thiz); // CObject::Render(CObject*)

	if (pNetGame)
	{
		CObjectPool* pObjectPool = pNetGame->GetObjectPool();
		if (pObjectPool)
		{
			CObject* pObject = pObjectPool->GetObjectFromGtaPtr(thiz);
			if (pObject)
			{
				if (pObject->m_pEntity)
					g_iLastRenderedObject = pObject->m_pEntity->nModelIndex;
				
				((void(*)())(SA_ADDR(0x00559EF8 + 1)))(); // DeActivateDirectional

				// Object Material
				if (pObject->m_bMaterials)
					((uintptr_t(*)(uintptr_t, uintptr_t, uintptr_t))(SA_ADDR(0x1AEE2C + 1)))(*(uintptr_t*)(pAtomic + 4), (uintptr_t)RwFrameForAllObjectsCALLBACK1, (uintptr_t)pObject); // RwFrameForAllObjects
			
				/*RwObject* rwObject = (RwObject*)pObject->GetRWObject();
				if(rwObject)
				{
					if(pObject->m_bHasMaterialText)
						RwFrameForAllObjects((RwFrame*)rwObject->parent, (RwObject *(*)(RwObject *, void *))ObjectMaterialTextCallBack, pObject);
				}*/
			}
			
		}
	}

    CObject__Render(thiz);

	for (auto& p : resetEntries)
		* p.first = p.second;

	resetEntries.clear();
}

void (*CMaterialText__Render)(uintptr_t thiz);
void CMaterialText__Render_hook(uintptr_t thiz)
{
	ENTITY_TYPE *object = (ENTITY_TYPE*)thiz;
	if(pNetGame && object != 0)
	{
		CObject *pObject = pNetGame->GetObjectPool()->GetObjectFromGtaPtr(object);
		if(pObject)
		{
			RwObject* rwObject = (RwObject*)pObject->GetRWObject();
			if(rwObject)
			{
                if(pObject->m_bHasMaterialText)
                    RwFrameForAllObjects((RwFrame*)rwObject->parent, (RwObject *(*)(RwObject *, void *))ObjectMaterialTextCallBack, pObject);
			}
		}
	}
	CallFunction<void>(g_libGTASA + 0x559EF9);
	CMaterialText__Render(thiz);
	CallFunction<void>(g_libGTASA + 0x559F15);
}

#pragma optimize( "", off )
#ifdef USE_SANTROPE_BULLSHIT
char CStreaming__ConvertBufferToObject_hook(int a1, int a2, int a3)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

	uint32_t tickStart = GetTickCount();
	//CGameResourcesDecryptor::CStreaming__ConvertBufferToObject_hook((char*)a1, a2, a3);
	if (a2 >= 15000 && a2 <= 15100)
	{
		//pChatWindow->AddDebugMessage("loading time %d", GetTickCount() - tickStart);
	}
	char a12 = CStreaming__ConvertBufferToObject(a1, a2, a3);
	return a12;
}
#endif

bool isEncrypted(const char* szArch)
{
	return false;

	for (const auto & i : encrArch)
		if (!strcmp(i.decrypt(), szArch)) return true;

	return false;
}

signed int (*OS_FileOpen)(unsigned int a1, int* a2, const char* a3, int a4);
signed int OS_FileOpen_hook(unsigned int a1, int* a2, const char* a3, int a4)
{
	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;
	signed int retn = OS_FileOpen(a1, a2, a3, a4);

	//Log("OS_FileOpen: %s", a3);

	if (calledFrom == 0x001BCE9A + 1)
	{
		if (isEncrypted(a3))
			lastOpenedFile = *a2;
	}

	return retn;
}

// CGameIntegrity
// CodeObfuscation

static uint32_t dwRLEDecompressSourceSize = 0;

int(*OS_FileRead)(void* a1, void* a2, int a3);
#ifdef USE_SANTROPE_BULLSHIT
	void InitCTX(AES_ctx& ctx, const uint8_t* pKey);
#endif
int OS_FileRead_hook(void* a1, void* a2, int a3)
{

	char *pLine = (char*)(a2);
	std::int32_t maxLen = (std::int32_t)a3;

	//Log("OS_FileRead: %s", pLine);


// ... пока сантроп, хз кто тя юзает кроме лайв рашки))
#ifdef USE_SANTROPE_BULLSHIT
	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	if (!a3) {
		return 0;
	}

	if (calledFrom == 0x001BCEE0 + 1 && a1 == (void*)lastOpenedFile)
	{
		lastOpenedFile = 0;
		
		AES_ctx ctx;
		InitCTX(ctx, &g_iEncryptionKeyTXD[0]);
		
		int retv = OS_FileRead(a1, a2, a3);
		int fileSize = a3;
		int iters = fileSize / PART_SIZE_TXD;

		auto pointer = (uintptr_t)a2;

		for (int i = 0; i < iters; i++)
		{
			AES_CBC_decrypt_buffer(&ctx, (uint8_t*)pointer, PART_SIZE_TXD);
			pointer += PART_SIZE_TXD;
		}

		return retv;
	}

	if (calledFrom == 0x001BDD34 + 1)
	{
		int retn = OS_FileRead(a1, a2, a3);

		dwRLEDecompressSourceSize = *(uint32_t*)a2;

		return retn;
	}
	else
	{
		int retn = OS_FileRead(a1, a2, a3);

		return retn;
	}
#else 
	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	if (!a3) {
		return 0;
	}

	if (calledFrom == 0x001BDD34 + 1)
	{
		int retn = OS_FileRead(a1, a2, a3);

		dwRLEDecompressSourceSize = *(uint32_t*)a2;

		return retn;
	}
	else
	{
		int retn = OS_FileRead(a1, a2, a3);

		return retn;
	}
#endif
}

/*int(*OS_FileRead)(void* a1, void* a2, int a3);
int OS_FileRead_hook(void* a1, void* a2, int a3)
{
	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	if (!a3)
	{
		return 0;
	}

	if (calledFrom == 0x001BCEE0 + 1 && a1 == (void*)lastOpenedFile)
	{
		lastOpenedFile = 0;
		CTinyEncrypt tinyEnc;
		tinyEnc.SetKey(&g_iEncryptionKeyVersion2TXD[0]);
		int retv = OS_FileRead(a1, a2, a3);
		int fileSize = a3;
		int iters = fileSize / PART_SIZE_TXD;
		uintptr_t pointer = (uintptr_t)a2;
		for (int i = 0; i < iters; i++)
		{
			tinyEnc.DecryptData((void*)pointer, PART_SIZE_TXD, 16);
			pointer += PART_SIZE_TXD;
		}
		return retv;
	}
	else
	{
		int retn = OS_FileRead(a1, a2, a3);

		return retn;
	}
}*/

char(*CFileMgr__ReadLine)(int, int, int);
char CFileMgr__ReadLine_hook(int a1, int a2, int a3)
{
#ifdef USE_SANTROPE_BULLSHIT
	char retv = CFileMgr__ReadLine(a1, a2, a3);
	char* pStr = (char*)a2;
	int value = *(int*)pStr;

	if (value == g_unobfuscate(g_iIdentifierVersion2))
	{
		pStr += 4;
		int length = *(int*)pStr;
		pStr -= 4;
		memcpy((void*)pStr, (const void*)& pStr[8], length);

		pStr[length] = 0;
		std::stringstream ss;

		uint32_t keyi = g_unobfuscate(g_i64Encrypt);

		ss << keyi;

		std::string key(ss.str());
		std::string val(pStr);

		std::string decr = decrypt(val, key);

		strcpy((char*)a2, decr.c_str());
	}

	return retv;
#else
	char retv = CFileMgr__ReadLine(a1, a2, a3);

	return retv;
#endif
}

#pragma optimize( "", on )

uintptr_t (*GetTexture_orig)(const char* a1, const char* a2);
uintptr_t GetTexture_hook(const char* a1, const char* a2)
{
	if(!a1) return 0;
	uintptr_t texture = ((uintptr_t(*)(const char*))(SA_ADDR(0x1BE990 + 1)))(a1);
	if (texture == 0) return 0;
	int count = *(int*)(texture + 0x54);
	count++;
	*(int*)(texture + 0x54) = count;
	return texture;
}

uintptr_t(*CPlayerInfo__FindObjectToSteal)(uintptr_t, uintptr_t);
uintptr_t CPlayerInfo__FindObjectToSteal_hook(uintptr_t a1, uintptr_t a2)
{
	return 0;
}

RwFrame* CClumpModelInfo_GetFrameFromId_Post(RwFrame* pFrameResult, RpClump* pClump, int id)
{
	if (pFrameResult)
		return pFrameResult;

	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	// �� ��������� ���� ���� ��� �����
	if (calledFrom == 0x00515708                // CVehicle::SetWindowOpenFlag
		|| calledFrom == 0x00515730             // CVehicle::ClearWindowOpenFlag
		|| calledFrom == 0x00338698             // CVehicleModelInfo::GetOriginalCompPosition
		|| calledFrom == 0x00338B2C)            // CVehicleModelInfo::CreateInstance
		return NULL;

	for (uint i = 2; i < 40; i++)
	{
		RwFrame* pNewFrameResult = NULL;
		uint     uiNewId = id + (i / 2) * ((i & 1) ? -1 : 1);
		
		pNewFrameResult = ((RwFrame * (*)(RpClump * pClump, int id))(SA_ADDR(0x335CC0 + 1)))(pClump, i);

		if (pNewFrameResult)
			return pNewFrameResult;
	}

	return NULL;
}
RwFrame* (*CClumpModelInfo_GetFrameFromId)(RpClump*, int);
RwFrame* CClumpModelInfo_GetFrameFromId_hook(RpClump* a1, int a2)
{
	return CClumpModelInfo_GetFrameFromId_Post(CClumpModelInfo_GetFrameFromId(a1, a2), a1, a2);
}

void (*CWidgetRegionLook__Update)(uintptr_t thiz);
void CWidgetRegionLook__Update_hook(uintptr_t thiz)
{
	if (bBlockCWidgetRegionLookUpdate)
		return;
	else return CWidgetRegionLook__Update(thiz);
}

uint8_t* (*RLEDecompress)(uint8_t* pDest, size_t uiDestSize, uint8_t const* pSrc, size_t uiSegSize, uint32_t uiEscape);
uint8_t* RLEDecompress_hook(uint8_t* pDest, size_t uiDestSize, uint8_t const* pSrc, size_t uiSegSize, uint32_t uiEscape)
{
	if (!pDest)
		return pDest;

	if (!pSrc)
		return pDest;

	uint8_t* pTempDest = pDest;
	const uint8_t* pTempSrc = pSrc;
	uint8_t* pEndOfDest = &pDest[uiDestSize];

	auto* pEndOfSrc = (uint8_t*)&pSrc[dwRLEDecompressSourceSize];

	if (pDest < pEndOfDest) 
	{
		do 
		{
			if (*pTempSrc == uiEscape) 
			{
				uint8_t ucCurSeg = pTempSrc[1];
				if (ucCurSeg) 
				{
					uint8_t* ucCurDest = pTempDest;
					uint8_t ucCount = 0;
					do 
					{
						++ucCount;
						pDest = (uint8_t*)memcpy(ucCurDest, pTempSrc + 2, uiSegSize);
						
						ucCurDest += uiSegSize;
					}
					while (ucCurSeg != ucCount);

					pTempDest += uiSegSize * ucCurSeg;
				}
				pTempSrc += 2 + uiSegSize;
			}

			else 
			{
				if (pTempSrc + uiSegSize >= pEndOfSrc)
					return pDest;
				else
				{
					pDest = (uint8_t*)memcpy(pTempDest, pTempSrc, uiSegSize);
					pTempDest += uiSegSize;
					pTempSrc += uiSegSize;
				}
			}
		}
		while (pEndOfDest > pTempDest);
	}

	dwRLEDecompressSourceSize = 0;

	return pDest;
}

char g_bufRenderQueueCommand[200];
uintptr_t g_dwRenderQueueOffset;

char* (*RenderQueue__ProcessCommand)(uintptr_t thiz, char* a2);
char* RenderQueue__ProcessCommand_hook(uintptr_t thiz, char* a2)
{
	if (thiz && a2)
	{
		auto* dwRenderQueue = (uintptr_t*)thiz;

		memset(g_bufRenderQueueCommand, 0, sizeof(g_bufRenderQueueCommand));

		g_dwRenderQueueOffset = *(uintptr_t*)a2;
		snprintf(g_bufRenderQueueCommand, 190, OBFUSCATE("offset: %d | name: %s"), g_dwRenderQueueOffset, (const char*)(*(dwRenderQueue + 100 + g_dwRenderQueueOffset)));

		return RenderQueue__ProcessCommand(thiz, a2);
	}
	else return nullptr;
}

int (*_rwFreeListFreeReal)(int a1, unsigned int a2);
int _rwFreeListFreeReal_hook(int a1, unsigned int a2)
{
	if (a1)
		return _rwFreeListFreeReal(a1, a2);
	else return 0;
}

void readVehiclesAudioSettings();
void (*CVehicleModelInfo__SetupCommonData)();
void CVehicleModelInfo__SetupCommonData_hook()
{
	CVehicleModelInfo__SetupCommonData();
	readVehiclesAudioSettings();
}

extern VehicleAudioPropertiesStruct VehicleAudioProperties[20000];
static uintptr_t addr_veh_audio = (uintptr_t)& VehicleAudioProperties[0];

void (*CAEVehicleAudioEntity__GetVehicleAudioSettings)(uintptr_t thiz, int16_t a2, int a3);
void CAEVehicleAudioEntity__GetVehicleAudioSettings_hook(uintptr_t dest, int16_t a2, int ID)
{
	memcpy((void*)dest, &VehicleAudioProperties[(ID - 400)], sizeof(VehicleAudioPropertiesStruct));
}

std::list<std::pair<unsigned int*, unsigned int>> resetEntriesVehicle;

RpMaterial* CVehicle__SetupRenderMatCB(RpMaterial* material, void* data)
{
	if (material)
	{
		if (material->texture)
		{
			CVehicle* pVeh = (CVehicle*)data;
			for (size_t i = 0; i < MAX_REPLACED_TEXTURES; i++)
			{
				if (pVeh->m_bReplaceTextureStatus[i])
				{
					if (!strcmp(&(material->texture->name[0]), &(pVeh->m_szReplacedTextures[i].szOld[0])))
					{
						if (pVeh->m_szReplacedTextures[i].pTexture)
						{
							resetEntriesVehicle.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&(material->texture)), *reinterpret_cast<unsigned int*>(&(material->texture))));
							material->texture = pVeh->m_szReplacedTextures[i].pTexture;

							if (strstr(pVeh->m_szReplacedTextures[i].szOld, OBFUSCATE("ret_t")))
								material->color.alpha = 255;
						}
					}
				}
			}
		}
	}

	return material;
}

uintptr_t CVehicle__SetupRenderCB(uintptr_t atomic, void* data)
{
	if (*(RpGeometry * *)(atomic + 24))
		((RpGeometry * (*)(RpGeometry*, uintptr_t, void*))(SA_ADDR(0x1E284C + 1)))(*(RpGeometry * *)(atomic + 24), (uintptr_t)CVehicle__SetupRenderMatCB, (void*)data); // RpGeometryForAllMaterials

	return atomic;
}

void(*CVehicleModelInfo__SetEditableMaterials)(uintptr_t);
void CVehicleModelInfo__SetEditableMaterials_hook(uintptr_t clump)
{
	#ifdef USE_SANTROPE_BULLSHIT
		PROTECT_CODE_MODELINFO_EDITABLE;
	#endif

	auto* pClump = (RpClump*)clump;

	if (pNetGame && pClump)
	{
		if (pNetGame->GetVehiclePool())
		{
			VEHICLEID vehId = pNetGame->GetVehiclePool()->FindIDFromRwObject((RwObject*)clump);
			CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(vehId);
			if (pVehicle)
			{
				if (pVehicle->m_bReplacedTexture)
					((RpClump * (*)(RpClump*, uintptr_t, void*))(g_libGTASA + 0x1E0EA0 + 1))(pClump, (uintptr_t)CVehicle__SetupRenderCB, (void*)pVehicle); // RpClumpForAllAtomics
			}
		}
	}

	CVehicleModelInfo__SetEditableMaterials(clump);
}

void (*CVehicle__ResetAfterRender)(uintptr_t);
void CVehicle__ResetAfterRender_hook(uintptr_t thiz)
{
	#ifdef USE_SANTROPE_BULLSHIT
		PROTECT_CODE_RESET_AFTER_RENDER;
	#endif

	for (auto& p : resetEntriesVehicle)
		*p.first = p.second;

	resetEntriesVehicle.clear();

	CVehicle__ResetAfterRender(thiz);
}

void* (*CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB)(void* object, RwInt32 offset, RwInt32 size);
void* CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB_hook(void* object, RwInt32 offset, RwInt32 size)
{
	if(pChatWindow) pChatWindow->AddDebugMessage(OBFUSCATE("m_objects %x"), *(uintptr_t * *)(SA_ADDR(0x669E48)));
	return CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB(object, offset, size);
}

static bool once = false;
static bool g_bFirstPersonOnFootEnabled = false;
void (*CGame__Process)();
void CGame__Process_hook()
{
	CGame__Process();

	if (!once)
	{
		CCustomPlateManager::Initialise();
		//CBinder::Initialise();
		// CSnow::Initialise();
		once = true;
	}

	/*if (pNetGame && pNetGame->GetPlayerPool() && pNetGame->GetPlayerPool()->GetLocalPlayer())
		CSnow::Process(pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed(), pGame->GetActiveInterior());*/

	if (pNetGame)
	{
		CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
		if (pTextDrawPool) {
			pTextDrawPool->SnapshotProcess();
		}
	}

	if (g_pWidgetManager)
	{
		PED_TYPE* pPed = GamePool_FindPlayerPed();
		if (g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE) && pPed)
		{
			static uint32_t lastTick = GetTickCount();
			bool bPressed = false;
			if (g_pWidgetManager->IsTouched(WIDGET_CAMERA_CYCLE) && GetTickCount() - lastTick >= 500)
			{
				if (pKeyBoard)
				{
					if (!pKeyBoard->IsOpen())
					{
						bPressed = true;
						lastTick = GetTickCount();
					}
				}
			}

			if (!CFirstPersonCamera::IsEnabled() && g_bFirstPersonOnFootEnabled)
				CFirstPersonCamera::SetEnabled(true);

			if (CFirstPersonCamera::IsEnabled() && !g_bFirstPersonOnFootEnabled)
				CFirstPersonCamera::SetEnabled(false);

			if (bPressed && !IN_VEHICLE(pPed))
			{
				CFirstPersonCamera::Toggle();
				if (CFirstPersonCamera::IsEnabled())
					g_bFirstPersonOnFootEnabled = true;
				else g_bFirstPersonOnFootEnabled = false;
			}

			if (g_pWidgetManager->GetSlotState(WIDGET_PASSENGER) && pPed)
			{
				if (g_pWidgetManager->IsTouched(WIDGET_PASSENGER) && GetTickCount() - lastTick >= 250)
				{
					bPressed = true;
					lastTick = GetTickCount();
				}

				if (bPressed && !IN_VEHICLE(pPed))
				{
					if (pNetGame)
					{
						auto pPlayerPool = pNetGame->GetPlayerPool();
						if (pPlayerPool)
						{
							auto pLocalPlayer = pPlayerPool->GetLocalPlayer();

							if (pLocalPlayer)
								pLocalPlayer->HandlePassengerEntryEx();
						}
					}
				}
			}
		}
	}

	if (pAudioStream) pAudioStream->Process();

	CCustomPlateManager::Process();
}

void (*CRenderer__RenderEverythingBarRoads)();
void CRenderer__RenderEverythingBarRoads_hook()
{
	CSkyBox::Process();

	CRenderer__RenderEverythingBarRoads();
}

uint16_t g_usLastProcessedModelIndexAutomobile = 0;
int g_iLastProcessedModelIndexAutoEnt = 0;

void (*CAutomobile__ProcessEntityCollision)(VEHICLE_TYPE* a1, ENTITY_TYPE* a2, int a3);
void CAutomobile__ProcessEntityCollision_hook(VEHICLE_TYPE* a1, ENTITY_TYPE* a2, int a3)
{
	#ifdef USE_SANTROPE_BULLSHIT
		PROTECT_CODE_AUTOMOBILE_COLLISION;
	#endif

	g_usLastProcessedModelIndexAutomobile = a1->entity.nModelIndex;
	g_iLastProcessedModelIndexAutoEnt = a2->nModelIndex;

	bool bReplace = false;
	void* pOld = nullptr;
	uint8_t* pColData = nullptr;

	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehId = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(a1);
			CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(vehId);
			if (pVeh)
			{
				if (pVeh->bHasSuspensionLines && pVeh->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
				{
					pColData = GetCollisionDataFromModel(a1->entity.nModelIndex);
					if (pColData && pVeh->m_pSuspensionLines)
					{
						if (*(void**)(pColData + 16))
						{
							pOld = *(void**)(pColData + 16);
							*(void**)(pColData + 16) = pVeh->m_pSuspensionLines;
							bReplace = true;
						}
					}
				}
			}
		}
	}
	CAutomobile__ProcessEntityCollision(a1, a2, a3);
	if (bReplace)
		*(void**)(pColData + 16) = pOld;
}

bool (*CGame__Shutdown)();
bool CGame__Shutdown_hook()
{
	Log(OBFUSCATE("CGame::Shutdown: Exiting game..."));

	makeNOP(SA_ADDR(0x341FCC), 2); // nop PauseOpenSLES
	makeNOP(SA_ADDR(0x46389E), 2); // nop saving

	if (pNetGame)
	{
		if (pNetGame->GetRakClient()) {
			pNetGame->GetRakClient()->Disconnect(500, 0);
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return CGame__Shutdown();
}

// TODO: VEHICLE RESET SUSPENSION
void (*CShadows__StoreCarLightShadow)(VEHICLE_TYPE* vehicle, int id, RwTexture* texture, VECTOR* posn, float frontX, float frontY, float sideX, float sideY, unsigned char red, unsigned char green, unsigned char blue, float maxViewAngle);
void CShadows__StoreCarLightShadow_hook(VEHICLE_TYPE* vehicle, int id, RwTexture* texture, VECTOR* posn, float frontX, float frontY, float sideX, float sideY, unsigned char red, unsigned char green, unsigned char blue, float maxViewAngle)
{
	uint8_t r, g, b;
	r = red;
	g = green;
	b = blue;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(vehicle);
			CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
				pVeh->ProcessHeadlightsColor(r, g, b);
		}
	}

	return CShadows__StoreCarLightShadow(vehicle, id, texture, posn, frontX, frontY, sideX, sideY, r, g, b, maxViewAngle);
}

void CVehicle__DoHeadLightReflectionTwin(CVehicle* pVeh, MATRIX4X4* a2)
{
	VEHICLE_TYPE* v2; // r4
	int v3; // r2
	MATRIX4X4* v4; // r5
	float* v5; // r3
	float v6; // s12
	float v7; // s5
	float* v8; // r2
	float v9; // r0
	float v10; // r1
	float v11; // r2
	float v12; // s14
	float v13; // s11
	float v14; // s15
	float v15; // s13
	float v16; // s10
	float v17; // s12
	float v18; // s15
	float v19; // ST08_4

	uintptr_t* dwModelarray = (uintptr_t*)(SA_ADDR(0x87BF48));

	v2 = pVeh->m_pVehicle;
	v3 = *((uintptr_t*)v2 + 5);
	v4 = a2;
	v5 = *(float**)(dwModelarray[v2->entity.nModelIndex] + 116);
	v6 = *v5;
	v7 = v5[1];
	if (v3)
		v8 = (float*)(v3 + 48);
	else
		v8 = (float*)((char*)v2 + 4);
	v9 = *v8;
	v10 = v8[1];
	v11 = v8[2];
	v12 = *((float*)v4 + 5);
	v13 = *((float*)v4 + 4);
	v14 = (float)(v12 * v12) + (float)(v13 * v13);
	if (v14 != 0.0)
		v14 = 1.0 / sqrtf(v14);
	v15 = v6 * 4.0;
	v16 = (float)(v15 + v15) + 1.0;
	v17 = v13 * v14;
	v18 = v12 * v14;

	v19 = v15 * v18;

	VECTOR pos;
	memcpy(&pos, &(v2->entity.mat->pos), sizeof(VECTOR));
	pos.Z += 2.0f;

	CShadows__StoreCarLightShadow(
		v2,
		(uintptr_t)v2 + 24,
		pVeh->m_Shadow.pTexture,
		&pos,
		(float)(v15 + v15) * v17 * pVeh->m_Shadow.fSizeX,
		(float)(v15 + v15) * v18 * pVeh->m_Shadow.fSizeX,
		v19 * pVeh->m_Shadow.fSizeY,
		-(float)(v15 * v17) * pVeh->m_Shadow.fSizeY,
		pVeh->m_Shadow.r, pVeh->m_Shadow.g, pVeh->m_Shadow.b,
		7.0f);
}

void (*CVehicle__DoHeadLightBeam)(VEHICLE_TYPE* vehicle, int arg0, MATRIX4X4& matrix, unsigned char arg2);
void CVehicle__DoHeadLightBeam_hook(VEHICLE_TYPE* vehicle, int arg0, MATRIX4X4& matrix, unsigned char arg2)
{
	uint8_t r, g, b;
	r = 0xFF;
	g = 0xFF;
	b = 0xFF;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(vehicle);
			CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
				pVeh->ProcessHeadlightsColor(r, g, b);
		}
	}

	*(uint8_t*)(SA_ADDR(0x9BAA70)) = r;
	*(uint8_t*)(SA_ADDR(0x9BAA71)) = g;
	*(uint8_t*)(SA_ADDR(0x9BAA72)) = b;

	*(uint8_t*)(SA_ADDR(0x9BAA94)) = r;
	*(uint8_t*)(SA_ADDR(0x9BAA95)) = g;
	*(uint8_t*)(SA_ADDR(0x9BAA96)) = b;

	*(uint8_t*)(SA_ADDR(0x9BAB00)) = r;
	*(uint8_t*)(SA_ADDR(0x9BAB01)) = g;
	*(uint8_t*)(SA_ADDR(0x9BAB02)) = b;

	CVehicle__DoHeadLightBeam(vehicle, arg0, matrix, arg2);

}

static CVehicle* g_pLastProcessedVehicleMatrix = nullptr;
static int g_iLastProcessedWheelVehicle = -1;

void (*CMatrix__Rotate)(void* thiz, float a1, float a2, float a3);
void CMatrix__Rotate_hook(void* thiz, float a1, float a2, float a3)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (dwRetAddr == 0x003A9D76 + 1)
	{
		CMatrix__Rotate(thiz, a1, a2, a3);
		return;
	}

	CMatrix__Rotate(thiz, a1, a2, a3);
	if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1)
	{
		if (g_pLastProcessedVehicleMatrix->m_bWheelAlignmentX || g_pLastProcessedVehicleMatrix->m_bWheelAlignmentY)
		{
			if (g_iLastProcessedWheelVehicle == 2)
				((void(*)(void*, float))(SA_ADDR(0x3E8BE4 + 1)))(thiz, 0.0f - g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY

			if (g_iLastProcessedWheelVehicle == 4)
				((void(*)(void*, float))(SA_ADDR(0x3E8BE4 + 1)))(thiz, 0.0f - g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY

			if (g_iLastProcessedWheelVehicle == 5)
				((void(*)(void*, float))(SA_ADDR(0x3E8BE4 + 1)))(thiz, g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY

			if (g_iLastProcessedWheelVehicle == 7)
				((void(*)(void*, float))(SA_ADDR(0x3E8BE4 + 1)))(thiz, g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY
		}
	}
}

void (*CMatrix__SetScale)(void* thiz, float x, float y, float z);
void CMatrix__SetScale_hook(void* thiz, float x, float y, float z)
{
	if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1)
	{
		if (g_iLastProcessedWheelVehicle >= 2 || g_iLastProcessedWheelVehicle <= 7)
		{
			// front wheel
			if (g_pLastProcessedVehicleMatrix->m_bWheelSize)
			{
				y *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f; // ���� ��� �������� scale �� ������
				z *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f;
			}

			if (g_pLastProcessedVehicleMatrix->m_bWheelWidth)
				x = g_pLastProcessedVehicleMatrix->m_fWheelWidth;
		}
	}

	CMatrix__SetScale(thiz, x, y, z);
}

void (*CAutomobile__UpdateWheelMatrix)(VEHICLE_TYPE* thiz, int, int);
void CAutomobile__UpdateWheelMatrix_hook(VEHICLE_TYPE* thiz, int nodeIndex, int flags)
{
	if (g_pLastProcessedVehicleMatrix)
		g_iLastProcessedWheelVehicle = nodeIndex;

	CAutomobile__UpdateWheelMatrix(thiz, nodeIndex, flags);
}

void (*CAutomobile__PreRender)(VEHICLE_TYPE* thiz);
void CAutomobile__PreRender_hook(VEHICLE_TYPE* thiz)
{
	uintptr_t* dwModelarray = (uintptr_t*)(SA_ADDR(0x87BF48));
	uint8_t* pModelInfoStart = (uint8_t*)dwModelarray[thiz->entity.nModelIndex];

	float fOldFront = *(float*)(pModelInfoStart + 88);
	float fOldRear = *(float*)(pModelInfoStart + 92);
	CVehicle* pVeh = nullptr;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(thiz);
			pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
			{
				pVeh->ProcessWheelsOffset();
				g_pLastProcessedVehicleMatrix = pVeh;

				if (pVeh->m_bWheelSize)
				{
					*(float*)(pModelInfoStart + 92) = pVeh->m_fWheelSize;
					*(float*)(pModelInfoStart + 88) = pVeh->m_fWheelSize;
				}

				if (pVeh->m_bShadow && pVeh->m_Shadow.pTexture)
					CVehicle__DoHeadLightReflectionTwin(pVeh, pVeh->m_pVehicle->entity.mat);
			}
		}
	}

	CAutomobile__PreRender(thiz);

	*(float*)(pModelInfoStart + 88) = fOldFront;
	*(float*)(pModelInfoStart + 92) = fOldRear;

	g_pLastProcessedVehicleMatrix = nullptr;
	g_iLastProcessedWheelVehicle = -1;
}

void (*CTaskSimpleUseGun__RemoveStanceAnims)(void* thiz, void* ped, float a3);
void CTaskSimpleUseGun__RemoveStanceAnims_hook(void* thiz, void* ped, float a3)
{
	uint32_t v5 = *((uint32_t*)thiz + 11);
	if (v5)
	{
		if (*(uint32_t*)(v5 + 20))
		{
			CTaskSimpleUseGun__RemoveStanceAnims(thiz, ped, a3);
			return;
		}
		else return;
	}
	else CTaskSimpleUseGun__RemoveStanceAnims(thiz, ped, a3);
}

float (*CRadar__LimitRadarPoint)(float* a1);
float CRadar__LimitRadarPoint_hook(float* a1)
{
	if (*(uint8_t*)(SA_ADDR(0x63E0B4)))
		return sqrtf((float)(a1[1] * a1[1]) + (float)(*a1 * *a1));

	if (!CRadarRect::IsEnabled())
		return CRadar__LimitRadarPoint(a1);

	return CRadarRect::CRadar__LimitRadarPoint_hook(a1);
}

void (*CSprite2d__DrawBarChart)(float x, float y, unsigned short width, unsigned char height, float progress, signed char progressAdd,
	unsigned char drawPercentage, unsigned char drawBlackBorder, CRGBA* color, CRGBA* addColor);
void CSprite2d__DrawBarChart_hook(float x, float y, unsigned short width, unsigned char height, float progress, signed char progressAdd,
	unsigned char drawPercentage, unsigned char drawBlackBorder, CRGBA* color, CRGBA* addColor)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	float fX = x;
	float fY = y;

	unsigned short usWidth = width;
	unsigned char usHeight = height;

	if (dwRetAddr == 0x0027D524 + 1)
	{
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).X != -1)
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).X);

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).Y != -1)
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).Y);

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).X != -1)
			usWidth = pGUI->ScaleX(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).X);

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).Y != -1)
			usHeight = pGUI->ScaleY(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).Y);

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_HP))
		{
			color->A = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).A;
			color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).R;
			color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).G;
			color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).B;
		}
	}
	else if (dwRetAddr == 0x0027D83E + 1)
	{
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).X != -1)
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).X);

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).Y != -1)
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).Y);

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).X != -1)
			usWidth = pGUI->ScaleX(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).X);

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).Y != -1)
			usHeight = pGUI->ScaleY(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).Y);

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_ARMOR))
		{
			color->A = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).A;
			color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).R;
			color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).G;
			color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).B;
		}
	}

	CSprite2d__DrawBarChart(fX, fY, usWidth, usHeight, progress, progressAdd, drawPercentage, drawBlackBorder, color, addColor);
}

static int g_iCurrentWanted = 0;
static float g_fInitialPos = 0.0f;

void (*CWidgetPlayerInfo__DrawWanted)(void*);
void CWidgetPlayerInfo__DrawWanted_hook(void* thiz)
{
	g_iCurrentWanted = 0;
	g_fInitialPos = *((float*)thiz + 10);
	CWidgetPlayerInfo__DrawWanted(thiz);
	g_iCurrentWanted = 0;
}

void (*CFont__PrintString)(float x, float y, uint16_t* text);
void CFont__PrintString_hook(float x, float y, uint16_t* text)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	float fX = x;
	float fY = y;

	if (dwRetAddr == 0x0027E15C + 1) // money
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_MONEY))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_MONEY);
			CFont::SetColor(&col);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_MONEY).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_MONEY).X / 100.0f;
			CFont::SetScale(value);
		}

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).X != -1)
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).X);

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).Y != -1)
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).Y);
	}
	else if (dwRetAddr == 0x0027D9E6 + 1) // wanted
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_WANTED))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_WANTED);
			CFont::SetColor(&col);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_WANTED).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_WANTED).X / 100.0f;
			CFont::SetScale(value);
		}
		
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).X != -1)
		{
			fX -= g_fInitialPos;
			fX += pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).X);
		}

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).Y != -1)
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).Y);

		g_iCurrentWanted++;

	}
	else if (dwRetAddr == 0x0027D330 + 1) // ammo text
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_AMMO))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_AMMO);
			CFont::SetColor(&col);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_AMMO).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_AMMO).X / 100.0f;
			CFont::SetScale(value);
		}

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).X != -1)
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).X);

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).Y != -1)
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).Y);
	}

	CFont__PrintString(fX, fY, text);
}

void (*CSprite2d__Draw)(CSprite2d* a1, CRect* a2, CRGBA* a3);
void CSprite2d__Draw_hook(CSprite2d* a1, CRect* a2, CRGBA* a3)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (!pGUI)
		return CSprite2d__Draw(a1, a2, a3);

	if(reinterpret_cast<uintptr_t>(a1) == SA_ADDR(0x8F0708) && pCrossHair->m_UsedCrossHair) // removing the standard sight (CrossHair)
		return;

	if (dwRetAddr == 0x003D3796 + 1 || dwRetAddr == 0x003D376C + 1 || dwRetAddr == 0x003D373E + 1 || dwRetAddr == 0x003D3710 + 1)
	{
		if (CRadarRect::m_pDiscTexture == nullptr)
			CRadarRect::m_pDiscTexture = a1->m_pRwTexture;

		if (CRadarRect::IsEnabled() && CRadarRect::m_pRectTexture) a1->m_pRwTexture = CRadarRect::m_pRectTexture;
		else a1->m_pRwTexture = CRadarRect::m_pDiscTexture;

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_RADAR))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_RADAR);
			a3->A = col.A;
			a3->B = col.B;
			a3->G = col.G;
			a3->R = col.R;
		}

		auto* thiz = (float*) * (uintptr_t*)(SA_ADDR(0x6580C8));
		if (thiz)
		{
			if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).X != -1)
				thiz[3] = (float)CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).X;
			else thiz[3] = 50.0f;

			if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).Y != -1)
				thiz[4] = (float)CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).Y;
			else thiz[4] = 70.0f;

			if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X != -1)
				thiz[5] = 45.0f * ((float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X / 100.0f);
			else thiz[5] = 45.0f;

			if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X != -1)
				thiz[6] = 45.0f * ((float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X / 100.0f);
			else thiz[6] = 45.0f;
		}
		
	}

	return CSprite2d__Draw(a1, a2, a3);
}

void (*CSprite2d__DrawRect)(RECT* a1, float a2);
void CSprite2d__DrawRect__hook(RECT* a1, float a2)
{
	// siteM16 rect
	float fCHairScreenMultX = RsGlobal->maximumWidth * *(float*)(SA_ADDR(0x8B07FC));
	float fCHairScreenMultY = RsGlobal->maximumHeight * *(float*)(SA_ADDR(0x8B07F8));
	
	if(a1->fLeft == (fCHairScreenMultX - 1.0) && a1->fRight == (fCHairScreenMultX + 1.0))
		return;
	
	return CSprite2d__DrawRect(a1, a2);
}

void (*CWidgetPlayerInfo__DrawWeaponIcon)(float* thiz, void* ped, CRect rect, float a4);
void CWidgetPlayerInfo__DrawWeaponIcon_hook(float* thiz, void* ped, CRect rect, float a4)
{
	float diffX = rect.right - rect.left;
	float diffY = rect.bottom - rect.top;

	if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X != -1)
	{
		rect.left = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X);
		rect.right = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X) + diffX;

		thiz[38] = rect.left;
		thiz[40] = rect.right;
	}

	if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y != -1)
	{
		rect.top = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y);
		rect.bottom = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y) + diffY;

		thiz[39] = rect.bottom;
		thiz[41] = rect.top;
	}

	if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_FIST).X != -1)
	{
		float coef = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_FIST).X / 100.0f;
		float diffX = rect.right - rect.left;
		float diffY = rect.bottom - rect.top;

		diffX *= coef;
		diffY *= coef;

		rect.right = rect.left + diffX;
		rect.bottom = rect.top + diffY;
	}

	return CWidgetPlayerInfo__DrawWeaponIcon(thiz, ped, rect, a4);
}

void(*CHud__Draw)();
void CHud__Draw_hook()
{
	float* thiz = (float*) * (uintptr_t*)(SA_ADDR(0x6580C8));
	if (thiz)
	{
		thiz[5] = 45.0f;
		thiz[6] = 45.0f;
	}

	CHud__Draw();
}

void (*CCam__Process)(uintptr_t);
void CCam__Process_hook(uintptr_t thiz)
{
	if (!CFirstPersonCamera::IsEnabled()) return CCam__Process(thiz);

	VECTOR vecSpeed;
	CVehicle* pVeh = nullptr;
	float pOld = *(float*)(SA_ADDR(0x608558));
	if (pNetGame && (*(uint16_t*)(thiz + 14) == 18 || *(uint16_t*)(thiz + 14) == 16) && CFirstPersonCamera::IsEnabled())
	{
		if (pNetGame->GetPlayerPool())
		{
			if (pNetGame->GetPlayerPool()->GetLocalPlayer())
			{
				CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
				pVeh = pNetGame->GetVehiclePool()->GetAt(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_CurrentVehicle);
				if (pVeh)
				{
					VECTOR vec;

					pVeh->GetMoveSpeedVector(&vecSpeed);

					vec.X = vecSpeed.X * 6.0f;
					vec.Y = vecSpeed.Y * 6.0f;
					vec.Z = vecSpeed.Z * 6.0f;

					pVeh->SetMoveSpeedVector(vec);
					*(float*)(SA_ADDR(0x608558)) = 200.0f;
				}
			}
		}
	}

	CCam__Process(thiz);

	if (pVeh)
	{
		pVeh->SetMoveSpeedVector(vecSpeed);
		*(float*)(SA_ADDR(0x608558)) = pOld;
	}

	if (*(uint16_t*)(thiz + 14) == 4 || *(uint16_t*)(thiz + 14) == 53) // 53 is weapon
	{
		if (pNetGame)
		{
			if (pNetGame->GetPlayerPool())
			{
				if (pNetGame->GetPlayerPool()->GetLocalPlayer())
				{
					CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
					if (pPed)
					{
						*(uint32_t*)(SA_ADDR(0x8B0808 + 120)) = 0xFFFFFFFF;
						*(uint32_t*)(SA_ADDR(0x8B0808 + 124)) = 0xFFFFFFFF;
						*(uint8_t*)(SA_ADDR(0x8B0808 + 40)) = 0;

						CFirstPersonCamera::ProcessCameraOnFoot(thiz, pPed);
					}
				}
			}
		}
	}

	if(* (uint16_t*)(thiz + 14) == 18 || *(uint16_t*)(thiz + 14) == 16) // in car
	{
		if (pNetGame)
		{
			if (pNetGame->GetPlayerPool())
			{
				if (pNetGame->GetPlayerPool()->GetLocalPlayer())
				{
					CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
					if (pPed)
					{
						*(uint32_t*)(SA_ADDR(0x8B0808 + 120)) = 0xFFFFFFFF;
						*(uint32_t*)(SA_ADDR(0x008B0808 + 124)) = 0xFFFFFFFF;
						*(uint8_t*)(SA_ADDR(0x008B0808 + 40)) = 0;

						CFirstPersonCamera::ProcessCameraInVeh(thiz, pPed, pVeh);
					}
				}
			}
		}
	}
}

int g_iCounterVehicleCamera = 0;

int (*CPad__CycleCameraModeDownJustDown)(void*);
int CPad__CycleCameraModeDownJustDown_hook(void* thiz)
{
	if (!g_pWidgetManager)
		return 0;

	if (!g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
		return 0;

	PED_TYPE* pPed = GamePool_FindPlayerPed();
	if (!pPed)
		return 0;

	static uint32_t lastTick = GetTickCount();
	bool bPressed = false;

	if (g_pWidgetManager->IsTouched(WIDGET_CAMERA_CYCLE) && GetTickCount() - lastTick >= 500)
	{
		bPressed = true;
		lastTick = GetTickCount();
	}

	if (IN_VEHICLE(pPed))
	{
		if (bPressed)
			g_iCounterVehicleCamera++;

		if (g_iCounterVehicleCamera == 6)
		{
			CFirstPersonCamera::SetEnabled(true);
			return 0;
		}
		else if (g_iCounterVehicleCamera >= 7)
		{
			g_iCounterVehicleCamera = 0;
			CFirstPersonCamera::SetEnabled(false);
			return 1;
			
		}
		else CFirstPersonCamera::SetEnabled(false);

		return bPressed;
	}
	return 0;
}

int (*CEntity__GetIsOnScreen)(ENTITY_TYPE*);
int CEntity__GetIsOnScreen_hook(ENTITY_TYPE* thiz)
{
	int retn = CEntity__GetIsOnScreen(thiz);

	return retn;
}

void (*FxEmitterBP_c__Render)(uintptr_t* a1, int a2, int a3, float a4, char a5);
void FxEmitterBP_c__Render_hook(uintptr_t* a1, int a2, int a3, float a4, char a5)
{
	uintptr_t* temp = *((uintptr_t**)a1 + 3);
	if (!temp)
		return;

	FxEmitterBP_c__Render(a1, a2, a3, a4, a5);
}

void(*CStreaming__RemoveModel)(int);
void CStreaming__RemoveModel_hook(int model)
{
	Log(OBFUSCATE("Removing model %d"), model);
	CStreaming__RemoveModel(model);
}

int g_iLastProcessedSkinCollision = 228;
int g_iLastProcessedEntityCollision = 228;

void (*CPed__ProcessEntityCollision)(PED_TYPE* thiz, ENTITY_TYPE* ent, void* colPoint);
void CPed__ProcessEntityCollision_hook(PED_TYPE* thiz, ENTITY_TYPE* ent, void* colPoint)
{
	g_iLastProcessedSkinCollision = thiz->entity.nModelIndex;
	g_iLastProcessedEntityCollision = ent->nModelIndex;

	CPed__ProcessEntityCollision(thiz, ent, colPoint);
}

uint32_t (*CWeapon_FireInstantHit)(WEAPON_SLOT_TYPE* _this, PED_TYPE* pFiringEntity, VECTOR* vecOrigin, VECTOR* muzzlePos, ENTITY_TYPE* targetEntity, VECTOR *target, VECTOR* originForDriveBy, int arg6, int muzzle);
uint32_t CWeapon_FireInstantHit_hook(WEAPON_SLOT_TYPE* _this, PED_TYPE* pFiringEntity, VECTOR* vecOrigin, VECTOR* muzzlePos, ENTITY_TYPE* targetEntity, VECTOR *target, VECTOR* originForDriveBy, int arg6, int muzzle)
{
	uintptr_t dwRetAddr = 0;
 	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

 	dwRetAddr -= g_libGTASA;
 	if(	dwRetAddr == 0x569A84 + 1 ||
 		dwRetAddr == 0x569616 + 1 ||
 		dwRetAddr == 0x56978A + 1 ||
 		dwRetAddr == 0x569C06 + 1)
 	{
		PED_TYPE *pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
		if(pLocalPed)
		{
			if(pFiringEntity != pLocalPed)
				return muzzle;

			if(pNetGame)
			{
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool)
				{
					pPlayerPool->ApplyCollisionChecking();
				}
			}

			if(pGame)
			{
				CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
				if(pPlayerPed)
				{
					pPlayerPed->FireInstant();
				}
			}

			if(pNetGame)
			{
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool)
				{
					pPlayerPool->ResetCollisionChecking();
				}
			}

			return muzzle;
		}
 	}

 	return CWeapon_FireInstantHit(_this, pFiringEntity, vecOrigin, muzzlePos, targetEntity, target, originForDriveBy, arg6, muzzle);
}

uint32_t (*CWeapon__FireSniper)(WEAPON_SLOT_TYPE *pWeaponSlot, PED_TYPE *pFiringEntity, ENTITY_TYPE *a3, VECTOR *vecOrigin); 
uint32_t CWeapon__FireSniper_hook(WEAPON_SLOT_TYPE *pWeaponSlot, PED_TYPE *pFiringEntity, ENTITY_TYPE *a3, VECTOR *vecOrigin) 
{ 
    if(GamePool_FindPlayerPed() == pFiringEntity) 
    { 
        if(pGame) 
        { 
            CPlayerPed* pPlayerPed = pGame->FindPlayerPed(); 
            if(pPlayerPed) 
                pPlayerPed->FireInstant(); 
          } 
     } 
    
    return 1; 
}

void SendBulletSync(VECTOR* vecOrigin, VECTOR* vecEnd, VECTOR* vecPos, ENTITY_TYPE** ppEntity)
{
	BULLET_DATA bulletData;
	memset(&bulletData, 0, sizeof(BULLET_DATA));

	bulletData.vecOrigin.X = vecOrigin->X;
	bulletData.vecOrigin.Y = vecOrigin->Y;
	bulletData.vecOrigin.Z = vecOrigin->Z;
	bulletData.vecPos.X = vecPos->X;
	bulletData.vecPos.Y = vecPos->Y;
	bulletData.vecPos.Z = vecPos->Z;

	if (ppEntity) {
		ENTITY_TYPE *pEntity = *ppEntity;
		if (pEntity) {
			if (pEntity->mat) 
			{
				bulletData.vecOffset.X = vecPos->X - pEntity->mat->pos.X;
				bulletData.vecOffset.Y = vecPos->Y - pEntity->mat->pos.Y;
				bulletData.vecOffset.Z = vecPos->Z - pEntity->mat->pos.Z;
			}

			bulletData.pEntity = pEntity;
		}
	}

	pGame->FindPlayerPed()->ProcessBulletData(&bulletData);
}

uint32_t(*CWorld__ProcessLineOfSight)(VECTOR*, VECTOR*, VECTOR*, PED_TYPE**, bool, bool, bool, bool, bool, bool, bool, bool);
uint32_t CWorld__ProcessLineOfSight_hook(VECTOR* vecOrigin, VECTOR* vecEnd, VECTOR* vecPos, PED_TYPE** ppEntity,
	bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

	dwRetAddr -= g_libGTASA;
	if (dwRetAddr == 0x0055E2FE + 1 ||
		dwRetAddr == 0x005681BA + 1 ||
		dwRetAddr == 0x00567AFC + 1)
	{
		ENTITY_TYPE* pEntity = nullptr;
		MATRIX4X4* pMatrix = nullptr;
		static VECTOR vecPosPlusOffset;

		if (pNetGame->GetLagCompensation() != 2)
		{
			if (g_pCurrentFiredPed != pGame->FindPlayerPed())
			{
				if (g_pCurrentBulletData)
				{
					if (g_pCurrentBulletData->pEntity)
					{
						if (!IsGameEntityArePlaceable(g_pCurrentBulletData->pEntity))
						{
							pMatrix = g_pCurrentBulletData->pEntity->mat;
							if (pMatrix)
							{
								if (pNetGame->GetLagCompensation())
								{
									vecPosPlusOffset.X = pMatrix->pos.X + g_pCurrentBulletData->vecOffset.X;
									vecPosPlusOffset.Y = pMatrix->pos.Y + g_pCurrentBulletData->vecOffset.Y;
									vecPosPlusOffset.Z = pMatrix->pos.Z + g_pCurrentBulletData->vecOffset.Z;
								}
								else ProjectMatrix(&vecPosPlusOffset, pMatrix, &g_pCurrentBulletData->vecOffset);

								vecEnd->X = vecPosPlusOffset.X - vecOrigin->X + vecPosPlusOffset.X;
								vecEnd->Y = vecPosPlusOffset.Y - vecOrigin->Y + vecPosPlusOffset.Y;
								vecEnd->Z = vecPosPlusOffset.Z - vecOrigin->Z + vecPosPlusOffset.Z;
							}
						}
					}
				}
			}
		}

		static uint32_t result = 0;
		result = CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5, b6, b7, b8);

		if (pNetGame->GetLagCompensation() == 2)
		{
			if (g_pCurrentFiredPed)
			{
				if (g_pCurrentFiredPed == pGame->FindPlayerPed())
					SendBulletSync(vecOrigin, vecEnd, vecPos, (ENTITY_TYPE**)ppEntity);
			}

			return result;
		}

		if (g_pCurrentFiredPed)
		{
			if (g_pCurrentFiredPed != pGame->FindPlayerPed())
			{
				if (g_pCurrentBulletData)
				{
					if (!g_pCurrentBulletData->pEntity)
					{
						PED_TYPE* pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
						if (*ppEntity == pLocalPed || (IN_VEHICLE(pLocalPed) && *(uintptr_t*)ppEntity == pLocalPed->pVehicle))
						{
							*ppEntity = nullptr;

							vecPos->X = 0.0f;
							vecPos->Y = 0.0f;
							vecPos->Z = 0.0f;

							return 0;
						}
					}
				}
			}
		}

		if (g_pCurrentFiredPed)
		{
			if (g_pCurrentFiredPed == pGame->FindPlayerPed())
				SendBulletSync(vecOrigin, vecEnd, vecPos, (ENTITY_TYPE**)ppEntity);
		}

		return result;
	}

	return CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5, b6, b7, b8);
}

bool (*CAutomobile__BurstTyre)(uintptr_t thiz, int a2, int a3);
bool CAutomobile__BurstTyre_hook(uintptr_t thiz, int a2, int a3)
{
	if (pNetGame)
	{
		// Did we get ped who fired?
		if (g_pCurrentFiredPed)
		{
			// Trying to get vehicle pointer
			CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
			if (pVehiclePool)
			{
				VEHICLEID vehId = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE*)thiz);
				if (vehId != INVALID_VEHICLE_ID)
				{
					CVehicle* pVehicle = pVehiclePool->GetAt(vehId);
					if (pVehicle)
						pVehicle->ProcessDamage();
				}
			}
		}
	}
	return CAutomobile__BurstTyre(thiz, a2, a3);
}

int(*CVehicle__UsesSiren)(uintptr_t thiz);
int CVehicle__UsesSiren_hook(uintptr_t thiz)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

	dwRetAddr -= g_libGTASA;
	if (dwRetAddr == 0x4E53E0 + 1 ||
		dwRetAddr == 0x35BFE4 + 1 ||
		dwRetAddr == 0x518920 + 1)
	{
		if (pNetGame)
		{
			VEHICLE_TYPE* pGtaVehicle = (VEHICLE_TYPE*)thiz;
			if (pGtaVehicle)
			{
				CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
				if (pVehiclePool)
				{
					VEHICLEID vehId = pVehiclePool->FindIDFromGtaPtr(pGtaVehicle);
					if (vehId != INVALID_VEHICLE_ID)
					{
						if (pVehiclePool->GetSlotState(vehId))
						{
							CVehicle* pVehicle = pVehiclePool->GetAt(vehId);
							if (pVehicle)
							{
								if (pVehicle->IsSirenAdded())
									return 1;
							}
						}
					}
				}
			}
		}
	}

	return CVehicle__UsesSiren(thiz);
}

/* =========================================== Ped damage handler =========================================== */
void onDamage(PED_TYPE* issuer, PED_TYPE* damaged)
{
	if (!pNetGame) return;
	PED_TYPE* pPedPlayer = GamePool_FindPlayerPed();
	if (damaged && (pPedPlayer == issuer))
	{
		if (pNetGame->GetPlayerPool()->FindRemotePlayerIDFromGtaPtr((PED_TYPE*)damaged) != INVALID_PLAYER_ID)
		{
			CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
			CAMERA_AIM* caAim = pPlayerPool->GetLocalPlayer()->GetPlayerPed()->GetCurrentAim();

			VECTOR aim;
			aim.X = caAim->f1x;
			aim.Y = caAim->f1y;
			aim.Z = caAim->f1z;

			pPlayerPool->GetLocalPlayer()->SendBulletSyncData(pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE*)damaged), BULLET_HIT_TYPE_PLAYER, aim);
		}
	}
}

float m_fWeaponDamagess[55] = 
{
	5.0,   // 0 - Fist
	5.0,   // 1 - Brass knuckles
	5.0,   // 2 - Golf club
	5.0,   // 3 - Nitestick
	5.0,   // 4 - Knife
	5.0,   // 5 - Bat
	5.0,   // 6 - Shovel
	5.0,   // 7 - Pool cue
	5.0,   // 8 - Katana
	5.0,   // 9 - Chainsaw
	5.0,   // 10 - Dildo
	5.0,   // 11 - Dildo 2
	5.0,   // 12 - Vibrator
	5.0,   // 13 - Vibrator 2
	5.0,   // 14 - Flowers
	5.0,   // 15 - Cane
	82.5,  // 16 - Grenade
	0.0,   // 17 - Teargas
	1.0,   // 18 - Molotov
	9.9,   // 19 - Vehicle M4 (custom)
	46.2,  // 20 - Vehicle minigun (custom)
	0.0,   // 21
	8.25,  // 22 - Colt 45
	13.2,  // 23 - Silenced
	46.2,  // 24 - Deagle
	3.3,   // 25 - Shotgun
	3.3,   // 26 - Sawed-off
	4.95,  // 27 - Spas
	6.6,   // 28 - UZI
	8.25,  // 29 - MP5
	9.9,   // 30 - AK47
	9.9,   // 31 - M4
	6.6,   // 32 - Tec9
	24.75, // 33 - Cuntgun
	41.25, // 34 - Sniper
	82.5,  // 35 - Rocket launcher
	82.5,  // 36 - Heatseeker
	1.0,   // 37 - Flamethrower
	46.2,  // 38 - Minigun
	82.5,  // 39 - Satchel
	0.0,   // 40 - Detonator
	0.33,  // 41 - Spraycan
	0.33,  // 42 - Fire extinguisher
	0.0,   // 43 - Camera
	0.0,   // 44 - Night vision
	0.0,   // 45 - Infrared
	0.0,   // 46 - Parachute
	0.0,   // 47 - Fake pistol
	2.64,  // 48 - Pistol whip (custom)
	9.9,   // 49 - Vehicle
	330.0, // 50 - Helicopter blades
	82.5,  // 51 - Explosion
	1.0,   // 52 - Car park (custom)
	1.0,   // 53 - Drowning
	165.0  // 54 - Splat
};

enum ePedPieceTypes
{
	PED_PIECE_UNKNOWN = 0,

	PED_PIECE_TORSO = 3,
	PED_PIECE_ASS,
	PED_PIECE_LEFT_ARM,
	PED_PIECE_RIGHT_ARM,
	PED_PIECE_LEFT_LEG,
	PED_PIECE_RIGHT_LEG,
	PED_PIECE_HEAD
};

struct CPedDamageResponseInterface
{
	float				fDamageHealth;
	float				fDamageArmor;
	bool				bUnk;
	bool				bForceDeath;
	bool				bDamageCalculated;
	bool				bUnk3;
};

struct CPedDamageResponseCalculatorInterface
{
	ENTITY_TYPE*		pEntity;
	float               fDamage;
	ePedPieceTypes      bodyPart;
	unsigned int        weaponType;
	bool                bSpeak;            // refers to a CPed::Say call (the dying scream?)
};

uintptr_t(*CPedDamageResponseCalculator__ComputeDamageResponse)(CPedDamageResponseCalculatorInterface*, PED_TYPE*, CPedDamageResponseInterface*, bool);
uintptr_t CPedDamageResponseCalculator__ComputeDamageResponse_hook(CPedDamageResponseCalculatorInterface* pPedDamageResponseCalculator, PED_TYPE* pDamagedPed, CPedDamageResponseInterface* pPedDamageResponse, bool bSpeak)
{
	if (pDamagedPed && pPedDamageResponseCalculator) onDamage((PED_TYPE*)*(uintptr_t*)pPedDamageResponseCalculator, (PED_TYPE*)pDamagedPed);
	// Make sure that everything is not null
	if (!pNetGame || !pPedDamageResponseCalculator || !pDamagedPed || !pPedDamageResponse || !pPedDamageResponseCalculator->pEntity) return CPedDamageResponseCalculator__ComputeDamageResponse(pPedDamageResponseCalculator, pDamagedPed, pPedDamageResponse, bSpeak);

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if(!pPlayerPool || !pVehiclePool) return CPedDamageResponseCalculator__ComputeDamageResponse(pPedDamageResponseCalculator, pDamagedPed, pPedDamageResponse, bSpeak);
	
	CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();
	if(!pLocalPlayer) return CPedDamageResponseCalculator__ComputeDamageResponse(pPedDamageResponseCalculator, pDamagedPed, pPedDamageResponse, bSpeak);

	PED_TYPE* pInflictor = (PED_TYPE*)pPedDamageResponseCalculator->pEntity;
	PED_TYPE* pLocalPed = GamePool_FindPlayerPed();
	if(!pLocalPed) return CPedDamageResponseCalculator__ComputeDamageResponse(pPedDamageResponseCalculator, pDamagedPed, pPedDamageResponse, bSpeak);
	
	uint32_t weaponid = pPedDamageResponseCalculator->weaponType;
	uint32_t bodypart = pPedDamageResponseCalculator->bodyPart;
	
	float fDamage = (pPedDamageResponseCalculator->fDamage / 3.0303030303);
	if (fDamage < 1.0) fDamage = m_fWeaponDamagess[weaponid];
	
	if (weaponid < 0)			
		weaponid = 255; //Suicide
	else if (weaponid == 18)
		weaponid = 37; //Flamethower
	else if (weaponid == 35 || weaponid == 16)
		weaponid = 51; //Explosion
			
	PLAYERID damagedid = pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pPedDamageResponseCalculator);
	PLAYERID issuerid = pNetGame->GetPlayerPool()->FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pDamagedPed);
	
	VEHICLEID vehicleId = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE*)pPedDamageResponseCalculator->pEntity);
	if (vehicleId != INVALID_VEHICLE_ID)
	{
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if (pVehicle)
		{
			if (pVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_PLANE)
				fDamage = m_fWeaponDamagess[50];
			
			else
				fDamage = m_fWeaponDamagess[49];
		}
	}

	// self damage like fall damage, drowned, etc
	if(issuerid == INVALID_PLAYER_ID && damagedid == INVALID_PLAYER_ID)
	{
		PLAYERID playerId = pPlayerPool->GetLocalPlayerID();
		pPlayerPool->GetLocalPlayer()->GiveTakeDamage(true, playerId, fDamage, weaponid, bodypart);
	}

	// give player damage
	if(issuerid != INVALID_PLAYER_ID && damagedid == INVALID_PLAYER_ID)
		pPlayerPool->GetLocalPlayer()->GiveTakeDamage(false, issuerid, fDamage, weaponid, bodypart);

	// player take damage
	else if(issuerid == INVALID_PLAYER_ID && damagedid != INVALID_PLAYER_ID)
		pPlayerPool->GetLocalPlayer()->GiveTakeDamage(true, damagedid, fDamage, weaponid, bodypart);

	return CPedDamageResponseCalculator__ComputeDamageResponse(pPedDamageResponseCalculator, pDamagedPed, pPedDamageResponse, bSpeak);
}
/* ========================================================================================================== */

int(*CWeapon__GenerateDamageEvent)(PED_TYPE* victim, ENTITY_TYPE *creator, unsigned int weaponType, int damageFactor, ePedPieceTypes pedPiece, int direction);
int CWeapon__GenerateDamageEvent_hook(PED_TYPE* victim, ENTITY_TYPE* creator, unsigned int weaponType, int damageFactor, ePedPieceTypes pedPiece, int direction)
{
	if (pedPiece != PED_PIECE_HEAD)
	{
		// Disable ComputeDamageAnim when it's not a head!
		makeNOP(SA_ADDR(0x566B30), 2);
		makeNOP(SA_ADDR(0x566B88), 2);
	}

	int result = CWeapon__GenerateDamageEvent(victim, creator, weaponType, damageFactor, pedPiece, direction);
	if (pedPiece != PED_PIECE_HEAD)
	{
		// Restore bytes
		WriteMemory(SA_ADDR(0x566B30), (uintptr_t)"\xC1\xF5\xD8\xF9", 4);
		WriteMemory(SA_ADDR(0x566B88), (uintptr_t)"\xBB\xF5\xFE\xF8", 4);
	}

	return result;
}

int (*CTaskSimpleUseGun__SetPedPosition)(uintptr_t thiz, PED_TYPE* pPed);
int CTaskSimpleUseGun__SetPedPosition_hook(uintptr_t thiz, PED_TYPE* pPed)
{
	if (pPed && pPed->WeaponSlots && pPed->byteCurWeaponSlot < 13)
	{
		WEAPON_SLOT_TYPE curWeaponSlot = pPed->WeaponSlots[pPed->byteCurWeaponSlot];
		if ((curWeaponSlot.dwType == 37) || (curWeaponSlot.dwType == 41) || (curWeaponSlot.dwType == 42)) *(uint8_t*)(thiz + 13) |= 1;
	}

	return CTaskSimpleUseGun__SetPedPosition(thiz, pPed);
}

int (*_RwTextureDestroy)(RwTexture*);
int _RwTextureDestroy_hook(RwTexture* texture)
{
    if(!texture)
    	return 1;

    return _RwTextureDestroy(texture);
}

uintptr_t (*GetMeshPriority)(RpMesh const*);
uintptr_t GetMeshPriority_hook(RpMesh const* rpMesh)
{
    if(rpMesh)
    {
        RpMaterial* rpMeshMat = rpMesh->material;
        if(rpMeshMat)
        {
            if(rpMeshMat->texture)
                if(!rpMeshMat->texture->raster) return 0;
        }
    }

    return GetMeshPriority(rpMesh);
}

void (*CTimer__StartUserPause)();
void CTimer__StartUserPause_hook()
{
	// process pause event
	if (g_pJavaWrapper)
	{
		if (pKeyBoard)
		{
			if (pKeyBoard->IsNewKeyboard())
				pKeyBoard->Close();
		}

		if (g_pJavaWrapper->isGlobalShowSpeedometer)
			g_pJavaWrapper->HideSpeedometer();

		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->HideHUD(false);

		if (g_pJavaWrapper->isGlobalShowDialog)
			g_pJavaWrapper->HideDialogWithoutResetContent();

		if (g_pJavaWrapper->isGlobalShowTab)
			g_pJavaWrapper->HideTab();
	}

	*(uint8_t*)(SA_ADDR(0x8C9BA3)) = 1;

	// LanguageScreen::Russian
	// ((void(*)())(SA_ADDR(0x25D104 + 1)))();
	//*(uint32_t*)(SA_ADDR(0x63E5A4)) = 5;
	// Menu_ApplyLanguage(void)	.text	00269F7C	0000008E	00000028	00000000	R	.	.	.	B	T	.
	//((void(*)())(SA_ADDR(0x00269F7C + 1)))();
}

void (*CTimer__EndUserPause)();
void CTimer__EndUserPause_hook()
{
	// process resume event
	if (g_pJavaWrapper)
	{
		if (g_pJavaWrapper->isGlobalShowSpeedometer)
			g_pJavaWrapper->ShowSpeedometer();

		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->ShowHUD(false);

		if (g_pJavaWrapper->isGlobalShowDialog)
			g_pJavaWrapper->ShowDialogWithOldContent();

		if (g_pJavaWrapper->isGlobalShowTab)
			g_pJavaWrapper->ShowTab(false);
	}

	*(uint8_t*)(SA_ADDR(0x8C9BA3)) = 0;

	// LanguageScreen::English
	// ((void(*)())(SA_ADDR(0x25D20C + 1)))();

	//*(uint32_t*)(SA_ADDR(0x63E5A4)) = 0;

	// Menu_ApplyLanguage(void)	.text	00269F7C	0000008E	00000028	00000000	R	.	.	.	B	T	.
	//((void(*)())(SA_ADDR(0x00269F7C + 1)))();
}

// colModel check null-pointer
uint32_t (*CCollision__ProcessVerticalLine)(float *colLine, float *transform, int colModel, int colPoint, int *maxTouchDistance, char seeThrough, int shootThrough, int storedCollPoly);
uint32_t CCollision__ProcessVerticalLine_hook(float *colLine, float *transform, int colModel, int colPoint, int *maxTouchDistance, char seeThrough, int shootThrough, int storedCollPoly)
{
	if(colModel)
		return CCollision__ProcessVerticalLine(colLine, transform, colModel, colPoint, maxTouchDistance, seeThrough, shootThrough, storedCollPoly);

	return 0;
}

void (*NvUtilInit)(void);
void NvUtilInit_hook(void)
{
	NvUtilInit();

	unProtect(SA_ADDR(0x5D1608));
	*(char**)(SA_ADDR(0x5D1608)) = (char*) OBFUSCATE("/storage/emulated/0/Android/data/criminal.russia/files/");
}

void InstallHooks()
{
	#ifdef USE_SANTROPE_BULLSHIT
		// useless shit...
		PROTECT_CODE_INSTALLHOOKS;
	#endif

	installHook(SA_ADDR(0x2749B0), (uintptr_t) CWidget_hook, (uintptr_t *) &CWidgets);
	installHook(SA_ADDR(0x241D94), (uintptr_t) NvUtilInit_hook, (uintptr_t *) &NvUtilInit);
	installHook(SA_ADDR(0x273338), (uintptr_t) CWidget__Update_hook, (uintptr_t *) &CWidget__Update);
	installHook(SA_ADDR(0x274178), (uintptr_t) CWidget__SetEnabled_hook, (uintptr_t *) &CWidget__SetEnabled);
	
	installHook(SA_ADDR(0x3BF784), (uintptr_t) CTimer__StartUserPause_hook, (uintptr_t *) &CTimer__StartUserPause);
	installHook(SA_ADDR(0x3BF7A0), (uintptr_t) CTimer__EndUserPause_hook, (uintptr_t *) &CTimer__EndUserPause);

	// -- Crash fix
	installHook(SA_ADDR(0x29947C), (uintptr_t) CCollision__ProcessVerticalLine_hook, (uintptr_t *) &CCollision__ProcessVerticalLine);

	// -- Ped damage handler
	installHook(SA_ADDR(0x327528), (uintptr_t) CPedDamageResponseCalculator__ComputeDamageResponse_hook, (uintptr_t *) &CPedDamageResponseCalculator__ComputeDamageResponse);
	installHook(SA_ADDR(0x5669D8), (uintptr_t) CWeapon__GenerateDamageEvent_hook, (uintptr_t *) &CWeapon__GenerateDamageEvent);

    // -- CrossHair fix
	installHook(SA_ADDR(0x5529AC), (uintptr_t) CSprite2d__DrawRect__hook, (uintptr_t *) &CSprite2d__DrawRect);

	// -- Fire extingusher fix
	installHook(SA_ADDR(0x46D6AC), (uintptr_t) CTaskSimpleUseGun__SetPedPosition_hook, (uintptr_t *) &CTaskSimpleUseGun__SetPedPosition);

	installHook(SA_ADDR(0x281398), (uintptr_t) CWidgetRegionLook__Update_hook, (uintptr_t *) &CWidgetRegionLook__Update);
	installHook(SA_ADDR(0x3D7CA8), (uintptr_t) CLoadingScreen_DisplayPCScreen_hook, (uintptr_t *) &CLoadingScreen_DisplayPCScreen);
	installHook(SA_ADDR(0x39AEF4), (uintptr_t) Render2dStuff_hook, (uintptr_t *) &Render2dStuff);
	installHook(SA_ADDR(0x39B098), (uintptr_t) Render2dStuffAfterFade_hook, (uintptr_t *) &Render2dStuffAfterFade);
	installHook(SA_ADDR(0x239D5C), (uintptr_t) TouchEvent_hook, (uintptr_t *) &TouchEvent);
	installHook(SA_ADDR(0x28E83C), (uintptr_t) CStreaming_InitImageList_hook, (uintptr_t *) &CStreaming_InitImageList);
	installHook(SA_ADDR(0x336690), (uintptr_t) CModelInfo_AddPedModel_hook, (uintptr_t *) &CModelInfo_AddPedModel); // hook is dangerous
	installHook(SA_ADDR(0x3DBA88), (uintptr_t) CRadar__GetRadarTraceColor_hook, (uintptr_t *) &CRadar__GetRadarTraceColor); // dangerous
	installHook(SA_ADDR(0x3DAF84), (uintptr_t) CRadar__SetCoordBlip_hook, (uintptr_t *) &CRadar__SetCoordBlip);
	installHook(SA_ADDR(0x3DE9A8), (uintptr_t) CRadar__DrawRadarGangOverlay_hook, (uintptr_t *) &CRadar__DrawRadarGangOverlay);
	installHook(SA_ADDR(0x3C70C0), (uintptr_t) CWorld__ProcessLineOfSight_hook, (uintptr_t *) &CWorld__ProcessLineOfSight);
	installHook(SA_ADDR(0x482E60), (uintptr_t) CTaskComplexEnterCarAsDriver_hook, (uintptr_t *) &CTaskComplexEnterCarAsDriver);
	installHook(SA_ADDR(0x4833CC), (uintptr_t) CTaskComplexLeaveCar_hook, (uintptr_t *) &CTaskComplexLeaveCar);
	//installHook(g_libGTASA + 0x0044A4CC, (uintptr_t)PointGunInDirection_hook, (uintptr_t*)&PointGunInDirection);
	CodeInject(SA_ADDR(0x2D99F4), (uintptr_t)PickupPickUp_hook, 1);
	//installHook(g_libGTASA + 0x00327528, (uintptr_t)ComputeDamageResponse_hooked, (uintptr_t*)(&ComputeDamageResponse));

	installHook(SA_ADDR(0x336268), (uintptr_t) CModelInfo_AddAtomicModel_hook, (uintptr_t *) &CModelInfo_AddAtomicModel);
	installHook(SA_ADDR(0x567964), (uintptr_t) CWeapon_FireInstantHit_hook, (uintptr_t *) &CWeapon_FireInstantHit);
	installHook(SA_ADDR(0x56668C), (uintptr_t) CWeapon__FireSniper_hook, (uintptr_t *) &CWeapon__FireSniper);
	installHook(SA_ADDR(0x336618), (uintptr_t) CModelInfo_AddVehicleModel_hook, (uintptr_t *) &CModelInfo_AddVehicleModel); // dangerous

	installHook(SA_ADDR(0x33DA5C), (uintptr_t) CAnimManager__UncompressAnimation_hook, (uintptr_t *) &CAnimManager__UncompressAnimation);
	installHook(SA_ADDR(0x531118), (uintptr_t) CCustomRoadsignMgr__RenderRoadsignAtomic_hook, (uintptr_t *) &CCustomRoadsignMgr__RenderRoadsignAtomic);
	installHook(SA_ADDR(0x1AECC0), (uintptr_t) RwFrameAddChild_hook, (uintptr_t *) &RwFrameAddChild);
	installHook(SA_ADDR(0x2DFD30), (uintptr_t) CUpsideDownCarCheck__IsCarUpsideDown_hook, (uintptr_t *) &CUpsideDownCarCheck__IsCarUpsideDown);
	installHook(SA_ADDR(0x33AD78), (uintptr_t) CAnimBlendNode__FindKeyFrame_hook, (uintptr_t *) &CAnimBlendNode__FindKeyFrame);

	// Add custom siren to our vehicle
	installHook(SA_ADDR(0x510B08), (uintptr_t) CVehicle__UsesSiren_hook, (uintptr_t *) &CVehicle__UsesSiren);

	// TextDraw render
	installHook(SA_ADDR(0x3D5894), (uintptr_t) CHud__DrawScriptText_hook, (uintptr_t *) &CHud__DrawScriptText);

	//widgets
	installHook(SA_ADDR(0x276510), (uintptr_t) CWidgetButtonEnterCar_Draw_hook, (uintptr_t *) &CWidgetButtonEnterCar_Draw);

	// attached objects
	installHook(SA_ADDR(0x3C1BF8), (uintptr_t) CWorld_ProcessPedsAfterPreRender_hook, (uintptr_t *) &CWorld_ProcessPedsAfterPreRender);

	//remove building
	installHook(SA_ADDR(0x395994), (uintptr_t) CFileLoader__LoadObjectInstance_hook, (uintptr_t *) &CFileLoader__LoadObjectInstance);

	// retexture
	installHook(SA_ADDR(0x391E20), (uintptr_t) CObject__Render_hook, (uintptr_t *) &CObject__Render);
	installHook(SA_ADDR(0x3EF518), (uintptr_t) CMaterialText__Render_hook, (uintptr_t *) &CMaterialText__Render);

	// gettexture fix crash
	installHook(SA_ADDR(0x258910), (uintptr_t) GetTexture_hook, (uintptr_t *) &GetTexture_orig);

	// steal objects fix
	installHook(SA_ADDR(0x3AC114), (uintptr_t) CPlayerInfo__FindObjectToSteal_hook, (uintptr_t *) &CPlayerInfo__FindObjectToSteal);

	// GetFrameFromID fix
	installHook(SA_ADDR(0x335CC0), (uintptr_t) CClumpModelInfo_GetFrameFromId_hook, (uintptr_t *) &CClumpModelInfo_GetFrameFromId);

	// RLEDecompress fix
	installHook(SA_ADDR(0x1BC314), (uintptr_t) RLEDecompress_hook, (uintptr_t *) &RLEDecompress);

	// todo: 3 pools fix crash

	// random crash fix
	installHook(SA_ADDR(0x1A8530), (uintptr_t) RenderQueue__ProcessCommand_hook, (uintptr_t *) &RenderQueue__ProcessCommand);

	// fix
	installHook(SA_ADDR(0x1B9D74), (uintptr_t) _rwFreeListFreeReal_hook, (uintptr_t *) &_rwFreeListFreeReal);

	installHook(SA_ADDR(0x338CBC), (uintptr_t) CVehicleModelInfo__SetEditableMaterials_hook, (uintptr_t *) &CVehicleModelInfo__SetEditableMaterials);
	installHook(SA_ADDR(0x50DEF4), (uintptr_t) CVehicle__ResetAfterRender_hook, (uintptr_t *) &CVehicle__ResetAfterRender);

	installHook(SA_ADDR(0x3986CC), (uintptr_t) CGame__Process_hook, (uintptr_t *) &CGame__Process);

	// -- Render SkyBox
	installHook(SA_ADDR(0x3B1778), (uintptr_t) CRenderer__RenderEverythingBarRoads_hook, (uintptr_t *) &CRenderer__RenderEverythingBarRoads);

	installHook(SA_ADDR(0x4D4A6C), (uintptr_t) CAutomobile__ProcessEntityCollision_hook, (uintptr_t *) &CAutomobile__ProcessEntityCollision);

	installHook(SA_ADDR(0x398334), (uintptr_t) CGame__Shutdown_hook, (uintptr_t *) &CGame__Shutdown);

	WriteMemory(SA_ADDR(0x3DA86C), (uintptr_t)"\x80\xB4\x00\xAF\x1B\x4B\x7B\x44\x1B\x68", 10);

	makeNOP(SA_ADDR(0x3DA876), 3);

	// headlights color, wheel size, wheel align
	installHook(SA_ADDR(0x5466EC), (uintptr_t) CShadows__StoreCarLightShadow_hook, (uintptr_t *) &CShadows__StoreCarLightShadow);
	installHook(SA_ADDR(0x518EC4), (uintptr_t) CVehicle__DoHeadLightBeam_hook, (uintptr_t *) &CVehicle__DoHeadLightBeam);

	installHook(SA_ADDR(0x4E671C), (uintptr_t) CAutomobile__PreRender_hook, (uintptr_t *) &CAutomobile__PreRender);
	installHook(SA_ADDR(0x4DC6E8), (uintptr_t) CAutomobile__UpdateWheelMatrix_hook, (uintptr_t *) &CAutomobile__UpdateWheelMatrix);
	installHook(SA_ADDR(0x3E8D48), (uintptr_t) CMatrix__Rotate_hook, (uintptr_t *) &CMatrix__Rotate);
	installHook(SA_ADDR(0x3E8884), (uintptr_t) CMatrix__SetScale_hook, (uintptr_t *) &CMatrix__SetScale);

	installHook(SA_ADDR(0x46CEF4), (uintptr_t) CTaskSimpleUseGun__RemoveStanceAnims_hook, (uintptr_t *) &CTaskSimpleUseGun__RemoveStanceAnims);

	installHook(SA_ADDR(0x3DA86C), (uintptr_t) CRadar__LimitRadarPoint_hook, (uintptr_t *) &CRadar__LimitRadarPoint); // TO FIX
	installHook(SA_ADDR(0x5529FC), (uintptr_t) CSprite2d__DrawBarChart_hook, (uintptr_t *) &CSprite2d__DrawBarChart);
	installHook(SA_ADDR(0x5353B4), (uintptr_t) CFont__PrintString_hook, (uintptr_t *) &CFont__PrintString);
	installHook(SA_ADDR(0x55265C), (uintptr_t) CSprite2d__Draw_hook, (uintptr_t *) &CSprite2d__Draw);

	installHook(SA_ADDR(0x27D8A8), (uintptr_t) CWidgetPlayerInfo__DrawWanted_hook, (uintptr_t *) &CWidgetPlayerInfo__DrawWanted);

	installHook(SA_ADDR(0x27CE88), (uintptr_t) CWidgetPlayerInfo__DrawWeaponIcon_hook, (uintptr_t *) &CWidgetPlayerInfo__DrawWeaponIcon);
	installHook(SA_ADDR(0x389D74), (uintptr_t) CCam__Process_hook, (uintptr_t *) &CCam__Process);
	// sub_45C88(dword_14A458 + 0x541AC4, sub_2CBF0, &unk_28E378); nop
	//sub_45C88(g_libGTASA + 0x001A8530, sub_2CBDC, &dword_28E374);

	//installHook(SA_ADDR(0x3D6E6C), (uintptr_t) CHud__Draw_hook, (uintptr_t *) &CHud__Draw);
	installHook(SA_ADDR(0x39DC68), (uintptr_t) CPad__CycleCameraModeDownJustDown_hook, (uintptr_t *) &CPad__CycleCameraModeDownJustDown);

	//installHook(SA_ADDR(0x391FE0), (uintptr_t) CEntity__GetIsOnScreen_hook, (uintptr_t *) &CEntity__GetIsOnScreen);
	installHook(SA_ADDR(0x31B164), (uintptr_t) FxEmitterBP_c__Render_hook, (uintptr_t *) &FxEmitterBP_c__Render);
	installHook(SA_ADDR(0x43A17C), (uintptr_t) CPed__ProcessEntityCollision_hook, (uintptr_t *) &CPed__ProcessEntityCollision);
	installHook(SA_ADDR(0x1B1808), (uintptr_t) _RwTextureDestroy_hook, (uintptr_t *) &_RwTextureDestroy);
	installHook(SA_ADDR(0x1E4AE4), (uintptr_t) GetMeshPriority_hook, (uintptr_t *) &GetMeshPriority);
	
	installHook(SA_ADDR(0x4052B8), (uintptr_t) CVehicleModelInfo__SetupCommonData_hook, (uintptr_t *) &CVehicleModelInfo__SetupCommonData);
	installHook(SA_ADDR(0x35BE30), (uintptr_t) CAEVehicleAudioEntity__GetVehicleAudioSettings_hook, (uintptr_t *) &CAEVehicleAudioEntity__GetVehicleAudioSettings);

	HookCPad();
}