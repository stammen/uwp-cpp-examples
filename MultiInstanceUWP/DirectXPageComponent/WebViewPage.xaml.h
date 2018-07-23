//
// WebViewPage.xaml.h
// Declaration of the WebViewPage class
//

#pragma once

#include "WebViewPage.g.h"
#include "Common/StepTimer.h"
#include "Common\DeviceResources.h"
#include "AppServiceListener.h"
#include "ProtocolArgs.h"
#include <memory>
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
        static Platform::String^ PageName();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void UpdateWebView();
        void UpdateWebViewBitmap(unsigned int width, unsigned int height);
        void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
        void OnNavigatedStarting(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs^ args);
        void OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args);
        void CreateWebView(Windows::Foundation::Collections::ValueSet^ info);
        void CreateDirectxTextures();
        void UpdateDirectxTextures(const void *buffer, int width, int height);
        void OnClick(int x, int y);
        void OnScroll(int x, int y);
        void GetOffsets();

        Windows::UI::Xaml::Controls::WebView^ m_webView;
        DX::StepTimer m_timer;
        Windows::Graphics::Imaging::BitmapTransform^ m_transform;
        AppServiceListener^ m_appServiceListener;
        std::shared_ptr<DX::DeviceResources> m_deviceResources;
        Microsoft::WRL::ComPtr<ID3D11Resource>  m_quadTexture;
        Microsoft::WRL::ComPtr<ID3D11Resource>  m_stagingTexture;
        int m_width;
        int m_height;
        Platform::String^ m_sharedTextureHandleName;
        Platform::String^ m_id;
        unsigned int m_sleepInterval;
        unsigned int m_fps;
        bool m_contentLoaded;
        bool m_pointerTracking;
        Windows::Foundation::Point m_startPointerPosition;
        Windows::Foundation::Point m_currentPointerPosition;
    };
}
