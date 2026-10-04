#include "InputBuffers.hlsli"



Texture2D meshTexture : register(t0);
Texture2D normalTexture : register(t1);
//Texture2D shadowMap : register(t2);

SamplerState MeshTextureSampler : register(s0);
//SamplerComparisonState ShadowMapSampler : register(s1);

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
};

