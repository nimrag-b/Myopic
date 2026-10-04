#include "Assets.h"
#include <fstream>
#include <exception>
#include <array>
#include <chrono>

//custom exception to be thrown if unrecoverable mistake
//if possible, return a default error asset, only throw this if no other choice
class load_error : public std::exception {
private:
	std::string message;
public:

	// Constructor accepting const char*
	load_error(std::string msg) :
		message(msg) {
	}

	// Override what() method, marked
	// noexcept for modern C++
	const char* what() const noexcept {
		return message.c_str();
	}
};


void Assets::CreateShader(const std::string& tag, const ResourceLocation& vertex, const ResourceLocation& fragment, VertexLayout* layout)
{
	std::string vres = vertex.ToString();
	std::string pres = fragment.ToString();
	ComPtr<ID3D11VertexShader> vs;
	if (d3dVertexShaders.contains(vres)) {
		vs = d3dVertexShaders.at(vres);
	}
	else {
		const std::wstring wvert = std::wstring(vres.begin(), vres.end());
		vs = gfx.DXCreateVertexShader(wvert,layout);
	}

	ComPtr<ID3D11PixelShader> ps;
	if (d3dPixelShaders.contains(pres)) {
		ps = d3dPixelShaders.at(pres);
	}
	else {
		const std::wstring wpix = std::wstring(pres.begin(), pres.end());
		ps = gfx.DXCreatePixelShader(wpix);
	}

	Register(tag, gfx.DXCreateShader(vs, ps, *layout));
}

void Assets::LoadTexture(const ResourceLocation& tag) {

	std::string path = tag.Filepath("textures");

	SDL_Surface* surfraw = SDL_LoadSurface(path.c_str());

	if (surfraw == nullptr) {
		std::cout << "Could not find texture file at " << path << std::endl;
		return;
	}

	SDL_Surface* surf = SDL_ConvertSurface(surfraw, SDL_PIXELFORMAT_BGRA32);

	SDL_DestroySurface(surfraw);

	bool v = SDL_LockSurface(surf);

	auto tex = gfx.DXCreateTexture(surf->pixels, surf->w, surf->h, surf->pitch, DXGI_FORMAT_B8G8R8A8_UNORM);

	SDL_UnlockSurface(surf);

	SDL_DestroySurface(surf);

	Register(tag.ToString(), std::move(tex));
}

void Assets::LoadCubemap(const ResourceLocation& tag) {

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
			return;
		}

		SDL_Surface* surf = SDL_ConvertSurface(surfraw, SDL_PIXELFORMAT_BGRA32);

		SDL_DestroySurface(surfraw);

		surfs[i] = surf;

		bool v = SDL_LockSurface(surf);

		pixels[i] = surf->pixels;
	}




	auto tex = gfx.DXCreateCubemap(pixels, surfs[0]->w, surfs[0]->h, surfs[0]->pitch, DXGI_FORMAT_B8G8R8A8_UNORM);

	for (size_t i = 0; i < 6; i++)
	{
		SDL_UnlockSurface(surfs[i]);

		SDL_DestroySurface(surfs[i]);
	}



	Register(tag.ToString(), std::move(tex));
}

resource_ptr<Texture> Assets::GetOrCreateTexture(const ResourceLocation& tag)
{
	std::string name = tag.ToString();
	auto& reg = GetRegistry<Texture>();
	resource_ptr<Texture> res = reg.get(name);
	if (res == nullptr) {
		LoadTexture(tag);
		res = reg.get(name);
	}

	if (res == nullptr) {
		res = reg.get_err();
	}
	return res;

}

resource_ptr<Mesh> Assets::GetOrCreateMesh(const ResourceLocation& tag)
{
	std::string name = tag.ToString();
	auto& reg = GetRegistry<Mesh>();
	resource_ptr<Mesh> res = reg.get(name);
	if (res == nullptr) {
		LoadModel(tag);
		res = reg.get(name);
	}

	if (res == nullptr) {
		res = reg.get_err();
	}
	return res;

}

//
//void Assets::LoadMesh(const ResourceLocation& tag)
//{
//	std::string path = tag.Filepath("meshes", ".naf");
//	std::ifstream file(path, std::ios::binary);
//
//	struct alignas(4) naf_header {
//		unsigned int magic;
//		unsigned int format;
//		unsigned int mesh_count;
//		unsigned int vertex_count;
//	};
//
//
//	if (file.is_open()) {
//
//		std::vector<char> bytes(
//			(std::istreambuf_iterator<char>(file)),
//			(std::istreambuf_iterator<char>()));
//		size_t ptr = 0;
//
//		naf_header header = *(reinterpret_cast<naf_header*>(&bytes[ptr]));
//		ptr += sizeof(naf_header);
//
//		if (header.magic != 0xB33FB33F) throw load_error("File '" + path + "' is not a valid mesh");
//
//		if (header.format != 1) throw load_error("Invalid mesh format for '" + path + "'");
//
//
//		try {
//			GVertex* begin = reinterpret_cast<GVertex*>(&bytes[ptr]);
//			std::vector<GVertex> vertices = std::vector<GVertex>(begin, begin + header.vertex_count);
//			ptr += header.vertex_count * sizeof(GVertex);
//
//			auto Mesh = gfx.CreateMesh(vertices, header.mesh_count);
//
//			for (size_t i = 0; i < header.mesh_count; i++)
//			{
//				unsigned int nameLength = *(reinterpret_cast<unsigned int*>(&bytes[ptr]));
//				ptr += sizeof(nameLength);
//				std::string name(&bytes[ptr], &bytes[ptr] + nameLength);
//				ptr += nameLength * sizeof(char);
//
//				unsigned int indexCount = *(reinterpret_cast<unsigned int*>(&bytes[ptr]));
//				ptr += sizeof(indexCount);
//				uint32_t* ibegin = reinterpret_cast<uint32_t*>(&bytes[ptr]);
//				std::vector<uint32_t> indices = std::vector<uint32_t>(ibegin, ibegin + indexCount);
//				ptr += indexCount * sizeof(uint32_t);
//
//				gfx.SetSubMesh(Mesh, i, indices);
//			}
//
//
//			Register(tag.ToString(), std::move(Mesh));
//
//		}
//		catch (std::exception e) {
//			throw load_error("Error in mesh data causing '" + std::string(e.what()) + "'");
//		}
//
//		file.close();
//
//		return;
//	}
//
//	throw load_error("File '" + path + "' does not exist");
//}


struct mesh_part {
	uint32_t material_index;
	std::vector<float> verts;
	std::vector<uint32_t> indices;
};



std::unique_ptr<Model> Assets::LoadModel(const ResourceLocation& tag)
{
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
			char* str = new char[size+1];

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

	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(numMeshes);

	//GVERTEX is tightly packed so we can just read the array directly into the mesh without padding
	if (format == 1) {

		for (size_t i = 0; i < numMeshes; i++)
		{
			SubMesh& sm = mesh->Get(i);
			mesh_part& part = parts[i];
			gfx.CreateSubMesh(sm, part.verts, sizeof(GVertex), part.indices, part.material_index);
		}
	}
	else {
		std::cout << "Unknown Mesh format for " << path << std::endl;
		return nullptr;
	}



	std::vector<resource_ptr<Material>> mats;

	std::string resloc = tag.ToString();

	for (size_t i = 0; i < numMaterials; i++)
	{
		mats.push_back(CreateMaterial(resloc + "_mat" + std::to_string(i), mat_parts[i]));
	}

	auto mesh_res = meshes.insert(resloc, std::move(mesh));

	auto endClock = std::chrono::high_resolution_clock::now();

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(endClock - startClock);

	std::cout << "Loaded " << path << " in " << ms << std::endl;

	return std::make_unique<Model>(mesh_res,mats);
}

resource_ptr<Material> Assets::CreateMaterial(const ResourceLocation& name, const MaterialDef& def)
{
	if (materials.contains(name.ToString())) {
		throw load_error("Cannot create material '" + name.ToString() + "' as a material with the same name already exists");
	}

	resource_ptr<Texture> tex;
	if (def.texture.empty()) {
		tex = Get<Texture>("myopic:default/texture");
	}
	else {
		tex = GetOrCreateTexture(def.texture);
	}
	resource_ptr<Texture> normal;
	if (def.normal.empty()) {
		normal = Get<Texture>("myopic:default/normal");
	}
	else {
		normal = GetOrCreateTexture(def.normal);
	}

	resource_ptr<Shader> shader = Get<Shader>(def.shader.ToString());

	return materials.insert( name.ToString(), std::make_unique<Material>(def.colour, def.specular, def.fresnelRange, shader.get(), tex.get(), normal.get()));
}

void Assets::RemoveUnreferenced()
{
	materials.remove_unreferenced();
	meshes.remove_unreferenced();
	textures.remove_unreferenced();
	shaders.remove_unreferenced();

}



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



void Assets::CreateDefaults()
{

	VertexLayout UIlayout{
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
	Register("myopic:gui", gfx.LoadShader("shaders/GUI.vs.cso", "shaders/GUI.ps.cso", &UIlayout)).get()->set_intrinsic(true);

	Register("myopic:standard_lit", gfx.LoadShader("shaders/Main.vs.cso", "shaders/Main.ps.cso", nullptr)).get()->set_intrinsic(true);



	//textures
	auto missing_tex_raw = generate_missing_tex();
	auto missing_tex = gfx.DXCreateTexture((void*)missing_tex_raw.data(), missing_tex_width, missing_tex_width, missing_tex_width * 4, DXGI_FORMAT_B8G8R8A8_UNORM);

	const uint8_t normalData[4]{ //1x1 normal map pointing out
	255,128,128,255
	};

	auto missing_normal = gfx.DXCreateTexture((void*)normalData, 1, 1, 4, DXGI_FORMAT_B8G8R8A8_UNORM);


	const uint8_t blankData[4]{ //1x1 normal map pointing out
	255,255,255,255
	};

	auto blank_texture = gfx.DXCreateTexture((void*)blankData, 1, 1, 4, DXGI_FORMAT_B8G8R8A8_UNORM);


	//this feels a bit hacky but hey, it works
	Register("myopic:error/texture", std::move(missing_tex)).get()->set_intrinsic(true);
	Register("myopic:default/normal", std::move(missing_normal)).get()->set_intrinsic(true);
	Register("myopic:default/texture", std::move(blank_texture)).get()->set_intrinsic(true);

	textures.set_err_path("myopic:error/texture");


}
