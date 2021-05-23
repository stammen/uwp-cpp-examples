#pragma once

#include "Common\DeviceResources.h"
#include "MRWin32Main.h"
#include <thread>
#include <memory>

class AppMain sealed
{
public:
    AppMain();

    void Initialize();
    void Activate(HWND window);
    void Close();

private:

    void HolographicThread();

    bool m_activated;
    bool m_close;
    HWND m_hwnd;
    std::thread m_thread;
    Windows::Graphics::Holographic::HolographicSpace^ m_holographicSpace;
    Windows::UI::Input::Spatial::SpatialInteractionManager^ m_spatialInteractionManager;
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::unique_ptr<MRWin32::MRWin32Main> m_main;
};