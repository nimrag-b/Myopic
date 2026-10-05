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
#include <sstream>

bool should_quit = false;

int avg_fps_v = 0;

int avg_fps() {
	return avg_fps_v;
}

bool input_set = false;
std::string input;

void read_input() {
	while (!should_quit) {
		if (input_set) {
			Sleep(5);
			continue;
		}
		
		std::getline(std::cin, input);
		input_set = true;
	}
}


std::unique_ptr<Model> mod;
void load(Assets& assets) {


	assets.LoadModel("paimon");
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
	mod = assets.LoadModel("2B_Rip_v1_6_R");
}

bool keys[SDL_SCANCODE_COUNT];

int game(Graphics& gfx, Assets& assets) {

	assets.LoadCubemap("SkyboxBasic");

	auto texCube = assets.GetOrCreateTexture("SkyboxBasic");

	assets.RemoveUnreferenced();

	float heavy_rot = 0;




	Uint64 LAST = SDL_GetTicksNS();
	double delta = 0;

	double t = 0;

	int frames = 0;

#define FPS_AVG_COUNT 100
	double fps_avg[FPS_AVG_COUNT] = { 0 };
	int fps_avg_ptr = 0;


	while (!should_quit) {


		if (input_set) {
			std::string in = input;
			input_set = false;

			std::stringstream ss(in);
			std::vector<std::string> args;
			std::string s;
			while (std::getline(ss, s, ' ')) {
				args.push_back(s);
			}
			if (args.size() != 0) {
				if (args[0] == "wireframe") {
					if (args.size() == 2) {
						int v = std::stoi(args[1]);
						if (v == 0) {
							gfx.DisableWireframe();
							std::cout << "Disabled Wireframe" << std::endl;
						}
						else if (v == 1) {
							gfx.EnableWireframe();
							std::cout << "Enabled Wireframe" << std::endl;
						}
					}
				}
			}


		}

		Uint64 CUR = SDL_GetTicksNS();

		delta = static_cast<double>(CUR - LAST) / SDL_NS_PER_SECOND;

		LAST = CUR;

		t += delta;
		frames++;

		fps_avg[fps_avg_ptr] = delta;
		fps_avg_ptr = (fps_avg_ptr + 1) % FPS_AVG_COUNT;


		double fps_avg_v = 0;
		for (size_t i = 0; i < FPS_AVG_COUNT; i++)
		{
			fps_avg_v += fps_avg[i];
		}

		fps_avg_v /= FPS_AVG_COUNT;

		avg_fps_v = 1 / fps_avg_v;

		//avg_fps_v = 1.0 / delta;


		if (t > 1) {

			avg_fps_v = frames;

			//std::cout << frames << "fps" << std::endl;

			t = 0;
			frames = 0;





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
			case SDL_EVENT_KEY_UP:
				keys[e.key.scancode] = false;
				break;
			case SDL_EVENT_KEY_DOWN:

				keys[e.key.scancode] = true;

				if (e.key.key == SDLK_F) {
					gfx.SetFullscreen(!gfx.GetFullscreen());
				}
				else if (e.key.key == SDLK_V) {
					gfx.SetVsync(!gfx.Vsync());
				}
				else if (e.key.key == SDLK_LEFTBRACKET) {
					SDL_SetWindowRelativeMouseMode(gfx.SdlWindow(), true);
				}
				else if (e.key.key == SDLK_RIGHTBRACKET) {
					SDL_SetWindowRelativeMouseMode(gfx.SdlWindow(), false);
				}

				break;
			case SDL_EVENT_MOUSE_MOTION:
				gfx.GetCamera().Rotate(e.motion.xrel * 0.002f, -e.motion.yrel * 0.002f);
				
				break;
			default:
				break;
			}
		}


		if (keys[SDL_SCANCODE_W]) {

			Camera& cam = gfx.GetCamera();

			XMFLOAT3 forward = cam.Forward();
			XMVECTOR vec = 50 * delta * XMLoadFloat3(&forward);
			XMStoreFloat3(&forward, vec);
			cam.Move(forward);
			//std::cout << cam.Position().x << ", " << cam.Position().y << ", " << cam.Position().z << std::endl;
		}
		if (keys[SDL_SCANCODE_S]) {
			Camera& cam = gfx.GetCamera();

			XMFLOAT3 forward = cam.Forward();
			XMVECTOR vec = 50 * -delta * XMLoadFloat3(&forward);
			XMStoreFloat3(&forward, vec);
			cam.Move(forward);
			//std::cout << cam.Position().x << ", " << cam.Position().y << ", " << cam.Position().z << std::endl;
		}

		if (keys[SDL_SCANCODE_A]) {
			Camera& cam = gfx.GetCamera();

			XMFLOAT3 right = cam.Right();
			XMVECTOR vec = 50 * -delta * XMLoadFloat3(&right);
			XMStoreFloat3(&right, vec);
			cam.Move(right);
			//std::cout << cam.Position().x << ", " << cam.Position().y << ", " << cam.Position().z << std::endl;
		}
		if (keys[SDL_SCANCODE_D]) {
			Camera& cam = gfx.GetCamera();

			XMFLOAT3 right = cam.Right();
			XMVECTOR vec = 50 * delta * XMLoadFloat3(&right);
			XMStoreFloat3(&right, vec);
			cam.Move(right);
			//std::cout << cam.Position().x << ", " << cam.Position().y << ", " << cam.Position().z << std::endl;
		}


		//gfx.GetCamera().Rotate(1.0f * (float)delta, 0);

		XMFLOAT3 sunPos = { 1,0,0 };
		XMFLOAT3 axis = { 0,0,1 };
		XMStoreFloat3(&sunPos, XMVector3Rotate(XMLoadFloat3(&sunPos), XMQuaternionRotationAxis(XMLoadFloat3(&axis), heavy_rot)));
		//std::cout << sunPos.x << ", " << sunPos.y << ", " << sunPos.z << std::endl;
		//std::cout << sunPos.x << std::endl;


		XMFLOAT3 day = { 1.0f, 0.9f, 0.9f };
		XMFLOAT3 night = { 49 / 255.0f, 49 / 255.0f, 163 / 255.0f };
		XMFLOAT3 res;
		XMStoreFloat3(&res, XMVectorLerp(XMLoadFloat3(&day), XMLoadFloat3(&night), (sunPos.x + 1)/2.0f));

		gfx.UpdateSun(sunPos, res);

		gfx.BeginRender();


		gfx.RenderSkybox(texCube.get());


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

	return 0;
}

int myopic() {


	Graphics gfx;
	gfx.Initialise();
	Assets assets(gfx);

	std::thread load_thread(load, std::ref(assets));

	std::thread input_thread(read_input);

	game(gfx, assets);

	mod.reset();

	gfx.Destroy();

	load_thread.join();
	input_thread.join();

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