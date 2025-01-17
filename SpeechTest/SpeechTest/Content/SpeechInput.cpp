//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "SpeechInput.h"
#include "AudioCapturePermissions.h"


using namespace Speech;
using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel::Resources::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::SpeechRecognition;

SpeechInput::SpeechInput() 
    : m_speechRecognizer(nullptr)
{

}

SpeechInput::~SpeechInput()
{
    Stop();
}

void SpeechInput::SetDelegate(IMRAppServiceListenerDelegate* delegate)
{
    m_delegate = delegate;
}

Concurrency::task<bool> SpeechInput::Available()
{
    // Prompt the user for permission to access the microphone. This request will only happen
    // once, it will not re-prompt if the user rejects the permission.
	return create_task(AudioCapturePermissions::RequestMicrophonePermissionAsync());
}

void SpeechInput::StopSpeechRecognition()
{
	auto speechRecognizer = m_speechRecognizer;
	m_speechRecognizer = nullptr;
	if (speechRecognizer != nullptr)
	{
		speechRecognizer->StateChanged -= m_stateChangedToken;
		speechRecognizer->ContinuousRecognitionSession->Completed -= m_completedToken;
		speechRecognizer->ContinuousRecognitionSession->ResultGenerated -= m_resultEventToken;
		speechRecognizer->RecognitionQualityDegrading -= m_qualityDegradedToken;

		// If we're currently active, start a cancellation task, and wait for it to finish before shutting down
		// the recognizer.
		Concurrency::task<void> cleanupTask;
		if (speechRecognizer->State != SpeechRecognizerState::Idle)
		{
			cleanupTask = create_task(speechRecognizer->ContinuousRecognitionSession->CancelAsync());
		}
		else
		{
			cleanupTask = task_from_result();
		}

		cleanupTask.then([speechRecognizer]()
		{
			if (speechRecognizer)
			{
				delete speechRecognizer;
			}
		});
	}
}

/// <summary>
/// Upon leaving, clean up the speech recognizer. Ensure we aren't still listening, and disable the event 
/// handlers to prevent leaks.
/// </summary>
/// <param name="e">Unused navigation parameters.</param>
void SpeechInput::Stop()
{
	auto cancelation = m_cancellationToken.get();
	if (cancelation != nullptr)
	{
		cancelation->cancel();
	}
	else
	{
		StopSpeechRecognition();
	}
}

void SpeechInput::Start(IMRAppServiceListenerDelegate* delegate, Platform::Collections::Vector<Platform::String^>^ keys)
{
	m_delegate = delegate;

	if (m_cancellationToken.get() != nullptr || m_speechRecognizer != nullptr)
	{
		throw ref new Platform::Exception(-1, L"Reentrant call to Start()");
	}

	m_cancellationToken = std::make_shared<Concurrency::cancellation_token_source>(cancellation_token_source());
	m_errorMessage = L"";

	auto startTask = create_task(create_task(AudioCapturePermissions::RequestMicrophonePermissionAsync()));
	startTask.then([this, keys](bool hasMicPermissions)
	{
		if (hasMicPermissions && !m_cancellationToken.get()->get_token().is_canceled())
		{
			return Initialize(keys);
		}
		else
		{
			cancel_current_task();
		}
	}).then([this](bool success)
	{
		if (success && !m_cancellationToken.get()->get_token().is_canceled())
		{
			return StartSpeechRecognition();
		}
		else
		{
			cancel_current_task();
		}
	}).then([this](task<void> t)
	{
		try
		{
			t.get();

			m_stateChangedToken = m_speechRecognizer->StateChanged +=
				ref new TypedEventHandler<
				SpeechRecognizer ^,
				SpeechRecognizerStateChangedEventArgs ^>(
					this,
					&SpeechInput::SpeechRecognizer_StateChanged);

			m_qualityDegradedToken = m_speechRecognizer->RecognitionQualityDegrading +=
				ref new TypedEventHandler<
				SpeechRecognizer ^,
				SpeechRecognitionQualityDegradingEventArgs ^>(
					this,
					&SpeechInput::OnSpeechQualityDegraded);

			// Handle continuous recognition events. Completed fires when various error states occur. ResultGenerated fires when
			// some recognized phrases occur, or the garbage rule is hit.
			m_completedToken = m_speechRecognizer->ContinuousRecognitionSession->Completed +=
				ref new TypedEventHandler<
				SpeechContinuousRecognitionSession ^,
				SpeechContinuousRecognitionCompletedEventArgs ^>(
					this,
					&SpeechInput::ContinuousRecognitionSession_Completed);

			m_resultEventToken = m_speechRecognizer->ContinuousRecognitionSession->ResultGenerated +=
				ref new TypedEventHandler<
				SpeechContinuousRecognitionSession ^,
				SpeechContinuousRecognitionResultGeneratedEventArgs ^>(
					this,
					&SpeechInput::ContinuousRecognitionSession_ResultGenerated);

		}
		catch (Platform::Exception^ ex)
		{
			m_errorMessage = ex->Message;
			StopSpeechRecognition();
			m_delegate->OnSpeechRecognizerError(m_errorMessage);
		}
		catch (task_canceled)
		{
			StopSpeechRecognition();
			m_errorMessage = L"Start cancelled";
			m_delegate->OnSpeechRecognizerError(m_errorMessage);
		}

		m_cancellationToken = nullptr;

	});
}

Concurrency::task<void> SpeechInput::StartSpeechRecognition()
{
    // The recognizer can only start listening in a continuous fashion if the recognizer is currently idle.
    // This prevents an exception from occurring.
    if (m_speechRecognizer->State == SpeechRecognizerState::Idle)
    {
		return create_task(m_speechRecognizer->ContinuousRecognitionSession->StartAsync());
    }
	else
	{
		// Cancelling recognition prevents any currently recognized speech from
		// generating a ResultGenerated event. StopAsync() will allow the final session to 
		// complete.
		return create_task(m_speechRecognizer->ContinuousRecognitionSession->CancelAsync());
	}
}

Concurrency::task<bool> SpeechInput::Initialize(Platform::Collections::Vector<Platform::String^>^ keys)
{
    // Note: Language not fully implemented and code only tested for English
    Windows::Globalization::Language^ speechLanguage = SpeechRecognizer::SystemSpeechLanguage;
    m_speechContext = ResourceContext::GetForCurrentView();
    m_speechContext->Languages = ref new VectorView<String^>(1, speechLanguage->LanguageTag);
    m_speechResourceMap = ResourceManager::Current->MainResourceMap->GetSubtree(L"LocalizationSpeechResources");
    return InitializeRecognizer(keys, SpeechRecognizer::SystemSpeechLanguage);
}

/// <summary>
/// Creates a SpeechRecognizer instance and initializes the grammar.
/// </summary>
Concurrency::task<bool> SpeechInput::InitializeRecognizer(Platform::Collections::Vector<Platform::String^>^ keys, Windows::Globalization::Language^ recognizerLanguage)
{
    // If reinitializing the recognizer (ie, changing the speech language), clean up the old recognizer first.
    // Avoid doing this while the recognizer is active by disabling the ability to change languages while listening.
    if (m_speechRecognizer != nullptr)
    {
        m_speechRecognizer->StateChanged -= m_stateChangedToken;
        m_speechRecognizer->RecognitionQualityDegrading -= m_qualityDegradedToken;
        m_speechRecognizer->ContinuousRecognitionSession->Completed -= m_completedToken;
        m_speechRecognizer->ContinuousRecognitionSession->ResultGenerated -= m_resultEventToken;
        delete m_speechRecognizer;
        m_speechRecognizer = nullptr;
    }

    return create_task([this, keys, recognizerLanguage]() 
    {
        m_speechRecognizer = ref new SpeechRecognizer(recognizerLanguage);



        SpeechRecognitionListConstraint^ spConstraint = ref new SpeechRecognitionListConstraint(keys);
        m_speechRecognizer->Constraints->Clear();
        m_speechRecognizer->Constraints->Append(spConstraint);

        return create_task(m_speechRecognizer->CompileConstraintsAsync(), task_continuation_context::use_current())
            .then([this](task<SpeechRecognitionCompilationResult^> previousTask)
        {
            SpeechRecognitionCompilationResult^ compilationResult = previousTask.get();

            if (compilationResult->Status != SpeechRecognitionResultStatus::Success)
            {

            }
            else
            {

            }

            return(compilationResult->Status == SpeechRecognitionResultStatus::Success);
        }, task_continuation_context::use_current());
    });
}

/// <summary>
/// Provide feedback to the user based on whether the recognizer is receiving their voice input.
/// </summary>
/// <param name="sender">The recognizer that is currently running.</param>
/// <param name="args">The current state of the recognizer.</param>
void SpeechInput::SpeechRecognizer_StateChanged(SpeechRecognizer ^sender, SpeechRecognizerStateChangedEventArgs ^args)
{
    if (m_delegate)
    {
        m_delegate->OnRecognizerStateChanged(sender, args);
    }
}

/// <summary>
/// Handle events fired when error conditions occur, such as the microphone becoming unavailable, or if
/// some transient issues occur.
/// </summary>
/// <param name="sender">The continuous recognition session</param>
/// <param name="args">The state of the recognizer</param>
void SpeechInput::ContinuousRecognitionSession_Completed(SpeechContinuousRecognitionSession ^sender, SpeechContinuousRecognitionCompletedEventArgs ^args)
{
    if (m_delegate)
    {
        //m_delegate->On(sender, args);
    }
}

/// <summary>
/// Handle events fired when a result is generated. This may include a garbage rule that fires when general room noise
/// or side-talk is captured (this will have a confidence of Rejected typically, but may occasionally match a rule with
/// low confidence).
/// </summary>
/// <param name="sender">The Recognition session that generated this result</param>
/// <param name="args">Details about the recognized speech</param>
void SpeechInput::ContinuousRecognitionSession_ResultGenerated(SpeechContinuousRecognitionSession ^sender, SpeechContinuousRecognitionResultGeneratedEventArgs ^args)
{
    if (m_delegate)
    {
        m_delegate->OnSpeechResultGenerated(sender, args);
    }
}

void SpeechInput::OnSpeechQualityDegraded(Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognitionQualityDegradingEventArgs^ args)
{
    if (m_delegate)
    {
        m_delegate->OnSpeechQualityDegraded(recognizer, args);
    }
}


