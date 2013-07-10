struct WeaponSpec_t
{
	string	sName;
	int		iWhatType;
	int		iClips;
	int		iMaxClips;
	int		iClipAmmo;
	float	fROF;
	float	fReloadTime;
	float	fProjSpeed;
	float	fInaccuracy;
	float	fMaxDamage;
	float	fMaxLife;
};

/*struct WeaponInfo_t
{
	int		iClips;
	int		iClipAmmo;
	bool	bReloading;
	int		iTimer;
};*/

struct Projectile_t
{
	float	fX;
	float	fY;
	float	fVelX, fVelY;
	int		iWhatType;
	float	fMaxDamage;
	bool	bAvaliable;
};

class CWeapon
{
public:
	CWeapon();
	~CWeapon();

	void Tick();
	void Init(int iWhoIsOwner, int iWeapon);
	void Fire();
	void Reload();
	int GetClips();
	int GetClipAmmo();
	void GiveClip();
	bool IsReloading();

private:
	int		iWhatWeapon;
	int		iOwnerID;
	int		iClips;
	int		iClipAmmo;
	bool	bReloading;
	bool	bShouldReload;
	float	fTimer;
};

// initialize all weapons specs
void WeaponInitSpecs();
