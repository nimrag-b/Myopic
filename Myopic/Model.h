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

	Model(const std::vector<Mesh*>& meshes, const std::vector<Material*>& materialsvec) : meshes(meshes), material_count(materialsvec.size()) {
		if (materialsvec.size() > MAX_MATERIALS) {
			throw std::exception();
		}
		for (size_t i = 0; i < materialsvec.size(); i++)
		{
			materials[i] = materialsvec[i];
		}
	}

private:
	std::vector<Mesh*> meshes;
	std::array<resource_ptr<Material>, MAX_MATERIALS> materials;
	size_t material_count;

};

