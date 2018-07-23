// Copyright (c) Microsoft. All rights reserved.

#include "pch.h"
#include "AppActivation.h"
#include "ProtocolArgs.h"
#include "WebViewPage.xaml.h"
#include "DirectXPage.xaml.h"
#include <string>
#include <ppltasks.h>

using namespace DirectXPageComponent;

using namespace Concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Interop;

/// <summary>
/// Invoked when application is launched through protocol.
/// Read more - http://msdn.microsoft.com/library/windows/apps/br224742
/// </summary>
/// <param name="args"></param>
bool AppActivation::OnAppActivated(IActivatedEventArgs^ args)
{
    if (args->Kind == ActivationKind::Protocol)
    {
        Platform::String^ apptype = DirectXPageComponent::DirectXPage::PageName();
        auto eventArgs = safe_cast<ProtocolActivatedEventArgs^>(args);
        auto queryParsed = eventArgs->Uri->QueryParsed;
        ProtocolArgs args(queryParsed);
        apptype = args.GetStringParameter(L"apptype", DirectXPageComponent::DirectXPage::PageName());

        if (apptype != DirectXPageComponent::DirectXPage::PageName())
        {
            InitializePage(DirectXPageComponent::WebViewPage::typeid, eventArgs->Uri);
            return true;
        }
        else
        {
            InitializePage(DirectXPageComponent::DirectXPage::typeid, eventArgs->Uri);
            return true;
        }
    }

    return false;
}

Frame^ AppActivation::CreateRootFrame()
{
    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and navigate to the first page
        rootFrame = ref new Frame();

        // Set the default language
        rootFrame->Language = Windows::Globalization::ApplicationLanguages::Languages->GetAt(0);

        // Place the frame in the current Window
        Window::Current->Content = rootFrame;
    }
    return rootFrame;
}

void AppActivation::InitializePage(Platform::Type^ pageType, Uri^ uri)
{
    //ApplicationView::GetForCurrentView()->SuppressSystemOverlays = false;
    Frame^ rootFrame = CreateRootFrame();
    if (rootFrame->Content == nullptr)
    {
        bool result = rootFrame->Navigate(TypeName(pageType), uri);

        if (!result)
        {
            throw ref new Platform::Exception(-1, "Failed to create XAML page");
        }
    }

    Window::Current->Activate();
}

Concurrency::task<bool> AppActivation::LaunchXamlApp()
{
    auto uri = ref new Uri("stammen-multi-instance-uwp:?id=1234&apptype=xaml"); // The protocol handled by the launched app
    auto options = ref new LauncherOptions();
    return create_task(Launcher::LaunchUriAsync(uri, options));
}