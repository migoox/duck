#include "pch.h"

#include "duckDemo.h"
#include "mesh.h"
#include "path.h"

#include <iostream>

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const XMFLOAT4 DuckDemo::LIGHT_POS[2]     = {{1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f, 1.0f}};
const XMFLOAT4 DuckDemo::ROOM_WALLS_COLOR = {0.8f, 0.8f, 0.4f, 1.f};

DuckDemo::DuckDemo(HINSTANCE appInstance)
    : DxApplication(appInstance, 1280, 720, L"Kaczucha"),
      // Constant Buffers
      m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),   //
      m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),    //
      m_cbTexMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),     //
      m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()), //
      m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()), //
      m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4, 2>()),  //
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
    m_roomWalls = Mesh::ShadedBox(m_device, ROOM_WIDTH, ROOM_HEIGHT, ROOM_DEPTH, true);

    // Constant buffers content
    UpdateBuffer(m_cbLightPos, LIGHT_POS);

    // Render states
    CreateRenderStates();

    // Textures
    auto texturesDir = Path::TexturesDir();
    m_envTexture     = m_device.CreateShaderResourceView(texturesDir / "cubeMap.dds");

    //  Shaders
    auto shadersDir = Path::ShadersDir();

    auto vsCode = m_device.LoadByteCode(shadersDir / L"phongVS.cso");
    auto psCode = m_device.LoadByteCode(shadersDir / L"phongPS.cso");
    m_phongVS   = m_device.CreateVertexShader(vsCode);
    m_phongPS   = m_device.CreatePixelShader(psCode);

    vsCode       = m_device.LoadByteCode(shadersDir / L"texturedVS.cso");
    psCode       = m_device.LoadByteCode(shadersDir / L"texturedPS.cso");
    m_texturedVS = m_device.CreateVertexShader(vsCode);
    m_texturedPS = m_device.CreatePixelShader(psCode);

    vsCode  = m_device.LoadByteCode(shadersDir / L"envVS.cso");
    psCode  = m_device.LoadByteCode(shadersDir / L"envPS.cso");
    m_envVS = m_device.CreateVertexShader(vsCode);
    m_envPS = m_device.CreatePixelShader(psCode);

    m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);
    m_device.context()->IASetInputLayout(m_inputlayout.get());
    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // We have to make sure all shaders use constant buffers in the same slots!
    ID3D11Buffer* vsb[] = {m_cbWorldMtx.get(), m_cbViewMtx.get(), m_cbProjMtx.get(), m_cbTexMtx.get()};
    m_device.context()->VSSetConstantBuffers(
        0, 4,
        vsb); // Vertex Shaders - 0: worldMtx, 1: viewMtx,invViewMtx, 2: projMtx, 3: texMtx
    ID3D11Buffer* gsb[] = {m_cbProjMtx.get(), m_cbViewMtx.get(), m_cbLightPos.get()};
    m_device.context()->GSSetConstantBuffers(0, 3, gsb); // Geometry Shaders - 0: projMtx, 1: viewMtx, 2: lightPos[2]
    ID3D11Buffer* psb[] = {m_cbSurfaceColor.get(), m_cbLightPos.get()};
    m_device.context()->PSSetConstantBuffers(0, 2, psb); // Pixel Shaders - 0: surfaceColor, 1: lightPos[2]
}

void mini::gk2::DuckDemo::CreateRenderStates()
{
    m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());

    // Setup wrap sampler
    SamplerDescription sd;
    sd.AddressU      = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV      = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW      = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter        = D3D11_FILTER_ANISOTROPIC;
    sd.MaxAnisotropy = 16;

    m_samplerWrap = m_device.CreateSamplerState(sd);
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
    HandleCameraInput(dt);
    HandleControls(dt);
    if (m_isAnimated)
    {
    }
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
            m_orbitCamera.Zoom(d.x * ZOOM_SPEED * dt);
            result = true;
        }
    }

    return result;
}

void DuckDemo::SetSurfaceColor(DirectX::XMFLOAT4 color)
{
    UpdateBuffer(m_cbSurfaceColor, color);
}

void DuckDemo::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps)
{
    m_device.context()->VSSetShader(vs.get(), nullptr, 0);
    m_device.context()->PSSetShader(ps.get(), nullptr, 0);
}

void DuckDemo::SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                           const dx_ptr<ID3D11SamplerState>& sampler)
{
    m_device.context()->PSSetShaderResources(0, resList.size(), resList.begin());
    auto s_ptr = sampler.get();
    m_device.context()->PSSetSamplers(0, 1, &s_ptr);
}

void DuckDemo::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
    SetWorldMtx(worldMtx);
    m.Render(m_device.context());
}

void DuckDemo::DrawRoomWalls()
{
    SetSurfaceColor(ROOM_WALLS_COLOR);
    XMFLOAT4X4 mat;
    DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
    DrawMesh(m_roomWalls, mat);
}

void DuckDemo::DrawScene()
{
    SetShaders(m_envVS, m_envPS);
    SetTextures({m_envTexture.get()}, m_samplerWrap);
    DrawRoomWalls();
}

void DuckDemo::Render()
{
    Base::Render();

    ResetRenderTarget();
    m_device.context()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_device.context()->OMSetDepthStencilState(nullptr, 0);
    UpdateBuffer(m_cbProjMtx, m_projMtx);
    UpdateCameraCB();
    DrawScene();
}