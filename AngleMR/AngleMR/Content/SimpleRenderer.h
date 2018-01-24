#pragma once

#include "pch.h"
#include "..\Common\StepTimer.h"

namespace AngleMR
{
    enum EVREye
    {
        Eye_Left = 0,
        Eye_Right = 1
    };

    class SimpleRenderer
    {
    public:
        SimpleRenderer();
        ~SimpleRenderer();
        void Render(EVREye eye);

        void Draw();
        void UpdateWindowSize(GLsizei width, GLsizei height);

        // Repositions the sample hologram.
        void PositionHologram(Windows::UI::Input::Spatial::SpatialPointerPose^ pointerPose) {};
        void Update(const DX::StepTimer& timer) {};

        void SetPosition(Windows::Foundation::Numerics::float3 pos) { m_position = pos; }
        Windows::Foundation::Numerics::float3 GetPosition() { return m_position; }

        void UpdateProjections(const DirectX::XMFLOAT4X4* proj);

        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();

    private:
        GLuint mProgram;
        GLsizei mWindowWidth;
        GLsizei mWindowHeight;

        GLint mPositionAttribLocation;
        GLint mColorAttribLocation;
        GLint mHolographicViewProjectionMatrix;

        GLint mModelUniformLocation;
        GLint mViewUniformLocation;
        GLint mProjUniformLocation;

        GLuint mVertexPositionBuffer;
        GLuint mVertexColorBuffer;
        GLuint mIndexBuffer;

        int mDrawCount;
        Windows::Foundation::Numerics::float3           m_position = { 0.f, 0.f, -2.f };
    };
}