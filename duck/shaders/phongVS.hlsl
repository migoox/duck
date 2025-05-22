cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
    matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
    matrix projMatrix;
};

cbuffer cbWorldClipPlane : register(b4) //Vertex Shader constant buffer slot 4
{
    float4 worldClipPlanePos;
    float4 worldClipPlaneNormal;
};

struct VSInput
{
    float3 pos : POSITION;
    float3 norm : NORMAL0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 viewVec : TEXCOORD0;
    float clipDistance : SV_ClipDistance0;
};

PSInput main(VSInput i)
{
    PSInput o;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f)).xyz;
    o.pos = mul(viewMatrix, float4(o.worldPos, 1.0f));
    o.pos = mul(projMatrix, o.pos);
    o.norm = mul(worldMatrix, float4(i.norm, 0.0f)).xyz;
    o.norm = normalize(o.norm);
    float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    o.viewVec = camPos - o.worldPos;
    
    // Hardware accelarated plane clipping
    float3 cameraPos = mul(invViewMatrix, float4(0.0, 0.0, 0.0, 1.0)).xyz;
    float3 faceDir = worldClipPlanePos.xyz - cameraPos;

    float3 clipNorm = worldClipPlaneNormal;
    if (dot(faceDir, worldClipPlaneNormal.xyz) <= 0)
    {
        clipNorm = -clipNorm;
    }
    o.clipDistance = dot(o.worldPos.xyz, clipNorm) - dot(clipNorm, worldClipPlanePos.xyz);

    return o;
}