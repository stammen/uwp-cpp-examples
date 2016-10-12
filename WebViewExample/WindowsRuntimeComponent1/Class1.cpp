#include "pch.h"
#include "Class1.h"

using namespace WindowsRuntimeComponent1;
using namespace Platform;
using namespace Windows::UI::Core;

Class1::Class1()
{
    auto window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    m_dispatcher = window->Dispatcher;
}

void Class1::stringFromJavaScript(Platform::String^ args)
{
    m_dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, args]()
    {
        OnJavaScriptCallback(this, args);
    }));
}

void Class1::boolFromJavaScript(Platform::Boolean arg)
{
    m_dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, arg]()
    {
        OnJavaScriptCallback(this, "boolFromJavaScript");
    }));
}

