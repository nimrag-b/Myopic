#include "Assets.h"
#include <fstream>
#include <exception>
#include <array>
#include <chrono>


struct mesh_part {
	uint32_t material_index;
	std::vector<float> verts;
	std::vector<uint32_t> indices;
};


constexpr size_t missing_tex_width = 100;
constexpr size_t missing_tex_block_size = 20;

constexpr size_t missing_tex_bytes = missing_tex_width * missing_tex_width * 4;

std::vector<uint8_t> generate_missing_tex() {

	std::vector<uint8_t> arr( missing_tex_bytes );

	for (size_t j = 0; j < missing_tex_width * 4; j += 4)
	{
		for (size_t i = 0; i < missing_tex_width * 4; i += 4)
		{
			size_t index = (i + (j * missing_tex_width));

			bool x = (i / missing_tex_block_size) % 2;
			bool y = (j / missing_tex_block_size) % 2;

			if (x ? y : !y) {
				arr[index + 0] = 0;
				arr[index + 1] = 0;
				arr[index + 2] = 0;
				arr[index + 3] = 255;
			}
			else {
				arr[index + 0] = 255;
				arr[index + 1] = 0;
				arr[index + 2] = 255;
				arr[index + 3] = 255;
			}
		}

	}

	return arr;
};



Shader* AssetManager::LoadShader(const std::string& tag, const std::string& vertex, const std::string& fragment, VertexLayout* layout)
{
	if (mLoadMode == LoadMode::Locked) return nullptr;

	ComPtr<ID3D11VertexShader> vs;
	const std::wstring wvert = std::wstring(vertex.begin(), vertex.end());
	vs = gfx.DXCreateVertexShader(wvert, layout);

	ComPtr<ID3D11PixelShader> ps;
	const std::wstring wpix = std::wstring(fragment.begin(), fragment.end());
	ps = gfx.DXCreatePixelShader(wpix);

	Shader* shader = mAllocator.alloc_object<Shader>();
	
	if (shader->Create(ps, vs, layout) != gfx_err::OK) {
		mAllocator.dealloc(shader);
		return nullptr;
	}
	mShaders.insert(compute_string_hash(tag.c_str()), shader);

	return shader;
}

Texture* AssetManager::LoadTexture(const ResourceLocation& tag)
{
	if (mLoadMode == LoadMode::Locked) return nullptr;

	std::string path = tag.Filepath("textures");

	SDL_Surface* surfraw = SDL_LoadSurface(path.c_str());

	if (surfraw == nullptr) {
		std::cout << "Could not find texture file at " << path << std::endl;
		return nullptr;
	}

	SDL_Surface* surf = SDL_ConvertSurface(surfraw, SDL_PIXELFORMAT_BGRA32);

	SDL_DestroySurface(surfraw);

	bool v = SDL_LockSurface(surf);

	Texture* t = mAllocator.alloc_object<Texture>();

	gfx_err err = gfx.DXCreateTexture(t, surf->pixels, surf->w, surf->h, surf->pitch, DXGI_FORMAT_B8G8R8A8_UNORM);

	SDL_UnlockSurface(surf);

	SDL_DestroySurface(surf);

	if (err != gfx_err::OK) {
		mAllocator.dealloc(t);
		return nullptr;
	}

	mTextures.insert(compute_string_hash(tag.CStr()), t);

	return t;
}

Texture* AssetManager::LoadCubemap(const ResourceLocation& tag) {

	if (mLoadMode == LoadMode::Locked) return nullptr;
	char str[512];

	std::string pathBase = tag.Filepath("textures");

	SDL_Surface* surfs[6];
	surfs[0] = SDL_LoadSurface((pathBase + "/right.png").c_str());
	surfs[1] = SDL_LoadSurface((pathBase + "/left.png").c_str());

	surfs[2] = SDL_LoadSurface((pathBase + "/up.png").c_str());
	surfs[3] = SDL_LoadSurface((pathBase + "/down.png").c_str());

	surfs[4] = SDL_LoadSurface((pathBase + "/forward.png").c_str());
	surfs[5] = SDL_LoadSurface((pathBase + "/back.png").c_str());

	void* pixels[6];

	for (size_t i = 0; i < 6; i++)
	{
		SDL_Surface* surfraw = surfs[i];
		if (surfraw == nullptr) {
			std::cout << "Failed to load cubemap in folder at " << pathBase << std::endl;
			for (size_t j = 0; j < 6; j++)
			{
				SDL_DestroySurface(surfs[j]);
			}
			return nullptr;
		}

		SDL_Surface* surf = SDL_ConvertSurface(surfraw, SDL_PIXELFORMAT_BGRA32);

		SDL_DestroySurface(surfraw);

		surfs[i] = surf;

		bool v = SDL_LockSurface(surf);

		pixels[i] = surf->pixels;
	}


	Texture* t = mAllocator.alloc_object<Texture>();

	gfx_err err = gfx.DXCreateCubemap(t, pixels, surfs[0]->w, surfs[0]->h, surfs[0]->pitch, DXGI_FORMAT_B8G8R8A8_UNORM);


	for (size_t i = 0; i < 6; i++)
	{
		SDL_UnlockSurface(surfs[i]);

		SDL_DestroySurface(surfs[i]);
	}

	if (err != gfx_err::OK) {
		mAllocator.dealloc(t);
		return nullptr;
	}

	mTextures.insert(compute_string_hash(tag.CStr()), t);
	return t;
}

Material* AssetManager::LoadMaterial(const ResourceLocation& name, const MaterialDef& def)
{

	if (mLoadMode == LoadMode::Locked) return nullptr;
	
	Texture* tex;
	if (def.texture.empty()) {
		static STR_ID sDefaultTexID = compute_string_hash("myopic:default/texture");
		tex = mTextures.get(sDefaultTexID);
	}
	else {
		STR_ID texHash = compute_string_hash(def.texture.CStr());
		if (mTextures.contains(texHash)) {
			tex = mTextures.get(texHash);
		}
		else {
			tex = LoadTexture(def.texture);
		}
	}
	Texture* normal;
	if (def.normal.empty()) {
		static STR_ID sDefaultNormalID = compute_string_hash("myopic:default/normal");
		normal = mTextures.get(sDefaultNormalID);
	}
	else {
		STR_ID texHash = compute_string_hash(def.normal.CStr());
		if (mTextures.contains(texHash)) {
			normal = mTextures.get(texHash);
		}
		else {
			normal = LoadTexture(def.normal);
		}
	}
	
	Shader* shader = mShaders.get(compute_string_hash(def.shader.CStr()));
	
	//cant alloc object as material has no default ctor
	Material* mat = new (mAllocator.alloc(sizeof(Material))) Material(def.colour, def.specular, def.fresnelRange, shader, tex, normal);

	mMaterials.insert(compute_string_hash(name.CStr()), mat);

	return mat;
}


Model* AssetManager::LoadModel(const ResourceLocation& tag) {

	if (mLoadMode == LoadMode::Locked) return nullptr;

	auto startClock = std::chrono::high_resolution_clock::now();

	std::string path = tag.Filepath("meshes", ".nge");
	std::ifstream fs(path, std::ios::binary);
	if (!fs.is_open()) {
		std::cout << "Failed to open file " << path << std::endl;
		return nullptr;
	}

	char magic[4] = { 0 };

	fs.read(magic, 4);
	if (memcmp(magic, "NGE", 4) != 0) {
		return nullptr;
	}

	uint32_t version;
	uint32_t format;

	fs.read((char*)&version, sizeof(uint32_t));
	fs.read((char*)&format, sizeof(uint32_t));

	//for now we are on version one and the only format we support is GVERTEX so just exit if either is wrong
	if (version != 1 || format != 1) {
		return nullptr;
	}

	//amount of submeshes contained in this file
	uint32_t numMeshes;
	fs.read((char*)&numMeshes, sizeof(uint32_t));

	//amount of materials contained in this file
	uint32_t numMaterials;
	fs.read((char*)&numMaterials, sizeof(uint32_t));

	//unused space to be used later
	uint32_t padding[4] = { 0 };
	fs.read((char*)padding, sizeof(padding));

	std::string header;
	char next;

	char buffer[40];

	std::vector<mesh_part> parts(numMeshes);

	mesh_part* current = nullptr;

	std::vector<MaterialDef> mat_parts;

	MaterialDef* current_mat = nullptr;

	while (1) {
		fs >> next;

		if (fs.eof()) break;

		//all section headers must start with .
		if (next != '.') {
			std::cout << "Mesh file invalid for " << path << std::endl;
			return nullptr;
		}

		//read section header
		char* ptr = buffer;
		while (next != 0) {
			*ptr++ = next;
			fs >> next;
		}
		*ptr = 0;
		header = buffer;

		//std::cout << header << std::endl;

		//new mesh
		if (header == ".mesh") {

			uint32_t mesh_index;
			fs.read((char*)&mesh_index, sizeof(uint32_t));
			current = &parts[mesh_index];


			uint32_t material_index;
			fs.read((char*)&material_index, sizeof(uint32_t));
			current->material_index = material_index;
			continue;
		}

		//other than setting a new mesh, there should never be no set mesh
		if (current == nullptr) {
			std::cout << "Mesh file invalid for " << path << std::endl;
			return nullptr;
		}

		//read vertex data
		if (header == ".vertex") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			current->verts.resize(size);

			fs.read((char*)current->verts.data(), size);
			continue;
		}

		//read index data
		if (header == ".index") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			current->indices.resize(size);

			fs.read((char*)current->indices.data(), size);
			continue;
		}

		//material output
		if (header == ".mat") {
			current_mat = &mat_parts.emplace_back();
			continue;
		}

		if (current_mat == nullptr) {
			std::cout << "Mesh file invalid for " << path << std::endl;
			return nullptr;
		}

		if (header == ".diffuse") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			char* str = new char[size + 1];

			fs.read(str, size);
			str[size] = 0;

			current_mat->texture = str;

			delete[] str;
			continue;
		}
		if (header == ".normal") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			char* str = new char[size + 1];

			fs.read(str, size);
			str[size] = 0;

			current_mat->normal = str;

			delete[] str;
			continue;
		}
		if (header == ".colour") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			fs.read((char*)&current_mat->colour, sizeof(current_mat->colour));

			continue;
		}
		if (header == ".spec") {
			uint32_t size; //size in bytes
			fs.read((char*)&size, sizeof(uint32_t));
			fs.read((char*)&current_mat->specular, sizeof(current_mat->specular));

			continue;
		}

	}
	//make sure we have loaded the correct number of meshes
	assert(parts.size() == numMeshes);
	assert(mat_parts.size() == numMaterials);

	std::vector<Mesh*> meshes;

	//GVERTEX is tightly packed so we can just read the array directly into the mesh without padding
	if (format == 1) {

		for (size_t i = 0; i < numMeshes; i++)
		{
			Mesh* mesh = mAllocator.alloc_object<Mesh>();
			mesh_part& part = parts[i];
			gfx.CreateMesh(mesh, part.verts, sizeof(GVertex), part.indices, part.material_index);

			meshes.push_back(mesh);
		}
	}
	else {
		std::cout << "Unknown Mesh format for " << path << std::endl;
		return nullptr;
	}



	std::vector<Material*> mats;

	std::string resloc = tag.CStr();

	for (size_t i = 0; i < numMaterials; i++)
	{
		mats.push_back(LoadMaterial((resloc + "_mat" + std::to_string(i)).c_str(), mat_parts[i]));
	}

	for (size_t i = 0; i < numMeshes; i++)
	{
		mMeshes.insert(compute_string_hash((resloc + "_mesh" + std::to_string(i)).c_str()), meshes[i]);
	}

	//auto mesh_res = meshes.insert(resloc, std::move(mesh));

	auto endClock = std::chrono::high_resolution_clock::now();

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(endClock - startClock);

	std::cout << "Loaded " << path << " in " << ms << std::endl;

	

	Model* model = mAllocator.alloc_object<Model>();
	model->Create(meshes, mats);
	mModels.insert(compute_string_hash(resloc.c_str()), model);
	return model;
}

void AssetManager::CreateDefaults()
{
	static VertexLayout Basiclayout{
	{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	static VertexLayout UIlayout{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GUIVertex, position), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(GUIVertex, texCoord), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};


	std::vector<GUIVertex> GUIvertices = {
	{ POSITION{ -1.0f,  1.0f, 0.0f }, TEXCOORD{ 0, 1 } },
	{ POSITION{  1.0f, -1.0f, 0.0f }, TEXCOORD{ 1, 0 } },
	{ POSITION{ -1.0f, -1.0f, 0.0f }, TEXCOORD{ 0, 0 } },
	{ POSITION{  1.0f,  1.0f, 0.0f }, TEXCOORD{ 1, 1 } },
	};
	std::vector<uint32_t> GUIindices = {
		0, 1, 2,   // first triangle
		0, 3, 1
	};

	//shaders
	LoadShader("myopic:gui", "shaders/GUI.vs.cso", "shaders/GUI.ps.cso", &UIlayout);
	LoadShader("myopic:standard_lit", "shaders/Main.vs.cso", "shaders/Main.ps.cso", &sGvertexLayout);

	gfx_err err;

	//textures
	auto missing_tex_raw = generate_missing_tex();
	Texture* missing_tex = mAllocator.alloc_object<Texture>();
	err = gfx.DXCreateTexture(missing_tex, (void*)missing_tex_raw.data(), missing_tex_width, missing_tex_width, missing_tex_width * 4, DXGI_FORMAT_B8G8R8A8_UNORM);

	const uint8_t normalData[4]{ //1x1 normal map pointing out
	255,128,128,255
	};

	Texture* missing_normal = mAllocator.alloc_object<Texture>();
	err = gfx.DXCreateTexture(missing_normal, (void*)normalData, 1, 1, 4, DXGI_FORMAT_B8G8R8A8_UNORM);


	const uint8_t blankData[4]{ //1x1 normal map pointing out
	255,255,255,255
	};

	Texture* blank_texture = mAllocator.alloc_object<Texture>();
	err = gfx.DXCreateTexture(blank_texture, (void*)blankData, 1, 1, 4, DXGI_FORMAT_B8G8R8A8_UNORM);


	//this feels a bit hacky but hey, it works
	mTextures.insert(compute_string_hash("myopic:error/texture"), missing_tex);
	mTextures.insert(compute_string_hash("myopic:default/normal"), missing_normal);
	mTextures.insert(compute_string_hash("myopic:default/texture"), blank_texture);

	mTextures.setFallback(missing_tex);


}