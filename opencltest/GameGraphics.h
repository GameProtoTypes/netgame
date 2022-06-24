#pragma once
#include <string>
#include <iostream>

#include <SDL.h>
#include "SDL_opengl.h"


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
	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;

	void BeginDraw();

	void Swap();

	SDL_Window* gWindow = nullptr;

	SDL_Renderer* gRenderer = nullptr;

	ImGuiContext* imguicontext;

	//Loads individual image as texture
	SDL_Texture* loadTexture(std::string path);
	

    std::vector<GEShaderProgram*> shaderProgramList;
    std::vector<GEShader*> shaderList;
	GEShaderProgram* pPeepShadProgram;
	GEShaderProgram* pBasicShadProgram;
	
	
	unsigned int quadVAO, quadVBO;
	unsigned int instanceVBO;        
	
	glm::vec2* worldPositions =  nullptr;
    glm::vec3* colors = nullptr;

};

