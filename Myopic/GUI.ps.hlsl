struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    output.color = Texture.Sample(TextureSampler, input.texCoord); //float4(input.texCoord.x, input.texCoord.y ,0 , 1.0);
    
    return output;
}