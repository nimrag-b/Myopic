#include <iostream>
#include <SDL3/SDL.h>

#define WIN32_LEAN_AND_MEAN
//#include <d3d11.h>
#include <d3d11_4.h>
#include "Graphics.h"
#include "Assets.h"

#include <atomic>

#include "Model.h"

#include <thread>

int avg_fps_v = 0;

int avg_fps() {
	return avg_fps_v;
}

std::unique_ptr<Model> mod;
void load(Assets& assets) {
	assets.LoadModel("paimon");
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	mod = assets.LoadModel("2B_Rip_v1_6_R");
}

int myopic() {

	Graphics gfx;
	gfx.Initialise();
	Assets assets(gfx);

	std::thread load_thread(load, std::ref(assets));


	


	assets.RemoveUnreferenced();

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
		XMFLOAT3 day = { 1.0f, 0.9f, 0.9f };
		XMFLOAT3 night = { 49/255.0f, 49/255.0f, 163/255.0f };
		XMFLOAT3 res;
		XMStoreFloat3(&res,XMVectorLerp(XMLoadFloat3(&day), XMLoadFloat3(&night), abs(sin(heavy_rot/10.0f))));

		gfx.UpdateSun({ 0,1,1 }, res);

		gfx.BeginRender();

		heavy_rot += (float)delta;

		XMFLOAT4X4 transform;
		XMMATRIX xmm = ::XMMatrixRotationX(XM_PIDIV2) * ::XMMatrixRotationY(heavy_rot) * DirectX::XMMatrixScaling(5, 5, 5) * DirectX::XMMatrixTranslation(0, -5, 10);

		//store the model matrix in the correct spot
		DirectX::XMStoreFloat4x4(
			&transform,
			XMMatrixTranspose(xmm)
		);

		gfx.RenderModel(mod.get(), transform);

		//gfx.EnableAlphaBlend();

		//gfx.SetShader(guiShader.get());
		//gfx.DXBindD2dTexture();
		//gfx.RenderMesh(guiQuad.get());

		//gfx.DisableAlphaBlend();



		gfx.EndRender();
	}


	gfx.Destroy();

	load_thread.join();

	return 0;
}

#ifdef _CONSOLE
int main() {

	myopic();
}
#else

#define UNREFERENCED_PARAMETER(p) (p)

//#define CREATECONSOLE

extern "C" int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

#ifdef CREATECONSOLE
	AllocConsole();
	FILE* dummy;
	freopen_s(&dummy, "CONIN$", "rb", stdin);
	freopen_s(&dummy, "CONOUT$", "wb", stdout);
	freopen_s(&dummy, "CONOUT$", "wb", stderr);
#endif


	myopic();
}
#endif