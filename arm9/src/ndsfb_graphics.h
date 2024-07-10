#pragma once

#include <nds.h>

class ndsfb_graphics
{
    static bool isConsoleInitialized;
	static int rotation;
	static int bgMainID;
	static int bgSubID;

public:
    static void Initialize();
    static void SwapBuffers();

	static void AskRotationMode();
	static int getRotation() {return rotation;}

	static void UpdateFull(bool sub=true);
	static void Update();

    static void SetSubScreenConsole(bool on);

private:
	static void UpdateNormalMode();
	static void UpdateRotatedMode();
	static void UpdateInvRotatedMode();
};