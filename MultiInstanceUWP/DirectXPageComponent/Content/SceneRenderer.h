//--------------------------------------------------------------------------------------
// File: SceneRenderer.h
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

#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

namespace DirectXPageComponent
{
    // This class renders a scene using DirectXTK
    class SceneRenderer
    {
    public:
        SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void Render();


    private:
        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

        std::unique_ptr<DirectX::CommonStates>                                  m_states;
        std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
        std::unique_ptr<DirectX::IEffectFactory>                                m_fxFactory;
        std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
        std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

        bool                                                                    m_retryDefault;

        DirectX::SimpleMath::Matrix                                             m_world;
        DirectX::SimpleMath::Matrix                                             m_view;
        DirectX::SimpleMath::Matrix                                             m_projection;

        float                                                                   m_textureWidth;
        float                                                                   m_textureHeight;

    };
}

