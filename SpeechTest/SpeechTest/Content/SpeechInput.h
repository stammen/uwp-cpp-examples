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

#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <pplcancellation_token.h>
#include <memory>

namespace Speech
{
    interface IMRAppServiceListenerDelegate
    {
        virtual void OnSpeechQualityDegraded(Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognitionQualityDegradingEventArgs^ args) = 0;
        virtual void OnSpeechResultGenerated(Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession ^sender, Windows::Media::SpeechRecognition::SpeechContinuousRecognitionResultGeneratedEventArgs ^args) = 0;
		virtual void OnRecognizerStateChanged(Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognizerStateChangedEventArgs^ args) = 0;
		virtual void OnSpeechRecognizerError(Platform::String^ error) = 0;
	};


    ref class SpeechInput sealed
    {
    public:
        SpeechInput();
        virtual ~SpeechInput();

    internal:
		void Start(IMRAppServiceListenerDelegate* delegate, Platform::Collections::Vector<Platform::String^>^ keys);
		static Concurrency::task<bool> Available();

    private:
		Concurrency::task<bool> Initialize(Platform::Collections::Vector<Platform::String^>^ keys);
		Concurrency::task<void> StartSpeechRecognition();
		void Stop();
		void StopSpeechRecognition();

		void SetDelegate(IMRAppServiceListenerDelegate* delegate);

        static const unsigned int HResultPrivacyStatementDeclined = 0x80045509;
        static const unsigned int HResultRecognizerNotFound = 0x8004503a;

        Windows::Media::SpeechRecognition::SpeechRecognizer^ m_speechRecognizer;
        Windows::ApplicationModel::Resources::Core::ResourceContext^ m_speechContext;
        Windows::ApplicationModel::Resources::Core::ResourceMap^ m_speechResourceMap;
        bool isPopulatingLanguages = false;


        Concurrency::task<bool> InitializeRecognizer(Platform::Collections::Vector<Platform::String^>^ keys, Windows::Globalization::Language^ recognizerLanguage);

        Windows::Foundation::EventRegistrationToken m_stateChangedToken;
        Windows::Foundation::EventRegistrationToken m_resultEventToken;
        Windows::Foundation::EventRegistrationToken m_qualityDegradedToken;
        Windows::Foundation::EventRegistrationToken m_completedToken;

        void SpeechRecognizer_StateChanged(Windows::Media::SpeechRecognition::SpeechRecognizer ^sender, Windows::Media::SpeechRecognition::SpeechRecognizerStateChangedEventArgs ^args);
        void ContinuousRecognitionSession_Completed(Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession ^sender, Windows::Media::SpeechRecognition::SpeechContinuousRecognitionCompletedEventArgs ^args);
        void ContinuousRecognitionSession_ResultGenerated(Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession ^sender, Windows::Media::SpeechRecognition::SpeechContinuousRecognitionResultGeneratedEventArgs ^args);
        void OnSpeechQualityDegraded(Windows::Media::SpeechRecognition::SpeechRecognizer^ recognizer, Windows::Media::SpeechRecognition::SpeechRecognitionQualityDegradingEventArgs^ args);

        IMRAppServiceListenerDelegate* m_delegate;

		std::shared_ptr<Concurrency::cancellation_token_source> m_cancellationToken;
		Platform::String^ m_errorMessage;

    };
}
