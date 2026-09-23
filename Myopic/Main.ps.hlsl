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


/*
//shadow mapping is not implemented, so this is just sitting here for now.
float CalculateShadowFactor(float4 light_view_projection)
{
    // Complete projection to NDC
    float3 shadow_coords = light_view_projection.xyz / light_view_projection.w;
    
    // Check if outside light frustum
    if (shadow_coords.z > 1.0f || shadow_coords.z < 0.0f)
    {
        return 1.0f;
    }

    // Transform from NDC [-1, 1] to UV [0, 1]
    float2 tex_coords;
    tex_coords.x = shadow_coords.x * 0.5f + 0.5f;
    tex_coords.y = -shadow_coords.y * 0.5f + 0.5f;

    // SampleCmp performs the comparison (pixel_depth < map_depth) 
    // and returns 1.0 if NOT in shadow, 0.0 if in shadow (based on D3D11_COMPARISON_LESS)
    return shadowMap.SampleCmpLevelZero(ShadowMapSampler, tex_coords, shadow_coords.z);
}
*/

float Fresnel2(float3 vNormal, float3 vEyeDir)
{
    float fresnel = 1 - saturate(dot(vNormal, vEyeDir)); // 1-(N.V) for Fresnel term
    return fresnel * fresnel; // Square for a more subtle look
}

float Fresnel(float3 normal, float3 viewDir)
{
    float fresnel, f = Fresnel2(normal, viewDir); // Traditional Fresnel
    
    if (f > 0.5f)
        fresnel = lerp(fresnelRange.y, fresnelRange.z, (2 * f) - 1); // Blend between mid and high values
    else
        fresnel = lerp(fresnelRange.x, fresnelRange.y, 2 * f); // Blend between low and mid values

    return fresnel;
}

float3 CalculateConstantLight(float intensity, float3 normal, float3 viewDir)
{
    float3 lightColor = float3(1.0f, 0.9f, 0.9f) * intensity;
    float3 lightDir = normalize(float3(7, -7, -10));
    
        //return float4(normal, 1);
    
    //ambient
    float ambientStrength = 0.3f;
    float3 ambient = ambientStrength * lightColor;
    
    //diffuse
    lightDir = -lightDir;
    
    float lightIntensity = saturate(dot(normal, lightDir));
    
    
    float3 diffuse = saturate(lightIntensity * lightColor);
    
    //specular
    float specularStrength = specular;

    float3 reflectDir = reflect(-lightDir, normal);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor;
    
    
    //float shadow = CalculateShadowFactor(light_view_projection);
    
    
    return ambient + ((diffuse + specular)/* * shadow */);

}



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

float3 CalculatePointLight(Light light, float3 position, float3 normal, float3 viewDir)
{

    float3 lightColor = light.colour;

    
    float3 lightDir = normalize(light.position.xyz - position);
    
    float light_distance = length(distance(light.position, position));
    
    
    float3 attenuate_constants = float3(light.attenuation.constant, light.attenuation.linear_, light.attenuation.quadratic);
    float attenuate = 1.0f / dot(attenuate_constants, float3(1.0f, light_distance, light_distance * light_distance));
    

    //ambient
    float ambientStrength = 0.3f;
    float3 ambient = ambientStrength * lightColor;
    
    //diffuse

    float lightIntensity = saturate(dot(normal, lightDir));
    
    
    float3 diffuse = saturate(lightIntensity * lightColor) * attenuate;
    
    //specular
    float specularStrength = specular;

    float3 reflectDir = reflect(-lightDir, normal);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor * attenuate;
    
    return ambient + diffuse + specular;
}


float4 main(PixelShaderInput input) : SV_Target
{

    //setup data

    float4 baseColor = meshTexture.Sample(MeshTextureSampler, input.texcoord);
    
    float3 normal = normalTexture.Sample(MeshTextureSampler, input.texcoord).rgb;
    
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(mul(input.TBN, normal));
    
    float3 viewDir = normalize(viewPos - input.world.xyz);
    


    //calculate result
    float fresnel = Fresnel(normal, viewDir);
    float3 constantLight = CalculateConstantLight(1, normal, viewDir);
    //Attenuation att;
    //att.constant = 0.1;
    //att.linear_ = 0.022;
    //att.quadratic = 0.02;
    
    //Light l1;
    //l1.attenuation = att;
    //l1.colour = float3(1.0f, 0.1f, 0.1f);
    //l1.position = float3(5, 10, 5);
    
    //Light l2;
    //l2.attenuation = att;
    //l2.colour = float3(0.1f, 1.0f, 0.1f);
    //l2.position = float3(-5, 0, 5);
    
    //float3 light1 = CalculatePointLight(l1, input.world.xyz, normal, viewDir);
    //float3 light2 = CalculatePointLight(l2, input.world.xyz, normal, viewDir);
    
    float3 result = (constantLight + fresnel) * color.xyz;

    return float4(result, 1.0f) * baseColor;
}
