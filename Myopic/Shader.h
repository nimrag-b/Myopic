#pragma once
#include "Resource.h"
#include "GraphicsDef.h"


class Shader : public IResource {
	friend class Graphics;
public:
	Shader() {

	}
	Shader(ComPtr<ID3D11PixelShader> pixelShader, ComPtr<ID3D11VertexShader> vertexShader, const VertexLayout* layout) : d3dPixelShader(pixelShader), d3dVertexShader(vertexShader), layout(layout) {

	}

	const VertexLayout& Layout() const {
		return *layout;
	}
private:
	ComPtr<ID3D11PixelShader> d3dPixelShader = nullptr;
	ComPtr<ID3D11VertexShader> d3dVertexShader = nullptr;
	const VertexLayout* layout = nullptr;
};

