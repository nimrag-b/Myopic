#pragma once
#include "Graphics.h"
#include "Resource.h"

class Assets
{
public:
	Assets(Graphics& gfx) : gfx(gfx) {

	}

	void CreateShader(const std::string& tag, const std::string& vertex, const std::string& fragment, VertexLayout* layout);

	template<typename T>
	void Register(const std::string& tag, std::unique_ptr<T> res) {
		GetRegistry<T>().insert(tag, std::move(res));
	}

	template<typename T>
	resource_ptr<T> Get(const std::string& tag) {
		return GetRegistry<T>().get(tag);
	}

	template <typename T>
	Registry<T>& GetRegistry() {
		static_assert(false);
	}
	template <>
	Registry<Shader>& GetRegistry() {
		return shaders;
	}

	template <>
	Registry<Mesh>& GetRegistry() {
		return meshes;
	}



private:
	Graphics& gfx;

	std::unordered_map<std::string, ComPtr<ID3D11VertexShader>> d3dVertexShaders;
	std::unordered_map<std::string, ComPtr<ID3D11PixelShader>> d3dPixelShaders;

	Registry<Shader> shaders;
	Registry<Mesh> meshes;
};

