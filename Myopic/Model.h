#pragma once
#include <array>

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"



class Model : public IResource
{
	friend class Graphics;
public:
	static constexpr size_t MAX_MATERIALS = 10;

	Model(){}

	//release references on destroy
	~Model() {
		for (auto m : mMeshes)
		{
			m->release();
		}

		for (size_t i = 0; i < mMaterialCount; i++)
		{
			mMaterials[i]->release();
		}
	}

	gfx_err Create(const std::vector<Mesh*>& meshes, const std::vector<Material*>& materials){
		if (materials.size() > MAX_MATERIALS) {
			return gfx_err::OUT_OF_RANGE;
		}
		mMaterialCount = materials.size();

		mMeshes.resize(meshes.size());

		//make sure we add appropriate meshes
		for (size_t i = 0; i < meshes.size(); i++)
		{
			mMeshes[i] = meshes[i];
			mMeshes[i]->addref();
		}

		for (size_t i = 0; i < materials.size(); i++)
		{
			mMaterials[i] = materials[i];
			mMaterials[i]->addref();
		}
		_initialised = true;
		return gfx_err::OK;
	}

private:
	std::vector<Mesh*> mMeshes;
	std::array<Material*, MAX_MATERIALS> mMaterials{};
	size_t mMaterialCount = 0;

};

