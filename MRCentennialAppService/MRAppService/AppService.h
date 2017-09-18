#pragma once

#include <mutex>    
#include <map>    


namespace MRAppService
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AppService  sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
		AppService();

		virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);

	private:
		void OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);
		void OnTaskCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);

        void AppService::AddListener(Platform::String^ id, Windows::ApplicationModel::AppService::AppServiceConnection^ connection);
        void AppService::RemoveListener(Platform::String^ id);

        void BroadcastMessage(Windows::Foundation::Collections::ValueSet^ message, Platform::String^ fromAppId);
        void SendConnectedApps(Platform::String^ id, Windows::ApplicationModel::AppService::AppServiceConnection^ connection);

        void ForwardMessage(
            Platform::String^ id, 
            Windows::Foundation::Collections::ValueSet^ message,
            Windows::ApplicationModel::AppService::AppServiceRequest^ request,
            Windows::ApplicationModel::AppService::AppServiceDeferral^ deferral);

		Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> m_backgroundTaskDeferral = nullptr;
		Windows::ApplicationModel::AppService::AppServiceConnection^ m_appServiceconnection = nullptr;
		static Windows::Foundation::Collections::ValueSet^ s_data;
        static std::mutex s_mutex;
        static std::map<Platform::String^, Windows::ApplicationModel::AppService::AppServiceConnection^> AppService::s_connectionMap;

    };
}
