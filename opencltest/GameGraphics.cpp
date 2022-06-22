#include "GameGraphics.h"

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


        SDL_GL_CreateContext(gWindow);
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


}

GameGraphics::~GameGraphics()
{

    //Destroy window	
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();


}

void GameGraphics::Draw()
{
}
