struct GSOutput
{
    float4 pos : SV_POSITION;
};

float4 main(GSOutput i) : SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
};
