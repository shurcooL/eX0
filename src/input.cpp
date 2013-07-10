#include "globals.h"

int			iCursorX = 320;
int			iCursorY = 240;
int			iMouseX, iMouseY;
int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
float		fFilteredMouseMovedX = 0;
float		fFilteredMouseMovedY = 0;
int			iMouseButtonsDown;
float		fMouseSensivity = 0.25f;
bool		bAutoReload = false;

// do mouse movement calcs
void InputMouseMovCalcs()
{
	bool bMouseMoved = false;

	glfwGetMousePos(&iMouseX, &iMouseY);

	if ((iMouseX - 320) || (iMouseY - 240))
	{
		glfwSetMousePos(320, 240);
	}

	for (int iLoop1 = MOUSE_FILTERING_SAMPLES - 1; iLoop1 > 0 ; iLoop1--)
	{
		iMouseMovedX[iLoop1] = iMouseMovedX[iLoop1 - 1];
		iMouseMovedY[iLoop1] = iMouseMovedY[iLoop1 - 1];

		if (iMouseMovedX[iLoop1] || iMouseMovedY[iLoop1])
			bMouseMoved = true;
	}

	iMouseMovedX[0] = iMouseX - 320;
	iMouseMovedY[0] = iMouseY - 240;
	//iMouseMovedX = iMouseX - 320;
	//iMouseMovedY = iMouseY - 240;

	if (iMouseMovedX[0] || iMouseMovedY[0])
	//if (iMouseMovedX || iMouseMovedY)
		bMouseMoved = true;

	if (bMouseMoved)
		InputMouseMoved();
}

// check if a key is held down
void InputKeyHold()
{
	if (!iGameState)
	{
		// set stealth
		if (glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT))
		{
			oPlayers[iLocalPlayerID]->SetStealth(true);
		}
		else
		{
			oPlayers[iLocalPlayerID]->SetStealth(false);
		}

		// Rotate left/right using arrow keys
		if (glfwGetKey(GLFW_KEY_LEFT) && !glfwGetKey(GLFW_KEY_RIGHT))
			oPlayers[iLocalPlayerID]->Rotate(fTimePassed * -1.5);
		else if (glfwGetKey(GLFW_KEY_RIGHT) && !glfwGetKey(GLFW_KEY_LEFT))
			oPlayers[iLocalPlayerID]->Rotate(fTimePassed * 1.5);

		if (glfwGetKey('A') && !glfwGetKey('D'))
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->Move(7);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->Move(5);
			}
			else
			{
				oPlayers[iLocalPlayerID]->Move(6);
			}
		}
		else if (glfwGetKey('D') && !glfwGetKey('A'))
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->Move(1);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->Move(3);
			}
			else
			{
				oPlayers[iLocalPlayerID]->Move(2);
			}
		}
		else
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->Move(0);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->Move(4);
			}
			else
			{
				oPlayers[iLocalPlayerID]->Move(-1);
			}
		}
	}
}

// key processing
void InputProcessKey(int iKey, int iAction)
{
	if (iAction == GLFW_PRESS)
	{
		switch (iKey) {
		// escape key
		case GLFW_KEY_ESC:
			Terminate(0);
			break;
		// 'r' - reload
		case 'R':
			oPlayers[iLocalPlayerID]->Reload();
			break;
		// 'b' - buy clips
		case 'B':
			oPlayers[iLocalPlayerID]->BuyClip();
			break;
		// 'v' - change camera view
		case 'V':
			iCameraType = (++iCameraType) % 2;
			break;
		// restart the game
		case GLFW_KEY_F1:
			RestartGame();
			break;
		// pause the game
		case 'P':
			bPaused = !bPaused;
			break;
		// DEBUG: Insta-Knife
		case 'F':
			if (!oPlayers[iLocalPlayerID]->IsDead()) {
				for (int iLoop1 = 0; iLoop1 < iNumPlayers; ++iLoop1) {
					if (iLoop1 == iLocalPlayerID) continue;
					if (pow((oPlayers[iLocalPlayerID]->GetIntX() + Math::Sin(oPlayers[iLocalPlayerID]->GetZ()) * PLAYER_WIDTH) - oPlayers[iLoop1]->GetIntX(), 2)
					  + pow((oPlayers[iLocalPlayerID]->GetIntY() + Math::Cos(oPlayers[iLocalPlayerID]->GetZ()) * PLAYER_WIDTH) - oPlayers[iLoop1]->GetIntY(), 2)
					  <= PLAYER_HALF_WIDTH_SQR)
						oPlayers[iLoop1]->GiveHealth(-100);
				}
			}
			break;
		// any other key
		default:
			break;
		}
	}
	/*else if (iAction == GLFW_RELEASE)
	{
		switch (iKey) {
		// any other key
		default:
			break;
		}
	}*/
}

// do whatever if mouse has moved
void InputMouseMoved()
{
	InputFilteredMouseMoved();

	if (!iGameState)
	// if we're actally in game
	{
		oPlayers[iLocalPlayerID]->Rotate(fFilteredMouseMovedX * fMouseSensivity / 100.0);
	}
}

// calculate the filtered mouse moved vars
void InputFilteredMouseMoved()
{
	fFilteredMouseMovedX = 0.0;
	fFilteredMouseMovedY = 0.0;

	for (int iLoop1 = 0; iLoop1 < MOUSE_FILTERING_SAMPLES; iLoop1++)
	{
		fFilteredMouseMovedX += float(iMouseMovedX[iLoop1]);
		fFilteredMouseMovedY += float(iMouseMovedY[iLoop1]);
	}

	fFilteredMouseMovedX /= float(MOUSE_FILTERING_SAMPLES);
	fFilteredMouseMovedY /= float(MOUSE_FILTERING_SAMPLES);
}

// check if a mouse button is held down
void InputMouseHold()
{
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	// left mouse button is held down
	{
		oPlayers[iLocalPlayerID]->Fire();
	}
	else
	// left mouse button is NOT held down
	{
	}

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	// right mouse button is held down
	{
	}
	else
	// right mouse button is NOT held down
	{
	}

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	// middle mouse button is held down
	{
	}
	else
	// middle mouse button is NOT held down
	{
	}
}

// processes mouse clicks
void InputProcessMouse(int iButton, int iAction)
{
	// mouse button down
	if (iAction == GLFW_PRESS)
	{
		switch (iButton) {
		case GLFW_MOUSE_BUTTON_LEFT:
			//iMouseButtonsDown |= 1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			//iMouseButtonsDown |= 2;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			//iMouseButtonsDown |= 4;
			oPlayers[iLocalPlayerID]->iSelWeapon =
				((oPlayers[iLocalPlayerID]->iSelWeapon == 3) ? 2 : 3);
			break;
		default:
			break;
		}
		// any button
		// ...
	}
	// mouse button up
	else if (iAction == GLFW_RELEASE)
	{
		switch (iButton) {
		case GLFW_MOUSE_BUTTON_LEFT:
			//iMouseButtonsDown &= ~1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			//iMouseButtonsDown &= ~2;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			//iMouseButtonsDown &= ~4;
			break;
		default:
			break;
		}
		// any button
		// ...
	}
}

// check if a mouse button is pressed
/*bool InputIsMouseButtonDown(int iButton)
{
	switch (iButton) {
	case 0:
		// left mouse button
		if ((iMouseButtonsDown & 1) == 1) return true;
		break;
	case 1:
		// right mouse button
		if ((iMouseButtonsDown & 2) == 2) return true;
		break;
	case 2:
		// middle mouse button
		if ((iMouseButtonsDown & 4) == 4) return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}*/