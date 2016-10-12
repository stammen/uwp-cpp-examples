//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace WebViewExample;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();

    m_helper = ref new WindowsRuntimeComponent1::Class1();
    m_helper->OnJavaScriptCallback += ref new WindowsRuntimeComponent1::JavaScriptCallbackHandler(this, &MainPage::JavaScriptCallbackHandler);
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    //auto uri = ref new Uri("http://www.microsoft.com");
    auto uri = ref new Uri("ms-appx-web:///Assets/index.html");
    m_webToken = webView->NavigationStarting += ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>(this, &MainPage::webView_NavigationStarting);
    webView->Navigate(uri);
}

void MainPage::OnNavigatedFrom(NavigationEventArgs^ e)
{
    webView->NavigationStarting -= m_webToken;
}

void MainPage::webView_NavigationStarting(WebView^ sender, WebViewNavigationStartingEventArgs^ args)
{
    if (args->Uri->Path == "/Assets/index.html")
    {
        webView->AddWebAllowedObject("class1", m_helper);
    }
}

// This is called on the Xaml UI thread when WindowsRuntimeComponent1::Class1 receives a callback from JavaScript
void MainPage::JavaScriptCallbackHandler(WindowsRuntimeComponent1::Class1^ sender, Platform::String^ args)
{
    // Show the message dialog
    auto msg = ref new Windows::UI::Popups::MessageDialog(args, "JavaScriptCallbackHandler");
    // Set the command to be invoked when a user presses 'ESC'
    msg->CancelCommandIndex = 1;
    msg->ShowAsync();
}



