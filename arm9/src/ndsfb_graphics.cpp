#include "ndsfb_graphics.h"

#include <cstdio>
#include <cstring>
#include <malloc.h>

bool ndsfb_graphics::isConsoleInitialized = false;
int ndsfb_graphics::bgMainID = 0;
int ndsfb_graphics::bgSubID = 0;

void ndsfb_graphics::Initialize()
{
	videoSetMode(MODE_FB0);
	//videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_LCD);
	//vramSetBankC(VRAM_C_SUB_BG);

	//bgSubID = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);

	ndsfb_graphics::InitializeConsole();
}

void ndsfb_graphics::SwapBuffers()
{
	swiWaitForVBlank();
}

void ndsfb_graphics::InitializeConsole()
{
	if (isConsoleInitialized)
		return;

	vramSetBankH(VRAM_H_SUB_BG);
	videoSetModeSub(MODE_0_2D);

	consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 14, 0, false, true);
	consoleDebugInit(DebugDevice_NOCASH);

	isConsoleInitialized = true;
}