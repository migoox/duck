#pragma once
#include "mesh.h"
#include <DirectXMath.h>

namespace mini
{

class ShadowVolume
{
  public:
    ShadowVolume() = default;

    static CPUMesh<VertexPosition> GenerateCPUMeshForTargetMesh(DirectX::XMVECTOR pointLightPos,
                                                                const CPUMesh<VertexPositionNormal>& targetMesh);

    static ShadowVolume CreateFromTargetMesh(const DxDevice& device, DirectX::XMVECTOR pointLightPos,
                                             const CPUMesh<VertexPositionNormal>& targetMesh);

    void ResetForTargetMesh(DirectX::XMVECTOR pointLightPos, const CPUMesh<VertexPositionNormal>& targetMesh);

  private:
    Mesh m_mesh;
};

} // namespace mini
