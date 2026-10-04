#pragma once
#include "Graphics.h"
#include "Resource.h"
#include "ResourceLocation.h"


class Assets
{
public:
	Assets(Graphics& gfx) : gfx(gfx) {
		CreateDefaults();
	}

	~Assets() {
		//must be done in the right order to avoid dangling pointers (ie materials reference both textures and shaders so much be cleared first);
		materials.clear();
		textures.clear();
		shaders.clear();
	}

	void CreateShader(const std::string& tag, const ResourceLocation& vertex, const ResourceLocation& fragment, VertexLayout* layout);
	//void LoadMesh(const ResourceLocation& tag);
	std::unique_ptr<Model> LoadModel(const ResourceLocation& tag);

	void LoadTexture(const ResourceLocation& tag);
	void LoadCubemap(const ResourceLocation& tag);

	resource_ptr<Mesh> GetOrCreateMesh(const ResourceLocation& tag);
	resource_ptr<Texture> GetOrCreateTexture(const ResourceLocation& tag);

	resource_ptr<Material> CreateMaterial(const ResourceLocation& name, const MaterialDef& def);

	void RemoveUnreferenced();

	template<typename T>
	resource_ptr<T> Register(const std::string& tag, std::unique_ptr<T> res) {
		return GetRegistry<T>().insert(tag, std::move(res));
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

	template <>
	Registry<Texture>& GetRegistry() {
		return textures;
	}

	template <>
	Registry<Material>& GetRegistry() {
		return materials;
	}

private:

	void CreateDefaults();

	Graphics& gfx;

	std::unordered_map<std::string, ComPtr<ID3D11VertexShader>> d3dVertexShaders;
	std::unordered_map<std::string, ComPtr<ID3D11PixelShader>> d3dPixelShaders;

	Registry<Material> materials;
	Registry<Texture> textures;
	Registry<Shader> shaders;
	Registry<Mesh> meshes;
};

