#include "glew.h"


#include "GameGraphics.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


#include "glfw3native.h"


#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include <gtx/string_cast.hpp>

#include "peep.h"




GameGraphics::GameGraphics()
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

        //Create window
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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



    std::shared_ptr<GEShader> pVertShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "vertPeep.shad");
    std::shared_ptr<GEShader> pFragShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "fragPeep.shad");


    shaderList.push_back(pVertShad);
    shaderList.push_back(pFragShad);

    //create programs to use those shaders.
    pPeepShadProgram = std::make_shared<GEShaderProgram>();
    pPeepShadProgram->AttachShader(pVertShad);
    pPeepShadProgram->AttachShader(pFragShad);

    shaderProgramList.push_back(pPeepShadProgram);


    pVertShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "vertShader.shad");
    pFragShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "fragShader.shad");
    shaderList.push_back(pVertShad);
    shaderList.push_back(pFragShad);

    //create programs to use those shaders.
    pBasicShadProgram = std::make_shared<GEShaderProgram>();
    pBasicShadProgram->AttachShader(pVertShad);
    pBasicShadProgram->AttachShader(pFragShad);

    shaderProgramList.push_back(pBasicShadProgram);


    glBindAttribLocation(pPeepShadProgram->ProgramID(), 0, "VertexPosition");
    glBindAttribLocation(pPeepShadProgram->ProgramID(), 1, "VertexColor");

    glBindAttribLocation(pBasicShadProgram->ProgramID(), 0, "VertexPosition");
    glBindAttribLocation(pBasicShadProgram->ProgramID(), 1, "VertexColor");


    pPeepShadProgram->Link();
    pBasicShadProgram->Link();


    // store instance data in an array buffer
    // --------------------------------------
    
    glGenBuffers(1, &peepInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, peepInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec2) + sizeof(glm::vec3)) * MAX_PEEPS, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float quadVertices[] = {
        // positions    
        -1.0f,  1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // also set instance data
    peepInstanceSIZE = sizeof(glm::vec2) + sizeof(glm::vec3);//size for each peep
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, peepInstanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, peepInstanceSIZE, (void*)0);//position
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, peepInstanceSIZE, (void*)sizeof(glm::vec2));//color
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.





    //map

    glGenBuffers(1, &mapTileInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mapTileInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec2) + sizeof(glm::vec3))* SQRT_MAPSIZE*SQRT_MAPSIZE, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);













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
