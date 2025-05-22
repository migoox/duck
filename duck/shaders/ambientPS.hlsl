cbuffer cbSurfaceColor : register(b0)
{
    float4 surfaceColor;
};

cbuffer cbLights : register(b1)
{
    float4 lightPos[2];
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 viewVec : TEXCOORD0;
    float clipDistance : SV_ClipDistance0;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);

float4 main(PSInput i) : SV_TARGET
{
    float3 viewVec = normalize(i.viewVec);
    float3 normal = normalize(i.norm);
    float3 color = surfaceColor.rgb * ambientColor;
    return float4(saturate(color), surfaceColor.a);
}