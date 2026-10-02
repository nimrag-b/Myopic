#pragma once
#include "Resource.h"
#include "GraphicsDef.h"

class Mesh : public IResource {
	friend class Graphics;
public: 
	void Render(ID3D11DeviceContext* deviceContext, int index)
	{
		deviceContext->IASetIndexBuffer(subMeshes[index].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->DrawIndexed(static_cast<UINT>(subMeshes[index].indexCount), 0, 0);

	}
	void Reset() {
		d3dVertexBuffer.Reset();

		for (size_t i = 0; i < subMeshCount; i++)
		{
			subMeshes[i].indexBuffer.Reset();
		}
	}
private:
	Mesh() {}
	//vertex data
	ComPtr<ID3D11Buffer> d3dVertexBuffer = nullptr;

	size_t stride = 0; //stride
	size_t vertexCount = 0;


	struct index_group {
		ComPtr<ID3D11Buffer> indexBuffer = nullptr;
		size_t indexCount = 0;
	};
	std::unique_ptr<index_group[]> subMeshes = nullptr;
	size_t subMeshCount = 0;
};