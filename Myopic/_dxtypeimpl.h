#pragma once

#include "Graphics.h"


#pragma region Opaque Pointer Implementations
struct Shader::ShaderImpl {

	ShaderImpl(ComPtr<ID3D11PixelShader> pixelShader, ComPtr<ID3D11VertexShader> vertexShader, const VertexLayout& layout) : d3dPixelShader(pixelShader), d3dVertexShader(vertexShader), layout(layout) {

	}

	const VertexLayout& Layout() const {
		return layout;
	}

	void Bind(ID3D11DeviceContext4* deviceContext) const {
		deviceContext->VSSetShader(d3dVertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(d3dPixelShader.Get(), nullptr, 0);
	}
	void Reset() {
		d3dPixelShader.Reset();
		d3dVertexShader.Reset();
	}

	ComPtr<ID3D11PixelShader> d3dPixelShader;
	ComPtr<ID3D11VertexShader> d3dVertexShader;
	const VertexLayout& layout;
};

struct Mesh::MeshImpl {

	void Render(ID3D11DeviceContext4* deviceContext, int index)
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


struct Texture::TextureImpl {
	ComPtr<ID3D11ShaderResourceView> srv;
};


Shader::Shader(ShaderImpl* impl)
{
	this->impl = std::unique_ptr<ShaderImpl>(impl);
}

Shader::~Shader() = default;

Mesh::Mesh(MeshImpl* impl)
{
	this->impl = std::unique_ptr<MeshImpl>(impl);
}

Mesh::~Mesh() = default;


Texture::Texture(TextureImpl* impl)
{
	this->impl = std::unique_ptr<TextureImpl>(impl);
}

Texture::~Texture() = default;

#pragma endregion