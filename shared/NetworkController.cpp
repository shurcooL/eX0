// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

NetworkController::NetworkController(CPlayer & oPlayer)
	: PlayerController(oPlayer),
	  cCurrentCommandSeriesNumber(0),
	  m_oMutex(glfwCreateMutex())
{
}

NetworkController::~NetworkController()
{
	glfwDestroyMutex(m_oMutex);
}

void NetworkController::ProvideNextCommand()
{
}

void NetworkController::ChildReset()
{
}

void NetworkController::ProcessCommand(CPacket & oPacket)
{
	u_char cCommandSequenceNumber;
	u_char cCommandSeriesNumber;
	u_char cMovesCount;
	char cMoveDirection;
	u_char cStealth;
	float fZ;
	oPacket.unpack("ccc", &cCommandSequenceNumber, &cCommandSeriesNumber, &cMovesCount);
	cMovesCount += 1;	// De-normalize back to 1 (min value)

	NetworkController * pNetworkController = this;

	if (cCommandSeriesNumber != pNetworkController->cCurrentCommandSeriesNumber) {
		printf("Got a Command with mismatching series number, ignoring. cCommandSequenceNumber = %d, cMovesCount = %d, ignoring.\n", cCommandSequenceNumber, cMovesCount);
		return;
	}

	//printf("%f NC: got %d cmds\n", g_pGameSession->LogicTimer().GetGameTime(), (int)cMovesCount);
	for (int nMove = 0; nMove < (int)cMovesCount; ++nMove)
	{
		oPacket.unpack("ccf", &cMoveDirection, &cStealth, &fZ);

		SequencedCommand_t oSequencedCommand;
		oSequencedCommand.oCommand.cMoveDirection = cMoveDirection;
		oSequencedCommand.oCommand.cStealth = cStealth;
		oSequencedCommand.oCommand.fZ = fZ;
		oSequencedCommand.cSequenceNumber = static_cast<u_char>(cCommandSequenceNumber - (cMovesCount - 1) + nMove);

		eX0_assert(m_oPlayer.m_oCommandsQueue.push(oSequencedCommand), "m_oCommandsQueue.push(oCommand) failed, lost a command!!\n");
		//printf("%f NC: pushed %d\n", g_pGameSession->LogicTimer().GetGameTime(), oSequencedCommand.cSequenceNumber);
	}
	/* Outdated... Testing something else: pushing all the recvd commands into m_oCommandsQueue, and only the needed ones will be used from there
	// A special case for the first command we receive from this client in this new series
	if (pNetworkController->bFirstCommand) {
		pNetworkController->cLastRecvedCommandSequenceNumber = (u_char)(cCommandSequenceNumber - cMovesCount);
		pNetworkController->bFirstCommand = false;
	}

	if (cCommandSequenceNumber == pNetworkController->cLastRecvedCommandSequenceNumber) {
		printf("Got a duplicate UDP command packet from player %d, discarding.\n", m_oPlayer.iID);
	} else if ((char)(cCommandSequenceNumber - pNetworkController->cLastRecvedCommandSequenceNumber) < 0) {
		printf("Got an out of order UDP command packet from player %d, discarding.\n", m_oPlayer.iID);
	} else
	{
		int nMove = (int)cMovesCount - (char)(cCommandSequenceNumber - pNetworkController->cLastRecvedCommandSequenceNumber);
		if (nMove < 0) printf("!!MISSING!! %d command move(s) from player #%d, due to lost packets:\n", -nMove, m_oPlayer.iID);
		if (nMove < 0) nMove = 0;

		++pNetworkController->cLastRecvedCommandSequenceNumber;
		if (cCommandSequenceNumber != pNetworkController->cLastRecvedCommandSequenceNumber) {
			printf("Lost %d UDP command packet(s) from player #%d!\n", (u_char)(cCommandSequenceNumber - pNetworkController->cLastRecvedCommandSequenceNumber), m_oPlayer.iID);
		}
		pNetworkController->cLastRecvedCommandSequenceNumber = cCommandSequenceNumber;

		for (int nSkip = 0; nSkip < nMove; ++nSkip)
			oPacket.unpack("ccf", &cMoveDirection, &cStealth, &fZ);
		for (; nMove < (int)cMovesCount; ++nMove)
		{
			oPacket.unpack("ccf", &cMoveDirection, &cStealth, &fZ);
			//printf("execing command %d\n", cCommandSequenceNumber - (cMovesCount - 1) + nMove);

			SequencedCommand_t oSequencedCommand;

			// Set the inputs
			oSequencedCommand.oCommand.cMoveDirection = cMoveDirection;
			oSequencedCommand.oCommand.cStealth = cStealth;
			oSequencedCommand.oCommand.fZ = fZ;

			oSequencedCommand.cSequenceNumber = static_cast<u_char>(cCommandSequenceNumber - (cMovesCount - 1) + nMove);

			eX0_assert(m_oPlayer.m_oCommandsQueue.push(oSequencedCommand), "m_oCommandsQueue.push(oCommand) failed, lost a command!!\n");
			//printf("pushed %d\n", cCommandSequenceNumber - (cMovesCount - 1) + nMove);
		}
	}*/
}
