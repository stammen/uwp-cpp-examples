#pragma once

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
		Concurrency::task<bool> LaunchAppWithProtocol(Platform::String^ protocol);

		Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> m_backgroundTaskDeferral = nullptr;
		Windows::ApplicationModel::AppService::AppServiceConnection^ m_appServiceconnection = nullptr;
		static unsigned int m_data;
    };
}
