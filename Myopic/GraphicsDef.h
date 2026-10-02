#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <initializer_list>
#include <memory>


template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

using vector2_t = XMFLOAT2;
using vector3_t = XMFLOAT3;
using vector4_t = XMFLOAT4;


enum class gfx_err {
	OK = 0,
	CONFIG_ERROR,
	BACKEND_ERROR,
	NULL_ERROR,
	UNKNOWN
};

enum class CONSTANT_DATA_FLAGS {
    TEXTURE_DISABLE = 1,
};

//for data that rarely changes
struct alignas(16)  ConstantDataBuffer {
    CONSTANT_DATA_FLAGS flags;
    DirectX::XMFLOAT3 lightDirection;
};

//changed on every frame
struct alignas(16) FrameDataBuffer {
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;

    DirectX::XMFLOAT3 viewPos;

};

//changed for every object
struct alignas(16) ObjectDataBuffer {
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4 meshColor;
    float specular;
    DirectX::XMFLOAT3 fresnelRange;
};



using POSITION = ::XMFLOAT3;
using COLOUR = ::XMFLOAT3;
using NORMAL = ::XMFLOAT3;
using TEXCOORD = ::XMFLOAT2;
using TANGENT = ::XMFLOAT3;
using BINORMAL = ::XMFLOAT3;



class VertexLayout {
public:

	VertexLayout(D3D11_INPUT_ELEMENT_DESC* desc, size_t size) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(size);
		std::copy(desc, desc + size, _desc.get());
		_size = size;
	}

	VertexLayout(std::initializer_list<D3D11_INPUT_ELEMENT_DESC> desc) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(desc.size());
		std::copy(desc.begin(), desc.end(), _desc.get());
		_size = desc.size();
	}

	template <size_t S>
	VertexLayout(D3D11_INPUT_ELEMENT_DESC(&desc)[S]) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(S);
		std::copy(desc, desc + S, _desc.get());
		_size = S;
	}



	D3D11_INPUT_ELEMENT_DESC* desc() const {
		return _desc.get();
	}
	size_t size() const {
		return _size;
	}
	ID3D11InputLayout* layout() const {
		return _layout.Get();
	}
	ID3D11InputLayout** layout_address() {
		return _layout.GetAddressOf();
	}

	bool has_layout() const {
		return _layout != nullptr;
	}

	void invalidate_layout() {
		_layout.Reset();
	}

	bool operator==(VertexLayout& other) {
		return _layout.Get() == other._layout.Get();
	}
	bool operator!=(VertexLayout& other) {
		return _layout.Get() != other._layout.Get();
	}

	bool operator==(const VertexLayout& other) {
		return _layout.Get() == other._layout.Get();
	}
	bool operator!=(const VertexLayout& other) {
		return _layout.Get() != other._layout.Get();
	}

private:
	ComPtr<ID3D11InputLayout> _layout = nullptr;
	std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> _desc;
	size_t _size;
};



//standard game vertex
struct GVertex
{
	POSITION position;
	NORMAL normal;
	TEXCOORD textureCoordinate;
	TANGENT tangent;
	//BINORMAL binormal;
};



static D3D11_INPUT_ELEMENT_DESC GVertexLayout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(GVertex, textureCoordinate), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
	//{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, binormal), D3D11_INPUT_PER_VERTEX_DATA, 0},
};






