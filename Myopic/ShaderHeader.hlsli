cbuffer ConstantDataBuffer : register(b0)
{
    int flags;

};
cbuffer FrameDataBuffer : register(b1)
{
    matrix projection;
    matrix view;
    //SHADOW MAPPING FIELDS - NOT IMPLEMENTED
    /*
    matrix lightProjection;
    matrix lightView;
    */
    float3 viewPos;

};
cbuffer ObjectDataBuffer : register(b2)
{
    matrix model;
    float4 color;
    float specular;
    float3 fresnelRange;
};


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

