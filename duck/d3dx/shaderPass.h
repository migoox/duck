// #pragma once
// #include "dxDevice.h"
// #include "dxptr.h"
// #include <d3d11.h>
//
// namespace mini
//{
//
// struct ShaderPassDescription
//{
//     dx_ptr<ID3D11InputLayout> inputLayout;
//     dx_ptr<ID3D11VertexShader> vertexShader;
//     dx_ptr<ID3D11PixelShader> pixelShader;
//
//     std::shared_ptr<DxDevice> device;
// };
//
// class ShaderPass
//{
//   public:
//     ShaderPass() = default;
//
//     static ShaderPass Create(ShaderPassDescription&& desc);
//
//     void BindTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
//                       const dx_ptr<ID3D11SamplerState>& sampler);
//     void Use();
//
//   private:
//     explicit ShaderPass(ShaderPassDescription&& desc);
//     ShaderPassDescription m_;
// };
//
// } // namespace mini
