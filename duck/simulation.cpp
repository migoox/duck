#include "pch.h"

#include "simulation.h"
#include "utils/profiling.h"

mini::gk2::Simulation::Simulation() : m_simSpeed(1.0), m_deltaTime(0.0), m_stepTime(0.01666666)
{
}

bool mini::gk2::Simulation::Update(double dt)
{
    PROFILE_ZONE("Simulation::Update");

    m_deltaTime += m_simSpeed * dt;

    bool result = false;
    while (m_deltaTime > m_stepTime)
    {
        Step();
        m_deltaTime -= m_stepTime;
        result = true;
    }

    m_deltaTime = std::max<float>(m_deltaTime, 0.f);

    if (result)
    {
        PostUpdate();
    }
    return result;
}
