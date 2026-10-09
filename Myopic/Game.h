#pragma once
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
		static Game gGame;
		return gGame;
	}
private:

};

