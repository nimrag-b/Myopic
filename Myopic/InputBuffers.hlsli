


struct Attenuation
{
    float constant;
    float linear_;
    float quadratic;
};

struct Light
{
    float3 position;
    float3 colour;
    Attenuation attenuation;

};

#define MAX_LIGHTS 16
cbuffer ConstantDataBuffer : register(b0)
{
    int flags;
    int light_count;
    float3 constantLightDir;
    float3 constantLightColor;
    Light lights[MAX_LIGHTS];
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
    uint ticks; // time val, incremented per game tick

};
cbuffer ObjectDataBuffer : register(b2)
{
    matrix model;
    float4 color;
    float3 fresnelRange;
    float specular;
};

