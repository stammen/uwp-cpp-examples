//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "WebViewPage.xaml.h"
#include "ProtocolArgs.h"
#include "AppActivation.h"
#include <sstream> 

using namespace DirectXPageComponent;

using namespace concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;


Platform::String^ DirectXPage::PageName()
{
    return L"directxpage";
}

DirectXPage::DirectXPage():
	m_windowVisible(true),
    m_appServiceConnected(false),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

    window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyPressed);
    window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &DirectXPage::OnCharacterReceived);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);


        // Register for pointer events, which will be raised on the background thread.
        m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
        m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
        m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<DirectXMain>(new DirectXMain(m_deviceResources));
	m_main->StartRenderLoop();
}

void DirectXPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
    if (m_appServiceListener == nullptr)
    {
        m_appServiceListener = ref new AppServiceListener(L"DirectXPage");
        auto connectTask = m_appServiceListener->ConnectToAppService(APPSERVICE_ID, Windows::ApplicationModel::Package::Current->Id->FamilyName);
        connectTask.then([this](AppServiceConnectionStatus response)
        {
            if (response == AppServiceConnectionStatus::Success)
            {
                auto listenerTask = m_appServiceListener->RegisterListener(this).then([this](AppServiceResponse^ response)
                {
                    if (response->Status == AppServiceResponseStatus::Success)
                    {
                        m_appServiceConnected = true;
                        OutputDebugString(L"DirectXPage is connected to the App Service");
                    }
                });
            }
        });
    }
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}


void DirectXPage::SendKeyboardEvent(Platform::String^ eventType, unsigned int keyCode)
{
    ValueSet^ message = ref new ValueSet();
    message->Insert(L"KeyboardMessage", eventType);
    message->Insert(L"Key", keyCode);
    m_appServiceListener->SendAppServiceMessage(L"WebView", message).then([this](AppServiceResponse^ response)
    {
        auto responseMessage = response->Message;

        if (response->Status == AppServiceResponseStatus::Success)
        {
        }
        else
        {
        }
    });
}

void DirectXPage::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
    SendKeyboardEvent(L"OnKeyPressed", args->KeyCode);
}

void DirectXPage::OnKeyPressed(CoreWindow^ sender, KeyEventArgs^ args)
{
    switch (args->VirtualKey)
    {
    case VirtualKey::Back:
        SendKeyboardEvent(L"OnKeyPressed", (unsigned int)args->VirtualKey);
        break;
    }
}

void DirectXPage::SendPointerMessage(Platform::String^ pointerEvent, float x, float y)
{
    if (!m_appServiceConnected)
    {
        return;
    }

    ValueSet^ message = ref new ValueSet();
    message->Insert(L"PointerMessage", pointerEvent);
    message->Insert(L"x", x);
    message->Insert(L"y", y);
    m_appServiceListener->SendAppServiceMessage(L"WebView", message).then([this](AppServiceResponse^ response)
    {
        auto responseMessage = response->Message;

        if (response->Status == AppServiceResponseStatus::Success)
        {
        }
        else
        {
        }
    });
}


void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
    SendPointerMessage(L"OnPointerPressed", e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
    SendPointerMessage(L"OnPointerMoved", e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
    SendPointerMessage(L"OnPointerReleased", e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Use the app bar if it is appropriate for your app. Design the app bar, 
	// then fill in event handlers (like this one).
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

ValueSet^ DirectXPage::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    ValueSet^ request = args->Request->Message;
    ValueSet^ message = safe_cast<ValueSet^>(request->Lookup(L"Data"));

    if (message->HasKey("fps"))
    {
        CoreApplication::MainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, message]()
        {
            unsigned int fps = (unsigned int)(message->Lookup(L"fps"));
            std::wstringstream w;
            w << L"WebView texture updated at " << fps << L" FPS" << std::endl;
            fpsText->Text = ref new Platform::String(w.str().c_str());
        }));
    }

    auto response = ref new ValueSet();
    response->Insert(L"Status", L"OK");
    return response;
}


void DirectXPageComponent::DirectXPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    LaunchAppInstance();
}

Concurrency::task<bool> DirectXPage::LaunchAppInstance()
{
    Platform::String^ protocol = APP_PROTOCOL + ":?id=1&apptype=" + WebViewPage::PageName() + "&sharedtexture=DirectXPageSharedTexture&width=512&height=512&source=https://www.google.com&fps=60";
    return AppActivation::LaunchAppWithProtocol(protocol);
}
