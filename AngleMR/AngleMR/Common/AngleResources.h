
#pragma once


#include "DeviceResources.h"

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

        void CreateSurfaces(float width, float height);
        EGLSurface CreateSurface(float width, float height, EyeIndex eye);
        void DestroySurfaces();

        EGLDisplay mEglDisplay;
        EGLContext mEglContext;
        EGLSurface mLeftSurface;
        EGLSurface mRightSurface;
        EGLConfig mEGLConfig;

        std::shared_ptr<DX::DeviceResources> m_deviceResources;


        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_leftTexture;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_rightTexture;

        float m_width;
        float m_height;
        float m_textureWidth;
        float m_textureHeight;
    };
}




