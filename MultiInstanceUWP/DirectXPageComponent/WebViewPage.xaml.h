//
// WebViewPage.xaml.h
// Declaration of the WebViewPage class
//

#pragma once

#include "WebViewPage.g.h"
#include "Common/StepTimer.h"
#include "AppServiceListener.h"
#include <ppltasks.h>

namespace DirectXPageComponent
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

	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class WebViewPage sealed : IAppServiceListenerDelegate
	{
	public:
		WebViewPage();
        virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);

    private:
        void UpdateWebView();
        Concurrency::task<void> UpdateWebViewBitmap(unsigned int width, unsigned int height);
        void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args);
        Windows::UI::Xaml::Controls::WebView^ m_webView;
        DX::StepTimer m_timer;
        Windows::Graphics::Imaging::BitmapTransform^ m_transform;
        int m_requestedWebViewWidth;
        int m_requestedWebViewHeight;
        AppServiceListener^ m_appServiceListener;

    };
}
