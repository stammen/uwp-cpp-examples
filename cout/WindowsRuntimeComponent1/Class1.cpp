#include "pch.h"
#include "Class1.h"

#include <iostream>


using namespace WindowsRuntimeComponent1;
using namespace Platform;

Class1::Class1()
{
    std::cout << "***WindowsRuntimeComponent1::Class1: Hello world!***" << std::endl;
    std::wcout << L"***WindowsRuntimeComponent1::Class1: Hello wide world!***" << std::endl;
}
