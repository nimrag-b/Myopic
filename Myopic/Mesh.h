#pragma once
#include "Resource.h"
#include "GraphicsDef.h"


class Mesh : public IResource {
	friend class Graphics;
public: 
private:

	Mesh(){}

	ComPtr<ID3D11Buffer> d3dVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> d3dIndexBuffer = nullptr;
	size_t indexCount = 0;
	size_t vertexCount = 0;
	size_t stride = 0; //stride
	size_t materialIndex = 0;
};