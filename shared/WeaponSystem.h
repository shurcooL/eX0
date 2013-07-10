#pragma once
#ifndef __WeaponSystem_H__
#define __WeaponSystem_H__

struct WeaponSpec_st
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

struct WeaponType_st
{
	uint8			id;
	std::string		name;

	uint8			projectile_type_id;
	uint32			max_ammo;
	uint32			max_clips;
	double			rate_of_fire;
	double			reload_time;
	//float			fInaccuracy;
	//float			fMaxDamage;
	//float			fMaxLife;

	double			bring_out_time;
};

/*struct Projectile_st
{
	float	fX;
	float	fY;
	float	fVelX, fVelY;
	int		iWhatType;
	float	fMaxDamage;
	bool	bAvaliable;
};*/

struct WeaponInstance_st;
struct WpnCommand_st;

class WeaponSystem
{
public:
	WeaponSystem(uint8 OwnerPlayerId);
	~WeaponSystem();

	void Reset();
	void AddWeapon(uint8 WeaponTypeId);
	void ClearWeapons();

	bool IsCommandOutdated(WpnCommand_st & oWpnCommand, bool bPushForwardCorrectionEnabled);
	bool IsReadyForNextCommand(WpnCommand_st & oWpnCommand);
	void PreprocessWpnCommand(WpnCommand_st & oWpnCommand);
	bool ProcessWpnCommand(WpnCommand_st & oWpnCommand);

	void Render();

	uint32 GetAmmo();
	uint32 GetClips();
	bool IsReloading();
	int32 ChangingWeaponTo();
	void GiveClip();

	// TODO: If IDLE turns out not to be needed, remove it
	enum WpnAction { IDLE = 0, FIRE, RELOAD, CHANGE_WEAPON };

private:
	WeaponInstance_st & GetSelectedWeapon();
	const WeaponType_st & GetSelectedWeaponType();

	uint8	m_OwnerPlayerId;

	std::vector<WeaponInstance_st>	m_Weapons;
	int32							m_SelectedWeapon;

	double		m_ActionBeginTime;
	double		m_dNextReadyTime;
	WpnAction	m_nAction;

	enum MuzzleFlashState { READY, VISIBLE, COOLDOWN } m_oMuzzleFlashState;

	static const WeaponType_st & GetWeaponType(uint8 WeaponTypeId);
};

struct WeaponInstance_st
{
	uint8		weapon_type_id;

	uint32		ammo;
	uint32		clips;
};

struct WpnCommand_st
{
	WeaponSystem::WpnAction	nAction;
	//float				fGameTime;
	double				dTime;
	union {
		float	fZ;
		uint8	WeaponNumber;
	} Parameter;

	double GetTimeAgo();
};

// initialize all weapons specs
void WeaponInitSpecs();

#endif // __WeaponSystem_H__
