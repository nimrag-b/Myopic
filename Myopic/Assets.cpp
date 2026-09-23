#include "Assets.h"

void Assets::CreateShader(const std::string& tag, const std::string& vertex, const std::string& fragment, VertexLayout* layout)
{
	ComPtr<ID3D11VertexShader> vs;
	if (d3dVertexShaders.contains(vertex)) {
		vs = d3dVertexShaders.at(vertex);
	}
	else {
		const std::wstring wvert = std::wstring(vertex.begin(), vertex.end());
		vs = gfx.DXCreateVertexShader(wvert,layout);
	}

	ComPtr<ID3D11PixelShader> ps;
	if (d3dPixelShaders.contains(fragment)) {
		ps = d3dPixelShaders.at(fragment);
	}
	else {
		const std::wstring wpix = std::wstring(fragment.begin(), fragment.end());
		ps = gfx.DXCreatePixelShader(wpix);
	}

	Register(tag, gfx.DXCreateShader(vs, ps, *layout));
}
