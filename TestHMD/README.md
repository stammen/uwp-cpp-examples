# Mixed Reality Win32 App to detect HMD Presence
This sample demonstrates how to create a Win32 app that can do the following:

1) Launch the MR Portal if it is not running
2) Launch a MR App using its protocol
3) Detect if an MR App is running
4_ Detect if the user has put on an HMD and launch an MR App when the HMD is detected

## Requirements

[Install the Windows Mixed Reality Development Tools](https://developer.microsoft.com/en-us/windows/mixed-reality/install_the_tools)

## Running the Sample

* Open TestHMD.sln with Visual Studio 2017

* Select the Debug/x86 or Debug/x64 configuration (or Release). 

* Select Local Machine as the target device.

* Set the TestHMD project as the StartUp project

* Select Deploy Solution from the Build menu. This will build the solution and deploy the TestHMDApp MR app so it can run in the MR Portal.

* Press F5 to start the TestHMD Win32 console app. This will do the following:

  * Start the TestHMD console app
  * Start the MR Portal if it is not running
  * After a 10 second delay, the TestHMDApp will be launched in the MR Portal. **Note: You may have to click the mouse to see the spinning cube.**


## Discussion

In order for the TestHMD to be able to launch your MR App, you will need to add a Protocol section to your package.appxmanifest file
Add the following to the Application block:

```xml
      <Extensions>
        <uap:Extension Category="windows.protocol">
          <uap:Protocol Name="testhmdapp"/>
        </uap:Extension>
      </Extensions>
```

Note: testhmdapp is used in this example. You will want to use your own unique protocol name here.

In the TestHDM.cpp, change MR_APP_PROTOCOL to the protocol you declared in the package.appxmanifest. Make sure you add a : to the end of the name. For example

```cpp
#define MR_APP_PROTOCOL L"testhmdapp:"
```

The TestHMD app detects the following situations

1. On the startup of the TestHMD app, it launches the MR Portal and after a 10 second delay, it launches your app
1. TestHMD checks if your app is running every second.
1. If the user is not present and the MR Portal has terminated your MR App, it will not relaunch the App until the user is detected.
1. If the MR Portal is asleep, it will awaken when the user puts on the HMD. The TestHMD app will detect this event and launch your MR app.

## TestHMD Configuration

The Win32 console app calls a few UWP apis to launch the MR Portal App. In order to be able to do this, the project properties must be modified to include the following for all platforms and configurations:

**C/C++ | General | Addition #using Directions** needs to have the following added:

$(VC_ReferencesPath_x86)\store\references;C:\Program Files (x86)\Windows Kits\10\UnionMetadata;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.UniversalApiContract\3.0.0.0;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.FoundationContract\3.0.0.0;%(AdditionalUsingDirectories)

**C/C++ | General | Consume Windows RunTime Support** should be **Yes(/ZW)**

**C/C++ | Code Generation | Enable Minimal Rebuild** should be **No(/Gm-)** 


