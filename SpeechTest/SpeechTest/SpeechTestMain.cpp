#include "pch.h"
#include "SpeechTestMain.h"
#include "Common\DirectXHelper.h"

#include <windows.graphics.directx.direct3d11.interop.h>
#include <Collection.h>


using namespace SpeechTest;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Holographic;
using namespace Windows::Media::SpeechRecognition;
using namespace Windows::Perception::Spatial;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input::Spatial;
using namespace std::placeholders;



// Loads and initializes application assets when the application is loaded.
SpeechTestMain::SpeechTestMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources)
{
    // Register to be notified if the device is lost or recreated.
    m_deviceResources->RegisterDeviceNotify(this);

    // If connected, a game controller can also be used for input.
    m_gamepadAddedEventToken = Gamepad::GamepadAdded +=
        ref new EventHandler<Gamepad^>(
            bind(&SpeechTestMain::OnGamepadAdded, this, _1, _2)
            );

    m_gamepadRemovedEventToken = Gamepad::GamepadRemoved +=
        ref new EventHandler<Gamepad^>(
            bind(&SpeechTestMain::OnGamepadRemoved, this, _1, _2)
            );

    for (auto const& gamepad : Gamepad::Gamepads)
    {
        OnGamepadAdded(nullptr, gamepad);
    }

    m_speechInput = ref new Speech::SpeechInput();
    InitializeSpeechCommandList();
}

void SpeechTestMain::SetHolographicSpace(HolographicSpace^ holographicSpace)
{
    UnregisterHolographicEventHandlers();

    m_holographicSpace = holographicSpace;

    //
    // TODO: Add code here to initialize your holographic content.
    //

#ifdef DRAW_SAMPLE_CONTENT
    // Initialize the sample hologram.
    m_spinningCubeRenderer = std::make_unique<SpinningCubeRenderer>(m_deviceResources);

    m_spatialInputHandler = std::make_unique<SpatialInputHandler>();
#endif

    // Subscribe for notifications about changes to the state of the default HolographicDisplay 
    // and its SpatialLocator.
    m_holographicDisplayIsAvailableChangedEventToken = HolographicSpace::IsAvailableChanged +=
        ref new EventHandler<Object^>(
            bind(&SpeechTestMain::OnHolographicDisplayIsAvailableChanged, this, _1, _2)
            );

    // Acquire the current state of the default HolographicDisplay and its SpatialLocator.
    OnHolographicDisplayIsAvailableChanged(nullptr, nullptr);

    // Respond to camera added events by creating any resources that are specific
    // to that camera, such as the back buffer render target view.
    // When we add an event handler for CameraAdded, the API layer will avoid putting
    // the new camera in new HolographicFrames until we complete the deferral we created
    // for that handler, or return from the handler without creating a deferral. This
    // allows the app to take more than one frame to finish creating resources and
    // loading assets for the new holographic camera.
    // This function should be registered before the app creates any HolographicFrames.
    m_cameraAddedToken =
        m_holographicSpace->CameraAdded +=
            ref new TypedEventHandler<HolographicSpace^, HolographicSpaceCameraAddedEventArgs^>(
                std::bind(&SpeechTestMain::OnCameraAdded, this, _1, _2)
                );

    // Respond to camera removed events by releasing resources that were created for that
    // camera.
    // When the app receives a CameraRemoved event, it releases all references to the back
    // buffer right away. This includes render target views, Direct2D target bitmaps, and so on.
    // The app must also ensure that the back buffer is not attached as a render target, as
    // shown in DeviceResources::ReleaseResourcesForBackBuffer.
    m_cameraRemovedToken =
        m_holographicSpace->CameraRemoved +=
            ref new TypedEventHandler<HolographicSpace^, HolographicSpaceCameraRemovedEventArgs^>(
                std::bind(&SpeechTestMain::OnCameraRemoved, this, _1, _2)
                );

    // Notes on spatial tracking APIs:
    // * Stationary reference frames are designed to provide a best-fit position relative to the
    //   overall space. Individual positions within that reference frame are allowed to drift slightly
    //   as the device learns more about the environment.
    // * When precise placement of individual holograms is required, a SpatialAnchor should be used to
    //   anchor the individual hologram to a position in the real world - for example, a point the user
    //   indicates to be of special interest. Anchor positions do not drift, but can be corrected; the
    //   anchor will use the corrected position starting in the next frame after the correction has
    //   occurred.
}

void SpeechTestMain::UnregisterHolographicEventHandlers()
{
    if (m_holographicSpace != nullptr)
    {
        // Clear previous event registrations.

        if (m_cameraAddedToken.Value != 0)
        {
            m_holographicSpace->CameraAdded -= m_cameraAddedToken;
            m_cameraAddedToken.Value = 0;
        }

        if (m_cameraRemovedToken.Value != 0)
        {
            m_holographicSpace->CameraRemoved -= m_cameraRemovedToken;
            m_cameraRemovedToken.Value = 0;
        }
    }

    if (m_spatialLocator != nullptr)
    {
        m_spatialLocator->LocatabilityChanged -= m_locatabilityChangedToken;
    }
}

SpeechTestMain::~SpeechTestMain()
{
    // Deregister device notification.
    m_deviceResources->RegisterDeviceNotify(nullptr);

    UnregisterHolographicEventHandlers();

    if (m_gamepadAddedEventToken.Value != 0)
    {
        Gamepad::GamepadAdded -= m_gamepadAddedEventToken;
    }
    if (m_gamepadRemovedEventToken.Value != 0)
    {
        Gamepad::GamepadRemoved -= m_gamepadRemovedEventToken;
    }
    if (m_holographicDisplayIsAvailableChangedEventToken.Value != 0)
    {
        HolographicSpace::IsAvailableChanged -= m_holographicDisplayIsAvailableChangedEventToken;
    }
}

// Updates the application state once per frame.
HolographicFrame^ SpeechTestMain::Update()
{
    UpdateSpeechRecognizer(m_timer.GetTotalSeconds());

    // Before doing the timer update, there is some work to do per-frame
    // to maintain holographic rendering. First, we will get information
    // about the current frame.

    // The HolographicFrame has information that the app needs in order
    // to update and render the current frame. The app begins each new
    // frame by calling CreateNextFrame.
    HolographicFrame^ holographicFrame = m_holographicSpace->CreateNextFrame();

    // Get a prediction of where holographic cameras will be when this frame
    // is presented.
    HolographicFramePrediction^ prediction = holographicFrame->CurrentPrediction;

    // Back buffers can change from frame to frame. Validate each buffer, and recreate
    // resource views and depth buffers as needed.
    m_deviceResources->EnsureCameraResources(holographicFrame, prediction);
    
#ifdef DRAW_SAMPLE_CONTENT
    if (m_stationaryReferenceFrame != nullptr)
    {
        // Check for new input state since the last frame.
        for (auto& gamepadWithButtonState : m_gamepads)
        {
            bool buttonDownThisUpdate = ((gamepadWithButtonState.gamepad->GetCurrentReading().Buttons & GamepadButtons::A) == GamepadButtons::A);
            if (buttonDownThisUpdate && !gamepadWithButtonState.buttonAWasPressedLastFrame)
            {
                m_pointerPressed = true;
            }
            gamepadWithButtonState.buttonAWasPressedLastFrame = buttonDownThisUpdate;
        }

        SpatialInteractionSourceState^ pointerState = m_spatialInputHandler->CheckForInput();
        SpatialPointerPose^ pose = nullptr;
        if (pointerState != nullptr)
        {
            pose = pointerState->TryGetPointerPose(m_stationaryReferenceFrame->CoordinateSystem);
        }
        else if (m_pointerPressed)
        {
            pose = SpatialPointerPose::TryGetAtTimestamp(m_stationaryReferenceFrame->CoordinateSystem, prediction->Timestamp);
        }
        m_pointerPressed = false;

        // When a Pressed gesture is detected, the sample hologram will be repositioned
        // two meters in front of the user.
        m_spinningCubeRenderer->PositionHologram(pose);
    }
#endif

    m_timer.Tick([&] ()
    {
        //
        // TODO: Update scene objects.
        //
        // Put time-based updates here. By default this code will run once per frame,
        // but if you change the StepTimer to use a fixed time step this code will
        // run as many times as needed to get to the current step.
        //

#ifdef DRAW_SAMPLE_CONTENT
        m_spinningCubeRenderer->Update(m_timer);
#endif
    });

    // We complete the frame update by using information about our content positioning
    // to set the focus point.

    for (auto cameraPose : prediction->CameraPoses)
    {
#ifdef DRAW_SAMPLE_CONTENT
        // The HolographicCameraRenderingParameters class provides access to set
        // the image stabilization parameters.
        HolographicCameraRenderingParameters^ renderingParameters = holographicFrame->GetRenderingParameters(cameraPose);

        // SetFocusPoint informs the system about a specific point in your scene to
        // prioritize for image stabilization. The focus point is set independently
        // for each holographic camera.
        // You should set the focus point near the content that the user is looking at.
        // In this example, we put the focus point at the center of the sample hologram,
        // since that is the only hologram available for the user to focus on.
        // You can also set the relative velocity and facing of that content; the sample
        // hologram is at a fixed point so we only need to indicate its position.
        if (m_stationaryReferenceFrame != nullptr)
        {
            renderingParameters->SetFocusPoint(
                m_stationaryReferenceFrame->CoordinateSystem,
                m_spinningCubeRenderer->GetPosition()
                );
        }
#endif
    }

    // The holographic frame will be used to get up-to-date view and projection matrices and
    // to present the swap chain.
    return holographicFrame;
}

// Renders the current frame to each holographic camera, according to the
// current application and spatial positioning state. Returns true if the
// frame was rendered to at least one camera.
bool SpeechTestMain::Render(HolographicFrame^ holographicFrame)
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

    //
    // TODO: Add code for pre-pass rendering here.
    //
    // Take care of any tasks that are not specific to an individual holographic
    // camera. This includes anything that doesn't need the final view or projection
    // matrix, such as lighting maps.
    //

    // Lock the set of holographic camera resources, then draw to each camera
    // in this frame.
    return m_deviceResources->UseHolographicCameraResources<bool>(
        [this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
    {
        // Up-to-date frame predictions enhance the effectiveness of image stablization and
        // allow more accurate positioning of holograms.
        holographicFrame->UpdateCurrentPrediction();
        HolographicFramePrediction^ prediction = holographicFrame->CurrentPrediction;

        bool atLeastOneCameraRendered = false;
        for (auto cameraPose : prediction->CameraPoses)
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose->HolographicCamera->Id].get();

            // Get the device context.
            const auto context = m_deviceResources->GetD3DDeviceContext();
            const auto depthStencilView = pCameraResources->GetDepthStencilView();

            // Set render targets to the current holographic camera.
            ID3D11RenderTargetView *const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
            context->OMSetRenderTargets(1, targets, depthStencilView);

            // Clear the back buffer and depth stencil view.
            context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
            context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            //
            // TODO: Replace the sample content with your own content.
            //
            // Notes regarding holographic content:
            //    * For drawing, remember that you have the potential to fill twice as many pixels
            //      in a stereoscopic render target as compared to a non-stereoscopic render target
            //      of the same resolution. Avoid unnecessary or repeated writes to the same pixel,
            //      and only draw holograms that the user can see.
            //    * To help occlude hologram geometry, you can create a depth map using geometry
            //      data obtained via the surface mapping APIs. You can use this depth map to avoid
            //      rendering holograms that are intended to be hidden behind tables, walls,
            //      monitors, and so on.
            //    * Black pixels will appear transparent to the user wearing the device, but you
            //      should still use alpha blending to draw semitransparent holograms. You should
            //      also clear the screen to Transparent as shown above.
            //


            // The view and projection matrices for each holographic camera will change
            // every frame. This function refreshes the data in the constant buffer for
            // the holographic camera indicated by cameraPose.
            if (m_stationaryReferenceFrame)
            {
                pCameraResources->UpdateViewProjectionBuffer(m_deviceResources, cameraPose, m_stationaryReferenceFrame->CoordinateSystem);
            }

            // Attach the view/projection constant buffer for this camera to the graphics pipeline.
            bool cameraActive = pCameraResources->AttachViewProjectionBuffer(m_deviceResources);

#ifdef DRAW_SAMPLE_CONTENT
            // Only render world-locked content when positional tracking is active.
            if (cameraActive)
            {
                // Draw the sample hologram.
                m_spinningCubeRenderer->Render();
            }
#endif
            atLeastOneCameraRendered = true;
        }

        return atLeastOneCameraRendered;
    });
}

void SpeechTestMain::SaveAppState()
{
    //
    // TODO: Insert code here to save your app state.
    //       This method is called when the app is about to suspend.
    //
    //       For example, store information in the SpatialAnchorStore.
    //
}

void SpeechTestMain::LoadAppState()
{
    //
    // TODO: Insert code here to load your app state.
    //       This method is called when the app resumes.
    //
    //       For example, load information from the SpatialAnchorStore.
    //
}

void SpeechTestMain::OnPointerPressed()
{
    m_pointerPressed = true;
}

// Notifies classes that use Direct3D device resources that the device resources
// need to be released before this method returns.
void SpeechTestMain::OnDeviceLost()
{
#ifdef DRAW_SAMPLE_CONTENT
    m_spinningCubeRenderer->ReleaseDeviceDependentResources();
#endif
}

// Notifies classes that use Direct3D device resources that the device resources
// may now be recreated.
void SpeechTestMain::OnDeviceRestored()
{
#ifdef DRAW_SAMPLE_CONTENT
    m_spinningCubeRenderer->CreateDeviceDependentResources();
#endif
}

void SpeechTestMain::OnLocatabilityChanged(SpatialLocator^ sender, Object^ args)
{
    switch (sender->Locatability)
    {
    case SpatialLocatability::Unavailable:
        // Holograms cannot be rendered.
        {
            String^ message = L"Warning! Positional tracking is " +
                                        sender->Locatability.ToString() + L".\n";
            OutputDebugStringW(message->Data());
        }
        break;

    // In the following three cases, it is still possible to place holograms using a
    // SpatialLocatorAttachedFrameOfReference.
    case SpatialLocatability::PositionalTrackingActivating:
        // The system is preparing to use positional tracking.

    case SpatialLocatability::OrientationOnly:
        // Positional tracking has not been activated.

    case SpatialLocatability::PositionalTrackingInhibited:
        // Positional tracking is temporarily inhibited. User action may be required
        // in order to restore positional tracking.
        break;

    case SpatialLocatability::PositionalTrackingActive:
        // Positional tracking is active. World-locked content can be rendered.
        break;
    }
}

void SpeechTestMain::OnCameraAdded(
    HolographicSpace^ sender,
    HolographicSpaceCameraAddedEventArgs^ args
    )
{
    Deferral^ deferral = args->GetDeferral();
    HolographicCamera^ holographicCamera = args->Camera;
    create_task([this, deferral, holographicCamera] ()
    {
        //
        // TODO: Allocate resources for the new camera and load any content specific to
        //       that camera. Note that the render target size (in pixels) is a property
        //       of the HolographicCamera object, and can be used to create off-screen
        //       render targets that match the resolution of the HolographicCamera.
        //

        // Create device-based resources for the holographic camera and add it to the list of
        // cameras used for updates and rendering. Notes:
        //   * Since this function may be called at any time, the AddHolographicCamera function
        //     waits until it can get a lock on the set of holographic camera resources before
        //     adding the new camera. At 60 frames per second this wait should not take long.
        //   * A subsequent Update will take the back buffer from the RenderingParameters of this
        //     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
        //     Content can then be rendered for the HolographicCamera.
        m_deviceResources->AddHolographicCamera(holographicCamera);

        // Holographic frame predictions will not include any information about this camera until
        // the deferral is completed.
        deferral->Complete();
    });
}

void SpeechTestMain::OnCameraRemoved(
    HolographicSpace^ sender,
    HolographicSpaceCameraRemovedEventArgs^ args
    )
{
    create_task([this]()
    {
        //
        // TODO: Asynchronously unload or deactivate content resources (not back buffer 
        //       resources) that are specific only to the camera that was removed.
        //
    });

    // Before letting this callback return, ensure that all references to the back buffer 
    // are released.
    // Since this function may be called at any time, the RemoveHolographicCamera function
    // waits until it can get a lock on the set of holographic camera resources before
    // deallocating resources for this camera. At 60 frames per second this wait should
    // not take long.
    m_deviceResources->RemoveHolographicCamera(args->Camera);
}

void SpeechTestMain::OnGamepadAdded(Object^, Gamepad^ args)
{
    for (auto const& gamepadWithButtonState : m_gamepads)
    {
        if (args == gamepadWithButtonState.gamepad)
        {
            // This gamepad is already in the list.
            return;
        }
    }

    GamepadWithButtonState newGamepad = { args, false };
    m_gamepads.push_back(newGamepad);
}

void SpeechTestMain::OnGamepadRemoved(Object^, Gamepad^ args)
{
    m_gamepads.erase(std::remove_if(m_gamepads.begin(), m_gamepads.end(), [&](GamepadWithButtonState& gamepadWithState)
        {
            return gamepadWithState.gamepad == args;
        }),
        m_gamepads.end());
}

void SpeechTestMain::OnHolographicDisplayIsAvailableChanged(Object^, Object^)
{
    // Get the spatial locator for the default HolographicDisplay, if one is available.
    SpatialLocator^ spatialLocator = nullptr;
    HolographicDisplay^ defaultHolographicDisplay = HolographicDisplay::GetDefault();
    if (defaultHolographicDisplay)
    {
        spatialLocator = defaultHolographicDisplay->SpatialLocator;
    }

    if (m_spatialLocator != spatialLocator)
    {
        // If the spatial locator is disconnected or replaced, we should discard all state that was
        // based on it.
        if (m_spatialLocator != nullptr)
        {
            m_spatialLocator->LocatabilityChanged -= m_locatabilityChangedToken;
            m_spatialLocator = nullptr;
        }

        m_stationaryReferenceFrame = nullptr;

        if (spatialLocator != nullptr)
        {
            // Use the SpatialLocator from the default HolographicDisplay to track the motion of the device.
            m_spatialLocator = spatialLocator;

            // Respond to changes in the positional tracking state.
            m_locatabilityChangedToken =
                m_spatialLocator->LocatabilityChanged +=
                ref new TypedEventHandler<SpatialLocator^, Object^>(
                    std::bind(&SpeechTestMain::OnLocatabilityChanged, this, _1, _2)
                    );

            // The simplest way to render world-locked holograms is to create a stationary reference frame
            // based on a SpatialLocator. This is roughly analogous to creating a "world" coordinate system
            // with the origin placed at the device's position as the app is launched.
            m_stationaryReferenceFrame = m_spatialLocator->CreateStationaryFrameOfReferenceAtCurrentLocation();
        }
    }
}

void SpeechTestMain::InitializeSpeechCommandList()
{
    m_speechCommandData = ref new Platform::Collections::Map<String^, float4>();

    m_speechCommandData->Insert(L"white", float4(1.0f, 1.0f, 1.0f, 1.f));
    m_speechCommandData->Insert(L"grey", float4(0.5f, 0.5f, 0.5f, 1.f));
    m_speechCommandData->Insert(L"green", float4(0.0f, 1.0f, 0.0f, 1.f));
    m_speechCommandData->Insert(L"black", float4(0.1f, 0.1f, 0.1f, 1.f));
    m_speechCommandData->Insert(L"red", float4(1.0f, 0.0f, 0.0f, 1.f));
    m_speechCommandData->Insert(L"yellow", float4(1.0f, 1.0f, 0.0f, 1.f));
    m_speechCommandData->Insert(L"aquamarine", float4(0.0f, 1.0f, 1.0f, 1.f));
    m_speechCommandData->Insert(L"blue", float4(0.0f, 0.0f, 1.0f, 1.f));
    m_speechCommandData->Insert(L"purple", float4(1.0f, 0.0f, 1.0f, 1.f));

    // You can use non-dictionary words as speech commands.
    m_speechCommandData->Insert(L"SpeechRecognizer", float4(0.5f, 0.1f, 1.f, 1.f));
}

void SpeechTestMain::InitializeMicrophone()
{
    m_speechInput->Available().then([this](bool hasMicPermission)
    {
        m_hasMicPermission = hasMicPermission;
        if (m_hasMicPermission)
        {
            OutputDebugString(L"Microphone permission enabled\n");
        }
        else
        {
            OutputDebugString(L"Microphone permission disabled\n");
        }
    });
}

void SpeechTestMain::InitializeSpeech()
{
    m_speechInput->Available().then([this](bool hasMicPermission)
    {
        if (true == hasMicPermission)
        {
            OutputDebugString(L"Have microphone permissions\n");

            // Here, we compile the list of voice commands by reading them from the map.
            Platform::Collections::Vector<String^>^ speechCommandList = ref new Platform::Collections::Vector<String^>();
            for each (auto pair in m_speechCommandData)
            {
                speechCommandList->Append(pair->Key);
            }

            m_speechInput->Initialize(speechCommandList).then([this](bool result)
            {
                if (true == result)
                {
                    OutputDebugString(L"Started recognizing speech commands\n");
                    m_speechInput->Start().then([this](bool result) {
                        m_speechInput->SetDelegate(this);
                    });
                }
                else
                {
                    OutputDebugString(L"Could NOT start recognizing speech commands\n");
                }
            });
        }
        else
        {
            OutputDebugString(L"Could not get microphone permissions\n");
        }
    });
}


void SpeechTestMain::InitializeSpeechWithDelay()
{
    Windows::Foundation::TimeSpan delay;
    delay.Duration = 6 * 10000000; // 6 seconds

    CoreDispatcher^ dispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;

    ThreadPoolTimer ^ PeriodicTimer = ThreadPoolTimer::CreateTimer(
        ref new TimerElapsedHandler([this, dispatcher](ThreadPoolTimer^ source) {

        // run this ThreadPoolTimer on the main UI thread
        dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, dispatcher]()
        {
            InitializeSpeech();
        }));
    }), delay);
}

void SpeechTestMain::OnActivated(bool activated)
{
    m_activated = activated;
}

void SpeechTestMain::UpdateSpeechRecognizer(double time)
{
    if (time > 6.0)
    {
        if (m_activated && !m_micInitialized)
        {
            m_micInitialized = true;
            InitializeMicrophone();
            OutputDebugString(L"Requesting mic permission\n");
        }

        if (m_hasMicPermission)
        {
            if (m_activated && !m_speechInitialized)
            {
                m_speechInitialized = true;
                InitializeSpeech();
                OutputDebugString(L"Starting speech recognition\n");
            }
            else if (!m_activated && m_speechInitialized)
            {
                m_speechInitialized = false;
                m_micInitialized = false;
                m_hasMicPermission = false;
                m_speechInput->Stop();
                OutputDebugString(L"Stopping speech recognition\n");
            }
        }
    }
}

void SpeechTestMain::OnSpeechResultGenerated(SpeechContinuousRecognitionSession ^sender, SpeechContinuousRecognitionResultGeneratedEventArgs ^args)
{
    OutputDebugString(L"OnResultGenerated\n");

    // For our list of commands, medium confidence is good enough. 
    // We also accept results that have high confidence.
    if ((args->Result->Confidence == SpeechRecognitionConfidence::High) ||
        (args->Result->Confidence == SpeechRecognitionConfidence::Medium))
    {
        if (m_speechCommandData->HasKey(args->Result->Text))
        {
            m_spinningCubeRenderer->SetColor(m_speechCommandData->Lookup(args->Result->Text));

        }

        // When the debugger is attached, we can print information to the debug console.
        OutputDebugString(
            (std::wstring(L"Last command was: ") +
                args->Result->Text->Data() +
                L"\n").c_str()
        );
    }
    else
    {
        OutputDebugStringW(L"Recognition confidence not high enough.\n");
    }
}

void SpeechTestMain::OnSpeechQualityDegraded(Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognitionQualityDegradingEventArgs^ args)
{
    switch (args->Problem)
    {
    case SpeechRecognitionAudioProblem::TooFast:
        OutputDebugStringW(L"The user spoke too quickly.\n");
        break;

    case SpeechRecognitionAudioProblem::TooSlow:
        OutputDebugStringW(L"The user spoke too slowly.\n");
        break;

    case SpeechRecognitionAudioProblem::TooQuiet:
        OutputDebugStringW(L"The user spoke too softly.\n");
        break;

    case SpeechRecognitionAudioProblem::TooLoud:
        OutputDebugStringW(L"The user spoke too loudly.\n");
        break;

    case SpeechRecognitionAudioProblem::TooNoisy:
        OutputDebugStringW(L"There is too much noise in the signal.\n");
        break;

    case SpeechRecognitionAudioProblem::NoSignal:
        OutputDebugStringW(L"There is no signal.\n");
        break;

    case SpeechRecognitionAudioProblem::None:
    default:
        OutputDebugStringW(L"An error was reported with no information.\n");
        break;
    }
}

void SpeechTest::SpeechTestMain::OnRecognizerStateChanged(Windows::Media::SpeechRecognition::SpeechRecognizer ^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognizerStateChangedEventArgs ^ args)
{
    switch (args->State)
    {
    case SpeechRecognizerState::Capturing:
        OutputDebugString(L"OnRecognizerStateChanged: Capturing\n");
        break;

    case SpeechRecognizerState::Idle:
        OutputDebugString(L"OnRecognizerStateChanged: Idle\n");
        break;

    case SpeechRecognizerState::Paused:
        OutputDebugString(L"OnRecognizerStateChanged: Paused\n");
        break;

    case SpeechRecognizerState::Processing:
        OutputDebugString(L"OnRecognizerStateChanged: Processing\n");
        break;

    case SpeechRecognizerState::SoundStarted:
        OutputDebugString(L"OnRecognizerStateChanged: Sound Started\n");
        break;

    case SpeechRecognizerState::SoundEnded:
        OutputDebugString(L"OnRecognizerStateChanged: Sound Ended\n");
        break;

    case SpeechRecognizerState::SpeechDetected:
        OutputDebugString(L"OnRecognizerStateChanged: Speech Detected\n");
        break;
    }
}



