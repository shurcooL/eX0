#include "globals.h"

bool		bWindowWasActive = false;
int			nDesktopCursorX, nDesktopCursorY;
int			iMouseX, iMouseY;
int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
float		fFilteredMouseMovedX = 0;
float		fFilteredMouseMovedY = 0;
float		fMouseSensitivity = 0.25f;
bool		bAutoReload = true;

int			nChatMode;
string		sChatString;

bool		bSelectTeamDisplay = true;
bool		bSelectTeamReady = false;		// Indicates we're ready to select a team, ie. got a response to previous request

// do mouse movement calcs
void InputMouseMovCalcs()
{
	bool bWindowActive = (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE && !bSelectTeamDisplay);

	if (bWindowActive && bWindowWasActive) {
		bool bMouseMoved = false;

		glfwGetMousePos(&iMouseX, &iMouseY);

		if ((iMouseX - 320) || (iMouseY - 240))
		{
			glfwSetMousePos(320, 240);
		}

		for (int iLoop1 = MOUSE_FILTERING_SAMPLES - 1; iLoop1 > 0; --iLoop1)
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
	} else if (bWindowActive && !bWindowWasActive)
	{
		printf("Window became active.\n");

		// Window became active again
		//glfwGetMousePos(&nDesktopCursorX, &nDesktopCursorY);
		//POINT pos; GetCursorPos(&pos); nDesktopCursorX = pos.x; nDesktopCursorY = pos.y; printf("Mouse was at pos (%d, %d).\n", nDesktopCursorX, nDesktopCursorY);
		glfwDisable(GLFW_MOUSE_CURSOR);
		glfwSetMousePos(320, 240);
	} else if (!bWindowActive && bWindowWasActive)
	{
		printf("Window went inactive.\n");

		// Window went inactive
		glfwEnable(GLFW_MOUSE_CURSOR);
		//glfwSetMousePos(nDesktopCursorX, nDesktopCursorY);
		//SetCursorPos(nDesktopCursorX, nDesktopCursorY); printf("Putting mouse to pos (%d, %d).\n", nDesktopCursorX, nDesktopCursorY);

		for (int nLoop1 = 0; nLoop1 < MOUSE_FILTERING_SAMPLES; ++nLoop1) {
			iMouseMovedX[nLoop1] = 0;
			iMouseMovedY[nLoop1] = 0;
		}

		bSelectTeamDisplay = true;
	}/* else
	{
		int x, y;
		glfwGetMousePos(&x, &y); printf("Mouse is now at pos (%d, %d).\n", x, y);
	}*/

	bWindowWasActive = bWindowActive;
}

// check if a key is held down
void InputKeyHold()
{
	// Don't use the keyboard input when typing a message
	if (nChatMode) return;
}

// key processing
void GLFWCALL InputProcessKey(int iKey, int iAction)
{
	if (iAction == GLFW_PRESS)
	{
		if (nChatMode) {
			switch (iKey) {
			// Enter key
			case GLFW_KEY_ENTER:
				nChatMode = 0;
				glfwDisable(GLFW_KEY_REPEAT);
				// Send the chat string
				if (sChatString.length() > 0) {
					// Send the text message packet
					CPacket oSendMessagePacket;
					oSendMessagePacket.pack("hct", 0, (u_char)10, &sChatString);
					oSendMessagePacket.CompleteTpcPacketSize();
					pServer->SendTcp(oSendMessagePacket);
				}
				break;
			// Escape key
			case GLFW_KEY_ESC:
				nChatMode = 0;
				glfwDisable(GLFW_KEY_REPEAT);
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

		switch (iKey)
		{
		// escape key
		case GLFW_KEY_ESC:
			if (bSelectTeamDisplay && iGameState == 0) {
				bSelectTeamDisplay = false;
			} else {
				glfwCloseWindow();		// A window must be open since we got this event
			}
			break;
		// 'r' - reload
		case 'R':
			if (iGameState == 0) pLocalPlayer->Reload();
			break;
		// 'b' - buy clips
		case 'B':
			if (iGameState == 0) pLocalPlayer->BuyClip();
			break;
		// 'v' - change camera view
		case 'V':
			iCameraType = (iCameraType + 1) % 2;
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
			if (iGameState == 0) {
glfwLockMutex(oPlayerTick);
				if (!pLocalPlayer->IsDead()) {
					for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; ++iLoop1) {
						if (PlayerGet(iLoop1) == NULL || iLoop1 == iLocalPlayerID) continue;
						if (pow((pLocalPlayer->GetIntX() + Math::Sin(pLocalPlayer->GetZ()) * PLAYER_WIDTH) - PlayerGet(iLoop1)->GetIntX(), 2)
						  + pow((pLocalPlayer->GetIntY() + Math::Cos(pLocalPlayer->GetZ()) * PLAYER_WIDTH) - PlayerGet(iLoop1)->GetIntY(), 2)
						  <= PLAYER_HALF_WIDTH_SQR)
							PlayerGet(iLoop1)->GiveHealth(-150);
					}
				}
glfwUnlockMutex(oPlayerTick);
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
			glfwEnable(GLFW_KEY_REPEAT);
			break;
		// chat
		case 'T':
			sChatString = "";
			nChatMode = 1;
			glfwEnable(GLFW_KEY_REPEAT);
			break;
		// Select Team display
		case 'J':
			if (!bSelectTeamDisplay && iGameState == 0) {
				bSelectTeamDisplay = true;
			}
			break;
		case '1':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 0 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest;
				oJoinTeamRequest.pack("hcc", 0, (u_char)27, (u_char)0);
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '2':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 1 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest;
				oJoinTeamRequest.pack("hcc", 0, (u_char)27, (u_char)1);
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '3':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 2 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest;
				oJoinTeamRequest.pack("hcc", 0, (u_char)27, (u_char)2);
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '0':
			if (bSelectTeamDisplay && bSelectTeamReady && iGameState == 0) {
				bSelectTeamDisplay = false;
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

	/*pLocalPlayer->fAimingDistance -= fFilteredMouseMovedY * fMouseSensitivity * 2;
	if (pLocalPlayer->fAimingDistance > 500.0)
		pLocalPlayer->fAimingDistance = 500.0;
	if (pLocalPlayer->fAimingDistance < 25.0)
		pLocalPlayer->fAimingDistance = 25.0;*/

	pLocalPlayer->Rotate(fFilteredMouseMovedX * fMouseSensitivity / 100.0f
		* (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 0.5f : 1.0f));
		//* 200.0 / pLocalPlayer->fAimingDistance);
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
		//pLocalPlayer->Fire();
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
			//if (iGameState == 0) pLocalPlayer->Fire();
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if (iGameState == 0) pLocalPlayer->iSelWeapon = ((pLocalPlayer->iSelWeapon == 3) ? 2 : 3);
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
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		default:
			break;
		}
		// any button
		// ...
	}
}
