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
    float3 norm : NORMAL0;
    float3 tangent : NORMAL1;
    float2 tex : TEXCOORD0;
    float3 viewVec : TEXCOORD1;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float kd = 0.5, ks = 0.2f, m = 100.0f;

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

        if (any(i.tangent))
        {
            // anisotropic lighting
            //float alpha_x = 0.9;
            //float alpha_y = 0.3;
            //float alpha_x_squared = alpha_x * alpha_x;
            //float alpha_y_squared = alpha_y * alpha_y;

            //float3 tangent = normalize(i.anisotropyDir);
            //float3 bitangent = normalize(cross(normal, tangent)); // Make sure it's orthogonal
            //float3 h = normalize(viewVec + lightVec); // Half vector

            //float3 h_tangentSpace = float3(dot(h, tangent), dot(h, bitangent), dot(h, normal));
            //float nh = h_tangentSpace.z;

            //float exponent = (pow(h_tangentSpace.x, 2) / alpha_x_squared) +
            //     (pow(h_tangentSpace.y, 2) / alpha_y_squared);

            //float spec = exp(-exponent / (nh * nh + 1e-5));
            //spec *= ks * saturate(dot(normal, lightVec));
            //color += lightColor * spec;
            float nh = dot(i.tangent, halfVec);
            nh *= nh;
            nh = sqrt(1.0 - nh);
            nh = saturate(nh);
            nh = pow(nh, m);
            nh *= ks;
            color += lightColor * nh;
        }
        else
        {
            // isotropic lighting
            float nh = dot(normal, halfVec);
            nh = saturate(nh);
            nh = pow(nh, m);
            nh *= ks;
            color += lightColor * nh;
        }
        
    }
    return float4(saturate(color), surfaceColor.a);
}