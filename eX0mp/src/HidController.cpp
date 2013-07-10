#include "globals.h"

HidController::HidController(CPlayer & oPlayer)
	: LocalController(oPlayer),
	  m_pPlayerInputListener(new PlayerInputListener())
{
	g_pInputManager->RegisterListener(m_pPlayerInputListener);
}

HidController::~HidController()
{
	g_pInputManager->UnregisterListener(m_pPlayerInputListener);
	delete m_pPlayerInputListener;
}

void HidController::ProvideRealtimeInput(double /*dTimePassed*/)
{
	m_oPlayer.Rotate(static_cast<float>(m_pPlayerInputListener->GetRotationAmount()));
}

void HidController::ProvideNextCommand()
{
	SequencedCommand_t oSequencedCommand;

	// Set the inputs
	oSequencedCommand.oCommand.cMoveDirection = m_pPlayerInputListener->GetMoveDirection();
	oSequencedCommand.oCommand.cStealth = (m_pPlayerInputListener->GetStealth() ? 1 : 0);
	oSequencedCommand.oCommand.fZ = m_oPlayer.GetZ();

	oSequencedCommand.cSequenceNumber = m_oPlayer.GlobalStateSequenceNumberTEST;

	//printf("ProvideNextCommand %d\n", oSequencedCommand.cSequenceNumber);
	eX0_assert(m_oPlayer.m_oCommandsQueue.push(oSequencedCommand), "m_oCommandsQueue.push(oCommand) failed, lost a command!!\n");
}

void HidController::ChildReset()
{
	m_pPlayerInputListener->Reset();
}
