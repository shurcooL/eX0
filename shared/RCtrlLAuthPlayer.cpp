// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

RCtrlLAuthPlayer::RCtrlLAuthPlayer()
	: CPlayer()
{
	printf("RCtrlLAuthPlayer(%p) Ctor.\n", this);
}

RCtrlLAuthPlayer::~RCtrlLAuthPlayer()
{
	printf("RCtrlLAuthPlayer(%p) ~Dtor.\n", this);
}

void RCtrlLAuthPlayer::ProcessInputCmdTEST()
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

			cCurrentCommandSequenceNumber = oSequencedInput.cSequenceNumber;
		}//end section from Network.cpp
//static u_int i = 0; if (i++ % 50 == 0) printf("processed Input Command         in %.5lf ms\n", (glfwGetTime() - t1) * 1000);
	}
}
