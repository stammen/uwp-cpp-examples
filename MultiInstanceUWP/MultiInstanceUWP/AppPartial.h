// Copyright (c) Microsoft. All rights reserved.

#pragma once 
#include "pch.h"

namespace MultiInstanceUWP
{
    partial ref class App
    {
    private:
        Windows::UI::Xaml::Controls::Frame^ App::CreateRootFrame();
        void InitializePage(Platform::Type^ pageType, Windows::Foundation::Uri^ uri);
        bool OnAppActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
        Concurrency::task<bool> LaunchXamlApp();
    };

}
