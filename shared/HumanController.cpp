// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

HumanController::HumanController(CPlayer & oPlayer)
	: PlayerController(oPlayer)
{
}

HumanController::~HumanController()
{
}

bool HumanController::RequestInput(u_char cSequenceNumber)
{
	// set stealth
	if (glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT))
	{
		m_oPlayer.SetStealth(true);
	}
	else
	{
		m_oPlayer.SetStealth(false);
	}

	// Rotate left/right using arrow keys
	if (glfwGetKey(GLFW_KEY_LEFT) && !glfwGetKey(GLFW_KEY_RIGHT))
		m_oPlayer.Rotate((float)dTimePassed * -1.5f);
	else if (glfwGetKey(GLFW_KEY_RIGHT) && !glfwGetKey(GLFW_KEY_LEFT))
		m_oPlayer.Rotate((float)dTimePassed * 1.5f);

	if (glfwGetKey('A') && !glfwGetKey('D'))
	{
		if ((glfwGetKey('W') && !glfwGetKey('S'))
			|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
		{
			m_oPlayer.MoveDirection(7);
		}
		else if ((glfwGetKey('S') && !glfwGetKey('W'))
				|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
		{
			m_oPlayer.MoveDirection(5);
		}
		else
		{
			m_oPlayer.MoveDirection(6);
		}
	}
	else if (glfwGetKey('D') && !glfwGetKey('A'))
	{
		if ((glfwGetKey('W') && !glfwGetKey('S'))
			|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
		{
			m_oPlayer.MoveDirection(1);
		}
		else if ((glfwGetKey('S') && !glfwGetKey('W'))
				|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
		{
			m_oPlayer.MoveDirection(3);
		}
		else
		{
			m_oPlayer.MoveDirection(2);
		}
	}
	else
	{
		if ((glfwGetKey('W') && !glfwGetKey('S'))
			|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
		{
			m_oPlayer.MoveDirection(0);
		}
		else if ((glfwGetKey('S') && !glfwGetKey('W'))
				|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
		{
			m_oPlayer.MoveDirection(4);
		}
		else
		{
			m_oPlayer.MoveDirection(-1);
		}
	}

	return false;
}
