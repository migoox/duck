#include "pch.h"

#include "duckSimulation.h"
#include "utils/profiling.h"
#include <algorithm>
#include <ranges>

mini::gk2::DuckSimulation::DuckSimulation(DirectX::XMFLOAT2 min, DirectX::XMFLOAT2 max)
    : m_uniformDistX(min.x, max.x), m_uniformDistY(min.y, max.y)
{
    InitDeBoorPoints();
    m_frame.normal = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
}

const mini::gk2::DuckSimulation::Frame& mini::gk2::DuckSimulation::GetCurrentFrame()
{
    return m_frame;
}

void mini::gk2::DuckSimulation::Step()
{
    PROFILE_ZONE("DuckSimulation::Step");
    using namespace DirectX;

    const auto p0 = XMLoadFloat2(&m_points[0]);
    const auto p1 = XMLoadFloat2(&m_points[1]);
    const auto p2 = XMLoadFloat2(&m_points[2]);
    const auto p3 = XMLoadFloat2(&m_points[3]);

    // Convert to bezier (uniform knots)
    const auto bp0 = (p0 + 4.f * p1 + p2) / 6.f;
    const auto bp1 = (4.f * p1 + 2.f * p2) / 6.f;
    const auto bp2 = (2.f * p1 + 4.f * p2) / 6.f;
    const auto bp3 = (p1 + 4.f * p2 + p3) / 6.f;

    const auto t = static_cast<float>(m_tParam);
    const auto u = (1.f - t);

    // Evaulate bezier
    const auto b3 = t * t * t;
    const auto b2 = 3.f * t * t * u;
    const auto b1 = 3.f * t * u * u;
    const auto b0 = u * u * u;

    const auto pos = bp0 * b0 + bp1 * b1 + bp2 * b2 + bp3 * b3;
    m_frame.pos    = XMVectorSet(XMVectorGetX(pos), 0.f, XMVectorGetY(pos), 1.f);

    // Find derivative (https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Cubic_B%C3%A9zier_curves)
    const auto c23 = 3.f * t * t;
    const auto c12 = 6.f * u * t;
    const auto c01 = 3.f * u * u;

    const auto v      = c01 * (bp1 - bp0) + c12 * (bp2 - bp1) + c23 * (bp3 - bp2);
    m_frame.tangent   = XMVector3Normalize(XMVectorSet(XMVectorGetX(v), 0.f, XMVectorGetY(v), 0.f));
    m_frame.bitangent = XMVector3Normalize(XMVector3Cross(m_frame.normal, m_frame.tangent));

    m_tParam += ANIMATION_SPEED * StepTime();
}

void mini::gk2::DuckSimulation::PostUpdate()
{
    UpdateDeBoorPoints();
}

void mini::gk2::DuckSimulation::UpdateDeBoorPoints()
{
    PROFILE_ZONE("DuckSimulation::UpdateDeBoorPoints");
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
    for (auto i = 0; i < MAX_POINTS; i++)
    {
        m_points[i].x = m_uniformDistX(m_randGenerator);
        m_points[i].y = m_uniformDistY(m_randGenerator);
    }
}
