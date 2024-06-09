#pragma once

#include <nds.h>

class nds_input
{
public:
    static void Initialize();
    static void ScanPads();

    static bool Exit();
    static bool Pause();
    static bool NewGame();

    static bool LaunchBallDown();
    static bool LaunchBallUp();

    static bool MoveLeftPaddleDown();
    static bool MoveLeftPaddleUp();
    static bool MoveRightPaddleDown();
    static bool MoveRightPaddleUp();

    static bool NudgeLeftDown();
    static bool NudgeLeftUp();
    static bool NudgeRightDown();
    static bool NudgeRightUp();
    static bool NudgeUpDown();
    static bool NudgeUpUp();

    static bool SkipError();

private:
    static uint32_t ndsButtonsDown;
    static uint32_t ndsButtonsUp;
    static uint32_t ndsButtonsHeld;
};