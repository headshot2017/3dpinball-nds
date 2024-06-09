#pragma once

enum class BitmapTypes : uint8_t
{
	None = 0,
	RawBitmap = 1,
	DibBitmap = 2,
	Spliced = 3,
};

struct Rgba
{
#ifdef BIG_ENDIAN
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
	uint8_t Alpha;
#else
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
#endif
};

union ColorRgba
{
	ColorRgba() = default;

	explicit ColorRgba(uint32_t color)
		: Color(color)
	{
	}

	explicit ColorRgba(Rgba rgba)
		: rgba(rgba)
	{
	}

	uint32_t Color;
	Rgba rgba;
};

static_assert(sizeof(ColorRgba) == 4, "Wrong size of RGBA color");

struct gdrv_bitmap8
{
	gdrv_bitmap8(int width, int height, bool indexed);
	gdrv_bitmap8(const struct dat8BitBmpHeader& header);
	~gdrv_bitmap8();
	void ScaleIndexed(float scaleX, float scaleY);
	ColorRgba* BmpBufPtr1;
	char* IndexedBmpPtr;
	int Width;
	int Height;
	int Stride;
	int IndexedStride;
	BitmapTypes BitmapType;
	int XPosition;
	int YPosition;
	unsigned Resolution;
};


class gdrv
{
public:
	static int display_palette(ColorRgba* plt);
	static void fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, uint8_t fillChar);
	static void copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
	                        int srcXOff, int srcYOff);
	static void copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
	                                       gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff);
	static void grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6);
	static void ApplyPalette(gdrv_bitmap8& bmp);
private:
	static ColorRgba current_palette[256];
};
