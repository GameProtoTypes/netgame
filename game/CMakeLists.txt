project(MinerTycoon CXX)
cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)

#include("cxx-modules-rules.cmake")



##################################################################
#LLVM
##################################################################
#find_package(LLVM REQUIRED CONFIG)

#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


#separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
#add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components
#llvm_map_components_to_libnames(llvm_libs support core irreader clang)








################################################################################
# Source groups
################################################################################
set(Header_Files
    
    "GEShaderProgram.h"
    "GEShader.h"
    "GameGraphics.h"
    "GameCompute.h"
    "GameNetworking.h" 
    "GE.Basic.Macro.defines.h"
    "GE.Includes.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
    "GEShaderProgram.cpp"
    "GEShader.cpp"
    "GameGraphics.cpp"
    "GameCompute.cpp"
    "GameNetworking.cpp"
)
source_group("Source Files" FILES ${Source_Files})


set(Shader_Files

    "vertPeep.glsl"
    "vertShader.glsl"
    "vertMapTile.glsl"

    "geomMapTile.glsl"

    "fragPeep.glsl"
    "fragShader.glsl"
    "fragMapTile.glsl"

)
source_group("Shader Files" FILES ${Shader_Files})



set(OPENCL_Source_Files
)
source_group("CL Source Files" FILES ${OPENCL_Source_Files})

set(OPENCL_Header_Files
)
source_group("CL Header Files" FILES ${OPENCL_Header_Files})


set(Game_Source_Files
    #"gameCode/Game.cpp"
)
source_group("Game Source Files" FILES ${Game_Source_Files})

set(Game_Header_Files

)
source_group("Game Header Files" FILES ${Game_Header_Files})

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






set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    
    ${OPENCL_Source_Files}
    ${OPENCL_Header_Files}

    ${Engine_Module_Files}

    ${Game_Source_Files}
    ${Game_Header_Files}
    ${Game_Module_Files}
)
set(COMPILE_FILES
    ${Header_Files}
    ${Source_Files}

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
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLM_ENABLE_EXPERIMENTAL")
#target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../stb/stb_image.h")
# target_sources(${PROJECT_NAME}
#   PRIVATE
#     FILE_SET CXX_MODULES
#       TYPE CXX_MODULES
#       FILES
#       ${Game_Module_Files})







################################################################################
# Include directories
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../glew-2.1.0/include/GL;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../GLFW;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SDL2-2.0.22/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../stb;"
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../glm-master/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SLikeNet/Source/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../angelscriptsdk/angelscript/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../angelscriptsdk/angelscript/source"  

)

    
set_property(SOURCE ${OPENCL_Source_Files} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")




################################################################################
# Dependencies
################################################################################



#separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
#add_definitions(${LLVM_DEFINITIONS_LIST})




set(ADDITIONAL_LIBRARY_DEPENDENCIES
#    "OpenCL;"
    "SDL2;"
    "SDL2main;"
    "Opengl32;"
    "glew32;"
    "glfw3;"
    "imgui;"
    "SLikeNet;"
    "angelscript;"
    "gcem"
)


target_compile_options(${PROJECT_NAME} PRIVATE 
#"/showIncludes" 
"$<$<CONFIG:RELEASE>: /O2>"
"$<$<CONFIG:RELEASE>: /fp:fast>"
"$<$<CONFIG:RELEASE>: /arch:AVX512>")

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")



target_link_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/SDL2-2.0.22/lib/x64"
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/lib/x64"
    "${CMAKE_SOURCE_DIR}/glew-2.1.0/lib/Release/x64"
    "${CMAKE_SOURCE_DIR}/GLFW/lib"
)


#COPY ThINGS INTO BINARY DIRECTORY
add_custom_command(
        TARGET MinerTycoon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/SDL2-2.0.22/lib/x64/SDL2.dll
                ${CMAKE_CURRENT_BINARY_DIR}/Debug/SDL2.dll)


add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/glew-2.1.0/bin/Release/x64/glew32.dll
            ${CMAKE_CURRENT_BINARY_DIR}/Debug/glew32.dll)

add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders)
       

add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/angelScript
        ${CMAKE_CURRENT_BINARY_DIR}/Debug/angelScript)

#compression binaries
add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/windows_binaries
        ${CMAKE_CURRENT_BINARY_DIR}/Debug/windows_binaries)
        



add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/TileSet.png
            ${CMAKE_CURRENT_BINARY_DIR}/Debug/TileSet.png)

add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/16x16_sm_ascii.png
            ${CMAKE_CURRENT_BINARY_DIR}/Debug/16x16_sm_ascii.png)



#COPY THINGS FROM RUNTIME BACK INTO SOURCE (FOR EDITOR NESSECITIES AND SUCH STRANGE THINGS)
#add_custom_command(
#    TARGET MinerTycoon POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#            ${CMAKE_CURRENT_BINARY_DIR}/Debug/openCL/dynamicDefines.h
#            ${CMAKE_CURRENT_SOURCE_DIR}/openCL/dynamicDefines.h)

























