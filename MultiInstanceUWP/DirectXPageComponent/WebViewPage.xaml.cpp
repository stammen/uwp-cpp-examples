//
// WebViewPage.xaml.cpp
// Implementation of the WebViewPage class
//

#include "pch.h"
#include "WebViewPage.xaml.h"
#include "common/DirectXHelper.h"
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
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_contentLoaded = false;
}

void WebViewPage::OnNavigatedStarting(WebView^ sender, WebViewNavigationStartingEventArgs^ args)
{
    m_contentLoaded = false;
}

void WebViewPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ args)
{
    // :?id=1&apptype=webview&sharedtextue=DirectXPageSharedTexture&width=512&height=512&source=https://www.microsoft.com&fps=60
    auto uri = safe_cast<Uri^>(args->Parameter);
    auto queryParsed = uri->QueryParsed;
    DirectXPageComponent::ProtocolArgs pa(queryParsed);
    auto apptype = pa.GetStringParameter(L"apptype", L"");
    m_fps = pa.GetIntParameter(L"fps", 30);
    m_width = pa.GetIntParameter(L"width", 0);
    m_height = pa.GetIntParameter(L"height", 0);
    m_sharedTextureHandleName = pa.GetStringParameter(L"sharedtexture", "");
    m_id = pa.GetStringParameter(L"id", "");
    auto source = pa.GetStringParameter(L"source", "");

    if (source->IsEmpty() || apptype != L"webview" || m_sharedTextureHandleName->IsEmpty() || m_id->IsEmpty() || m_width == 0 || m_height == 0)
    {
        throw ref new Platform::Exception(-1, L"invalid protocol query paramter");
    }

    // create the requested WebView
    m_transform = ref new BitmapTransform();
    m_webView = ref new WebView(WebViewExecutionMode::SeparateThread);
    m_webView->Source = ref new Windows::Foundation::Uri(source);
    m_webView->Width = m_width;
    m_webView->Height = m_height;
    m_webView->Visibility = Windows::UI::Xaml::Visibility::Visible;
    m_webView->NavigationStarting += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs ^>(this, &WebViewPage::OnNavigatedStarting);
    m_webView->NavigationCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::WebView ^, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^>(this, &WebViewPage::OnWebContentLoaded);
    mainGrid->Children->Append(m_webView);
    m_sleepInterval = 1000 / m_fps;

    // open connection to App Service
    if (m_appServiceListener == nullptr)
    {
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
}

void WebViewPage::OnWebContentLoaded(Windows::UI::Xaml::Controls::WebView ^ webview, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
{
    auto status = args->WebErrorStatus;
    auto success = args->IsSuccess;
    m_webView->Visibility = Windows::UI::Xaml::Visibility::Visible;

    auto width = m_webView->ActualWidth;

    auto scripts = ref new Platform::Collections::Vector<Platform::String^>();
    Platform::String^ hideScroll = L"function SetBodyOverFlowHidden(){document.body.style.overflow = 'hidden'; return 'Set Style to hidden';} SetBodyOverFlowHidden();";
    scripts->Append(hideScroll);
    m_webView->InvokeScriptAsync("eval", scripts);

    OutputDebugString(L"OnWebContentLoaded");
    CreateDirectxTextures();
    m_contentLoaded = true;
    UpdateWebView();
}

void WebViewPage::CreateWebView(ValueSet^ info)
{


}

void WebViewPage::UpdateWebView()
{
    m_timer.Tick([&]()
    {
        UpdateWebViewBitmap(m_width, m_height);
    });
}

void WebViewPage::UpdateWebViewBitmap(unsigned int width, unsigned int height)
{
    if (!m_contentLoaded)
    {
        return;
    }

    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();

    // capture the WebView
   auto task = create_task(m_webView->CapturePreviewToStreamAsync(stream))
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
            auto pixelData = pixelDataProvider->DetachPixelData();
            UpdateDirectxTextures(pixelData->Data, width, height);
        });
    }).then([this]()
    {
        std::wstringstream w;
        auto fps = m_timer.GetFramesPerSecond();
        if ((m_timer.GetFrameCount() % m_fps) == 0)
        {
            if (fps < m_fps)
            {
                if (m_sleepInterval > 2)
                {
                    m_sleepInterval--;
                }
            }
            else if (fps > m_fps)
            {
                m_sleepInterval++;
            }
        }

        w << L" FPS:" << fps << L" Sleep:" << m_sleepInterval << std::endl;
        OutputDebugString(w.str().c_str());
        Sleep(m_sleepInterval);
        if (m_contentLoaded)
        {
            UpdateWebView();
        }
    });
}

void WebViewPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
#if 1
    auto scripts = ref new Platform::Collections::Vector<Platform::String^>();
    Platform::String^ ScrollToTopString = L"window.scrollTo(0, 10000); ";
    scripts->Append(ScrollToTopString);
    m_webView->InvokeScriptAsync("eval", scripts);
#else
    if (m_appServiceListener == nullptr || !m_appServiceListener->IsConnected())
    {
        OutputDebugString(L"Not connected to AppService");
        return;
    }

    auto message = ref new ValueSet();
    message->Clear(); // using empty message for now
    message->Insert(L"Hello", L"Hello");

    m_appServiceListener->SendAppServiceMessage(L"DirectXPage", message).then([this](AppServiceResponse^ response)
    {
        auto responseMessage = response->Message;

        if (response->Status == AppServiceResponseStatus::Success)
        {
        }
        else
        {
        }
    });
#endif
}

ValueSet^ WebViewPage::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    return ref new ValueSet();
}

void WebViewPage::CreateDirectxTextures()
{
    m_stagingTexture.Reset();
    m_quadTexture.Reset();

    ID3D11Texture2D *pTexture = NULL;

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->OpenSharedResourceByName(m_sharedTextureHandleName->Data(), DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, __uuidof(ID3D11Texture2D), (LPVOID*)&pTexture)
    );

    m_quadTexture = pTexture;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pTexture = NULL;

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &pTexture)
    );

    m_stagingTexture = pTexture;
}

void WebViewPage::UpdateDirectxTextures(const void *buffer, int width, int height)
{
    if (m_quadTexture.Get() == nullptr || m_stagingTexture.Get() == nullptr)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    const auto context = m_deviceResources->GetD3DDeviceContext();

    DX::ThrowIfFailed(
        context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)
    );

    byte* data1 = (byte*)mapped.pData;
    byte* data2 = (byte*)buffer;
    unsigned int length = width * 4;
    for (int i = 0; i < height; ++i)
    {
        memcpy((void*)data1, (void*)data2, length);
        data1 += mapped.RowPitch;
        data2 += length;
    }

    context->Unmap(m_stagingTexture.Get(), 0);
    context->CopyResource(m_quadTexture.Get(), m_stagingTexture.Get());
}
