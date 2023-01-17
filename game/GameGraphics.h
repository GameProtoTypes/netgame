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
class GameCompute;
class GameGraphics
{
public:

	GameGraphics(GameCompute* gameCompute);
	~GameGraphics();

	void Init();

	//Screen dimension constants
	const int32_t SCREEN_WIDTH = 1920;
	const int32_t SCREEN_HEIGHT = 1080;

	struct RenderClientState {
		int32_t mousex = 0;
		int32_t mousey = 0;
		int32_t mouse_dragBeginx = 0;
		int32_t mouse_dragBeginy = 0;
		int32_t mousescroll = 0;
		int32_t mousescroll_1 = 0;
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

		bool waitingMapAction = false;
		bool waitingDelete = false;

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
	std::shared_ptr<GEShaderProgram> pParticleShadProgram;
	std::shared_ptr<GEShaderProgram> pMapTileShadProgram;



	std::shared_ptr<GEShaderProgram> pBasicShadProgram;
	std::shared_ptr<GEShaderProgram> pTileShadProgram;
	std::shared_ptr<GEShaderProgram> pGuiShadProgram;
	
	uint32_t peepVAO, peepQuadVBO, peepQuadUVVBO, peepInstanceVBO;
	int peepInstanceSIZE = 0;

	uint32_t particleVAO, particleQuadVBO, particleQuadUVVBO, particleInstanceVBO;
	int particleInstanceSIZE = 0;

	uint32_t guiRectVAO, guiRectInstanceVBO;

	uint32_t linesVAO, linesVBO;


	uint32_t mapTile1VAO, mapTile1VBO, mapTile1AttrVBO, mapTile1OtherAttrVBO;	
	uint32_t mapTile2VAO, mapTile2VBO, mapTile2AttrVBO, mapTile2OtherAttrVBO;
	int mapTileInstanceSIZE = 0;
	GLuint mapTileTexId = 0;
	GLuint lettersTileTexId = 0;

	float viewScaleInterp = 0.0f;

	GameCompute* gameCompute;
};

