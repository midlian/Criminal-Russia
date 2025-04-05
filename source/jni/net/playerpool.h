#pragma once

#define NO_TEAM 255

class CPlayerPool
{
public:
	CPlayerPool();
	~CPlayerPool();

	bool Process();
	PLAYERID GetCount();
	// LOCAL

	void UpdateScore(PLAYERID playerId, int iScore);
	void UpdatePing(PLAYERID playerId, uint32_t dwPing);

	int GetLocalPlayerScore() { return m_iLocalPlayerScore; }
	uint32_t GetLocalPlayerPing() { return m_dwLocalPlayerPing; }

	int GetRemotePlayerScore(PLAYERID playerId)
	{
		if (!ARRAY_VALIDATE(m_iPlayerScores, playerId)) return 0;
		return m_iPlayerScores[playerId];
	}

	uint32_t GetRemotePlayerPing(PLAYERID playerId)
	{
		if (!ARRAY_VALIDATE(m_dwPlayerPings, playerId)) return 0;
		return m_dwPlayerPings[playerId];
	}

	void SetLocalPlayerName(const char* szName) { strcpy(m_szLocalPlayerName, szName); }
	char* GetLocalPlayerName() { return m_szLocalPlayerName; }
	void SetLocalPlayerID(PLAYERID MyPlayerID)
	{
		if (!ARRAY_VALIDATE(m_szPlayerNames, MyPlayerID)) return;
		if (!STR_VALIDATE(m_szLocalPlayerName)) return;

		strcpy(m_szPlayerNames[MyPlayerID], m_szLocalPlayerName);
		m_LocalPlayerID = MyPlayerID;
	}
	PLAYERID GetLocalPlayerID() { return m_LocalPlayerID; }
	CLocalPlayer* GetLocalPlayer() { return m_pLocalPlayer; }
	// remote
	bool New(PLAYERID playerId, char* szPlayerName, bool bIsNPC);
	bool Delete(PLAYERID playerId, uint8_t byteReason);

	CRemotePlayer *GetAt(PLAYERID playerId)
	{
		if (!ARRAY_VALIDATE(m_pPlayers, playerId)) return nullptr;
		return m_pPlayers[playerId];
	};

	bool GetSlotState(PLAYERID playerId)
	{
		if (!ARRAY_VALIDATE(m_bPlayerSlotState, playerId)) return false;
		return m_bPlayerSlotState[playerId];
	};

	void SetPlayerName(PLAYERID playerId, char* szName) 
	{ 
		if (!ARRAY_VALIDATE(m_szPlayerNames, playerId)) return;
		if (!STR_VALIDATE(szName)) return;
		strcpy(m_szPlayerNames[playerId], szName); 
	};

	char* GetPlayerName(PLAYERID playerId)
	{ 
		if (!ARRAY_VALIDATE(m_szPlayerNames, playerId)) return "Error_NT_1";
		return m_szPlayerNames[playerId]; 
	};

	PLAYERID FindRemotePlayerIDFromGtaPtr(PED_TYPE * pActor);
	PLAYERID FindRemotePlayerIDFromGtaEntity(ENTITY_TYPE* pEntity);

	void ApplyCollisionChecking();
	void ResetCollisionChecking();
private:
	// LOCAL
	PLAYERID		m_LocalPlayerID;
	CLocalPlayer	*m_pLocalPlayer;
	char			m_szLocalPlayerName[MAX_PLAYER_NAME+1];
	int				m_iLocalPlayerScore;
	uint32_t		m_dwLocalPlayerPing;

	// REMOTE
	CRemotePlayer	*m_pPlayers[MAX_PLAYERS];
	bool			m_bPlayerSlotState[MAX_PLAYERS];
	char			m_szPlayerNames[MAX_PLAYERS][MAX_PLAYER_NAME+1];
	int				m_iPlayerScores[MAX_PLAYERS];
	uint32_t		m_dwPlayerPings[MAX_PLAYERS];

	bool			m_bCollisionChecking[MAX_PLAYERS];
};