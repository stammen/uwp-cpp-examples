#include "pch.h"
#include "Lib1.h"

#include <iostream>


void lib_cout_test()
{
    std::cout << "***Lib1: Hello world!***" << std::endl;
    std::wcout << L"***Lib1: Hello wide world!***" << std::endl;
}

