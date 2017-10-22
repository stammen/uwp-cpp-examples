// ScreenCaptureApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "..\MRAppService\MRAppServiceListener.h"

#include <iostream>
#include <string>
#include <collection.h>  
#include <thread>        
#include <ppltasks.h>

using namespace concurrency;
using namespace MRAppService;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

class SendInputUWP : public MRAppService::IMRAppServiceListenerDelegate
{
public:
    SendInputUWP()
        : m_appServiceListener(nullptr)
    {

    }

    ~SendInputUWP()
    {
        m_quitting = true;
    }

    int Run()
    {
        std::thread t(&SendInputUWP::AppThread, this);
        t.join();
        return 0;
    }

private:
    Concurrency::task<void> ConnectToAppService()
    {
        m_appServiceListener = ref new MRAppServiceListener(L"Win32-App");
        auto connectTask = m_appServiceListener->ConnectToAppService(MRAPPSERVICE_ID, MRAppService::MRAppServiceListener::GetPackageFamilyName());
        return connectTask.then([this](AppServiceConnectionStatus status)
        {
            if (status == AppServiceConnectionStatus::Success)
            {
                m_appServiceListener->RegisterListener(this).then([this](AppServiceResponse^ response)
                {
                    if (response->Status == AppServiceResponseStatus::Success)
                    {


                    }
                });
            }
        });
    }

    void Ping()
    {
        ValueSet^ message = ref new ValueSet();

        m_appServiceListener->SendPing(L"MR-App").then([this](AppServiceResponse^ response)
        {
            if (response->Status != AppServiceResponseStatus::Success)
            {
                m_quitting = true;
            }
            else
            {
                auto responseMessage = response->Message;

                // The response from the MR-App contains the info we need to open the shared texture
                if (responseMessage->HasKey(L"Status"))
                {
                    auto status = dynamic_cast<Platform::String^>(responseMessage->Lookup(L"Status"));
                    if (status != L"OK")
                    {
                        m_quitting = true;
                    }
                }
            }
        });
    }

    void AppThread()
    {
        m_quitting = false;
        unsigned int frameCount = 0;

        auto task = ConnectToAppService();
        try
        {
            task.get(); // blocks until Connection task completes

            if (m_appServiceListener != nullptr && m_appServiceListener->IsConnected())
            {
                while (!m_quitting)
                {
                    // ping the UWP app every second
                    Sleep(1000);
                    Ping();
                }
            }

            if (m_appServiceListener)
            {
                m_appServiceListener->UnregisterListener();
            }
        }

        catch (Platform::Exception^ ex)
        {
            OutputDebugString(ex->Message->Data());
        }
    }

    virtual ValueSet^ OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
    {
        ValueSet^ response = ref new ValueSet;
        ValueSet^ request = args->Request->Message;

        MRAppServiceMessage message = (MRAppServiceMessage)(static_cast<int>(request->Lookup(L"Message")));
        Platform::String^ id = dynamic_cast<Platform::String^>(request->Lookup(L"SenderId"));

        switch (message)
        {
        case MRAppServiceMessage::App_Connected:
            response->Insert(L"Status", "OK");
            break;

        case MRAppServiceMessage::App_Disconnected:
            response->Insert(L"Status", "OK");
            break;

        case MRAppServiceMessage::App_Message:
            response = HandleMessage(request);
            break;

        case MRAppServiceMessage::App_Ping:
            response->Insert(L"Status", "OK");
            break;

        default:
            response->Insert(L"Status", "Error");
            response->Insert(L"ErrorMessage", "Received unknown MRAppServiceMessage");
            break;
        }

        return response;
    }
    
    virtual void OnServiceClosed(AppServiceConnection^ sender, AppServiceClosedEventArgs^ args)
    {
        m_quitting = true;
    }
        
    ValueSet^ HandleMessage(ValueSet^ message)
    {
        auto data = dynamic_cast<ValueSet^>(message->Lookup("Data"));
        ValueSet^ response = ref new ValueSet;

        if (data->HasKey(L"MOUSEINPUT"))
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dx = static_cast<int>(data->Lookup(L"dx"));
            input.mi.dy = static_cast<int>(data->Lookup(L"dy"));
            input.mi.mouseData = static_cast<unsigned int>(data->Lookup(L"mouseData"));
            input.mi.dwFlags = static_cast<unsigned int>(data->Lookup(L"dwFlags"));
            input.mi.time = static_cast<unsigned int>(data->Lookup(L"time"));

            if (SendInput(1, &input, sizeof(input)) == 1)
            {
                response->Insert(L"Status", "OK");
            }
            else
            {
                response->Insert(L"Status", "Error");
                response->Insert(L"StatusMessage", "Error sending MOUSEINPUT with SendInput()");
            }
        }
        else if (data->HasKey(L"GetWindowSize"))
        {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = static_cast<int>(data->Lookup(L"wVk"));
            input.ki.wScan = static_cast<int>(data->Lookup(L"wScan"));
            input.ki.dwFlags = static_cast<unsigned int>(data->Lookup(L"dwFlags"));
            input.mi.time = static_cast<unsigned int>(data->Lookup(L"time"));

            if (SendInput(1, &input, sizeof(input)) == 1)
            {
                response->Insert(L"Status", "OK");
            }
            else
            {
                response->Insert(L"Status", "Error");
                response->Insert(L"StatusMessage", "Error sending MOUSEINPUT with SendInput()");
            }
        }
        else
        {
            response->Insert(L"Status", "Error");
            response->Insert(L"StatusMessage", "Received unknown message");
        }

        return response;
    }



    MRAppService::MRAppServiceListener^ m_appServiceListener;
    bool m_quitting;
};

[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    SendInputUWP app;
    int result = app.Run();
    return result;
}

