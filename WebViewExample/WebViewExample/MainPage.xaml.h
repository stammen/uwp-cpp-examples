//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace WebViewExample
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
    public ref class MainPage sealed
	{
	public:
		MainPage();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void webView_NavigationStarting(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs^ args);

        void JavaScriptCallbackHandler(WindowsRuntimeComponent1::Class1^ sender, Platform::String^ args);

        WindowsRuntimeComponent1::Class1^ m_helper;
        Windows::Foundation::EventRegistrationToken m_webToken;
    };
}
