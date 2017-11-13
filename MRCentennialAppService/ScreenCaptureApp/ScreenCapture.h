#pragma once

#include "../MRAppService/MRAppServiceListener.h"
#include "directx/DeviceResources.h"
#include <vector>

class ScreenCapture : public MRAppService::IMRAppServiceListenerDelegate
{
public:
    ScreenCapture();
    ~ScreenCapture();
    int Run();
    void MonitorCallback(HMONITOR hMonitor);

private:
    Concurrency::task<void> ConnectToAppService();
    void Ping();

    void ScreenCaptureThread();
    void DoScreenCapture();
    void GetScreenSize(int& width, int& height);

    virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);
    virtual void OnServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args);
    Windows::Foundation::Collections::ValueSet^ ScreenCapture::HandleMessage(Windows::Foundation::Collections::ValueSet^ message);


    void UpdateDirectxTextures(const void* buffer, int width, int height);
    void CreateDirectxTextures(Windows::Foundation::Collections::ValueSet^ info);
    void ResizeDirectxTextures(int width, int height);

    MRAppService::MRAppServiceListener^ m_appServiceListener;
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;
    Microsoft::WRL::ComPtr<ID3D11Resource>  m_quadTexture;
    Microsoft::WRL::ComPtr<ID3D11Resource>  m_stagingTexture;
    int m_textureWidth;
    int m_textureHeight;
    bool m_quitting;
    std::vector<byte> m_buffer;
};
