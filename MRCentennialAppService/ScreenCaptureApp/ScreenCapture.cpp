#include "ScreenCapture.h"
#include <iostream>
#include <string>
#include <collection.h>  
#include <thread>        
#include <ppltasks.h>

using namespace concurrency;
using namespace MRAppService;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

ScreenCapture::ScreenCapture()
    : m_appServiceListener(nullptr)
    , m_quadTexture(nullptr) 
    , m_stagingTexture(nullptr)
{

}

ScreenCapture::~ScreenCapture()
{
    m_quitting = true;
    m_quadTexture.Reset();
    m_stagingTexture.Reset();
}

void ScreenCapture::ScreenCaptureThread()
{
    int width;
    int height;
    m_quitting = false;
    unsigned int frameCount = 0;

    GetScreenSize(width, height);
    m_buffer.resize(width * height * 4);

    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->SetWindow(NULL, width, height);
    m_deviceResources->CreateDeviceResources();

    auto task = ConnectToAppService();
    try
    {
        task.get(); // blocks until Connection task completes

        if (m_appServiceListener != nullptr && m_appServiceListener->IsConnected())
        {
            while (!m_quitting)
            {
                // continually capture the screen
                DoScreenCapture();
                if (frameCount++ > 10)
                {
                    frameCount = 0;
                    Ping();
                }
            }
        }
    }
    catch (Platform::Exception^ ex)
    {
        OutputDebugString(ex->Message->Data());
    }
}

int ScreenCapture::Run()
{
    // run screen capture on separate thread
    std::thread t(&ScreenCapture::ScreenCaptureThread, this);
    t.join();
    return 0;
}

void ScreenCapture::Ping()
{
    ValueSet^ message = ref new ValueSet();

    m_appServiceListener->SendPing(L"MR-App").then([this](AppServiceResponse^ response)
    {
        if (response->Status != AppServiceResponseStatus::Success)
        {
            m_quitting = true;
        }
        else
        {
            auto responseMessage = response->Message;

            // The response from the MR-App contains the info we need to open the shared texture
            if (responseMessage->HasKey(L"Status"))
            {
                auto status = dynamic_cast<Platform::String^>(responseMessage->Lookup(L"Status"));
                if (status != L"OK")
                {
                    m_quitting = true;
                }
            }
        }
    });
}

Concurrency::task<void> ScreenCapture::ConnectToAppService()
{
    m_appServiceListener = ref new MRAppServiceListener(L"Win32-App");
    auto connectTask = m_appServiceListener->ConnectToAppService(MRAPPSERVICE_ID, MRAPPSERVICE_FAMILY_NAME);
    return connectTask.then([this](AppServiceConnectionStatus status)
    {
        if (status == AppServiceConnectionStatus::Success)
        {
            m_appServiceListener->RegisterListener(this).then([this](AppServiceResponse^ response)
            {
                if (response->Status == AppServiceResponseStatus::Success)
                {

                    ValueSet^ message = ref new ValueSet();

                    // Tell the MR-App we are now ready to receive messages
                    message->Insert(L"Win32-App-Connected", true);
                    m_appServiceListener->SendAppServiceMessage(L"MR-App", message).then([this](AppServiceResponse^ response)
                    {
                        auto responseMessage = response->Message;

                        // The response from the MR-App contains the info we need to open the shared texture
                        if (responseMessage->HasKey(L"Message"))
                        {
                            auto messageType = dynamic_cast<Platform::String^>(responseMessage->Lookup(L"Message"));
                            if (messageType == L"SharedTextureInfo")
                            {
                                CreateDirectxTextures(responseMessage);
                            }
                        }
                    });
                }
            });
        }
    });
}

ValueSet^ ScreenCapture::OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args)
{
    ValueSet^ response = ref new ValueSet;
    ValueSet^ request = args->Request->Message;

    MRAppServiceMessage message = (MRAppServiceMessage)(static_cast<int>(request->Lookup(L"Message")));
    Platform::String^ id = dynamic_cast<Platform::String^>(request->Lookup(L"Id"));

    switch (message)
    {
        case MRAppServiceMessage::App_Connected:
            break;

        case MRAppServiceMessage::App_Disconnected:
            break;

        case MRAppServiceMessage::App_Message:
            response = HandleMessage(request);
            break;

        case MRAppServiceMessage::App_Ping:
            response->Insert(L"Status", "OK");
            break;
    }

    return response;
}

ValueSet^ ScreenCapture::HandleMessage(ValueSet^ message)
{
    auto data = dynamic_cast<ValueSet^>(message->Lookup("Data"));
    ValueSet^ response = ref new ValueSet;

    if (data->HasKey(L"SharedTextureInfo"))
    {
        CreateDirectxTextures(data);
        response->Insert(L"Status", "OK");
    }
    else if (data->HasKey(L"GetWindowSize"))
    {
        int width;
        int height;

        GetScreenSize(width, height);
        response->Insert(L"WindowSize", true);
        response->Insert(L"Width", width);
        response->Insert(L"Height", height);
        response->Insert(L"Status", "OK");
    }

    return response;
}

void ScreenCapture::OnServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args)
{
    m_quitting = true;
}


void ScreenCapture::GetScreenSize(int& width, int& height)
{
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
}

void ScreenCapture::DoScreenCapture()
{
    HDC hdcScreen;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    if (m_quadTexture.Get() == nullptr || m_stagingTexture.Get() == nullptr)
    {
        return;
    }

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);

    // Get the client area for size calculation
    RECT rcClient;
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = GetSystemMetrics(SM_CXSCREEN);
    rcClient.bottom = GetSystemMetrics(SM_CYSCREEN);

    hdcMemDC = CreateCompatibleDC(hdcScreen);

    //This is the best stretch mode
    SetStretchBltMode(hdcScreen, HALFTONE);

    // Create a compatible bitmap from the Screen DC
    hbmScreen = CreateCompatibleBitmap(hdcScreen, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

    if (!hbmScreen)
    {
        MessageBox(NULL, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
        0, 0,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hdcScreen,
        0, 0,
        SRCCOPY))
    {
        MessageBox(NULL, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcScreen, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        m_buffer.data(),
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    UpdateDirectxTextures(m_buffer.data(), bmpScreen.bmWidth, bmpScreen.bmHeight);

done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}

void ScreenCapture::UpdateDirectxTextures(const void* buffer, int width, int height)
{
    if (m_quadTexture.Get() == nullptr || m_stagingTexture.Get() == nullptr)
    {
        return;
    }

    if (width != m_textureWidth || height != m_textureHeight)
    {
        ResizeDirectxTextures(width, height);
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

void ScreenCapture::ResizeDirectxTextures(int width, int height)
{
    m_stagingTexture.Reset();
    m_quadTexture.Reset();

    ValueSet^ message = ref new ValueSet();

    // Tell the MR-App we are now ready to receive messages
    message->Insert(L"Win32-App-Resize", true);
    message->Insert(L"Width", width);
    message->Insert(L"Height", height);

    m_appServiceListener->SendAppServiceMessage(L"MR-App", message).then([this](AppServiceResponse^ response)
    {
        auto responseMessage = response->Message;

        // The response from the MR-App contains the info we need to open the shared texture
        if (responseMessage->HasKey(L"Message"))
        {
            auto messageType = dynamic_cast<Platform::String^>(responseMessage->Lookup(L"Message"));
            if (messageType == L"SharedTextureInfo")
            {
                CreateDirectxTextures(responseMessage);
            }
        }
    });
}

void ScreenCapture::CreateDirectxTextures(ValueSet^ info)
{
    m_stagingTexture.Reset();
    m_quadTexture.Reset();

    HANDLE sharedTextureHandle = (HANDLE)static_cast<uintptr_t>(info->Lookup(L"SharedTextureHandle"));
    m_textureWidth = (int)info->Lookup(L"Width");
    m_textureHeight = (int)info->Lookup(L"Height");

    ID3D11Texture2D *pTexture = NULL;
    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->OpenSharedResource(sharedTextureHandle, __uuidof(ID3D11Texture2D), (LPVOID*)&pTexture)
    );

    m_quadTexture = pTexture;
    
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_textureWidth;
    desc.Height = m_textureHeight;
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
