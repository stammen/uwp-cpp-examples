//
// SecondaryPage.xaml.h
// Declaration of the SecondaryPage class
//

#pragma once

#include "SecondaryPage.g.h"
#include "MainPage.xaml.h"
#include "StepTimer.h"

namespace WebViewCapture
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SecondaryPage sealed
	{
	public:
		SecondaryPage();

    protected:
        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
        void viewButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void TimerTick(Platform::Object^ sender, Platform::Object^ e);
        void Update();
        Windows::UI::Xaml::DispatcherTimer^ m_dispatcherTimer;
        MainPage^ m_mainPage;
        DX::StepTimer m_timer;
        Windows::Foundation::EventRegistrationToken m_timerToken;
    };
}
