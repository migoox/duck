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

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 inCubePos : POSITION0; // represents position of a water plane vertex in [-1,1]^3 cube
    float3 worldPos : POSITION1;
};

static float WaterLevel = -0.2;

VSOutput main(float3 pos : POSITION0)
{
    VSOutput o;
    o.inCubePos = pos;
    o.inCubePos.y = WaterLevel;
    o.worldPos = mul(worldMatrix, float4(o.inCubePos, 1.0)).xyz;
    o.pos = mul(viewMatrix, float4(o.worldPos, 1.0));
    o.pos = mul(projMatrix, o.pos);

    return o;
}