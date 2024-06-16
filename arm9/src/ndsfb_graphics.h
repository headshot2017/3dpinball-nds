#pragma once

#include <nds.h>

class ndsfb_graphics
{
    static bool isConsoleInitialized;
	static bool rotated;
	static int bgMainID;
	static int bgSubID;

public:
    static void Initialize();
    static void SwapBuffers();

	static void AskRotationMode();
	static bool isRotated() {return rotated;}

	static void UpdateFull(bool sub=true);
	static void Update();

    static void SetSubScreenConsole(bool on);

private:
	static void UpdateNormalMode();
	static void UpdateRotatedMode();
};