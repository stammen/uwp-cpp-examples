#pragma once

#include <functional>
#include <ppltasks.h>

#define APPSERVICE_ID L"com.stammen.multiinstanceapp.appservice"


namespace DirectXPageComponent
{ 
    public interface class IAppServiceListenerDelegate
    {
        virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args) = 0;
    };

    ref class AppServiceListener sealed
    {
    public:
        AppServiceListener(Platform::String^ listenerId);
        virtual ~AppServiceListener();
        bool IsConnected() { return m_bAppServiceConnected; }

    internal:
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^> SendAppServiceMessage(Platform::String^ listenerId, Windows::Foundation::Collections::ValueSet^ message);
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceConnectionStatus> ConnectToAppService(Platform::String^ serviceName, Platform::String^ packageFamilyName);
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^>  RegisterListener(IAppServiceListenerDelegate^ delegate);

    private:

        IAppServiceListenerDelegate^ m_delegate;
        void OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);

        Platform::String^                                               m_listenerId;
        Windows::ApplicationModel::AppService::AppServiceConnection^	m_appService;
        bool															m_bAppServiceConnected;
    };
};
