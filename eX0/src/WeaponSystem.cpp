// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

WeaponSpec_st	oWeaponSpecs[5];
std::vector<WeaponType_st>		weapon_types;

double WpnCommand_st::GetTimeAgo()
{
	/*eX0_assert(fGameTime >= 0 && fGameTime < 256, "GetTimeAgo(): fGameTime >= 0 && fGameTime < 256");

	double dTimeAgo = g_pGameSession->LogicTimer().GetGameTime() - fGameTime;
	dTimeAgo *= 1.0 / g_cCommandRate;
	//if (dTimeAgo > 0) dTimeAgo = 0;		// TODO: Also make this more flexible... i.e. allow small values over zero
										//       It's mostly to prevent overflow (a really old WpnCommand thought to be one way in the future...
										//       ruins the player's ability to shoot for the next 3 seconds), nothing more

	return dTimeAgo;*/
	return g_pGameSession->LogicTimer().GetTime() - dTime;
}

// implementation of the player class
WeaponSystem::WeaponSystem(uint8 OwnerPlayerId)
	: m_OwnerPlayerId(OwnerPlayerId),
	  m_Weapons(),
	  m_SelectedWeapon(0),
	  m_ActionBeginTime(0.0),
	  m_dNextReadyTime(0.0),
	  m_nAction(IDLE),
	  m_oMuzzleFlashState(READY)
{
}

WeaponSystem::~WeaponSystem()
{
}

void WeaponSystem::Reset()
{
	m_SelectedWeapon = 0;
	m_Weapons.clear();

	m_ActionBeginTime = 0.0;
	m_dNextReadyTime = 0.0;
	m_nAction = IDLE;

	m_oMuzzleFlashState = READY;
}

void WeaponSystem::AddWeapon(uint8 WeaponTypeId)
{
	// Create a new Weapon
	WeaponInstance_st Weapon;
	Weapon.weapon_type_id = WeaponTypeId;
	Weapon.ammo = GetWeaponType(WeaponTypeId).max_ammo;
	Weapon.clips = GetWeaponType(WeaponTypeId).max_clips;

	m_Weapons.push_back(Weapon);
}

void WeaponSystem::ClearWeapons()
{
	m_Weapons.clear();
}

WeaponInstance_st & WeaponSystem::GetSelectedWeapon()
{
	return m_Weapons.at(m_SelectedWeapon);
}

const WeaponType_st & WeaponSystem::GetSelectedWeaponType()
{
	return GetWeaponType(GetSelectedWeapon().weapon_type_id);
}

bool WeaponSystem::IsCommandOutdated(WpnCommand_st & oWpnCommand, bool bPushForwardCorrectionEnabled)
{
	// TODO: Make this function slightly more forgiving.
	//       i.e. If the WpnCommand happened just slightly before NextReadyTime, you should still accept it,
	//            just correct its TimeAgo to match the NextReadyTime...
	//       Or something like that.
	//return (g_pGameSession->LogicTimer().GetTime() - oWpnCommand.GetTimeAgo() < m_dNextReadyTime);
	//if (&pLocalPlayer->GetSelWeaponTEST() != this && (oWpnCommand.dTime < m_dNextReadyTime)) printf("%f: %.20f diff\n", oWpnCommand.dTime, m_dNextReadyTime - oWpnCommand.dTime);
	//eX0_assert(&pLocalPlayer->GetSelWeaponTEST() == this || !(oWpnCommand.dTime < m_dNextReadyTime), "command outdated");

	//bool CommandOutdated = (oWpnCommand.dTime < m_dNextReadyTime && !(oWpnCommand.dTime >= m_ActionBeginTime && CHANGE_WEAPON == oWpnCommand.nAction));
	bool CommandNotOutdatedA = (oWpnCommand.dTime >= m_dNextReadyTime);
	bool CommandNotOutdatedB = (oWpnCommand.dTime >= m_ActionBeginTime && CHANGE_WEAPON == oWpnCommand.nAction);
	bool CommandNotOutdated = (CommandNotOutdatedA || CommandNotOutdatedB);

	/* TODO: Make this work for both types of outdated commands, not just the first
	if (bPushForwardCorrectionEnabled && oWpnCommand.dTime < m_dNextReadyTime && m_dNextReadyTime - oWpnCommand.dTime <= 0.001) {
		printf("Command %d was outdated by %.20f, fixed.\n", oWpnCommand.nAction, m_dNextReadyTime - oWpnCommand.dTime);
		oWpnCommand.dTime = m_dNextReadyTime;
		return false;
	}*/

	return !CommandNotOutdated;
}

bool WeaponSystem::IsReadyForNextCommand(WpnCommand_st & oWpnCommand)
{
	bool ReadyA = (m_dNextReadyTime <= g_pGameSession->LogicTimer().GetTime());
	bool ReadyB = (m_ActionBeginTime <= g_pGameSession->LogicTimer().GetTime() && CHANGE_WEAPON == oWpnCommand.nAction);

	return (ReadyA || ReadyB);
}

void WeaponSystem::PreprocessWpnCommand(WpnCommand_st & oWpnCommand)
{
#if 0	// VAIO change
	if (IDLE == oWpnCommand.nAction)
	{
		m_nAction = IDLE;
		m_dNextReadyTime = oWpnCommand.dTime;
	}
	else if (FIRE == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = m_dNextReadyTime;	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
													//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		switch(GetSelectedWeaponType().projectile_type_id)
		{
		// the gun fires "blanks" - ignore
		case 0:
			break;
		// bullets
		case CParticle::BULLET:
		// bouncy bullets
		case CParticle::BOUNCY_BULLET:
			//while (dTimer <= 0)
			//while (m_dNextReadyTime <= g_pGameSession->LogicTimer().GetTime())
			{
				if (GetSelectedWeapon().ammo > 0)
				{
					m_nAction = FIRE;
					bImportantCommand = true;

					// play shot sound
					// ...
					m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().rate_of_fire;
					--GetSelectedWeapon().ammo;

					// make a projectile particle
					State_st oState = PlayerGet(m_OwnerPlayerId)->GetStateAtTime(oWpnCommand.dTime);
					//printf("firing from %f, %f at %.20f\n", oState.fX, oState.fY, oWpnCommand.dTime);
					float fProjSpeed = 275;
					float fMaxDamage = 40;
					float fMaxLife = 1000;
					if (GetSelectedWeaponType().projectile_type_id == CParticle::BOUNCY_BULLET) {
						fProjSpeed = 10;
						fMaxDamage = 25;
						fMaxLife = 3.0;
					}
					oParticleEngine.AddParticle(oWpnCommand.dTime, oState.fX, oState.fY,
						// DEBUG: I will need to re-add player's velocity to the equation eventually...
						Math::Sin(oWpnCommand.Parameter.fZ) * fProjSpeed/* + PlayerGet(iOwnerID)->GetVelX()*/,
						Math::Cos(oWpnCommand.Parameter.fZ) * fProjSpeed/* + PlayerGet(iOwnerID)->GetVelY()*/,
						GetSelectedWeaponType().projectile_type_id, fMaxDamage,
						fMaxLife, m_OwnerPlayerId);

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
					/*if (PlayerGet(iOwnerID)->bEmptyClicked == false) {
						// play dry fire sound
						// ...
						PlayerGet(iOwnerID)->bEmptyClicked = true;
					}*/
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	else if (RELOAD == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = m_dNextReadyTime;	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
													//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		// Can reload
		if (GetSelectedWeapon().ammo < GetSelectedWeaponType().max_ammo && GetSelectedWeapon().clips > 0)
		{
			m_nAction = RELOAD;
			bImportantCommand = true;

			// Start reloading
			m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().reload_time;
		}
		else
		{
			// CONTINUE: Here \/
			// TODO: This is going in the right direction, but needs to be redone properly (right now, the results are bad;
			//       for example, if you hold down reload, no end-of-reload idle packet is sent)
			m_nAction = IDLE;
			m_dNextReadyTime = oWpnCommand.dTime;
		}
	}
	else if (CHANGE_WEAPON == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = std::min<double>(oWpnCommand.dTime, m_dNextReadyTime);	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
																						//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		// Can change the selected weapon
		if (oWpnCommand.Parameter.WeaponNumber != m_SelectedWeapon)
		{
			m_nAction = CHANGE_WEAPON;
			bImportantCommand = true;

			m_SelectedWeapon = oWpnCommand.Parameter.WeaponNumber;
			m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().bring_out_time;
		}
	}
#endif
}

bool WeaponSystem::ProcessWpnCommand(WpnCommand_st & oWpnCommand)
{
	bool bImportantCommand = false;

	eX0_assert(IsReadyForNextCommand(oWpnCommand), "IsReadyForNextCommand(oWpnCommand)");

	// Finish reloading
	if (RELOAD == m_nAction)
	{
		if (CHANGE_WEAPON != oWpnCommand.nAction)
		{
#if 0	// VAIO change
			if (IDLE == oWpnCommand.nAction) {
				bImportantCommand = true;
				printf("did bImportantCommand = true; for idle command (%d) probly [reload]\n", (int)oWpnCommand.nAction);
				eX0_assert((int)oWpnCommand.nAction == 0, "did bImportantCommand = true; for idle command [reload]");
			}
#endif

			// Perform the actual Reload action
			--GetSelectedWeapon().clips;
			GetSelectedWeapon().ammo = GetSelectedWeaponType().max_ammo;
		}
	}
#if 0	// VAIO change
	// Finish changing weapon
	else if (CHANGE_WEAPON == m_nAction)
	{
		if (CHANGE_WEAPON != oWpnCommand.nAction)
		{
			if (IDLE == oWpnCommand.nAction) {
				bImportantCommand = true;
				printf("did bImportantCommand = true; for idle command (%d) probly [ch wpn]\n", (int)oWpnCommand.nAction);
				eX0_assert((int)oWpnCommand.nAction == 0, "did bImportantCommand = true; for idle command [ch wpn]");
			}
		}
	}
#endif

	if (IDLE == oWpnCommand.nAction)
	{
		m_nAction = IDLE;
		m_dNextReadyTime = oWpnCommand.dTime;
	}
	else if (FIRE == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = m_dNextReadyTime;	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
													//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		switch(GetSelectedWeaponType().projectile_type_id)
		{
		// the gun fires "blanks" - ignore
		case 0:
			break;
		// bullets
		case CParticle::BULLET:
		// bouncy bullets
		case CParticle::BOUNCY_BULLET:
			//while (dTimer <= 0)
			//while (m_dNextReadyTime <= g_pGameSession->LogicTimer().GetTime())
			{
				if (GetSelectedWeapon().ammo > 0)
				{
					m_nAction = FIRE;
					bImportantCommand = true;

					// play shot sound
					// ...
					m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().rate_of_fire;
					--GetSelectedWeapon().ammo;

					// make a projectile particle
					State_st oState = PlayerGet(m_OwnerPlayerId)->GetStateAtTime(oWpnCommand.dTime);
					//printf("firing from %f, %f at %.20f\n", oState.fX, oState.fY, oWpnCommand.dTime);
					float fProjSpeed = 275;
					float fMaxDamage = 40;
					float fMaxLife = 1000;
					if (GetSelectedWeaponType().projectile_type_id == CParticle::BOUNCY_BULLET) {
						fProjSpeed = 10;
						fMaxDamage = 25;
						fMaxLife = 3.0;
					}
					oParticleEngine.AddParticle(oWpnCommand.dTime, oState.fX, oState.fY,
						// DEBUG: I will need to re-add player's velocity to the equation eventually...
						Math::Sin(oWpnCommand.Parameter.fZ) * fProjSpeed/* + PlayerGet(iOwnerID)->GetVelX()*/,
						Math::Cos(oWpnCommand.Parameter.fZ) * fProjSpeed/* + PlayerGet(iOwnerID)->GetVelY()*/,
						GetSelectedWeaponType().projectile_type_id, fMaxDamage,
						fMaxLife, m_OwnerPlayerId);

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
					/*if (PlayerGet(iOwnerID)->bEmptyClicked == false) {
						// play dry fire sound
						// ...
						PlayerGet(iOwnerID)->bEmptyClicked = true;
					}*/
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	else if (RELOAD == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = m_dNextReadyTime;	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
													//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		// Can reload
		if (GetSelectedWeapon().ammo < GetSelectedWeaponType().max_ammo && GetSelectedWeapon().clips > 0)
		{
			m_nAction = RELOAD;
			bImportantCommand = true;

			// Start reloading
			m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().reload_time;
		}
#if 0	// VAIO change
		else
		{
			// CONTINUE: Here \/
			// TODO: This is going in the right direction, but needs to be redone properly (right now, the results are bad;
			//       for example, if you hold down reload, no end-of-reload idle packet is sent)
			m_nAction = IDLE;
			m_dNextReadyTime = oWpnCommand.dTime;
		}
#endif
	}
	else if (CHANGE_WEAPON == oWpnCommand.nAction)
	{
		if (IDLE == m_nAction)
		{
			m_dNextReadyTime = oWpnCommand.dTime;
		}
		else if (FIRE == m_nAction || RELOAD == m_nAction || CHANGE_WEAPON == m_nAction)
		{
			oWpnCommand.dTime = std::min<double>(oWpnCommand.dTime, m_dNextReadyTime);	// TEST: See if this is the right way of doing it... It should only happen on local-controller type of players
																						//       I guess it won't happen on non-local-controller players because of the automatic Idle wpn commands inserted
		}

		// Can change the selected weapon
		if (oWpnCommand.Parameter.WeaponNumber != m_SelectedWeapon)
		{
			m_nAction = CHANGE_WEAPON;
			bImportantCommand = true;

			m_SelectedWeapon = oWpnCommand.Parameter.WeaponNumber;
			m_dNextReadyTime = oWpnCommand.dTime + GetSelectedWeaponType().bring_out_time;
		}
	}

	m_ActionBeginTime = oWpnCommand.dTime;

	return bImportantCommand;
}

void WeaponSystem::Render()
{
#ifdef EX0_CLIENT
	// Draw the gun
	glPushMatrix();
	double GunLength = GetSelectedWeapon().weapon_type_id == 1 ? 10 : 8;
	if (IsReloading()) { glRotated(30, 0, 0, -1); GunLength *= 0.75; }
	if (-1 == ChangingWeaponTo())
	{
		glBegin(GL_QUADS);
			glVertex2d(-1, 3 + GunLength);
			glVertex2d(-1, 3 - GunLength > 8 ? 1 : 0);
			glVertex2d(1, 3 - GunLength > 8 ? 1 : 0);
			glVertex2d(1, 3 + GunLength);
		glEnd();

		// Draw the aiming-guide
		if (pLocalPlayer == PlayerGet(m_OwnerPlayerId) && !IsReloading()) {
			OglUtilsSetMaskingMode(WITH_MASKING_MODE);
			glLineWidth(1.0);
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
				glColor4f(0.9f, 0.2f, 0.1f, 0.5f);
				glVertex2d(0, 3 + GunLength);
				glColor4f(0.9f, 0.2f, 0.1f, 0.0f);
				glVertex2i(0, 900);
			glEnd();
			glShadeModel(GL_FLAT);
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glLineWidth(2.0);
		}
	}
	glPopMatrix();

	// Render the Muzzle Flash
	if (m_oMuzzleFlashState == VISIBLE)
	{
		m_oMuzzleFlashState = COOLDOWN;

		if (pLocalPlayer == PlayerGet(m_OwnerPlayerId))
			OglUtilsSetMaskingMode(NO_MASKING_MODE);
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, oTextureIDs.nM4A1MuzzleFlash);

		int nVariation = rand() % 6;
		glColor4d(0.9, 1, 0.75, 0.9);
		glBegin(GL_QUADS);
			glTexCoord2d(0.5 * (nVariation%2+1), (256.0-(nVariation/2+1)*85) / 256); glVertex2d( 42.5 * 0.75, 121 * 0.75);
			glTexCoord2d(0.5 * (nVariation%2+1), (256.0-(nVariation/2)*85) / 256); glVertex2d(-42.5 * 0.75, 121 * 0.75);
			glTexCoord2d(0.5 * (nVariation%2), (256.0-(nVariation/2)*85) / 256); glVertex2d(-42.5 * 0.75, -7 * 0.75);
			glTexCoord2d(0.5 * (nVariation%2), (256.0-(nVariation/2+1)*85) / 256); glVertex2d( 42.5 * 0.75, -7 * 0.75);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (m_oMuzzleFlashState == COOLDOWN)
		m_oMuzzleFlashState = READY;
#endif // EX0_CLIENT
}

uint32 WeaponSystem::GetAmmo()
{
	return GetSelectedWeapon().ammo;
}

uint32 WeaponSystem::GetClips()
{
	return GetSelectedWeapon().clips;
}

bool WeaponSystem::IsReloading()
{
	return (RELOAD == m_nAction);
}

int32 WeaponSystem::ChangingWeaponTo()
{
	return (CHANGE_WEAPON == m_nAction ? m_SelectedWeapon : -1);
}

void WeaponSystem::GiveClip()
{
#if 0
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
#endif // 0

	/* TODO: Make this network-enabled
	if (GetSelectedWeapon().clips < GetSelectedWeaponType().max_clips)
		++GetSelectedWeapon().clips;*/
}

const WeaponType_st & WeaponSystem::GetWeaponType(uint8 WeaponTypeId)
{
	eX0_assert(WeaponTypeId == weapon_types.at(WeaponTypeId - 1).id, "WeaponTypeId == weapon_types.at(WeaponTypeId - 1).id");

	return weapon_types.at(WeaponTypeId - 1);
}

// initialize all weapons specs
void WeaponInitSpecs()
{
	// First New Test M4-like Gun
	WeaponType_st weapon;
	weapon.id = 1;
	weapon.name = "First New Test M4-like Gun";
	weapon.projectile_type_id = CParticle::BULLET;
	weapon.max_ammo = 31;
	weapon.max_clips = 6;
	weapon.rate_of_fire = 0.1;
	weapon.reload_time = 2.030;
	weapon.bring_out_time = 1.0;
	weapon_types.push_back(weapon);

	// test bouncy bullet/grenade gun
	WeaponType_st Weapon;
	Weapon.id = 2;
	Weapon.name = "test gun";
	Weapon.projectile_type_id = CParticle::BOUNCY_BULLET;
	Weapon.max_ammo = 500;
	Weapon.max_clips = 2;
	Weapon.rate_of_fire = 1.5;
	Weapon.reload_time = 3;
	Weapon.bring_out_time = 0.75;
	weapon_types.push_back(Weapon);

	/////////////////////////////////////

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
	oWeaponSpecs[1].fROF = 0.100f;
	oWeaponSpecs[1].fReloadTime = 2.030f;
	oWeaponSpecs[1].fProjSpeed = 275;//125;//85;
	oWeaponSpecs[1].fInaccuracy = 0.15f;
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

	// non-auto gun TEST
	oWeaponSpecs[3].sName = "non-auto gun (to be)";
	oWeaponSpecs[3].iWhatType = CParticle::BULLET;		// bullets
	oWeaponSpecs[3].iClips = 6;
	oWeaponSpecs[3].iMaxClips = 6;
	oWeaponSpecs[3].iClipAmmo = 70;
	oWeaponSpecs[3].fROF = 1.300f;
	oWeaponSpecs[3].fReloadTime = 2.540f;
	oWeaponSpecs[3].fProjSpeed = 155;
	oWeaponSpecs[3].fInaccuracy = 0.15f;
	oWeaponSpecs[3].fMaxDamage = 180;
	oWeaponSpecs[3].fMaxLife = 1000;

	// networked gun TEST
	oWeaponSpecs[4].sName = "network test gun";
	oWeaponSpecs[4].iWhatType = CParticle::BOUNCY_BULLET;
	oWeaponSpecs[4].iClips = 1;
	oWeaponSpecs[4].iMaxClips = 1;
	oWeaponSpecs[4].iClipAmmo = 250;
	oWeaponSpecs[4].fROF = 1.000f;
	oWeaponSpecs[4].fReloadTime = 2.000f;
	oWeaponSpecs[4].fProjSpeed = 10;
	oWeaponSpecs[4].fInaccuracy = 0.0f;
	oWeaponSpecs[4].fMaxDamage = 10;
	oWeaponSpecs[4].fMaxLife = 3;
}
