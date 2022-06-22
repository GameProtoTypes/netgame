#pragma once
class GameGraphics
{
public:

	GameGraphics();
	~GameGraphics();


	void Draw();


	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;

	//The window renderer
	SDL_Renderer* gRenderer = NULL;

};

