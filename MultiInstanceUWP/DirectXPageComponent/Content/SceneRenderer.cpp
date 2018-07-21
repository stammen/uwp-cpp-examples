//--------------------------------------------------------------------------------------
// File: SceneRenderer.cpp
//
// This is a simple universal Windows app showing use of DirectXTK
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"

#include "SceneRenderer.h"
#include <WICTextureLoader.h>

#include "..\Common\DirectXHelper.h"	// For ThrowIfFailed and ReadDataAsync

using namespace DirectXPageComponent;

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;

SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources)
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
    Size outputSize = m_deviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // Note that the OrientationTransform3D matrix is post-multiplied here
    // in order to correctly orient the scene to match the display orientation.
    // This post-multiplication step is required for any draw calls that are
    // made to the swap chain render target. For draw calls to other targets,
    // this transform should not be applied.

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    Matrix perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
        );

    Matrix orientationMatrix = m_deviceResources->GetOrientationTransform3D();
    m_projection = perspectiveMatrix * orientationMatrix;
    m_batchEffect->SetProjection(m_projection);
    m_sprites->SetRotation( m_deviceResources->ComputeDisplayRotation() );
}



void SceneRenderer::Update(DX::StepTimer const& timer)
{
    Vector3 eye(0.0f, 0.7f, 1.5f);
    Vector3 at(0.0f, -0.1f, 0.0f);
    m_view = Matrix::CreateLookAt(eye, at, Vector3::UnitY);
    m_world = Matrix::CreateRotationY( float(timer.GetTotalSeconds() * XM_PIDIV4) );
    m_batchEffect->SetView(m_view);
    m_batchEffect->SetWorld(Matrix::Identity);
}


void SceneRenderer::Render()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Set render targets to the screen.
    ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
    Size outputSize = m_deviceResources->GetOutputSize();

    float x = (outputSize.Width / 2.0f) - (m_textureWidth / 2.0f);
    float y = (outputSize.Height / 2.0f) - (m_textureHeight / 2.0f);
        
    // Draw sprite
    m_sprites->Begin();
    m_sprites->Draw(m_texture.Get(), XMFLOAT2(x, y), nullptr, Colors::White);
    m_sprites->End();
}

void SceneRenderer::CreateDeviceDependentResources()
{
    // Create DirectXTK objects
    auto device = m_deviceResources->GetD3DDevice();
    m_states = std::make_unique<CommonStates>(device);

    auto fx = new EffectFactory( device );
    fx->SetDirectory( L"Assets" );
    m_fxFactory.reset( fx );

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
            VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_batchInputLayout.ReleaseAndGetAddressOf())
            );
    }

    ComPtr<ID3D11Resource> res;

    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"assets\\Square150x150Logo.scale-200.png", res.GetAddressOf(), m_texture.ReleaseAndGetAddressOf())
        );

    ComPtr<ID3D11Texture2D> tex;
    DX::ThrowIfFailed(res.As(&tex));
    D3D11_TEXTURE2D_DESC desc;
    tex->GetDesc(&desc);
    m_textureWidth = (float)desc.Width;
    m_textureHeight = (float)desc.Height;
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_texture.Reset();
    m_batchInputLayout.Reset();
}