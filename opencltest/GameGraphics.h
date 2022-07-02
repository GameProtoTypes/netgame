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
	

    std::vector<GEShaderProgram*> shaderProgramList;
    std::vector<GEShader*> shaderList;
	GEShaderProgram* pPeepShadProgram;
	GEShaderProgram* pBasicShadProgram;
	
	
	uint32_t quadVAO, quadVBO;
	uint32_t instanceVBO;        
	
	glm::vec2* worldPositions =  nullptr;
    glm::vec3* colors = nullptr;

};

