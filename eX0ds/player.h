#define		PLAYER_TICK_TIME		0.045f
//#define		PLAYER_TICK_TIME		0.25f
//#define		PLAYER_TICK_TIME		1.0f
#define		PLAYER_WIDTH			15.49193f
#define		PLAYER_HALF_WIDTH		7.74597f
#define		PLAYER_WIDTH_SQR		240.0f
#define		PLAYER_HALF_WIDTH_SQR	60.0f
#define		PLAYER_COL_DET_TOLERANCE 0.005f

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
	void SetTeam(int iValue);
	void SetStealth(bool bOn);
	float GetX();
	float GetY();
	void SetX(float fValue);
	void SetY(float fValue);
	void Position(float fPosX, float fPosY);
	float GetVelX();
	float GetVelY();
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
	bool IsReloading();
	float GetHealth();
	void GiveHealth(float fValue);
	bool IsDead();

	int			iID;
	bool		bEmptyClicked;
	int			iSelWeapon;
	float		fAimingDistance;

	bool		bConnected;
	SOCKET		nTcpSocket;

private:
	float		fX, fY;
	float		fOldX, fOldY;
	float		fVelX, fVelY;
	float		fIntX, fIntY;
	float		fZ;
	int			iIsStealth;
	int			nMoveDirection;
	//int			iSelWeapon;
	//CWeapon		oWeapons[4];
	float		fHealth;
	string		sName;
	int			iTeam;
	float		fTicks;
};

// allocate memory for all the players
void PlayerInit();

// Returns a player
CPlayer * PlayerGet(int nPlayerID);

// Returns a player from their socket number
CPlayer * PlayerGetFromSocket(SOCKET nSocket);

void PlayerTick();

int PlayerGetFreePlayerID();
