#include "globals.h"

HidController::HidController(CPlayer & oPlayer)
	: LocalController(oPlayer),
	  m_pPlayerInputListener(NULL)
{
	m_pPlayerInputListener = new PlayerInputListener();
	g_pInputManager->RegisterListener(m_pPlayerInputListener);
}

HidController::~HidController()
{
}

void HidController::ProvideRealtimeInput(double /*dTimePassed*/)
{
	m_oPlayer.Rotate(static_cast<float>(m_pPlayerInputListener->GetRotationAmount()));
}

//bool HidController::RequestCommand(u_char/* cSequenceNumber*/)
void HidController::ProvideNextCommand()
{
	SequencedCommand_t oSequencedCommand;

	// Set the inputs
	oSequencedCommand.oCommand.cMoveDirection = m_pPlayerInputListener->GetMoveDirection();
	oSequencedCommand.oCommand.cStealth = (m_pPlayerInputListener->GetStealth() ? 1 : 0);
	oSequencedCommand.oCommand.fZ = m_oPlayer.GetZ();

	oSequencedCommand.cSequenceNumber = g_cCurrentCommandSequenceNumber;

	eX0_assert(m_oPlayer.m_oInputCmdsTEST.push(oSequencedCommand), "m_oInputCmdsTEST.push(oCommand) failed, lost a command!!\n");
}
