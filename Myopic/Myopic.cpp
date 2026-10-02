#include <iostream>
#include <SDL3/SDL.h>

#define WIN32_LEAN_AND_MEAN
//#include <d3d11.h>
#include <d3d11_4.h>
#include "Graphics.h"
#include "Assets.h"

#include <atomic>

#include "Model.h"


struct VertexPositionColor
{
	POSITION position;
	COLOUR color;
};

struct GUIVertex {
	POSITION position;
	TEXCOORD texCoord;
};

VertexLayout basiclayout{
{
	"POSITION",
	0,
	DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
	0,
	offsetof(VertexPositionColor, position),
	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
	0
},
{
	"COLOR",
	0,
	DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
	0,
	offsetof(VertexPositionColor, color),
	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
	0
},
};



std::vector<VertexPositionColor> vertices = {
{ POSITION{  0.0f,  0.5f, 0.0f }, COLOUR{ 1, 0, 0 } },
{ POSITION{  0.5f, -0.5f, 0.0f }, COLOUR{ 0, 1, 0 } },
{ POSITION{ -0.5f, -0.5f, 0.0f }, COLOUR{ 0, 0, 1 } },
};
std::vector<uint32_t> indices = {
	0, 1, 2,   // first triangle
};

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

int avg_fps_v = 0;

int avg_fps() {
	return avg_fps_v;
}


int myopic() {

	Graphics gfx;
	gfx.Initialise();
	Assets assets(gfx);

	assets.Register("gui", gfx.LoadShader("shaders/GUI.vs.cso", "shaders/GUI.ps.cso", &UIlayout));

	assets.Register("basic", gfx.LoadShader("shaders/Basic.vs.cso", "shaders/Basic.ps.cso", &basiclayout));
	assets.Register("myopic:standard_lit", gfx.LoadShader("shaders/Main.vs.cso", "shaders/Main.ps.cso", nullptr));


	assets.LoadModel("2B_Rip_v1_6_R");

	assets.CreateMaterial("2BRip_Boots", MaterialDef{
		.shader = "standard_lit",
		.texture = "asdf2b0_tex06"
		});
	assets.CreateMaterial("2BRip_Cloth", MaterialDef{
	.shader = "standard_lit",
	.texture = "asdf2b0_tex06"
		});
	assets.CreateMaterial("2BRip_Eye", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex04"
		});
	assets.CreateMaterial("2BRip_Eyelash", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex09"
		});
	assets.CreateMaterial("2BRip_EyeShadow", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex04"
		});
	assets.CreateMaterial("2BRip_Hair1", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex09",
.normal = "asdf2b0_tex10"
		});
	assets.CreateMaterial("2BRip_Hair2", MaterialDef{
.shader = "standard_lit",
		});
	assets.CreateMaterial("2BRip_Satin", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex09"
		});
	assets.CreateMaterial("2BRip_Skin", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex04"
		});
	assets.CreateMaterial("2BRip_Tights", MaterialDef{
.shader = "standard_lit",
.texture = "asdf2b0_tex00"
		});


	//auto rawmesh = gfx.CreateMesh(vertices);
	//gfx.SetSubMesh(rawmesh, 0, indices);
	//assets.Register("Quad", std::move(rawmesh));

	//auto guiraw = gfx.CreateMesh(GUIvertices);
	//gfx.SetSubMesh(guiraw, 0, GUIindices);
	//assets.Register("GUIQuad", std::move(guiraw));


	//assets.CreateMaterial("myopic:heavymat", MaterialDef{
	//	.shader = "standard_lit",
	//	.texture = "hvyweapon_red",
	//	.normal = "hvyweapon_normal",
	//	.specular = 0.05f
	//	});

	//assets.CreateMaterial("myopic:heavyhead", MaterialDef{
	//	.shader = "standard_lit",
	//	.texture = "heavy_head",
	//	.specular = 0.05f
	//	});

	//assets.LoadMesh("myopic:heavy");

	//assets.LoadTexture("myopic:heavy_head");
	//assets.LoadTexture("myopic:hvyweapon_normal");
	//assets.LoadTexture("myopic:hvyweapon_red");


	//auto heavy = assets.Get<Mesh>("myopic:heavy");

	//auto heavy_mat = assets.Get<Material>("myopic:heavymat");
	//auto heavy_head = assets.Get<Material>("myopic:heavyhead");

	//Model heavy_mod( heavy, std::vector<resource_ptr<Material>>{heavy_mat, heavy_head, heavy_mat, heavy_head, heavy_mat} );

	auto mesh = assets.Get<Mesh>("myopic:2B_Rip_v1_6_R");

	Model mod_2b(mesh, std::vector<resource_ptr<Material>>{
			assets.Get<Material>("myopic:2BRip_Cloth"),
			assets.Get<Material>("myopic:2BRip_Satin"),
			assets.Get<Material>("myopic:2BRip_Tights"),
			assets.Get<Material>("myopic:2BRip_Hair1"),
			assets.Get<Material>("myopic:2BRip_Hair2"),
			assets.Get<Material>("myopic:2BRip_Eyelash"),
			assets.Get<Material>("myopic:2BRip_Skin"),
			assets.Get<Material>("myopic:2BRip_EyeShadow"),
			assets.Get<Material>("myopic:2BRip_Boots"),
			assets.Get<Material>("myopic:2BRip_Eye"),
	});

	float heavy_rot = 0;

	Uint64 LAST = SDL_GetTicksNS();
	double delta = 0;

	double t = 0;

	int frames = 0;

#define FPS_AVG_COUNT 100
	double fps_avg[FPS_AVG_COUNT] = { 0 };
	int fps_avg_ptr = 0;

	bool should_quit = false;
	while (!should_quit) {

		Uint64 CUR = SDL_GetTicksNS();

		delta = static_cast<double>(CUR - LAST) / SDL_NS_PER_SECOND;

		LAST = CUR;

		t += delta;
		frames++;

		fps_avg[fps_avg_ptr] = delta;
		fps_avg_ptr = (fps_avg_ptr + 1) % FPS_AVG_COUNT;





		if (t > 1) {

			t = 0;
			frames = 0;

			double fps_avg_v = 0;
			for (size_t i = 0; i < FPS_AVG_COUNT; i++)
			{
				fps_avg_v += fps_avg[i];
			}

			fps_avg_v /= FPS_AVG_COUNT;

			int fps_avg_vi = 1 / fps_avg_v;

			avg_fps_v = fps_avg_vi;

			std::cout << avg_fps_v << "ms" << std::endl;

			//std::string title = "(avg " + std::to_string(fps_avg_vi) + ")";
			//SDL_SetWindowTitle(gfx.SdlWindow(), title.c_str());
		}



		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type)
			{
			case SDL_EVENT_QUIT:
				should_quit = true;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				gfx.OnResize();
				break;
			case SDL_EVENT_KEY_DOWN:
				if (e.key.key == SDLK_F) {
					gfx.SetFullscreen(!gfx.GetFullscreen());
				}
				break;
			default:
				break;
			}
		}

		//gfx.GetCamera().Rotate(1.0f * (float)delta, 0);

		gfx.BeginRender();

		heavy_rot += (float)delta;

		XMFLOAT4X4 transform;
		XMMATRIX xmm = ::XMMatrixRotationX(XM_PIDIV2) * ::XMMatrixRotationY(heavy_rot) * DirectX::XMMatrixScaling(5, 5, 5) * DirectX::XMMatrixTranslation(0, -5, 10);

		//store the model matrix in the correct spot
		DirectX::XMStoreFloat4x4(
			&transform,
			XMMatrixTranspose(xmm)
		);

		gfx.RenderModel(&mod_2b, transform);

		//gfx.EnableAlphaBlend();

		//gfx.SetShader(guiShader.get());
		//gfx.DXBindD2dTexture();
		//gfx.RenderMesh(guiQuad.get());

		//gfx.DisableAlphaBlend();



		gfx.EndRender();
	}


	gfx.Destroy();

	return 0;
}

#ifdef _CONSOLE
int main() {

	myopic();
}
#else

#define UNREFERENCED_PARAMETER(p) (p)

extern "C" int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	myopic();
}
#endif