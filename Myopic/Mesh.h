#pragma once
#include "Resource.h"
#include "GraphicsDef.h"


class Mesh : public IResource {
	friend class Graphics;
public: 
	Mesh() {}

	gfx_err Create(ID3D11Device* d3dDevice, std::vector<float>& vertices, size_t vertex_size_bytes, std::vector<uint32_t>& indices, size_t material_index) {
		//create vertex buffer

		mVertexCount = (vertices.size() * sizeof(float)) / vertex_size_bytes;
		mStride = vertex_size_bytes;

		D3D11_BUFFER_DESC bd = { 0 };
		D3D11_SUBRESOURCE_DATA initData = { 0 };

		//create vertex buffer
		bd.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE; //regular usage, no streaming or weirdness
		bd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(float));
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //bind vertexes to vertex buffer
		bd.CPUAccessFlags = 0;
		initData.pSysMem = vertices.data(); //attach data


		HRESULT hr = d3dDevice->CreateBuffer(&bd, &initData, d3dVertexBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			return gfx_err::BACKEND_ERROR;
		}

		//create index buffer

		mIndexCount = indices.size();

		bd = { 0 };
		initData = { 0 };
		//create index buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		initData.pSysMem = indices.data();


		hr = d3dDevice->CreateBuffer(&bd, &initData, d3dIndexBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			d3dVertexBuffer.Reset();
			return gfx_err::BACKEND_ERROR;
		}

		//assign extra data
		mMaterialIndex = material_index;
		_initialised = true;
		return gfx_err::OK;
	}

private:

	ComPtr<ID3D11Buffer> d3dVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> d3dIndexBuffer = nullptr;
	size_t mIndexCount = 0;
	size_t mVertexCount = 0;
	size_t mStride = 0; //stride
	size_t mMaterialIndex = 0;
};