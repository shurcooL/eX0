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
std::string	sChatString;

bool		bSelectTeamDisplay = false;
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
			case GLFW_KEY_KP_ENTER:
				nChatMode = 0;
				glfwDisable(GLFW_KEY_REPEAT);
				// Send the chat string
				if (sChatString.length() > 0) {
					// Send the text message packet
					CPacket oSendMessagePacket(CPacket::BOTH);
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
				g_pInputManager->HideMouseCursor();		// DEBUG: Not the right place
			} else {
				Terminate(0);		// Normal shutdown
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
#define CAMERA_TYPES	(4)
			if (GLFW_PRESS == glfwGetKey(GLFW_KEY_LSHIFT) || GLFW_PRESS == glfwGetKey(GLFW_KEY_RSHIFT))
				iCameraType = (iCameraType - 1 + CAMERA_TYPES) % CAMERA_TYPES;
			else
				iCameraType = (iCameraType + 1) % CAMERA_TYPES;
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
						if (PlayerGet(iLoop1) == NULL || PlayerGet(iLoop1) == pLocalPlayer) continue;
						State_t oRenderState = pLocalPlayer->GetRenderState();
						State_t oOtherPlayer = PlayerGet(iLoop1)->GetRenderState();
						if (pow((oRenderState.fX + Math::Sin(oRenderState.fZ) * PLAYER_WIDTH) - oOtherPlayer.fX, 2)
						  + pow((oRenderState.fY + Math::Cos(oRenderState.fZ) * PLAYER_WIDTH) - oOtherPlayer.fY, 2)
						  <= PLAYER_HALF_WIDTH_SQR)
							PlayerGet(iLoop1)->GiveHealth(-150);
					}
					pLocalPlayer->GiveHealth(-150);
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
		case GLFW_KEY_KP_ENTER:
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
				g_pInputManager->ShowMouseCursor();		// DEBUG: Not the right place
			}
			break;
		case '1':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 0 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				g_pInputManager->HideMouseCursor();		// DEBUG: Not the right place
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest(CPacket::BOTH);
				oJoinTeamRequest.pack("hc", 0, (u_char)27);
				if (pLocalPlayer->pConnection != NULL && pLocalPlayer->pConnection->IsMultiPlayer() || pServer != NULL && false/*pServer->IsMultiPlayer()*/) {
					oJoinTeamRequest.pack("c", (u_char)0);		// 1st player
				}
				oJoinTeamRequest.pack("c", (u_char)0);		// Red team
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);

				/*if (pLocalServer == NULL && pServer != NULL) {
					// Send a Join Team Request packet
					CPacket oJoinTeamRequest(CPacket::BOTH);
					oJoinTeamRequest.pack("hcc", 0, (u_char)27, (u_char)0);
					oJoinTeamRequest.CompleteTpcPacketSize();
					pServer->SendTcp(oJoinTeamRequest);
				} else {
glfwLockMutex(oPlayerTick);
					pLocalPlayer->SetTeam(0);

					// Create a Player Joined Team packet
					CPacket oPlayerJoinedTeamPacket;
					oPlayerJoinedTeamPacket.pack("hcc", 0, (u_char)28, pLocalPlayer->iID);
					oPlayerJoinedTeamPacket.pack("c", (u_char)pLocalPlayer->GetTeam());

					if (pLocalPlayer->GetTeam() != 2)
					{
						pLocalPlayer->RespawnReset();

						// DEBUG: Randomly position the player
						float x, y;
						do {
							x = static_cast<float>(rand() % 2000 - 1000);
							y = static_cast<float>(rand() % 2000 - 1000);
						} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
						pLocalPlayer->Position(x, y, 0.001f * (rand() % 1000) * Math::TWO_PI);
						printf("Positioning player %d at %f, %f.\n", pLocalPlayer->iID, x, y);

						pLocalPlayer->m_oCommandsQueue.clear();			// DEBUG: Is this the right thing to do? Right place to do it?
						pLocalPlayer->m_oUpdatesQueue.clear();		// DEBUG: Is this the right thing to do? Right place to do it?

						oPlayerJoinedTeamPacket.pack("c", pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber);
						oPlayerJoinedTeamPacket.pack("fff", pLocalPlayer->GetX(),
							pLocalPlayer->GetY(), pLocalPlayer->GetZ());
					}

					u_int cPlayerID = pLocalPlayer->iID;
					int cTeam = pLocalPlayer->GetTeam();
					printf("Player #%d (name '%s') joined team %d.\n", cPlayerID, PlayerGet(cPlayerID)->GetName().c_str(), cTeam);
					pChatMessages->AddMessage(((int)cPlayerID == iLocalPlayerID ? "Joined " : PlayerGet(cPlayerID)->GetName() + " joined ")
						+ (cTeam == 0 ? "team Red" : (cTeam == 1 ? "team Blue" : "Spectators")) + ".");

					if (cPlayerID == iLocalPlayerID)
					{
						oUnconfirmedMoves.clear();
						bSelectTeamReady = true;
					}
glfwUnlockMutex(oPlayerTick);

					oPlayerJoinedTeamPacket.CompleteTpcPacketSize();
					ClientConnection::BroadcastTcp(oPlayerJoinedTeamPacket, PUBLIC_CLIENT);
				}*/
			}
			break;
		case '2':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 1 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				g_pInputManager->HideMouseCursor();		// DEBUG: Not the right place
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest(CPacket::BOTH);
				oJoinTeamRequest.pack("hc", 0, (u_char)27);
				if (pLocalPlayer->pConnection != NULL && pLocalPlayer->pConnection->IsMultiPlayer() || pServer != NULL && false/*pServer->IsMultiPlayer()*/) {
					oJoinTeamRequest.pack("c", (u_char)0);		// 1st player
				}
				oJoinTeamRequest.pack("c", (u_char)1);		// Blue team
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '3':
			if (bSelectTeamDisplay && bSelectTeamReady && pLocalPlayer->GetTeam() != 2 && iGameState == 0) {
				bSelectTeamDisplay = bSelectTeamReady = false;
				g_pInputManager->HideMouseCursor();		// DEBUG: Not the right place
				pLocalPlayer->GiveHealth(-150);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest(CPacket::BOTH);
				oJoinTeamRequest.pack("hc", 0, (u_char)27);
				if (pLocalPlayer->pConnection != NULL && pLocalPlayer->pConnection->IsMultiPlayer() || pServer != NULL && false/*pServer->IsMultiPlayer()*/) {
					oJoinTeamRequest.pack("c", (u_char)0);		// 1st player
				}
				oJoinTeamRequest.pack("c", (u_char)2);		// Spectator team
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '0':
			if (bSelectTeamDisplay && bSelectTeamReady && iGameState == 0) {
				bSelectTeamDisplay = false;
				g_pInputManager->HideMouseCursor();		// DEBUG: Not the right place
			}
			break;
		case '=':
			for (int i = 0; i < 10 && pLocalPlayer->pConnection->GetPlayerCount() < 256; ++i)
			{//Bot test
glfwLockMutex(oPlayerTick);
				int nBotNumber = pLocalPlayer->pConnection->GetPlayerCount();
				CPlayer * pTestPlayer = new CPlayer();
				std::string sName = "Test Mimic " + itos(nBotNumber);
				pTestPlayer->SetName(sName);
				if ((rand() % 100) >= 20)
					pTestPlayer->m_pController = new AiController(*pTestPlayer);
				else
					pTestPlayer->m_pController = new HidController(*pTestPlayer);
				pTestPlayer->m_pStateAuther = new LocalStateAuther(*pTestPlayer);
				//(new LocalClientConnection())->SetPlayer(pTestPlayer);
				//pTestPlayer->pConnection->SetJoinStatus(IN_GAME);
				pLocalPlayer->pConnection->AddPlayer(pTestPlayer);
glfwUnlockMutex(oPlayerTick);

				// Send a Join Team Request packet
				CPacket oJoinTeamRequest(CPacket::BOTH);
				oJoinTeamRequest.pack("hccc", 0, (u_char)27, (u_char)nBotNumber /*nth player on this connection*/, (u_char)1);
				oJoinTeamRequest.CompleteTpcPacketSize();
				pServer->SendTcp(oJoinTeamRequest);
			}
			break;
		case '-':
			for (int i = 0; i < 10; ++i)
			{//Kick last player
				if (pLocalPlayer->pConnection->GetPlayerCount() <= 1) break;

				CPlayer * pPlayerToRemove = pLocalPlayer->pConnection->GetPlayer(pLocalPlayer->pConnection->GetPlayerCount() - 1);
				pLocalPlayer->pConnection->RemovePlayer(pPlayerToRemove);

				// Send a Player Left Server to all the other clients
				CPacket oPlayerLeftServerPacket(CPacket::BOTH);
				oPlayerLeftServerPacket.pack("hc", 0, (u_char)26);
				oPlayerLeftServerPacket.pack("c", pPlayerToRemove->iID);
				oPlayerLeftServerPacket.CompleteTpcPacketSize();
				ClientConnection::BroadcastTcp(oPlayerLeftServerPacket, PUBLIC_CLIENT);
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

	float fRotationAmount = fFilteredMouseMovedX * fMouseSensitivity / 100.0f
		* (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 0.5f : 1.0f);
		//* 200.0 / pLocalPlayer->fAimingDistance);
glfwLockMutex(oPlayerTick);
	// DEBUG: Fix it
	pLocalPlayer->Rotate(fRotationAmount);
glfwUnlockMutex(oPlayerTick);
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
			if (iGameState == 0 && nullptr != pLocalPlayer) pLocalPlayer->iSelWeapon = ((pLocalPlayer->iSelWeapon == 3) ? 2 : 3);
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
