#pragma once
#ifndef __Player_H__
#define __Player_H__

#define		PLAYER_WIDTH			15.49193f
#define		PLAYER_HALF_WIDTH		7.74597f
#define		PLAYER_WIDTH_SQR		240.0f
#define		PLAYER_HALF_WIDTH_SQR	60.0f
#define		PLAYER_COL_DET_TOLERANCE 0.005f

typedef struct {
	float	fX;
	float	fY;
	float	fZ;
	//float	fVelX;
	//float	fVelY;
} State_t;

typedef struct SequencedState_st {
	State_t	oState;
	u_char	cSequenceNumber;
} SequencedState_t;

typedef struct {
	char	cMoveDirection;
	u_char	cStealth;
	float	fZ;
} Command_t;

typedef struct {
	Command_t	oCommand;
	u_char		cSequenceNumber;
} SequencedCommand_t;

typedef struct {
	Command_t	oCommand;
	State_t		oState;
} Move_t;

// Returns a player
CPlayer * PlayerGet(u_int nPlayerId);

class CPlayer
{
public:
	CPlayer();
	CPlayer(u_int nPlayerId);
	virtual ~CPlayer();

	void MoveDirection(int nDirection);
	int GetMoveDirection() const { return nMoveDirection; }
	void Rotate(float fAmount);
	void CalcTrajs();
	void CalcColResp();
	void Render();
	void RenderInPast(double dTimeAgo);
	void SetTeam(int nTeam);
	int GetTeam();
	void SetStealth(bool bOn);
	float GetX();
	float GetY();
	float GetOldX();
	float GetOldY();
//#ifdef EX0_CLIENT
	void PushStateHistory(SequencedState_t & oSequencedState);
//#endif // EX0_CLIENT
	void SetX(float fValue);
	void SetY(float fValue);
	void SetOldX(float fValue);
	void SetOldY(float fValue);
	void Position(float fNewX, float fNewY, float fNewZ);
#ifdef EX0_CLIENT
	void Position(float fNewX, float fNewY, float fNewZ, u_char cSequenceNumber);
#endif // EX0_CLIENT
	float GetVelX();
	float GetVelY();
	void SetVelX(float fValue);
	void SetVelY(float fValue);
	float GetVelocity();
	float GetIntX();
	float GetIntY();
	void SetZ(float fValue);
	float GetZ();
	void UpdateInterpolatedPos();
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
	State_t GetStateInPastX(float fTimeAgo);
	State_t GetStateInPast(double dTimeAgo);
#endif // EX0_CLIENT

	int64		GlobalStateSequenceNumberTEST;

	u_char		cLatestAuthStateSequenceNumber;

	ThreadSafeQueue<SequencedCommand_t, 100>	m_oInputCmdsTEST;
	ThreadSafeQueue<SequencedState_t, 3>		m_oAuthUpdatesTEST;

	u_int		iID;
	bool		bEmptyClicked;
	int			iSelWeapon;
	float		fAimingDistance;
	float		fTicks;
	float		fTickTime;
	float		fOldZ;
//#ifndef EX0_CLIENT
	ClientConnection * pConnection;
//#endif

	static u_int GetPlayerCount();
	static void DeleteAll();

protected:

private:
	CPlayer(const CPlayer &);
	CPlayer & operator =(const CPlayer &);

	float		fX, fY;
	float		fOldX, fOldY;
	float		fVelX, fVelY;
	float		fIntX, fIntY;
	float		fZ;
	State_t		m_oDeadState;
	int			iIsStealth;
	int			nMoveDirection;
	CWeapon		oWeapons[4];
	float		fHealth;
	string		sName;
	int			m_nTeam;

	std::list<SequencedState_t>		oStateHistory;		// The front has the latest entries
	State_t							oOnlyKnownState;

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
