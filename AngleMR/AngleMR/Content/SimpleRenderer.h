#pragma once

#include "pch.h"

#include "..\Common\StepTimer.h"

namespace AngleMR
{
    class SimpleRenderer
    {
    public:
        SimpleRenderer();
        ~SimpleRenderer();
        void Render();
        void Update(const DX::StepTimer& timer);

        void UpdateWindowSize(GLsizei width, GLsizei height);
        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();

        // Repositions the sample hologram.
        void PositionHologram(Windows::UI::Input::Spatial::SpatialPointerPose^ pointerPose);

        // Property accessors.
        void SetPosition(Windows::Foundation::Numerics::float3 pos) { m_position = pos; }
        Windows::Foundation::Numerics::float3 GetPosition() { return m_position; }

    private:
        Windows::Foundation::Numerics::float3           m_position = { 0.f, 0.f, -2.f };

        GLuint mProgram;
        GLsizei mWindowWidth;
        GLsizei mWindowHeight;

        GLint mPositionAttribLocation;
        GLint mColorAttribLocation;

        GLint mModelUniformLocation;
        GLint mViewUniformLocation;
        GLint mProjUniformLocation;

        GLuint mVertexPositionBuffer;
        GLuint mVertexColorBuffer;
        GLuint mIndexBuffer;

        int mDrawCount;
    };
}