#pragma once
#include "Graphics.h"
#include "Resource.h"
#include "ResourceLocation.h"
#include "Allocators.h"

typedef long long STR_ID;

inline STR_ID compute_string_hash(const char* str) {
	const int p = 31;
	const int m = 1e9 + 9;
	STR_ID hash_value = 0;
	long long p_pow = 1;
	char ch = 0;
	while ((ch = *(str++))) {
		hash_value = (hash_value + (ch - 'a' + 1) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}
	return hash_value;
}




template <typename T>
class AssetRegistry {
public:
	AssetRegistry() {
	}

	void setFallback(T* fallback) {
		mFallback = fallback;
	}

	bool contains(STR_ID id) {
		return mItems.contains(id);
	}

	void insert(STR_ID id, T* item) {

		assert(!mItems.contains(id));

		mItems.insert({ id,item });
	}

	T* get(STR_ID id) {
		if (mItems.contains(id)) {
			return mItems[id];
		}
		return mFallback;
	}

	T* remove(STR_ID id) {
		if (mItems.contains(id)) {
			T* item = mItems[id];
			mItems.erase(id);
			return item;
		}
		return nullptr;
	}

	void destructAll() {
		for (auto it : mItems)
		{
			it.second->~T();
		}

		mItems.clear();
	}

private:

	T* mFallback = nullptr;
	std::unordered_map<STR_ID, T*> mItems;
};


//10 mb
constexpr size_t gAllocMax = 10 * 1000 * 1000;

class AssetManager {
public:

	AssetManager(Graphics& gfx) : gfx(gfx), mAllocator(gAllocMax / PoolAllocator::BlockSize) {
		CreateDefaults();
	}

	~AssetManager() {
		Clear();
	}

	enum class LoadMode {
		Locked, //no loading at all
		Allow, //loading allowed
	};

	LoadMode mLoadMode = LoadMode::Allow;

	Shader* LoadShader(const std::string& tag, const std::string& vertex, const std::string& fragment, VertexLayout* layout);

	Texture* LoadTexture(const ResourceLocation& tag);

	Texture* LoadCubemap(const ResourceLocation& tag);

	Model* LoadModel(const ResourceLocation& tag);

	Material* LoadMaterial(const ResourceLocation& name, const MaterialDef& def);

	void Clear() {
		//mTextures.remove(214)->~Texture();
		mModels.destructAll();
		mMaterials.destructAll();
		mMeshes.destructAll();
		mTextures.destructAll();
		mShaders.destructAll();

		mAllocator.clear();
	}


	AssetRegistry<Texture> mTextures;
	AssetRegistry<Mesh> mMeshes;
	AssetRegistry<Model> mModels;
	AssetRegistry<Material> mMaterials;
	AssetRegistry<Shader> mShaders;

private:
	Graphics& gfx;
	PoolAllocator mAllocator;

	void CreateDefaults();
};

