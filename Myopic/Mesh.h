#pragma once
#include "Resource.h"
#include "GraphicsDef.h"

struct SubMesh {
	ComPtr<ID3D11Buffer> d3dVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> d3dIndexBuffer = nullptr;
	size_t indexCount = 0;
	size_t vertexCount = 0;
	size_t stride = 0; //stride
	size_t materialIndex = 0;
};

class Mesh : public IResource {
	friend class Graphics;
public: 
	Mesh(size_t size) : subMeshes(size) {

	}
	void Reset(int newCount = 0) {

		subMeshes.clear();
		subMeshes.resize(newCount);
	}

	SubMesh& Get(size_t index) {
		return subMeshes[index];
	}

private:
	Mesh() {}

	std::vector<SubMesh> subMeshes;

};