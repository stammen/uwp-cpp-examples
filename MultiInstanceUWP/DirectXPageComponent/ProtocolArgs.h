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
        Platform::String^ GetStringParameter(Platform::String^ key, Platform::String^ defaultValue);
        int GetIntParameter(Platform::String^ key, int defaultValue);
        double GetDoubleParameter(Platform::String^ key, double defaultValue);

    private:
        std::map<Platform::String^, Platform::String^> m_args;

    };
}

