Texture2D albedo : register(t0);
SamplerState samp : register(s0);

cbuffer cbLights : register(b1)
{
    float4 lightPos[2];
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 tangent : NORMAL0;
    float3 norm : NORMAL1;
    float2 tex : TEXCOORD0;
    float3 viewVec : TEXCOORD1;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float kd = 0.8, ks = 0.2f, m = 20.0f;

float4 main(PSInput i) : SV_TARGET
{
    const float4 surfaceColor = albedo.Sample(samp, i.tex);

    float3 viewVec = normalize(i.viewVec);
    float3 normal = normalize(i.norm);
    float3 color = surfaceColor.rgb * ambientColor;
    for (int k = 0; k < 1; k++)
    {
        float3 lightPosition = lightPos[k].xyz;
        float3 lightVec = normalize(lightPosition - i.worldPos);
        float3 halfVec = normalize(viewVec + lightVec);
        color += lightColor * surfaceColor.rgb * kd * saturate(dot(normal, lightVec)); //diffuse color
        // anisotropic specular 
        float3 t = normalize(i.tangent);
        // http://www.bluevoid.com/opengl/sig00/advanced00/notes/node159.html
        float lt = dot(lightVec, t);
        float vt = dot(viewVec, t);
        float vr = saturate(sqrt(1.0 - lt * lt) * sqrt(1.0 - vt * vt) - lt * vt);
        vr = ks * pow(vr, m);
        color += lightColor * vr;

        
    }
    return float4(saturate(color), surfaceColor.a);
}