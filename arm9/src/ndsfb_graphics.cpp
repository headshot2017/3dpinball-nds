#include "ndsfb_graphics.h"
#include "pch.h"
#include "maths.h"
#include "render.h"
#include "dsi.h"

#include "bg_screenmode.h"
#include "bg_screenmode2.h"
#include "bg_screenmode3.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <malloc.h>

typedef void (*voidCallback)();

static int tableStartPos1 = (0) * 256 + (256-1-16); // pinball table start position for rotated screen mode
static int tableStartPos2 = (192-1-0) * 256 + (16); // pinball table start position for inv. rotated screen mode
static std::array<voidCallback, 3> rotationCb;


bool ndsfb_graphics::isConsoleInitialized = false;
int ndsfb_graphics::rotation = 0;
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

	rotationCb[0] = UpdateNormalMode;
	rotationCb[1] = UpdateRotatedMode;
	rotationCb[2] = UpdateInvRotatedMode;
}

void ndsfb_graphics::SwapBuffers()
{
	swiWaitForVBlank();
}

void ndsfb_graphics::AskRotationMode()
{
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	int bgID = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	const unsigned* bgTiles[] = {bg_screenmodeTiles, bg_screenmode2Tiles, bg_screenmode3Tiles};
	const u16* bgMap[] = {bg_screenmodeMap, bg_screenmode2Map, bg_screenmode3Map};
	const u16* bgPal[] = {bg_screenmodePal, bg_screenmode2Pal, bg_screenmode3Pal};
	u32 bgTilesLen[] = {bg_screenmodeTilesLen, bg_screenmode2TilesLen, bg_screenmode3TilesLen};

	bool selected = false;
	int selection = 0;

	while (!selected)
	{
		rotation = selection;

		dmaCopy(bgTiles[selection], bgGetGfxPtr(bgID), bgTilesLen[selection]);
		dmaCopy(bgMap[selection], bgGetMapPtr(bgID), 1536);
		dmaCopy(bgPal[selection], BG_PALETTE_SUB, 512);

		dmaFillHalfWords(0, VRAM_A, 256*192*2);
		UpdateFull(false);

		while (1)
		{
			scanKeys();
			int key = keysDown();
			if (key & KEY_RIGHT)
			{
				selection = (selection+1) % 3;
				break;
			}
			if (key & KEY_LEFT)
			{
				selection--;
				if (selection < 0) selection += 3;
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
	if (rotation == 0) // default rotation
	{
		// Full bitmap
		for (int y = 0; y < 192; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				int px = f32toint( mulf32( divf32( inttof32(x), inttof32(256) ), inttof32(render::vscreen->Width) ) );
				int py = f32toint( mulf32( divf32( inttof32(y), inttof32(192) ), inttof32(render::vscreen->Height) ) );

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
				VRAM_A[y * 256 + x] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}
		return;
	}

	if (rotation == 1) // rotated 90° CCW
	{
		// Rotated 90° CCW index: (x) * 256 + (256 - 1 - y)

		// Table bitmap
		int vwidth = 360;
		if (!dsi::isDSi()) vwidth /= 2;

		for (int y = 0; y < 224; y++)
		{
			for (int x = 0; x < 360; x++)
			{
				int px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(vwidth) ) );
				int py = f32toint( mulf32( divf32( inttof32(y), inttof32(224) ), inttof32(render::vscreen->Height) ) );

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
				int ind = tableStartPos1 + ((x-3) * 256 + (256-1-y));
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
				int px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(render::vscreen->Width-vwidth) ) );
				int py = f32toint( mulf32( divf32( inttof32(y), inttof32(256) ), inttof32(render::vscreen->Height) ) );
				u16* vram_ptr = bgGetGfxPtr(bgSubID);

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
				int ind = (x-328) * 256 + (256-1-y);
				vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}

		return;
	}

	// Inverted rotation (90° CW) index: (192 - 1 - x) * 256 + (y)

	// Info bitmap
	int vwidth = 380;
	if (!dsi::isDSi()) vwidth /= 2;

	for (int y = 0; y < 256; y++)
	{
		for (int x = 328; x < 328+192; x++)
		{
			int px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(render::vscreen->Width-vwidth) ) );
			int py = f32toint( mulf32( divf32( inttof32(y), inttof32(256) ), inttof32(render::vscreen->Height) ) );

			Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
			int ind = (192-1-(x-328)) * 256 + (y);
			VRAM_A[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
		}
	}

	if (!sub) return;

	// Table bitmap
	vwidth = 360;
	if (!dsi::isDSi()) vwidth /= 2;

	for (int y = 0; y < 224; y++)
	{
		for (int x = 0; x < 360; x++)
		{
			int px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(vwidth) ) );
			int py = f32toint( mulf32( divf32( inttof32(y), inttof32(224) ), inttof32(render::vscreen->Height) ) );
			u16* vram_ptr = bgGetGfxPtr(bgSubID);

			Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
			int ind = tableStartPos2 - ((x-2) * 256) + (y+1);
			vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
		}
	}
}

void ndsfb_graphics::Update()
{
	// update dirty bitmap regions, depending on rotation mode
	rotationCb[rotation]();

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

		dirty.XPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.XPosition), inttof32(render::vscreen->Width) ), inttof32(256) ) ) )-1;
		dirty.YPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.YPosition), inttof32(render::vscreen->Height) ), inttof32(192) ) ) )-1;
		dirty.Width = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Width), inttof32(render::vscreen->Width) ), inttof32(256) ) ) )+1;
		dirty.Height = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Height), inttof32(render::vscreen->Height) ), inttof32(192) ) ) )+1;

		for (int y = dirty.YPosition; y < dirty.YPosition+dirty.Height; y++)
		{
			for (int x = dirty.XPosition; x < dirty.XPosition+dirty.Width; x++)
			{
				int px = f32toint( mulf32( divf32( inttof32(x), inttof32(256) ), inttof32(render::vscreen->Width) ) );
				int py = f32toint( mulf32( divf32( inttof32(y), inttof32(192) ), inttof32(render::vscreen->Height) ) );
				if (px >= render::vscreen->Width) px = render::vscreen->Width-1;
				if (py >= render::vscreen->Height) px = render::vscreen->Height-1;

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;
				VRAM_A[y * 256 + x] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
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
		u16* vram_ptr = (dirty.XPosition < xPosCheck) ? VRAM_A : bgGetGfxPtr(bgSubID);

		if (vram_ptr == VRAM_A)
		{
			dirty.XPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.XPosition), inttof32(vwidthTable) ), inttof32(192) ) ) )-1;
			dirty.YPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.YPosition), inttof32(render::vscreen->Height) ), inttof32(224) ) ) )-1;
			dirty.Width = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Width), inttof32(vwidthTable) ), inttof32(192) ) ) )+1;
			dirty.Height = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Height), inttof32(render::vscreen->Height) ), inttof32(224) ) ) )+1;
		}
		else
		{
			dirty.XPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.XPosition), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) )-1;
			dirty.YPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.YPosition), inttof32(render::vscreen->Height) ), inttof32(256) ) ) )-1;
			dirty.Width = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Width), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) )+1;
			dirty.Height = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Height), inttof32(render::vscreen->Height) ), inttof32(256) ) ) )+1;
		}

		for (int y = dirty.YPosition; y < dirty.YPosition+dirty.Height; y++)
		{
			for (int x = dirty.XPosition; x < dirty.XPosition+dirty.Width; x++)
			{
				int px, py, ind;

				if (vram_ptr == VRAM_A)
				{
					px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(vwidthTable) ) );
					py = f32toint( mulf32( divf32( inttof32(y), inttof32(224) ), inttof32(render::vscreen->Height) ) );
					if (px >= vwidthTable) px = vwidthTable-1;
					if (py >= render::vscreen->Height) py = render::vscreen->Height-1;
				}
				else
				{
					px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(render::vscreen->Width-vwidthInfo) ) );
					py = f32toint( mulf32( divf32( inttof32(y), inttof32(256) ), inttof32(render::vscreen->Height) ) );
				}

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;

				ind = (vram_ptr == VRAM_A) ?
					tableStartPos1 + ((x-3) * 256 + (256-1-y)) :
					(x-328) * 256 + (256-1-y);
				vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}
	}
}

void ndsfb_graphics::UpdateInvRotatedMode()
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
		u16* vram_ptr = (dirty.XPosition < xPosCheck) ? bgGetGfxPtr(bgSubID) : VRAM_A;

		if (vram_ptr != VRAM_A)
		{
			dirty.XPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.XPosition), inttof32(vwidthTable) ), inttof32(192) ) ) )-1;
			dirty.YPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.YPosition), inttof32(render::vscreen->Height) ), inttof32(224) ) ) )-1;
			dirty.Width = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Width), inttof32(vwidthTable) ), inttof32(192) ) ) )+1;
			dirty.Height = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Height), inttof32(render::vscreen->Height) ), inttof32(224) ) ) )+1;
		}
		else
		{
			dirty.XPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.XPosition), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) )-1;
			dirty.YPosition = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.YPosition), inttof32(render::vscreen->Height) ), inttof32(256) ) ) )-1;
			dirty.Width = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Width), inttof32(render::vscreen->Width-vwidthInfo) ), inttof32(192) ) ) )+1;
			dirty.Height = f32toint( maths::ceilf32(mulf32( divf32( inttof32(dirty.Height), inttof32(render::vscreen->Height) ), inttof32(256) ) ) )+1;
		}

		for (int y = dirty.YPosition; y < dirty.YPosition+dirty.Height; y++)
		{
			for (int x = dirty.XPosition; x < dirty.XPosition+dirty.Width; x++)
			{
				int px, py, ind;

				if (vram_ptr != VRAM_A)
				{
					px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(vwidthTable) ) );
					py = f32toint( mulf32( divf32( inttof32(y), inttof32(224) ), inttof32(render::vscreen->Height) ) );
					if (px >= vwidthTable) px = vwidthTable-1;
					if (py >= render::vscreen->Height) py = render::vscreen->Height-1;
				}
				else
				{
					px = f32toint( mulf32( divf32( inttof32(x), inttof32(192) ), inttof32(render::vscreen->Width-vwidthInfo) ) );
					py = f32toint( mulf32( divf32( inttof32(y), inttof32(256) ), inttof32(render::vscreen->Height) ) );
				}

				Rgba color = render::vscreen->BmpBufPtr1[py * render::vscreen->Width + px].rgba;

				ind = (vram_ptr == VRAM_A) ?
					(192-1-(x-328)) * 256 + (y) :
					tableStartPos2 - ((x-2) * 256) + (y+1);
				vram_ptr[ind] = (!color.Alpha) ? 0 : ARGB16(1, color.Blue>>3, color.Green>>3, color.Red>>3);
			}
		}
	}
}
