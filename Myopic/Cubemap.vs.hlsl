#include "InputBuffers.hlsli"

struct VSInput
{
    float3 pos : POSITION;
};

struct PSInput
{
    float4 pos : SV_Position;
    float3 texCoord : TEXCOORD;
};


PSInput main(VSInput input)
{
    PSInput output;
    
    float4x4 rotview = view; //remove translation 
    rotview[3][0] = 0;
    rotview[3][1] = 0;
    rotview[3][2] = 0;
    
    output.texCoord = input.pos.xzy; //flip triangle winding
    output.pos = float4(input.pos.xzy, 1);
    output.pos = mul(output.pos, rotview);
    output.pos = mul(output.pos, projection);
	return output;
}