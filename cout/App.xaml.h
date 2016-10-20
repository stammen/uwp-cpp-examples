//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"
#include "OutputDebugStringBuf.h"

namespace cout
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);

#ifdef _DEBUG
        // overrides std::cout and std::wcout to use DebugOutputString
        OutputDebugStringBufA m_charDebugOutput;
        OutputDebugStringBufW m_wcharDebugOutput;
#endif
	};
}
