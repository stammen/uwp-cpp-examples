#include "pch.h"
#include "ProtocolArgs.h"

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

Platform::String^ ProtocolArgs::GetParameter(Platform::String^ key, Platform::String^ defaultValue)

{
    auto it = m_args.find(key);
    if (it != m_args.end())
    {
        return it->second;
    }

    return defaultValue;
}

