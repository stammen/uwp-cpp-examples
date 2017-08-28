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

std::vector<byte> MainPage::s_bitmap1;
std::vector<byte> MainPage::s_bitmap2;

std::mutex MainPage::s_mutex;

MainPage::MainPage()
{
    InitializeComponent();
    m_transform = ref new BitmapTransform();
    TimeSpan span;
    span.Duration = 10000000L / 30L;
    m_dispatcherTimer = ref new DispatcherTimer();
    m_dispatcherTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &MainPage::TimerTick);
    m_dispatcherTimer->Interval = span;
    //webview1 = ref new WebView(WebViewExecutionMode::SeparateThread);
    webview1->NavigationCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^>(this, &HolographicWebView::MainPage::OnWebContentLoaded);

}

void MainPage::button1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto xamlViewId = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Id;

    if (m_holographicView.Get() == nullptr)
    {
        try
        {
            m_holographicView = CoreApplication::CreateNewView(ref new HolographicWebView::AppViewSource());
        }
        catch (Platform::COMException^ e)
        {
            // This exception is thrown if the environment doesn't support holographic content
            statusText->Text = L"Holographic environment not available.";
            return;
        }
    }

    m_holographicView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, xamlViewId]()
    {
        auto viewId = ApplicationView::GetForCurrentView()->Id;
        CoreWindow::GetForCurrentThread()->Activate();
        ApplicationViewSwitcher::SwitchAsync(viewId, xamlViewId);
    }));
}

void MainPage::DisplayWebView(Platform::String^ url, unsigned int width, unsigned int height)
{
    StopTimer();

    Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri(url);
    webview1->Visibility = Windows::UI::Xaml::Visibility::Visible;
    webview1->Source = uri;
    webview1->Width = width;
    webview1->Height = height;
    m_bFrameReceived = true;
    // need to add code to scale webview zoom to display entire width of webpage inside of webview
}

void MainPage::OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
{
    StartTimer();
}

void MainPage::StartTimer()
{
    m_bFrameReceived = true;
    m_dispatcherTimer->Start();
}

void MainPage::StopTimer()
{
    m_dispatcherTimer->Stop();
}

void MainPage::TimerTick(Platform::Object^ sender, Platform::Object^ e)
{
    m_timer.Tick([&]()
    {
        if (m_bFrameReceived)
        {
            m_bFrameReceived = false;
            //UpdateWebViewBitmap((unsigned int)webview1->ActualWidth, (unsigned int)webview1->ActualHeight);
            UpdateWebViewBitmap(400, 400);
        }
    });
}


task<void> MainPage::UpdateWebViewBitmap(unsigned int width, unsigned int height)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

    if (s_bitmap1.size() == 0 || s_bitmap2.size() == 0)
    {

        double webViewControlWidth = webview1->ActualWidth;
        double webViewControlHeight = webview1->ActualHeight;

 
    }

    // capture the WebView
    return create_task(webview1->CapturePreviewToStreamAsync(stream))
        .then([width, height, stream]()
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
            std::unique_lock<std::mutex> lock(s_mutex);
            Platform::Array<byte>^ pixelData = pixelDataProvider->DetachPixelData();

            if (s_bitmap1.size() != pixelData->Length)
            {
               s_bitmap1.resize(pixelData->Length);
            }

            if (s_bitmap2.size() != pixelData->Length)
            {
                s_bitmap2.resize(pixelData->Length);
            }
            
            memcpy(s_bitmap2.data(), pixelData->Data, pixelData->Length);

        });
    }).then([this]()
    {
        m_bFrameReceived = true;
        std::wstringstream w;
        w << L" FPS:" << m_timer.GetFramesPerSecond() << std::endl;
        statusText->Text = ref new Platform::String(w.str().c_str());
    });
}

const std::vector<byte>& MainPage::GetBitmap()
{
    std::unique_lock<std::mutex> lock(s_mutex);
    //std::swap(s_bitmap1, s_bitmap2);
    return s_bitmap2;
};


