#pragma once
#include "dxDevice.h"
#include <random>
namespace mini::gk2
{
class WaterSurfaceSimulation
{
  public:
    WaterSurfaceSimulation();

    static constexpr int SAMPLES_DEFAULT_SIZE = 256;
    static constexpr float DEFAULT_VELOCITY   = 1;
    static constexpr float DROP_HEIGHT        = 0.25f;

    static constexpr float ANIMATION_SPEED  = 0.3f;
    static constexpr float DROP_PROBABILITY = 0.2f;

    void Update(float dt);
    void MapToSurfaceTexture(::mini::DxDevice& device, dx_ptr<ID3D11Texture2D>& texture);

  private:
    void Step();

    void InitNormalMap();
    void UpdateNormalMap();
    void InitDistances();

    std::vector<float>& GetCurrentHeightBuffer()
    {
        return m_heightBuffers[m_currentHeightBuffer];
    }

    std::vector<float>& GetNextHeightBuffer()
    {
        return m_heightBuffers[(m_currentHeightBuffer + 1) % 2];
    }

    void SwapHeightBuffers()
    {
        m_currentHeightBuffer = (m_currentHeightBuffer + 1) % 2;
    }

    float GetValue(const std::vector<float>& buff, int i, int j) const
    {
        return buff[i * m_samplesCount + j];
    }

    float SetValue(std::vector<float>& buff, int i, int j, float val) const
    {
        return buff[i * m_samplesCount + j] = val;
    }

  private:
    std::array<std::vector<float>, 2> m_heightBuffers;
    std::vector<BYTE> m_normalMap;
    std::vector<float> m_distances;
    int m_currentHeightBuffer;
    int m_samplesCount;
    float m_velocity;

    float m_stepTime;
    float m_deltaTime;

    std::mt19937 m_randGenerator;
    std::uniform_int_distribution<int> m_uniformDist;
};
} // namespace mini::gk2
