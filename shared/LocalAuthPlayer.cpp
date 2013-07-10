// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

LocalAuthPlayer::LocalAuthPlayer()
	: CPlayer()
{
	printf("LocalAuthPlayer(%p) Ctor.\n", this);
}

LocalAuthPlayer::~LocalAuthPlayer()
{
	printf("LocalAuthPlayer(%p) ~Dtor.\n", this);
}

void LocalAuthPlayer::ProcessInputCmdTEST()
{
	while (!m_oInputCmdsTEST.empty()) {
//double t1 = glfwGetTime();
		//eX0_assert(m_oInputCmdsTEST.size() == 1, "m_oAuthUpdatesTEST.size() is != 1!!!!\n");

		SequencedInput_t oSequencedInput;
		m_oInputCmdsTEST.pop(oSequencedInput);
		//static u_char cmdNum = 0; printf("popped an input cmd %d\n", cmdNum++);

		{//begin section from Network.cpp
			// Set the inputs
			MoveDirection(oSequencedInput.oInput.cMoveDirection);
			SetStealth(oSequencedInput.oInput.cStealth != 0);
			SetZ(oSequencedInput.oInput.fZ);

			// Player tick
			CalcTrajs();
			CalcColResp();

			cLatestAuthStateSequenceNumber = oSequencedInput.cSequenceNumber;
		}//end section from Network.cpp
//static u_int i = 0; if (i++ % 50 == 0) printf("processed Input Command         in %.5lf ms\n", (glfwGetTime() - t1) * 1000);
	}
}

void LocalAuthPlayer::SendUpdate()
{
	if (pConnection->GetJoinStatus() < IN_GAME)
		return;

	while (dCurTime >= m_dNextUpdateTime)
	{
		m_dNextUpdateTime += 1.0 / pConnection->cUpdateRate;

		++pConnection->cCurrentUpdateSequenceNumber;		// First update is sent with cCurrentUpdateSequenceNumber == 1

		// Send the Update Others Position packet
		CPacket oServerUpdatePacket;
		oServerUpdatePacket.pack("cc", (u_char)2, pConnection->cCurrentUpdateSequenceNumber);
		for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
			if (PlayerGet(nPlayer) != NULL && PlayerGet(nPlayer)->pConnection->GetJoinStatus() == IN_GAME
				&& PlayerGet(nPlayer)->GetTeam() != 2)
			{
				oServerUpdatePacket.pack("c", (u_char)1);

				oServerUpdatePacket.pack("c", dynamic_cast<LocalAuthPlayer *>(PlayerGet(nPlayer))->cLatestAuthStateSequenceNumber);
				oServerUpdatePacket.pack("fff", PlayerGet(nPlayer)->GetX(),
					PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
			} else {
				oServerUpdatePacket.pack("c", (u_char)0);
			}
		}
		if ((rand() % 100) >= 0 || pConnection->GetPlayerID() != 0) // DEBUG: Simulate packet loss
			pConnection->SendUdp(oServerUpdatePacket);
	}
}
