//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "Common/StepTimer.h"
#include <vector>
#include <ppltasks.h>
#include <functional>

namespace HolographicWebView
{
    public ref class WebViewImageInfo sealed
    {
    public:
        WebViewImageInfo() {}
        property Platform::Array<byte>^ PixelData;
        property Windows::Graphics::Imaging::BitmapPixelFormat Format;
        property int Width;
        property int Height;
        property int framesPerSecond;
    };


    ref class MainPage;

	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();


        void DisplayWebView(Platform::String^ url, unsigned int width, unsigned int height);

        void OnClick(int x, int y);

    internal:
        static Concurrency::task<MainPage^> CreatePage();
        std::function<void(MainPage^, WebViewImageInfo^)> OnImage;

    private:

        void UpdateWebView();
        Concurrency::task<void> UpdateWebViewBitmap(unsigned int width, unsigned int height);

        void OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args);

        Platform::Agile<Windows::ApplicationModel::Core::CoreApplicationView> m_holographicView;
        DX::StepTimer m_timer;
        Windows::Graphics::Imaging::BitmapTransform^ m_transform;

        int m_requestedWebViewWidth;
        int m_requestedWebViewHeight;
    };
}
