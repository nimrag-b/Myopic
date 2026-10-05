#include "InputBuffers.hlsli"

struct PSInput
{
    float4 pos : SV_Position;
    float3 texCoord : TEXCOORD;
};


TextureCube cubeTex : register(s0);

SamplerState MeshTextureSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
        
    //float t = abs(input.texCoord.y) - 0.5f;
    
    
    //float3 day_colour = {0.54,0.83,0.86 };
    //float3 day_colour1 = { 0.4, 0.93, 0.96 };

    //float3 v = lerp(day_colour, day_colour1, t);
    
    //float3 night_colour = { 0.14, 0.23, 0.26 };

    //float3 v1 = night_colour; //lerp(day_colour, day_colour1, t);
    
    float4 colour = cubeTex.Sample(MeshTextureSampler, input.texCoord);
    return colour;
}