TextureCube envMap : register(t0);
SamplerState samp : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 tex : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
    float3 color = envMap.Sample(samp, i.tex).rgb;
    color = pow(color, 0.4545);
    return float4(color, 1.0f);
}