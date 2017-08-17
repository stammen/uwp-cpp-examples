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

AppService::AppService()
	: m_requestCount(0)
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
	ValueSet^ returnData = ref new ValueSet();

	returnData->Insert(L"Result", ++m_requestCount);

	create_task(args->Request->SendResponseAsync(returnData)).then([messageDeferral](AppServiceResponseStatus response)
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

