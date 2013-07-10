#pragma once
#ifndef __Player_H__
#define __Player_H__

#define		PLAYER_WIDTH			15.49193f
#define		PLAYER_HALF_WIDTH		7.74597f
#define		PLAYER_WIDTH_SQR		240.0f
#define		PLAYER_HALF_WIDTH_SQR	60.0f
#define		PLAYER_COL_DET_TOLERANCE 0.005f

typedef struct State_st {
	float	fX;
	float	fY;
	float	fZ;
	//float	fVelX;
	//float	fVelY;

	bool CloseTo(State_st oOtherState)
	{
		Vector2 oOwnPosition(fX, fY);
		Vector2 oOtherPosition(oOtherState.fX, oOtherState.fY);

		return ((oOwnPosition - oOtherPosition).SquaredLength() <= 0.001f &&
				std::abs(fZ - oOtherState.fZ) <= 0.001f);
	}
} State_t;

typedef struct {
	State_t	oState;
	u_char	cSequenceNumber;
} SequencedState_t;

typedef struct {
	SequencedState_t	oState;
	bool				bAuthed;
} AuthState_t;

typedef struct {
	char	cMoveDirection;
	u_char	cStealth;
	float	fZ;
} Command_t;

typedef struct {
	Command_t	oCommand;
	u_char		cSequenceNumber;
} SequencedCommand_t;

/*typedef struct {
	Command_t	oCommand;
	State_t		oResultState;
} Move_t;*/

// Returns a player
CPlayer * PlayerGet(u_int nPlayerId);

class CPlayer
{
public:
	CPlayer();
	CPlayer(u_int nPlayerId);
	virtual ~CPlayer();

	SequencedState_t PhysicsTickTEST(SequencedState_t oStartingState, SequencedCommand_t oCommand);
	void Render();
	void RenderInPast(double dTimeAgo);
	void UpdateRenderState();
	const State_t GetRenderState();
	void SetTeam(int nTeam);
	int GetTeam();
	void PushStateHistory(AuthState_t oAuthState);
	void Position(SequencedState_t oSequencedState);
	void Position(float fX, float fY, float fZ, u_char cLastCommandSequenceNumber);
	//float GetVelocity();
	void Rotate(float fAmount);
	void SetZ(float fValue);
	float GetZ();
	void Fire();
	void Reload();
	int GetSelClips();
	int GetSelClipAmmo();
	void InitWeapons();
	void BuyClip();
	bool IsReloading();
	float GetHealth();
	void GiveHealth(float fValue);
	bool IsDead();
	std::string & GetName();
	void SetName(std::string & sNewName);
	void RespawnReset();
	PlayerController * m_pController;
	PlayerStateAuther * m_pStateAuther;
	void SeekRealtimeInput(double dTimePassed);
	void Tick();
	bool bWeaponFireTEST;
	void WeaponTickTEST();
	void AfterTick()				{ if (m_pStateAuther) m_pStateAuther->AfterTick(); }
	void ProcessAuthUpdateTEST()	{ if (m_pStateAuther) m_pStateAuther->ProcessAuthUpdateTEST(); }
	void SendUpdate()				{ if (m_pStateAuther) m_pStateAuther->SendUpdate(); }
	double		m_dNextUpdateTime;

#ifdef EX0_CLIENT
	//State_t GetStateInPastX(float fTimeAgo);
	State_t GetStateInPast(double dTimeAgo);
#endif // EX0_CLIENT

	u_char		GlobalStateSequenceNumberTEST;

	SequencedState_t	oLatestAuthStateTEST;
	//u_char		cLatestAuthStateSequenceNumber;

	std::list<AuthState_t>			oStateHistory;		// The front has the latest entries
	IndexedCircularBuffer<Command_t, u_char>	oUnconfirmedCommands;		// The front has the oldest commands

	ThreadSafeQueue<SequencedCommand_t, 100>	m_oCommandsQueue;
	ThreadSafeQueue<SequencedState_t, 3>		m_oUpdatesQueue;		// Contains authed state updates

	u_int		iID;
	bool		bEmptyClicked;
	int			iSelWeapon;
//#ifndef EX0_CLIENT
	ClientConnection * pConnection;
//#endif

	static u_int GetPlayerCount();
	static void DeleteAll();

protected:

private:
	CPlayer(const CPlayer &);
	CPlayer & operator =(const CPlayer &);

	void CalcTrajs(State_t & oState);
	void CalcColResp(State_t & oState);

	Command_t	m_oCommand;
	State_t		m_oRenderState;
	float		fZ;
	State_t		m_oDeadState;
	CWeapon		oWeapons[4];
	float		fHealth;
	string		sName;
	int			m_nTeam;

	//std::list<SequencedState_t>		oStateHistory;		// The front has the latest entries
	//State_t							oOnlyKnownState;

	static std::vector<CPlayer *>		m_oPlayers;
	static u_int						m_nPlayerCount;

	static void Add(CPlayer * pPlayer);
	static void Add(CPlayer * pPlayer, u_int nPlayerId);
	static u_int NextFreePlayerId();
	static void Remove(CPlayer * pPlayer);

	friend CPlayer * PlayerGet(u_int nPlayerId);
	friend void RenderPlayers();
};

void PlayerTick();

#endif // __Player_H__
