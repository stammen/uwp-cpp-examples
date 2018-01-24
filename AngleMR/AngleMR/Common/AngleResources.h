
#pragma once

namespace DX
{
    class DeviceResources;

    namespace ANGLE
    {
        enum EyeIndex
        {
            Eye_Left = 0,
            Eye_Right = 1
        };

        // Creates and manages a Direct3D device and immediate context, Direct2D device and context (for debug), and the holographic swap chain.
        class AngleResources
        {
        public:
            AngleResources(const std::shared_ptr<DX::DeviceResources>& deviceResources);
            void InitializeEGL(float width, float height);
            void PrepareEye(EyeIndex eye);
            void UpdateWindowSize(float width, float height);
            void Submit(ID3D11DeviceContext* context, ID3D11Texture2D* texture, EyeIndex eye);

            // Public methods related to Direct3D devices.
            void HandleDeviceLost();
            void Present();
            void CleanupEGL();
        private:
            void CreateDeviceDependentResources();
            ID3D11Texture2D * ResolveTexture(ID3D11Texture2D *source, unsigned int subresource);

            EGLSurface CreateSurface(float width, float height);

            EGLDisplay mEglDisplay;
            EGLContext mEglContext;
            EGLSurface mEglSurface;
            EGLConfig mEGLConfig;

            std::shared_ptr<DX::DeviceResources> m_deviceResources;


            Microsoft::WRL::ComPtr<ID3D11Texture2D> m_defaultSharedTexture;
            float m_width;
            float m_height;
            float m_textureWidth;
            float m_textureHeight;
        };
    }
}



