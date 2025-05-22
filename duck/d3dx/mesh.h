#pragma once

#include "dxDevice.h"
#include "dxptr.h"
#include "hashCombine.h"
#include "vertexTypes.h"
#include <D3D11.h>
#include <DirectXMath.h>
#include <vector>

namespace mini
{

struct Edge
{
    unsigned short v1;
    unsigned short v2;

    Edge(unsigned short v1_ind, unsigned short v2_ind) : v1(v1_ind), v2(v2_ind)
    {
    }

    friend bool operator==(const Edge& left, const Edge& right)
    {
        return left.v1 == right.v1 && left.v2 == right.v2;
    }
};

struct Position
{
    float x, y, z;

    Position(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
    {
    }

    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

} // namespace mini

namespace std
{

template <> struct hash<mini::Edge>
{
    std::size_t operator()(const mini::Edge edge) const noexcept
    {
        std::size_t h1 = hash<uint32_t>{}(edge.v1);
        std::size_t h2 = hash<uint32_t>{}(edge.v2);
        mini::HashCombine(h1, h2);
        return h1;
    }
};

template <> struct hash<mini::Position>
{
    std::size_t operator()(const mini::Position pos) const noexcept
    {
        std::size_t h1 = hash<float>{}(pos.x);
        std::size_t h2 = hash<float>{}(pos.y);
        std::size_t h3 = hash<float>{}(pos.z);
        mini::HashCombine(h1, h2);
        mini::HashCombine(h1, h3);
        return h1;
    }
};

} // namespace std

namespace mini
{

template <CVertexLayout Layout> struct CPUMesh
{
    std::vector<Layout> vertices;
    std::vector<unsigned int> indices;
    D3D_PRIMITIVE_TOPOLOGY primitiveType;

    static constexpr D3D_PRIMITIVE_TOPOLOGY DEFAULT_PRIMITIVE_TOPOLOGY = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CPUMesh() : vertices({}), indices({}), primitiveType(DEFAULT_PRIMITIVE_TOPOLOGY)
    {
    }

    CPUMesh(std::vector<Layout>&& _vertices, std::vector<unsigned int>&& _indices,
            D3D_PRIMITIVE_TOPOLOGY _primitiveType = DEFAULT_PRIMITIVE_TOPOLOGY)
        : vertices(std::move(_vertices)), indices(std::move(_indices)), primitiveType(_primitiveType)
    {
    }
};

class Mesh
{
  public:
    Mesh();
    Mesh(dx_ptr_vector<ID3D11Buffer>&& vbuffers, std::vector<unsigned int>&& vstrides, dx_ptr<ID3D11Buffer>&& indices,
         unsigned int indexCount, D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
        : Mesh(std::move(vbuffers), std::move(vstrides), std::vector<unsigned>(vbuffers.size(), 0U), std::move(indices),
               indexCount, primitiveType)
    {
    }
    Mesh(dx_ptr_vector<ID3D11Buffer>&& vbuffers, std::vector<unsigned int>&& vstrides,
         std::vector<unsigned int>&& voffsets, dx_ptr<ID3D11Buffer>&& indices, unsigned int indexCount,
         D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    Mesh(Mesh&& right) noexcept;
    Mesh(const Mesh& right) = delete;
    void Release();
    ~Mesh();

    Mesh& operator=(const Mesh& right) = delete;
    Mesh& operator=(Mesh&& right) noexcept;
    void Render(const dx_ptr<ID3D11DeviceContext>& context) const;

    template <typename VertexType>
    static Mesh SimpleTriMesh(const DxDevice& device, const std::vector<VertexType> verts,
                              const std::vector<unsigned short> idxs)
    {
        if (idxs.empty())
            return {};
        Mesh result;
        result.m_indexBuffer = device.CreateIndexBuffer(idxs);
        result.m_vertexBuffers.push_back(device.CreateVertexBuffer(verts));
        result.m_strides.push_back(sizeof(VertexType));
        result.m_offsets.push_back(0);
        result.m_indexCount    = idxs.size();
        result.m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        return result;
    }

    /** Creates a mesh with adjacent triangles from SimpleTriMesh data
     *
     */
    template <typename VertexType>
    static Mesh SimpleTriAdjMesh(const DxDevice& device, const std::vector<VertexType> verts,
                                 const std::vector<unsigned short> idxs)
    {
        auto idxsAdj = ConvertTriangleListIdxToTriangleListAdjIdx(verts, idxs);
        if (idxsAdj.empty())
            return {};
        Mesh result;
        result.m_indexBuffer = device.CreateIndexBuffer(idxsAdj);
        result.m_vertexBuffers.push_back(device.CreateVertexBuffer(verts));
        result.m_strides.push_back(sizeof(VertexType));
        result.m_offsets.push_back(0);
        result.m_indexCount    = idxsAdj.size();
        result.m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        return result;
    }

    // Box Mesh Creation

    static std::vector<VertexPositionColor> ColoredBoxVerts(float width, float height, float depth);

    static std::vector<VertexPositionColor> ColoredBoxVerts(float side = 1.0f)
    {
        return ColoredBoxVerts(side, side, side);
    }
    static std::vector<VertexPositionNormal> ShadedBoxVerts(float width, float height, float depth,
                                                            bool reverse = false);
    static std::vector<VertexPositionNormal> ShadedBoxVerts(float side = 1.0f)
    {
        return ShadedBoxVerts(side, side, side);
    }

    static std::vector<unsigned short> BoxIdxs(bool reverse = false);

    static Mesh ColoredBox(const DxDevice& device, float width, float height, float depth)
    {
        return SimpleTriMesh(device, ColoredBoxVerts(width, height, depth), BoxIdxs());
    }

    static Mesh ColoredBox(const DxDevice& device, float side = 1.0f)
    {
        return ColoredBox(device, side, side, side);
    }

    static Mesh ShadedBox(const DxDevice& device, float width, float height, float depth, bool reverse = false)
    {
        return SimpleTriMesh(device, ShadedBoxVerts(width, height, depth, reverse), BoxIdxs(reverse));
    }

    static Mesh ShadedBox(const DxDevice& device, float side = 1.0f)
    {
        return ShadedBox(device, side, side, side);
    }

    // Pentagon Mesh Creation

    static std::vector<VertexPositionNormal> PentagonVerts(float radius = 1.0f);
    static std::vector<unsigned short> PentagonIdxs();
    static Mesh Pentagon(const DxDevice& device, float radius = 1.0f)
    {
        return SimpleTriMesh(device, PentagonVerts(), PentagonIdxs());
    }

    // Double-sided Rectangle Mesh Creation

    static std::vector<VertexPositionNormal> DoubleRectVerts(float width, float height);
    static std::vector<VertexPositionNormal> DoubleRectVerts(float side = 1.0f)
    {
        return DoubleRectVerts(side, side);
    }
    static std::vector<unsigned short> DoubleRectIdxs();
    static Mesh DoubleRect(const DxDevice& device, float width, float height, bool triangleAdjacency = false)
    {
        if (triangleAdjacency)
        {
            return SimpleTriAdjMesh(device, DoubleRectVerts(width, height), DoubleRectIdxs());
        }
        return SimpleTriMesh(device, DoubleRectVerts(width, height), DoubleRectIdxs());
    }
    static Mesh DoubleRect(const DxDevice& device, float side = 1.0f, bool triangle_adjacency = false)
    {
        return DoubleRect(device, side, side, triangle_adjacency);
    }

    // Single-side Rectangle/Bilboard Mesh Creation
    static std::vector<VertexPositionNormal> RectangleVerts(float width, float height);
    static std::vector<VertexPositionNormal> RectangleVerts(float side = 1.0f)
    {
        return RectangleVerts(side, side);
    }
    static std::vector<unsigned short> RectangleIdx();
    static Mesh Rectangle(const DxDevice& device, float width, float height)
    {
        return SimpleTriMesh(device, RectangleVerts(width, height), RectangleIdx());
    }
    static Mesh Rectangle(const DxDevice& device, float side = 1.0f)
    {
        return Rectangle(device, side, side);
    }
    static std::vector<DirectX::XMFLOAT3> BillboardVerts(float width, float height);
    static std::vector<DirectX::XMFLOAT3> BillboardVerts(float side = 1.0f)
    {
        return BillboardVerts(side, side);
    }
    static Mesh Billboard(const DxDevice& device, float width, float height)
    {
        return SimpleTriMesh(device, BillboardVerts(width, height), RectangleIdx());
    }
    static Mesh Billboard(const DxDevice& device, float side = 1.0f)
    {
        return Billboard(device, side, side);
    }

    // Sphere Mesh Creation
    static std::vector<VertexPositionNormal> SphereVerts(unsigned int stacks, unsigned int slices, float radius = 1.0f);
    static std::vector<unsigned short> SphereIdx(unsigned int stacks, unsigned int slices);
    static Mesh Sphere(const DxDevice& device, unsigned int stacks, unsigned int slices, float radius = 1.0f)
    {
        return SimpleTriMesh(device, SphereVerts(stacks, slices, radius), SphereIdx(stacks, slices));
    }

    // Cylinder Mesh Creation
    static std::vector<VertexPositionNormal> CylinderVerts(unsigned int stacks, unsigned int slices, float height,
                                                           float radius);
    static std::vector<unsigned short> CylinderIdx(unsigned int stacks, unsigned int slices);
    static Mesh Cylinder(const DxDevice& device, unsigned int stacks, unsigned int slices, float height, float radius,
                         bool triangleAdjacency = false)
    {
        if (triangleAdjacency)
        {
            return SimpleTriAdjMesh(device, CylinderVerts(stacks, slices, height, radius), CylinderIdx(stacks, slices));
        }
        return SimpleTriMesh(device, CylinderVerts(stacks, slices, height, radius), CylinderIdx(stacks, slices));
    }

    // Disc Mesh Creation
    static std::vector<VertexPositionNormal> DiskVerts(unsigned int slices, float radius = 1.0f);
    static std::vector<unsigned short> DiskIdx(unsigned int slices);
    static Mesh Disk(const DxDevice& device, unsigned int slices, float radius = 1.0f)
    {
        return SimpleTriMesh(device, DiskVerts(slices, radius), DiskIdx(slices));
    }

    // Mesh Loading
    static Mesh LoadMesh(const DxDevice& device, const std::filesystem::path& meshPath);

    static std::vector<unsigned short> ConvertTriangleListIdxToTriangleListAdjIdx(
        const std::vector<VertexPositionNormal>& vertices, const std::vector<unsigned short>& indices);

  private:
    dx_ptr<ID3D11Buffer> m_indexBuffer;
    dx_ptr_vector<ID3D11Buffer> m_vertexBuffers;
    std::vector<unsigned int> m_strides;
    std::vector<unsigned int> m_offsets;
    unsigned int m_indexCount;
    D3D_PRIMITIVE_TOPOLOGY m_primitiveType;
};

} // namespace mini
