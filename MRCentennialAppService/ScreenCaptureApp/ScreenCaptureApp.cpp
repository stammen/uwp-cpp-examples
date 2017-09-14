// ScreenCaptureApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ScreenCapture.h"



// Global Variables:
HINSTANCE hInst;                                // current instance


[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
 
    ScreenCapture sc;
    int result = sc.Run();
    return result;
}

