#include "pch.h"
#include "zdrv.h"
#include "winmain.h"
#include "maths.h"


zmap_header_type::zmap_header_type(int width, int height, int stride)
{
	Resolution = 0;
	Width = width;
	Height = height;
	Stride = stride >= 0 ? stride : pad(width);
	origStride = stride;
	ZPtr1 = new unsigned short[Stride * Height];
}

zmap_header_type::~zmap_header_type()
{
	delete[] ZPtr1;
}

void zmap_header_type::Scale(float scaleX, float scaleY)
{
	int newWidht = static_cast<int>(Width * scaleX), newHeight = static_cast<int>(Height * scaleY);
	if (Width == newWidht && Height == newHeight)
		return;

	int newStride = pad(newWidht);

	auto newZPtr1 = new unsigned short[newHeight * newStride];
	memset(newZPtr1, 0xff, newHeight*newStride*2);
	for (int y = 0; y < Height; y++)
	{
		int smallY = (int)(y / (float)Height * (float)newHeight);

		for (int x = 0; x < Stride; x++)
		{
			int smallX = (int)(x / (float)Stride * (float)newStride);
			newZPtr1[smallY * newStride + smallX] = ZPtr1[(y * Stride) + x];
		}
	}

	Width = newWidht;
	Height = newHeight;
	Stride = newStride;

	delete[] ZPtr1;
	ZPtr1 = newZPtr1;
}

int zmap_header_type::pad(int width)
{
	int result = width;
	if (width & 3)
		result = width - (width & 3) + 4;
	return result;
}


void zdrv::fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, uint16_t fillWord)
{
	auto dstPtr = &zmap->ZPtr1[zmap->Stride * yOff + xOff];
	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{
			*dstPtr++ = fillWord;
		}
		dstPtr += zmap->Stride - width;
	}
}


void zdrv::paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff, zmap_header_type* dstZMap,
                 int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff, int srcBmpYOff,
                 zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff)
{
	assertm(srcBmp->BitmapType != BitmapTypes::Spliced, "Wrong bmp type");

	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcBmpYOff + srcBmpXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * dstBmpYOff + dstBmpXOff];
	auto srcPtrZ = &srcZMap->ZPtr1[srcZMap->Stride * srcZMapYOff + srcZMapXOff];
	auto dstPtrZ = &dstZMap->ZPtr1[dstZMap->Stride * dstZMapYOff + dstZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if (*dstPtrZ >= *srcPtrZ)
			{
				*dstPtr = *srcPtr;
				*dstPtrZ = *srcPtrZ;
			}
			++srcPtr;
			++dstPtr;
			++srcPtrZ;
			++dstPtrZ;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		srcPtrZ += srcZMap->Stride - width;
		dstPtrZ += dstZMap->Stride - width;
	}
}

void zdrv::paint_flat(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
                      zmap_header_type* zMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff,
                      int srcBmpYOff, uint16_t depth)
{
	assertm(srcBmp->BitmapType != BitmapTypes::Spliced, "Wrong bmp type");

	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * dstBmpYOff + dstBmpXOff];
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcBmpYOff + srcBmpXOff];
	auto zPtr = &zMap->ZPtr1[zMap->Stride * dstZMapYOff + dstZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if ((*srcPtr).Color && *zPtr > depth)
			{
				*dstPtr = *srcPtr;
			}
			++srcPtr;
			++dstPtr;
			++zPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		zPtr += zMap->Stride - width;
	}
}

void zdrv::FlipZMapHorizontally(const zmap_header_type& zMap)
{
	// Flip in-place, iterate over Height/2 lines
	auto dst = zMap.ZPtr1;
	auto src = zMap.ZPtr1 + zMap.Stride * (zMap.Height - 1);
	for (auto y = zMap.Height - 1; y >= zMap.Height / 2; y--)
	{
		for (auto x = 0; x < zMap.Width; x++)
		{
			std::swap(*dst++, *src++);
		}
		dst += zMap.Stride - zMap.Width;
		src -= zMap.Stride + zMap.Width;
	}
}
