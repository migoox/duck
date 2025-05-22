#include "pch.h"

#include "waterSurfaceSimulation.h"
#include <dxDevice.h>
#include <iostream>

mini::gk2::WaterSurfaceSimulation::WaterSurfaceSimulation()
    : m_currentHeightBuffer(0), m_samplesCount(SAMPLES_DEFAULT_SIZE),
      m_stepTime(1.f / static_cast<float>(SAMPLES_DEFAULT_SIZE)), m_velocity(DEFAULT_VELOCITY),
      m_randGenerator(std::random_device{}()), m_uniformDist(0, SAMPLES_DEFAULT_SIZE)
{
    GetCurrentHeightBuffer().resize(m_samplesCount * m_samplesCount, 0.f);
    GetNextHeightBuffer().resize(m_samplesCount * m_samplesCount, 0.f);
    m_normalMap.resize(m_samplesCount * m_samplesCount * 4, 0);
    m_distances.resize(m_samplesCount * m_samplesCount, 0.f);
    for (auto i = 0; i < m_samplesCount; i++)
    {
        for (auto j = 0; j < m_samplesCount; j++)
        {
            // Chebyshev distance
            auto dist                           = std::max<int>(std::abs(0 - i),
                                                                std::max<int>(std::abs(0 - j), std::max<int>(std::abs(m_samplesCount - i),
                                                                                                             std::abs(m_samplesCount - j))));
            auto l                              = 2.f * static_cast<float>(dist) / static_cast<float>(m_samplesCount);
            m_distances[i * m_samplesCount + j] = 0.95 * std::min<float>(1.f, l / 0.2f);
        }
    }
}

void mini::gk2::WaterSurfaceSimulation::Update(float dt)
{
    m_deltaTime += dt;
    while (m_deltaTime > m_stepTime)
    {
        Step();
        m_deltaTime -= m_stepTime;
    }
    m_deltaTime = std::max<float>(m_deltaTime, 0.f);

    // Generate a normal map
    auto& curr = GetCurrentHeightBuffer();
    for (auto i = 0; i < m_samplesCount; i++)
    {
        for (auto j = 0; j < m_samplesCount; j++)
        {
            // find gradients
            auto dx   = (GetValue(curr, i, j - 1) - GetValue(curr, i, j + 1)) / 2.f;
            auto dy   = 1.f;
            auto dz   = (GetValue(curr, i - 1, j) - GetValue(curr, i + 1, j)) / 2.f;
            auto sqrt = std::sqrt(dx * dx + dy * dy + dz * dz);

            // m_normalMap[i * m_samplesCount * 4 + j]     = dx / sqrt;
            // m_normalMap[i * m_samplesCount * 4 + j + 1] = dy / sqrt;
            // m_normalMap[i * m_samplesCount * 4 + j + 2] = dz / sqrt;
            m_normalMap[i * m_samplesCount * 4 + j]     = 255;
            m_normalMap[i * m_samplesCount * 4 + j + 1] = 255;
            m_normalMap[i * m_samplesCount * 4 + j + 2] = 255;
            m_normalMap[i * m_samplesCount * 4 + j + 3] = 255; // alpha
        }
    }
}

void mini::gk2::WaterSurfaceSimulation::MapToSurfaceTexture(DxDevice& device, dx_ptr<ID3D11Texture2D>& texture)
{
    // D3D11_MAPPED_SUBRESOURCE mapped;
    // HRESULT hr = device.context()->Map(texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    // if (SUCCEEDED(hr))
    //{
    //     BYTE* dest      = reinterpret_cast<BYTE*>(mapped.pData);
    //     const BYTE* src = reinterpret_cast<const BYTE*>(m_normalMap.data());
    //     size_t rowSize  = m_samplesCount * 4 * sizeof(char);
    //     for (UINT y = 0; y < m_samplesCount; ++y)
    //     {
    //         memcpy(dest + y * mapped.RowPitch, src + y * rowSize, rowSize);
    //     }
    //     device.context()->Unmap(texture.get(), 0);
    // }
}

void mini::gk2::WaterSurfaceSimulation::Step()
{
    auto& curr = GetCurrentHeightBuffer();
    auto& next = GetNextHeightBuffer();

    // Add new distortions
    if (m_uniformDist(m_randGenerator) > m_samplesCount / 2)
    {
        auto i = m_uniformDist(m_randGenerator);
        auto j = m_uniformDist(m_randGenerator);
        SetValue(curr, i, j, GetValue(curr, i, j) + 0.25f);
    }

    auto h      = 2.f / static_cast<float>(m_samplesCount - 1);
    auto a      = m_velocity * m_velocity * m_stepTime * m_stepTime / h / h;
    auto b      = 2.f - 4.f * a;
    using Clock = std::chrono::high_resolution_clock;
    auto start  = Clock::now();
    for (auto i = 1; i < m_samplesCount - 1; i++)
    {
        for (auto j = 1; j < m_samplesCount - 1; j++)
        {
            // Chebyshev distance
            auto bb = b * GetValue(curr, i, j) - GetValue(next, i, j);
            auto aa = a * (GetValue(curr, i + 1, j) + GetValue(curr, i - 1, j) + GetValue(curr, i, j - 1) +
                           GetValue(curr, i, j + 1));
            SetValue(next, i, j, GetValue(m_distances, i, j) * (aa + bb));
        }
    }
    auto end                                          = Clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Loop took " << elapsed.count() << " ms\n";

    SwapHeightBuffers();
}
