#include "pch.h"

#include "duckDemo.h"
#include "mesh.h"
#include "path.h"

#include <iostream>

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const XMFLOAT4 DuckDemo::LIGHT_POS[2]     = {{0.0f, 4.f, 0.0f, 1.0f}, {3.f, 4.f, 0.0f, 1.0f}};
const XMFLOAT4 DuckDemo::ROOM_WALLS_COLOR = {0.8f, 0.8f, 0.4f, 1.f};

DuckDemo::DuckDemo(HINSTANCE appInstance)
    : DxApplication(appInstance, 1280, 720, L"Kaczucha"),
      // Constant Buffers
      m_cbWorldMtx(m_device->CreateConstantBuffer<XMFLOAT4X4>()),   //
      m_cbProjMtx(m_device->CreateConstantBuffer<XMFLOAT4X4>()),    //
      m_cbTexMtx(m_device->CreateConstantBuffer<XMFLOAT4X4>()),     //
      m_cbViewMtx(m_device->CreateConstantBuffer<XMFLOAT4X4, 2>()), //
      m_cbSurfaceColor(m_device->CreateConstantBuffer<XMFLOAT4>()), //
      m_cbLightPos(m_device->CreateConstantBuffer<XMFLOAT4, 2>()),  //
      m_orbitCamera(XMFLOAT3(0, 0, 0))
{

    // Projection matrix
    auto s  = m_window.getClientSize();
    auto ar = static_cast<float>(s.cx) / s.cy;
    XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
    UpdateBuffer(m_cbProjMtx, m_projMtx);
    UpdateCameraCB();

    // Set the camera starting position
    m_orbitCamera.Zoom(5.f);

    // Meshes
    m_roomWalls  = Mesh::ShadedBox(*m_device, ROOM_SIZE, ROOM_SIZE, ROOM_SIZE, true);
    m_waterPlane = Mesh::Rectangle(*m_device, 2.f);
    DirectX::XMStoreFloat4x4(&m_waterPlaneMtx, XMMatrixScaling(ROOM_SIZE / 2.f, ROOM_SIZE / 2.f, ROOM_SIZE / 2.f));

    auto meshesDir = Path::MeshesDir();
    m_duck         = Mesh::LoadMesh(*m_device, meshesDir / "duck" / "duck.txt");
    DirectX::XMStoreFloat4x4(&m_duckMtx, XMMatrixScaling(1.f / DUCK_SCALE, 1.f / DUCK_SCALE, 1.f / DUCK_SCALE));

    // Constant buffers content
    UpdateBuffer(m_cbLightPos, LIGHT_POS);

    // Render states
    CreateRenderStates();

    // Textures
    auto texturesDir  = Path::TexturesDir();
    m_envTextureView  = m_device->CreateShaderResourceView(texturesDir / "cubeMap.dds");
    m_duckTextureView = m_device->CreateShaderResourceView(texturesDir / "ducktex.jpg");
    CreateWaterSurfaceTexture();

    //  Shaders
    auto shadersDir = Path::ShadersDir();

    auto vsCode        = m_device->LoadByteCode(shadersDir / L"phongVS.cso");
    auto psCode        = m_device->LoadByteCode(shadersDir / L"phongPS.cso");
    m_phongVS          = m_device->CreateVertexShader(vsCode);
    m_phongPS          = m_device->CreatePixelShader(psCode);
    m_phongInputLayout = m_device->CreateInputLayout(VertexPositionNormalTexCoords::Layout, vsCode);

    vsCode       = m_device->LoadByteCode(shadersDir / L"texturedVS.cso");
    psCode       = m_device->LoadByteCode(shadersDir / L"texturedPS.cso");
    m_texturedVS = m_device->CreateVertexShader(vsCode);
    m_texturedPS = m_device->CreatePixelShader(psCode);

    vsCode           = m_device->LoadByteCode(shadersDir / L"envVS.cso");
    psCode           = m_device->LoadByteCode(shadersDir / L"envPS.cso");
    m_envVS          = m_device->CreateVertexShader(vsCode);
    m_envPS          = m_device->CreatePixelShader(psCode);
    m_envInputLayout = m_device->CreateInputLayout(VertexPosition::Layout, vsCode);

    vsCode             = m_device->LoadByteCode(shadersDir / L"waterVS.cso");
    psCode             = m_device->LoadByteCode(shadersDir / L"waterPS.cso");
    m_waterVS          = m_device->CreateVertexShader(vsCode);
    m_waterPS          = m_device->CreatePixelShader(psCode);
    m_waterInputLayout = m_device->CreateInputLayout(VertexPosition::Layout, vsCode);

    m_device->context()->IASetInputLayout(m_phongInputLayout.get());
    m_device->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // We have to make sure all shaders use constant buffers in the same slots!
    ID3D11Buffer* vsb[] = {m_cbWorldMtx.get(), m_cbViewMtx.get(), m_cbProjMtx.get(), m_cbTexMtx.get()};
    m_device->context()->VSSetConstantBuffers(
        0, 4,
        vsb); // Vertex Shaders - 0: worldMtx, 1: viewMtx,invViewMtx, 2: projMtx, 3: texMtx
    ID3D11Buffer* gsb[] = {m_cbProjMtx.get(), m_cbViewMtx.get(), m_cbLightPos.get()};
    m_device->context()->GSSetConstantBuffers(0, 3, gsb); // Geometry Shaders - 0: projMtx, 1: viewMtx, 2: lightPos[2]
    ID3D11Buffer* psb[] = {m_cbSurfaceColor.get(), m_cbLightPos.get(), m_cbViewMtx.get()};
    m_device->context()->PSSetConstantBuffers(0, 3, psb); // Pixel Shaders - 0: surfaceColor, 1: lightPos[2], 2: ViewMtx
}

void mini::gk2::DuckDemo::CreateWaterSurfaceTexture()
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width                = WaterSurfaceSimulation::SAMPLES_DEFAULT_SIZE;
    desc.Height               = WaterSurfaceSimulation::SAMPLES_DEFAULT_SIZE;
    desc.MipLevels            = 1;
    desc.ArraySize            = 1;
    desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count     = 1;
    desc.SampleDesc.Quality   = 0;
    desc.Usage                = D3D11_USAGE_DYNAMIC;
    desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags            = 0;

    m_waterSurfaceTexture = m_device->CreateTexture(desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = desc.Format;
    srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels             = 1;

    m_waterSurfaceTextureView = m_device->CreateShaderResourceView(m_waterSurfaceTexture);
}

void mini::gk2::DuckDemo::CreateRenderStates()
{
    m_bsAlpha = m_device->CreateBlendState(BlendDescription::AlphaBlendDescription());

    // Setup wrap sampler
    SamplerDescription sd;
    sd.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxAnisotropy  = 16;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.BorderColor[0] = 1.f;
    sd.BorderColor[1] = 1.f;
    sd.BorderColor[2] = 1.f;
    sd.BorderColor[3] = 1.f;
    sd.MinLOD         = -D3D11_FLOAT32_MAX;
    sd.MaxLOD         = D3D11_FLOAT32_MAX;

    m_samplerWrap = m_device->CreateSamplerState(sd);

    sd.Filter         = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sd.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxAnisotropy  = 16;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.BorderColor[0] = 1.f;
    sd.BorderColor[1] = 1.f;
    sd.BorderColor[2] = 1.f;
    sd.BorderColor[3] = 1.f;
    sd.MinLOD         = 0;
    sd.MaxLOD         = 0;

    m_samplerNormalMap = m_device->CreateSamplerState(sd);

    // Setup rasterizer without culling
    RasterizerDescription rsDesc;
    rsDesc.FrontCounterClockwise = true;
    rsDesc.CullMode              = D3D11_CULL_NONE;
    m_rsCullNone                 = m_device->CreateRasterizerState(rsDesc);
}

void DuckDemo::UpdateCameraCB(XMMATRIX viewMtx)
{
    XMVECTOR det;
    XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
    XMFLOAT4X4 view[2];
    XMStoreFloat4x4(view, viewMtx);
    XMStoreFloat4x4(view + 1, invViewMtx);
    UpdateBuffer(m_cbViewMtx, view);
}

void DuckDemo::HandleControls(double dt)
{
    m_prevKeyboardState = m_currKeyboardState;
    if (m_keyboard.GetState(m_currKeyboardState))
    {
        if (m_currKeyboardState.isKeyDown(DIK_O))
        {
            // SetCameraMode(!m_useOrbitCamera);
        }
    }
}

void DuckDemo::Update(const Clock& c)
{
    double dt = c.getFrameTime();
    m_waterSimulation.Update(dt);
    HandleCameraInput(dt);
    HandleControls(dt);
    if (m_isAnimated)
    {
    }

    m_waterSimulation.MapToSurfaceTexture(*m_device, m_waterSurfaceTexture);
}
void DuckDemo::SetWorldMtx(DirectX::XMFLOAT4X4 mtx)
{
    UpdateBuffer(m_cbWorldMtx, mtx);
}

bool DuckDemo::HandleCameraInput(double dt)
{
    bool result = false;
    KeyboardState kstate;
    if (m_keyboard.GetState(kstate))
    {
    }

    MouseState mstate;
    if (m_mouse.GetState(mstate))
    {
        auto d = mstate.getMousePositionChange();
        if (mstate.isButtonDown(0))
        {
            m_orbitCamera.Rotate(d.y * ROTATION_SPEED * dt, d.x * ROTATION_SPEED * dt);
            result = true;
        }
        if (mstate.isButtonDown(1))
        {
            m_orbitCamera.Zoom(d.y * ZOOM_SPEED * dt);
            result = true;
        }
    }

    return result;
}

void DuckDemo::SetSurfaceColor(DirectX::XMFLOAT4 color)
{
    UpdateBuffer(m_cbSurfaceColor, color);
}

void mini::gk2::DuckDemo::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps,
                                     const dx_ptr<ID3D11InputLayout>& inputLayout)
{
    m_device->context()->IASetInputLayout(inputLayout.get());
    m_device->context()->VSSetShader(vs.get(), nullptr, 0);
    m_device->context()->PSSetShader(ps.get(), nullptr, 0);
}

void mini::gk2::DuckDemo::SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                                      std::initializer_list<ID3D11SamplerState*> samplerList)
{

    m_device->context()->PSSetShaderResources(0, resList.size(), resList.begin());
    m_device->context()->PSSetSamplers(0, samplerList.size(), samplerList.begin());
}

void DuckDemo::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
    SetWorldMtx(worldMtx);
    m.Render(m_device->context());
}

void DuckDemo::DrawRoomWalls()
{
    XMFLOAT4X4 mat;
    DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
    DrawMesh(m_roomWalls, mat);
}

void mini::gk2::DuckDemo::DrawWater()
{
    ID3D11RasterizerState* rsPrev;
    m_device->context()->RSGetState(&rsPrev);
    m_device->context()->RSSetState(m_rsCullNone.get());

    SetSurfaceColor(ROOM_WALLS_COLOR);
    DrawMesh(m_waterPlane, m_waterPlaneMtx);

    m_device->context()->RSSetState(rsPrev);
}

void mini::gk2::DuckDemo::DrawDuck()
{
    DrawMesh(m_duck, m_duckMtx);
}

void DuckDemo::DrawScene()
{
    SetShaders(m_envVS, m_envPS, m_envInputLayout);
    SetTextures({m_envTextureView.get()}, {m_samplerWrap.get()});
    DrawRoomWalls();

    SetShaders(m_waterVS, m_waterPS, m_waterInputLayout);
    SetTextures({m_envTextureView.get(), m_waterSurfaceTextureView.get()},
                {m_samplerWrap.get(), m_samplerNormalMap.get()});
    DrawWater();

    SetShaders(m_phongVS, m_phongPS, m_phongInputLayout);
    SetTextures({m_duckTextureView.get()}, {m_samplerWrap.get()});
    DrawDuck();
}

void DuckDemo::Render()
{
    Base::Render();

    ResetRenderTarget();
    m_device->context()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_device->context()->OMSetDepthStencilState(nullptr, 0);
    UpdateBuffer(m_cbProjMtx, m_projMtx);
    UpdateCameraCB();
    DrawScene();
}