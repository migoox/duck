
Texture2D surfaceNormalMap : register(t0);
SamplerState normalMapSamp : register(s0);

cbuffer cbView : register(b2) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
};

static const float s_n1 = 1.0; // air
static const float s_n2 = 4.0 / 3.0; // water

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 inCubePos : POSITION0;
    float3 worldPos : POSITION1;
};

float4 main(PSInput i) : SV_TARGET
{
    float2 tex = 0.5 * (i.inCubePos.xz + float2(1.0, 1.0));
    float3 norm = surfaceNormalMap.Sample(normalMapSamp, tex) * 2.0 - 1.0;
    float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 viewVec = normalize(camPos - i.worldPos);

    if (dot(viewVec, norm) > 0.0)
    {
        return float4(1.0, 1.0, 1.0, 1.0);
    }
    float eta = (s_n2 / s_n1);
    float3 refr = refract(viewVec, norm, eta);
    if (any(refr))
        discard;
    return float4(1.0, 1.0, 1.0, 1.0);
}