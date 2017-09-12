// ScreenCapture.cpp : Defines the exported functions for the DLL application.
//

#pragma once

#if defined(_USRDLL)
    #define DllExport     __declspec(dllexport)
#else
    #define DllExport     __declspec(dllimport)
#endif

DllExport void ScreenCapture_GetScreenSize(int& width, int& height);
DllExport int ScreenCapture_Capture(void* buffer, int width, int height);
