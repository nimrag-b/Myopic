#include "Assets.h"
#include <fstream>
#include <exception>
#include <array>

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
	std::string pres = vertex.ToString();
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

	std::string path = tag.Filepath("textures", ".png");

	SDL_Surface* surfraw = SDL_LoadSurface(path.c_str());

	SDL_Surface* surf = SDL_ConvertSurface(surfraw, SDL_PIXELFORMAT_BGRA32);

	SDL_DestroySurface(surfraw);

	bool v = SDL_LockSurface(surf);

	auto tex = gfx.DXCreateTexture(surf->pixels, surf->w, surf->h, surf->pitch, DXGI_FORMAT_B8G8R8A8_UNORM);

	SDL_UnlockSurface(surf);

	SDL_DestroySurface(surf);

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
		//return default
	}
	return res;

}

resource_ptr<Mesh> Assets::GetOrCreateMesh(const ResourceLocation& tag)
{
	std::string name = tag.ToString();
	auto& reg = GetRegistry<Mesh>();
	resource_ptr<Mesh> res = reg.get(name);
	if (res == nullptr) {
		LoadMesh(tag);
		res = reg.get(name);
	}

	if (res == nullptr) {
		//return default
	}
	return res;

}

void Assets::LoadMesh(const ResourceLocation& tag)
{
	std::string path = tag.Filepath("meshes", ".naf");
	std::ifstream file(path, std::ios::binary);

	struct alignas(4) naf_header {
		unsigned int magic;
		unsigned int format;
		unsigned int mesh_count;
		unsigned int vertex_count;
	};


	if (file.is_open()) {

		std::vector<char> bytes(
			(std::istreambuf_iterator<char>(file)),
			(std::istreambuf_iterator<char>()));
		size_t ptr = 0;

		naf_header header = *(reinterpret_cast<naf_header*>(&bytes[ptr]));
		ptr += sizeof(naf_header);

		if (header.magic != 0xB33FB33F) throw load_error("File '" + path + "' is not a valid mesh");

		if (header.format != 1) throw load_error("Invalid mesh format for '" + path + "'");


		try {
			GVertex* begin = reinterpret_cast<GVertex*>(&bytes[ptr]);
			std::vector<GVertex> vertices = std::vector<GVertex>(begin, begin + header.vertex_count);
			ptr += header.vertex_count * sizeof(GVertex);

			auto Mesh = gfx.CreateMesh(vertices, header.mesh_count);

			for (size_t i = 0; i < header.mesh_count; i++)
			{
				unsigned int nameLength = *(reinterpret_cast<unsigned int*>(&bytes[ptr]));
				ptr += sizeof(nameLength);
				std::string name(&bytes[ptr], &bytes[ptr] + nameLength);
				ptr += nameLength * sizeof(char);

				unsigned int indexCount = *(reinterpret_cast<unsigned int*>(&bytes[ptr]));
				ptr += sizeof(indexCount);
				uint32_t* ibegin = reinterpret_cast<uint32_t*>(&bytes[ptr]);
				std::vector<uint32_t> indices = std::vector<uint32_t>(ibegin, ibegin + indexCount);
				ptr += indexCount * sizeof(uint32_t);

				gfx.SetSubMesh(Mesh, i, indices);
			}


			Register(tag.ToString(), std::move(Mesh));

		}
		catch (std::exception e) {
			throw load_error("Error in mesh data causing '" + std::string(e.what()) + "'");
		}

		file.close();

		return;
	}

	throw load_error("File '" + path + "' does not exist");
}

void Assets::CreateMaterial(const std::string& name, const MaterialDef& def)
{
	if (materials.contains(name)) {
		throw load_error("Cannot create material '" + name + "' as a material with the same name already exists");
	}

	resource_ptr<Texture> tex = NULL;
	if (def.texture.empty()) {
		tex = Get<Texture>("myopic:error.texture");
	}
	else {
		tex = GetOrCreateTexture(def.texture);
	}
	resource_ptr<Texture> normal = NULL;
	if (def.normal.empty()) {
		normal = Get<Texture>("myopic:error.normal");
	}
	else {
		normal = GetOrCreateTexture(def.normal);
	}

	resource_ptr<Shader> shader = Get<Shader>(def.shader.ToString());

	materials.insert( name,std::make_unique<Material>(def.colour, def.specular, def.fresnelRange, shader.get(), tex.get(), normal.get()));
}



constexpr size_t missing_tex_width = 100;
constexpr size_t missing_tex_block_size = 20;

constexpr size_t missing_tex_bytes = missing_tex_width * missing_tex_width * 4;

void generate_missing_tex(std::array<uint8_t, missing_tex_bytes>& arr) {
	for (size_t i = 0; i < missing_tex_bytes; i += 4)
	{
		if ((i / missing_tex_block_size) % 2) {
			arr[i + 0] = 0;
			arr[i + 1] = 0;
			arr[i + 2] = 0;
			arr[i + 3] = 255;
		}
		else {
			arr[i + 0] = 255;
			arr[i + 1] = 0;
			arr[i + 2] = 255;
			arr[i + 3] = 255;
		}
	}
};


void generate_missing_normal(std::array<uint8_t, missing_tex_bytes>& arr) {
	for (size_t i = 0; i < missing_tex_bytes; i += 4)
	{
		if ((i / missing_tex_block_size) % 2) {
			arr[i + 0] = 0;
			arr[i + 1] = 0;
			arr[i + 2] = 0;
			arr[i + 3] = 255;
		}
		else {
			arr[i + 0] = 255;
			arr[i + 1] = 0;
			arr[i + 2] = 255;
			arr[i + 3] = 255;
		}
	}
};



void Assets::CreateDefaults()
{
	std::array<uint8_t, missing_tex_bytes> missing_tex_raw{};
	generate_missing_tex(missing_tex_raw);
	auto missing_tex = gfx.DXCreateTexture((void*)missing_tex_raw.data(), missing_tex_width, missing_tex_width, missing_tex_width * 4, DXGI_FORMAT_B8G8R8A8_UNORM);

	const uint8_t normalData[4]{ //1x1 normal map pointing out
	128,128,255,255
	};

	auto missing_normal = gfx.DXCreateTexture((void*)normalData, 1, 1, 4, DXGI_FORMAT_B8G8R8A8_UNORM);

	Register("myopic:error.texture", std::move(missing_tex));
	Register("myopic:error.normal", std::move(missing_normal));

	textures.set_err_path("myopic:error.texture");


}
