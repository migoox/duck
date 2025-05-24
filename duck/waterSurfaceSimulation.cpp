#include "pch.h"

#include "utils/profiling.h"
#include "waterSurfaceSimulation.h"
#include <dxDevice.h>
#include <iostream>

mini::gk2::WaterSurfaceSimulation::WaterSurfaceSimulation()
    : Simulation(), m_currentHeightBuffer(0), m_samplesCount(SAMPLES_DEFAULT_SIZE), m_velocity(DEFAULT_VELOCITY),
      m_randGenerator(std::random_device{}()), m_uniformDist(0, SAMPLES_DEFAULT_SIZE - 1)
{
    SetStepTime(1.f / static_cast<float>(SAMPLES_DEFAULT_SIZE));
    SetSimSpeed(ANIMATION_SPEED);

    GetCurrentHeightBuffer().resize(m_samplesCount * m_samplesCount, 0.f);
    GetNextHeightBuffer().resize(m_samplesCount * m_samplesCount, 0.f);
    m_normalMap.resize(m_samplesCount * m_samplesCount * 4, 0);
    m_distances.resize(m_samplesCount * m_samplesCount, 0.f);
    InitNormalMap();
    InitDistances();
}

void mini::gk2::WaterSurfaceSimulation::MapToSurfaceTexture(DxDevice& device, dx_ptr<ID3D11Texture2D>& texture)
{
    PROFILE_ZONE("WaterSurfaceSimulation::MapToSurfaceTexture");
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = device.context()->Map(texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        const UINT rowSize = m_samplesCount * 4;

        BYTE* dest      = reinterpret_cast<BYTE*>(mapped.pData);
        const BYTE* src = reinterpret_cast<const BYTE*>(m_normalMap.data());
        for (UINT y = 0; y < m_samplesCount; ++y)
        {
            memcpy(dest, src, rowSize);
            dest += mapped.RowPitch;
            src += rowSize;
        }

        device.context()->Unmap(texture.get(), 0);
    }
}

void mini::gk2::WaterSurfaceSimulation::Step()
{
    PROFILE_ZONE("WaterSurfaceSimulation::Step");
    auto stepTime = StepTime();

    auto& curr = GetCurrentHeightBuffer();
    auto& next = GetNextHeightBuffer();

    auto h = 2.f / static_cast<float>(m_samplesCount - 1);
    auto a = m_velocity * m_velocity * stepTime * stepTime / h / h;
    auto b = 2.f - 4.f * a;

    // Edges are unchanged (Miguel Gomez, Game Programming Gems 1)
    for (auto i = 1; i < m_samplesCount - 1; i++)
    {
        for (auto j = 1; j < m_samplesCount - 1; j++)
        {
            auto aa = a * (GetValue(curr, i + 1, j) + GetValue(curr, i - 1, j) + GetValue(curr, i, j - 1) +
                           GetValue(curr, i, j + 1));
            auto bb = b * GetValue(curr, i, j) - GetValue(next, i, j);
            SetValue(next, i, j, GetValue(m_distances, i, j) * (aa + bb));
        }
    }

    SwapHeightBuffers();

    // Add new drops
    if (m_uniformDist(m_randGenerator) >
        m_samplesCount - static_cast<int>(static_cast<float>(m_samplesCount) * DROP_PROBABILITY))
    {
        auto i = m_uniformDist(m_randGenerator);
        auto j = m_uniformDist(m_randGenerator);
        SetValue(next, i, j, GetValue(next, i, j) + DROP_HEIGHT);
    }
}

void mini::gk2::WaterSurfaceSimulation::PostUpdate()
{
    UpdateNormalMap();
}

void mini::gk2::WaterSurfaceSimulation::InitNormalMap()
{
    PROFILE_ZONE("WaterSurfaceSimulation::InitNormalMap")
    auto& curr = GetCurrentHeightBuffer();
    for (auto i = 0; i < m_samplesCount; i++)
    {
        for (auto j = 0; j < m_samplesCount; j++)
        {
            auto normIdx             = (i * m_samplesCount + j) * 4;
            m_normalMap[normIdx]     = 0;
            m_normalMap[normIdx + 1] = 255;
            m_normalMap[normIdx + 2] = 0;
            m_normalMap[normIdx + 3] = 255; // alpha
        }
    }
}

void mini::gk2::WaterSurfaceSimulation::UpdateNormalMap()
{
    PROFILE_ZONE("WaterSurfaceSimulation::UpdateNormalMap")
    auto& curr = GetCurrentHeightBuffer();
    // Blinn method (https://en.wikipedia.org/wiki/Bump_mapping#Methods)
    for (auto i = 1; i < m_samplesCount - 1; i++)
    {
        for (auto j = 1; j < m_samplesCount - 1; j++)
        {

            // 1. Gradient approx with finite difference (https://en.wikipedia.org/wiki/Finite_difference) for each
            // partial derivative
            auto dx = curr[m_samplesCount * i + (j - 1)] - curr[m_samplesCount * i + (j + 1)];
            auto dy = 1.f;
            auto dz = curr[m_samplesCount * (i - 1) + j] - curr[m_samplesCount * (i + 1) + j];

            // 2. Normalize
            const auto denom = std::sqrt(dx * dx + dy * dy + dz * dz);
            dx               = dx / denom;
            dy               = dy / denom;
            dz               = dz / denom;

            // 3. [-1, -1] -> [0, 1]
            dx = (dx + 1.f) / 2.f;
            dy = (dy + 1.f) / 2.f;
            dz = (dz + 1.f) / 2.f;

            const auto normIdx = 4 * (i * m_samplesCount + j);

            m_normalMap[normIdx]     = static_cast<BYTE>(dx * 255.f);
            m_normalMap[normIdx + 1] = static_cast<BYTE>(dy * 255.f);
            m_normalMap[normIdx + 2] = static_cast<BYTE>(dz * 255.f);
            m_normalMap[normIdx + 3] = 255; // alpha
        }
    }
}

void mini::gk2::WaterSurfaceSimulation::InitDistances()
{
    PROFILE_ZONE("WaterSurfaceSimulation::InitChebyshevDistance")
    for (auto i = 0; i < m_samplesCount; i++)
    {
        for (auto j = 0; j < m_samplesCount; j++)
        {
            auto wallDist = std::min<int>(std::min<int>(i, j), std::min<int>(m_samplesCount - i, m_samplesCount - j));
            auto l        = 2.f * static_cast<float>(wallDist) / static_cast<float>(m_samplesCount);
            m_distances[i * m_samplesCount + j] = 0.95f * std::min<float>(1.f, l / 0.2f);
        }
    }
}
