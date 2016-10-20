#pragma once

#if defined(_WINDLL)
#define DLL_EXPORT     __declspec(dllexport)
#else         /* use a DLL library */
#define DLL_EXPORT		__declspec(dllimport)
#endif

DLL_EXPORT void dll_cout_test();
