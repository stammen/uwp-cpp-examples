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
    m_dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([args]()
    {
        // Show the message dialog
        auto msg = ref new Windows::UI::Popups::MessageDialog(args, "Class1::fromJavaScrip");
        // Set the command to be invoked when a user presses 'ESC'
        msg->CancelCommandIndex = 1;
        msg->ShowAsync();
    }));

    auto s = args;
}
