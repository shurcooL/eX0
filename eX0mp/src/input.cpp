#include "globals.h"

int			iCursorX = 320;
int			iCursorY = 240;
int			iMouseX, iMouseY;
int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
float		fFilteredMouseMovedX = 0;
float		fFilteredMouseMovedY = 0;
int			iMouseButtonsDown;
float		fMouseSensitivity = 0.25f;
bool		bAutoReload = true;

int			nChatMode;
string		sChatString;

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
	// Don't use the keyboard input when typing a message
	if (nChatMode) return;

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
			oPlayers[iLocalPlayerID]->Rotate(fTimePassed * -1.5f);
		else if (glfwGetKey(GLFW_KEY_RIGHT) && !glfwGetKey(GLFW_KEY_LEFT))
			oPlayers[iLocalPlayerID]->Rotate(fTimePassed * 1.5f);

		if (glfwGetKey('A') && !glfwGetKey('D'))
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(7);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(5);
			}
			else
			{
				oPlayers[iLocalPlayerID]->MoveDirection(6);
			}
		}
		else if (glfwGetKey('D') && !glfwGetKey('A'))
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(1);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(3);
			}
			else
			{
				oPlayers[iLocalPlayerID]->MoveDirection(2);
			}
		}
		else
		{
			if ((glfwGetKey('W') && !glfwGetKey('S'))
				|| (glfwGetKey(GLFW_KEY_UP) && !glfwGetKey(GLFW_KEY_DOWN)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(0);
			}
			else if ((glfwGetKey('S') && !glfwGetKey('W'))
					|| (glfwGetKey(GLFW_KEY_DOWN) && !glfwGetKey(GLFW_KEY_UP)))
			{
				oPlayers[iLocalPlayerID]->MoveDirection(4);
			}
			else
			{
				oPlayers[iLocalPlayerID]->MoveDirection(-1);
			}
		}
	}
}

// key processing
void InputProcessKey(int iKey, int iAction)
{
	if (iAction == GLFW_PRESS)
	{
		if (nChatMode) {
			switch (iKey) {
			// Enter key
			case GLFW_KEY_ENTER:
				nChatMode = 0;
				// Send the chat string
				// DEBUG: Finish
				if (sChatString.length() > 0) {
					// Send the text message packet
					CPacket oSendMessagePacket;
					oSendMessagePacket.pack("hht", 0, 10, &sChatString);
					oSendMessagePacket.CompleteTpcPacketSize();
					oSendMessagePacket.SendTcp();
				}
				break;
			// Escape key
			case GLFW_KEY_ESC:
				nChatMode = 0;
				break;
			// Backspace key
			case GLFW_KEY_BACKSPACE:
				if (sChatString.length() > 0)
					sChatString = sChatString.substr(0, sChatString.length() - 1);
				break;
			// Any other key
			default:
				break;
			}

			return;
		}

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

			// Reset the mask
			OglUtilsSetMaskingMode(RENDER_TO_MASK_MODE);
			glClear(GL_STENCIL_BUFFER_BIT);
			OglUtilsSetMaskingMode(NO_MASKING_MODE);
			break;
		// restart the game
		/*case GLFW_KEY_F1:
			RestartGame();
			break;*/
		// pause the game
		case 'P':
			bPaused = !bPaused;
			break;
		// DEBUG: Insta-Knife
		case 'F':
			if (!oPlayers[iLocalPlayerID]->IsDead()) {
				for (int iLoop1 = 0; iLoop1 < nPlayerCount; ++iLoop1) {
					if (iLoop1 == iLocalPlayerID) continue;
					if (pow((oPlayers[iLocalPlayerID]->GetIntX() + Math::Sin(oPlayers[iLocalPlayerID]->GetZ()) * PLAYER_WIDTH) - oPlayers[iLoop1]->GetIntX(), 2)
					  + pow((oPlayers[iLocalPlayerID]->GetIntY() + Math::Cos(oPlayers[iLocalPlayerID]->GetZ()) * PLAYER_WIDTH) - oPlayers[iLoop1]->GetIntY(), 2)
					  <= PLAYER_HALF_WIDTH_SQR)
						oPlayers[iLoop1]->GiveHealth(-100);
				}
			}
			break;
		// DEBUG: Toggle the wireframe mode
		case GLFW_KEY_F2:
			bWireframe = !bWireframe;
			break;
		// DEBUG: Toggle between gpc and PolyBoolean triangulations
		case GLFW_KEY_F3:
			bUseDefaultTriangulation = !bUseDefaultTriangulation;
			printf("Using %s triangulation.\n", bUseDefaultTriangulation ? "gpc" : "PB");
			break;
		// DEBUG: Toggle on/off the stencil operations
		case GLFW_KEY_F4:
			bStencilOperations = !bStencilOperations;
			printf("Turned %s stencil operations.\n", bStencilOperations ? "ON" : "OFF");
			break;
		// chat
		case GLFW_KEY_ENTER:
			sChatString = "";
			nChatMode = 2;
			break;
		// chat
		case 'T':
			sChatString = "";
			nChatMode = 1;
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

// Character processing function
void GLFWCALL InputProcessChar(int nChar, int nAction)
{
	if (nAction == GLFW_PRESS)
	{
		if (nChatMode == 1)
			nChatMode = 2;
		else if (nChatMode == 2) {
			if (sChatString.length() < 57 && nChar < 256)
				sChatString = sChatString + (char)nChar;
			// DEBUG: ... I dunno if this is necessary to check or if it's always gonna pass
			if (!isprint(nChar)) { printf("OMG ERRORZRE!\n"); Terminate(152); }
		}
	}
	/*else if (iAction == GLFW_RELEASE)
	{
	}*/
}

// do whatever if mouse has moved
void InputMouseMoved()
{
	InputFilteredMouseMoved();

	if (!iGameState)
	// if we're actally in game
	{
		/*oPlayers[iLocalPlayerID]->fAimingDistance -= fFilteredMouseMovedY * fMouseSensitivity * 2;
		if (oPlayers[iLocalPlayerID]->fAimingDistance > 500.0)
			oPlayers[iLocalPlayerID]->fAimingDistance = 500.0;
		if (oPlayers[iLocalPlayerID]->fAimingDistance < 25.0)
			oPlayers[iLocalPlayerID]->fAimingDistance = 25.0;*/

		oPlayers[iLocalPlayerID]->Rotate(fFilteredMouseMovedX * fMouseSensitivity / 100.0f
			* (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 0.5f : 1.0f));
			//* 200.0 / oPlayers[iLocalPlayerID]->fAimingDistance);
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
		//oPlayers[iLocalPlayerID]->Fire();
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
void GLFWCALL InputProcessMouse(int iButton, int iAction)
{
	// mouse button down
	if (iAction == GLFW_PRESS)
	{
		switch (iButton) {
		case GLFW_MOUSE_BUTTON_LEFT:
			//if (iGameState == 0) oPlayers[iLocalPlayerID]->Fire();
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			//iMouseButtonsDown |= 2;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			//iMouseButtonsDown |= 4;
			if (iGameState == 0) oPlayers[iLocalPlayerID]->iSelWeapon =
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
