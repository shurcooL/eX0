#include "globals.h"

InputManager * g_pInputManager = NULL;

InputManager * InputManager::m_pInstance = NULL;

InputManager::InputManager()
	: m_oListeners(),
	  m_oListenersMutex(glfwCreateMutex()),
	  m_bIsMousePointerVisible(true),
	  m_oJoysticks()
{
	eX0_assert(m_pInstance == NULL, "m_pInstance wasn't == NULL, means we have more than 1 instance of InputManager, not right");
	m_pInstance = this;

	SetGlfwCallbacks();

	InitializeJoysticks();
}

InputManager::~InputManager()
{
	RemoveGlfwCallbacks();

	ShowMouseCursor();

	glfwDestroyMutex(m_oListenersMutex);

	m_pInstance = nullptr;
}

void InputManager::RegisterListener(InputListener * pListener)
{
	glfwLockMutex(m_oListenersMutex);

	m_oListeners.push_back(pListener);

	glfwUnlockMutex(m_oListenersMutex);
}

void InputManager::UnregisterListener(InputListener * pListener)
{
	glfwLockMutex(m_oListenersMutex);

	for (std::vector<InputListener *>::const_iterator it1 = m_oListeners.begin(); it1 != m_oListeners.end(); ++it1)
		if ((*it1) == pListener)
		{
			m_oListeners.erase(it1);
			break;
		}

	glfwUnlockMutex(m_oListenersMutex);
}

void InputManager::ShowMouseCursor()
{
	m_bIsMousePointerVisible = true;
	glfwEnable(GLFW_MOUSE_CURSOR);
}

void InputManager::HideMouseCursor()
{
	m_bIsMousePointerVisible = false;
	glfwDisable(GLFW_MOUSE_CURSOR);
}

bool InputManager::IsMousePointerVisible()
{
	return (true == m_bIsMousePointerVisible || false == glfwGetWindowParam(GLFW_ACTIVE));
}

void InputManager::SetGlfwCallbacks()
{
	glfwSetKeyCallback(&InputManager::ProcessKey);
	glfwSetCharCallback(&InputManager::ProcessChar);
	glfwSetMouseButtonCallback(&InputManager::ProcessMouseButton);
	glfwSetMousePosCallback(&InputManager::ProcessMousePos);
	glfwSetMouseWheelCallback(&InputManager::ProcessMouseWheel);
}

void InputManager::RemoveGlfwCallbacks()
{
	glfwSetKeyCallback(NULL);
	glfwSetCharCallback(NULL);
	glfwSetMouseButtonCallback(NULL);
	glfwSetMousePosCallback(NULL);
	glfwSetMouseWheelCallback(NULL);
}

void InputManager::InitializeJoysticks()
{
	for (u_int nJoystick = GLFW_JOYSTICK_1; nJoystick <= GLFW_JOYSTICK_LAST; ++nJoystick)
	{
		if (GL_TRUE == glfwGetJoystickParam(nJoystick, GLFW_PRESENT))
		{
			Joystick_t oJoystick;
			oJoystick.nId = nJoystick;
			oJoystick.nAxes = glfwGetJoystickParam(nJoystick, GLFW_AXES);
			oJoystick.nButtons = glfwGetJoystickParam(nJoystick, GLFW_BUTTONS);

			m_oJoysticks.push_back(oJoystick);

			printf("Joystick id=%d: %d axes, %d buttons.\n", oJoystick.nId, oJoystick.nAxes, oJoystick.nButtons);
		}
	}

	printf("%d joysticks initialized.\n", m_oJoysticks.size());
}

void GLFWCALL InputManager::ProcessKey(int nKey, int nAction)
{
	//printf("  ProcessKey %d %d\n", nKey, nAction);

	glfwLockMutex(m_pInstance->m_oListenersMutex);

	for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
	{
		(*it1)->ProcessButton(0 /* Keyboard */, nKey, (GLFW_PRESS == nAction));
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);

	// DEBUG: Hack, remove old behaviour eventually
	InputProcessKey(nKey, nAction);
}

void GLFWCALL InputManager::ProcessChar(int nChar, int nAction)
{
	//printf("  ProcessChar %d %d\n", nChar, nAction);

	glfwLockMutex(m_pInstance->m_oListenersMutex);

	for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
	{
		(*it1)->ProcessCharacter(nChar, (GLFW_PRESS == nAction));
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);

	// DEBUG: Hack, remove old behaviour eventually
	InputProcessChar(nChar, nAction);	
}

void GLFWCALL InputManager::ProcessMouseButton(int nMouseButton, int nAction)
{
	//printf("  ProcessMouseButton %d %d\n", nMouseButton, nAction);

	glfwLockMutex(m_pInstance->m_oListenersMutex);

	if (true == m_pInstance->IsMousePointerVisible())
	{
		for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
		{
			(*it1)->ProcessMouseButton(nMouseButton, (GLFW_PRESS == nAction));
		}
	}
	else
	{
		for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
		{
			(*it1)->ProcessButton(1000 /* Mouse */, nMouseButton, (GLFW_PRESS == nAction));
		}
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);

	// DEBUG: Hack, remove old behaviour eventually
	InputProcessMouse(nMouseButton, nAction);
}

void GLFWCALL InputManager::ProcessMousePos(int nMousePosX, int nMousePosY)
{
	//printf("  ProcessMousePos %d %d\n", nMousePosX, nMousePosY);
	//printf(" mouse pos x = %d, cursor visible = %d\n", nMousePosX, m_pInstance->IsMousePointerVisible() ? 1 : 0);

	glfwLockMutex(m_pInstance->m_oListenersMutex);

	static bool		bPreviousMousePosSet = false;

	if (true == m_pInstance->IsMousePointerVisible())
	{
		bPreviousMousePosSet = false;

		for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
		{
			(*it1)->ProcessMousePosition(nMousePosX, nMousePosY);
		}

		//printf("       pointer moved to = (%d, %d)\n", nMousePosX, nMousePosY);
	}
	else
	{
		static int		nPreviousMousePosX;
		static int		nPreviousMousePosY;

		if (false == bPreviousMousePosSet)
		{
			//printf("  initial mouse pos = (%d, %d)\n", nMousePosX, nMousePosY);
			nPreviousMousePosX = nMousePosX;
			nPreviousMousePosY = nMousePosY;
			bPreviousMousePosSet = true;
		}
		else
		{
			int nMouseMovedX = nMousePosX - nPreviousMousePosX;
			int nMouseMovedY = nMousePosY - nPreviousMousePosY;
			nPreviousMousePosX = nMousePosX;
			nPreviousMousePosY = nMousePosY;

			for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
			{
				(*it1)->ProcessSlider(1000 /* Mouse */, 0 /* Mouse X Axis */, static_cast<double>(nMouseMovedX));
				(*it1)->ProcessSlider(1000 /* Mouse */, 1 /* Mouse Y Axis */, static_cast<double>(nMouseMovedY));
			}
		}
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);
}

void GLFWCALL InputManager::ProcessMouseWheel(int nMouseWheelPosition)
{
	//printf("  ProcessMouseWheel %d\n", nMouseWheelPosition);

	glfwLockMutex(m_pInstance->m_oListenersMutex);

	if (true == m_pInstance->IsMousePointerVisible())
	{
	}
	else
	{
		static int		nPreviousMouseWheelPosition;

		int nMouseWheelMoved = nMouseWheelPosition - nPreviousMouseWheelPosition;
		nPreviousMouseWheelPosition = nMouseWheelPosition;

		for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
		{
			(*it1)->ProcessSlider(1000 /* Mouse */, 2 /* Mouse Wheel */, nMouseWheelMoved);
		}
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);
}

void InputManager::ProcessJoysticks()
{
	glfwLockMutex(m_pInstance->m_oListenersMutex);

	int nDevice = 2000;		// Joystick
	for (std::vector<Joystick_t>::const_iterator it1 = m_oJoysticks.begin(); it1 != m_oJoysticks.end(); ++nDevice)
	{
		Joystick_t	oJoystick = *it1;
		float *		pJoystickPos = new float[oJoystick.nAxes];
		u_char *	pJoystickButtons = new u_char[oJoystick.nButtons];

		int nReturnedAxes = glfwGetJoystickPos(oJoystick.nId, pJoystickPos, oJoystick.nAxes);
		int nReturnedButtons = glfwGetJoystickButtons(oJoystick.nId, pJoystickButtons, oJoystick.nButtons);

		for (u_int nAxis = 0; nAxis < oJoystick.nAxes; ++nAxis)
		{
			for (std::vector<InputListener *>::const_reverse_iterator it2 = m_pInstance->m_oListeners.rbegin(); it2 != m_pInstance->m_oListeners.rend(); ++it2)
			{
				(*it2)->ProcessAxis(nDevice /* Joystick Number */, nAxis /* Axis Number */, pJoystickPos[nAxis] /* Axis Position */);
			}
		}

		for (u_int nButton = 0; nButton < oJoystick.nButtons; ++nButton)
		{
			for (std::vector<InputListener *>::const_reverse_iterator it2 = m_pInstance->m_oListeners.rbegin(); it2 != m_pInstance->m_oListeners.rend(); ++it2)
			{
				(*it2)->ProcessButton(nDevice /* Joystick Number */, nButton /* Button Number */, (GLFW_PRESS == pJoystickButtons[nButton]) /* Button Pressed */);
			}
		}

		delete[] pJoystickPos;
		delete[] pJoystickButtons;

		if (0 == nReturnedAxes && 0 == nReturnedButtons)
		{
			// Joystick no longer functional/connected
			printf("WARNING: Joystick id=%d has been disconnected.\n", oJoystick.nId);
			it1 = m_oJoysticks.erase(it1);
		}
		else
			++it1;
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);
}

void InputManager::TimePassed(double dTimePassed)
{
	glfwLockMutex(m_pInstance->m_oListenersMutex);

	for (std::vector<InputListener *>::const_reverse_iterator it1 = m_pInstance->m_oListeners.rbegin(); it1 != m_pInstance->m_oListeners.rend(); ++it1)
	{
		(*it1)->TimePassed(dTimePassed);
	}

	glfwUnlockMutex(m_pInstance->m_oListenersMutex);
}
