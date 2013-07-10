#include "globals.h"

WeaponSpec_t	oWeaponSpecs[3];

// implementation of the player class
CWeapon::CWeapon()
	: m_oMuzzleFlashState(READY),
	  bReloading(false),
	  bReloadRequested(false),
	  dTimer(0.0)
{}

CWeapon::~CWeapon()
{
	// nothing to do here yet
}

void CWeapon::Tick()
{
//if (this->iOwnerID == 0) printf("dTimer: %f => ", dTimer);
	// TODO - The whole weapon timing/event scheduling (rate of fire, reloading, etc.) system needs to be reworked
	if (dTimer > 0.0) dTimer -= dTimePassed;
	else dTimer = 0.0;
//if (this->iOwnerID == 0) printf("%f\n", dTimer);
	if ((bReloading || (bReloadRequested && glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE))
		|| (bAutoReload && iClipAmmo <= 0 && dTimer <= 0 && glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE))
	{
		Reload();
	}

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		PlayerGet(iOwnerID)->bEmptyClicked = false;
}

void CWeapon::Fire()
{
	// can't fire when reloading
	if (bReloading || bReloadRequested)
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
		while (dTimer <= 0)
		{
			if (iClipAmmo > 0)
			{
				// play shot sound
				// ...
				iClipAmmo--;
				dTimer += oWeaponSpecs[iWhatWeapon].fROF;// * PARTICLE_TICK_TIME / fPlayerTickTime;

				// make a projectile particle
				oParticleEngine.AddParticle(PlayerGet(iOwnerID)->GetIntX(), PlayerGet(iOwnerID)->GetIntY(),
					Math::Sin(PlayerGet(iOwnerID)->GetZ()) * oWeaponSpecs[iWhatWeapon].fProjSpeed + PlayerGet(iOwnerID)->GetVelX(),
					Math::Cos(PlayerGet(iOwnerID)->GetZ()) * oWeaponSpecs[iWhatWeapon].fProjSpeed + PlayerGet(iOwnerID)->GetVelY(),
					oWeaponSpecs[iWhatWeapon].iWhatType, oWeaponSpecs[iWhatWeapon].fMaxDamage,
					oWeaponSpecs[iWhatWeapon].fMaxLife, iOwnerID);

				// Create a muzzle flash
				if (m_oMuzzleFlashState == READY)
					m_oMuzzleFlashState = VISIBLE;

				// inaccuracy
				// ... rotate the player
				// DEBUG: A temporary hack for recoil
				//PlayerGet(iOwnerID)->Rotate(((rand() % 1000)/1000.0f * 0.25f - 0.125) * oWeaponSpecs[iWhatWeapon].fInaccuracy);
			}
			else
			{
				if (PlayerGet(iOwnerID)->bEmptyClicked == false) {
					// play dry fire sound
					// ...
					PlayerGet(iOwnerID)->bEmptyClicked = true;
				}
				break;
			}
		}
		break;
	default:
		break;
	}
}

void CWeapon::Reload()
{
	if (bReloading)
	// continue reloading
	{
		if (dTimer <= 0)
		// end reloading
		{
			bReloading = false;
			iClipAmmo = oWeaponSpecs[iWhatWeapon].iClipAmmo;
		}
	}
	else if (bReloadRequested && dTimer <= 0)
	// start reloading
	{
		bReloadRequested = false;
		bReloading = true;

		// play reloading sound
		// ...
		dTimer += oWeaponSpecs[iWhatWeapon].fReloadTime;
		--iClips;
	}
}

void CWeapon::Render()
{
	// Render the Muzzle Flash
	if (m_oMuzzleFlashState == VISIBLE || glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		m_oMuzzleFlashState = COOLDOWN;

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, oTextureIDs.nM4A1MuzzleFlash);

		glColor4d(1, 1, 0.75, 0.9);
		glBegin(GL_QUADS);
			glTexCoord2d(0.5, (1.0+2*85)/256); glVertex2i( 42.5 * 0.75, 118 * 0.75);
			glTexCoord2d(0.5, (1.0+3*85)/256); glVertex2i(-42.5 * 0.75, 118 * 0.75);
			glTexCoord2d(0, (1.0+3*85)/256); glVertex2f(-42.5 * 0.75, -10 * 0.5);
			glTexCoord2d(0, (1.0+2*85)/256); glVertex2f( 42.5 * 0.75, -10 * 0.5);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (m_oMuzzleFlashState == COOLDOWN)
		m_oMuzzleFlashState = READY;
}

void CWeapon::StartReloading()
{
	if (!bReloading && iClipAmmo < oWeaponSpecs[iWhatWeapon].iClipAmmo && iClips > 0) {
		bReloadRequested = true;
	}
}

bool CWeapon::IsReloading()
{
	return bReloading;
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
	oWeaponSpecs[1].iClips = 6;
	oWeaponSpecs[1].iMaxClips = 6;
	oWeaponSpecs[1].iClipAmmo = 30;
	oWeaponSpecs[1].fROF = 0.100;
	oWeaponSpecs[1].fReloadTime = 2.030;
	oWeaponSpecs[1].fProjSpeed = 125;//85;
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
