#pragma once
#ifndef __Main_H__
#define __Main_H__

void eX0_assert(bool expression, string message = "");

// set glfw callback functions
void SetGlfwCallbacks(void);

// syncronizes random seed with all clients
void SyncRandSeed(void);

// quits
void Terminate(int nExitCode);

// Restarts the game
void RestartGame(void);

#endif // __Main_H__
