#include "ShaderHeader.hlsli"


struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 world : POSITION0;
    //for shadow mapping
    //float4 light_view_projection : POSITION1;
    float3x3 TBN : NORMAL;
    float2 texcoord : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 position = float4(input.pos, 1.0f);

    //output.fragPos = mul(pos, model);
    
    output.pos = position;
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);

    output.world = mul(position, model);

    output.texcoord = input.texcoord;
    
    float3 binormal = normalize(cross(input.normal, input.tangent));
    
    float3 T = normalize(mul(float4(input.tangent, 0.0),model).xyz);
    float3 B = normalize(mul(float4(binormal, 0.0), model).xyz);
    float3 N = normalize(mul(float4(input.normal, 0.0), model).xyz);
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
