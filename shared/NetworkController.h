#pragma once
#ifndef __NetworkController_H__
#define __NetworkController_H__

class NetworkController
	: public PlayerController
{
public:
	NetworkController(CPlayer & oPlayer);
	~NetworkController();

	void ProcessCommand(CPacket & oPacket);

	bool IsLocal(void) { return false; }

	//u_char		cLastRecvedCommandSequenceNumber;
	//bool		bFirstCommand;						// When true, indicates we are expecting the first command from a client (so far got nothing) and will be set to false when it arrives
	u_char		cCurrentCommandSeriesNumber;		// A number that changes on every respawn, team change, etc. and the server will ignore any Commands with mismatching series number

protected:
	void ProvideNextCommand();
	void ChildReset();

private:
	NetworkController(const NetworkController &);
	NetworkController & operator =(const NetworkController &);

	GLFWmutex		m_oMutex;		// DEBUG: What is this for? I forgot lol
};

#endif // __NetworkController_H__
