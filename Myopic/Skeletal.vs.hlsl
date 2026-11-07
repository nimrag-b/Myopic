#include "ShaderHeader.hlsli"

#define MAX_ANIM_BONES 100
#define MAX_BONE_INFLUENCE 4

cbuffer AnimDataBuffer : register(b3)
{
    matrix boneMatrices[MAX_ANIM_BONES];
};


struct SkeletalVertexShaderInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
    int boneIDs[MAX_BONE_INFLUENCE] : BONEID;
    float weights[MAX_BONE_INFLUENCE] : BONEWEIGHT;
    
};


struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 world : POSITION0;
    //for shadow mapping
    //float4 light_view_projection : POSITION1;
    float3x3 TBN : NORMAL;
    float2 texcoord : TEXCOORD;
};


PixelShaderInput main(SkeletalVertexShaderInput input)
{
    float4 totalPosition = float4(0,0,0,0);
    float3 totalNormal = float3(0,0,0);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (input.boneIDs[i] == -1)
        {
            continue;
        }
            
        if (input.boneIDs[i] >= MAX_ANIM_BONES)
        {
            totalPosition = float4(input.pos, 1.0f);
            totalNormal = input.normal;
            break;
        }
        float4 localPosition = mul(boneMatrices[input.boneIDs[i]], float4(input.pos, 1.0f));
        totalPosition += localPosition * input.weights[i];
        
        float3 localNormal = mul((float3x3)boneMatrices[input.boneIDs[i]], input.normal);
        totalNormal += localNormal * input.weights[i];
    }
    
    PixelShaderInput output;
    //float4 position = float4(input.pos, 1.0f);

    //output.fragPos = mul(pos, model);
    
    output.pos = totalPosition;
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);

    output.world = mul(totalPosition, model);

    output.texcoord = input.texcoord;
    
    float3 binormal = normalize(cross(totalNormal, input.tangent));
    
    float3 T = normalize(mul(float4(input.tangent, 0.0), model).xyz);
    float3 B = normalize(mul(float4(binormal, 0.0), model).xyz);
    float3 N = normalize(mul(float4(totalNormal, 0.0), model).xyz);
    float3x3 TBN = transpose(float3x3(T, B, N));
    output.TBN = TBN;
    
    //SHADOW MAPPING
    /*
    output.light_view_projection = mul(float4(input.pos, 1.0f), model);
    output.light_view_projection = mul(output.light_view_projection, lightView);
    output.light_view_projection = mul(output.light_view_projection, lightProjection);
    */
    return output;
}
