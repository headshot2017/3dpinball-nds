#include "nds_input.h"

uint32_t nds_input::ndsButtonsDown = 0;
uint32_t nds_input::ndsButtonsUp = 0;
uint32_t nds_input::ndsButtonsHeld = 0;

void nds_input::Initialize()
{
	
}

void nds_input::ScanPads()
{
	scanKeys();
	ndsButtonsDown = keysDown();
	ndsButtonsUp = keysUp();
	ndsButtonsHeld = keysHeld();
}

bool nds_input::Exit()
{
	return (ndsButtonsHeld & KEY_START) && (ndsButtonsHeld & KEY_SELECT);
}

bool nds_input::Pause()
{
	return ndsButtonsDown & KEY_START;
}

bool nds_input::NewGame()
{
	return ndsButtonsDown & KEY_SELECT;
}

bool nds_input::LaunchBallDown()
{
	return ndsButtonsDown & KEY_A;
}

bool nds_input::LaunchBallUp()
{
	return ndsButtonsUp & KEY_A;
}

bool nds_input::MoveLeftPaddleDown()
{
	return (ndsButtonsDown & KEY_L) || (!(ndsButtonsHeld & KEY_Y) && (ndsButtonsDown & KEY_LEFT));
}

bool nds_input::MoveLeftPaddleUp()
{
	return (ndsButtonsUp & KEY_L) || (!(ndsButtonsHeld & KEY_Y) && (ndsButtonsUp & KEY_LEFT));
}

bool nds_input::MoveRightPaddleDown()
{
	return (ndsButtonsDown & KEY_R) || (!(ndsButtonsHeld & KEY_Y) && (ndsButtonsDown & KEY_RIGHT));
}

bool nds_input::MoveRightPaddleUp()
{
	return (ndsButtonsUp & KEY_R) || (!(ndsButtonsHeld & KEY_Y) && (ndsButtonsUp & KEY_RIGHT));
}

bool nds_input::NudgeLeftDown()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsDown & KEY_LEFT);
}

bool nds_input::NudgeLeftUp()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsUp & KEY_LEFT);
}

bool nds_input::NudgeRightDown()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsDown & KEY_RIGHT);
}

bool nds_input::NudgeRightUp()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsUp & KEY_RIGHT);
}

bool nds_input::NudgeUpDown()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsDown & KEY_UP);
}

bool nds_input::NudgeUpUp()
{
	return (ndsButtonsHeld & KEY_Y) && (ndsButtonsUp & KEY_UP);
}

bool nds_input::SkipError()
{
	return ndsButtonsDown & KEY_A;
}