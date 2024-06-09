#include "nds_graphics.h"

#include <cstdio>
#include <cstring>
#include <malloc.h>

#define SCALE_VERTICES 1024

bool nds_graphics::isConsoleInitialized = false;

void nds_graphics::Initialize()
{
	videoSetMode(MODE_0_3D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankC(VRAM_C_TEXTURE);
	vramSetBankD(VRAM_D_TEXTURE);

	nds_graphics::InitializeConsole();
	glInit();

	glViewport(0, 0, 255, 191);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
}

void nds_graphics::LoadOrthoProjectionMatrix(float top, float bottom, float left, float right, float near, float far)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, near, far);
}

void nds_graphics::Load2DModelViewMatrix(float x, float y)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef32(inttof32(SCALE_VERTICES), inttof32(SCALE_VERTICES), inttof32(SCALE_VERTICES));
	glTranslatef32(floattof32(x/SCALE_VERTICES), floattof32(y/SCALE_VERTICES), floattof32(-5.f/SCALE_VERTICES));
}

int nds_graphics::CreateTexture(uint16_t *textureData, uint16_t width, uint16_t height)
{
    int texID = 0;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	int success = glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, TEXGEN_TEXCOORD, textureData);
	if (!success)
	{
		printf("FAILED TEXIMAGE2D %d %d %s\n", width, height, textureData?"true":"false");
		return -1;
	}

	return texID;
}

void nds_graphics::DrawQuad(int texID, float top, float bottom, float left, float right, float uvTop, float uvBottom, float uvLeft, float uvRight)
{
	top /= SCALE_VERTICES;
	bottom /= SCALE_VERTICES;
	left /= SCALE_VERTICES;
	right /= SCALE_VERTICES;

	glBindTexture(GL_TEXTURE_2D, texID);

	glBegin(GL_QUADS);

	glColor3b(255,255,255);
	glTexCoord2f(uvLeft, uvTop);
	glVertex3f(left, top, 0);

	glColor3b(255,255,255);
	glTexCoord2f(uvLeft, uvBottom);
	glVertex3f(left, bottom, 0);

	glColor3b(255,255,255);
	glTexCoord2f(uvRight, uvBottom);
	glVertex3f(right, bottom, 0);

	glColor3b(255,255,255);
	glTexCoord2f(uvRight, uvTop);
	glVertex3f(right, top, 0);

	glEnd();
}

void nds_graphics::FlushDataCache(void *startAddress, uint32_t size)
{
	DC_FlushRange(startAddress, size);
}

void nds_graphics::SwapBuffers()
{
	glFlush(0);
}

void nds_graphics::InitializeConsole()
{
	if (isConsoleInitialized)
		return;

	vramSetBankH(VRAM_H_SUB_BG);

	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 14, 0, false, true);
	consoleDebugInit(DebugDevice_NOCASH);

	isConsoleInitialized = true;
}
