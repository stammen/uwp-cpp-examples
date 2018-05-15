#pragma once

#include "Common\DeviceResources.h"
#include "MRWin32Main.h"
#include <mutex>
class AppMain sealed
{
public:
    AppMain();

    void Initialize();
    void Activate(HWND window);
    void Close();

private:
    static DWORD WINAPI WindowInteropThreadProcStatic(
        LPVOID renderer);

    static LRESULT CALLBACK WindowProcStatic(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);

    void CreateWindowForInteropAsync();
    DWORD WindowInteropThreadProc();

    bool m_activated;
    bool m_close;
    HANDLE m_hwndThread;
    HWND m_hwnd;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    Windows::Graphics::Holographic::HolographicSpace^ m_holographicSpace;
    Windows::UI::Input::Spatial::SpatialInteractionManager^ m_spatialInteractionManager;
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::unique_ptr<MRWin32::MRWin32Main> m_main;

};