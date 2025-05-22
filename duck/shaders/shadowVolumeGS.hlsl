cbuffer cbProj : register(b0) //Vertex Shader constant buffer slot 2
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
    float4 pos : SV_POSITION;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
};

float4 Transform(float4 vert)
{
    vert = mul(viewMatrix, vert);
    vert = mul(projMatrix, vert);
    return vert;
}

float4 Transform(float3 vert)
{
    return Transform(float4(vert, 1.0));
}

const float EPS = 0.0001;
static const float SHADOW_LENGTH = 10.0;

void EmitQuad(float3 edgeStart, float3 edgeEnd, inout TriangleStream<GSOutput> output)
{
    GSOutput o;

    float3 lPos = lightPos[0];

    float3 lightDir = normalize(edgeStart - lPos);
    o.pos = Transform(float4(edgeStart + lightDir * EPS, 1.0));
    output.Append(o);
    o.pos = Transform(float4(edgeStart + lightDir * SHADOW_LENGTH, 1.0));
    output.Append(o);

    lightDir = normalize(edgeEnd - lPos);
    o.pos = Transform(float4(edgeEnd + lightDir * EPS, 1.0));
    output.Append(o);
    o.pos = Transform(float4(edgeEnd + lightDir * SHADOW_LENGTH, 1.0));
    output.Append(o);

    output.RestartStrip();
}

[maxvertexcount(18)]
void main(
	triangleadj float4 input[6] : SV_POSITION, // input[0,2,4] = triangle vertices, [1,3,5] = adjacent vertices
	inout TriangleStream<GSOutput> output
)
{
    float3 lPos = lightPos[0];

    float3 e1 = input[2].xyz - input[0].xyz;
    float3 e2 = input[4].xyz - input[0].xyz;
    float3 e3 = input[1].xyz - input[0].xyz;
    float3 e4 = input[3].xyz - input[2].xyz;
    float3 e5 = input[4].xyz - input[2].xyz;
    float3 e6 = input[5].xyz - input[0].xyz;

    float3 faceNorm = cross(e1, e2);
    float3 lightDir = lPos - input[0];

    // An edge is considered a silhouette edge if one triangle faces the light but the second one does not
    // To check this we first check if the main triangle (0,2,4) is frontfacing the light. Then for each
    // edge (0,2), (2,4) and (4,0) we check if the triangle is backfacing the light and generate a quad.
    // NOTE: CW winding is assumed

    // Skip backfaced triangles 
    if (dot(faceNorm, lightDir) > 0.0)
    {
        return;
    }

    float3 norm = cross(e3, e1);
    lightDir = lPos - input[0].xyz;
    if (dot(norm, lightDir) > 0.0)
    {
        EmitQuad(input[0].xyz, input[2].xyz, output);
    }

    norm = cross(e4, e5);
    lightDir = lPos - input[2].xyz;
    if (dot(norm, lightDir) > 0.0)
    {
        EmitQuad(input[2].xyz, input[4].xyz, output);
    }

    norm = cross(e2, e6);
    lightDir = lPos - input[4].xyz;
    if (dot(norm, lightDir) > 0.0)
    {
        EmitQuad(input[4].xyz, input[0].xyz, output);
    }

    // Add a top cap
    for (int i = 0; i < 3; ++i)
    {
        lightDir = normalize(input[i * 2].xyz - lPos);
        GSOutput o;
        o.pos = Transform(input[i * 2].xyz + lightDir * EPS);
        output.Append(o);
    }
    output.RestartStrip();

    // Add bottom cap
    for (int i = 2; i > -1; --i)
    {
        lightDir = normalize(input[i * 2].xyz - lPos);
        GSOutput o;
        o.pos = Transform(input[i * 2].xyz + lightDir * SHADOW_LENGTH);
        output.Append(o);
    }
    output.RestartStrip();
}

