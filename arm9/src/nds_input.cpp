#include "nds_input.h"
#include "ndsfb_graphics.h"

uint32_t nds_input::ndsButtonsDown = 0;
uint32_t nds_input::ndsButtonsUp = 0;
uint32_t nds_input::ndsButtonsHeld = 0;

static u32 nudgeKey, launchKey, leftKey, rightKey, upKey;

void nds_input::Initialize()
{
	switch (ndsfb_graphics::getRotation())
	{
		case 0: // default
			nudgeKey = KEY_X;
			launchKey = KEY_DOWN | KEY_B;
			leftKey = KEY_LEFT | KEY_Y | KEY_L;
			rightKey = KEY_RIGHT | KEY_A | KEY_R;
			upKey = KEY_UP;
			break;

		case 1: // portrait (90° CCW)
			nudgeKey = KEY_L;
			launchKey = KEY_LEFT;
			leftKey = KEY_UP;
			rightKey = KEY_DOWN;
			upKey = KEY_RIGHT;
			break;

		case 2: // inverted portrait (90° CW)
			nudgeKey = KEY_R;
			launchKey = KEY_A;
			leftKey = KEY_B;
			rightKey = KEY_X;
			upKey = KEY_Y;
			break;
	}
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
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & launchKey);
}

bool nds_input::LaunchBallUp()
{
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & launchKey);
}

bool nds_input::MoveLeftPaddleDown()
{
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & leftKey);
}

bool nds_input::MoveLeftPaddleUp()
{
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & leftKey);
}

bool nds_input::MoveRightPaddleDown()
{
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & rightKey);
}

bool nds_input::MoveRightPaddleUp()
{
	return !(ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & rightKey);
}

bool nds_input::NudgeLeftDown()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & leftKey);
}

bool nds_input::NudgeLeftUp()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & leftKey);
}

bool nds_input::NudgeRightDown()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & rightKey);
}

bool nds_input::NudgeRightUp()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & rightKey);
}

bool nds_input::NudgeUpDown()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsDown & upKey);
}

bool nds_input::NudgeUpUp()
{
	return (ndsButtonsHeld & nudgeKey) && (ndsButtonsUp & upKey);
}

bool nds_input::SkipError()
{
	return ndsButtonsDown & KEY_A;
}