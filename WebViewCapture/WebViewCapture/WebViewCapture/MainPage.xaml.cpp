//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "SecondaryPage.xaml.h"
#include <algorithm>
#include <robuffer.h> // IBufferByteAccess
#include <string> 
#include <sstream> 

using namespace WebViewCapture;
using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
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

MainPage::MainPage()
{
	InitializeComponent();
    m_transform = ref new BitmapTransform();
    m_bitmap1 = nullptr;
    m_bitmap2 = nullptr;
    TimeSpan span;
    span.Duration = 10000000L / 60L;
    m_dispatcherTimer = ref new DispatcherTimer();
    m_dispatcherTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &MainPage::TimerTick);
    m_dispatcherTimer->Interval = span;
    m_dispatcherTimer->Start();
}

void MainPage::PointerReleased(int x, int y)
{
    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, x, y]()
    {
        auto scripts = ref new Platform::Collections::Vector<Platform::String^>();
        std::wstringstream w;
        w << L"document.elementFromPoint(" << x << "," << y << L").click()";
        scripts->Append(ref new Platform::String(w.str().c_str()));
        webview1->InvokeScriptAsync(ref new Platform::String(L"eval"), scripts);
    }));
}



void MainPage::viewButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    //auto scripts = ref new Platform::Collections::Vector<Platform::String^>();

    //scripts->Append(ref new Platform::String(L"document.elementFromPoint(105,95).click()"));

    //webview1->InvokeScriptAsync(ref new Platform::String(L"eval"), scripts);

    //return;

    if (m_secondaryView.Get() == nullptr)
    {
        m_secondaryView = CoreApplication::CreateNewView();
    }

    // We must launch the switch from the exclusive view's UI thread so we can access its view ID
    m_secondaryView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
    {
        Controls::Frame^ frame = ref new Controls::Frame();
        frame->Navigate(TypeName(SecondaryPage::typeid), this);
        Window::Current->Content = frame;

        CoreWindow::GetForCurrentThread()->Activate();
        auto viewId = ApplicationView::GetForCurrentView()->Id;

        // But the main view must do the actual switching, so we run it from it's thread
        CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([viewId]()
        {
            auto asyncAction = ApplicationViewSwitcher::SwitchAsync(viewId, ApplicationView::GetForCurrentView()->Id);
        }));
    }));
}

void MainPage::TimerTick(Platform::Object^ sender, Platform::Object^ e)
{
    //auto scripts = ref new Platform::Collections::Vector<Platform::String^>();

    //scripts->Append(ref new Platform::String(L"document.elementFromPoint(105,95).click()"));

    //webview1->InvokeScriptAsync(ref new Platform::String(L"eval"), scripts);
    
    m_timer.Tick([&]()
    {
        Update();
    });
}

void MainPage::Update()
{
    // Scale the bitmap to the space available for the thumbnail image,
    // preserving aspect ratio.
    double thumbnailWidth = image1->Width;
    double thumbnailHeight = image1->Height;
    double webViewControlWidth = webview1->ActualWidth;
    double webViewControlHeight = webview1->ActualHeight;

    if (thumbnailWidth == 0 || thumbnailHeight == 0 ||
        webViewControlWidth == 0 || webViewControlHeight == 0)
    {
        // Avoid 0x0 bitmaps, which cause all sorts of problems.
        return;
    }

    double horizontalScale = thumbnailWidth / webViewControlWidth;
    double verticalScale = thumbnailHeight / webViewControlHeight;
    double scale = std::min(horizontalScale, verticalScale);

    int width = (int)(webViewControlWidth * scale);
    int height = (int)(webViewControlHeight * scale);

    if (m_bitmap1 == nullptr)
    {
        m_bitmap1 = ref new WriteableBitmap(width, height);
    }

    if (m_bitmap2 == nullptr)
    {
        m_bitmap2 = ref new WriteableBitmap(width, height);
    }

    DisplayScaledBitmap(width, height);
}

task<void> MainPage::DisplayScaledBitmap(unsigned int width, unsigned int height)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

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
            std::unique_lock<std::mutex> lock(m_mutex);
            Platform::Array<byte>^ pixelData = pixelDataProvider->DetachPixelData();
            ComPtr<IInspectable> bufferAsInspectable(reinterpret_cast<IInspectable*>(m_bitmap2->PixelBuffer));
            ComPtr<IBufferByteAccess> bufferAsByteAccess;
            bufferAsInspectable.As(&bufferAsByteAccess);
            byte* pixels;
            bufferAsByteAccess->Buffer(&pixels);
            memcpy(pixels, pixelData->Data, pixelData->Length);
            m_bitmap2->Invalidate();
        });
    }).then([this]()
    {
        // display the bitmap
        //image1->Source = m_bitmap;
        std::wstring w = std::to_wstring(m_timer.GetFramesPerSecond());
        frameCount->Text = ref new Platform::String(w.c_str());
    });
}

WriteableBitmap^ WebViewCapture::MainPage::GetBitmap()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::swap(m_bitmap1, m_bitmap2);
    return m_bitmap1;
};



