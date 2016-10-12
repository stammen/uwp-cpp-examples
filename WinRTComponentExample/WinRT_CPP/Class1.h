#pragma once

#include <collection.h>
#include <ppl.h>
#include <amp.h>
#include <amp_math.h>

namespace WinRT_CPP
{
    public delegate void PrimeFoundHandler(int result);
    
    public ref class Class1 sealed
    {
    public:
        Class1();

        // Synchronous method.
        Windows::Foundation::Collections::IVector<double>^  ComputeResult(double input);

        // Asynchronous methods
        Windows::Foundation::IAsyncOperationWithProgress<Windows::Foundation::Collections::IVector<int>^, double>^
            GetPrimesOrdered(int first, int last);
        Windows::Foundation::IAsyncActionWithProgress<double>^ GetPrimesUnordered(int first, int last);

        // Event whose type is a delegate "class"
        event PrimeFoundHandler^ primeFoundEvent;

    private:
        bool is_prime(int n);
        Windows::UI::Core::CoreDispatcher^ m_dispatcher;
    };
}
