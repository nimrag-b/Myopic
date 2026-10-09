#pragma once
#include <DirectXMath.h>
#include "Resource.h"
#include <string>

#include "Shader.h"
#include "Texture.h"
#include "ResourceLocation.h"

//a struct that defines the properties of a material
struct MaterialDef {
	ResourceLocation shader = "standard_lit";
	ResourceLocation texture = nullptr;
	ResourceLocation normal = nullptr;

	DirectX::XMFLOAT4 colour = { 1,1,1,1 };
	float specular = 0.2f;
	DirectX::XMFLOAT3 fresnelRange = { 0,0.5f,1 };

};


class Material : public IResource
{
public:

	Material(DirectX::XMFLOAT4 meshColour, float specular, DirectX::XMFLOAT3 fresnelRange, Shader* shader, Texture* diffuse, Texture* normal) :
		mMeshColour(meshColour),
		mSpecular(specular),
		mFresnelRange(fresnelRange),
		mShader(shader),
		mTexture(diffuse),
		mNormal(normal)
	{ 
		_initialised = true; //material always initialised
	}

	const DirectX::XMFLOAT4 mMeshColour;
	const float mSpecular;
	const DirectX::XMFLOAT3 mFresnelRange;
	const resource_ptr<Shader> const mShader;
	const resource_ptr<Texture> const mTexture; //resource view into texture
	const resource_ptr<Texture> const mNormal; //resource view into texture
};
