#include "pch.h"
#include "ProtocolArgs.h"
#include <stdlib.h>

using namespace DirectXPageComponent;
using namespace Windows::Foundation;

ProtocolArgs::ProtocolArgs(WwwFormUrlDecoder^ args)
{
    for (unsigned int i = 0; i < args->Size; ++i)
    {
        auto w = args->GetAt(i);
        m_args[w->Name] = w->Value;
    }
}

Platform::String^ ProtocolArgs::GetStringParameter(Platform::String^ key, Platform::String^ defaultValue)
{
    auto it = m_args.find(key);
    if (it != m_args.end())
    {
        return it->second;
    }

    return defaultValue;
}

int ProtocolArgs::GetIntParameter(Platform::String^ key, int defaultValue)
{
    auto it = m_args.find(key);
    if (it != m_args.end())
    {
        return _wtoi(it->second->Data());
    }

    return defaultValue;
}

double ProtocolArgs::GetDoubleParameter(Platform::String^ key, double defaultValue)
{
    auto it = m_args.find(key);
    if (it != m_args.end())
    {
        return _wtof(it->second->Data());
    }

    return defaultValue;
}




