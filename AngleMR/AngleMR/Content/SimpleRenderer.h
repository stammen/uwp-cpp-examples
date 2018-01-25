#pragma once

#include "pch.h"
#include "..\Common\StepTimer.h"
#include "..\Common\AngleResources.h"

namespace AngleMR
{

    class SimpleRenderer
    {
    public:
        SimpleRenderer();
        ~SimpleRenderer();
        void Render(ANGLE::EyeIndex eye);

        void Draw(ANGLE::EyeIndex eye);
        void UpdateWindowSize(GLsizei width, GLsizei height);

        // Repositions the sample hologram.
        void PositionHologram(Windows::UI::Input::Spatial::SpatialPointerPose^ pointerPose);
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
        GLint mRtvIndexUniformLocation;

        GLuint mVertexPositionBuffer;
        GLuint mVertexColorBuffer;
        GLuint mIndexBuffer;

        int mDrawCount;
        Windows::Foundation::Numerics::float3           m_position = { 0.f, 0.f, -2.f };
        float                                           m_degreesPerSecond = 45.f;
    };
}