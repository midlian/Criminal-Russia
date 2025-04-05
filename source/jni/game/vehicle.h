#pragma once

#pragma pack(1)
typedef struct _CAR_MOD_INFO
{
	uint8_t byteCarMod0;
	uint8_t byteCarMod1;
	uint8_t byteCarMod2;
	uint8_t byteCarMod3;
	uint8_t byteCarMod4;
	uint8_t byteCarMod5;
	uint8_t byteCarMod6;
	uint8_t byteCarMod7;
	uint8_t byteCarMod8;
	uint8_t byteCarMod9;
	uint8_t byteCarMod10;
	uint8_t byteCarMod11;
	uint8_t byteCarMod12;
	uint8_t byteCarMod13;
	uint8_t bytePaintJob;
	int iColor0;
	int iColor1;
} CAR_MOD_INFO;

enum E_CUSTOM_COMPONENTS
{
	ccBumperF = 0,
	ccBumperR,
	ccFenderF,
	ccFenderR,
	ccSpoiler,
	ccExhaust,
	ccRoof,
	ccTaillights,
	ccHeadlights,
	ccDiffuser,
	ccSplitter,
	ccExtra,
	ccMax
};

enum E_HANDLING_PARAMS
{
	hpMaxSpeed,
	hpAcceleration,
	hpGear,
	hpEngineInertion,
	hpMass,
	hpMassTurn,

	hpBrakeDeceleration,
	hpTractionMultiplier,
	hpTractionLoss,
	hpTractionBias,
	hpSuspensionLowerLimit,
	hpSuspensionBias,

	hpWheelSize,

	hpMax
};

#define EXTRA_COMPONENT_BOOT			10
#define EXTRA_COMPONENT_BONNET			11
#define EXTRA_COMPONENT_BUMP_REAR		12
#define EXTRA_COMPONENT_DEFAULT_DOOR 	13
#define EXTRA_COMPONENT_WHEEL			14
#define EXTRA_COMPONENT_BUMP_FRONT		15

#define MAX_REPLACED_TEXTURES	32
#define MAX_REPLACED_TEXTURE_NAME	32

struct SReplacedTexture
{
	char szOld[MAX_REPLACED_TEXTURE_NAME];
	RwTexture* pTexture;
};

struct SCustomCarShadow
{
	RwTexture* pTexture;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	float fSizeX;
	float fSizeY;
};

void* GetSuspensionLinesFromModel(int nModelIndex, int& numWheels);
uint8_t* GetCollisionDataFromModel(int nModelIndex);

class CVehicle : public CEntity
{
public:
	CVehicle(int iType, float fPosX, float fPosY, float fPosZ, float fRotation = 0.0f, bool bSiren = false);
	~CVehicle();

	bool HasFreePassengerSeats();

	void LinkToInterior(int iInterior);
	void SetColor(int iColor1, int iColor2);

	void SetHealth(float fHealth);
	float GetHealth();

	// 0.3.7
	bool IsOccupied();

	// 0.3.7
	void SetInvulnerable(bool bInv);
	bool IsInvulnerable();
	// 0.3.7
	bool IsDriverLocalPlayer();
	// 0.3.7
	bool HasSunk();

	void ProcessMarkers(float VehicleDistance);

	void RemoveEveryoneFromVehicle();

	void SetWheelPopped(uint8_t bytePopped);
	void SetDoorState(int iState);
	int GetDoorState();

	void UpdateDamageStatus(uint32_t dwPanelDamage, uint32_t dwDoorDamage, uint8_t byteLightDamage, uint8_t byteTireDamage);

	unsigned int GetVehicleSubtype();

	void SetEngineState(int iState);
	void EnableEngine(bool bEnable);
	int GetEngineState();
	
	void SetLightsState(int iState);

	uint8_t GetSirenState();
	void SetSirenState(uint8_t state);

	VEHICLE_TYPE* GetVehiclePtr();
	void ProcessDamage();
	void GetDamageStatusEncoded(uint8_t* byteTyreFlags, uint8_t* byteLightFlags, uint32_t* dwDoorFlags, uint32_t *dwPanelFlags);

	void ToggleTiresInvulnerable(bool bToggle);
	bool HasDamageModel();

	void SetBonnetAndBootStatus(bool bBonnet, bool bBoot);
	void SetDoorOpenStatus(bool bDriver, bool bPassenger, bool bBackLeft, bool bBackRight);

	bool IsSirenAdded();

	uint8_t GetRailTrack();
	void SetRailTrack(uint8_t ucTrackID);
	bool IsDerailed();
	float GetTrainPosition();

	float GetTrainSpeed();
	void SetTrainSpeed(float fTrainSpeed);

	void AttachTrailer();
	void DetachTrailer();
	void SetTrailer(CVehicle* pTrailer);
	CVehicle* GetTrailer();

	void RemoveComponent(uint16_t uiComponent);

	void SetComponentVisible(uint8_t group, uint16_t components);
	void SetHandlingData(std::vector<SHandlingData>& vHandlingData);
	void ResetVehicleHandling();

	void ApplyVinyls(uint8_t bSlot1, uint8_t bSlot2);
	void ApplyToner(uint8_t bSlot, uint8_t bID);

	void ApplyTexture(const char* szTexture, const char* szNew);
	void ApplyTexture(const char* szTexture, RwTexture* pTexture);
	void RemoveTexture(const char* szOldTexture);
	bool IsRetextured(const char* szOldTexture);

	void SetHeadlightsColor(uint8_t r, uint8_t g, uint8_t b);
	void ProcessHeadlightsColor(uint8_t& r, uint8_t& g, uint8_t& b);
	void SetWheelAlignment(int iWheel, float angle);
	void SetWheelOffset(int iWheel, float offset);
	void SetWheelWidth(float fValue);
	void ProcessWheelsOffset();

	void SetCustomShadow(uint8_t r, uint8_t g, uint8_t b, float fSizeX, float fSizeY, const char* szTex);

	/* CDamageManager functions */
	uint8_t GetWheelStatus(eWheelPosition bWheel);
	void SetWheelStatus(eWheelPosition bWheel, uint8_t bTireStatus);
	void SetPanelStatus(uint8_t bPanel, uint8_t bPanelStatus);
	void SetPanelStatus(uint32_t ulPanelStatus);
	uint8_t GetPanelStatus(uint8_t bPanel);
	uint32_t GetPanelStatus();
	void SetLightStatus(uint8_t bLight, uint8_t bLightStatus);
	void SetLightStatus(uint8_t ucStatus);
	uint8_t GetLightStatus(uint8_t bLight);
	uint8_t GetDoorStatus(eDoors bDoor);
	void SetDoorStatus(eDoors bDoor, uint8_t bDoorStatus, bool spawnFlyingComponen);
	void SetDoorStatus(uint32_t dwDoorStatus, bool spawnFlyingComponen);
	void SetBikeWheelStatus(uint8_t bWheel, uint8_t bTireStatus);
	uint8_t GetBikeWheelStatus(uint8_t bWheel);

private:
	void ProcessWheelOffset(RwFrame* pFrame, bool bLeft, float fValue, int iID);
	void SetComponentAngle(bool bUnk, int iID, float angle);

	void SetComponentVisibleInternal(const char* szComponent, bool bVisible);
	std::string GetComponentNameByIDs(uint8_t group, int subgroup);

	void CopyGlobalSuspensionLinesToPrivate();
	
	int m_dwLastDamageTick;
	
public:
	VEHICLE_TYPE	*m_pVehicle;
	bool 			m_bIsLocked;
	CVehicle		*m_pTrailer;
	uint32_t		m_dwMarkerID;
	bool 			m_bIsInvulnerable;
	bool 			m_bDoorsLocked;
	uint8_t			m_byteObjectiveVehicle; // Is this a special objective vehicle? 0/1
	uint8_t			m_bSpecialMarkerEnabled;
	uint8_t			m_bEngineState;


	uint8_t			m_byteColor1;
	uint8_t			m_byteColor2;
	bool 			m_bColorChanged;

	// Attached objects
	std::list<CObject*> m_pAttachedObjects;

	// Damage status
	uint8_t			m_byteTyreStatus;
	uint8_t			m_byteLightStatus;
	uint32_t		m_dwDoorStatus;
	uint32_t		m_dwPanelStatus;

	bool			m_bAddSiren;

	tHandlingData* m_pCustomHandling;

	SReplacedTexture m_szReplacedTextures[MAX_REPLACED_TEXTURES];
	bool m_bReplaceTextureStatus[MAX_REPLACED_TEXTURES];
	bool m_bReplacedTexture;

	void* m_pSuspensionLines;
	bool bHasSuspensionLines;

	bool m_bHeadlightsColor;
	uint8_t m_bHeadlightsR;
	uint8_t m_bHeadlightsG;
	uint8_t m_bHeadlightsB;

	bool m_bWheelAlignmentX;
	float m_fWheelAlignmentX;

	bool m_bWheelAlignmentY;
	float m_fWheelAlignmentY;

	bool m_bWheelSize;
	float m_fWheelSize;

	bool m_bWheelWidth;
	float m_fWheelWidth;

	bool m_bWheelOffsetX;
	float m_fWheelOffsetX;

	bool m_bWheelOffsetY;
	float m_fWheelOffsetY;

	float m_fNewOffsetX;
	float m_fNewOffsetY;

	bool m_bWasWheelOffsetProcessedX;
	bool m_bWasWheelOffsetProcessedY;
	uint32_t m_uiLastProcessedWheelOffset;

	MATRIX4X4 m_vInitialWheelMatrix[4];

	SCustomCarShadow m_Shadow;
	bool m_bShadow;
};