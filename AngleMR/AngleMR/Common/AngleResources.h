
#pragma once

#include <map>

namespace DX
{
    class DeviceResources;

    namespace ANGLE
    {
        class StereoTexture
        {
        public:
            StereoTexture();
            ~StereoTexture();
            void UpdateTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, unsigned int index);
            EGLSurface GetSurface(unsigned int index);

        private:
            EGLSurface m_surfaces[2];
            Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textures[2];
            HANDLE m_sharedHandles[2];
            float m_width;
            float m_height;
        };

        // Creates and manages a Direct3D device and immediate context, Direct2D device and context (for debug), and the holographic swap chain.
        class AngleResources
        {
        public:
            AngleResources(DX::DeviceResources *pDeviceResources, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr, float width = 0.0f, float height = 0.0f);

            void AddHolographicBackBuffer(DX::DeviceResources* pDeviceResources, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, float width, float height);
            void MakeCurrent(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

            void RemoveTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

            // Public methods related to Direct3D devices.
            void HandleDeviceLost();
            void Present();
            void InitializeEGL(DX::DeviceResources *pDeviceResources, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, float width, float height);
            void CleanupEGL();
        private:
            void CreateDeviceDependentResources();
            ID3D11Texture2D * ResolveTexture(DX::DeviceResources *pDeviceResources, ID3D11Texture2D *source, unsigned int subresource);

            std::map<Microsoft::WRL::ComPtr<ID3D11Texture2D>, EGLSurface> m_surfaces;
            std::mutex  m_mutex;

            EGLDisplay mEglDisplay;
            EGLContext mEglContext;
            EGLSurface mEglSurface;
            EGLConfig mEGLConfig;

            Microsoft::WRL::ComPtr<ID3D11Texture2D> m_defaultSharedTexture;
            float m_defaultWidth;
            float m_defaultHeight;
        };
    }
}



