// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MRAppServiceListener.h"


#include <conio.h>
#include <ctype.h>  
#include <ppltasks.h>
#include <iostream>
#include <string>
#include <memory>

using namespace concurrency;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

class AppServiceDelegate;

std::unique_ptr<AppServiceDelegate> gAppService = nullptr;

class AppServiceDelegate : public MRAppService::IMRAppServiceListenerDelegate
{
public:
    AppServiceDelegate() {};
    ~AppServiceDelegate() 
    {
        if (m_appServiceListener != nullptr)
        {


        }
    };

    void SendDistance(float distance)
    {

        if (m_appServiceListener == nullptr || !m_appServiceListener->IsConnected())
        {
            std::cout << "Not connected to AppService." << std::endl;
            return;
        }

        auto message = ref new ValueSet();
        message->Clear(); // using empty message for now
        message->Insert(L"Distance", distance);

        m_appServiceListener->SendAppServiceMessage(L"MR-App", message).then([this, distance](AppServiceResponse^ response)
        {
            auto responseMessage = response->Message;

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

    void Connect()
    {
        m_appServiceListener = ref new MRAppService::MRAppServiceListener(L"Win32-App");
        auto connectTask = m_appServiceListener->ConnectToAppService(MRAPPSERVICE_ID, MRAPPSERVICE_FAMILY_NAME);
        connectTask.then([this](AppServiceConnectionStatus status)
        {
            if (status == AppServiceConnectionStatus::Success)
            {
                m_appServiceListener->RegisterListener(this).then([this](AppServiceResponse^ response)
                {
                    if (response->Status == AppServiceResponseStatus::Success)
                    {
                        ValueSet^ message = ref new ValueSet();

                        // Tell the MR-App we are now ready to receive messages
                        message->Insert(L"Win32-App-Connected", true);
                        m_appServiceListener->SendAppServiceMessage(L"MR-App", message).then([this](AppServiceResponse^ response)
                        {
                            auto responseMessage = response->Message;

                            // The response from the MR-App contains the info we need to open the shared texture
                            if (responseMessage->HasKey(L"Message"))
                            {
                                auto messageType = dynamic_cast<Platform::String^>(responseMessage->Lookup(L"Message"));
                            }
                        });
                    }
                });
            }
        });
    }

    virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args)
    {
        return ref new ValueSet;
    }

private:
    MRAppService::MRAppServiceListener^ m_appServiceListener;

};

bool LaunchApp()
{
	bool done = false;
	auto uri = ref new Uri("mrappservicedemo-uwp:"); // The protocol handled by the launched app
	auto options = ref new LauncherOptions();
	concurrency::task<bool> task(Launcher::LaunchUriAsync(uri, options));
	return task.get();
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

    gAppService = std::make_unique<AppServiceDelegate>();
    gAppService->Connect();

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
                gAppService->SendDistance(distance);
                break;

			case 'S':
                distance += 0.5f;
                gAppService->SendDistance(distance);
                break;

			default:
				break;
		}
	} while (ch != 'Q');

    return 0;
}

