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

* Create a new Visual C++ | Windows Desktop | Windows Desktop Application. 

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



### Adding UWP Support to the Desktop Extension

We need to add a few settings so our Desktop Extension can call UWP methods. Please do the following:

* Right-click on the SystemControlDesktopExtension project and select Properties. Make sure All Configurations and All Platforms are selected.

* Select C/C++ | General and add the following settings

    * Set Additional #using Directories to $(VC_LibraryPath_VC_x86_Store)\references\;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\$(TargetPlatformVersion);C:\Program Files (x86)\Windows Kits\10\UnionMetadata;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.UniversalApiContract\3.0.0.0;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.FoundationContract\3.0.0.0;%(AdditionalUsingDirectories)
    
    * Set Consume Windows Runtime Extension to Yes/(ZW)
    
    * Add /Zc:twoPhase-  to the C/C++ | Commannd line
    
    * Set C/C++ Precompiled Headers to Not Using Precompiled Headers
    
    * Set Configuration Properties | General | Output Directory to $(SolutionDir)$(PlatformTarget)\$(Configuration)\

* Try to build the solution. All projects should build without an error.

### Adding the App Service to the UWP project


### Add a Packaging Project to Package the Win32 .exe with the UWP App

We will use a Packaging Project to Package the Win32 .exe with the UWP App. The Packaging Project will add both the Win32 .exe and the UWP App to the AppX package.
Add a Packaging Project to your solution.

* Right-click on the Solution and select Add | New Project...

* Select Visual C# | Windows Universal  | Windows Application Packaging Project

* Name the project PackageProject and click OK

* Set PackageProject as the Startup Project

* In the PackageProject right click on Applications and select Add Reference

* Select both the UWP Project and the Win32 project 

* Expand the Applications folder and right-click on the UWP project. Select Set as EntryPoint.

* Right-click on the Package.appxmanifest file in the PackageProject. Select View Code.

* Replace all instances of PackageProject with the name of your App. (It this example it is SystemControl)

* Modify the Package section to add the Desktop xmlns attribute

```xml
<Package
  xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
  xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
  xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
  xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"
  xmlns:desktop="http://schemas.microsoft.com/appx/manifest/desktop/windows10" 
  IgnorableNamespaces="uap mp rescap desktop">
 ```

* Add the Desktop Extension to the Application Section

```xml
  <Applications>
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="$targetentrypoint$">
      <uap:VisualElements
        DisplayName="SystemControl"
        Description="SystemControl"
        BackgroundColor="transparent"
        Square150x150Logo="Images\Square150x150Logo.png"
        Square44x44Logo="Images\Square44x44Logo.png">
        <uap:DefaultTile
          Wide310x150Logo="Images\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
        <desktop:Extension Category="windows.fullTrustProcess" Executable="SystemControlDesktopExtension/SystemControlDesktopExtension.exe" />
      </Extensions>
    </Application>
  </Applications>
 ```
 
* Add the AppService to the Application section

```xml
  <Extensions>
    <uap:Extension Category="windows.appService">
      <uap:AppService Name="com.stammen.systemcontrol.appservice" />
    </uap:Extension>
  </Extensions>
```

* Your Application section should now look something like this:

```xml
  <Applications>
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="$targetentrypoint$">
      <uap:VisualElements
        DisplayName="SystemControl"
        Description="SystemControl"
        BackgroundColor="transparent"
        Square150x150Logo="Images\Square150x150Logo.png"
        Square44x44Logo="Images\Square44x44Logo.png">
        <uap:DefaultTile
          Wide310x150Logo="Images\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
          <uap:Extension Category="windows.appService">
            <uap:AppService Name="com.stammen.systemcontrol.appservice" />
          </uap:Extension>
        <desktop:Extension Category="windows.fullTrustProcess" Executable="SystemControlDesktopExtension/SystemControlDesktopExtension.exe" />
      </Extensions>
    </Application>
  </Applications>
```

Note: change com.stammen.systemcontrol.appservice to a name that makes sense for your app!

The Packaging Project is now configured to correctly build your application.


### Modify the UWP app to launch the Desktop Extension

In order to use FullTrustProcessLauncher we need to add a reference to the Windows Desktop Extension for the UWP

We will use the UWP Method FullTrustProcessLauncher to launch the Desktop Extension app from the UWP app. In order to use FullTrustProcessLauncher we need to do the following:

* In the UWP project, right-click on References and select Add Reference. 

* Click on the Universal Windows tab. Select Extensions

* Carefully select the Windows Desktop Extension for the UWP (version 10.0.15063). Click on OK.

* Add the following methods to MainPage.xaml.cs

```csharp
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation.Metadata;

protected override async void OnNavigatedTo(NavigationEventArgs e)
{
    await LaunchDesktopExtension();
}

private async Task LaunchDesktopExtension()
{
    if (ApiInformation.IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0))
    {
        await FullTrustProcessLauncher.LaunchFullTrustProcessForCurrentAppAsync();
    }
}
```

In order to debug the Desktop Extension, edit the Debugging Properties of the UWP Project.

* Right-click on the UWP project and select Properties

* In the Debugging tab, set Background task process to Native Only

* Set a breakpoint in SystemControlDesktopExtension.cpp

* Build and run the app. The app should stop at the breakpoint.

### Send a Message from the UWP App to the Desktop Extension

We will now send a message to the Desktop Extension. Add a button to MainPage.xaml and create a Button click handler

* In MainPage.xaml add

```xml
<Grid>
	<StackPanel Orientation="Vertical" Margin="10">
		<Button Click="Button_Click" Content="Send Message" />
		<TextBlock x:Name="resultText" Margin="10, 10"/>
	</StackPanel>
</Grid>
```

 * In MainPage.xaml.cs add the following method:
 
 ```csharp
private async void Button_Click(object sender, RoutedEventArgs e)
{
    ValueSet message = new ValueSet();
    message.Add("Message", "Hello");
    message.Add("Name", "Jim");
    var app = App.Current as App;
    var result = await app.SendMessage(message);
    if(result.ContainsKey("Status"))
    {
        string status = result["Status"] as string;
        if(status == "OK")
        {
            resultText.Text = result["Message"] as string;
        }
    }
}
```

* Build and run the app. You should be able to send to and receive a message from the Desktop Extension via the AppService.

Note: The Title Bar of your App may be Black. We will update these instructions with a fix for this.


### Adding Win32 API Functions to the Desktop Extension

Take a look at SystemControl.cpp, SystemVolume.cpp, Brightness.cpp, Applications.cpp in this repo for how you can add Win32 API function via the AppService and Desktop Extension
 