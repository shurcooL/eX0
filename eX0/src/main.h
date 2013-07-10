#pragma once
#ifndef __Main_H__
#define __Main_H__

void eX0_assert(bool expression, std::string message = "", bool fatal = false);

// This function is not used to ensure thread-safely (it doesn't, as the operation isn't atomic and creates a race condition),
// but rather used during shut down in sequential order from main thread, to delete each subsystem.
// The only reason to set null first, delete after, is if the object's destructed may use its own pointer for some reason? Or delete another object,
// which may then check if the first is not null? This can be avoided: DON'T USE GLOBALS! The sub-object shouldn't be accessing a global in the 1st place.
// Conclusion: Don't do this.
/*template <typename T>
void SetNullptrAndDelete(T *& pPointer)
{
	T * pPointerCopy = pPointer;
	pPointer = nullptr;
	delete pPointerCopy;
}*/

// DEBUG
void DumpStateHistory(std::list<AuthState_st> & oStateHistory);

// set glfw callback functions
void SetGlfwCallbacks();

// syncronizes random seed with all clients
void SyncRandSeed();

// quits
void Terminate(int nExitCode);

// Restarts the game
void RestartGame();

#endif // __Main_H__
