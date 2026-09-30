#pragma once
#include <DirectXMath.h>
#include "Resource.h"
#include <string>

#include "Shader.h"
#include "Texture.h"
#include "ResourceLocation.h"

//a struct that defines the properties of a material
struct MaterialDef {
	DirectX::XMFLOAT4 colour = { 1,1,1,1 };
	float specular = 0.2f;
	DirectX::XMFLOAT3 fresnelRange = { 0,0.5f,1 };
	ResourceLocation shader;
	ResourceLocation texture;
	ResourceLocation normal;

};


class Material : public IResource
{
public:

	Material(DirectX::XMFLOAT4 meshColour, float specular, DirectX::XMFLOAT3 fresnelRange, Shader* shader, Texture* diffuse, Texture* normal) :
		meshColour(meshColour),
		specular(specular),
		fresnelRange(fresnelRange),
		shader(shader),
		texture(diffuse),
		normal(normal)
	{ 
	}
	//void Use(ID3D11DeviceContext* context, ObjectDataBuffer* data);

	const DirectX::XMFLOAT4 meshColour;
	const float specular;
	const DirectX::XMFLOAT3 fresnelRange;
	const Shader* const shader;
	const Texture* const texture; //resource view into texture
	const Texture* const normal; //resource view into texture
};
