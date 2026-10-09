#pragma once

class TextureManager {

};
class MeshManager {

};
class ShaderManager {

};
class ModelManager {

};


class Game
{
public:
	Game() {

	}
	~Game() {

	}

	void Init();

	void Shutdown();

	static Game& get() {
		static Game sGame;
		return sGame;
	}
private:

};

