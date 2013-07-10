#pragma once
#ifndef __Weapon_H__
#define __Weapon_H__

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
	void Render();
	void Init(int iWhoIsOwner, int iWeapon);
	void Fire();
	int GetClips();
	int GetClipAmmo();
	void GiveClip();
	void StartReloading();
	bool IsReloading();

private:
	int		iWhatWeapon;
	int		iOwnerID;
	int		iClips;
	int		iClipAmmo;
	bool	bReloading;
	bool	bReloadRequested;
	double	dTimer;

	enum MuzzleFlashState { READY, VISIBLE, COOLDOWN } m_oMuzzleFlashState;

	void Reload();
};

// initialize all weapons specs
void WeaponInitSpecs();

#endif // __Weapon_H__
