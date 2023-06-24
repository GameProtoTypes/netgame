cmake_minimum_required(VERSION 3.25)


project(MinerTycoon CXX)

set(CMAKE_CXX_STANDARD 20)



################################################################################
# Source groups
################################################################################
set(Header_Files
    "GameGraphics.h"
    "GameCompute.h"
    "GameNetworking.h" 
    "GE.Basic.Macro.defines.h"
    "GE.Includes.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
    "GameGraphics.cpp"
    "GameCompute.cpp"
    "GameNetworking.cpp"
)
source_group("Source Files" FILES ${Source_Files})


set(Shader_Files

    "shaders/vertPeep.glsl"
    "shaders/vertShader.glsl"
    "shaders/vertMapTile.glsl"

    "shaders/geomMapTile.glsl"

    "shaders/fragPeep.glsl"
    "shaders/fragShader.glsl"
    "shaders/fragMapTile.glsl"

)
source_group("Shaders" FILES ${Shader_Files})

set(OPENCL_Source_Files
)
#source_group("CL Source Files" FILES ${OPENCL_Source_Files})

set(OPENCL_Header_Files
)
#source_group("CL Header Files" FILES ${OPENCL_Header_Files})


set(Game_Source_Files
    #"gameCode/Game.cpp"
)
#source_group("Game Source Files" FILES ${Game_Source_Files})

set(Game_Header_Files
)
#source_group("Game Header Files" FILES ${Game_Header_Files})

set(Game_Module_Files
    "gameCode/Game.ixx"
    "gameCode/Game.Graphics.ixx"
    "gameCode/Game.cpp"
)
source_group("Game Module Files" FILES ${Game_Module_Files})

set(Engine_Module_Files 
"GE.Basic-FixedPoint.cpp"
"GE.Basic.ixx"

"GE.Basic-Types.ixx"
"GE.Basic-FixedPoint.ixx"
"GE.Basic-FixedPoint_InterestingMath.ixx"

"GE.ImGui.cpp"
"GE.ImGui.ixx"
)
source_group("Engine Module Files" FILES ${Engine_Module_Files})






set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    
    ${OPENCL_Source_Files}
    ${OPENCL_Header_Files}

    ${Engine_Module_Files}

    ${Shader_Files}

    ${Game_Source_Files}
    ${Game_Header_Files}
    ${Game_Module_Files}
)
set(COMPILE_FILES
    ${Header_Files}
    ${Source_Files}

    ${Shader_Files}

    ${Engine_Module_Files}

    ${Game_Source_Files}
    ${Game_Header_Files}
    ${Game_Module_Files}
)

################################################################################
# Target
################################################################################


add_executable(${PROJECT_NAME} ${COMPILE_FILES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE "gameCode" )
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLM_ENABLE_EXPERIMENTAL" "IMGUI_DISABLE_OBSOLETE_FUNCTIONS")




################################################################################
# Include directories
################################################################################

# target_include_directories(${PROJECT_NAME} PUBLIC
#     "${CMAKE_CURRENT_SOURCE_DIR}/../glew-2.1.0/include/GL;"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../GLFW;"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../SDL2-2.0.22/include;"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../stb;"
#     "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../glm-master/glm"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../SLikeNet/Source/include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../angelscriptsdk/angelscript/include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../angelscriptsdk/angelscript/source"  

# )

    
set_property(SOURCE ${OPENCL_Source_Files} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")




################################################################################
# Dependencies
################################################################################

find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED GL Shaders Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui Glm)



set(ADDITIONAL_LIBRARY_DEPENDENCIES
#    "OpenCL;"
    "Corrade::Main;"
    "Magnum::Application;"
    "Magnum::GL;"
    "Magnum::Magnum;"
    "Magnum::Shaders;"
    "MagnumIntegration::ImGui;"
    "MagnumIntegration::Glm;"
    "SLikeNet;"
    "gcem;"

)


target_compile_options(${PROJECT_NAME} PRIVATE 
#"/showIncludes" 
"$<$<CONFIG:RELEASE>: /O2>"
"$<$<CONFIG:RELEASE>: /fp:fast>"
"$<$<CONFIG:RELEASE>: /arch:AVX512>")

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")



#Copy Dependancies
add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders)
       

add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/angelScript
        ${CMAKE_CURRENT_BINARY_DIR}/angelScript)

#compression binaries
add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/windows_binaries
        ${CMAKE_CURRENT_BINARY_DIR}/windows_binaries)
        



add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/TileSet.png
            ${CMAKE_CURRENT_BINARY_DIR}/TileSet.png)

add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/16x16_sm_ascii.png
            ${CMAKE_CURRENT_BINARY_DIR}/16x16_sm_ascii.png)





















