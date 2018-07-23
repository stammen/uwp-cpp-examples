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
    m_textureWidth = 512;
    m_textureHeight = 512;
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
    m_sprites->Draw(m_textureShaderResourceView.Get(), XMFLOAT2(x, y), nullptr, Colors::White);
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

#if 0
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"assets\\Square150x150Logo.scale-200.png", res.GetAddressOf(), m_textureShaderResourceView.ReleaseAndGetAddressOf())
        );
#endif

    CreateSharedTexture();

}

void SceneRenderer::ReleaseDeviceDependentResources()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();    
    m_textureShaderResourceView.Reset();
    m_texture.Reset();
    m_batchInputLayout.Reset();
}

void SceneRenderer::CreateSharedTexture()
{
    m_texture.Reset();
    m_textureShaderResourceView.Reset();

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = static_cast<UINT>(m_textureWidth);
    desc.Height = static_cast<UINT>(m_textureHeight);
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
    ID3D11Texture2D *pTexture = NULL;
    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &pTexture)
    );

    m_texture = pTexture;

    // QI IDXGIResource interface to synchronized shared surface.
    m_sharedTextureHandle = NULL;
    IDXGIResource1* pDXGIResource1 = NULL;
    DX::ThrowIfFailed(
        m_texture->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource1)
    );

    // obtain handle to IDXGIResource object.
    DX::ThrowIfFailed(
        pDXGIResource1->CreateSharedHandle(NULL,
            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
            L"DirectXPageSharedTexture",
            &m_sharedTextureHandle)
    );

    pDXGIResource1->Release();

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = desc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = 1;

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateShaderResourceView(m_texture.Get(), &SRVDesc, m_textureShaderResourceView.GetAddressOf())
    );
}