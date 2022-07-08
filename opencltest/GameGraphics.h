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

#define GL_HOST_ERROR_CHECK() {GLenum err = glGetError();  if(err != 0){printf("[GRAPHICS] GLERROR: %d", int(err)); assert(0);}}

//graphics resources/buffers access
class GameGraphics
{
public:

	GameGraphics();
	~GameGraphics();


	//Screen dimension constants
	const int32_t SCREEN_WIDTH = 1024;
	const int32_t SCREEN_HEIGHT = 768;

	struct RenderClientState {
		int32_t mousex = 0;
		int32_t mousey = 0;
		int32_t mouse_dragBeginx = 0;
		int32_t mouse_dragBeginy = 0;
		int32_t mousescroll = 0;
		int32_t clicked = 0;

		int32_t mousePrimaryDown = 0;
		int32_t mousePrimaryPressed = 0;
		int32_t mousePrimaryReleased = 0;

		int32_t mouseSecondaryDown = 0;
		int32_t mouseSecondaryPressed = 0;
		int32_t mouseSecondaryReleased = 0;

		float viewX = 0.0f;
		float viewY = 0.0f;
		float view_beginX = 0.0f;
		float view_beginY = 0.0f;
		float viewScale = 1.0f;

		int viewZIdx =0 ;

		glm::vec2 viewFrameDelta = glm::vec2();

		glm::vec2 worldCameraPos = glm::vec2();
	};
	RenderClientState renderClientState;

	void BeginDraw();

	void Swap();

	SDL_GLContext sdlGLContext;

	SDL_Window* gWindow = nullptr;

	SDL_Renderer* gRenderer = nullptr;

	ImGuiContext* imguicontext;

	//Loads individual image as texture
	SDL_Texture* loadTexture(std::string path);
	

    std::vector<std::shared_ptr<GEShaderProgram>> shaderProgramList;

	std::shared_ptr<GEShaderProgram> pPeepShadProgram;
	std::shared_ptr<GEShaderProgram> pMapTileShadProgram;



	std::shared_ptr<GEShaderProgram> pBasicShadProgram;
	std::shared_ptr<GEShaderProgram> pTileShadProgram;
	
	uint32_t peepVAO, peepQuadVBO, peepInstanceVBO;     
	int peepInstanceSIZE = 0;


	uint32_t mapTileVAO, mapTileVBO;	
	int mapTileInstanceSIZE = 0;
	GLuint mapTileTexId = 0;


};

