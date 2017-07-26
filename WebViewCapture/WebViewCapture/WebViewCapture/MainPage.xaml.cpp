//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <algorithm>
#include <robuffer.h> // IBufferByteAccess
#include <string> 

using namespace WebViewCapture;
using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
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

MainPage::MainPage()
{
	InitializeComponent();
    TimeSpan span;
    span.Duration = 10000000L / 60L;
    m_dispatcherTimer = ref new DispatcherTimer();
    m_dispatcherTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &MainPage::TimerTick);
    m_dispatcherTimer->Interval = span;
    m_dispatcherTimer->Start();
}

void MainPage::TimerTick(Platform::Object^ sender, Platform::Object^ e)
{
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

    DisplayScaledBitmap(width, height);
}

task<void> MainPage::DisplayScaledBitmap(unsigned int width, unsigned int height)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

    return create_task(webview1->CapturePreviewToStreamAsync(stream))
        .then([width, height, stream]()
    {
        return create_task(BitmapDecoder::CreateAsync(stream));
    }).then([width, height](BitmapDecoder^ decoder)
    {
        BitmapTransform^ transform = ref new BitmapTransform();
        transform->ScaledHeight = height;
        transform->ScaledWidth = width;
        return create_task(decoder->GetPixelDataAsync(
            BitmapPixelFormat::Bgra8,
            BitmapAlphaMode::Straight,
            transform,
            ExifOrientationMode::RespectExifOrientation,
            ColorManagementMode::DoNotColorManage))
            .then([width, height](PixelDataProvider^ pixelDataProvider)
        {
            WriteableBitmap^ bitmap = ref new WriteableBitmap(width, height);
            Platform::Array<byte>^ pixelData = pixelDataProvider->DetachPixelData();
            ComPtr<IInspectable> bufferAsInspectable(reinterpret_cast<IInspectable*>(bitmap->PixelBuffer));
            ComPtr<IBufferByteAccess> bufferAsByteAccess;
            bufferAsInspectable.As(&bufferAsByteAccess);
            byte* pixels;
            bufferAsByteAccess->Buffer(&pixels);
            memcpy(pixels, pixelData->Data, pixelData->Length);
            bitmap->Invalidate();
            return bitmap;
        });
    }).then([this](WriteableBitmap^ bitmap)
    {
        image1->Source = bitmap;
        std::wstring w = std::to_wstring(m_timer.GetFramesPerSecond());
        frameCount->Text = ref new Platform::String(w.c_str());
    });
}





