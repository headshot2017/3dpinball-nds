#include "ndsfb_graphics.h"
#include "pch.h"
#include "maths.h"
#include "render.h"
#include "dsi.h"

#include <cstdio>
#include <cstring>
#include <malloc.h>

static int tableStartPos = (0) * 256 + (256-1-16); // pinbal table start position for rotated screen mode


bool ndsfb_graphics::isConsoleInitialized = false;
bool ndsfb_graphics::rotated = false;
int ndsfb_graphics::bgMainID = 0;
int ndsfb_graphics::bgSubID = 0;

void ndsfb_graphics::Initialize()
{
	// framebuffer mode is not supported on sub screen,
	// so use a Bmp background mode instead which achieves
	// the same effect

	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_LCD);
	vramSetBankC(VRAM_C_SUB_BG);
}

void ndsfb_graphics::SwapBuffers()
{
	swiWaitForVBlank();
}

void ndsfb_graphics::AskRotationMode()
{
	ndsfb_graphics::SetSubScreenConsole(true);

	bool selected = false;
	int selection = 0;

	const char* items[2] = {"Normal", "90° rotation CCW"};
	const char* controls[2] = {
		"A = Launch ball\nDPad or L/R buttons = Paddles\nY + DPad = Nudge/tilt table\nStart = Pause\nSelect = New game\nStart + Select = Exit",
		"DPad Up = Launch ball\nDPad Left/Right = Paddles\nL + DPad = Nudge/tilt table\nStart = Pause\nSelect = New game\nStart + Select = Exit",
	};

	while (!selected)
	{
		rotated = (selection == 1);

		consoleClear();
		printf("Select screen mode with DPad\nPress A to confirm\n\n");
		for (int i=0; i<2; i++)
			printf("%s %s\n", (selection == i) ? "->" : "  ", items[i]);
		printf("\n\n\n%s", controls[selection]);

		dmaFillHalfWords(0, VRAM_A, 256*192*2);
		UpdateFull(false);

		while (1)
		{
			scanKeys();
			int key = keysDown();
			if (key & KEY_DOWN)
			{
				selection = (selection+1) % 2;
				break;
			}
			if (key & KEY_UP)
			{
				selection = (selection-1) % 2;
				if (selection < 0) selection = -selection;
				break;
			}
			if (key & KEY_A)
			{
				selected = true;
				break;
			}
			swiWaitForVBlank();
		}
	}
}

void ndsfb_graphics::UpdateFull(bool sub)
{
	// copy full bitmap to screen, depending on rotation mode
	if (!rotated)
	{
		// Full bitmap
		for (int y = 0; y < 192; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				int smallX = f32toint( mulf32( divf32( inttof32(x), inttof32(256) ), inttof32(render::vscreen->Width) ) );
				int smallY = f32toint( mulf32( divf32( inttof32(y), inttof32(192) ), inttof32(render::vscreen->Height) ) );

				Rgba color = render::vscreen->BmpBufPtr1[smallY * render::vscreen->Width + smallX].rgba;
				VRAM_A[y * 256 + x] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}
		return;
	}

	// Rotated 90° CCW index: (x) * 256 + (256 - 1 - y)

	// Table bitmap
	int vwidth = 360;
	if (!dsi::isDSi()) vwidth /= 2;

	for (int y = 0; y < 224; y++)
	{
		for (int x = 0; x < 360; x++)
		{
			int smallX = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(vwidth) ) );
			int smallY = f32toint( mulf32( divf32( inttof32(y), inttof32(224) ), inttof32(render::vscreen->Height) ) );

			Rgba color = render::vscreen->BmpBufPtr1[smallY * render::vscreen->Width + smallX].rgba;
			int ind = tableStartPos + ((x-3) * 256 + (256-1-y));
			VRAM_A[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
		}
	}

	if (!sub) return;

	// Info bitmap
	vwidth = 380;
	if (!dsi::isDSi()) vwidth /= 2;
	for (int y = 0; y < 256; y++)
	{
		for (int x = 328; x < 328+192; x++)
		{
			int smallX = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(render::vscreen->Width-vwidth) ) );
			int smallY = f32toint( mulf32( divf32( inttof32(y), inttof32(256) ), inttof32(render::vscreen->Height) ) );
			u16* vram_ptr = bgGetGfxPtr(bgSubID);

			Rgba color = render::vscreen->BmpBufPtr1[smallY * render::vscreen->Width + smallX].rgba;
			int ind = (x-328) * 256 + (256-1-y);
			vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
		}
	}
}

void ndsfb_graphics::Update()
{
	// update dirty bitmap regions, depending on rotation mode
	if (!rotated)
		UpdateNormalMode();
	else
		UpdateRotatedMode();

	render::get_dirty_regions().clear();
}

void ndsfb_graphics::SetSubScreenConsole(bool on)
{
	videoSetModeSub(on ? MODE_0_2D : MODE_5_2D);

	vramSetBankC(VRAM_C_LCD);
	dmaFillHalfWords(0, VRAM_C, 256*192*2);
	vramSetBankC(VRAM_C_SUB_BG);

	if (on)
		consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 14, 0, false, true);
	else
		bgSubID = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);

	isConsoleInitialized = on;
}

void ndsfb_graphics::UpdateNormalMode()
{
	for (u32 i=0; i<render::get_dirty_regions().size(); i++)
	{
		rectangle_type dirty = render::get_dirty_regions()[i];

		for (int y = dirty.YPosition; y < dirty.YPosition+dirty.Height; y++)
		{
			for (int x = dirty.XPosition; x < dirty.XPosition+dirty.Width; x++)
			{
				int smallX = f32toint( maths::ceilf32(mulf32( divf32( inttof32(x), inttof32(render::vscreen->Width) ), inttof32(256) ) ) );
				int smallY = f32toint( maths::ceilf32(mulf32( divf32( inttof32(y), inttof32(render::vscreen->Height) ), inttof32(192) ) ) );

				Rgba color = render::vscreen->BmpBufPtr1[y * render::vscreen->Width + x].rgba;
				VRAM_A[smallY * 256 + (smallX)] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}
	}
}

void ndsfb_graphics::UpdateRotatedMode()
{
	int vwidthTable = 360;
	int vwidthInfo = 380;
	int xPosCheck = 370;
	if (!dsi::isDSi())
	{
		vwidthTable /= 2;
		vwidthInfo /= 2;
		xPosCheck /= 2;
	}

	for (u32 i=0; i<render::get_dirty_regions().size(); i++)
	{
		rectangle_type dirty = render::get_dirty_regions()[i];

		for (int y = dirty.YPosition; y < dirty.YPosition+dirty.Height; y++)
		{
			for (int x = dirty.XPosition; x < dirty.XPosition+dirty.Width; x++)
			{
				int smallX, smallY, prevSmallX, prevSmallY, ind;
				u16* vram_ptr = (dirty.XPosition < xPosCheck) ? VRAM_A : bgGetGfxPtr(bgSubID);

				if (vram_ptr == VRAM_A)
				{
					smallX = f32toint( maths::ceilf32(mulf32( divf32( inttof32(x), inttof32(vwidthTable) ), inttof32(192) ) ) );
					smallY = f32toint( maths::ceilf32(mulf32( divf32( inttof32(y), inttof32(render::vscreen->Height) ), inttof32(224) ) ) );
					prevSmallX = f32toint( maths::ceilf32(mulf32( divf32( inttof32(x-1), inttof32(vwidthTable) ), inttof32(192) ) ) );
					prevSmallY = f32toint( maths::ceilf32(mulf32( divf32( inttof32(y-1), inttof32(render::vscreen->Height) ), inttof32(224) ) ) );
				}
				else
				{
					smallX = f32toint( maths::ceilf32(mulf32( divf32( inttof32(x), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) );
					smallY = f32toint( maths::ceilf32(mulf32( divf32( inttof32(y), inttof32(render::vscreen->Height) ), inttof32(256) ) ) );
					prevSmallX = f32toint( maths::ceilf32(mulf32( divf32( inttof32(x-1), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) );
					prevSmallY = f32toint( maths::ceilf32(mulf32( divf32( inttof32(y-1), inttof32(render::vscreen->Height) ), inttof32(256) ) ) );
				}

				Rgba color = render::vscreen->BmpBufPtr1[y * render::vscreen->Width + x].rgba;
				for (int finalY = prevSmallY+1; finalY <= smallY; finalY++)
				{
					for (int finalX = prevSmallX+1; finalX <= smallX; finalX++)
					{
						ind = (vram_ptr == VRAM_A) ?
							tableStartPos + ((finalX-3) * 256 + (256-1-finalY)) :
							(finalX-328) * 256 + (256-1-finalY);

						vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
					}
				}
			}
		}
	}
}