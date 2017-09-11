#include "pch.h"
#include "MRAppServiceListener.h"

#include <ppltasks.h>

using namespace Concurrency;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace MRAppService;

MRAppServiceListener::MRAppServiceListener(Platform::String^ listenerId)
    : m_listenerId(listenerId)
    , m_appService(nullptr)
    , m_bAppServiceConnected(false)
    , m_delegate(nullptr)
{
    
}

MRAppServiceListener::~MRAppServiceListener()
{
}


Concurrency::task<AppServiceConnectionStatus> MRAppServiceListener::ConnectToAppService(Platform::String^ serviceName, Platform::String^ packageFamilyName)
{
    m_appService = ref new AppServiceConnection();
    m_appService->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(this, &MRAppServiceListener::OnRequestReceived);

    // Here, we use the app service name defined in the app service provider's Package.appxmanifest file in the <Extension> section.
    m_appService->AppServiceName = serviceName;

    // Use Windows.ApplicationModel.Package.Current.Id.FamilyName within the app service provider to get this value.
    m_appService->PackageFamilyName = packageFamilyName;

    return create_task(m_appService->OpenAsync()).then([this](AppServiceConnectionStatus status)
    {
        if (status != AppServiceConnectionStatus::Success)
        {
            m_appService = nullptr;
        }
        else
        {
            OutputDebugString(L"Connected to AppService.\n");
            m_bAppServiceConnected = true;
        }

        return status;
    });
}

Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^> MRAppServiceListener::RegisterListener(IMRAppServiceListenerDelegate* delegate)
{
    auto request = ref new ValueSet();
    request->Insert(L"Message", L"Register");
    request->Insert(L"Id", m_listenerId);

    return create_task(m_appService->SendMessageAsync(request)).then([this, delegate](AppServiceResponse^ response)
    {
        if (response->Status == AppServiceResponseStatus::Success)
        {
            m_delegate = delegate;
        }
        else
        {
            m_delegate = nullptr;
        }
        return response;
    });
}

Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^> MRAppServiceListener::SendAppServiceMessage(Platform::String^ listenerId, ValueSet^ message)
{
    ValueSet^ request = ref new ValueSet();
    request->Insert(L"Message", L"Message");
    request->Insert(L"Id", listenerId);
    request->Insert(L"Data", message);
    return create_task(m_appService->SendMessageAsync(request)).then([this](AppServiceResponse^ response)
    {
        auto status = response->Status;
        return response;
    });
}

void MRAppServiceListener::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    ValueSet^ response = ref new ValueSet();

    if (m_delegate)
    {
        response = m_delegate->OnRequestReceived(sender, args);
    }

    // Get a deferral because we use an async API below to respond to the message
    // and we don't want this call to get cancelled while we are waiting.
    auto messageDeferral = args->GetDeferral();

    create_task(args->Request->SendResponseAsync(response)).then([messageDeferral](AppServiceResponseStatus response)
    {
        messageDeferral->Complete();
    });
}
