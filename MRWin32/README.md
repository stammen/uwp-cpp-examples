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






 