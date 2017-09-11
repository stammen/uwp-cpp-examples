#pragma once

#include <functional>
#include <ppltasks.h>

#define MRAPPSERVICE_ID L"com.mrappservicedemo.appservice"
#define MRAPPSERVICE_FAMILY_NAME L"661fcf9b-01c2-450d-be4b-a62a0fe9913c_e8xk87pxx0yyw"


namespace MRAppService
{ 
    interface IMRAppServiceListenerDelegate
    {
        virtual Windows::Foundation::Collections::ValueSet^ OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args) = 0;
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

        Platform::String^                                               m_listenerId;
        Windows::ApplicationModel::AppService::AppServiceConnection^	m_appService;
        bool															m_bAppServiceConnected;
    };
};
