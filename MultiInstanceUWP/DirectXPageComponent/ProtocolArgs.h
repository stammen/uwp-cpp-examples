#pragma once

#include <map>
#include <string>

#define APP_PROTOCOL L"stammen-multi-instance-uwp"

namespace DirectXPageComponent
{
    public ref class ProtocolArgs sealed
    {
    public:
        ProtocolArgs(Windows::Foundation::WwwFormUrlDecoder^ args);
        Platform::String^ GetParameter(Platform::String^ key, Platform::String^ defaultValue);

    private:
        std::map<Platform::String^, Platform::String^> m_args;

    };
}

