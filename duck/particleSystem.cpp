#include "pch.h"
#include "particleSystem.h"

#include "dxDevice.h"
#include "exceptions.h"

#include <iterator>
#include <ranges>

using namespace mini;
using namespace DirectX;
using namespace std;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[4] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}};

const XMFLOAT3 ParticleSystem::EMITTER_DIR = XMFLOAT3(0.0f, 1.0f, 0.0f);
const float ParticleSystem::TIME_TO_LIVE   = 1.0f;
const float ParticleSystem::EMISSION_RATE  = 80.0f;
const float ParticleSystem::MAX_ANGLE      = XM_PIDIV2 / 9.0f;
const float ParticleSystem::MIN_VELOCITY   = 0.2f;
const float ParticleSystem::MAX_VELOCITY   = 3.33f;
const float ParticleSystem::PARTICLE_SIZE  = 0.08f;
const float ParticleSystem::PARTICLE_SCALE = 1.0f;
const float ParticleSystem::MIN_ANGLE_VEL  = -XM_PI;
const float ParticleSystem::MAX_ANGLE_VEL  = XM_PI;
const int ParticleSystem::MAX_PARTICLES    = 1000;
const XMFLOAT3 ParticleSystem::GRAVITY     = XMFLOAT3(0.0f, -9.81f, 0.0f);

ParticleSystem::ParticleSystem(DirectX::XMFLOAT3 emmiterPosition)
    : m_emitterPos(emmiterPosition), m_particlesToCreate(0.0f), m_random(random_device{}())
{
}

vector<ParticleVertex> ParticleSystem::Update(float dt, DirectX::XMFLOAT4 cameraPosition)
{
    size_t removeCount = 0;
    for (auto& p : m_particles)
    {
        UpdateParticle(p, dt);
        if (p.Vertex.Age >= TIME_TO_LIVE)
            ++removeCount;
    }
    m_particles.erase(m_particles.begin(), m_particles.begin() + removeCount);

    m_particlesToCreate += dt * EMISSION_RATE;
    while (m_particlesToCreate >= 1.0f)
    {
        --m_particlesToCreate;
        if (m_particles.size() < MAX_PARTICLES)
            m_particles.push_back(RandomParticle());
    }
    return GetParticleVerts(cameraPosition);
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
    static std::normal_distribution<float> distX(0.0f, 0.5f);
    static std::normal_distribution<float> distY(0.0f, 0.5f);
    static std::normal_distribution<float> distZ(0.0f, 0.5f);
    static std::uniform_real_distribution<float> velDist(MIN_VELOCITY, MAX_VELOCITY);

    XMVECTOR meanDir = XMVector3Normalize(XMLoadFloat3(&m_emitterNormal));

    XMFLOAT3 perturbation(distX(m_random), distY(m_random), distZ(m_random));

    XMVECTOR randomOffset = XMLoadFloat3(&perturbation);

    XMVECTOR dir = XMVector3Normalize(meanDir + randomOffset);

    float speed = velDist(m_random);
    dir         = XMVectorScale(dir, speed);

    XMFLOAT3 result;
    XMStoreFloat3(&result, dir);
    return result;
}

Particle ParticleSystem::RandomParticle()
{
    static uniform_real_distribution anglularVelDist{MIN_ANGLE_VEL, MAX_ANGLE_VEL};
    Particle p;
    p.Vertex.Age   = 0.0f;
    p.Vertex.Size  = PARTICLE_SIZE;
    p.Vertex.Pos   = m_emitterPos;
    p.Vertex.Angle = 0.0f;

    p.Velocities.AngularVelocity = anglularVelDist(m_random);
    p.Velocities.Velocity        = RandomVelocity();

    return p;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
    p.Vertex.Age += dt;
    XMVECTOR pos = XMLoadFloat3(&p.Vertex.Pos);
    XMVECTOR vel = XMLoadFloat3(&p.Velocities.Velocity);
    vel += XMLoadFloat3(&GRAVITY) * dt;
    XMStoreFloat3(&p.Velocities.Velocity, vel);
    XMStoreFloat3(&p.Vertex.Pos, pos + vel * dt);
    p.Vertex.Angle += p.Velocities.AngularVelocity * dt;
    p.Vertex.Size += PARTICLE_SCALE * PARTICLE_SIZE * dt;
}

vector<ParticleVertex> ParticleSystem::GetParticleVerts(DirectX::XMFLOAT4 cameraPosition)
{
    XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);
    const XMVECTOR camPosVec = XMLoadFloat4(&cameraPosition);

    auto distanceView = m_particles | views::transform([&](const Particle& p) {
                            const XMVECTOR pos = XMLoadFloat3(&p.Vertex.Pos);
                            const float distSq = XMVectorGetX(XMVector3Length(pos - camPosVec));
                            return pair{distSq, p.Vertex};
                        });

    std::vector pairVec(distanceView.begin(), distanceView.end());

    ranges::sort(pairVec, [](const auto& a, const auto& b) { return a.first > b.first; });

    vector<ParticleVertex> vertices;
    vertices.reserve(pairVec.size());
    ranges::transform(pairVec, back_inserter(vertices), [](auto&& pair) { return move(pair.second); });

    return vertices;
}