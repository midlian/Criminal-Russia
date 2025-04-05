#include "../main.h"
#include "../game/game.h"
#include "../game/crosshair.h"
#include "../game/CRadarRect.h"
#include "netgame.h"

#include "md5.h" //��� RPC
#include <iostream> //for std::cout
#include <string.h> //for std::string
extern CWidgetManager* g_pWidgetManager;
extern CCrossHair *pCrossHair;
#include "../keyboard.h"
extern CKeyBoard *pKeyBoard;
#include "../chatwindow.h"

#include "..//CClientInfo.h"
#include "..//CLocalisation.h"
#include "../util/CJavaWrapper.h"

#include "../str_obfuscator_no_template.hpp"

#define NETGAME_VERSION 4057
#define AUTH_BS "E02262CF28BC542486C558D4BE9EFB716592AFAF8B"
const auto encAuthKey = cryptor::create("E02262CF28BC542486C558D4BE9EFB716592AFAF8B", 42);


extern CGame *pGame;
extern CJavaWrapper *g_pJavaWrapper;
extern CChatWindow *pChatWindow;

int iVehiclePoolProcessFlag = 0;
int iPickupPoolProcessFlag = 0;

void RegisterRPCs(RakClientInterface* pRakClient);
void UnRegisterRPCs(RakClientInterface* pRakClient);
void RegisterScriptRPCs(RakClientInterface* pRakClient);
void UnRegisterScriptRPCs(RakClientInterface* pRakClient);

unsigned char GetPacketID(Packet *p)
{
	if(p == 0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	else
		return (unsigned char) p->data[0];
}

// #define USE_LOCAL_SRV
CNetGame::CNetGame(const char* szHostOrIp, int iPort, const char* szPlayerName, const char* szPass)
{
	//#ifdef USE_LOCAL_SRV
	//	szHostOrIp = "192.168.1.103";
	//	iPort = 7777;
	//#endif

	strcpy(m_szHostName, OBFUSCATE("San Andreas Multiplayer"));
	strncpy(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	m_iPort = iPort;

	m_pPlayerPool = new CPlayerPool();
	m_pPlayerPool->SetLocalPlayerName(szPlayerName);
	
	m_pVehiclePool = new CVehiclePool();
	m_pObjectPool = new CObjectPool();
	m_pPickupPool = new CPickupPool();
	m_pGangZonePool = new CGangZonePool();
	m_pLabelPool = new CText3DLabelsPool();
	m_pTextDrawPool = new CTextDrawPool();
	g_pWidgetManager = new CWidgetManager();
	m_pStreamPool = new CStreamPool();
	m_pActorPool = new CActorPool();
	m_pNotificationPool = new NotificationPool();
	// m_pVoiceNotificationPool = new VoiceNotificationPool();

	m_pRakClient = RakNetworkFactory::GetRakClientInterface();
	RegisterRPCs(m_pRakClient);
	RegisterScriptRPCs(m_pRakClient);
	m_pRakClient->SetPassword(szPass);

	m_dwLastConnectAttempt = GetTickCount();
	m_iGameState = 	GAMESTATE_WAIT_CONNECT;

	m_iSpawnsAvailable = 0;
	m_bHoldTime = true;
	m_byteWorldMinute = 0;
	m_byteWorldTime = 12;
	m_byteWeather =	10;
	m_fGravity = (float)0.008000000;
	m_bUseCJWalk = false;
	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 60.0f;
	m_bZoneNames = false;
	m_bInstagib = false;
	m_iDeathDropMoney = 0;
	m_bNameTagLOS = false;

	for(int i=0; i<100; i++)
		m_dwMapIcons[i] = 0;

	pGame->EnableClock(false);
	pGame->EnableZoneNames(false);

                  g_pJavaWrapper->SetLoadingText(2);

//#ifdef _UALANG
//	if(pChatWindow) pChatWindow->AddDebugMessageNonFormatted("Запущений клієнт");
//#else
//	if(pChatWindow) pChatWindow->AddDebugMessageNonFormatted("Клиент запущен");
//#endif
}
#include "..//voice/CVoiceChatClient.h"
extern CVoiceChatClient* pVoice;
CNetGame::~CNetGame()
{
	if(m_pRakClient) {
		m_pRakClient->Disconnect(0);

		UnRegisterRPCs(m_pRakClient);
		UnRegisterScriptRPCs(m_pRakClient);
	
		RakNetworkFactory::DestroyRakClientInterface(m_pRakClient);
	}
	
	SAFE_DELETE(m_pPlayerPool);
	SAFE_DELETE(m_pVehiclePool);
	SAFE_DELETE(m_pPickupPool);
	SAFE_DELETE(m_pGangZonePool);
	SAFE_DELETE(m_pLabelPool);
	SAFE_DELETE(m_pTextDrawPool);
	SAFE_DELETE(g_pWidgetManager);
	SAFE_DELETE(m_pStreamPool);
	SAFE_DELETE(m_pNotificationPool);
	// SAFE_DELETE(m_pVoiceNotificationPool);
}

bool g_IsVoiceServer();
static bool once = false;
void CNetGame::Process()
{
	// todo: 30 fps fixed rate
	//static uint32_t time = GetTickCount();
	bool bProcess = false;
	UpdateNetwork();
	bProcess = true;
	/*if (GetTickCount() - time >= 1000 / 30)
	{
		UpdateNetwork();
		time = GetTickCount();
		bProcess = true;
	}*/

	// server checkpoints update
	if (m_pPlayerPool)
	{
		if (m_pPlayerPool->GetLocalPlayer())
		{
			if (m_pPlayerPool->GetLocalPlayer()->m_bIsActive && m_pPlayerPool->GetLocalPlayer()->GetPlayerPed()) 
			{
				pGame->UpdateCheckpoints();
			}
		}
	}

	if(m_bHoldTime) {
		pGame->SetWorldTime(m_byteWorldTime, m_byteWorldMinute);
	}

	if(GetGameState() == GAMESTATE_CONNECTED)
	{
		// pool process
		if(m_pPlayerPool && bProcess) m_pPlayerPool->Process();
		if (m_pObjectPool) m_pObjectPool->Process();
		if(m_pVehiclePool && iVehiclePoolProcessFlag > 5)
		{
			m_pVehiclePool->Process();
			iVehiclePoolProcessFlag = 0;
		}
		else
		{
			++iVehiclePoolProcessFlag;
		}

		if(m_pPickupPool && iPickupPoolProcessFlag > 5) 
		{
			m_pPickupPool->Process();
			iPickupPoolProcessFlag = 0;
		}
		else
		{
			//++iPickupPoolProcessFlag;
		}
		if(g_pJavaWrapper && pKeyBoard && m_pTextDrawPool) 
		{
			static uint32_t dwLastKeysUpdateTick = GetTickCount(); 

			bool bResUI = g_pJavaWrapper->IsSomethingUIShitOnScreenActive();
			bool bKeyOn = pKeyBoard->IsOpen();
			bool bTextDrawOn = m_pTextDrawPool->IsSomeoneActiveOnScreen();
			bool bOptional = bResUI || bKeyOn || bTextDrawOn || pGame->IsGamePaused();

			if(GetTickCount() - dwLastKeysUpdateTick >= 60) {
				g_pJavaWrapper->CallLauncherActivityWithSpecialValue(1223, bOptional ? 1 : 0);
				dwLastKeysUpdateTick = GetTickCount();
			} 
		}

		if(m_pNotificationPool) {
			m_pNotificationPool->Process();

			// if(m_pVoiceNotificationPool) {
				// m_pVoiceNotificationPool->Process();
			// }
		}
	}
	else
	{
		CPlayerPed *pPlayer = pGame->FindPlayerPed();
		CCamera *pCamera = pGame->GetCamera();

		if(pPlayer && pCamera)
		{
			if(pPlayer->IsInVehicle())
				pPlayer->RemoveFromVehicleAndPutAt(1093.4f, -2036.5f, 82.7106f);
			else pPlayer->TeleportTo(1093.4f, -2036.5f, 82.7106f);

			pCamera->SetPosition(1093.0f, -2036.0f, 90.0f, 0.0f, 0.0f, 0.0f);
			pCamera->LookAtPoint(384.0f, -1557.0f, 20.0f, 2);

			pGame->SetWorldWeather(m_byteWeather);
			pGame->DisplayWidgets(false);
		}
	}

	if(GetGameState() == GAMESTATE_WAIT_CONNECT && (GetTickCount() - m_dwLastConnectAttempt) > 3000)
	{
		if(pChatWindow)
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTING));

		if(m_pRakClient) {
			m_pRakClient->Connect(m_szHostOrIp, m_iPort, 0, 0, 15);
		}
		
		m_dwLastConnectAttempt = GetTickCount();

		SetGameState(GAMESTATE_CONNECTING);
	}

	if (pVoice && !once)
	{
	 	if (g_IsVoiceServer()) {
	 		pVoice->StartProcessing();
	 	}

	 	once = true;
	}
}

void CNetGame::UpdateNetwork()
{
	Packet* pkt = nullptr;
	unsigned char packetIdentifier;

	while(pkt = m_pRakClient->Receive())
	{
		packetIdentifier = GetPacketID(pkt);

		switch(packetIdentifier)
		{
			case ID_AUTH_KEY:
			Log(OBFUSCATE("Incoming packet: ID_AUTH_KEY"));
			Packet_AuthKey(pkt);
			break;

			case ID_CONNECTION_BANNED:
			case ID_CONNECTION_ATTEMPT_FAILED:
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTION_ATTEMPT_FAILED));
                                                      g_pJavaWrapper->SetLoadingText(5);
			SetGameState(GAMESTATE_WAIT_CONNECT);
			break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::FULL_SERVER));
                                                      g_pJavaWrapper->SetLoadingText(3);
			SetGameState(GAMESTATE_WAIT_CONNECT);
			break;

			case ID_DISCONNECTION_NOTIFICATION:
			Packet_DisconnectionNotification(pkt);
			break;

			case ID_CONNECTION_LOST:
			Packet_ConnectionLost(pkt);
			break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);
			break;

			case ID_FAILED_INITIALIZE_ENCRIPTION:
			pChatWindow->AddDebugMessage(OBFUSCATE("Failed to initialize encryption."));
			break;

			case ID_INVALID_PASSWORD:
			pChatWindow->AddDebugMessage(OBFUSCATE("Wrong server password."));
			m_pRakClient->Disconnect(0);
			break;

			case ID_PLAYER_SYNC:
			Packet_PlayerSync(pkt);
			break;

			case ID_VEHICLE_SYNC:
			Packet_VehicleSync(pkt);
			break;

			case ID_PASSENGER_SYNC:
			Packet_PassengerSync(pkt);
			break;

			case ID_MARKERS_SYNC:
			Packet_MarkersSync(pkt);
			break;

			case ID_AIM_SYNC:
			Packet_AimSync(pkt);
			break;

			case ID_BULLET_SYNC:
			Packet_BulletSync(pkt);
			break;

			case ID_TRAILER_SYNC:
			Packet_TrailerSync(pkt);
			break;

			case ID_CUSTOM_RPC:
			Packet_CustomRPC(pkt);
			break;
		}

		m_pRakClient->DeallocatePacket(pkt);
	}
}

void CNetGame::Packet_TrailerSync(Packet* p)
{
	CRemotePlayer* pPlayer;
	RakNet::BitStream bsSpectatorSync((unsigned char*)p->data, p->length, false);

	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	BYTE bytePacketID = 0;
	BYTE bytePlayerID = 0;

	TRAILER_SYNC_DATA trSync;

	bsSpectatorSync.Read(bytePacketID);
	bsSpectatorSync.Read(bytePlayerID);
	bsSpectatorSync.Read((char*)& trSync, sizeof(TRAILER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if (pPlayer)
		pPlayer->StoreTrailerFullSyncData(&trSync);
}

#define CUSTOM_RPC_TOGGLE_HUD_ELEMENT   0x1

#define RPC_STREAM_CREATE				0x2
#define RPC_STREAM_POS					0x3
#define RPC_STREAM_DESTROY				0x4
#define RPC_STREAM_INDIVIDUAL			0x5
#define RPC_STREAM_VOLUME				0x6
#define RPC_STREAM_ISENABLED			0x7
#define RPC_OPEN_LINK					0x8
#define RPC_TIMEOUT_CHAT				0x9
#define RPC_CUSTOM_COMPONENT 			0x10
#define RPC_CUSTOM_HANDLING 			0x11
#define RPC_CUSTOM_ADD_PED				0x12
#define RPC_CUSTOM_VISUALS				0x13
#define RPC_CUSTOM_HANDLING_DEFAULTS	0x14
#define RPC_OPEN_SETTINGS				0x15
#define RPC_CUSTOM_AIM 					0x30
#define RPC_CUSTOM_CASH 				0x31
#define RPC_SPEED_MENU					0x32

#define RPC_SATIETY						0x39
#define RPC_HUD_STATUS					0x40 

#define RPC_GPS_VIEW					0x35
#define RPC_GREENZONE_VIEW				0x36

#define RPC_CUSTOM_ACTOR_PUT_IN_VEH		0x20
#define RPC_CUSTOM_ACTOR_REMOVE_VEH		0x21
#define RPC_CUSTOM_ACTOR_ADD_ADDITIONAL	0x22

#define RPC_SPAWN_SELECT 				0x23
#define RPC_SET_SKYBOX 					0x24
#define RPC_SET_RADARLINE				0x25

#define RPC_CHECK_CASH					0x26
// new
#define RPC_MUTE_VOICE 					0x65	
#define RPC_VOLUME_VOICE 				0x66	

#define RPC_CREATENOTIFY 		0x86
#define RPC_REMOVENOTIFY 		0x87

#include "../game/CCustomPlateManager.h"
#include "../util/CJavaWrapper.h"

#include "../graphics/CSkyBox.h"

void CNetGame::Packet_CustomRPC(Packet* p)
{
	
	RakNet::BitStream bs((unsigned char*)p->data, p->length, false);
	uint8_t packetID;
	uint32_t rpcID;

	bs.Read(packetID);
	bs.Read(rpcID);

	//pChatWindow->AddDebugMessage("p %d rpc %d", packetID, rpcID);

	switch (rpcID)
	{
		case RPC_MUTE_VOICE: {
			uint32_t playerId = 0;
			uint8_t muteState = 0;
			bs.Read(playerId);
			bs.Read(muteState);
			if(pVoice) {
				if(muteState) {
					pVoice->MutePlayer(playerId);
				} else {
					pVoice->UnMutePlayer(playerId);
				}
			}
			
			break;
		}

		case RPC_VOLUME_VOICE: {
			uint32_t playerId = 0;
			uint8_t volume = 0;
			bs.Read(playerId);
			bs.Read(volume);
			if(pVoice) {
				if(playerId == this->GetPlayerPool()->GetLocalPlayerID()) {
					pVoice->SetVolume(volume);
				}
				else 
				{
					pVoice->SetVolumePlayer(playerId, volume);
				}
			}
			break;
		}

		case RPC_OPEN_SETTINGS:
		{
			g_pJavaWrapper->ShowClientSettings();
			break;
		}

		case RPC_REMOVENOTIFY: {
			uint16_t notifyId;
			bs.Read(notifyId);
			this->GetNotificationPool()->Remove(notifyId);
			break;
		}

		case RPC_CREATENOTIFY: 
		{
			uint8_t type;
			uint32_t duration;
			float screenX, screenY;
			uint8_t textLen;
			char textBuffer[256+1] = { '\0' };
			float fTextFontSize;
			uint32_t textColor;
			uint8_t bHasOutline;
			uint32_t textOutlineColor;
			int textOutlineOffset;
			uint8_t bHasIcon;
			float iconScaleX, iconScaleY;
			uint8_t dbLen;
			char dbName[256+1] = { '\0' };
			uint8_t texLen;
			char texName[256+1] = { '\0' };
			uint8_t bRecreate;

			memset(textBuffer, 0, ARRAY_SIZE(textBuffer));
			memset(dbName, 0, ARRAY_SIZE(dbName));
			memset(texName, 0, ARRAY_SIZE(texName));

			bs.Read(type);
			bs.Read(duration);
			bs.Read(screenX);
			bs.Read(screenY);

			bs.Read(textLen);
			if (textLen >= sizeof(textBuffer) - 1) {
				return;
			}

			bs.Read(&textBuffer[0], textLen);

			bs.Read(fTextFontSize);
			bs.Read(textColor);
			bs.Read(bHasOutline);
			bs.Read(textOutlineColor);
			bs.Read(textOutlineOffset);
			bs.Read(bHasIcon);
			bs.Read(iconScaleX);
			bs.Read(iconScaleY);

			bs.Read(dbLen);
			if (dbLen >= sizeof(dbName) - 1) {
				return;
			}

			bs.Read(&dbName[0], dbLen);

			bs.Read(texLen);
			if (texLen >= sizeof(texName) - 1) {
				return;
			}

			bs.Read(&texName[0], texLen);

			bs.Read(bRecreate);

			uintptr_t icon_texture = LoadTextureFromDB(dbName, texName);
			if(!icon_texture) break;

			this->GetNotificationPool()->Add(type, duration, ImVec2(screenX, screenY), textBuffer, fTextFontSize, textColor, bHasOutline, textOutlineColor, textOutlineOffset, bHasIcon, ImVec2(iconScaleX, iconScaleY), icon_texture, bRecreate);
			break;
		}
		
		case RPC_CHECK_CASH:
		{
			uint8_t bLen, bLen1;
			uint32_t bVersion;
			char szText[30];
			char szText1[30];

			memset(szText, 0, 30);
			memset(szText1, 0, 30);
			
			bs.Read(bLen);
			if (bLen >= sizeof(szText) - 1)
				return;

			bs.Read(&szText[0], bLen);
			
			bs.Read(bLen1);
			if (bLen1 >= sizeof(szText1) - 1)
				return;

			bs.Read(&szText1[0], bLen1);
			
			bs.Read(bVersion);
			
			RwTexture* pCashTexture = nullptr;
			pCashTexture = (RwTexture*)LoadTextureFromDB(szText1, szText);
			
			int iVersion;
			if (pCashTexture) iVersion = bVersion;
			else iVersion = 0;
				
			RakNet::BitStream bsParams;
			
			bsParams.Write(iVersion);
			m_pRakClient->RPC(&RPC_CustomHash, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
			break;
		}

		case RPC_SPAWN_SELECT:
		{
			uint8_t home;
			bs.Read(home);

                                                      uint8_t fraction;
                                                      bs.Read(fraction);

                                                      g_pJavaWrapper->ShowSpawnSelector(home, fraction);                                                   
			break;
		}
		
		case RPC_SET_SKYBOX:
		{
			uint8_t bLen;
			char szText[30];

			memset(szText, 0, 30);
			
			bs.Read(bLen);
			if (bLen >= sizeof(szText) - 1)
				return;

			bs.Read(&szText[0], bLen);
			
			CSkyBox::SetTexture(szText);
			break;
		}
		case RPC_CUSTOM_CASH:
		{
			uint8_t bLen;
			char szText[30];

			memset(szText, 0, 30);
			
			bs.Read(bLen);
			if (bLen >= sizeof(szText) - 1)
				return;

			bs.Read(&szText[0], bLen);
			jstring jCashSend = (*g_pJavaWrapper->GetEnv()).NewStringUTF(szText);
			g_pJavaWrapper->SetCashMoney(jCashSend);
			break;
		}
		case RPC_SATIETY:
		{
			uint32_t value = 0;
			bs.Read(value);
			g_pJavaWrapper->SetHudSatiety(value);
			break;
		}
		case RPC_HUD_STATUS:
		{
			uint32_t value = 0;
			bs.Read(value); 
			if(value == 1) {
				g_pJavaWrapper->CallLauncherActivity(1300);
				pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, true);
				pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, true);
			}
			if(value == 2) {
				g_pJavaWrapper->CallLauncherActivity(1301);
				pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, false);
				pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, false);
			}
			break;
		}
		case RPC_SPEED_MENU:
		{
			uint32_t stype = 0;
			uint32_t svalue = 0;

			bs.Read(stype);
			bs.Read(svalue);

	
			g_pJavaWrapper->SetSpeedMenu(stype,svalue);
			break;
		}
		case RPC_GPS_VIEW:
		{
			uint32_t value = 0;
			bs.Read(value);
			if(value == 1) g_pJavaWrapper->CallLauncherActivity(1230);
			if(value == 2) g_pJavaWrapper->CallLauncherActivity(1231);
			break;
		}
		case RPC_GREENZONE_VIEW:
		{
			uint32_t value = 0;
			bs.Read(value);
			if(value == 1) g_pJavaWrapper->CallLauncherActivity(1232);
			if(value == 2) g_pJavaWrapper->CallLauncherActivity(1233);
			break;
		}
		case RPC_SET_RADARLINE:
		{
			uint8_t bLen, bType;
			char szText[30];

			memset(szText, 0, 30);
			
			bs.Read(bType);
			bs.Read(bLen);
			if (bLen >= sizeof(szText) - 1)
				return;

			bs.Read(&szText[0], bLen);
			
			CRadarRect::ChangeTextures(bType, szText);
			break;
		}

		case RPC_CUSTOM_HANDLING_DEFAULTS:
		{
			uint16_t vehId;
			bs.Read(vehId);

			if (GetVehiclePool())
			{
				CVehicle* pVeh = GetVehiclePool()->GetAt(vehId);
				if (pVeh)
					pVeh->ResetVehicleHandling();
			}

			break;
		}

		case RPC_CUSTOM_VISUALS:
		{
			uint16_t vehId;
			uint8_t bLightsColor[3];
			int8_t bWheelAlignX;
			int8_t bWheelAlignY;
			int16_t sWheelOffsetXX;
			int16_t sWheelOffsetXY;
			uint8_t bToner[3];
			uint8_t bVinyls[2];
			uint16_t fWheelWidth;

			uint8_t bPlateType, bLen;

			char szText[30];
			char szRegion[10];

			memset(szText, 0, 30);
			memset(szRegion, 0, 10);

			bs.Read(vehId);
			bs.Read(bLightsColor[0]);
			bs.Read(bLightsColor[1]);
			bs.Read(bLightsColor[2]);
			bs.Read(fWheelWidth);
			bs.Read(bWheelAlignX);
			bs.Read(bWheelAlignY);
			bs.Read(sWheelOffsetXX);
			bs.Read(sWheelOffsetXY);
			bs.Read(bToner[0]);
			bs.Read(bToner[1]);
			bs.Read(bToner[2]);
			bs.Read(bVinyls[0]);
			bs.Read(bVinyls[1]);
			bs.Read(bPlateType);

			if (bPlateType == 1 || bPlateType == 4 || bPlateType == 5)
			{
				bs.Read(bLen);
				if (bLen >= sizeof(szText) - 1)
					return;

				bs.Read(&szText[0], bLen);

				bs.Read(bLen);
				if (bLen >= sizeof(szRegion) - 1)
					return;

				bs.Read(&szRegion[0], bLen);
			}
			else if (bPlateType == 2 || bPlateType == 3)
			{
				bs.Read(bLen);
				if (bLen >= sizeof(szText) - 1)
					return;

				bs.Read(&szText[0], bLen);
			}

			uint8_t bShadowColor[3];
			uint8_t bShadowSizeX, bShadowSizeY;
			char szName[32];
			
			memset(szName, 0, sizeof(szName));

			bs.Read(bShadowColor[0]);
			bs.Read(bShadowColor[1]);
			bs.Read(bShadowColor[2]);
			bs.Read(bShadowSizeX);
			bs.Read(bShadowSizeY);
			bs.Read(bLen);

			if (bLen >= sizeof(szName) + 1)
				return;

			bs.Read(szName, bLen);

			if (GetVehiclePool())
			{
				CVehicle* pVeh = GetVehiclePool()->GetAt(vehId);
				if (pVeh)
				{
					pVeh->SetCustomShadow(bShadowColor[0], bShadowColor[1], bShadowColor[2], (float)bShadowSizeX / 10.0f, (float)bShadowSizeY / 10.0f, szName);

					if (bLightsColor[0] != 0xFF || bLightsColor[1] != 0xFF || bLightsColor[2] != 0xFF)
						pVeh->SetHeadlightsColor(bLightsColor[0], bLightsColor[1], bLightsColor[2]);

					if (fWheelWidth)
						pVeh->SetWheelWidth((float)fWheelWidth / 100.0f);

					if (bWheelAlignX)
						pVeh->SetWheelAlignment(0, (float)bWheelAlignX);

					if (bWheelAlignY)
						pVeh->SetWheelAlignment(1, (float)bWheelAlignY);

					if (sWheelOffsetXX)
					{
						auto fValueX = (float)((float)sWheelOffsetXX / 100.0f);
						pVeh->SetWheelOffset(0, fValueX);
						//pVeh->ProcessWheelsOffset();
					}

					if (sWheelOffsetXY)
					{
						auto fValueX = (float)((float)sWheelOffsetXY / 100.0f);
						pVeh->SetWheelOffset(1, fValueX);
						//pVeh->ProcessWheelsOffset();
					}

					pVeh->ApplyToner(1, bToner[0]);
					pVeh->ApplyToner(2, bToner[1]);
					pVeh->ApplyToner(3, bToner[2]);
					pVeh->ApplyVinyls(bVinyls[0], bVinyls[1]);

					//pChatWindow->AddDebugMessage("%d %d %d %d %d", bToner[0], bToner[1], bToner[2], bVinyls[0], bVinyls[1]);
					if (bPlateType)
					{
						// pChatWindow->AddDebugMessage("%s %s", szText, szRegion);
						CCustomPlateManager::PushPlate(vehId, (uint32_t)bPlateType, szText, szRegion);
					}
				}
			}

			break;
		}

		case RPC_CUSTOM_ACTOR_PUT_IN_VEH:
		{
			uint16_t actorId;
			VEHICLEID vehicleId;
			uint8_t seat;

			bs.Read(actorId);
			bs.Read(vehicleId);
			bs.Read(seat);

#ifdef _CDEBUG
			pChatWindow->AddDebugMessage(OBFUSCATE("Put actor %d to %d in %d"), actorId, vehicleId, seat);
#endif

			CActorPool* pActorPool = GetActorPool();
			if (pActorPool && GetVehiclePool())
			{
				if (pActorPool->GetAt(actorId) && GetVehiclePool()->GetAt(vehicleId))
				{
					int iCarID = GetVehiclePool()->FindGtaIDFromID((int)vehicleId);

					pActorPool->GetAt(actorId)->PutDirectlyInVehicle(iCarID, (int)seat);
				}
			}
			break;
		}

		case RPC_CUSTOM_ACTOR_REMOVE_VEH:
		{
			uint16_t actorId;
			bs.Read(actorId);

			CActorPool* pActorPool = GetActorPool();
			if (pActorPool)
			{
				if (pActorPool->GetAt(actorId))
					pActorPool->GetAt(actorId)->RemoveFromVehicle();
			}
			break;
		}

		case RPC_CUSTOM_ACTOR_ADD_ADDITIONAL:
		{
			uint16_t actorId;
			VEHICLEID vehicleId;
			uint8_t seat;

			bs.Read(actorId);
			bs.Read(vehicleId);
			bs.Read(seat);

			if (GetActorPool() && GetVehiclePool())
			{
				if (GetActorPool()->GetAt(actorId) && GetVehiclePool()->GetAt(vehicleId))
				{
					int iCarID = GetVehiclePool()->FindGtaIDFromID((int)vehicleId);

					GetActorPool()->GetAt(actorId)->PutDirectlyInVehicle(iCarID, (int)seat);
				}
			}
			break;
		}

		case RPC_CUSTOM_ADD_PED:
		{
			uint16_t player;
			uint8_t moveAnim;
			bs.Read(player);
			bs.Read(moveAnim);
			
			if (m_pPlayerPool)
			{
				if (player == m_pPlayerPool->GetLocalPlayerID())
				{
					if (m_pPlayerPool->GetLocalPlayer())
					{
						if (m_pPlayerPool->GetLocalPlayer()->GetPlayerPed())
							m_pPlayerPool->GetLocalPlayer()->GetPlayerPed()->SetMoveAnim((int)moveAnim);
					}
				}

				if (m_pPlayerPool->GetAt(player))
				{
					if (m_pPlayerPool->GetAt(player)->GetPlayerPed())
						m_pPlayerPool->GetAt(player)->GetPlayerPed()->SetMoveAnim((int)moveAnim);
				}
			}
			break;
		}

		case RPC_CUSTOM_HANDLING:
		{
			uint16_t veh;
			uint8_t value;
			bs.Read(veh);
			bs.Read(value);
			std::vector<SHandlingData> comps;

			for (uint8_t i = 0; i < value; i++)
			{
				uint8_t id;
				float fvalue;

				bs.Read(id);
				bs.Read(fvalue);

				comps.push_back(SHandlingData(id, fvalue, 0));
				Log(OBFUSCATE("Pushed %d %f"), id, fvalue);
			}

			if (m_pVehiclePool)
			{
				if (m_pVehiclePool->GetAt(veh))
					m_pVehiclePool->GetAt(veh)->SetHandlingData(comps);
			}
			break;
		}

		case RPC_CUSTOM_COMPONENT:
		{
			uint16_t veh, extra_comp;
			uint8_t comp;
			bs.Read(veh);
			CVehicle* pVehicle = nullptr;

			if (m_pVehiclePool)
				pVehicle = m_pVehiclePool->GetAt(veh);

			if (!pVehicle)
				return;

			for (int i = 0; i < E_CUSTOM_COMPONENTS::ccMax; i++)
			{
				if (i == E_CUSTOM_COMPONENTS::ccExtra)
				{
					bs.Read(extra_comp);
					pVehicle->SetComponentVisible(i, (uint16_t)extra_comp);
				}
				else
				{
					bs.Read(comp);
					pVehicle->SetComponentVisible(i, (uint16_t)comp);
				}
			}
			break;
		}

		case CUSTOM_RPC_TOGGLE_HUD_ELEMENT:
		{
			uint32_t hud, toggle;
			bs.Read(hud);
			bs.Read(toggle);
			if(hud == 8) {				
				if(toggle == true) {
				                  // -- SHOW
					g_pJavaWrapper->ShowHUD(false);
					pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, true);

				} else {
					// -- HIDE
					g_pJavaWrapper->HideHUD(false);
					pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, false);
				}
			}
			else {
				pGame->ToggleHUDElement(hud, toggle);
				pGame->HandleChangedHUDStatus();
			}
	
			break;
		}
		case RPC_STREAM_CREATE:
		{
			char str[255];

			uint8_t len;
			uint16_t id, vw, interior;

			VECTOR pos;

			float fDistance;
			bs.Read(id);
			bs.Read(pos.X);
			bs.Read(pos.Y);
			bs.Read(pos.Z);
			bs.Read(fDistance);
			bs.Read(vw);
			bs.Read(interior);
			bs.Read(len);
			bs.Read(&str[0], len);
			str[len] = 0;

			GetStreamPool()->AddStream(id, &pos, vw, interior, fDistance, (const char*)&str[0]);
			break;
		}

		case RPC_STREAM_INDIVIDUAL:
		{
			char str[255];
			uint8_t len;

			bs.Read(len);
			bs.Read(&str[0], len);
			str[len] = 0;

			GetStreamPool()->PlayIndividualStream(&str[0]);
			break;
		}

		case RPC_STREAM_POS:
		{
			break;
		}

		case RPC_STREAM_DESTROY:
		{
			uint32_t id;
			bs.Read(id);

			m_pStreamPool->DeleteStreamByID(id);
			break;
		}

		case RPC_STREAM_VOLUME:
		{
			uint16_t id;
			float fVolume;
			bs.Read(id);
			bs.Read(fVolume);

			m_pStreamPool->SetStreamVolume(id, fVolume);
			break;
		}

		case RPC_STREAM_ISENABLED:
		{
			uint32_t isEnabled;
			bs.Read(isEnabled);
			if (isEnabled)
				m_pStreamPool->Activate();
			else if (isEnabled == 0)
				m_pStreamPool->Deactivate();

			break;
		}
		case RPC_OPEN_LINK:
		{
			uint16_t len;
			bs.Read(len);

			if (len >= 0xFA)
				break;

			char link[0xFF];
			memset(&link[0], 0, 0xFF);
			bs.Read((char*)& link, len);

			AND_OpenLink(&link[0]);

			break;
		}

		case RPC_TIMEOUT_CHAT:
		{
			uint32_t timeoutStart = 0;
			uint32_t timeoutEnd = 0;

			bs.Read(timeoutStart);
			bs.Read(timeoutEnd);

			if (pChatWindow)
				pChatWindow->SetChatDissappearTimeout(timeoutStart, timeoutEnd);

			break;
		}
		
		case RPC_CUSTOM_AIM:
		{
			uint8_t szLen;
			char szName[32];
			
			memset(szName, 0, sizeof(szName));
			
			bs.Read(szLen);

			if (szLen >= sizeof(szName) + 1)
				return;

			bs.Read(szName, szLen);
			pCrossHair->ChangeAim(szName);
			break;
		}
	}
}

void CNetGame::ResetVehiclePool()
{
	Log(OBFUSCATE("ResetVehiclePool"));
	SAFE_DELETE(m_pVehiclePool);

	m_pVehiclePool = new CVehiclePool();
}

void CNetGame::ResetObjectPool()
{
	Log(OBFUSCATE("ResetObjectPool"));
	SAFE_DELETE(m_pObjectPool);

	m_pObjectPool = new CObjectPool();
}

void CNetGame::ResetNotificationPool()
{
	Log(OBFUSCATE("ResetNotificationPool"));
	SAFE_DELETE(m_pNotificationPool);

	m_pNotificationPool = new NotificationPool();
}
/*
void CNetGame::ResetVoiceNotificationPool()
{
	Log(OBFUSCATE("ResetVoiceNotificationPool"));
	SAFE_DELETE(m_pVoiceNotificationPool);

	m_pVoiceNotificationPool = new VoiceNotificationPool();
}*/

void CNetGame::ResetPickupPool()
{
	Log(OBFUSCATE("ResetPickupPool"));
	SAFE_DELETE(m_pPickupPool);

	m_pPickupPool = new CPickupPool();
}

void CNetGame::ResetGangZonePool()
{
	Log(OBFUSCATE("ResetGangZonePool"));
	SAFE_DELETE(m_pGangZonePool);

	m_pGangZonePool = new CGangZonePool();
}

void CNetGame::ResetLabelPool()
{
	Log(OBFUSCATE("ResetLabelPool"));
	SAFE_DELETE(m_pLabelPool);

	m_pLabelPool = new CText3DLabelsPool();
}

void CNetGame::ResetActorPool()
{
	Log(OBFUSCATE("ResetActorPool"));
	SAFE_DELETE(m_pActorPool);

	m_pActorPool = new CActorPool();
}

void CNetGame::ResetTextDrawPool()
{
	Log(OBFUSCATE("ResetTextDrawPool"));
	SAFE_DELETE(m_pTextDrawPool);

	m_pTextDrawPool = new CTextDrawPool();
}

extern int RemoveModelIDs[1200];
extern VECTOR RemovePos[1200];
extern float RemoveRad[1200];
extern int iTotalRemovedObjects;

extern CNetGame *pNetGame;

void CNetGame::ShutDownForGameRestart()
{
	iTotalRemovedObjects = 0;
	for (int & RemoveModelID : RemoveModelIDs)
		RemoveModelID = -1;

	for(PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		CRemotePlayer* pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
		{
			//pPlayer->SetTeam(NO_TEAM);
			//pPlayer->ResetAllSyncAttributes();
		}
	}

	CLocalPlayer *pLocalPlayer = m_pPlayerPool->GetLocalPlayer();
	if(pLocalPlayer)
	{
		pLocalPlayer->ResetAllSyncAttributes();
		pLocalPlayer->ToggleSpectating(false);
	}

	m_iGameState = GAMESTATE_RESTARTING;

	//m_pPlayerPool->DeactivateAll();
	m_pPlayerPool->Process();

	ResetVehiclePool();
	ResetObjectPool();
	ResetPickupPool();
	ResetGangZonePool();
	ResetLabelPool();
	ResetTextDrawPool();
	ResetActorPool();
	ResetNotificationPool();
	// ResetVoiceNotificationPool();

	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 60.0f;
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather = 1;
	m_bHoldTime = true;
	m_bNameTagLOS = true;
	m_bUseCJWalk = false;
	m_fGravity = 0.008f;
	m_iDeathDropMoney = 0;

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed)
	{
		pPlayerPed->SetInterior(0);
		//pPlayerPed->SetDead();
		pPlayerPed->SetArmour(0.0f);
	}

	pGame->ToggleCJWalk(false);
	pGame->ResetLocalMoney();
	pGame->EnableZoneNames(false);
	m_bZoneNames = false;
	GameResetRadarColors();
	pGame->SetGravity(m_fGravity);
	pGame->EnableClock(false);
}

void CNetGame::SendChatMessage(const char* szMsg)
{
	if (!m_pRakClient || GetGameState() != GAMESTATE_CONNECTED) return;

	RakNet::BitStream bsSend;
	uint8_t byteTextLen = strlen(szMsg);

	bsSend.Write(byteTextLen);
	bsSend.Write(szMsg, byteTextLen);

	m_pRakClient->RPC(&RPC_Chat,&bsSend,HIGH_PRIORITY,RELIABLE,0,false, UNASSIGNED_NETWORK_ID, NULL);
}


void CNetGame::SendHash(const char* sHash)
{
	int RPC_CustomHashMd5 = 221;
	RakNet::BitStream bsHash;
	int iStrlen = strlen(OBFUSCATE("ddddd"));
	bsHash.Write(iStrlen);
	bsHash.Write(OBFUSCATE("ddddd"), iStrlen);
	pNetGame->GetRakClient()->RPC(&RPC_Chat, &bsHash, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);	
}

void CNetGame::SendChatCommand(const char* szCommand)
{
	if (!m_pRakClient || GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsParams;
	int iStrlen = strlen(szCommand);

	bsParams.Write(iStrlen);
	bsParams.Write(szCommand, iStrlen);
	m_pRakClient->RPC(&RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SendDialogResponse(uint16_t wDialogID, uint8_t byteButtonID, uint16_t wListBoxItem, char* szInput)
{
	if(!g_pJavaWrapper) {
		return;
	}

	if (g_pJavaWrapper->isGlobalShowHUD)
		g_pJavaWrapper->ShowHUD(true);

	pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, true);

	if (g_pJavaWrapper->isGlobalShowSpeedometer)
		g_pJavaWrapper->ShowSpeedometer();

	uint8_t respLen = strlen(szInput);

	RakNet::BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(byteButtonID);
	bsSend.Write(wListBoxItem);
	bsSend.Write(respLen);
	bsSend.Write(szInput, respLen);

	m_pRakClient->RPC(&RPC_DialogResponse, &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SetMapIcon(uint8_t byteIndex, float fX, float fY, float fZ, uint8_t byteIcon, int iColor, int style)
{
	if(!ARRAY_VALIDATE(m_dwMapIcons, byteIndex)) {
		return;
	}

	if(m_dwMapIcons[byteIndex]) {
		DisableMapIcon(byteIndex);
	}

	m_dwMapIcons[byteIndex] = pGame->CreateRadarMarkerIcon(byteIcon, fX, fY, fZ, iColor, style);
}

void CNetGame::DisableMapIcon(uint8_t byteIndex)
{
	if(!ARRAY_VALIDATE(m_dwMapIcons, byteIndex)) {
		return;
	}

	ScriptCommand(&disable_marker, m_dwMapIcons[byteIndex]);
	m_dwMapIcons[byteIndex] = 0;
}

void CNetGame::UpdatePlayerScoresAndPings()
{
	static uint32_t dwLastUpdateTick = 0;

	if ((GetTickCount() - dwLastUpdateTick) >= 3000)
	{
		dwLastUpdateTick = GetTickCount();
		RakNet::BitStream bsParams;
		if(m_pRakClient) m_pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void gen_auth_key(char buf[260], char* auth_in);
void CNetGame::Packet_AuthKey(Packet* pkt)
{
	RakNet::BitStream bsAuth((unsigned char *)pkt->data, pkt->length, false);

	uint8_t byteAuthLen;
	char szAuth[260];

	bsAuth.IgnoreBits(8);
	bsAuth.Read(byteAuthLen);
	bsAuth.Read(szAuth, byteAuthLen);
	szAuth[byteAuthLen] = '\0';

	char szAuthKey[260];
	gen_auth_key(szAuthKey, szAuth);

	RakNet::BitStream bsKey;
	auto byteAuthKeyLen = (uint8_t)strlen(szAuthKey);

	bsKey.Write((uint8_t)ID_AUTH_KEY);
	bsKey.Write((uint8_t)byteAuthKeyLen);
	bsKey.Write(szAuthKey, byteAuthKeyLen);
	m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);

	Log(OBFUSCATE("[AUTH] %s -> %s"), szAuth, szAuthKey);
}

void CNetGame::Packet_DisconnectionNotification(Packet* pkt)
{
	if(pChatWindow) {
		pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::SERVER_CLOSED_CONNECTION));
	}

	if(m_pRakClient) {
		m_pRakClient->Disconnect(2000);
	}

	if(pVoice) {
	 	pVoice->FullDisconnect();
	}
}

void CNetGame::Packet_ConnectionLost(Packet* pkt)
{
	if(m_pRakClient) {
		m_pRakClient->Disconnect(0);
	}

	if(pChatWindow) {
		pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTION_LOST));
	}

	if(pVoice) {
	 	pVoice->Disconnect();
                  }

	ShutDownForGameRestart();

	for(PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		CRemotePlayer *pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer) {
			m_pPlayerPool->Delete(playerId, 0);
		}
	}

	SetGameState(GAMESTATE_WAIT_CONNECT);
}

#include "..//CServerManager.h"
bool g_isValidSum(int a)
{
	for (const auto & g_sEncryptedAddresse : g_sEncryptedAddresses)
		if (g_sEncryptedAddresse.getSum() == a) return true;

	return false;
}
void WriteVerified1();
void CNetGame::Packet_ConnectionSucceeded(Packet* pkt)
{
	if(pChatWindow) pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTED));
                  g_pJavaWrapper->SetLoadingText(6);
	SetGameState(GAMESTATE_AWAIT_JOIN);

	RakNet::BitStream bsSuccAuth((unsigned char *)pkt->data, pkt->length, false);
	PLAYERID MyPlayerID;
	unsigned int uiChallenge;

	bsSuccAuth.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
	bsSuccAuth.IgnoreBits(32); // binaryAddress
	bsSuccAuth.IgnoreBits(16); // port
	bsSuccAuth.Read(MyPlayerID);
	bsSuccAuth.Read(uiChallenge);

	char ip[0x7F];
	strncpy(ip, m_szHostOrIp, sizeof(ip));

	/*std::vector<std::string> strings;
	std::istringstream f((const char*)&ip[0]);
	std::string s;
	int sum = 0;
	while (getline(f, s, '.'))
	 	sum += std::atoi(s.c_str());

	if (g_isValidSum(sum))
	 	WriteVerified1();*/

	Log("AUTH_BS: %s", AUTH_BS);

	m_pPlayerPool->SetLocalPlayerID(MyPlayerID);

	int iVersion = NETGAME_VERSION;
	char byteMod = 0x01;
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;

	char byteAuthBSLen = (char)strlen(AUTH_BS);
	char byteNameLen = (char)strlen(m_pPlayerPool->GetLocalPlayerName());
	char byteClientverLen = (char)strlen(SAMP_VERSION);

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(m_pPlayerPool->GetLocalPlayerName(), byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write(AUTH_BS, byteAuthBSLen);
	bsSend.Write(byteClientverLen);
	bsSend.Write(SAMP_VERSION, byteClientverLen);

	CClientInfo::WriteClientInfoToBitStream(bsSend);

	m_pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::Packet_PlayerSync(Packet* pkt)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPlayerSync((unsigned char *)pkt->data, pkt->length, false);
	ONFOOT_SYNC_DATA ofSync;
	uint8_t bytePacketID=0;
	PLAYERID playerId;

	bool bHasLR,bHasUD;
	bool bHasVehicleSurfingInfo;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&ofSync, 0, sizeof(ONFOOT_SYNC_DATA));

	bsPlayerSync.Read(bytePacketID);
	bsPlayerSync.Read(playerId);

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if(bHasLR) bsPlayerSync.Read(ofSync.lrAnalog);

	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if(bHasUD) bsPlayerSync.Read(ofSync.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(ofSync.wKeys);

	// VECTOR POS
	bsPlayerSync.Read((char*)&ofSync.vecPos,sizeof(VECTOR));

	// QUATERNION
	float tw, tx, ty, tz;
	bsPlayerSync.ReadNormQuat(tw, tx, ty, tz);
	ofSync.quat.w = tw;
	ofSync.quat.x = tx;
	ofSync.quat.y = ty;
	ofSync.quat.z = tz;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	uint8_t byteHealthArmour;
	uint8_t byteArmTemp=0,byteHlTemp=0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) ofSync.byteArmour = 100;
	else if(byteArmTemp == 0) ofSync.byteArmour = 0;
	else ofSync.byteArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) ofSync.byteHealth = 100;
	else if(byteHlTemp == 0) ofSync.byteHealth = 0;
	else ofSync.byteHealth = byteHlTemp * 7;

	// CURRENT WEAPON
    bsPlayerSync.Read(ofSync.byteCurrentWeapon);
    // SPECIAL ACTION
    bsPlayerSync.Read(ofSync.byteSpecialAction);

    // READ MOVESPEED VECTORS
    bsPlayerSync.ReadVector(tx, ty, tz);
    ofSync.vecMoveSpeed.X = tx;
    ofSync.vecMoveSpeed.Y = ty;
    ofSync.vecMoveSpeed.Z = tz;

    bsPlayerSync.Read(bHasVehicleSurfingInfo);
    if (bHasVehicleSurfingInfo)
    {
        bsPlayerSync.Read(ofSync.wSurfInfo);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.X);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.Y);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.Z);
    }
    else
        ofSync.wSurfInfo = INVALID_VEHICLE_ID;

	bool bHasAnimInfo;
	bsPlayerSync.Read(bHasAnimInfo);

	if (bHasAnimInfo)
		bsPlayerSync.Read(ofSync.dwAnimation);
	else ofSync.dwAnimation = 0b10000000000000000000000000000000;

	uint8_t key = 0;

    if(m_pPlayerPool)
    {
    	pPlayer = m_pPlayerPool->GetAt(playerId);
    	if(pPlayer)
    		pPlayer->StoreOnFootFullSyncData(&ofSync, 0, key);
    }
}

void CNetGame::Packet_VehicleSync(Packet* pkt)
{
	CRemotePlayer *pPlayer;
	RakNet::BitStream bsSync((unsigned char *)pkt->data, pkt->length, false);
	uint8_t bytePacketID = 0;
	PLAYERID playerId;
	INCAR_SYNC_DATA icSync;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

	bsSync.Read(bytePacketID);
	bsSync.Read(playerId);
	bsSync.Read(icSync.VehicleID);

	// keys
	bsSync.Read(icSync.lrAnalog);
	bsSync.Read(icSync.udAnalog);
	bsSync.Read(icSync.wKeys);

	// quaternion
	bsSync.ReadNormQuat(icSync.quat.w, icSync.quat.x, icSync.quat.y, icSync.quat.z);

	// position
	bsSync.Read((char*)&icSync.vecPos, sizeof(VECTOR));

	// speed
	bsSync.ReadVector(icSync.vecMoveSpeed.X, icSync.vecMoveSpeed.Y, icSync.vecMoveSpeed.Z);

	// vehicle health
	uint16_t wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	icSync.fCarHealth = (float)wTempVehicleHealth;

	// health/armour
	uint8_t byteHealthArmour;
	uint8_t byteArmTemp=0, byteHlTemp=0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) icSync.bytePlayerArmour = 100;
	else if(byteArmTemp == 0) icSync.bytePlayerArmour = 0;
	else icSync.bytePlayerArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) icSync.bytePlayerHealth = 100;
	else if(byteHlTemp == 0) icSync.bytePlayerHealth = 0;
	else icSync.bytePlayerHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	uint8_t byteTempWeapon;
	bsSync.Read(byteTempWeapon);
	icSync.byteCurrentWeapon ^= (byteTempWeapon ^ icSync.byteCurrentWeapon) & 0x3F;

	bool bCheck;

	// siren
	bsSync.Read(bCheck);
	if(bCheck) icSync.byteSirenOn = 1;
	// landinggear
	bsSync.Read(bCheck);
	if(bCheck) icSync.byteLandingGearState = 1;
	// train speed
	bsSync.Read(bCheck);
	if(bCheck) bsSync.Read(icSync.fTrainSpeed);
	// triler id
	bsSync.Read(bCheck);
	if(bCheck) bsSync.Read(icSync.TrailerID);

	if(m_pPlayerPool)
	{
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
			pPlayer->StoreInCarFullSyncData(&icSync, 0);
	}
}

void CNetGame::Packet_PassengerSync(Packet* pkt)
{
	CRemotePlayer *pPlayer;
	uint8_t bytePacketID;
	PLAYERID playerId;
	PASSENGER_SYNC_DATA psSync;

	if(GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsPassengerSync((unsigned char *)pkt->data, pkt->length, false);
	bsPassengerSync.Read(bytePacketID);
	bsPassengerSync.Read(playerId);
	bsPassengerSync.Read((char*)&psSync, sizeof(PASSENGER_SYNC_DATA));

	if(m_pPlayerPool)
	{
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
			pPlayer->StorePassengerFullSyncData(&psSync);
	}
}

void CNetGame::Packet_MarkersSync(Packet *pkt)
{
	CRemotePlayer *pPlayer;
	int			iNumberOfPlayers = 0;
	PLAYERID	playerId;
	short		sPos[3];
	bool		bIsPlayerActive;
	uint8_t 	unk0 = 0;

	if(GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsMarkersSync((unsigned char *)pkt->data, pkt->length, false);
	bsMarkersSync.Read(unk0);
	bsMarkersSync.Read(iNumberOfPlayers);

	if(iNumberOfPlayers)
	{
		for(int i=0; i<iNumberOfPlayers; i++)
		{
			bsMarkersSync.Read(playerId);
			bsMarkersSync.ReadCompressed(bIsPlayerActive);

			if(bIsPlayerActive)
			{
				bsMarkersSync.Read(sPos[0]);
				bsMarkersSync.Read(sPos[1]);
				bsMarkersSync.Read(sPos[2]);
			}

			if(playerId < MAX_PLAYERS && m_pPlayerPool->GetSlotState(playerId))
			{
				pPlayer = m_pPlayerPool->GetAt(playerId);
				if(pPlayer)
				{
					if(bIsPlayerActive)
						pPlayer->ShowGlobalMarker(sPos[0], sPos[1], sPos[2]);
					else pPlayer->HideGlobalMarker();
				}
			}
		}
	}
}

void CNetGame::Packet_BulletSync(Packet* pkt)
{
	uint8_t bytePacketID;
	uint16_t PlayerID;
	BULLET_SYNC_DATA btSync;
	RakNet::BitStream  bsBulletSync((unsigned char*)pkt->data, pkt->length, false);

	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	bsBulletSync.Read(bytePacketID);
	bsBulletSync.Read(PlayerID);
	bsBulletSync.Read((char*)&btSync, sizeof(BULLET_SYNC_DATA));

	CRemotePlayer* pRemotePlayer = m_pPlayerPool->GetAt(PlayerID);
	if (pRemotePlayer)
		pRemotePlayer->StoreBulletSyncData(&btSync);
}

void CNetGame::Packet_AimSync(Packet * p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsAimSync((unsigned char*)p->data, p->length, false);
	AIM_SYNC_DATA aimSync;
	uint8_t bytePacketID = 0;
	uint16_t bytePlayerID = 0;

	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	bsAimSync.Read(bytePacketID);
	bsAimSync.Read(bytePlayerID);
	bsAimSync.Read((char*)&aimSync, sizeof(AIM_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if (pPlayer)
		pPlayer->UpdateAimFromSyncData(&aimSync);
}
