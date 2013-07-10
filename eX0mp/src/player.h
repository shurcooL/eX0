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

class CPlayer
{
public:
	CPlayer(void);
	~CPlayer(void);

	void MoveDirection(int nDirection);
	void Rotate(float fAmount);
	void CalcTrajs(void);
	void CalcColResp(void);
	void Render(void);
	void RenderInPast(float fTimeAgo);
	void SetTeam(int iValue);
	void SetStealth(bool bOn);
	float GetX(void);
	float GetY(void);
	float GetOldX(void);
	float GetOldY(void);
	void PushStateHistory(SequencedState_t &oSequencedState);
	void SetX(float fValue);
	void SetY(float fValue);
	void SetOldX(float fValue);
	void SetOldY(float fValue);
	void Position(float fNewX, float fNewY, float fNewZ);
	void Position(float fNewX, float fNewY, float fNewZ, u_char cSequenceNumber);
	float GetVelX(void);
	float GetVelY(void);
	void SetVelX(float fValue);
	void SetVelY(float fValue);
	float GetVelocity(void);
	float GetIntX(void);
	float GetIntY(void);
	void SetZ(float fValue);
	float GetZ(void);
	void UpdateInterpolatedPos(void);
	void Fire(void);
	void Reload(void);
	int GetSelClips(void);
	int GetSelClipAmmo(void);
	void InitWeapons(void);
	void BuyClip(void);
	void Tick(void);
	bool IsReloading(void);
	float GetHealth(void);
	void GiveHealth(float fValue);
	bool IsDead(void);
	string & GetName(void);
	void SetName(string &sNewName);

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
	int			iTeam;

	list<SequencedState_t>		oStateHistory;
	State_t						oOnlyKnownState;

	State_t GetStateInPast(float fTimeAgo);
};

// allocate memory for all the players
void PlayerInit(void);

// Returns a player
CPlayer * PlayerGet(int nPlayerID);

void PlayerTick(void);

int PlayerGetFreePlayerID(void);
