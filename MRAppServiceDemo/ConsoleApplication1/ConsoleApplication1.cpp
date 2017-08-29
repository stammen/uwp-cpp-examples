// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <ctype.h>  
#include <ppltasks.h>
#include <iostream>
#include <string>

using namespace concurrency;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

AppServiceConnection^	gAppService = nullptr;
bool gAppServiceConnected = false;

bool LaunchApp()
{
	bool done = false;
	auto uri = ref new Uri("mrappservicedemo-uwp:"); // The protocol handled by the launched app
	auto options = ref new LauncherOptions();
	concurrency::task<bool> task(Launcher::LaunchUriAsync(uri, options));
	return task.get();
}

void LaunchAppService()
{
	if (gAppService == nullptr)
	{
		gAppService = ref new AppServiceConnection();

		// Here, we use the app service name defined in the app service provider's Package.appxmanifest file in the <Extension> section.
		gAppService->AppServiceName = "com.mrappservicedemo.appservice";

		// Use Windows.ApplicationModel.Package.Current.Id.FamilyName within the app service provider to get this value.
		gAppService->PackageFamilyName = "661fcf9b-01c2-450d-be4b-a62a0fe9913c_e8xk87pxx0yyw";

		create_task(gAppService->OpenAsync()).then([](AppServiceConnectionStatus status)
		{
			if (status != AppServiceConnectionStatus::Success)
			{
				gAppService = nullptr;
				std::cout << "LaunchAppService Error:" << (int)status <<  " Unable to connect to AppService" << std::endl;
			}
			else
			{
				std::cout << "Connected to AppService." << std::endl;
				gAppServiceConnected = true;
			}
		});
	}
}

void SendToAppService(float distance)
{
	if (!gAppServiceConnected)
	{
		std::cout << "Not connected to AppService." << std::endl;
		return;
	}

	auto message = ref new ValueSet();
	message->Clear(); // using empty message for now
	message->Insert(L"PostData", distance);
	create_task(gAppService->SendMessageAsync(message)).then([distance](AppServiceResponse^ response)
	{
		if (response->Status == AppServiceResponseStatus::Success)
		{
			std::cout << "Sent distance of " << distance << " meters to AppService" << std::endl;
		}
		else
		{
			std::cout << "SendToAppService Error:" << (int)response->Status << " Unable to send data to AppService" << std::endl;
		}
	});

}


int main(Platform::Array<Platform::String^>^ args)
{
	int ch;
   float distance = 2.0f;

	std::cout << "*********************************" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " l - launch UWP Holographic App" << std::endl;
    std::cout << " w - reduce viewing distance by 1 meter" << std::endl;
    std::cout << " s - increase viewing distance by 1 meter" << std::endl;
    std::cout << "*********************************" << std::endl;

    LaunchAppService();

	do
	{
		ch = _getch();
		ch = toupper(ch);

		switch (ch)
		{
			case 'L':
				LaunchApp();
				break;

            case 'W':
                distance = distance > 0.5f ? distance - 0.5f : 0.5f;
                SendToAppService(distance);
                break;

			case 'S':
                distance += 0.5f;
                SendToAppService(distance);
				break;

			default:
				break;
		}
	} while (ch != 'Q');

    return 0;
}

