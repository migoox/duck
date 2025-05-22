
TextureCube envMap : register(t0);
SamplerState samp : register(s0);

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
    float3 viewVec : TEXCOORD0;
};


float frenelCoeff(float3 normal, float3 viewVec, float n1, float n2)
{
    float sqrtF0 = (n2 - n1) / (n2 + n1);
    float F0 = sqrtF0 * sqrtF0;
    float c = (1 - max(abs(dot(normal, viewVec)), 0));
    float c5 = c * c * c * c * c;

    return F0 + (1 - F0) * c5;
}

float4 gamma_correction(float4 color)
{
    return pow(color, 0.4545);
}

float3 intersectRay(float3 origin, float3 dir)
{
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
    float3 invDir = 1.0 / dir;

    float3 tMin = (-1.0 - origin) * invDir;
    float3 tMax = (1.0 - origin) * invDir;

    float3 t1 = min(tMin, tMax);

    float t = max(max(t1.x, t1.y), t1.z);

    return origin + t * dir;
}

float4 main(PSInput i) : SV_TARGET
{
    float3 norm = float3(0.0, 1.0, 0.0);
    float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;

    float3 viewVec = normalize(camPos - i.worldPos);
    if (dot(viewVec, norm) > 0.0)
    {
        float f = frenelCoeff(norm, viewVec, s_n1, s_n2);

        float eta = s_n1 / s_n2;
        norm = -norm;

        float3 refr = refract(viewVec, norm, eta);
        float3 refrHit = intersectRay(i.inCubePos, refr);
        float4 color_refr = envMap.Sample(samp, refrHit);

        float3 refl = reflect(viewVec, norm);
        float3 reflHit = intersectRay(i.inCubePos, refl);
        float4 color_refl = envMap.Sample(samp, reflHit);

        float4 final_color = f * color_refl + (1.0 - f) * color_refr;
        return gamma_correction(final_color);
    }
    else
    {
        float f = frenelCoeff(norm, viewVec, s_n1, s_n2);

        float eta = s_n2 / s_n1;

        float3 refr = refract(viewVec, norm, eta);
        
        float3 refl = reflect(viewVec, norm);
        float3 reflHit = intersectRay(i.inCubePos, refl);
        float4 color_refl = envMap.Sample(samp, reflHit);

        if (!any(refr))
        {
            return gamma_correction(color_refl);
        }

        float3 refrHit = intersectRay(i.inCubePos, refr);
        float4 color_refr = envMap.Sample(samp, refrHit);

        float4 final_color = f * color_refl + (1.0 - f) * color_refr;
        return gamma_correction(final_color);
    }
}