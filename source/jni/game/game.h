#pragma once
#include "rgba.h"
#include "RW/common.h"
#include "aimstuff.h"
#include "pad.h"
#include "quaternion.h"
#include "entity.h"
#include "playerped.h"
#include "vehicle.h"
#include "object.h"
#include "camera.h"
#include "font.h"
#include "scripting.h"
#include "radarcolors.h"
#include "util.h"
#include "textdraw.h"
#include "sprite2d.h"
#include "RW/RenderWare.h"
#include "widget.h"
#include "widgetmanager.h"
#include "stream.h"
#include "CHandlingDefault.h"
#include "CActorPed.h"
#include "CExtendedCarColors.h"
#include "CWeaponsOutFit.h"
#include "CRadarRect.h"
#include "CAdjustableHudColors.h"
#include "CAdjustableHudPosition.h"
#include "CInfoBarText.h"
#include "CAdjustableHudScale.h"
#include "CFirstPersonCamera.h"
#include "CSnow.h"
#include "snapshothelper.h"

class CGame
{
public:
	CGame();
	~CGame() {};

	int GetScreenWidth() { return RsGlobal->maximumWidth; };
	int GetScreenHeight() { return RsGlobal->maximumHeight; };
	float GetHudVertScale() { return 0.002232143; };
	float GetHudHorizScale() { return 0.0015625; };

	void InitInMenu();
	void InitInGame();

	void RemoveModel(int iModel, bool bFromStreaming);

	void ToggleHUDElement(int iID, bool bToggle);
	bool IsToggledHUDElement(int iID);
	void HandleChangedHUDStatus();

	static void SetEnabledPCMoney(bool bEnabled);

	CCamera* GetCamera() { return m_pGameCamera; }
	// 0.3.7
	CPlayerPed* FindPlayerPed() { if(!m_pGamePlayer) m_pGamePlayer = new CPlayerPed(); return m_pGamePlayer; }
	// 0.3.7
	uint8_t FindFirstFreePlayerPedSlot();
	// 0.3.7
	CPlayerPed* NewPlayer(int iSkin, float fX, float fY, float fZ, float fRotation, uint8_t byteCreateMarker = 0);
	// 0.3.7
	void RemovePlayer(CPlayerPed* pPlayer);
	// 0.3.7
	CVehicle* NewVehicle(int iType, float fPosX, float fPosY, float fPosZ, float fRotation, bool bAddSiren);
	CObject* NewObject(int iModel, float fPosX, float fPosY, float fPosZ, VECTOR vecRot, float fDrawDistance);
	uint32_t CreatePickup(int iModel, int iType, float fX, float fY, float fZ, int *unk);

	float FindGroundZForCoord(float x, float y, float z);
	// 0.3.7
	uint8_t GetActiveInterior();
	// 0.3.7
	void SetWorldTime(int iHour, int iMinute);
	// 0.3.7
	void SetWorldWeather(unsigned char byteWeatherID);
	// 0.3.7
	void EnableClock(bool bEnable);
	void ToggleThePassingOfTime(bool bOnOff);
	// 0.3.7
	void EnableZoneNames(bool bEnable);
	void DisplayWidgets(bool bDisp);
	void PlaySound(int iSound, float fX, float fY, float fZ);

	// 0.3.7
	void ToggleRadar(bool iToggle);
	// 0.3.7
	void DisplayHUD(bool bDisp);

	// 0.3.7
	void SetCheckpoint(VECTOR* pos, VECTOR* extent);
	void DisableCheckpoint();
	// 0.3.7
	void ToggleRaceCheckpoints(bool bEnabled) { m_bRaceCheckpointsEnabled = bEnabled; }
	// 0.3.7
	void SetRaceCheckpointInformation(uint8_t byteType, VECTOR *pos, VECTOR *next, float fSize);
	// 0.3.7
	void UpdateCheckpoints();
	// 0.3.7
	void MakeRaceCheckpoint();
	// 0.3.7
	void DisableRaceCheckpoint();
	// 0.3.7
	uint32_t CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle);
	// 0.3.7
	void DisableMarker(uint32_t dwMarkerID);
	// 0.3.7
	void RequestModel(unsigned int iModelID, int iLoadingStream = 2);
	// 0.3.7
	void LoadRequestedModels();
	// 0.3.7
	uint8_t IsModelLoaded(unsigned int iModelID);
	// 0.3.7
	void RefreshStreamingAt(float x, float y);
	// 0.3.7
	void DisableTrainTraffic();
	// 0.3.7
	void SetMaxStats();

	void SetWantedLevel(uint8_t byteLevel);

	bool IsAnimationLoaded(char* szAnimFile);
	void RequestAnimation(char* szAnimFile);
	// 0.3.7
	void DisplayGameText(char* szStr, int iTime, int iType);
	// 0.3.7
	void SetGravity(float fGravity);
	void ToggleCJWalk(bool bUseCJWalk);
	// 0.3.7
	void DisableInteriorEnterExits();

	// 0.3.7
	int GetLocalMoney();
	// 0.3.7
	void AddToLocalMoney(int iAmmount);
	// 0.3.7
	void ResetLocalMoney();

	void DrawGangZone(float fPos[], uint32_t dwColor);

	uint8_t IsGamePaused();

private:
	bool aToggleStatusHUD[HUD_MAX];
	CCamera* 		m_pGameCamera;
	CPlayerPed*		m_pGamePlayer;

	// race checkpoint
	bool			m_bRaceCheckpointsEnabled;
	uint32_t		m_dwRaceCheckpointHandle;
	uint32_t		m_dwRaceCheckpointMarker;
	float			m_fRaceCheckpointSize;
	uint8_t			m_byteRaceType;
	VECTOR 			m_vecRaceCheckpointPos;
	VECTOR			m_vecRaceCheckpointNext;

	bool 			m_bClockEnabled;

	struct
	{
		bool m_bIsActive;
		VECTOR m_vecPosition;
		VECTOR m_vecExtent;
		uint32_t m_dwMarkerId;
	} m_checkpointData;
};