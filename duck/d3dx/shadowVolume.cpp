#include "shadowVolume.h"
#include "mesh.h"
#include "pch.h"
namespace mini
{

// CPUMesh<VertexPosition> ShadowVolume::GenerateCPUMeshForTargetMesh(DirectX::XMVECTOR pointLightPos,
//                                                                    const CPUMesh<VertexPositionNormal>& targetMesh)
//{
//     namespace dx = DirectX;
//
//     assert(targetMesh.primitiveType == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ &&
//            L"Only D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ topology is supported.");
//
//     assert(targetMesh.indices.size() % 3 == 0 && L"Target mesh indices count must be divisible by 3.");
//
//     // for (auto i = 0U; i < targetMesh.indices.size(); i += 3)
//     //{
//     //     auto p0 = dx::XMLoadFloat3(&targetMesh.vertices[i].position);
//     //     auto n0 = dx::XMLoadFloat3(&targetMesh.vertices[i].normal);
//
//     //    auto p1 = dx::XMLoadFloat3(&targetMesh.vertices[i + 1].position);
//     //    auto n1 = dx::XMLoadFloat3(&targetMesh.vertices[i + 1].normal);
//
//     //    auto p2 = dx::XMLoadFloat3(&targetMesh.vertices[i + 2].position);
//     //    auto n2 = dx::XMLoadFloat3(&targetMesh.vertices[i + 2].normal);
//
//     //    auto lightDir = dx::XMVectorSubtract(pointLightPos, p0);
//     //    if (dx::XMVectorGetX(dx::XMVector3Dot(lightDir, n0)) > 0.f)
//     //    {
//     //    }
//     //}
//
//     return CPUMesh<VertexPosition>();
// }
//
// ShadowVolume ShadowVolume::CreateFromTargetMesh(const DxDevice& device, DirectX::XMVECTOR pointLightPos,
//                                                 const CPUMesh<VertexPositionNormal>& targetMesh)
//{
//     return ShadowVolume();
// }
//
// void ShadowVolume::ResetForTargetMesh(DirectX::XMVECTOR pointLightPos, const CPUMesh<VertexPositionNormal>&
// targetMesh)
//{
// }

} // namespace mini
