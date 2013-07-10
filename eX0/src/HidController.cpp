#include "globals.h"

HidController::HidController(CPlayer & oPlayer)
	: LocalController(oPlayer),
	  m_oPlayerInputListener(oPlayer)
{
	g_pInputManager->RegisterListener(&m_oPlayerInputListener);
}

HidController::~HidController()
{
	g_pInputManager->UnregisterListener(&m_oPlayerInputListener);
}

void HidController::ProvideRealtimeInput(double /*dTimePassed*/)
{
	m_oPlayer.Rotate(static_cast<float>(m_oPlayerInputListener.GetRotationAmount()));
}

void HidController::ProvideNextCommand()
{
	SequencedCommand_st oSequencedCommand;

	// Set the inputs
	oSequencedCommand.oCommand = m_oPlayerInputListener.GetNextCommand();
	oSequencedCommand.cSequenceNumber = m_oPlayer.GlobalStateSequenceNumberTEST;

	//printf("ProvideNextCommand %d\n", oSequencedCommand.cSequenceNumber);
	eX0_assert(m_oPlayer.m_oCommandsQueue.push(oSequencedCommand), "m_oCommandsQueue.push(oCommand) failed, lost a command!!\n");
}

void HidController::ProvideNextWpnCommand()
{
	WpnCommand_st oWpnCommand;

	oWpnCommand = m_oPlayerInputListener.GetNextWpnCommand();

	WpnCommand_st oIdleWpnCommand = oWpnCommand;
	oIdleWpnCommand.nAction = WeaponSystem::IDLE;

	//if (WeaponSystem::IDLE != oWpnCommand.nAction) {
	//if (m_oPlayer.GetSelWeaponTEST().IsReadyForNextCommand())
		eX0_assert(m_oPlayer.m_oWpnCommandsQueue.push(oWpnCommand), "m_oPlayer.m_oWpnCommandsQueue.push(oWpnCommand)");
		iTempInt = (int)m_oPlayer.m_oWpnCommandsQueue.size();
		eX0_assert(m_oPlayer.m_oWpnCommandsQueue.push(oIdleWpnCommand), "m_oPlayer.m_oWpnCommandsQueue.push(oIdleWpnCommand)");

		/*State_st oRenderState = m_oPlayer.GetRenderState();
		printf("shot at x=%f, y=%f, z=%f\n", oRenderState.fX, oRenderState.fY, oRenderState.fZ);*/
	//}
}

void HidController::SubReset()
{
	m_oPlayerInputListener.Reset();
}
