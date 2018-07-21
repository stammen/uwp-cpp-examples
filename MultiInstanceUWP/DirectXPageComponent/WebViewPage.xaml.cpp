//
// WebViewPage.xaml.cpp
// Implementation of the WebViewPage class
//

#include "pch.h"
#include "WebViewPage.xaml.h"
#include <string> 
#include <sstream> 
#include <algorithm>

using namespace DirectXPageComponent;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

WebViewPage::WebViewPage()
{
	InitializeComponent();
    m_transform = ref new BitmapTransform();
    m_webView = ref new WebView(WebViewExecutionMode::SeparateThread);
    m_webView->Source = ref new Windows::Foundation::Uri(L"https://www.microsoft.com");
    m_requestedWebViewWidth = 512;
    m_requestedWebViewHeight = 512;
    m_webView->Width = m_requestedWebViewWidth;
    m_webView->Height = m_requestedWebViewHeight;
    m_webView->Visibility = Windows::UI::Xaml::Visibility::Visible;
    m_webView->NavigationCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^>(this, &WebViewPage::OnWebContentLoaded);
    mainGrid->Children->Append(m_webView);

    m_appServiceListener = ref new AppServiceListener(L"WebView");
    auto connectTask = m_appServiceListener->ConnectToAppService(APPSERVICE_ID, Windows::ApplicationModel::Package::Current->Id->FamilyName);
    connectTask.then([this](AppServiceConnectionStatus response)
    {
        if (response == AppServiceConnectionStatus::Success)
        {
            auto listenerTask = m_appServiceListener->RegisterListener(this).then([this](AppServiceResponse^ response)
            {
                if (response->Status == AppServiceResponseStatus::Success)
                {
                    OutputDebugString(L"WebViewPage is connected to the App Service");
                }
            });
        }
    });
}

void WebViewPage::OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
{
    auto status = args->WebErrorStatus;
    auto success = args->IsSuccess;
    m_webView->Visibility = Windows::UI::Xaml::Visibility::Visible;

    OutputDebugString(L"OnWebContentLoaded");
    UpdateWebView();
}

void WebViewPage::UpdateWebView()
{
    m_timer.Tick([&]()
    {
        UpdateWebViewBitmap(m_requestedWebViewWidth, m_requestedWebViewHeight);
    });
}

task<void> WebViewPage::UpdateWebViewBitmap(unsigned int width, unsigned int height)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

    // capture the WebView
    return create_task(m_webView->CapturePreviewToStreamAsync(stream))
        .then([this, width, height, stream]()
    {
        return create_task(BitmapDecoder::CreateAsync(stream));
    }).then([width, height, this](BitmapDecoder^ decoder)
    {
        // Convert to a bitmap
        m_transform->ScaledHeight = height;
        m_transform->ScaledWidth = width;
        return create_task(decoder->GetPixelDataAsync(
            BitmapPixelFormat::Bgra8,
            BitmapAlphaMode::Straight,
            m_transform,
            ExifOrientationMode::RespectExifOrientation,
            ColorManagementMode::DoNotColorManage))
            .then([width, height, this](PixelDataProvider^ pixelDataProvider)
        {
            Platform::Array<byte>^ pixelData = pixelDataProvider->DetachPixelData();

            WebViewImageInfo^ info = ref new WebViewImageInfo;
            info->PixelData = pixelData;
            info->Format = BitmapPixelFormat::Bgra8;
            info->Width = width;
            info->Height = height;
            info->framesPerSecond = m_timer.GetFramesPerSecond();

#if 0
            if (OnImage != nullptr)
            {
                OnImage(this, info);
            }
#endif
        });
    }).then([this]()
    {
        UpdateWebView();
        std::wstringstream w;
        w << L" FPS:" << m_timer.GetFramesPerSecond() << std::endl;
        OutputDebugString(w.str().c_str());
    });
}


void WebViewPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto scripts = ref new Platform::Collections::Vector<Platform::String^>();
    Platform::String^ ScrollToTopString = L"window.scrollTo(0, 0); ";
    scripts->Append(ScrollToTopString);
    m_webView->InvokeScriptAsync("eval", scripts);
}

ValueSet^ WebViewPage::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    return ref new ValueSet();
}
