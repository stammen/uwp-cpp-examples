//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "Common/StepTimer.h"
#include <mutex>
#include <vector>

namespace HolographicWebView
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
        void DisplayWebView(Platform::String^ url, unsigned int width, unsigned int height);

    internal:
        static const std::vector<byte>& GetBitmap();

    private:
        void button1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    private:
        void TimerTick(Platform::Object^ sender, Platform::Object^ e);
        void StartTimer();
        void StopTimer();
        Concurrency::task<void> UpdateWebViewBitmap(unsigned int width, unsigned int height);
        void OnDOMContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewDOMContentLoadedEventArgs ^ args);


        Platform::Agile<Windows::ApplicationModel::Core::CoreApplicationView> m_holographicView;
        DX::StepTimer m_timer;
        Windows::UI::Xaml::DispatcherTimer^ m_dispatcherTimer;
        Windows::Graphics::Imaging::BitmapTransform^ m_transform;
        static std::vector<byte> s_bitmap1;
        static std::vector<byte> s_bitmap2;
        static std::mutex s_mutex;
    };
}
