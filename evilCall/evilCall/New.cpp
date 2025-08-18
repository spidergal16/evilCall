#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

#define ERROR -1

using namespace std;
using namespace Gdiplus;

/*
Function gets the class id of a certain image encoder.
input: format - The format of the image.
pClsid - The pointer to the class id.
return: The success of the function.
*/
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0, size = 0, i = 0;
	int status = 0;
	int foundClsid = FALSE;

	ImageCodecInfo* pImage = NULL;

	// Get the size of the image encoder.
	GetImageEncodersSize(&num, &size);
	if (!size)
	{
		status = ERROR;
	}

	// Mallocing a pointer to the image.
	pImage = (ImageCodecInfo*)(malloc(size));
	if (!pImage)
	{
		status = ERROR;
	}

	else
	{
		// Getting all the image encoders.
		GetImageEncoders(num, size, pImage);

		// Finding the right encoder.
		for (i = 0; i < num && !foundClsid; i++)
		{
			if (wcscmp(pImage[i].MimeType, format) == 0)
			{
				*pClsid = pImage[i].Clsid;
				status = i;
				foundClsid = TRUE;
			}
		}
	}

	free(pImage);
	return status;
}

/*
Function captures a screenshot, and saves it.
input: The bitmaps and virtual screens needed to capture the screenshot.
output: none.
*/
void captureScreenShot(Bitmap* bmp, HBITMAP hbmScreen, HDC hdcScreen, HDC hdcMemDC)
{
	CLSID encoderId;

	// Creating the bitmap for the screen shot, and linking it to the screen.
	hbmScreen = CreateCompatibleBitmap(hdcScreen, 1920, 1080);
	SelectObject(hdcMemDC, hbmScreen);
	// Copying the pixel data to the bitmap.
	BitBlt(hdcMemDC, 0, 0, 1920, 1080, hdcScreen, 0, 0, SRCCOPY);

	// Getting the class id of the image encoder.
	GetEncoderClsid(L"image/png", &encoderId);

	// Creating a new bitmap, and saving it.
	bmp = new Bitmap(hbmScreen, (HPALETTE)0);
	bmp->Save(L"Test3.png", &encoderId, NULL);
}

int main()
{
	Bitmap* bmp = NULL;
	HBITMAP hbmScreen = NULL;
	GdiplusStartupInput gdip = NULL;
	ULONG_PTR gdipToken = NULL;

	// Get context of the screen, and creating a virtual screen.
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

	// Initializing the GDI engine.
	GdiplusStartup(&gdipToken, &gdip, NULL);

	// Capturing the screenshot.
	captureScreenShot(bmp, hbmScreen, hdcScreen, hdcMemDC);

	// Shutting down the gdi plus engine.
	GdiplusShutdown(gdipToken);

	// Releasing all objects.
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);

	return 0;
}