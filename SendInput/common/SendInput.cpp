#include "SendInput.h"
#include <Windows.h>



#ifdef MS_UWP
#include "../MRAppService/MRAppServiceListener.h"
#include <ppltasks.h>
#include <string>

using namespace Concurrency;
using namespace MRAppService;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

static MRAppServiceListener^ s_appServiceListener = nullptr;

Concurrency::task<AppServiceConnectionStatus> ConnectToAppService(const std::wstring& id);
Concurrency::task<bool> LaunchWin32App();
#endif

extern "C" {

    DLL_API bool Initialize()
    {
#ifdef MS_UWP
        LaunchWin32App();
        ConnectToAppService(L"UWP-App");
#endif
        return true;
    }

#ifdef MS_UWP
    DLL_API bool SendMouseInputUWP(MOUSEINPUT* input)
    {
        if (s_appServiceListener && s_appServiceListener->IsConnected())
        {
            ValueSet^ message = ref new ValueSet;
            message->Insert(L"MOUSEINPUT", true);
            message->Insert(L"dx", static_cast<int>(input->dx));
            message->Insert(L"dy", static_cast<int>(input->dy));
            message->Insert(L"mouseData", static_cast<unsigned int>(input->mouseData));
            message->Insert(L"dwFlags", static_cast<unsigned int>(input->dwFlags));
            message->Insert(L"time", static_cast<unsigned int>(input->time));
            s_appServiceListener->SendAppServiceMessage(L"Win32-App", message);
            return true;
        }

        return false;
    }

    DLL_API bool SendKeyboardInputUWP(KEYBDINPUT* input)
    {
        if (s_appServiceListener && s_appServiceListener->IsConnected())
        {
            ValueSet^ message = ref new ValueSet;
            message->Insert(L"KEYBDINPUT", true);
            message->Insert(L"wVk", static_cast<int>(input->wVk));
            message->Insert(L"wScan", static_cast<int>(input->wScan));
            message->Insert(L"dwFlags", static_cast<unsigned int>(input->dwFlags));
            message->Insert(L"time", static_cast<unsigned int>(input->time));
            return true;
        }
        return false;
    }
#else
    DLL_API bool SendInputWin32(INPUT* input, int numInputs)
    {
        auto count = SendInput(numInputs, input, sizeof(input));
        return count = numInputs;
    }
#endif
}

#ifdef MS_UWP
Concurrency::task<AppServiceConnectionStatus> ConnectToAppService(const std::wstring& id)
{
    Platform::String^ listenId = ref new Platform::String(id.c_str());
    s_appServiceListener = ref new MRAppServiceListener(listenId);
    return s_appServiceListener->ConnectToAppService(MRAPPSERVICE_ID, MRAppService::MRAppServiceListener::GetPackageFamilyName());
}

Concurrency::task<bool> LaunchWin32App()
{
    // Launch the Win32 App that will support SendInput for UWP apps
    auto uri = ref new Uri("sendinput-win32:"); // The protocol handled by the launched app
    auto options = ref new LauncherOptions();
    return create_task(Launcher::LaunchUriAsync(uri, options));
}

#endif


