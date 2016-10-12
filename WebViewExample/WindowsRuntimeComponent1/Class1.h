#pragma once

namespace WindowsRuntimeComponent1
{
    ref class Class1;
    public delegate void JavaScriptCallbackHandler(Class1^ sender, Platform::String^ s);

    [Windows::Foundation::Metadata::AllowForWeb]
    public ref class Class1 sealed
    {
    public:
        Class1();
        void fromJavaScript(Platform::String^ args);
        // Event whose type is a delegate "class"
        event JavaScriptCallbackHandler^ OnJavaScriptCallback;

    private:
        Windows::UI::Core::CoreDispatcher^ m_dispatcher;

    };
}
