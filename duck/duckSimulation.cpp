#include "pch.h"

#include "duckSimulation.h"
#include "utils/profiling.h"
#include <algorithm>
#include <ranges>

mini::gk2::DuckSimulation::DuckSimulation(DirectX::XMFLOAT2 min, DirectX::XMFLOAT2 max)
    : m_uniformDistX(200 * min.x, 200 * max.x), m_uniformDistY(200 * min.y, 200 * max.y)
{
    InitDeBoorPoints();
}

mini::gk2::DuckSimulation::Frame mini::gk2::DuckSimulation::GetCurrentFrame()
{
    return m_frame;
}

void mini::gk2::DuckSimulation::Step()
{
    using namespace DirectX;

    auto p0 = XMLoadFloat2(&m_points[0]);
    auto p1 = XMLoadFloat2(&m_points[1]);
    auto p2 = XMLoadFloat2(&m_points[2]);
    auto p3 = XMLoadFloat2(&m_points[3]);

    // Convert to bezier (uniform knots)
    auto bp0 = (p0 + 4.0f * p1 + p2) / 6.0f;
    auto bp1 = (4.0f * p1 + 2.0f * p2) / 6.0f;
    auto bp2 = (2.0f * p1 + 4.0f * p2) / 6.0f;
    auto bp3 = (p1 + 4.0f * p2 + p3) / 6.0f;

    // Evaulate the bezier
    auto t  = m_tParam;
    auto u  = (1.0 - t);
    auto b3 = t * t * t;
    auto b2 = 3.0 * t * t * u;
    auto b1 = 3.0 * t * u * u;
    auto b0 = u * u * u;
    XMStoreFloat2(&m_frame.pos, bp0 * b0 + bp1 * b1 + bp2 * b2 + bp3 * b3);

    m_tParam += ANIMATION_SPEED * StepTime();
}

void mini::gk2::DuckSimulation::PostUpdate()
{
    UpdateDeBoorPoints();
}

void mini::gk2::DuckSimulation::UpdateDeBoorPoints()
{
    if (m_tParam < 1.f)
    {
        return;
    }
    m_tParam -= 1.f;

    std::ranges::rotate(m_points, m_points.begin() + 1);
    m_points.back().x = m_uniformDistX(m_randGenerator);
    m_points.back().y = m_uniformDistY(m_randGenerator);
}
void mini::gk2::DuckSimulation::InitDeBoorPoints()
{
    for (auto i = 0; i < 5; i++)
    {
        m_points[i].x = m_uniformDistX(m_randGenerator);
        m_points[i].y = m_uniformDistY(m_randGenerator);
    }
}
