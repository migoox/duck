#include "pch.h"

#include "mesh.h"
#include <algorithm>
#include <fstream>

using namespace std;
using namespace mini;
using namespace DirectX;

Mesh::Mesh() : m_indexCount(0), m_primitiveType(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{
}

Mesh::Mesh(dx_ptr_vector<ID3D11Buffer>&& vbuffers, vector<unsigned int>&& vstrides, vector<unsigned int>&& voffsets,
           dx_ptr<ID3D11Buffer>&& indices, unsigned int indexCount, D3D_PRIMITIVE_TOPOLOGY primitiveType)
{
    assert(vbuffers.size() == voffsets.size() && vbuffers.size() == vstrides.size());
    m_indexCount    = indexCount;
    m_primitiveType = primitiveType;
    m_indexBuffer   = move(indices);

    m_vertexBuffers = std::move(vbuffers);
    m_strides       = move(vstrides);
    m_offsets       = move(voffsets);
}

Mesh::Mesh(Mesh&& right) noexcept
    : m_indexBuffer(move(right.m_indexBuffer)), m_vertexBuffers(move(right.m_vertexBuffers)),
      m_strides(move(right.m_strides)), m_offsets(move(right.m_offsets)), m_indexCount(right.m_indexCount),
      m_primitiveType(right.m_primitiveType)
{
    right.Release();
}

void Mesh::Release()
{
    m_vertexBuffers.clear();
    m_strides.clear();
    m_offsets.clear();
    m_indexBuffer.reset();
    m_indexCount    = 0;
    m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

Mesh& Mesh::operator=(Mesh&& right) noexcept
{
    Release();
    m_vertexBuffers = move(right.m_vertexBuffers);
    m_indexBuffer   = move(right.m_indexBuffer);
    m_strides       = move(right.m_strides);
    m_offsets       = move(right.m_offsets);
    m_indexCount    = right.m_indexCount;
    m_primitiveType = right.m_primitiveType;
    right.Release();
    return *this;
}

void Mesh::Render(const dx_ptr<ID3D11DeviceContext>& context) const
{
    if (!m_indexBuffer || m_vertexBuffers.empty())
        return;
    context->IASetPrimitiveTopology(m_primitiveType);
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetVertexBuffers(0, m_vertexBuffers.size(), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
    context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::~Mesh()
{
    Release();
}

std::vector<VertexPositionColor> mini::Mesh::ColoredBoxVerts(float width, float height, float depth)
{
    return {// Front Face
            {{-0.5f * width, -0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},
            {{+0.5f * width, -0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},
            {{+0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},
            {{-0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},

            // Back Face
            {{+0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 1.0f}},
            {{-0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 1.0f}},
            {{-0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 1.0f}},
            {{+0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 1.0f}},

            // Left Face
            {{-0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f * width, +0.5f * height, -0.5f * depth}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 0.0f}},

            // Right Face
            {{+0.5f * width, -0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 1.0f}},
            {{+0.5f * width, -0.5f * height, +0.5f * depth}, {1.0f, 0.0f, 1.0f}},
            {{+0.5f * width, +0.5f * height, +0.5f * depth}, {1.0f, 0.0f, 1.0f}},
            {{+0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 1.0f}},

            // Bottom Face
            {{-0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, 0.0f, 1.0f}},

            // Top Face
            {{-0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 1.0f, 0.0f}},
            {{+0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 1.0f, 0.0f}},
            {{+0.5f * width, +0.5f * height, +0.5f * depth}, {1.0f, 1.0f, 0.0f}},
            {{-0.5f * width, +0.5f * height, +0.5f * depth}, {1.0f, 1.0f, 0.0f}}};
}

std::vector<VertexPositionNormal> mini::Mesh::ShadedBoxVerts(float width, float height, float depth, bool reverse)
{
    auto vec = std::vector<VertexPositionNormal>({
        // Front face
        {{-0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, 0.0f, -1.0f}},
        {{+0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, 0.0f, -1.0f}},
        {{+0.5f * width, +0.5f * height, -0.5f * depth}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f * width, +0.5f * height, -0.5f * depth}, {0.0f, 0.0f, -1.0f}},

        // Back face
        {{+0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},
        {{+0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 0.0f, 1.0f}},

        // Left face
        {{-0.5f * width, -0.5f * height, +0.5f * depth}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f * width, -0.5f * height, -0.5f * depth}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f * width, +0.5f * height, -0.5f * depth}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f * width, +0.5f * height, +0.5f * depth}, {-1.0f, 0.0f, 0.0f}},

        // Right face
        {{+0.5f * width, -0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},
        {{+0.5f * width, -0.5f * height, +0.5f * depth}, {1.0f, 0.0f, 0.0f}},
        {{+0.5f * width, +0.5f * height, +0.5f * depth}, {1.0f, 0.0f, 0.0f}},
        {{+0.5f * width, +0.5f * height, -0.5f * depth}, {1.0f, 0.0f, 0.0f}},

        // Bottom face
        {{-0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, -1.0f, 0.0f}},
        {{+0.5f * width, -0.5f * height, +0.5f * depth}, {0.0f, -1.0f, 0.0f}},
        {{+0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, -1.0f, 0.0f}},
        {{-0.5f * width, -0.5f * height, -0.5f * depth}, {0.0f, -1.0f, 0.0f}},

        // Top face
        {{-0.5f * width, +0.5f * height, -0.5f * depth}, {0.0f, 1.0f, 0.0f}},
        {{+0.5f * width, +0.5f * height, -0.5f * depth}, {0.0f, 1.0f, 0.0f}},
        {{+0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f * width, +0.5f * height, +0.5f * depth}, {0.0f, 1.0f, 0.0f}},
    });

    if (reverse)
    {
        for (auto& v : vec)
        {
            v.normal.x = -v.normal.x;
            v.normal.y = -v.normal.y;
            v.normal.z = -v.normal.z;
        }
    }

    return vec;
}

std::vector<unsigned short> mini::Mesh::BoxIdxs(bool reverse)
{
    auto vec = std::vector<unsigned short>({0,  2,  1,  0,  3,  2,  4,  6,  5,  4,  7,  6,  8,  10, 9,  8,  11, 10,
                                            12, 14, 13, 12, 15, 14, 16, 18, 17, 16, 19, 18, 20, 22, 21, 20, 23, 22});
    if (reverse)
    {
        std::ranges::reverse(vec);
    }
    return vec;
}

std::vector<VertexPositionNormal> mini::Mesh::PentagonVerts(float radius)
{
    std::vector<VertexPositionNormal> vertices;
    vertices.reserve(5);
    float a = 0, da = XM_2PI / 5.0f;
    for (int i = 0; i < 5; ++i, a -= da)
    {
        float sina, cosa;
        XMScalarSinCos(&sina, &cosa, a);
        vertices.push_back({{cosa * radius, sina * radius, 0.0f}, {0.0f, 0.0f, -1.0f}});
    }
    return vertices;
}

std::vector<unsigned short> mini::Mesh::PentagonIdxs()
{
    return {0, 1, 2, 0, 2, 3, 0, 3, 4};
}

std::vector<VertexPositionNormal> mini::Mesh::DoubleRectVerts(float width, float height)
{
    return {{{-0.5f * width, 0.0f, -0.5f * height}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * width, 0.0f, -0.5f * height}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * width, 0.0f, +0.5f * height}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f * width, 0.0f, +0.5f * height}, {0.0f, 0.0f, 1.0f}},

            {{-0.5f * width, 0.0f, -0.5f * height}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f * width, 0.0f, +0.5f * height}, {0.0f, 0.0f, -1.0f}},
            {{+0.5f * width, 0.0f, +0.5f * height}, {0.0f, 0.0f, -1.0f}},
            {{+0.5f * width, 0.0f, -0.5f * height}, {0.0f, 0.0f, -1.0f}}};
}

std::vector<unsigned short> mini::Mesh::DoubleRectIdxs()
{
    return {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
}

std::vector<VertexPositionNormal> mini::Mesh::RectangleVerts(float width, float height)
{
    return {
        {{+0.5f * width, 0.0f, +0.5f * height}, {0.0f, 1.0f, 0.0f}},
        {{+0.5f * width, 0.0f, -0.5f * height}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f * width, 0.0f, -0.5f * height}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f * width, 0.0f, +0.5f * height}, {0.0f, 1.0f, 0.0f}},
    };
}

std::vector<unsigned short> mini::Mesh::RectangleIdx()
{
    return {0, 1, 2, 0, 2, 3};
}

std::vector<DirectX::XMFLOAT3> mini::Mesh::BillboardVerts(float width, float height)
{
    return {{-0.5f * width, -0.5f * height, 0.0f},
            {-0.5f * width, +0.5f * height, 0.0f},
            {+0.5f * width, +0.5f * height, 0.0f},
            {+0.5f * width, -0.5f * height, 0.0f}};
}

std::vector<VertexPositionNormal> mini::Mesh::SphereVerts(unsigned int stacks, unsigned int slices, float radius)
{
    assert(stacks > 2 && slices > 1);
    auto n = (stacks - 1U) * slices + 2U;
    vector<VertexPositionNormal> vertices(n);
    vertices[0].position = XMFLOAT3(0.0f, radius, 0.0f);
    vertices[0].normal   = XMFLOAT3(0.0f, 1.0f, 0.0f);
    auto dp              = XM_PI / stacks;
    auto phi             = dp;
    auto k               = 1U;
    for (auto i = 0U; i < stacks - 1U; ++i, phi += dp)
    {
        float cosp, sinp;
        XMScalarSinCos(&sinp, &cosp, phi);
        auto thau   = 0.0f;
        auto dt     = XM_2PI / slices;
        auto stackR = radius * sinp;
        auto stackY = radius * cosp;
        for (auto j = 0U; j < slices; ++j, thau += dt)
        {
            float cost, sint;
            XMScalarSinCos(&sint, &cost, thau);
            vertices[k].position = XMFLOAT3(stackR * cost, stackY, stackR * sint);
            vertices[k++].normal = XMFLOAT3(cost * sinp, cosp, sint * sinp);
        }
    }
    vertices[k].position = XMFLOAT3(0.0f, -radius, 0.0f);
    vertices[k].normal   = XMFLOAT3(0.0f, -1.0f, 0.0f);
    return vertices;
}

std::vector<unsigned short> mini::Mesh::SphereIdx(unsigned int stacks, unsigned int slices)
{
    assert(stacks > 2 && slices > 1);
    auto in = (stacks - 1U) * slices * 6U;
    vector<unsigned short> indices(in);
    auto n = (stacks - 1U) * slices + 2U;
    auto k = 0U;
    for (auto j = 0U; j < slices - 1U; ++j)
    {
        indices[k++] = 0U;
        indices[k++] = j + 2;
        indices[k++] = j + 1;
    }
    indices[k++] = 0U;
    indices[k++] = 1U;
    indices[k++] = slices;
    auto i       = 0U;
    for (; i < stacks - 2U; ++i)
    {
        auto j = 0U;
        for (; j < slices - 1U; ++j)
        {
            indices[k++] = i * slices + j + 1;
            indices[k++] = i * slices + j + 2;
            indices[k++] = (i + 1) * slices + j + 2;
            indices[k++] = i * slices + j + 1;
            indices[k++] = (i + 1) * slices + j + 2;
            indices[k++] = (i + 1) * slices + j + 1;
        }
        indices[k++] = i * slices + j + 1;
        indices[k++] = i * slices + 1;
        indices[k++] = (i + 1) * slices + 1;
        indices[k++] = i * slices + j + 1;
        indices[k++] = (i + 1) * slices + 1;
        indices[k++] = (i + 1) * slices + j + 1;
    }
    for (auto j = 0U; j < slices - 1U; ++j)
    {
        indices[k++] = i * slices + j + 1;
        indices[k++] = i * slices + j + 2;
        indices[k++] = n - 1;
    }
    indices[k++] = (i + 1) * slices;
    indices[k++] = i * slices + 1;
    indices[k]   = n - 1;
    return indices;
}

std::vector<VertexPositionNormal> mini::Mesh::CylinderVerts(unsigned int stacks, unsigned int slices, float height,
                                                            float radius)
{
    assert(stacks > 0 && slices > 1);
    auto n = (stacks + 1) * slices + 2 * slices + 2;
    vector<VertexPositionNormal> vertices(n);
    auto y  = height / 2;
    auto dy = height / stacks;
    auto dp = XM_2PI / slices;
    auto k  = 0U;
    for (auto i = 0U; i <= stacks; ++i, y -= dy)
    {
        auto phi = 0.0f;
        for (auto j = 0U; j < slices; ++j, phi += dp)
        {
            float sinp, cosp;
            XMScalarSinCos(&sinp, &cosp, phi);
            vertices[k].position = XMFLOAT3(radius * cosp, y, radius * sinp);
            vertices[k++].normal = XMFLOAT3(cosp, 0, sinp);
        }
    }

    // The top basis
    vertices[k].position = XMFLOAT3(0.f, height / 2.f, 0.f);
    vertices[k++].normal = XMFLOAT3(0.f, 1.f, 0.f);
    auto phi             = 0.0f;
    for (auto i = 0U; i < slices; ++i, phi += dp)
    {
        float sinp, cosp;
        XMScalarSinCos(&sinp, &cosp, phi);
        vertices[k].position = XMFLOAT3(radius * cosp, height / 2.f, radius * sinp);
        vertices[k++].normal = XMFLOAT3(0.f, 1.f, 0.f);
    }

    // The bottom basis
    vertices[k].position = XMFLOAT3(0.f, -height / 2.f, 0.f);
    vertices[k++].normal = XMFLOAT3(0.f, -1.f, 0.f);
    phi                  = 0.0f;
    for (auto i = 0U; i < slices; ++i, phi += dp)
    {
        float sinp, cosp;
        XMScalarSinCos(&sinp, &cosp, phi);
        vertices[k].position = XMFLOAT3(radius * cosp, -height / 2.f, radius * sinp);
        vertices[k++].normal = XMFLOAT3(0.f, -1.f, 0.f);
    }

    return vertices;
}

std::vector<unsigned short> mini::Mesh::CylinderIdx(unsigned int stacks, unsigned int slices)
{
    assert(stacks > 0 && slices > 1);
    auto in = 6 * stacks * slices + slices * 3 * 2;
    vector<unsigned short> indices(in);
    auto k = 0U;
    for (auto i = 0U; i < stacks; ++i)
    {
        auto j = 0U;
        for (; j < slices - 1; ++j)
        {
            indices[k++] = i * slices + j;
            indices[k++] = i * slices + j + 1;
            indices[k++] = (i + 1) * slices + j + 1;
            indices[k++] = i * slices + j;
            indices[k++] = (i + 1) * slices + j + 1;
            indices[k++] = (i + 1) * slices + j;
        }
        indices[k++] = i * slices + j;
        indices[k++] = i * slices;
        indices[k++] = (i + 1) * slices;
        indices[k++] = i * slices + j;
        indices[k++] = (i + 1) * slices;
        indices[k++] = (i + 1) * slices + j;
    }

    int center = (stacks + 1) * slices;
    // The top basis
    for (int i = 0; i < slices; ++i)
    {
        indices[k++] = center;
        indices[k++] = center + 1 + (i + 1) % slices;
        indices[k++] = center + 1 + i % slices;
    }

    center = (stacks + 1) * slices + slices + 1;
    // The bottom basis
    for (int i = 0; i < slices; ++i)
    {
        indices[k++] = center;
        indices[k++] = center + 1 + i % slices;
        indices[k++] = center + 1 + (i + 1) % slices;
    }

    return indices;
}

std::vector<VertexPositionNormal> mini::Mesh::DiskVerts(unsigned int slices, float radius)
{
    assert(slices > 1);
    auto n = slices + 1;
    vector<VertexPositionNormal> vertices(n);
    vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    vertices[0].normal   = XMFLOAT3(0.0f, 1.0f, 0.0f);
    auto phi             = 0.0f;
    auto dp              = XM_2PI / slices;
    auto k               = 1;
    for (auto i = 1U; i <= slices; ++i, phi += dp)
    {
        float cosp, sinp;
        XMScalarSinCos(&sinp, &cosp, phi);
        vertices[k].position = XMFLOAT3(radius * cosp, 0.0f, radius * sinp);
        vertices[k++].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    }
    return vertices;
}

std::vector<unsigned short> mini::Mesh::DiskIdx(unsigned int slices)
{
    assert(slices > 1);
    auto in = slices * 3;
    vector<unsigned short> indices(in);
    auto k = 0U;
    for (auto i = 0U; i < slices - 1; ++i)
    {
        indices[k++] = 0;
        indices[k++] = i + 2;
        indices[k++] = i + 1;
    }
    indices[k++] = 0;
    indices[k++] = 1;
    indices[k]   = slices;
    return indices;
}

Mesh mini::Mesh::LoadMesh(const DxDevice& device, const std::filesystem::path& meshPath)
{
    // File format for VN vertices and IN indices (IN divisible by 3, i.e. IN/3 triangles):
    // VN IN
    // pos.x pos.y pos.z norm.x norm.y norm.z tex.x tex.y [VN times, i.e. for each vertex]
    // t.i1 t.i2 t.i3 [IN/3 times, i.e. for each triangle]

    ifstream input;
    // In general we shouldn't throw exceptions on end-of-file,
    // however, in case of this file format if we reach the end
    // of a file before we read all values, the file is
    // ill-formated and we would need to throw an exception anyway
    input.exceptions(ios::badbit | ios::failbit | ios::eofbit);
    input.open(meshPath);

    int vn;
    input >> vn;

    vector<VertexFrameTexCoords> verts(vn);
    for (auto i = 0; i < vn; ++i)
    {
        input >> verts[i].position.x >> verts[i].position.y >> verts[i].position.z >> verts[i].normal.x >>
            verts[i].normal.y >> verts[i].normal.z >> verts[i].tex.x >> verts[i].tex.y;
    }

    int in;
    input >> in;
    vector<unsigned short> inds(3 * in);
    for (auto i = 0; i < in; ++i)
    {
        input >> inds[3 * i] >> inds[3 * i + 1] >> inds[3 * i + 2];
    }

    // From: https://www.cs.upc.edu/~virtual/G/1.%20Teoria/06.%20Textures/Tangent%20Space%20Calculation.pdf
    for (auto i = 0; i < in; ++i)
    {
        auto idx1 = inds[3 * i];
        auto idx2 = inds[3 * i + 1];
        auto idx3 = inds[3 * i + 2];

        const auto& v1 = verts[idx1].position;
        const auto& v2 = verts[idx2].position;
        const auto& v3 = verts[idx3].position;

        const auto& w1 = verts[idx1].tex;
        const auto& w2 = verts[idx2].tex;
        const auto& w3 = verts[idx3].tex;

        const float x1 = v2.x - v1.x;
        const float x2 = v3.x - v1.x;
        const float y1 = v2.y - v1.y;
        const float y2 = v3.y - v1.y;
        const float z1 = v2.z - v1.z;
        const float z2 = v3.z - v1.z;

        const float s1 = w2.x - w1.x;
        const float s2 = w3.x - w1.x;
        const float t1 = w2.y - w1.y;
        const float t2 = w3.y - w1.y;

        const float r = 1.0F / (s1 * t2 - s2 * t1);

        XMFLOAT3 sdir = {0.f, 0.f, 0.f};
        sdir.x += (t2 * x1 - t1 * x2) * r;
        sdir.y += (t2 * y1 - t1 * y2) * r;
        sdir.z += (t2 * z1 - t1 * z2) * r;

        verts[idx1].tangent.x += sdir.x;
        verts[idx1].tangent.y += sdir.y;
        verts[idx1].tangent.z += sdir.z;

        verts[idx2].tangent.x += sdir.x;
        verts[idx2].tangent.y += sdir.y;
        verts[idx2].tangent.z += sdir.z;

        verts[idx3].tangent.x += sdir.x;
        verts[idx3].tangent.y += sdir.y;
        verts[idx3].tangent.z += sdir.z;
    }
    for (auto i = 0; i < vn; ++i)
    {
        auto normal  = DirectX::XMLoadFloat3(&verts[i].normal);
        auto tangent = DirectX::XMLoadFloat3(&verts[i].tangent);

        // gram-smidt
        tangent = DirectX::XMVector3Normalize(tangent - normal * DirectX::XMVector3Dot(normal, tangent));
        DirectX::XMStoreFloat3(&verts[i].tangent, tangent);
    }

    return SimpleTriMesh(device, verts, inds);
}

std::vector<unsigned short> mini::Mesh::ConvertTriangleListIdxToTriangleListAdjIdx(
    const std::vector<VertexPositionNormal>& vertices, const std::vector<unsigned short>& indices)
{
    // Create unique indices mapping: It's required as there may be more than one vertex for one position
    // which results in multiple indices representing exactly the same position
    auto posToUniqueIdx = std::unordered_map<Position, unsigned short>();
    for (auto i = 0U; i < indices.size(); ++i)
    {
        auto f3  = vertices[indices[i]].position;
        auto pos = Position(f3.x, f3.y, f3.z);
        if (!posToUniqueIdx.contains(pos))
        {
            posToUniqueIdx.insert({pos, indices[i]});
        }
    }

    // Each edge is associated with 2 adjacent positions represented by unique indices
    auto uniqueEdgeToAdjacentIdx = std::unordered_map<Edge, unsigned short>();
    for (auto i = 0U; i < indices.size(); i += 3)
    {
        for (auto j = 0U; j < 3; ++j)
        {
            auto idx       = indices[i + j];
            auto f3        = vertices[idx].position;
            auto pos       = Position(f3.x, f3.y, f3.z);
            auto uniqueIdx = posToUniqueIdx[pos];

            auto nextIdx       = indices[i + (j + 1) % 3];
            auto nextf3        = vertices[nextIdx].position;
            auto nextPos       = Position(nextf3.x, nextf3.y, nextf3.z);
            auto nextUniqueIdx = posToUniqueIdx[nextPos];

            auto adjacentIdx       = indices[i + (j + 2) % 3];
            auto adjacentf3        = vertices[adjacentIdx].position;
            auto adjacentPos       = Position(adjacentf3.x, adjacentf3.y, adjacentf3.z);
            auto adjacentUniqueIdx = posToUniqueIdx[adjacentPos];

            uniqueEdgeToAdjacentIdx.insert({Edge(uniqueIdx, nextUniqueIdx), adjacentUniqueIdx});
        }
    }

    auto indicesAdj = std::vector<unsigned short>();
    indicesAdj.reserve(indices.size() * 2);
    for (auto i = 0U; i < indices.size(); i += 3)
    {
        for (auto j = 0U; j < 3; ++j)
        {
            auto idx       = indices[i + j];
            auto f3        = vertices[idx].position;
            auto pos       = Position(f3.x, f3.y, f3.z);
            auto uniqueIdx = posToUniqueIdx[pos];

            auto nextIdx       = indices[i + (j + 1) % 3];
            auto nextf3        = vertices[nextIdx].position;
            auto nextPos       = Position(nextf3.x, nextf3.y, nextf3.z);
            auto nextUniqueIdx = posToUniqueIdx[nextPos];

            indicesAdj.push_back(idx);
            indicesAdj.push_back(uniqueEdgeToAdjacentIdx[Edge(nextUniqueIdx, uniqueIdx)]); // reversed winding order
        }
    }

    return indicesAdj;
}
