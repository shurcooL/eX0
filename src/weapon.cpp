#include "globals.h"

WeaponSpec_t	oWeaponSpecs[3];

// implementation of the player class
CWeapon::CWeapon()
{
	// init vars
	bReloading = false;
	bShouldReload = false;
	fTimer = 0.0f;
}

CWeapon::~CWeapon()
{
	// nothing to do here yet
}

void CWeapon::Tick()
{
//if (this->iOwnerID == 0) printf("fTimer: %f => ", fTimer);
	// TODO - The whole weapon timing/event scheduling (rate of fire, reloading, etc.) system needs to be reworked
	if (fTimer > 0.0f) fTimer -= fTimePassed;
//if (this->iOwnerID == 0) printf("%f\n", fTimer);
	if ((bReloading || bShouldReload) || (bAutoReload && iClipAmmo <= 0 && fTimer <= 0
	  && glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE))
	{
		Reload();
	}

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		oPlayers[iOwnerID]->bEmptyClicked = false;
}

void CWeapon::GiveClip()
{
	if (iClips < oWeaponSpecs[iWhatWeapon].iMaxClips)
	{
		if (bAutoReload && iClips == 0 && iClipAmmo == 0 && glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			// play buy ammo sound
			// ...
			iClips++;
			Reload();
		}
		else
		{
			// play buy ammo sound
			// ...
			iClips++;
		}
	}
}

void CWeapon::Init(int iWhoIsOwner, int iWeapon)
{
	iOwnerID = iWhoIsOwner;
	iWhatWeapon = iWeapon;
	iClips = oWeaponSpecs[iWhatWeapon].iClips;
	iClipAmmo = oWeaponSpecs[iWhatWeapon].iClipAmmo;
}

void CWeapon::Fire()
{
	// can't fire when reloading
	if (bReloading || bShouldReload)
		return;

	switch(oWeaponSpecs[iWhatWeapon].iWhatType)
	{
	// the gun fires "blanks" - ignore
	case 0:
		break;
	// bullets
	case 1:
	// bouncy bullets
	case 2:
		while (fTimer <= 0)
		{
			if (iClipAmmo > 0)
			{
				// play shot sound
				// ...
				iClipAmmo--;
				fTimer += oWeaponSpecs[iWhatWeapon].fROF * PARTICLE_TICK_TIME / PLAYER_TICK_TIME;

				// make a projectile particle
				oParticleEngine.AddParticle(oPlayers[iOwnerID]->GetIntX(), oPlayers[iOwnerID]->GetIntY(),
					Math::Sin(oPlayers[iOwnerID]->GetZ()) * oWeaponSpecs[iWhatWeapon].fProjSpeed + oPlayers[iOwnerID]->GetVelX(),
					Math::Cos(oPlayers[iOwnerID]->GetZ()) * oWeaponSpecs[iWhatWeapon].fProjSpeed + oPlayers[iOwnerID]->GetVelY(),
					oWeaponSpecs[iWhatWeapon].iWhatType, oWeaponSpecs[iWhatWeapon].fMaxDamage,
					oWeaponSpecs[iWhatWeapon].fMaxLife, iOwnerID);

				// inaccuracy
				// ... rotate the player
				// DEBUG: A temporary hack for recoil
				//oPlayers[iOwnerID]->Rotate(((rand() % 1000)/1000.0f * 0.25f - 0.125) * oWeaponSpecs[iWhatWeapon].fInaccuracy);
			}
			else if (iClipAmmo <= 0)
			{
				if (!oPlayers[iOwnerID]->bEmptyClicked)
				{
					// play dry fire sound
					// ...
					oPlayers[iOwnerID]->bEmptyClicked = true;
				}

				fTimer = 0.0;
				break;
			}
		}
		break;
	default:
		break;
	}
}

bool CWeapon::IsReloading()
{
	return bReloading;
}

void CWeapon::Reload()
{
	if (bReloading)
	// continue reloading
	{
		if (fTimer <= 0)
		// end reloading
		{
			bReloading = false;
			iClipAmmo = oWeaponSpecs[iWhatWeapon].iClipAmmo;
		}
	}
	else if (iClipAmmo < oWeaponSpecs[iWhatWeapon].iClipAmmo && iClips > 0)
	// start reloading
	{
		if (fTimer <= 0)
		// we can start right now
		{
			// play reloading sound
			// ...
			fTimer += oWeaponSpecs[iWhatWeapon].fReloadTime;
			iClips--;
			bReloading = true;
			bShouldReload = false;
		}
		else
			// we'll start later
			bShouldReload = true;
	}
}

int CWeapon::GetClips()
{
	return iClips;
}

int CWeapon::GetClipAmmo()
{
	return iClipAmmo;
}

// initialize all weapons specs
void WeaponInitSpecs()
{
	// no weapon - a dummy
	oWeaponSpecs[0].sName = "";
	oWeaponSpecs[0].iWhatType = 0;		// a dummy
	oWeaponSpecs[0].iClips = 0;
	oWeaponSpecs[0].iMaxClips = 0;
	oWeaponSpecs[0].iClipAmmo = 0;
	oWeaponSpecs[0].fROF = 0;
	oWeaponSpecs[0].fReloadTime = 0;
	oWeaponSpecs[0].fProjSpeed = 0;
	oWeaponSpecs[0].fInaccuracy = 0;
	oWeaponSpecs[0].fMaxDamage = 0;
	oWeaponSpecs[0].fMaxLife = 0;

	// silenced m4a1
	oWeaponSpecs[1].sName = "silenced m4a1";
	oWeaponSpecs[1].iWhatType = CParticle::BULLET;		// bullets
	oWeaponSpecs[1].iClips = 4;
	oWeaponSpecs[1].iMaxClips = 6;
	oWeaponSpecs[1].iClipAmmo = 30;
	oWeaponSpecs[1].fROF = 0.100;
	oWeaponSpecs[1].fReloadTime = 2.030;
	oWeaponSpecs[1].fProjSpeed = 85;
	oWeaponSpecs[1].fInaccuracy = 0.15;
	oWeaponSpecs[1].fMaxDamage = 40;
	oWeaponSpecs[1].fMaxLife = 1000;

	// test bouncy bullet/grenade gun
	oWeaponSpecs[2].sName = "test gun";
	oWeaponSpecs[2].iWhatType = CParticle::BOUNCY_BULLET;		// bouncy bullets
	oWeaponSpecs[2].iClips = 2;
	oWeaponSpecs[2].iMaxClips = 2;
	oWeaponSpecs[2].iClipAmmo = 500;
	oWeaponSpecs[2].fROF = 1.5;
	oWeaponSpecs[2].fReloadTime = 2.0;
	oWeaponSpecs[2].fProjSpeed = 10;
	oWeaponSpecs[2].fInaccuracy = 0.0;
	oWeaponSpecs[2].fMaxDamage = 25;
	oWeaponSpecs[2].fMaxLife = 3.0;
}
