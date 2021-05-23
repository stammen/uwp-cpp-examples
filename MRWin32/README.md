# Win32 Windows Mixed Reality App

Starting with RS4, it is now possible to create a Win32 Windows Mixed Reality app using an HWND to create the HolographicSpace. In previous versions of Windows 10,
a CoreWindow was required to create the HolographicSpace.

This repo uses info from [Creating a holographic DirectX project](https://docs.microsoft.com/en-us/windows/mixed-reality/creating-a-holographic-directx-project#creating-a-win32-project) and 
[Getting a HolographicSpace](https://docs.microsoft.com/en-us/windows/mixed-reality/getting-a-holographicspace)

This sample combines a Win32 App with the Holographic portion of the Univeral Windows Holographic template to present a rotating cube in a HolographicSpace. A UWP launcher
app is packaged with the Win32 App to allow the App to appear in the Windows Mixed Reality Portal list of installed applications.

## Requirements

* Visual Studio 2017 with Windows Universal App Development package installed
* Windows SDK version 17134 (installed with Visual Studio 2017) or minimum SDK version 15063
* Windows Mixed Reality installed on your computer

## Running the Sample

* Open MRWin32.sln with Visual Studio 2017

* Select the Debug/x86 or Debug/x64 configuration. (Release/x86 and Release x/64 also work)

* Set the PackageProject project as the StartUp project

* Press F5 to build and run the solution. 

* The Win32 app should be launched and appear in the Windows Mixed Reality Portal


##  Setup Instructions

In order to replicate this scenario you will need to do the following:

* Create a new Visual C++ | Windows Desktop | Windows Desktop Application. Name the project MRWin32. 

* Right click on the solution and select Add | New Project...

* Select Visual C# | Windows Universal | Blank App (Univeral Windows). Name the project MRWin32-UWP. 


### Adding UWP Support to the Win32 Application

We need to add a few settings so our Desktop Extension can call UWP methods. Please do the following:

* Right-click on the MRWin32 project and select Properties. Make sure All Configurations and All Platforms are selected.

* Select C/C++ | General and add the following settings

* Set Additional #using Directories to $(VC_LibraryPath_VC_x86_Store)\references\;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\$(TargetPlatformVersion);C:\Program Files (x86)\Windows Kits\10\UnionMetadata;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.UniversalApiContract\3.0.0.0;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.FoundationContract\3.0.0.0;%(AdditionalUsingDirectories)

* Set Consume Windows Runtime Extension to Yes/(ZW)

* Add /Zc:twoPhase-  to the C/C++ | Commannd line

* Set C/C++ Precompiled Headers to Not Using Precompiled Headers

* Set Configuration Properties | General | Output Directory to $(SolutionDir)$(PlatformTarget)\$(Configuration)\

*In the MRWin32.cpp file add the following before the wWinMain function

```c++
[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
					 
					 
```

Try to build the solution. All projects should build without an error.

### Adding the Holographic Template code to the Win32 Project

An easy way to get started is to create a separate Windows Universal Holographic project and copy the need files over to the Win32 project.

* In Visual Studio create a new project

* Select a new Visual C++ | Windows Universal | Holographic | Holographic DirectX 11 App. 

* Name the project MRWin32 and create the project. You may need to save the solution in a different location to prevent overwriting the above MRWin32 project.
We nameed the project MRWin32 so the correct project namespaces will be generated.

After the solution is created close the project.

Copy the following folders from the Holographic project to your MRWin32 project folder:

* Common
* Content

Copy the following files to your MRWin32 project folder:

* MRWin32Main.cpp, 

* pch.h

Add the above files and folders to your MRWin32 project. 

You will most likely need to configure the properties for the shaders. 

* For each shader .hlsl file right-click on the file and select Properties

* Click on HLSL Compile | General 

* Set the Shader Type to the correct type (Pixel, Vertex or Geometry)

* Set the Shader Model to 5.0
 
Right click on the MRWin32 project and select Properties

* Add $(ProjectDir) to the C++ | General | Additional Includes Directories

* Add d2d1.lib;d3d11.lib;dxgi.lib;dwrite.lib;windowscodecs.lib;%(AdditionalDependencies) to Linker | Input | Additional Dependencies

Try to build the solution. All projects should build without an error.

### Connect the Holographic Code to the Win32 App

We now need to write some code to connect the the Holographic code to the Win32 app.

* Add a new class to your MRWin32 project. Call it AppMain.

* Replace the contents of AppMain.h with

```cpp
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
```

* Replace the contents of AppMain.cpp with

```c++
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

    // Set up the holographic space
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
        if (SUCCEEDED(hr))
        {
            m_spatialInteractionManager = reinterpret_cast<Windows::UI::Input::Spatial::SpatialInteractionManager^>(spSpatialInteractionManager.Get());
        }
    }

    // start the holographic presentation thread
    m_thread = std::thread(&AppMain::HolographicThread, this);
}

void AppMain::Close()
{
    m_close = true;

    // wait for the holographic presentation thread to exit
    m_thread.join();
}


void AppMain::HolographicThread()
{
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

    m_hwnd = nullptr;
}
```

* Take a look at AppMain::Activate(HWND hWnd). In this function, we obtain instances of the HolographicSpace and the SpatialInteractionManager and hand them off to the Hologrpahic code.

### Modifying the Holographic Code to support Win32

We need to make a few changes to the holographic code to support running in a Win32 app.

* Build the solution, you will get the error 'MRWin32::MRWin32Main::SetHolographicSpace': function does not take 2 arguments.

* Modify MRWin32Main.h to add the missing SpatialInteractionManager^ parameter to the SetHolographicSpace method

```cpp
public:
    void SetHolographicSpace(Windows::Graphics::Holographic::HolographicSpace^ holographicSpace, Windows::UI::Input::Spatial::SpatialInteractionManager^ spatialInteractionManager);

private:
    Windows::UI::Input::Spatial::SpatialInteractionManager^ m_spatialInteractionManager;
```

* Modify MRWin32Main.cpp to add the missing SpatialInteractionManager^ parameter to the SetHolographicSpace method

```cpp
void MRWin32Main::SetHolographicSpace(HolographicSpace^ holographicSpace, SpatialInteractionManager^ spatialInteractionManager)
{
    UnregisterHolographicEventHandlers();

    m_holographicSpace = holographicSpace;
    m_spatialInteractionManager = spatialInteractionManager;
```

* Modify Content\SpatialInputHandler.h to add the missing SpatialInteractionManager^ parameter to the Constructor

```cpp
    public:
        SpatialInputHandler(Windows::UI::Input::Spatial::SpatialInteractionManager^ manager);
```

* Modify Content\SpatialInputHandler.cpp to add the missing SpatialInteractionManager^ parameter to the Constructor

```cpp
SpatialInputHandler::SpatialInputHandler(Windows::UI::Input::Spatial::SpatialInteractionManager^ manager)
{
    // The interaction manager provides an event that informs the app when
    // spatial interactions are detected.
    m_interactionManager = manager;

    // Bind a handler to the SourcePressed event.
    m_sourcePressedEventToken =
        m_interactionManager->SourcePressed +=
            ref new TypedEventHandler<SpatialInteractionManager^, SpatialInteractionSourceEventArgs^>(
                bind(&SpatialInputHandler::OnSourcePressed, this, _1, _2)
                );

    //
    // TODO: Expand this class to use other gesture-based input events as applicable to
    //       your app.
    //
}
```

* Modify the CreateDeviceDependentResources method in Content\SpinningCubeRenderer.cpp to load the shaders from the Win32 .exe folder

```cpp
void SpinningCubeRenderer::CreateDeviceDependentResources()
{
    WCHAR full_path[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(nullptr, full_path, MAX_PATH + 1);

    std::wstring path = full_path;
    path = path.substr(0, path.rfind(L"\\") + 1);


    m_usingVprtShaders = m_deviceResources->GetDeviceSupportsVprt();

    // On devices that do support the D3D11_FEATURE_D3D11_OPTIONS3::
    // VPAndRTArrayIndexFromAnyShaderFeedingRasterizer optional feature
    // we can avoid using a pass-through geometry shader to set the render
    // target array index, thus avoiding any overhead that would be 
    // incurred by setting the geometry shader stage.
    std::wstring vertexShaderFileName = m_usingVprtShaders ? L"VprtVertexShader.cso" : L"VertexShader.cso";

    // Load shaders asynchronously.
    task<std::vector<byte>> loadVSTask = DX::ReadDataAsync(path + vertexShaderFileName);
    task<std::vector<byte>> loadPSTask = DX::ReadDataAsync(path + L"PixelShader.cso");

    task<std::vector<byte>> loadGSTask;
    if (!m_usingVprtShaders)
    {
        // Load the pass-through geometry shader.
        loadGSTask = DX::ReadDataAsync(path + L"GeometryShader.cso");
    }
```

* Modify MRWin32.cpp to add the methods to create the HolographicSpace.

```cpp
#include "AppMain.h"
#include <memory>

std::unique_ptr<AppMain> appMain;

[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
    
    ...
    MyRegisterClass(hInstance);

    // Initialize app object
    appMain = std::make_unique<AppMain>();
    appMain->Initialize();                
                     
                     
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    ...
    if (!hWnd)
    {
        return FALSE;
    }

   appMain->Activate(hWnd);
   ShowWindow(hWnd, nCmdShow);
   ...

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ...
    case WM_DESTROY:
        appMain->Close();
        PostQuitMessage(0);
        break;
    ...
```


* Add pch.h to stdafx.h

* Build and run the project. A spinning cube should appear in the display.

### Add a Packaging Project to add a UWP Launcher App

When you open the Start Menu in the Windows Mixed Reality Portal by pressing the Windows Key, you will notice that the MRWin32 app does not appear in the list of apps available
in the holographic space. This is because MRWin32 is a Win32 app and the Start Menu in the holographic space only lists UWP apps. We will now create a UWP Launcher App that will 
launch the Win32 MRWin32 app. We will package these apps together with a Packaging Project.

* Right-click on the solution and select Add | New Project...

* Select the Visual C# | Windows Universal | Windows Application Packaging Project

* Name the project PackageProject and click OK.

* In the PackageProject, right-click on the Applications item and select Add References\Windows

* Add a reference to the MRWin32 and MRWin32-UWP projects anc click OK

* Expand the Applications item and right-click on MRWin32-UWP. Select Set as Entry Point

* Right click on the PackageProject project and select Set as StartUp Project

* Build and run the solution. An empty UWP app should appear. 

Note: The title bar  of the UWP app may be black. This is a known issue that will be fixed in an update to the project template.

We will now modify the UWP app to launch the Win32 MRWin32 app

* Right click on the package.appxmanifest file in the PackageProject project and select View Code

* Add an Extensions section to the Application section. It should look like:

```xml
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="$targetentrypoint$">
      <uap:VisualElements
        DisplayName="MRWin32"
        Description="MRWin32"
        BackgroundColor="transparent"
        Square150x150Logo="Images\Square150x150Logo.png"
        Square44x44Logo="Images\Square44x44Logo.png">
        <uap:DefaultTile
          Wide310x150Logo="Images\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
        <uap:Extension Category="windows.protocol" Executable="MRWin32\MRWin32.exe" EntryPoint="Windows.FullTrustApplication">
          <uap:Protocol Name="mrwin32" />
        </uap:Extension>
      </Extensions>
    </Application>
```
 
* Build and run your solution to make sure you did not make any mistakes.

We will now nodify the UWP app to launch the Win32 App

* Open the MainPage.xaml.cs file in the MRWin32-UWP project

* Add the following methods:

```csharp

using System.Threading.Tasks;
using Windows.System;

protected override async void OnNavigatedTo(NavigationEventArgs e)
{
    // launch the Win32 Holographic App
    await LaunchApp("mrwin32:");
    var t = Task.Run(async delegate
    {
        // exit the UWP app
        await Task.Delay(1000);
        Application.Current.Exit();
    });
}
        
private async Task LaunchApp(string protocol)
{
    // Launch the Win32 App
    var uri = new Uri(protocol); // The protocol handled by the launched app
    var options = new LauncherOptions();
    await Launcher.LaunchUriAsync(uri, options);
}
```

* Build and run the solution. The UWP app will start and launch the MRWin32 app. The UWP app will then exit after a second leaving the MRWin32 app running in the Holographic space.

* If you check the start menu in the Windows Mixed Reality the MRWin32-UWP app should appear in the menu.
