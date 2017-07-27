//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "AppView.h"

using namespace HolographicWebView;

using namespace Platform;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::button1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (m_holographicView.Get() == nullptr)
    {
        try
        {
            m_holographicView = CoreApplication::CreateNewView(ref new HolographicWebView::AppViewSource());
        }
        catch (Platform::COMException^ e)
        {
            // This exception is thrown if the environment doesn't support holographic content
            statusText->Text = L"Holographic environment not available.";
            return;
        }
    }

    webview1->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    // We must launch the switch from the exclusive view's UI thread so we can access its view ID
    m_holographicView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
    {
        auto viewId = ApplicationView::GetForCurrentView()->Id;
        CoreWindow::GetForCurrentThread()->Activate();

        // But the main view must do the actual switching, so we run it from it's thread
        CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([viewId]()
        {
            auto asyncAction = ApplicationViewSwitcher::SwitchAsync(viewId, ApplicationView::GetForCurrentView()->Id);
        }));
    }));
}

void MainPage::DisplayWebView(Platform::String^ url)
{
    Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri(url);
    webview1->Visibility = Windows::UI::Xaml::Visibility::Visible;
    webview1->Source = uri;
}

