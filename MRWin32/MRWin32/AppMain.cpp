#include "pch.h"
#include "AppMain.h"

#include <..\winrt\WinRTBase.h>
#include <windows.graphics.holographic.h>
#include <windows.ui.input.spatial.h>
#include <..\um\HolographicSpaceInterop.h>
#include <..\um\SpatialInteractionManagerInterop.h>

#include <wrl.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics::Holographic;
using namespace ABI::Windows::UI::Input::Spatial;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#define WM_TIE_FOREGROUNDS WM_APP+0
#define WM_UNTIE_FOREGROUNDS WM_APP+1
#define WM_FOREGROUND_WINDOW_CHANGED WM_APP+2
#define WM_GIFTFOCUS WM_APP+3

AppMain::AppMain() :
    m_activated(false),
    m_close(false),
    m_holographicSpace(nullptr),
    m_spatialInteractionManager(nullptr)
{

}

void AppMain::Initialize()
{
    // At this point we have access to the device and we can create device-dependent
    // resources.
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_main = std::make_unique<MRWin32::MRWin32Main>(m_deviceResources);
}


void AppMain::Activate(HWND hWnd)
{
    if (m_activated)
    {
        return;
    }

    m_hwnd = hWnd;
    m_activated = true;

    ComPtr<IHolographicSpaceStatics> spHolographicSpaceFactory;
    HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Holographic_HolographicSpace).Get(), &spHolographicSpaceFactory);

    ComPtr<IHolographicSpaceInterop> spHolographicSpaceInterop;

    if (SUCCEEDED(hr))
    {
        hr = spHolographicSpaceFactory.As(&spHolographicSpaceInterop);
    }

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
    if (SUCCEEDED(hr))
    {
        hr = spHolographicSpaceInterop->CreateForWindow(hWnd, IID_PPV_ARGS(&spHolographicSpace));
        if (SUCCEEDED(hr))
        {
            m_holographicSpace = reinterpret_cast<Windows::Graphics::Holographic::HolographicSpace^>(spHolographicSpace.Get());
        }
    }

    ComPtr<ISpatialInteractionManagerStatics> spSpatialInteractionFactory;
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Input_Spatial_SpatialInteractionManager).Get(), &spSpatialInteractionFactory);

    ComPtr<ISpatialInteractionManagerInterop> spSpatialInterop;
    if (SUCCEEDED(hr))
    {
        hr = spSpatialInteractionFactory.As(&spSpatialInterop);
    }

    ComPtr<ISpatialInteractionManager> spSpatialInteractionManager;
    if (SUCCEEDED(hr))
    {
        hr = spSpatialInterop->GetForWindow(hWnd, IID_PPV_ARGS(&spSpatialInteractionManager));
        m_spatialInteractionManager = reinterpret_cast<Windows::UI::Input::Spatial::SpatialInteractionManager^>(spSpatialInteractionManager.Get());
    }

    CreateWindowForInteropAsync();
}

void AppMain::Close()
{
    m_close = true;
}

DWORD WINAPI AppMain::WindowInteropThreadProcStatic(LPVOID renderer)
{
    return static_cast<AppMain*>(renderer)->WindowInteropThreadProc();
}

LRESULT CALLBACK AppMain::WindowProcStatic(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (uMsg == WM_ACTIVATE && wParam != WA_INACTIVE)
    {
        // Post a message for the window message loop to pick up so it
        // can give focus to the last active hwnd.
        PostMessage(hwnd, WM_GIFTFOCUS, 0, 0);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void AppMain::CreateWindowForInteropAsync()
{
    HANDLE threadHandle = CreateThread(
        nullptr /* default security attributes */,
        0 /* default stack size*/,
        &WindowInteropThreadProcStatic,
        reinterpret_cast<LPVOID>(this),
        0, /* default flags */
        nullptr /* Don't care about our thread ID */
    );

    m_hwndThread = threadHandle;
}

DWORD AppMain::WindowInteropThreadProc()
{
    m_condition.notify_one();
    m_deviceResources->SetHolographicSpace(m_holographicSpace);

    // The main class uses the holographic space for updates and rendering.
    m_main->SetHolographicSpace(m_holographicSpace, m_spatialInteractionManager);

    // The last OpenVR client hwnd that got focus
    HWND lastActiveClientHwnd = 0;

    while (!m_close)
    {
        auto holographicFrame = m_main->Update();

        if (m_main->Render(holographicFrame))
        {
            // The holographic frame has an API that presents the swap chain for each
            // holographic camera.
            m_deviceResources->Present(holographicFrame);
        }
    }

    CloseHandle(m_hwndThread);
    m_hwnd = nullptr;
    
    return EXIT_SUCCESS;
}