#include "pch.h"
#include "AppService.h"
#include <ppltasks.h>    

using namespace concurrency;
using namespace MRAppService;
using namespace Platform;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

float AppService::m_data = 2.0f;

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

void AppService::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
	// Get a deferral because we use an async API below to respond to the message
	// and we don't want this call to get cancelled while we are waiting.
	auto messageDeferral = args->GetDeferral();

	ValueSet^ message = args->Request->Message;

	if (message->HasKey(L"LaunchApp"))
	{
		Platform::String^ protocol = dynamic_cast<Platform::String^>(message->Lookup(L"LaunchApp"));
		auto task = LaunchAppWithProtocol(protocol);
		task.then([this, messageDeferral, args](bool result)
		{
			ValueSet^ returnData = ref new ValueSet();
			returnData->Insert(L"Result", result);
			create_task(args->Request->SendResponseAsync(returnData)).then([messageDeferral](AppServiceResponseStatus response)
			{
				messageDeferral->Complete();
			});
		});
	}
	else if (message->HasKey(L"PostData"))
	{
		m_data = safe_cast<float>(message->Lookup(L"PostData"));
		ValueSet^ returnData = ref new ValueSet();
		returnData->Insert(L"Status", 1);
		create_task(args->Request->SendResponseAsync(returnData)).then([messageDeferral](AppServiceResponseStatus response)
		{
			messageDeferral->Complete();
		});
	}
	else
	{
		ValueSet^ returnData = ref new ValueSet();
		returnData->Insert(L"Result", m_data);

		create_task(args->Request->SendResponseAsync(returnData)).then([messageDeferral](AppServiceResponseStatus response)
		{
			messageDeferral->Complete();
		});
	}
}

void AppService::OnTaskCanceled(IBackgroundTaskInstance^ sender, BackgroundTaskCancellationReason reason)
{
	if (m_backgroundTaskDeferral != nullptr)
	{
		// Complete the service deferral.
		m_backgroundTaskDeferral->Complete();
	}
}

Concurrency::task<bool> AppService::LaunchAppWithProtocol(Platform::String^ protocol)
{
	auto uri = ref new Uri(protocol); // The protocol handled by the launched app
	auto options = ref new LauncherOptions();
	return create_task(Launcher::LaunchUriAsync(uri, options));
}




