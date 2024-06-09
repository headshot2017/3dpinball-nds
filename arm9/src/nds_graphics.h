#pragma once

#include <nds.h>

class nds_graphics
{
private:
    static bool isConsoleInitialized;

public:
    static void Initialize();
    static void LoadOrthoProjectionMatrix(float top, float bottom, float left, float right, float near, float far);
    static void Load2DModelViewMatrix(float x, float y);
    static int CreateTexture(uint16_t *textureData, uint16_t width, uint16_t height);
	static void DrawQuad(int texID, float top, float bottom, float left, float right, float uvTop, float uvBottom, float uvLeft, float uvRight);
    static void FlushDataCache(void *startAddress, uint32_t size);
    static void SwapBuffers();

    static void InitializeConsole();
};