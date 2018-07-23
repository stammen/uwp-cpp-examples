// Copyright (c) Microsoft. All rights reserved.

#pragma once 
#include "pch.h"

namespace DirectXPageComponent
{
    public ref class AppActivation sealed
    {
    public:
        static bool OnAppActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);

    private:
        static Windows::UI::Xaml::Controls::Frame^ CreateRootFrame();
        static void InitializePage(Platform::Type^ pageType, Windows::Foundation::Uri^ uri);
        static Concurrency::task<bool> LaunchXamlApp();
    };

}
