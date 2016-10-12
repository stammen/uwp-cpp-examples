#pragma once

namespace WindowsRuntimeComponent1
{
    [Windows::Foundation::Metadata::AllowForWeb]
    public ref class Class1 sealed
    {
    public:
        Class1();
        void fromJavaScript(Platform::String^ args);

    private:
        Windows::UI::Core::CoreDispatcher^ m_dispatcher;
    };
}
