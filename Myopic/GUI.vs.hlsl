struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.position = float4(input.position, 1.0);
    output.texCoord = input.texCoord;
    return output;
}