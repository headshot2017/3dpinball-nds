#pragma once

#include <nds.h>

class ndsfb_graphics
{
private:
    static bool isConsoleInitialized;
	static int bgMainID;
	static int bgSubID;

public:
    static void Initialize();
    static void SwapBuffers();

    static void InitializeConsole();

	static int getBgMain() {return bgMainID;}
	static int getBgSub() {return bgSubID;}
};