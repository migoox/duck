#pragma once
#include "duckSimulation.h"
#include "dxApplication.h"
#include "mesh.h"
#include "shaderPass.h"
#include "waterSurfaceSimulation.h"

namespace mini::gk2
{

class DuckDemo : public DxApplication
{
  public:
    using Base = DxApplication;

    explicit DuckDemo(HINSTANCE appInstance);
    ~DuckDemo() final = default;

  protected:
    void Update(const Clock& c) override;
    void Render() override;

  private:
    void CreateWaterSurfaceTexture();
    void CreateRenderStates();

    void HandleControls(double dt);
    bool HandleCameraInput(double dt);

    void UpdateCameraCB(DirectX::XMMATRIX viewMtx);

    void UpdateCameraCB()
    {
        UpdateCameraCB(m_orbitCamera.getViewMatrix());
    }

    void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);

    void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
    void SetSurfaceColor(DirectX::XMFLOAT4 color);
    void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps,
                    const dx_ptr<ID3D11InputLayout>& inputLayout);

    void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                     std::initializer_list<ID3D11SamplerState*> samplerList);

    void DrawRoomWalls();
    void DrawWater();
    void DrawScene();
    void DrawDuck();

#pragma region CONSTANTS
    // can't have in-class initializer since XMFLOAT... types' constructors are not constexpr
    static const DirectX::XMFLOAT4 LIGHT_POS[2];
    static const DirectX::XMFLOAT4 ROOM_WALLS_COLOR;
    static constexpr size_t PUMA_PARTS    = 6;
    static constexpr size_t ANGLE_COUNT   = 5;
    static constexpr float ROOM_SIZE      = 10.f;
    static constexpr float ROTATION_SPEED = 7.f;
    static constexpr float ZOOM_SPEED     = 5.f;
    static constexpr float DUCK_SCALE     = 1.f / 200.f;
    static constexpr float WATER_LEVEL    = -2.f; // REMEMBER TO MODIFY waterVS
    static constexpr float DUCK_HEIGHT    = 0.94f;

#pragma endregion

#pragma region BUFFERS
    dx_ptr<ID3D11Buffer> m_cbWorldMtx, // vertex shader constant buffer slot 0
        m_cbProjMtx,                   // vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
        m_cbTexMtx;                    // vertex shader constant buffer slot 3
    dx_ptr<ID3D11Buffer> m_cbViewMtx;  // vertex shader constant buffer slot 1

    dx_ptr<ID3D11Buffer> m_cbSurfaceColor; // pixel shader constant buffer slot 0
    dx_ptr<ID3D11Buffer> m_cbLightPos;     // pixel shader constant buffer slot 1
#pragma endregion

#pragma region MESHES
    Mesh m_roomWalls;
    Mesh m_waterPlane;
    Mesh m_duck;
#pragma endregion

#pragma region MATRICES
    DirectX::XMFLOAT4X4 m_projMtx, m_waterPlaneMtx, m_duckMtx;
#pragma endregion

#pragma region STATES_AND_LAYOUTS
    dx_ptr<ID3D11SamplerState> m_samplerWrap;
    dx_ptr<ID3D11SamplerState> m_samplerNormalMap;
    dx_ptr<ID3D11BlendState> m_bsAlpha;

    dx_ptr<ID3D11InputLayout> m_phongInputLayout;
    dx_ptr<ID3D11InputLayout> m_envInputLayout;
    dx_ptr<ID3D11InputLayout> m_waterInputLayout;

    dx_ptr<ID3D11RasterizerState> m_rsCullNone;
#pragma endregion

#pragma region SHADERS
    dx_ptr<ID3D11ShaderResourceView> m_duckTextureView;
    dx_ptr<ID3D11ShaderResourceView> m_envTextureView;
    dx_ptr<ID3D11ShaderResourceView> m_waterSurfaceTextureView;
    dx_ptr<ID3D11Texture2D> m_waterSurfaceTexture;

    dx_ptr<ID3D11VertexShader> m_phongVS;
    dx_ptr<ID3D11PixelShader> m_phongPS;
    dx_ptr<ID3D11VertexShader> m_texturedVS;
    dx_ptr<ID3D11PixelShader> m_texturedPS;
    dx_ptr<ID3D11VertexShader> m_envVS;
    dx_ptr<ID3D11PixelShader> m_envPS;
    dx_ptr<ID3D11VertexShader> m_waterVS;
    dx_ptr<ID3D11PixelShader> m_waterPS;
#pragma endregion

#pragma region CAMERA
    KeyboardState m_prevKeyboardState;
    KeyboardState m_currKeyboardState;

    OrbitCamera m_orbitCamera;

    bool m_isAnimated = true;
#pragma endregion

    WaterSurfaceSimulation m_waterSimulation;
    DuckSimulation m_duckSimulation;
};

} // namespace mini::gk2