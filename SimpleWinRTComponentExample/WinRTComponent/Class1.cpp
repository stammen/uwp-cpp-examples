#include "pch.h"
#include "Class1.h"

using namespace WinRTComponent;
using namespace Platform;

Class1::Class1()
{
}

Platform::String^ Class1::GetString()
{
    return ref new Platform::String(L"GetString");
}

