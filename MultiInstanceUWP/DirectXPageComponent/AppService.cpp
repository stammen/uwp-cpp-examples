﻿#include "pch.h"
#include "AppService.h"
#include <ppltasks.h>    
#include <mutex>    
#include <string>
#include <sstream> 

using namespace concurrency;
using namespace DirectXPageComponent;
using namespace Platform;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

ValueSet^ AppService::s_data = nullptr;
std::mutex AppService::s_mutex;
std::map<Platform::String^, Windows::ApplicationModel::AppService::AppServiceConnection^> AppService::s_connectionMap;


AppService::AppService()
{
}

void AppService::Run(IBackgroundTaskInstance^ taskInstance)
{
	// Get a deferral so that the service isn't terminated.
	m_backgroundTaskDeferral = taskInstance->GetDeferral();

	// Associate a cancellation handler with the background task.
	taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &AppService::OnTaskCanceled);
		
	// Retrieve the app service connection and set up a listener for incoming app service requests.
	auto details = (AppServiceTriggerDetails^)taskInstance->TriggerDetails;
	m_appServiceconnection = details->AppServiceConnection;
	m_appServiceconnection->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(this, &AppService::OnRequestReceived);
}

void AppService::AddListener(Platform::String^ id, AppServiceConnection^ connection)
{
    std::lock_guard<std::mutex> guard(s_mutex);
    s_connectionMap[id] = connection;
}

void AppService::RemoveListener(Platform::String^ id)
{
    std::lock_guard<std::mutex> guard(s_mutex);
    auto iter = s_connectionMap.find(id);
    if (iter != s_connectionMap.end())
    {
        s_connectionMap.erase(iter);
    }
}

void AppService::ForwardMessage(Platform::String^ id, ValueSet^ message, AppServiceRequest^ request, AppServiceDeferral^ deferral)
{
    AppServiceConnection^ appServiceConnection = nullptr;

    {
        std::lock_guard<std::mutex> guard(s_mutex);
        auto iter = s_connectionMap.find(id);
        if (iter != s_connectionMap.end())
        {
            appServiceConnection = iter->second;
        }
    }

    if (appServiceConnection != nullptr)
    {
        auto t = create_task(appServiceConnection->SendMessageAsync(message)).then([this, request, deferral](AppServiceResponse^ response)
        {
            auto status = response->Status;
            create_task(request->SendResponseAsync(response->Message)).then([deferral](AppServiceResponseStatus response)
            {
                deferral->Complete();
            });
        });
    }
    else
    {
        ValueSet^ error = ref new ValueSet;

        std::wstringstream w;
        w << L" Error:" << "Listener with id" << id->Data() << "does not exist" << std::endl;
        error->Insert(L"Error", ref new Platform::String(w.str().c_str()));
        create_task(request->SendResponseAsync(error)).then([deferral](AppServiceResponseStatus response)
        {
            deferral->Complete();
        });
    }
}

void AppService::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    ValueSet^ response = ref new ValueSet();

	// Get a deferral because we use an async API below to respond to the message
	// and we don't want this call to get cancelled while we are waiting.
	auto messageDeferral = args->GetDeferral();

	ValueSet^ request = args->Request->Message;

	if (request->HasKey(L"Message") && request->HasKey(L"Id"))
	{
        Platform::String^ message = dynamic_cast<Platform::String^>(request->Lookup(L"Message"));
        Platform::String^ id = dynamic_cast<Platform::String^>(request->Lookup(L"Id"));

        if (message == L"Register")
        {
            AddListener(id, sender);
            response->Insert(L"Status", L"OK");
        }
        else if (message == L"Unregister")
        {
            RemoveListener(id);
            response->Insert(L"Status", L"OK");
        }
        else if (message == L"Message")
        {
            ForwardMessage(id, request, args->Request, messageDeferral);
            // ForwardMessage handles response and deferral so we can return
            return;
        }
	}

	create_task(args->Request->SendResponseAsync(response)).then([messageDeferral](AppServiceResponseStatus response)
	{
		messageDeferral->Complete();
	});
}

void AppService::OnTaskCanceled(IBackgroundTaskInstance^ sender, BackgroundTaskCancellationReason reason)
{
	if (m_backgroundTaskDeferral != nullptr)
	{
		// Complete the service deferral.
		m_backgroundTaskDeferral->Complete();
	}
}





