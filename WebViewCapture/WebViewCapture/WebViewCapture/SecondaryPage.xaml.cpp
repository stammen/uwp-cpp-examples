//
// SecondaryPage.xaml.cpp
// Implementation of the SecondaryPage class
//

#include "pch.h"
#include "SecondaryPage.xaml.h"
#include <algorithm>
#include <robuffer.h> // IBufferByteAccess
#include <string>

using namespace WebViewCapture;

using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

SecondaryPage::SecondaryPage()
{
	InitializeComponent();
    TimeSpan span;
    span.Duration = 10000000L / 60L;
    m_dispatcherTimer = ref new DispatcherTimer();
    m_timerToken = m_dispatcherTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &SecondaryPage::TimerTick);
    m_dispatcherTimer->Interval = span;

    image1->PointerReleased += ref new PointerEventHandler(this, &SecondaryPage::PointerReleased);

}

void SecondaryPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    m_mainPage = (MainPage^)e->Parameter;
    m_dispatcherTimer->Start();
    m_timer.ResetElapsedTime();
}

void SecondaryPage::PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
    Windows::UI::Input::PointerPoint^ currentPoint = e->GetCurrentPoint(image1);
    auto x = currentPoint->Position.X;
    auto y = currentPoint->Position.Y;
    m_mainPage->PointerReleased(x, y);
}


void WebViewCapture::SecondaryPage::viewButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    m_dispatcherTimer->Stop();
    m_dispatcherTimer->Tick -= m_timerToken;

    // When a Pressed gesture is detected, the application asynchronously switches back to the main (XAML) view
    auto mainView = CoreApplication::MainView;

    auto dispatcher = Windows::UI::Xaml::Window::Current->CoreWindow->Dispatcher;

    // We must launch the switch from the main view's UI thread so we can access its view ID
    mainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, dispatcher]()
    {
        auto viewId = ApplicationView::GetForCurrentView()->Id;

        dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([viewId]()
        {
            auto asyncAction = ApplicationViewSwitcher::SwitchAsync(viewId, ApplicationView::GetForCurrentView()->Id);
        }));
    }));
}


void SecondaryPage::TimerTick(Platform::Object^ sender, Platform::Object^ e)
{
    m_timer.Tick([&]()
    {
        Update();
    });
}

void SecondaryPage::Update()
{
    // display the bitmap
    image1->Source = m_mainPage->GetBitmap();
    std::wstring w = std::to_wstring(m_timer.GetFramesPerSecond());
    frameCount->Text = ref new Platform::String(w.c_str());
}

