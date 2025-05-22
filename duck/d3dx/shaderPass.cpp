// #include "shaderPass.h"
#include "pch.h"
//
// namespace mini
//{
// ShaderPass ShaderPass::Create(ShaderPassDescription&& desc)
//{
//     return ShaderPass(std::move(desc));
// }
//
// void ShaderPass::BindTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
//                               const dx_ptr<ID3D11SamplerState>& sampler)
//{
//     m_.device->context()->PSSetShaderResources(0, resList.size(), resList.begin());
//     auto s_ptr = sampler.get();
//     m_.device->context()->PSSetSamplers(0, 1, &s_ptr);
// }
//
// void ShaderPass::Use()
//{
//     m_.device->context()->IASetInputLayout(m_.inputLayout.get());
//     m_.device->context()->VSSetShader(m_.vertexShader.get(), nullptr, 0);
//     m_.device->context()->PSSetShader(m_.pixelShader.get(), nullptr, 0);
// }
// } // namespace mini
