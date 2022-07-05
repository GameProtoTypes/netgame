#pragma once
#include <string>
#include <iostream>

#include <SDL.h>
#include "SDL_opengl.h"

#ifdef WIN32
#include <windows.h>
#endif


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


#include "GEShader.h"
#include "GEShaderProgram.h"


//graphics resources/buffers access
class GameGraphics
{
public:

	GameGraphics();
	~GameGraphics();


	//Screen dimension constants
	const int32_t SCREEN_WIDTH = 1024;
	const int32_t SCREEN_HEIGHT = 768;

	void BeginDraw();

	void Swap();

	SDL_GLContext sdlGLContext;

	SDL_Window* gWindow = nullptr;

	SDL_Renderer* gRenderer = nullptr;

	ImGuiContext* imguicontext;

	//Loads individual image as texture
	SDL_Texture* loadTexture(std::string path);
	

    std::vector<std::shared_ptr<GEShaderProgram>> shaderProgramList;
    std::vector<std::shared_ptr<GEShader>> shaderList;

	std::shared_ptr<GEShaderProgram> pPeepShadProgram;
	int peepInstanceSIZE = 0;



	std::shared_ptr<GEShaderProgram> pBasicShadProgram;
	std::shared_ptr<GEShaderProgram> pTileShadProgram;
	
	uint32_t quadVAO, quadVBO;
	uint32_t peepInstanceVBO;        
	uint32_t mapTileInstanceVBO;
};

