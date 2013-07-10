#include "globals.h"

PlayerInputListener::PlayerInputListener()
	: InputListener(),
	  m_dForwardAxis(0),
	  m_dStrafeAxis(0),
	  m_dRotationAxis(0),
	  m_dStealthHalfAxis(0),
	  m_dForwardAxisState(0),
	  m_dStrafeAxisState(0),
	  m_dRotationAxisState(0),
	  m_dStealthHalfAxisState(0)
{
}

PlayerInputListener::~PlayerInputListener()
{
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
	if (m_dStealthHalfAxis >= 1.0) {
		--m_dStealthHalfAxis;
		return true;
	} else
		return false;
}

double PlayerInputListener::GetRotationAmount()
{
	double dRotationAmount = m_dRotationAxis;
	m_dRotationAxis = 0;
	return dRotationAmount;
}

bool PlayerInputListener::ProcessButton(int nDevice, int nButton, bool bPressed)
{
	if (nDevice == 0)		// Keyboard
	{
		switch (nButton)
		{
		case 'W':
			// DEBUG: Test only, create ControlButton/Axis/etc. classes and use 'em
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
			// DEBUG: Can't access local player directly... this has to be general and work for any player
			pLocalPlayer->bWeaponFireTEST = bPressed;
			break;
		default:
			return false;
			break;
		}

		return true;
	}

	return false;
}

bool PlayerInputListener::ProcessSlider(int nDevice, int nSlider, double dMovedAmount)
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
			if (dMovedAmount <= 0)
			{
				// DEBUG: Can't access local player directly... this has to be general and work for any player
				pLocalPlayer->iSelWeapon = ++pLocalPlayer->iSelWeapon % 4;
			}
			else
			{
				// DEBUG: Can't access local player directly... this has to be general and work for any player
				pLocalPlayer->iSelWeapon = (--pLocalPlayer->iSelWeapon + 4) % 4;
			}
		}
	}

	return false;
}

bool PlayerInputListener::ProcessAxis(int nDevice, int nAxis, double dPosition)
{
	if (nDevice == 2000)		// 1st Joystick
	{
		if (nAxis == 0)			// 1st Axis
		{
			m_dRotationAxisState = dPosition / 10.0;
			//m_dStrafeAxisState = dPosition;
		}
		else if (nAxis == 1)	// 2nd Axis
		{
			m_dForwardAxisState = dPosition;
		}
		else
			return false;

		return true;
	}

	return false;
}

void PlayerInputListener::TimePassed(double dTimePassed)
{
	m_dForwardAxis += m_dForwardAxisState * dTimePassed * 20;
	m_dStrafeAxis += m_dStrafeAxisState * dTimePassed * 20;
	m_dRotationAxis += m_dRotationAxisState * dTimePassed * 20;
	m_dStealthHalfAxis += m_dStealthHalfAxisState * dTimePassed * 20;
}

void PlayerInputListener::Reset()
{
	m_dForwardAxis = 0;
	m_dStrafeAxis = 0;
	m_dRotationAxis = 0;
	m_dStealthHalfAxis = 0;
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
