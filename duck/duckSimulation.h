#pragma once
#include "simulation.h"
#include <random>

namespace mini::gk2
{
class DuckSimulation final : public Simulation
{
  public:
    struct Frame
    {
        DirectX::XMVECTOR pos;
        DirectX::XMVECTOR bitangent;
        DirectX::XMVECTOR tangent;
        DirectX::XMVECTOR normal;
    };

    DuckSimulation() = delete;

    DuckSimulation(DirectX::XMFLOAT2 min, DirectX::XMFLOAT2 max);
    ~DuckSimulation() final = default;

    const Frame& GetCurrentFrame();

    static constexpr double ANIMATION_SPEED = 0.2;

  private:
    void Step() final;
    void PostUpdate() final;

    void InitDeBoorPoints();
    void UpdateDeBoorPoints();

  private:
    static constexpr size_t MAX_POINTS = 4;
    std::array<DirectX::XMFLOAT2, MAX_POINTS> m_points;
    double m_tParam;

    std::mt19937 m_randGenerator;
    std::uniform_real_distribution<float> m_uniformDistX;
    std::uniform_real_distribution<float> m_uniformDistY;

    Frame m_frame;
};
} // namespace mini::gk2
