//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "StepTimer.h"
#include <mutex>
#include <algorithm>

namespace WebViewCapture
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ GetBitmap();
        void Update();
        void PointerReleased(int x, int y);
       
    private:
        void TimerTick(Platform::Object^ sender, Platform::Object^ e);
        Concurrency::task<void> DisplayScaledBitmap(unsigned int width, unsigned int height);
        void viewButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        Windows::UI::Xaml::DispatcherTimer^ m_dispatcherTimer;
        DX::StepTimer m_timer;
        Windows::Graphics::Imaging::BitmapTransform^ m_transform;
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ m_bitmap1;
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ m_bitmap2;
        Platform::Agile<Windows::ApplicationModel::Core::CoreApplicationView> m_secondaryView;
        std::mutex m_mutex;
    };
}
