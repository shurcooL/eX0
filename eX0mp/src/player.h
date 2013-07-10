#pragma once
#ifndef __CPlayer_H__
#define __CPlayer_H__

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

typedef struct {
	u_char	cSequenceNumber;
	State_t	oState;
} SequencedState_t;

typedef struct {
	char	cMoveDirection;
	float	fZ;
	//char	cStealth;
} Input_t;

typedef struct {
	Input_t	oInput;
	State_t	oState;
} Move_t;

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
	void PushStateHistory(SequencedState_t &oSequencedState);
	void SetX(float fValue);
	void SetY(float fValue);
	void SetOldX(float fValue);
	void SetOldY(float fValue);
	void Position(float fNewX, float fNewY, float fNewZ);
	void Position(float fNewX, float fNewY, float fNewZ, u_char cSequenceNumber);
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
	void FakeTick();
	bool IsReloading();
	float GetHealth();
	void GiveHealth(float fValue);
	bool IsDead();
	string & GetName();
	void SetName(string &sNewName);
	void RespawnReset();
	short unsigned int GetLastLatency();
	void SetLastLatency(short unsigned int nLastLatency);

	int			iID;
	bool		bEmptyClicked;
	int			iSelWeapon;
	float		fAimingDistance;
	float		fTicks;
	double		dNextTickTime;
	//float		fUpdateTicks;
	float		fTickTime;
	float		fOldZ;
	bool		bFirstUpdate;

	bool		bConnected;
	//u_char		cCurrentCommandSequenceNumber;
	u_char		cLastAckedCommandSequenceNumber;

private:
	float		fX, fY;
	float		fOldX, fOldY;
	float		fVelX, fVelY;
	float		fIntX, fIntY;
	float		fZ;
	int			iIsStealth;
	int			nMoveDirection;
	//int			iSelWeapon;
	CWeapon		oWeapons[4];
	float		fHealth;
	string		sName;
	int			m_nTeam;

	list<SequencedState_t>		oStateHistory;
	State_t						oOnlyKnownState;

	unsigned short int	m_nLastLatency;
	u_char		cCurrentCommandSeriesNumber;

	State_t GetStateInPast(float fTimeAgo);
};

// allocate memory for all the players
void PlayerInit();

// Returns a player
CPlayer * PlayerGet(int nPlayerID);

void PlayerTick();

int PlayerGetFreePlayerID();

#endif // __CPlayer_H__
