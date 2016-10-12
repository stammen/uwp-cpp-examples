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

void Class1::fromJavaScript(Platform::String^ args)
{
    m_dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, args]()
    {
        OnJavaScriptCallback(this, args);
    }));
}
