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



#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include <gtx/string_cast.hpp>

#include "GameGPUCompute.h"
GameGraphics::GameGraphics(GameGPUCompute* gameCompute)
{
    this->gameCompute = gameCompute;


}


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
        gWindow = SDL_CreateWindow("Dwarf Carts", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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


    std::shared_ptr<GEShader> pVertShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertShader.shad");
    std::shared_ptr<GEShader> pFragShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragShader.shad");
    std::shared_ptr<GEShader> pVertPeepShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertPeep.shad");
    std::shared_ptr<GEShader> pFragPeepShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragPeep.shad");
    std::shared_ptr<GEShader> pVertMapTileShad = std::make_shared<GEShader>(GL_VERTEX_SHADER, "shaders/vertMapTile.shad");
    std::shared_ptr<GEShader> pFragMapTileShad = std::make_shared<GEShader>(GL_FRAGMENT_SHADER, "shaders/fragMapTile.shad");
    std::shared_ptr<GEShader> pGeomMapTileShad = std::make_shared<GEShader>(GL_GEOMETRY_SHADER, "shaders/geomMapTile.shad");



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

    GL_HOST_ERROR_CHECK()


    for (auto shader : shaderProgramList)
    {
        shader->Link();
    }


    GL_HOST_ERROR_CHECK()


        float quadVertices[] = {
        // positions    
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,


        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    float quadUVs[] = {
        // positions    
        0.0f,  1 / 16.0f,
        0.0f, 0.0f,
        1 / 16.0f, 0.0f,

        0.0f,  1 / 16.0f,
        1 / 16.0f,  0.0f,
        1 / 16.0f,  1 / 16.0f
    };
    for (int i = 0; i < 6; i++)
    {
        quadUVs[i * 2 + 0] += 0.0f;
        quadUVs[i * 2 + 1] += 2 / 16.0f;
    }

    //Make VAO's

    GL_HOST_ERROR_CHECK()


    //map
    std::shared_ptr<cl_uint> mapStartData(new cl_uint[(gameCompute->mapDim * gameCompute->mapDim)]);
    for (int i = 0; i < (gameCompute->mapDim * gameCompute->mapDim); i++)
    {
        mapStartData.get()[i] = 0;
    }
    //primary map
    glGenVertexArrays(1, &mapTile1VAO);
    glBindVertexArray(mapTile1VAO);

        GL_HOST_ERROR_CHECK()

        GL_HOST_ERROR_CHECK()
        glGenBuffers(1, &mapTile1VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mapTile1VBO);
        glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uchar), mapStartData.get(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);



        glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(cl_uchar), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &mapTile1AttrVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mapTile1AttrVBO);
        glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uint), mapStartData.get(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);

        glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cl_uint), (void*)0);


        glGenBuffers(1, &mapTile1OtherAttrVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mapTile1OtherAttrVBO);
        glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uint), mapStartData.get(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(2);

        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(cl_uint), (void*)0);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    GL_HOST_ERROR_CHECK()

    //overlay map
    glGenVertexArrays(1, &mapTile2VAO);
    glBindVertexArray(mapTile2VAO);

    GL_HOST_ERROR_CHECK()

        GL_HOST_ERROR_CHECK()
        glGenBuffers(1, &mapTile2VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mapTile2VBO);
    glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uchar), mapStartData.get(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(cl_uchar), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mapTile2AttrVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mapTile2AttrVBO);
    glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uint), mapStartData.get(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(cl_uint), (void*)0);


    glGenBuffers(1, &mapTile2OtherAttrVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mapTile2OtherAttrVBO);
    glBufferData(GL_ARRAY_BUFFER, (gameCompute->mapDim * gameCompute->mapDim) * sizeof(cl_uint), mapStartData.get(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(cl_uint), (void*)0);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    GL_HOST_ERROR_CHECK()



        //peeps
        glGenVertexArrays(1, &peepVAO);
    glBindVertexArray(peepVAO);

    glGenBuffers(1, &peepQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, peepQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenBuffers(1, &peepQuadUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, peepQuadUVVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadUVs), quadUVs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // also set instance data
    peepInstanceSIZE = sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(float);//size for each peep
    glGenBuffers(1, &peepInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, peepInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, peepInstanceSIZE * gameCompute->maxPeeps, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, peepInstanceSIZE, (void*)0);//position
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, peepInstanceSIZE, (void*)sizeof(glm::vec2));//color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, peepInstanceSIZE, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));//angle


    glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(4, 1); // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(5, 1); // tell OpenGL this is an instanced vertex attribute.

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




    GL_HOST_ERROR_CHECK()





    //particles

    //set different image
    for (int i = 0; i < 6; i++)
    {
        quadUVs[i * 2 + 0] += 1 / 16.0f;
        quadUVs[i * 2 + 1] += 0;
    }

    glGenVertexArrays(1, &particleVAO);
    glBindVertexArray(particleVAO);

    glGenBuffers(1, &particleQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenBuffers(1, &particleQuadUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleQuadUVVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadUVs), quadUVs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // also set instance data
    particleInstanceSIZE = sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(float);//size for each particle
    glGenBuffers(1, &particleInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, particleInstanceSIZE* gameCompute->maxParticles, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, particleInstanceSIZE, (void*)0);//position
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, particleInstanceSIZE, (void*)sizeof(glm::vec2));//color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, particleInstanceSIZE, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));//angle


    glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(4, 1); // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(5, 1); // tell OpenGL this is an instanced vertex attribute.

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




    GL_HOST_ERROR_CHECK()








        glm::ivec2 dims;
    int chIFile;
    int desiredCh = 4;
    stbi_uc* stbimg = stbi_load("TileSet.png", &dims.x, &dims.y, &chIFile, desiredCh);

    if (chIFile != desiredCh)
        assert(0);

    //map textures
    glGenTextures(1, &mapTileTexId);
    glBindTexture(GL_TEXTURE_2D, mapTileTexId);



    int Mode = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, dims.x, dims.y, 0, Mode, GL_UNSIGNED_BYTE, stbimg);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);





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
