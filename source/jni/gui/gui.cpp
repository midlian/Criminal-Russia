#include "../main.h"
#include "gui.h"
#include "CBinder.h"
#include "../game/game.h"
#include "../game/crosshair.h"
#include "../net/netgame.h"
#include "../game/RW/RenderWare.h"
#include "../chatwindow.h"
#include "../playertags.h"
#include "../dialog.h"
#include "../keyboard.h"
#include "../CSettings.h"
#include "../scoreboard.h"
#include "../KillList.h"
#include "../voice/CVoiceChatClient.h"
#include "../CDebugInfo.h"
#include "../GButton.h"
#include "../Notification.h"
#include "../util/armhook.h"
#include "../util/CJavaWrapper.h"

static bool mTextureButtons = true;
static bool mALT_CTRLStuff= true;

extern CVoiceChatClient* pVoice;
extern CScoreBoard* pScoreBoard;
extern CChatWindow *pChatWindow;
extern CPlayerTags *pPlayerTags;
extern CDialogWindow *pDialogWindow;
extern CSettings *pSettings;
extern CKeyBoard *pKeyBoard;
extern CNetGame *pNetGame;
extern KillList *pKillList;
extern CCrossHair *pCrossHair;
extern CGame* pGame;
extern CGButton *pGButton;

void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_Init();
void ImGui_ImplRenderWare_NewFrame();
void ImGui_ImplRenderWare_ShutDown();

#define MULT_X	0.00052083333f	// 1/1920
#define MULT_Y	0.00092592592f 	// 1/1080

CGUI::CGUI()
{
	Log(OBFUSCATE("Initializing GUI.."));

	m_bMouseDown = 0;
	m_vTouchPos = ImVec2(-1, -1);
	m_bNextClear = false;
	m_bNeedClearMousePos = false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui_ImplRenderWare_Init();

	// SCALE
	m_vecScale.x = io.DisplaySize.x * MULT_X;
	m_vecScale.y = io.DisplaySize.y * MULT_Y;

	// FONT SIZE
	m_fFontSize = ScaleY( pSettings->GetReadOnly().fFontSize );

	Log(OBFUSCATE("GUI | Scale factor: %f, %f Font size: %f"), m_vecScale.x, m_vecScale.y, m_fFontSize);

	// STYLES
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 3.0f;
	style.ScrollbarSize = ScaleY(55.0f);
	style.WindowBorderSize = 0.0f;
	style.FrameBorderSize = 2.0f;
	ImGui::StyleColorsDark();

	// DOWNLOAD FONT
	char path[0xFF];

//#ifdef _RULANG
	sprintf(path, OBFUSCATE("%sSAMP/fonts/%s"), g_pszStorage, pSettings->GetReadOnly().szFont);
//#else
//	sprintf(path, OBFUSCATE("%sSAMP/fonts/arial_ua.ttf"), g_pszStorage);
//#endif
	
	// cp1251
	static const ImWchar ranges[] =
	{
		0x0020, 0x0080,
		0x00A0, 0x00C0,
		0x0400, 0x0460,
		0x0490, 0x04A0,
		0x2010, 0x2040,
		0x20A0, 0x20B0,
		0x2110, 0x2130,
		0
	};

	Log(OBFUSCATE("GUI | Loading font: %s"), pSettings->GetReadOnly().szFont);
	m_pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
	Log(OBFUSCATE("GUI | ImFont pointer = 0x%X"), m_pFont);
                  // ..

	m_pFontGTAWeap = LoadFont(OBFUSCATE("gtaweap3.ttf"), 0);
	style.WindowRounding = 0.0f;

	m_pSplashTexture = nullptr;
	m_pSplashTexture = (RwTexture*)LoadTextureFromDB(OBFUSCATE("txd"), OBFUSCATE("splash_icon"));

	CRadarRect::LoadTextures();

	m_bKeysStatus = false;    
	
                  // NEW BUTTONS
	if(mTextureButtons) {
                                    Log("Inject texture buttons..");

		b_alt = nullptr;
		b_alt = (RwTexture*)LoadTextureFromDB("samp", "ebtn_alt");
    
		b_l = nullptr;
		b_l = (RwTexture*)LoadTextureFromDB("samp", "ebtn_left");
    
		b_r = nullptr;
		b_r = (RwTexture*)LoadTextureFromDB("samp", "ebtn_right");
    
		b_h = nullptr;
		b_h = (RwTexture*)LoadTextureFromDB("samp", "ebtn_h");
    
		b_n = nullptr;
		b_n = (RwTexture*)LoadTextureFromDB("samp", "ebtn_n");
    
		b_y = nullptr;
		b_y = (RwTexture*)LoadTextureFromDB("samp", "ebtn_y");
    
		b_ctrl = nullptr;
		b_ctrl = (RwTexture*) LoadTextureFromDB("samp", "ebtn_ctrl");
    	} else {
                                    Log("Inject imgui buttons..");
                  } 

	this->m_bMemDebugOn = false;
                  this->m_bCarryStuff = false;
}

CGUI::~CGUI()
{
	ImGui_ImplRenderWare_ShutDown();
	ImGui::DestroyContext();
}

bool g_bVoiceEnabled = true;
bool g_bVoiceHidden = true;

bool g_IsVoiceServer()
{
	return g_bVoiceEnabled;
}

void toggleVoiceStuff() {
	g_bVoiceHidden ^= true;
}

bool isVoiceHidden() {
	return g_bVoiceHidden;
}

extern float g_fMicrophoneButtonPosX;
extern float g_fMicrophoneButtonPosY;

extern uint32_t g_uiLastTickVoice;

void CGUI::PreProcessInput()
{
	ImGuiIO& io = ImGui::GetIO();

	io.MousePos = m_vTouchPos;
	io.MouseDown[0] = m_bMouseDown;

	if (!m_bNeedClearMousePos && m_bNextClear)
		m_bNextClear = false;

	if (m_bNeedClearMousePos && m_bNextClear)
	{
		io.MousePos = ImVec2(-1, -1);
		m_bNextClear = true;
	}
}

void CGUI::PostProcessInput()
{
	ImGuiIO& io = ImGui::GetIO();

	if (m_bNeedClearMousePos && io.MouseDown[0])
		return;

	if (m_bNeedClearMousePos && !io.MouseDown[0])
	{
		io.MousePos = ImVec2(-1, -1);
		m_bNextClear = true;
	}
}

float EntityGetDistanceFromCamera(ENTITY_TYPE* m_pEntity)
{
	if(!m_pEntity || m_pEntity->vtable == SA_ADDR(0x5C7358))
		return 100000.0f;
	
	float tmpX = ( m_pEntity->mat->pos.X - *(float*)(SA_ADDR(0x8B1134)));
	float tmpY = ( m_pEntity->mat->pos.Y - *(float*)(SA_ADDR(0x8B1138)));
	float tmpZ = ( m_pEntity->mat->pos.Z - *(float*)(SA_ADDR(0x8B113C)));

	return sqrt(tmpX*tmpX + tmpY*tmpY + tmpZ*tmpZ);
}

#include <algorithm>
#include <string>
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
    	}

    	return str;
}

void CGUI::DrawTextureInfo(uintptr_t pEntity) {
	ENTITY_TYPE* sEntity = (ENTITY_TYPE*)pEntity;
	if(!sEntity->mat) {
		return;
	}

	uint16_t usEntityID = *(uint16_t*)(pEntity + 34);
	uintptr_t* pTexturesInfo = (uintptr_t*)(g_libGTASA + 0x6706E4);
	uintptr_t pTextureInfo = (uintptr_t)pTexturesInfo[usEntityID];

	if(pTextureInfo) 
	{
		char szTextureBufferInfo[0xFF] = { '\0' };

		float dff_size = (float)((float)((float)(pTextureInfo << 11) * 0.00097656) * 0.00097656);
		float tex_size = (float)((float)((float)pTextureInfo * 0.00097656) * 0.00097656);

		sprintf(szTextureBufferInfo, "DFF: %f MB\nTextures: %f MB\nID: %d\n \nt.me/weikton", dff_size, tex_size, usEntityID);
		std::string s(szTextureBufferInfo);
		std::string from("4095");
		std::string to("4");
		std::string repl = ReplaceAll(s, from, to);
		strcpy(szTextureBufferInfo, repl.c_str());

		VECTOR TagPos;
		TagPos.X = sEntity->mat->pos.X;
		TagPos.Y = sEntity->mat->pos.Y;
		TagPos.Z = sEntity->mat->pos.Z;
		TagPos.Z += 0.25f + (EntityGetDistanceFromCamera(sEntity) * 0.0475f);
	
		VECTOR Out;

		// CSprite::CalcScreenCoors
		((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))(&TagPos, &Out, 0, 0, 0, 0);
	
		if (Out.Z < 1.0f) {
			return;
		}
	
		ImVec2 pos = ImVec2(Out.X, Out.Y);
		RenderText(pos, 0xFFFFFFFF, true, szTextureBufferInfo);
	}
}

// extern Notification* notifies[10];

void CGUI::Render()
{
	if (pSettings->GetReadOnly().iNewHud)
		*(uint8_t*)(SA_ADDR(0x7165E8)) = false;

	PreProcessInput();

	ProcessPushedTextdraws();
	if (pChatWindow)
		pChatWindow->ProcessPushedCommands();

	ImGui_ImplRenderWare_NewFrame();
	ImGui::NewFrame();

//#ifdef _UALANG
//                  ImVec2 verpos = ImVec2(ScaleX(10), ScaleY(39));
//	RenderText(verpos, ImColor(0xFFFF00FF), true, "v2.2 (UA)"); 
//#else
//                  ImVec2 verpos = ImVec2(ScaleX(10), ScaleY(39));
//	RenderText(verpos, ImColor(0x0000FFFF), true, "v2.2 (RU)"); 
//#endif

	static bool bShowTexDebugger = false;
	if(this->m_bMemDebugOn) {
		
		ImGui::Begin("Debug");

		uintptr_t ms_memoryAvailable = *(uint32_t *)(g_libGTASA + 0x5DE734);
 		uintptr_t ms_memoryUsed = *(uint32_t *)(g_libGTASA + 0x67067C);
 		
 		uintptr_t storedTexels = (double)*(unsigned int *)(g_libGTASA + 0x61B8C0);
 		uintptr_t GetMaxStorage = ((int (*)(void))(g_libGTASA + 0x1BD8CD))();

 		if(pGame->FindPlayerPed() && pGame->FindPlayerPed()->m_pEntity && pGame->FindPlayerPed()->m_pEntity->mat) {
 			ImGui::Text("Position: %.2f %.2f %.2f", pGame->FindPlayerPed()->m_pEntity->mat->pos.X, pGame->FindPlayerPed()->m_pEntity->mat->pos.Y, pGame->FindPlayerPed()->m_pEntity->mat->pos.Z);
 		}
 		
 		ImGui::Text("Memory limit: (free)%d/(used)%d MB", ms_memoryUsed >> 20, ms_memoryAvailable >> 20);
 		ImGui::Text("Texture memory: %d/%d MB", (unsigned int)(storedTexels * 3.3) >> 20, (unsigned int)((double)GetMaxStorage * 3.3) >> 20);
 		ImGui::Text("Models requested: %d", *(uint32_t*)(g_libGTASA + 0x670680)); 

 		if(ImGui::Button("Enable/disable object info", ImVec2(ImGui::CalcTextSize("Enable/disable object info").x + 64, 64))) 
 		{
 			bShowTexDebugger ^= true;
 		}
		ImGui::End();
	}

	if(bShowTexDebugger) {
		uint32_t ms_nNoOfVisibleEntities = *(uint32_t *)(g_libGTASA + 0x8C162C);
		if (!ms_nNoOfVisibleEntities) {
			return;
		}
		
		uintptr_t* ms_aVisibleEntityPtrs = (uintptr_t*)(g_libGTASA + 0x8C0680);
		
		do {
			uintptr_t m_pEntityPointer = (uintptr_t)ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities];
			if(m_pEntityPointer) {
				DrawTextureInfo(m_pEntityPointer);
			}
			--ms_nNoOfVisibleEntities;
		} while(ms_nNoOfVisibleEntities);
	}
	
	// notifies
	if(pNetGame && pNetGame->GetNotificationPool()) {
		pNetGame->GetNotificationPool()->Render();

		/*if(pNetGame->GetVoiceNotificationPool()) {
			pNetGame->GetVoiceNotificationPool()->Render();
		}*/
	}

	if (pCrossHair)
		pCrossHair->Render();

	if (pKeyBoard)
		pKeyBoard->ProcessInputCommands();

	if (pPlayerTags) pPlayerTags->Render();
	
	if(pNetGame && pNetGame->GetLabelPool())
		pNetGame->GetLabelPool()->Draw();

                  if(pKillList)
                                    pKillList->Render();

	if (pNetGame)
		pNetGame->GetVehiclePool()->UpdateSpeed();

	if (pChatWindow)
		pChatWindow->Render();

	if (pScoreBoard)
		pScoreBoard->Draw();

	if (pKeyBoard)
		pKeyBoard->Render();

	if (pDialogWindow)
		pDialogWindow->Render();

	if (pNetGame && !pDialogWindow->m_bIsActive && pGame->IsToggledHUDElement(HUD_ELEMENT_BUTTONS))
	{
                                    if(mTextureButtons) {
			ImGuiIO& io = ImGui::GetIO();

			ImGui::GetStyle().ButtonTextAlign = ImVec2(0.5f, 0.5f);
			ImGui::GetStyle().WindowPadding = ImVec2(8, 8);
			ImGui::GetStyle().FrameBorderSize = 0.0f;

			m_fButWidth = ImGui::CalcTextSize("QWERTY|").x;
			m_fButHeight = (ImGui::CalcTextSize("QWER").x) + 0.3;

			ImVec2 vecButSize = ImVec2(ImGui::GetFontSize() * 3.5, ImGui::GetFontSize() * 2.5);
			ImGui::SetNextWindowPos(ImVec2(2.0f, io.DisplaySize.y / 2.8 - vecButSize.x / 2));

			ImGui::Begin("__extra_keyboard", nullptr, ImGuiWindowFlags_NoTitleBar 
				| ImGuiWindowFlags_NoBackground 
				| ImGuiWindowFlags_NoMove 
				| ImGuiWindowFlags_NoResize 
				| ImGuiWindowFlags_NoScrollbar 
				| ImGuiWindowFlags_NoSavedSettings 
				| ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
          
			if(ImGui::ImageButton(m_bKeysStatus ? (ImTextureID)b_l->raster : (ImTextureID)b_r->raster, ImVec2(m_fButWidth, m_fButHeight)))
			{
				if (m_bKeysStatus)
					m_bKeysStatus = false;
				else
					m_bKeysStatus = true;
			}

	                 	 	if(!mALT_CTRLStuff) {
				ImGui::SameLine();
				if(ImGui::ImageButton((ImTextureID)b_alt->raster, ImVec2(m_fButWidth, m_fButHeight)))
				{
						CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
						if (pPlayerPool)
						{
							CLocalPlayer* pLocalPlayer;
							if (!pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
								LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
							else
								LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
						}
				}
			}

			ImGui::SameLine();
			CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
			if (pPlayerPool)
			{
				if (pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
				{
						if(ImGui::ImageButton((ImTextureID)b_ctrl->raster, ImVec2(m_fButWidth, m_fButHeight)))
						{
							// LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION] = true;
                                                                                                                              g_pJavaWrapper->ShowSpeedMenu();
						}
				} else if(mALT_CTRLStuff) {
                                                                                                            ImGui::SameLine();
						if(ImGui::ImageButton((ImTextureID)b_alt->raster, ImVec2(m_fButWidth, m_fButHeight)))
						{
							CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
                                                                                                                              if (pPlayerPool)
				                                                      {
                                                                                                                                                  CLocalPlayer* pLocalPlayer;
                                                                                                                                                  if (!pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
                                                                                                                                                                      LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
                                                                                                                                                  else
						                                                          LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
                                                                                                                              }
						}
				           }
			}
		
			if (m_bKeysStatus)
			{
				ImGui::SameLine();
				if(ImGui::ImageButton((ImTextureID)b_y->raster, ImVec2(m_fButWidth, m_fButHeight)))
						LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;

				ImGui::SameLine();
				      if(ImGui::ImageButton((ImTextureID)b_n->raster, ImVec2(m_fButWidth, m_fButHeight)))
						LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;

				ImGui::SameLine();
			   	   if(ImGui::ImageButton((ImTextureID)b_h->raster, ImVec2(m_fButWidth, m_fButHeight)))
						LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;

				//ImGui::SameLine();
			  	//    if(ImGui::ImageButton((ImTextureID)b_f->raster, ImVec2(m_fButWidth, m_fButHeight)))
				//	LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
			}
			ImGui::End();
                                    } else {

			ImGuiIO& io = ImGui::GetIO();
			ImVec2 vecButSize = ImVec2(ImGui::GetFontSize() * 3.5, ImGui::GetFontSize() * 2.5);
			ImGui::SetNextWindowPos(ImVec2(2.0f, io.DisplaySize.y / 2.6 - vecButSize.x / 2));
			ImGui::Begin("###keys", nullptr,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_AlwaysAutoResize);

			if (ImGui::Button(m_bKeysStatus ? "<<" : ">>", vecButSize))
			{
				if (m_bKeysStatus)
					m_bKeysStatus = false;
				else
					m_bKeysStatus = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("ALT", vecButSize))
			{
				CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
				if (pPlayerPool)
				{
					CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();
		
					if(pLocalPlayer) {
						if(!pLocalPlayer->GetPlayerPed()->IsInVehicle()) {
							LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
						} else {
							LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
						}	
					}
				}
			}
			ImGui::SameLine();

			CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
			if (pVehiclePool)
			{
				VEHICLEID ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
				if (ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID))
				{
					CVehicle* pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
					if (pVehicle)
					{
						if (pVehicle->GetDistanceFromLocalPlayerPed() < 5.0f)
						{
							CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
							if (pPlayerPool)
							{
								CLocalPlayer* pLocalPlayer;
								if (pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
								{
									if (ImGui::Button("CTRL", vecButSize))
										LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION] = true;
								}
								ImGui::SameLine();
							}
						}
					}
				}
			}
			if (m_bKeysStatus)
			{
				ImGui::SameLine();
				if (ImGui::Button("Y", vecButSize)) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("N", vecButSize)) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("H", vecButSize)) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
				}
			}
			ImGui::End();
                                    }

	}

	if (pNetGame)
	{
		//if (pVoice && !isVoiceHidden() && g_IsVoiceServer())
		if (pVoice && g_IsVoiceServer())
		{
			if (pVoice->IsRecording() && GetTickCount() - g_uiLastTickVoice >= 20000)
			{
				char buf[64];
				sprintf(&buf[0], "%d", (int)((30000 - (GetTickCount() - g_uiLastTickVoice)) / 1000) + 1);
				ImVec2 test(ScaleX(pSettings->GetReadOnly().fButtonMicrophoneX + pSettings->GetReadOnly().fButtonMicrophoneSize / 2.0f) - ImGui::CalcTextSize(&buf[0]).x / 2.0f, ScaleY(g_fMicrophoneButtonPosY) - GetFontSize() * 2.6f);
				//RenderText(test, 0xFF0000FF, true, &buf[0]);
			}

			ImVec2 centre(ScaleX(1880.0f), ScaleY(35.0f));
			if (pVoice->IsDisconnected())
				ImGui::GetBackgroundDrawList()->AddCircleFilled(centre, 10.0f, ImColor(0.8f, 0.0f, 0.0f));

			if (pVoice->GetNetworkState() == VOICECHAT_CONNECTING || pVoice->GetNetworkState() == VOICECHAT_WAIT_CONNECT)
				ImGui::GetBackgroundDrawList()->AddCircleFilled(centre, 10.0f, ImColor(1.0f, 1.0f, 0.0f));

			if (pVoice->GetNetworkState() == VOICECHAT_CONNECTED)
				ImGui::GetBackgroundDrawList()->AddCircleFilled(centre, 10.0f, ImColor(0.0f, 0.8f, 0.0f));
		}
	}

	CDebugInfo::Draw();
	CBinder::Render();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());

	PostProcessInput();
}

bool CGUI::OnTouchEvent(int type, bool multi, int x, int y)
{
	if(!pKeyBoard->OnTouchEvent(type, multi, x, y))
		return false;

	if (!pScoreBoard->OnTouchEvent(type, multi, x, y))
		return false;

	bool bFalse = true;
	if (pNetGame)
	{
		if (pNetGame->GetTextDrawPool()->OnTouchEvent(type, multi, x, y))
		{
			if (!pChatWindow->OnTouchEvent(type, multi, x, y))
				return false;
		}
		else bFalse = false;
	}

	switch(type)
	{
		case TOUCH_PUSH:
		{
			m_vTouchPos = ImVec2(x, y);
			m_bMouseDown = true;
			m_bNeedClearMousePos = false;
			break;
		}

		case TOUCH_POP:
		{
			m_bMouseDown = false;
			m_bNeedClearMousePos = true;
			break;
		}

		case TOUCH_MOVE:
		{
			m_bNeedClearMousePos = false;
			m_vTouchPos = ImVec2(x, y);
			break;
		}
	}

	if (!bFalse)
		return false;

	return true;
}

void CGUI::RenderVersion()
{

}

void CGUI::ProcessPushedTextdraws()
{
	BUFFERED_COMMAND_TEXTDRAW* pCmd = nullptr;
	while (pCmd = m_BufferedCommandTextdraws.ReadLock())
	{
		RakNet::BitStream bs;
		bs.Write(pCmd->textdrawId);
		pNetGame->GetRakClient()->RPC(&RPC_ClickTextDraw, &bs, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, 0);
		m_BufferedCommandTextdraws.ReadUnlock();
	}
}

void CGUI::RenderRakNetStatistics()
{
		//StatisticsToString(rss, message, 0);

		/*ImGui::GetOverlayDrawList()->AddText(
			ImVec2(ScaleX(10), ScaleY(400)),
			ImColor(IM_COL32_BLACK), message);*/
}

extern uint32_t g_uiBorderedText;
void CGUI::RenderTextForChatWindow(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->GetReadOnly().iFontOutline;

	auto colOutline = ImColor(IM_COL32_BLACK);
	auto colDef = ImColor(col);

	colOutline.Value.w = colDef.Value.w;

	if (bOutline)
	{
		if (g_uiBorderedText)
		{
			posCur.x -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, colOutline, text_begin, text_end);
			posCur.x += iOffset;
			// right 
			posCur.x += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, colOutline, text_begin, text_end);
			posCur.x -= iOffset;
			// above
			posCur.y -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, colOutline, text_begin, text_end);
			posCur.y += iOffset;
			// below
			posCur.y += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, colOutline, text_begin, text_end);
			posCur.y -= iOffset;
		}
		else
		{
			ImColor co(0.0f, 0.0f, 0.0f, 0.4f);
			if (colOutline.Value.w <= 0.4)
				co.Value.w = colOutline.Value.w;

			ImVec2 b(posCur.x + ImGui::CalcTextSize(text_begin, text_end).x, posCur.y + GetFontSize());
			ImGui::GetBackgroundDrawList()->AddRectFilled(posCur, b, co);
		}
	}

	ImGui::GetBackgroundDrawList()->AddText(posCur, col, text_begin, text_end);
}

void CGUI::PushToBufferedQueueTextDrawPressed(uint16_t textdrawId)
{
	BUFFERED_COMMAND_TEXTDRAW* pCmd = m_BufferedCommandTextdraws.WriteLock();

	pCmd->textdrawId = textdrawId;

	m_BufferedCommandTextdraws.WriteUnlock();
}

void CGUI::RenderText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->GetReadOnly().iFontOutline;

	if (bOutline)
	{
		if (g_uiBorderedText)
		{
			posCur.x -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.x += iOffset;
			// right 
			posCur.x += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.x -= iOffset;
			// above
			posCur.y -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.y += iOffset;
			// below
			posCur.y += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.y -= iOffset;
		}
		else
		{
			ImVec2 b(posCur.x + ImGui::CalcTextSize(text_begin, text_end).x, posCur.y + GetFontSize());
			if (m_pSplashTexture)
			{
				ImColor co(1.0f, 1.0f, 1.0f, 0.4f);
				ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)m_pSplashTexture->raster, posCur, b, ImVec2(0, 0), ImVec2(1, 1), co);
			}
			else
			{
				ImColor co(0.0f, 0.0f, 0.0f, 0.4f);
				ImGui::GetBackgroundDrawList()->AddRectFilled(posCur, b, co);
			}
		}
	}

	ImGui::GetBackgroundDrawList()->AddText(posCur, col, text_begin, text_end);
}
void CGUI::RenderTextWithSize(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size)
{
    int iOffset = pSettings->GetReadOnly().iFontOutline;

    if (bOutline)
    {
        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
}
void CGUI::RenderTextA(float font_size, ImVec2& posCur, ImU32 col, bool bOutline, ImU32 outlineCol, int outlineOffset, const char* text_begin, const char* text_end)
{
    int iOffset = outlineOffset == -1 ? pSettings->GetReadOnly().iFontOutline : outlineOffset;

    if (bOutline)
    {
        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, outlineCol, text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, outlineCol, text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, outlineCol, text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, outlineCol, text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
}
ImFont* CGUI::LoadFont(char *font, float fontsize)
{
    ImGuiIO &io = ImGui::GetIO();

    // load fonts
    char path[0xFF];
    sprintf(path, OBFUSCATE("%sSAMP/fonts/%s"), g_pszStorage, font);

    // ranges
    static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x04FF, // Russia
		0x0E00, 0x0E7F, // Thai
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xF020, 0xF0FF, // Half-width characters
		0
	};

    ImFont* pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
    return pFont;
}

void CGUI::RenderTextDeathMessage(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size, ImFont *font, bool bOutlineUseTextColor)
{
    int iOffset = bOutlineUseTextColor ? 1 : pSettings->GetReadOnly().iFontOutline;
    if(bOutline)
    {
        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, col, text_begin, text_end);
}
void CGUI::AddText(ImFont* font, ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size)
{
    int iOffset = pSettings->GetReadOnly().iFontOutline;

    if (bOutline)
    {
        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, posCur, col, text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, posCur, col, text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, posCur, col, text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, posCur, col, text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(font, font_size, posCur, col, text_begin, text_end);
}