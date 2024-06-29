#include "pch.h"
#include "fullscrn.h"


#include "options.h"
#include "pb.h"
#include "render.h"
#include "winmain.h"


int fullscrn::screen_mode;
int fullscrn::display_changed;

int fullscrn::resolution = 0;
const resolution_info fullscrn::resolution_array[3] =
{
	{640, 480, 600, 416, 501},
	{800, 600, 752, 520, 502},
	{1024, 768, 960, 666, 503},
};
float fullscrn::ScaleX = 1;
float fullscrn::ScaleY = 1;
s16 fullscrn::OffsetX = 0;
s16 fullscrn::OffsetY = 0;

int fullscrn::GetResolution()
{
	return resolution;
}

void fullscrn::SetResolution(int value)
{
	// stay at lowest resolution (0) on DS
	/*
	if (!pb::FullTiltMode)
		value = 0;
	assertm(value >= 0 && value <= 2, "Resolution value out of bounds");
	resolution = value;
	*/
}

int fullscrn::GetMaxResolution()
{
	return 0;
}
