﻿//
// BlankPage.xaml.cpp
// Implementation of the BlankPage class.
//

#include "pch.h"
#include "BlankPage.xaml.h"
#include <string>
#include <sstream> 
#include <algorithm>
#include <ppltasks.h>
#include <robuffer.h> // IBufferByteAccess

using namespace WindowsRuntimeComponent1;

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

BlankPage::BlankPage()
    : OnImage(nullptr)
{
    InitializeComponent();
    m_transform = ref new BitmapTransform();
    //webview1 = ref new WebView(WebViewExecutionMode::SeparateThread);
    webview1->NavigationCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^>(this, &WindowsRuntimeComponent1::BlankPage::OnWebContentLoaded);
}

Concurrency::task<BlankPage^> BlankPage::CreatePage()
{
    task_completion_event<BlankPage ^> tce;
    task<BlankPage ^> event_set(tce);

    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([tce]()
    {
        Controls::Frame^ rootFrame = dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content);

        // Do not repeat app initialization when the Window already has content,
        // just ensure that the window is active
        if (rootFrame == nullptr)
        {
            // Create a Frame to act as the navigation context and associate it with
            // a SuspensionManager key
            rootFrame = ref new Windows::UI::Xaml::Controls::Frame();

            if (rootFrame->Content == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame->Navigate(TypeName(BlankPage::typeid), nullptr);
            }
            // Place the frame in the current Window
            Window::Current->Content = rootFrame;
            // Ensure the current window is active
            Window::Current->Activate();
        }

        BlankPage^ page = dynamic_cast<BlankPage^>(rootFrame->Content);
        tce.set(page);
    }));

    return event_set;
}

void BlankPage::DisplayWebView(Platform::String^ url, unsigned int width, unsigned int height)
{
    CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, url, width, height]()
    {
        Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri(url);
        webview1->Visibility = Windows::UI::Xaml::Visibility::Visible;
        webview1->Source = uri;
        webview1->Width = width;
        webview1->Height = height;

        m_requestedWebViewWidth = width;
        m_requestedWebViewHeight = height;

        // need to add code to scale webview zoom to display entire width of webpage inside of webview
    }));
}

void BlankPage::OnClick(int x, int y)
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

void BlankPage::OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
{
    UpdateWebView();
}


void BlankPage::UpdateWebView()
{
    UpdateWebViewBitmap(m_requestedWebViewWidth, m_requestedWebViewHeight);
#if 0
    m_timer.Tick([&]()
    {
        UpdateWebViewBitmap(m_requestedWebViewWidth, m_requestedWebViewHeight);
    });
#endif
}

task<void> BlankPage::UpdateWebViewBitmap(unsigned int width, unsigned int height)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

    // capture the WebView
    return create_task(webview1->CapturePreviewToStreamAsync(stream))
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
//            info->framesPerSecond = m_timer.GetFramesPerSecond();
            if (OnImage != nullptr)
            {
                OnImage(this, info);
            }
        });
    }).then([this]()
    {
        UpdateWebView();
#if 0
        std::wstringstream w;
        w << L" FPS:" << m_timer.GetFramesPerSecond() << std::endl;
        OutputDebugString(w.str().c_str());
#endif
    });
}




