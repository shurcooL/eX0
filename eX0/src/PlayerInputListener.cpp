#include "globals.h"

PlayerInputListener::PlayerInputListener(CPlayer & oPlayer)
	: InputListener(),
	  m_dForwardAxis(0),
	  m_dStrafeAxis(0),
	  m_dRotationAxis(0),
	  m_dStealthHalfAxis(0),
	  m_dForwardAxisState(0),
	  m_dStrafeAxisState(0),
	  m_dRotationAxisState(0),
	  m_dStealthHalfAxisState(0),
	  m_bWeaponFireTEST(false),
	  m_bWeaponReloadTEST(false),
	  m_WeaponChangeTEST(-1),
	  m_oPlayer(oPlayer)
{
}

PlayerInputListener::~PlayerInputListener()
{
}

Command_st PlayerInputListener::GetNextCommand()
{
	Command_st oCommand;

	glfwLockMutex(m_oInputMutex);
		// Set the inputs
		oCommand.cMoveDirection = GetMoveDirection();
		oCommand.bStealth = GetStealth();
		oCommand.fZ = m_oPlayer.GetZ() + static_cast<float>(m_dRotationAxis);		// Get the most up-to-date player Z value
	glfwUnlockMutex(m_oInputMutex);

	return oCommand;
}

// Input Mutex isn't needed here, because this function gets called in Main thread anyway
double PlayerInputListener::GetRotationAmount()
{
	double dLastRotationAmount = m_dRotationAxis;
	m_dRotationAxis = 0;

	return dLastRotationAmount;
}

WpnCommand_st PlayerInputListener::GetNextWpnCommand()
{
	WpnCommand_st oWpnCommand;

	glfwLockMutex(m_oInputMutex);
		if (-1 != m_WeaponChangeTEST) oWpnCommand.nAction = WeaponSystem::CHANGE_WEAPON;
		else if (m_bWeaponReloadTEST) oWpnCommand.nAction = WeaponSystem::RELOAD;
		else if (m_bWeaponFireTEST) oWpnCommand.nAction = WeaponSystem::FIRE;
		else oWpnCommand.nAction = WeaponSystem::IDLE;
		/*oWpnCommand.fGameTime = static_cast<float>(g_pGameSession->MainTimer().GetGameTime() - (WeaponSystem::FIRE == oWpnCommand.nAction ? 15 : 0));
		if (oWpnCommand.fGameTime < 0) oWpnCommand.fGameTime += 256;//TEST
		if (oWpnCommand.fGameTime >= 256) oWpnCommand.fGameTime -= 256;//TEST*/
		oWpnCommand.dTime = g_pGameSession->MainTimer().GetTime() - (/*WeaponSystem::FIRE == oWpnCommand.nAction*/false ? 1 : 0);
		if (WeaponSystem::FIRE == oWpnCommand.nAction) oWpnCommand.Parameter.fZ = m_oPlayer.GetZ() + static_cast<float>(m_dRotationAxis);		// Get the most up-to-date player Z value
		else if (WeaponSystem::CHANGE_WEAPON == oWpnCommand.nAction) oWpnCommand.Parameter.WeaponNumber = static_cast<uint8>(m_WeaponChangeTEST);

		//m_WeaponChangeTEST = -1;
	glfwUnlockMutex(m_oInputMutex);

	return oWpnCommand;
}

char PlayerInputListener::GetMoveDirection()
{
	char cMoveDirection;

	if (m_dStrafeAxis <= -1)	//m_bLeft && !m_bRight
	{
		++m_dStrafeAxis;
		if (m_dForwardAxis >= 1)	//m_bForward && !m_bBackward
		{
			--m_dForwardAxis;
			cMoveDirection = 7;
		}
		else if (m_dForwardAxis <= -1)	//m_bBackward && !m_bForward
		{
			++m_dForwardAxis;
			cMoveDirection = 5;
		}
		else
		{
			cMoveDirection = 6;
		}
	}
	else if (m_dStrafeAxis >= 1)	//m_bRight && !m_bLeft
	{
		--m_dStrafeAxis;
		if (m_dForwardAxis >= 1)	//m_bForward && !m_bBackward
		{
			--m_dForwardAxis;
			cMoveDirection = 1;
		}
		else if (m_dForwardAxis <= -1)	//m_bBackward && !m_bForward
		{
			++m_dForwardAxis;
			cMoveDirection = 3;
		}
		else
		{
			cMoveDirection = 2;
		}
	}
	else
	{
		if (m_dForwardAxis >= 1)	//m_bForward && !m_bBackward
		{
			--m_dForwardAxis;
			cMoveDirection = 0;
		}
		else if (m_dForwardAxis <= -1)	//m_bBackward && !m_bForward
		{
			++m_dForwardAxis;
			cMoveDirection = 4;
		}
		else
		{
			cMoveDirection = -1;
		}
	}

	return cMoveDirection;
}

bool PlayerInputListener::GetStealth()
{
	bool bStealth;

	if (m_dStealthHalfAxis >= 1.0) {
		--m_dStealthHalfAxis;
		bStealth = true;
	} else
		bStealth = false;

	return bStealth;
}

bool PlayerInputListener::MutexProcessButton(int nDevice, int nButton, bool bPressed)
{
	if (nDevice == 0)		// Keyboard
	{
		switch (nButton)
		{
		case 'W':
			// DEBUG: Test only, create ControlButton/Axis/etc. classes and use 'em
			// DEBUG: Also, using += instead of = poses a problem, because if PlayerInputListener is created while
			//        some buttons were pressed down (e.g. 'W'), the AxisState var should be set to appropriate value, not zero
			m_dForwardAxisState += (bPressed ? 1 : -1);
			break;
		case 'S':
			m_dForwardAxisState -= (bPressed ? 1 : -1);
			break;
		case 'A':
			m_dStrafeAxisState -= (bPressed ? 1 : -1);
			break;
		case 'D':
			m_dStrafeAxisState += (bPressed ? 1 : -1);
			break;
		case GLFW_KEY_LSHIFT:
			m_dStealthHalfAxisState += (bPressed ? 1 : -1);
			break;
		case GLFW_KEY_LEFT:
			m_dRotationAxisState -= (bPressed ? 1 : -1) / 10.0;
			break;
		case GLFW_KEY_RIGHT:
			m_dRotationAxisState += (bPressed ? 1 : -1) / 10.0;
			break;
		case 'R':
			m_bWeaponReloadTEST = (true == bPressed);
			break;
		case '1':
		case '2':
			if (true == bPressed)
				m_WeaponChangeTEST = nButton - '1';
			else
				m_WeaponChangeTEST = -1;
			break;
		default:
			return false;
			break;
		}

		return true;
	}
	else if (nDevice == 1000)		// Mouse
	{
		switch (nButton)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			m_bWeaponFireTEST = (true == bPressed);
			break;
		default:
			return false;
			break;
		}

		return true;
	}
	else if (nDevice == 2000)		// 1st Joystick
	{
		switch (nButton)
		{
		case 2:
			m_bWeaponReloadTEST = (true == bPressed);
			break;
		default:
			return false;
			break;
		}

		return true;
	}

	return false;
}

bool PlayerInputListener::MutexProcessSlider(int nDevice, int nSlider, double dMovedAmount)
{
	if (nDevice == 1000)		// Mouse
	{
		if (nSlider == 0)		// Mouse X Axis
		{
			float fRotationAmount = static_cast<float>(dMovedAmount) * fMouseSensitivity / 100.0f;// * (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 0.5f : 1.0f);
			m_dRotationAxis += fRotationAmount;

			return true;
		}
		else if (nSlider == 2)		// Mouse Z Axis
		{
			// DEBUG: Don't do this!! Very bad, what if dMovedAmount == 2 or 3? Need to loop this dMovedAmount number of times.
			/*if (dMovedAmount <= 0)
			{
				// DEBUG: Can't access local player directly... this has to be general and work for any player
				pLocalPlayer->iSelWeapon = ++pLocalPlayer->iSelWeapon % 4;
			}
			else
			{
				// DEBUG: Can't access local player directly... this has to be general and work for any player
				pLocalPlayer->iSelWeapon = (--pLocalPlayer->iSelWeapon + 4) % 4;
			}

			return true;*/
		}
	}

	return false;
}

bool PlayerInputListener::MutexProcessAxis(int nDevice, int nAxis, double dPosition)
{
	if (nDevice == 2000)		// 1st Joystick
	{
		if (nAxis == 0)			// 1st Axis
		{
			//m_dRotationAxisState = dPosition / 10.0;
			m_dStrafeAxisState = dPosition;
		}
		else if (nAxis == 1)	// 2nd Axis
		{
			m_dForwardAxisState = dPosition;
		}
		else if (nAxis == 2)	// 3rd Axis
		{
			m_bWeaponFireTEST = (dPosition <= -0.5);
		}
		else if (nAxis == 4)	// 5th Axis
		{
			m_dRotationAxisState = dPosition / 10.0;
		}
		else
			return false;

		return true;
	}

	return false;
}

void PlayerInputListener::MutexTimePassed(double dTimePassed)
{
	if (m_dForwardAxis * m_dForwardAxisState < 0) m_dForwardAxis = 0;
	if (m_dStrafeAxis * m_dStrafeAxisState < 0) m_dStrafeAxis = 0;

	m_dForwardAxis += m_dForwardAxisState * dTimePassed * g_cCommandRate;
	m_dStrafeAxis += m_dStrafeAxisState * dTimePassed * g_cCommandRate;
	m_dRotationAxis += m_dRotationAxisState * dTimePassed * g_cCommandRate;
	m_dStealthHalfAxis += m_dStealthHalfAxisState * dTimePassed * g_cCommandRate;
}

void PlayerInputListener::Reset()
{
	glfwLockMutex(m_oInputMutex);

	m_dForwardAxis = 0;
	m_dStrafeAxis = 0;
	m_dRotationAxis = 0;
	m_dStealthHalfAxis = 0;

	m_WeaponChangeTEST = -1;

	glfwUnlockMutex(m_oInputMutex);
}

/*
bool PlayerInputListener::ProcessMousePos(int nMousePosX, int nMousePosY)
{
	static bool bPreviousMouseXSet = false;
	if (bPreviousMouseXSet == false)
	{
printf("   mouse x = %d\n", nMousePosX);
		m_nPreviousMouseX = nMousePosX;
		bPreviousMouseXSet = true;
	}
	else
	{
		int nMouseMoved = nMousePosX - m_nPreviousMouseX;
		m_nPreviousMouseX = nMousePosX;

		float fRotationAmount = static_cast<float>(nMouseMoved) * fMouseSensitivity / 100.0f * (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 0.5f : 1.0f);
glfwLockMutex(oPlayerTick);
		// DEBUG: Fix it
		pLocalPlayer->Rotate(fRotationAmount);
glfwUnlockMutex(oPlayerTick);
	}

	return true;
}
*/
