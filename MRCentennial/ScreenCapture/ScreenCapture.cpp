// ScreenCapture.cpp : Defines the exported functions for the DLL application.
//
// Source code derived from https://msdn.microsoft.com/en-us/library/windows/desktop/dd183402(v=vs.85).aspx

#include "stdafx.h"
#include "ScreenCapture.h"

DllExport void ScreenCapture_GetScreenSize(int& width, int& height)
{
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
}


DllExport int ScreenCapture_Capture(void* buffer, int width, int height)
{
    HDC hdcScreen;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);

    // Get the client area for size calculation
    RECT rcClient;
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = GetSystemMetrics(SM_CXSCREEN);
    rcClient.bottom = GetSystemMetrics(SM_CYSCREEN);

    hdcMemDC = CreateCompatibleDC(hdcScreen);

    //This is the best stretch mode
    SetStretchBltMode(hdcScreen, HALFTONE);

    // Create a compatible bitmap from the Window DC
    hbmScreen = CreateCompatibleBitmap(hdcScreen, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

    if (!hbmScreen)
    {
        MessageBox(NULL, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
        0, 0,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hdcScreen,
        0, 0,
        SRCCOPY))
    {
        MessageBox(NULL, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcScreen, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        buffer,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    //Clean up
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    return 0;
}