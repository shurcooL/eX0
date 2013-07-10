#pragma once
#ifndef __Player_H__
#define __Player_H__

#define		PLAYER_WIDTH			15.49193f
#define		PLAYER_HALF_WIDTH		7.74597f
#define		PLAYER_WIDTH_SQR		240.0f
#define		PLAYER_HALF_WIDTH_SQR	60.0f
#define		PLAYER_COL_DET_TOLERANCE 0.005f

#ifdef EX0_CLIENT
typedef struct {
	float	fX;
	float	fY;
	float	fZ;
	//float	fVelX;
	//float	fVelY;
} State_t;

typedef struct {
	State_t	oState;
	u_char	cSequenceNumber;
} SequencedState_t;

typedef struct {
	char	cMoveDirection;
	u_char	cStealth;
	float	fZ;
} Input_t;

typedef struct {
	Input_t	oInput;
	State_t	oState;
} Move_t;
#endif // EX0_CLIENT

class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	void MoveDirection(int nDirection);
	void Rotate(float fAmount);
	void CalcTrajs();
	void CalcColResp();
	void Render();
	void RenderInPast(float fTimeAgo);
	void SetTeam(int nTeam);
	int GetTeam();
	void SetStealth(bool bOn);
	float GetX();
	float GetY();
	float GetOldX();
	float GetOldY();
#ifdef EX0_CLIENT
	void PushStateHistory(SequencedState_t &oSequencedState);
#endif
	void SetX(float fValue);
	void SetY(float fValue);
	void SetOldX(float fValue);
	void SetOldY(float fValue);
	void Position(float fNewX, float fNewY, float fNewZ);
#ifdef EX0_CLIENT
	void Position(float fNewX, float fNewY, float fNewZ, u_char cSequenceNumber);
#endif
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
	void Tick();
#ifdef EX0_CLIENT
	void FakeTick();
#endif
	bool IsReloading();
	float GetHealth();
	void GiveHealth(float fValue);
	bool IsDead();
	string & GetName();
	void SetName(string & sNewName);
#ifdef EX0_CLIENT
	void RespawnReset();
	short unsigned int GetLastLatency();
	void SetLastLatency(short unsigned int nLastLatency);
#endif

	int			iID;
	bool		bEmptyClicked;
	int			iSelWeapon;
	float		fAimingDistance;
	float		fTicks;
#ifdef EX0_CLIENT
	double		dNextTickTime;
#endif
	float		fTickTime;
	float		fOldZ;

#ifdef EX0_CLIENT
	bool		bConnected;
	//u_char		cCurrentCommandSequenceNumber;
	u_char		cLastAckedCommandSequenceNumber;
#else
	CClient		*pClient;
#endif

private:
	float		fX, fY;
	float		fOldX, fOldY;
	float		fVelX, fVelY;
	float		fIntX, fIntY;
	float		fZ;
	int			iIsStealth;
	int			nMoveDirection;
	CWeapon		oWeapons[4];
	float		fHealth;
	string		sName;
	int			m_nTeam;

#ifdef EX0_CLIENT
	list<SequencedState_t>		oStateHistory;		// The front has the latest entries
	State_t						oOnlyKnownState;

	unsigned short int	m_nLastLatency;
	u_char		cCurrentCommandSeriesNumber;

	State_t GetStateInPast(float fTimeAgo);
#endif
};

// allocate memory for all the players
void PlayerInit();

// Returns a player
CPlayer * PlayerGet(int nPlayerID);

void PlayerTick();

int PlayerGetFreePlayerID();

#endif // __Player_H__
