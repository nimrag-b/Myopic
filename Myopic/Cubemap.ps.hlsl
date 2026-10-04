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
    float4 colour = cubeTex.Sample(MeshTextureSampler, input.texCoord);
    return colour;
}