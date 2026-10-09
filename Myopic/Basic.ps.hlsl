struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D meshTexture : register(t0);

SamplerState MeshTextureSampler : register(s0);


PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = meshTexture.Sample(MeshTextureSampler, input.texCoord); //float4(input.texCoord,1,1);
    return output;
}