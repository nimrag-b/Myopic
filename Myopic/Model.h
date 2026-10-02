#pragma once
#include <array>

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"



class Model
{
	friend class Graphics;
public:
	static constexpr size_t MAX_MATERIALS = 10;

	Model(const resource_ptr<Mesh> mesh, const std::vector<resource_ptr<Material>>& materialsvec) : mesh(mesh), material_count(materialsvec.size()) {
		if (materialsvec.size() > MAX_MATERIALS) {
			throw std::exception();
		}
		std::copy(materialsvec.begin(), materialsvec.end(), materials.begin());
	}

private:
	resource_ptr<Mesh> mesh;
	std::array<resource_ptr<Material>, MAX_MATERIALS> materials;
	size_t material_count;

};

