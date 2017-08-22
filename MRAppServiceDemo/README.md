# Mixed Reality AppService Demo
This sample demonstrates how to create a DeskTop Bridge version of a UWP Mixed Reality app that can launch a Win32 exe and communicate
between the UWP and Win32 apps via an AppSerice. The UWP Mixed Reality app will continue to run in the HMD while the Win32 exe will run on the
user's desktop. Both apps connect to an AppService so data may be exchanged between the 2 processes.

Note: This example will not work for the Hololens as it is not able to launch a Win32 application.

## Requirements

[Install the Windows Mixed Reality Development Tools](https://developer.microsoft.com/en-us/windows/mixed-reality/install_the_tools)

## Running the Sample

* Open MRAppServiceDemo.sln with Visual Studio 2017

* Launch the Windows Mixed Reality Portal and setup your HMD device or Mixed Reality simulator)

* Select the Debug/x86 or Debug/x64 configuration. 

* Select Local Machine as the target device.

* Set the MRAppServiceDemo project as the StartUp project

* Press F5 to build and run the solution. The MRAppServiceDemo app should run in the Mixed Reality Portal.

![App running in Mixed Reality Portal](images/mrportal.png)

* Tap (or right click if using the Simulator) anywhere in the app's window. The Win32 console app will be launched on the user's desktop.

![Win32 App](images/win32.png)

* In the Win32 console app press the c key on the keyboard to connect the app to the App Service.

![Win32 App](images/connected.png)

* In the Win32 console app press the s key to send data to the UWP Mixed Reality app via the App Service.

![Win32 App](images/send.png)

* The UWP app will display the received data in the Debug output window of VS2017.

![Win32 App](images/data.png)


