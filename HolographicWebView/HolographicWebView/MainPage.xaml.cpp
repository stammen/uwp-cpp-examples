//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "AppView.h"
#include <string>
#include <sstream> 
#include <algorithm>
#include <ppltasks.h>
#include <robuffer.h> // IBufferByteAccess

using namespace HolographicWebView;

using namespace Platform;
using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409


MainPage::MainPage()
    : OnImage(nullptr)
{
    InitializeComponent();
    m_transform = ref new BitmapTransform();
    m_webView = ref new WebView(WebViewExecutionMode::SeparateThread);
    m_webView->NavigationCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^>(this, &HolographicWebView::MainPage::OnWebContentLoaded);
    gridMain->Children->Append(m_webView.Get());
}
 
Concurrency::task<MainPage^> MainPage::CreatePage()
{
    task_completion_event<MainPage ^> tce;
    task<MainPage ^> event_set(tce);

    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([tce]()
    {
        Controls::Frame^ frame = ref new Controls::Frame();
        frame->Navigate(TypeName(MainPage::typeid), nullptr);
    
        // Place the frame in the current Window
        Window::Current->Content = frame;
        // Ensure the current window is active
        Window::Current->Activate();

        MainPage^ page = dynamic_cast<MainPage^>(frame->Content);

        tce.set(page);
    }));

    return event_set;
}

void MainPage::DisplayWebView(Platform::String^ url, unsigned int width, unsigned int height)
{
    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, url, width, height]()
    {
        Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri(url);
        m_webView->Visibility = Windows::UI::Xaml::Visibility::Visible;
        m_webView->Source = uri;
        m_webView->Width = width;
        m_webView->Height = height;

        m_requestedWebViewWidth = width;
        m_requestedWebViewHeight = height;

        // need to add code to scale webview zoom to display entire width of webpage inside of webview
    }));
}

void MainPage::OnClick(int x, int y)
{
    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, x, y]()
    {
        auto scripts = ref new Platform::Collections::Vector<Platform::String^>();
        std::wstringstream w;
        w << L"document.elementFromPoint(" << x << "," << y << L").click()";
        scripts->Append(ref new Platform::String(w.str().c_str()));
        m_webView->InvokeScriptAsync(ref new Platform::String(L"eval"), scripts);
    }));
}

void MainPage::OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
{
    UpdateWebView();
}


void MainPage::UpdateWebView()
{
    m_timer.Tick([&]()
    {
        UpdateWebViewBitmap(m_requestedWebViewWidth, m_requestedWebViewHeight);
    });
}

task<void> MainPage::UpdateWebViewBitmap(unsigned int width, unsigned int height)
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
            if (OnImage != nullptr)
            {
                OnImage(this, info);
            }
        });
    }).then([this]()
    {
        UpdateWebView();
        std::wstringstream w;
        w << L" FPS:" << m_timer.GetFramesPerSecond() << std::endl;
        OutputDebugString(w.str().c_str());
    });
}




