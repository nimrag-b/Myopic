#include <iostream>
#include <SDL3/SDL.h>

#define WIN32_LEAN_AND_MEAN
//#include <d3d11.h>
#include <d3d11_4.h>
#include "Graphics.h"
#include "Assets.h"

#include <atomic>

int main1(Graphics& gfx) {
	gfx.Initialise();

	VertexLayout layout{ 
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(GVertex, textureCoordinate), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, binormal), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	Assets assets(gfx);
	std::vector<float> vertices = {
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	std::vector<uint32_t> indices = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};


	auto rawmesh = gfx.CreateMesh(vertices);
	gfx.SetSubMesh(rawmesh, 0, indices);
	assets.Register("Quad", std::move(rawmesh));

	assets.Register("StandardLit", gfx.LoadShader("Main.vs.hlsl", "Main.ps.hlsl", nullptr));


	bool should_quit = false;
	while (!should_quit) {

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
			default:
				break;
			}
		}

		gfx.Render();
	}


	gfx.Destroy();

	return 0;
}


int main() {
	{
		Graphics gfx;
		main1(gfx);
	}
}
