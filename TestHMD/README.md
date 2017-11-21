# Mixed Reality Win32 App Detect HMD Presence Demo
This sample demonstrates how to create a Win32 app that can do the following:

1) Launch the MR Portal if it is not running
2) Launch a MR App using its protocol
3) Detect if an MR App is running
4_ Detect if the user has put on an HMD and launch an MR App when the HMD is detected

## Requirements

[Install the Windows Mixed Reality Development Tools](https://developer.microsoft.com/en-us/windows/mixed-reality/install_the_tools)

## Running the Sample

* Open TestHMD.sln with Visual Studio 2017

* Select the Debug/x86 or Debug/x64 configuration. 

* Select Local Machine as the target device.

* Set the TestHMD project as the StartUp project

* Select Deploy Solutuin from the Build menu. This will build the solution and deploy the TestHMDApp so it can run in the MR Portal.

* Press F5 to start the TestHMD Win32 console app. This will do the following:

** Start the TestHMD console app
** Start the MR Portal if it is not running
** After a 10 second delay, the TestHMDApp will be launched in the MR Portal


## Discussion

