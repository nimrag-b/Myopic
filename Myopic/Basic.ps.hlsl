struct PSInput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D meshTexture : register(t0);

SamplerState MeshTextureSampler : register(s0);


PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    output.color = float4(input.color, 1.0);
    return output;
}