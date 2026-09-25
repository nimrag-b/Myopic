#include <iostream>
#include <SDL3/SDL.h>

#define WIN32_LEAN_AND_MEAN
//#include <d3d11.h>
#include <d3d11_4.h>
#include "Graphics.h"
#include "Assets.h"

#include <atomic>


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



int myopic() {
	Graphics gfx;
	gfx.Initialise();
	Assets assets(gfx);



	auto rawmesh = gfx.CreateMesh(vertices);
	gfx.SetSubMesh(rawmesh, 0, indices);
	assets.Register("Quad", std::move(rawmesh));

	auto guiraw = gfx.CreateMesh(GUIvertices);
	gfx.SetSubMesh(guiraw, 0, GUIindices);
	assets.Register("GUIQuad", std::move(guiraw));

	assets.Register("GUI", gfx.LoadShader("shaders/GUI.vs.cso", "shaders/GUI.ps.cso", &UIlayout));

	assets.Register("Basic", gfx.LoadShader("shaders/Basic.vs.cso", "shaders/Basic.ps.cso", &basiclayout));
	assets.Register("StandardLit", gfx.LoadShader("shaders/Main.vs.cso", "shaders/Main.ps.cso", nullptr));

	auto basicShader = assets.Get<Shader>("Basic");
	auto quad = assets.Get<Mesh>("Quad");

	auto guiShader = assets.Get<Shader>("GUI");
	auto guiQuad = assets.Get<Mesh>("GUIQuad");


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

			std::string title = "(avg " + std::to_string(fps_avg_vi) + ")";
			SDL_SetWindowTitle(gfx.SdlWindow(), title.c_str());
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

		gfx.BeginRender();


		gfx.SetShader(basicShader.get());
		gfx.RenderMesh(quad.get());


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