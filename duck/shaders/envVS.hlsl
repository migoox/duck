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
    float3 tex : TEXCOORD0;
};

VSOutput main(float3 pos : POSITION0)
{
    VSOutput o;
    o.tex = normalize(pos);
    o.pos = mul(worldMatrix, float4(pos, 1.0));
    o.pos = mul(viewMatrix, o.pos);
    o.pos = mul(projMatrix, o.pos);
    return o;
}