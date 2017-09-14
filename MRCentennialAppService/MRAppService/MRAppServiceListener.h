#pragma once

#include <functional>
#include <ppltasks.h>

#define MRAPPSERVICE_ID L"com.screencapture.appservice"
#define MRAPPSERVICE_FAMILY_NAME L"544d40ad-b0d8-4ed4-a545-fec1fbe581a3_e8xk87pxx0yyw"

namespace MRAppService
{ 
    interface IMRAppServiceListenerDelegate
    {
        virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args) = 0;
        virtual void OnServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args) = 0;
    };

    ref class MRAppServiceListener sealed
    {
    public:
        MRAppServiceListener(Platform::String^ listenerId);
        virtual ~MRAppServiceListener();
        bool IsConnected() { return m_bAppServiceConnected; }

    internal:
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^> SendAppServiceMessage(Platform::String^ listenerId, Windows::Foundation::Collections::ValueSet^ message);
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceConnectionStatus> ConnectToAppService(Platform::String^ serviceName, Platform::String^ packageFamilyName);
        Concurrency::task<Windows::ApplicationModel::AppService::AppServiceResponse^>  RegisterListener(IMRAppServiceListenerDelegate* delegate);

    private:

        IMRAppServiceListenerDelegate* m_delegate;
        void OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);
        void OnAppServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args);


        Platform::String^                                               m_listenerId;
        Windows::ApplicationModel::AppService::AppServiceConnection^	m_appService;
        bool															m_bAppServiceConnected;
    };
};
