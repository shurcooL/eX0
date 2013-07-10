#pragma once
#ifndef __Player_H__
#define __Player_H__

#define		NO_PLAYER				(static_cast<uint8>(-1))
#define		PLAYER_WIDTH			15.49193f
#define		PLAYER_HALF_WIDTH		7.74597f
#define		PLAYER_WIDTH_SQR		240.0f
#define		PLAYER_HALF_WIDTH_SQR	60.0f
#define		PLAYER_COL_DET_TOLERANCE 0.005f

struct State_st
{
	float	fX;
	float	fY;
	float	fZ;

	float	fVelX;
	float	fVelY;

	bool CloseTo(State_st oOtherState)
	{
		Vector2 oOwnPosition(fX, fY);
		Vector2 oOtherPosition(oOtherState.fX, oOtherState.fY);

		return ((oOwnPosition - oOtherPosition).SquaredLength() <= 0.001f &&
				std::abs(fZ - oOtherState.fZ) <= 0.001f);
	}
};

struct SequencedState_st
{
	State_st	oState;
	u_char	cSequenceNumber;
};

struct AuthState_st
{
	SequencedState_st	oState;
	bool				bAuthed;
};

struct Command_st {
	char	cMoveDirection;
	bool	bStealth;
	float	fZ;
};

struct SequencedCommand_st
{
	Command_st	oCommand;
	u_char		cSequenceNumber;
};

/*struct Move_t
{
	Command_st	oCommand;
	State_st	oResultState;
};*/

// Returns a player
CPlayer * PlayerGet(uint8 nPlayerId);

class CPlayer
{
public:
	CPlayer();
	CPlayer(uint8 nPlayerId);
	virtual ~CPlayer();

	SequencedState_st PhysicsTickTEST(SequencedState_st oStartingState, SequencedCommand_st oCommand);
	void Render();
	void RenderInsideMask();
	void RenderInPast(double dTimeAgo);
	void UpdateRenderState();
	const State_st GetRenderState();
	void SetTeam(int nTeam);
	int GetTeam();
	void PushStateHistory(AuthState_st oAuthState);
	void Position(SequencedState_st oSequencedState);
	void Position(float fX, float fY, float fZ, u_char cLastCommandSequenceNumber);
	//float GetVelocity();
	void Rotate(float fAmount);
	void SetZ(float fValue);
	float GetZ();
	uint32 GetAmmo();
	uint32 GetClips();
	void InitWeapons();
	void BuyClip();
	bool IsReloading();
	float GetHealth();
	void GiveHealth(float fValue, int nSourcePlayerId = -1);
	bool IsDead();
	std::string & GetName();
	void SetName(std::string & sNewName);
	void RespawnReset();
	PlayerController * m_pController;
	PlayerStateAuther * m_pStateAuther;
	void SeekRealtimeInput(double dTimePassed);
	void Tick();
	void WeaponTickTEST();
	WeaponSystem & GetSelWeaponTEST() { return oWeapons; };
	void ProcessCommands()			{ m_pStateAuther->ProcessCommands(); }
	void ProcessWpnCommands()		{ m_pStateAuther->ProcessWpnCommands(); }
	void ProcessUpdates()			{ m_pStateAuther->ProcessUpdates(); }
	void SendUpdate()				{ m_pStateAuther->SendUpdate(); }
	double		m_dNextUpdateTime;		// TODO: Move this inside LSA?

#ifdef EX0_CLIENT
	//State_st GetStateInPastX(float fTimeAgo);
	State_st GetStateInPast(double dTimeAgo, GameTimer * pTimer = nullptr);
	State_st GetStateAtTime(double dTime);
#endif // EX0_CLIENT

	u_char		GlobalStateSequenceNumberTEST;		// It's used to indicate the current/latest state the player has been simulated up to (on client, it == latest pred state, but on server it indicates how many move commands need to be executed)

	SequencedState_st	oLatestAuthStateTEST;		// This is a copy of what's latest auth state in oStateHistory
	//u_char		cLatestAuthStateSequenceNumber;

	std::list<AuthState_st>			oStateHistory;		// The front has the latest entries
	IndexedCircularBuffer<Command_st, u_char>	oUnconfirmedMoves;		// The front has the oldest commands

	ThreadSafeQueue<SequencedCommand_st, 100>	m_oCommandsQueue;
	ThreadSafeQueue<WpnCommand_st, 10>			m_oWpnCommandsQueue;
	ThreadSafeQueue<SequencedState_st, 10>		m_oUpdatesQueue;		// Contains authed state updates

	uint8		iID;
//#ifndef EX0_CLIENT
	ClientConnection * pConnection;
//#endif

	static u_int GetPlayerCount();
	static void DeleteAll();

protected:

private:
	CPlayer(const CPlayer &);
	CPlayer & operator =(const CPlayer &);

	void CalcTrajs(State_st & oState);
	void CalcColResp(State_st & oState);

	Command_st	m_oCommand;
	float		m_fOldX, m_fOldY;
	State_st		m_oRenderState;
	float		fZ;
	State_st		m_oDeadState;
	WeaponSystem	oWeapons;
	float		fHealth;
	string		sName;
	int			m_nTeam;

	//std::list<SequencedState_st>		oStateHistory;		// The front has the latest entries
	//State_st							oOnlyKnownState;

	static std::vector<CPlayer *>		m_oPlayers;
	static u_int						m_nPlayerCount;

	static void Add(CPlayer * pPlayer);
	static void Add(CPlayer * pPlayer, uint8 nPlayerId);
	static uint8 NextFreePlayerId();
	static void Remove(CPlayer * pPlayer);

	friend CPlayer * PlayerGet(uint8 nPlayerId);
	friend void RenderPlayers();
};

void PlayerTick();

#endif // __Player_H__
