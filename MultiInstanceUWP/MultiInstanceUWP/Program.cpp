#include "pch.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::System;
using namespace Windows::Security::Cryptography;

// This project includes DISABLE_XAML_GENERATED_MAIN in the build properties,
// which prevents the build system from generating the default Main method:
//int __cdecl main(::Platform::Array<::Platform::String^>^ args)
//{
//    ::Windows::UI::Xaml::Application::Start(
//        ref new ::Windows::UI::Xaml::ApplicationInitializationCallback(
//        [](::Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
//        (void)p; 
//        auto app = ref new ::App1::App();
//    }));
//}

// This example code shows how you could implement the required main method to
// support multi-instance redirection. The minimum requirement is to call
// Application::Start with a new App object. Beyond that, you may delete the
// rest of the example code and replace it with your custom code if you wish.


int __cdecl main(::Platform::Array<::Platform::String^>^ args)
{
    (void)args; // Unused parameter.

    // First, we'll get our activation event args, which are typically richer
    // than the incoming command-line args. We can use these in our app-defined
    // logic for generating the key for this instance.
    IActivatedEventArgs^ activatedArgs = AppInstance::GetActivatedEventArgs();

    Platform::String^ key = L"directxpage";

    if (activatedArgs->Kind == ActivationKind::Protocol)
    {
        ProtocolActivatedEventArgs^ protocolArgs = (ProtocolActivatedEventArgs^)activatedArgs;
        auto queryParsed = protocolArgs->Uri->QueryParsed;
        DirectXPageComponent::ProtocolArgs args(queryParsed);
        key = args.GetParameter(L"apptype", L"directxpage");

        AppInstance^ instance = AppInstance::FindOrRegisterInstanceForKey(key);
        if (instance->IsCurrentInstance)
        {
            // If we successfully registered this instance, we can now just
            // go ahead and do normal XAML initialization.
            ::Windows::UI::Xaml::Application::Start(
                ref new ::Windows::UI::Xaml::ApplicationInitializationCallback(
                    [](::Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
                (void)p; // Unused parameter.
                auto app = ref new ::MultiInstanceUWP::App();
            }));
        }
        else
        {
            // Some other instance has registered for this key, so we'll 
            // redirect this activation to that instance instead.
            instance->RedirectActivationTo();
        }
    }
    else
    {
        // In some scenarios, the platform might indicate a recommended instance.
        // If so, we can redirect this activation to that instance instead, if we wish.
        if (AppInstance::RecommendedInstance != nullptr)
        {
            AppInstance::RecommendedInstance->RedirectActivationTo();
        }
        else
        {
            ::Windows::UI::Xaml::Application::Start(
                ref new ::Windows::UI::Xaml::ApplicationInitializationCallback(
                    [](::Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
                (void)p; // Unused parameter.
                auto app = ref new ::MultiInstanceUWP::App();
            }));
        }
    }

    return 0;
}
