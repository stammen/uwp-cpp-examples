#include "pch.h"
#include "Dll1.h"
#include <iostream>

DLL_EXPORT void dll_cout_test()
{
    std::cout << "***DLL1: Hello world!***" << std::endl;
    std::wcout << L"***DLL1: Hello wide world!***" << std::endl;
}
