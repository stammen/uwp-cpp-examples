#pragma once

//
// Comment out this preprocessor definition to disable all of the
// sample content.
//
// To remove the content after disabling it:
//     * Remove the unused code from your app's Main class.
//     * Delete the Content folder provided with this template.
//
#define DRAW_SAMPLE_CONTENT

#include "Common\DeviceResources.h"
#include "Common\StepTimer.h"
#include "Content\SpeechInput.h"

#ifdef DRAW_SAMPLE_CONTENT
#include "Content\SpinningCubeRenderer.h"
#include "Content\SpatialInputHandler.h"
#endif

#include <ppltasks.h>
#include <collection.h>
#include <vector>

// Updates, renders, and presents holographic content using Direct3D.
namespace SpeechTest
{
    class SpeechTestMain : public DX::IDeviceNotify, public Speech::IMRAppServiceListenerDelegate
    {
    public:
        SpeechTestMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        ~SpeechTestMain();

        // Sets the holographic space. This is our closest analogue to setting a new window
        // for the app.
        void SetHolographicSpace(Windows::Graphics::Holographic::HolographicSpace^ holographicSpace);

        // Starts the holographic frame and updates the content.
        Windows::Graphics::Holographic::HolographicFrame^ Update();

        // Renders holograms, including world-locked content.
        bool Render(Windows::Graphics::Holographic::HolographicFrame^ holographicFrame);

        // Handle saving and loading of app state owned by AppMain.
        void SaveAppState();
        void LoadAppState();

        // Handle mouse input.
        void OnPointerPressed();

        // IDeviceNotify
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();

        // Initializes the speech command list.
        void InitializeSpeechCommandList();

		void InitializeSpeechWithDelay();
        void InitializeSpeech();

        // Handle HMD activated event
        void OnActivated(bool activated);

    private:
        void UpdateSpeechRecognizer(double time);
        void InitializeMicrophone();

        // Asynchronously creates resources for new holographic cameras.
        void OnCameraAdded(
            Windows::Graphics::Holographic::HolographicSpace^ sender,
            Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs^ args);

        // Synchronously releases resources for holographic cameras that are no longer
        // attached to the system.
        void OnCameraRemoved(
            Windows::Graphics::Holographic::HolographicSpace^ sender,
            Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs^ args);

        // Used to notify the app when the positional tracking state changes.
        void OnLocatabilityChanged(
            Windows::Perception::Spatial::SpatialLocator^ sender,
            Platform::Object^ args);

        // Used to be aware of gamepads that are plugged in after the app starts.
        void OnGamepadAdded(Platform::Object^, Windows::Gaming::Input::Gamepad^ args);

        // Used to stop updating gamepads that are removed while the app is running.
        void OnGamepadRemoved(Platform::Object^, Windows::Gaming::Input::Gamepad^ args);

        // Used to respond to changes to the default spatial locator.
        void OnHolographicDisplayIsAvailableChanged(Platform::Object^, Platform::Object^);

        // Clears event registration state. Used when changing to a new HolographicSpace
        // and when tearing down AppMain.
        void UnregisterHolographicEventHandlers();

     // Process continuous speech recognition results.
        void OnSpeechResultGenerated(
            Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession ^sender,
            Windows::Media::SpeechRecognition::SpeechContinuousRecognitionResultGeneratedEventArgs ^args
        );

        // Recognize when conditions might impact speech recognition quality.
        void OnSpeechQualityDegraded(
            Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer,
            Windows::Media::SpeechRecognition::SpeechRecognitionQualityDegradingEventArgs^ args
        );

		void OnRecognizerStateChanged(
			Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer,
			Windows::Media::SpeechRecognition::SpeechRecognizerStateChangedEventArgs^ args
		);
 


        // Maps commands to color data.
        // We will create a Vector of the key values in this map for use as speech commands.
		Platform::Collections::Map<Platform::String^, Windows::Foundation::Numerics::float4>^ m_speechCommandData;

#ifdef DRAW_SAMPLE_CONTENT
        // Renders a colorful holographic cube that's 20 centimeters wide. This sample content
        // is used to demonstrate world-locked rendering.
        std::unique_ptr<SpinningCubeRenderer>                           m_spinningCubeRenderer;

        // Listens for the Pressed spatial input event.
        std::shared_ptr<SpatialInputHandler>                            m_spatialInputHandler;
#endif

        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources>                            m_deviceResources;

        // Render loop timer.
        DX::StepTimer                                                   m_timer;

        // Represents the holographic space around the user.
        Windows::Graphics::Holographic::HolographicSpace^               m_holographicSpace;

        // SpatialLocator that is attached to the default HolographicDisplay.
        Windows::Perception::Spatial::SpatialLocator^                   m_spatialLocator;

        // A stationary reference frame based on spatialLocator.
        Windows::Perception::Spatial::SpatialStationaryFrameOfReference^ m_stationaryReferenceFrame;

        // Event registration tokens.
        Windows::Foundation::EventRegistrationToken                     m_cameraAddedToken;
        Windows::Foundation::EventRegistrationToken                     m_cameraRemovedToken;
        Windows::Foundation::EventRegistrationToken                     m_locatabilityChangedToken;
        Windows::Foundation::EventRegistrationToken                     m_gamepadAddedEventToken;
        Windows::Foundation::EventRegistrationToken                     m_gamepadRemovedEventToken;
        Windows::Foundation::EventRegistrationToken                     m_holographicDisplayIsAvailableChangedEventToken;

        // Keep track of gamepads.
        struct GamepadWithButtonState
        {
            GamepadWithButtonState(
                Windows::Gaming::Input::Gamepad^ gamepad,
                bool buttonAWasPressedLastFrame)
            {
                this->gamepad = gamepad;
                this->buttonAWasPressedLastFrame = buttonAWasPressedLastFrame;
            }
            Windows::Gaming::Input::Gamepad^ gamepad;
            bool buttonAWasPressedLastFrame = false;
        };
        std::vector<GamepadWithButtonState>                             m_gamepads;

        // Keep track of mouse input.
        bool                                                            m_pointerPressed = false;
        Speech::SpeechInput^                                             m_speechInput;
        bool                                                             m_speechInitialized;
        bool                                                             m_activated;
        bool                                                             m_micInitialized;
        bool                                                             m_hasMicPermission;
    };
}
