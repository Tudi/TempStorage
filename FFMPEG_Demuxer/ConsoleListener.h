#pragma once

/*
* Whole implementation needs to be converted to be cross platform
*/

enum PlayerCommands
{
	PC_DECODE_1_PACKET = 1,
	PC_CONTINUE_DECODING_PACKETS,
	PC_PAUSE_DECODING,
	PC_ABORT_DECODING,
};

extern int ExitAllThreads;
extern PlayerCommands PlayerCommand;
void LoopListenConsole();
void Sleep_(int MS);