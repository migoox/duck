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
    static constexpr float DEFAULT_VELOCITY   = 256;

    void Update(float dt);
    void MapToSurfaceTexture(::mini::DxDevice& device, dx_ptr<ID3D11Texture2D>& texture);

  private:
    void Step();

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
    { /* if (i < 0 || i >= m_samplesCount || j < 0 || j >= m_samplesCount) { return 0.f; }*/
        return buff[i * m_samplesCount + j];
    }

    float SetValue(std::vector<float>& buff, int i, int j, float val) const
    { /*
         if (i < 0 || i >= m_samplesCount || j < 0 || j >= m_samplesCount)
         {
             return 0.f;
         }*/

        return buff[i * m_samplesCount + j] = val;
    }

  private:
    std::array<std::vector<float>, 2> m_heightBuffers;
    std::vector<char> m_normalMap;
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
