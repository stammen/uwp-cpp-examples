#pragma once

#include <Windows.h>

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP
#define MS_UWP 
#endif

#define DLL_API __declspec(dllexport) 

extern "C" {
    DLL_API bool Initialize();
#ifdef MS_UWP
    DLL_API bool SendMouseInputUWP(MOUSEINPUT* input);
    DLL_API bool SendKeyboardInputUWP(KEYBDINPUT* input);
#else
    DLL_API bool SendInputWin32(INPUT* input, int numInputs);
#endif

}