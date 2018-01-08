
#pragma once

#include <map>

namespace ANGLE
{


    // Creates and manages a Direct3D device and immediate context, Direct2D device and context (for debug), and the holographic swap chain.
    class AngleResources
    {
    public:
        AngleResources(HANDLE sharedHandle = NULL, float width = 0.0f, float height = 0.0f);

        void AddSharedTexture(HANDLE sharedHandle, float width, float height);
        void MakeCurrect(HANDLE sharedHandle);

        void RemoveSharedTexture(HANDLE sharedHandle);

        // Public methods related to Direct3D devices.
        void HandleDeviceLost();
        void Present();
        void InitializeEGL(HANDLE sharedHandle, float width, float height);
        void CleanupEGL();
    private:


        std::map<HANDLE, EGLSurface> m_elgSurfaces;
        std::mutex  m_mutex;

        EGLDisplay mEglDisplay;
        EGLContext mEglContext;
        EGLSurface mEglSurface;
        EGLConfig mEGLConfig;

        HANDLE m_defaultSharedTexture;
        float m_defaultWidth;
        float m_defaultHeight;
    };
}



