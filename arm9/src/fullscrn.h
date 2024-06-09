#pragma once

struct resolution_info
{
	int16_t ScreenWidth;
	int16_t ScreenHeight;
	int16_t TableWidth;
	int16_t TableHeight;
	int16_t ResolutionMenuId;
};

class fullscrn
{
public:
	static int screen_mode;
	static int display_changed;
	static const resolution_info resolution_array[3];
	static float ScaleX;
	static float ScaleY;
	static int16_t OffsetX;
	static int16_t OffsetY;

	static int GetResolution();
	static void SetResolution(int value);
	static int GetMaxResolution();
private :
	static int resolution;
};
