cbuffer cbProj : register(b0) //Geometry Shader constant buffer slot 0
{
	matrix projMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
};

cbuffer cbLights : register(b2)
{
    float4 lightPos[2];
};
struct GSInput
{
	float4 pos : POSITION;
	float age : TEXCOORD0;
	float angle : TEXCOORD1;
	float size : TEXCOORD2;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

static const float TimeToLive = 4.0f;

[maxvertexcount(4)]
void main(point GSInput inArray[1], inout TriangleStream<PSInput> ostream)
{
	GSInput i = inArray[0];
	float sina, cosa;
	sincos(i.angle, sina, cosa);
	float dx = (cosa - sina) * 0.5 * i.size;
	float dy = (cosa + sina) * 0.5 * i.size;
	PSInput o = (PSInput)0;

    float normalizedAge = i.age / TimeToLive;

	// Bottom-left
    o.pos = mul(projMatrix, i.pos + float4(-dx, -dy, 0, 0));
    o.tex1 = float2(0, 1);
    o.tex2 = float2(normalizedAge, 0.5f);
    ostream.Append(o);

	// Top-left
    o.pos = mul(projMatrix, i.pos + float4(-dy, dx, 0, 0));
    o.tex1 = float2(0, 0);
    o.tex2 = float2(normalizedAge, 0.5f);
    ostream.Append(o);

	// Bottom-right
    o.pos = mul(projMatrix, i.pos + float4(dy, -dx, 0, 0));
    o.tex1 = float2(1, 1);
    o.tex2 = float2(normalizedAge, 0.5f);
    ostream.Append(o);

	// Top-right
    o.pos = mul(projMatrix, i.pos + float4(dx, dy, 0, 0));
    o.tex1 = float2(1, 0);
    o.tex2 = float2(normalizedAge, 0.5f);
    ostream.Append(o);


	ostream.RestartStrip();
}