#include "InputBuffers.hlsli"

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

#define PI 3.141592653589793

float2 SpherePointToUV(float3 p)
{
    float3 dir = normalize(p);
    float longitude = atan2(dir.x, -dir.z);
    float latitude = asin(dir.y);
    
    float u = (longitude / PI + 1) * 0.5;
    float v = latitude / PI + 0.5;
    return float2(u, v);
}

PSInput main(VSInput input)
{

    PSInput output;
    output.texCoord = SpherePointToUV(input.position.xyz);
    
    output.position = float4(input.position, 1);
    output.position = mul(output.position, model);
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
    return output;
}