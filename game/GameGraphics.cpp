#include "glew.h"


#include "GameGraphics.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


#include "glfw3native.h"

#include <SDL.h>
#include "SDL_opengl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "implot.h"



#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include <gtx/string_cast.hpp>

#include "GameCompute.h"

GameGraphics::GameGraphics(GameCompute* gameCompute)
{
    this->gameCompute = gameCompute;
}

//__declspec(dllexport) DWORD NvOptimusEnablement = 0;
//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

void GameGraphics::Init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }
        if (!SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "1"))
        {
            printf("Warning: SDL_HINT_FORCE_RAISEWINDOW not enabled!");
        }



        //Create window
        gWindow = SDL_CreateWindow("Astroid Miner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }


        sdlGLContext = SDL_GL_CreateContext(gWindow);
        std::cout << " SDL GL CONTEXT ERRORS: " << SDL_GetError() << std::endl;
        //swap buffer at the monitors rate
        SDL_GL_SetSwapInterval(1);


        //GLEW is an OpenGL Loading Library used to reach GL functions
        //Sets all functions available

        glewInit();


    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    imguicontext = ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font1 = io.Fonts->AddFontDefault();
    // Setup Platform/Renderer bindings
    // window is the SDL_Window*
    // context is the SDL_GLContext
    ImGui_ImplSDL2_InitForOpenGL(gWindow, imguicontext);
    ImGui_ImplOpenGL3_Init();





    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion =
        glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    printf("GL Vendor    : %s\n", vendor);
    printf("GL Renderer  : %s\n", renderer);
    printf("GL Version (string)  : %s\n", version);
    printf("GL Version (integer) : %d.%d\n", major, minor);
    printf("GLSL Version : %s\n", glslVersion);


    std::shared_ptr<GEShader> pVertShad        = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertShader.glsl");
    std::shared_ptr<GEShader> pFragShad        = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragShader.glsl");
    std::shared_ptr<GEShader> pVertPeepShad    = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertPeep.glsl");
    std::shared_ptr<GEShader> pFragPeepShad    = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragPeep.glsl");
    std::shared_ptr<GEShader> pVertMapTileShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertMapTile.glsl");
    std::shared_ptr<GEShader> pFragMapTileShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragMapTile.glsl");
    std::shared_ptr<GEShader> pGeomMapTileShad = std::make_shared<GEShader>(GL_GEOMETRY_SHADER, "shaders/geomMapTile.glsl");
    std::shared_ptr<GEShader> pGUIVertShad     = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/guiVertShader.glsl");
    std::shared_ptr<GEShader> pGUIFragShad     = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/guiFragShader.glsl");


    //create programs to use those shaders.
    pBasicShadProgram = std::make_shared<GEShaderProgram>();
    pBasicShadProgram->AttachShader(pVertShad);
    pBasicShadProgram->AttachShader(pFragShad);
    shaderProgramList.push_back(pBasicShadProgram);



    pPeepShadProgram = std::make_shared<GEShaderProgram>();
    pPeepShadProgram->AttachShader(pVertPeepShad);
    pPeepShadProgram->AttachShader(pFragPeepShad);
    shaderProgramList.push_back(pPeepShadProgram);


    pParticleShadProgram = std::make_shared<GEShaderProgram>();
    pParticleShadProgram->AttachShader(pVertPeepShad);
    pParticleShadProgram->AttachShader(pFragPeepShad);
    shaderProgramList.push_back(pParticleShadProgram);


    pMapTileShadProgram = std::make_shared<GEShaderProgram>();
    pMapTileShadProgram->AttachShader(pVertMapTileShad);
    pMapTileShadProgram->AttachShader(pGeomMapTileShad);
    pMapTileShadProgram->AttachShader(pFragMapTileShad);
    shaderProgramList.push_back(pMapTileShadProgram);


    pGuiShadProgram = std::make_shared<GEShaderProgram>();
    pGuiShadProgram->AttachShader(pGUIVertShad);
    pGuiShadProgram->AttachShader(pGUIFragShad);
    shaderProgramList.push_back(pGuiShadProgram);


    GL_HOST_ERROR_CHECK()


    for (auto shader : shaderProgramList)
    {
        shader->Link();
    }


    GL_HOST_ERROR_CHECK()





    glFinish();

}

GameGraphics::~GameGraphics()
{

    //Destroy window	
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();




    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

}

void GameGraphics::BeginDraw()
{
            //Clear screen
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(gWindow);
            ImGui::NewFrame();

}

void GameGraphics::Swap()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //Update screen
    SDL_GL_SwapWindow(gWindow);

}
