project(MinerTycoon)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
################################################################################
# Source groups
################################################################################
set(Header_Files
    "peep.h"
    "GEShaderProgram.h"
    "GEShader.h"
    "GameGraphics.h"
    "clLimits.h"
    "cl_type_glue.h"
    "GameGPUCompute.h"
    "GameNetworking.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
    "GEShaderProgram.cpp"
    "GEShader.cpp"
    "GameGraphics.cpp"
    "GameGPUCompute.cpp"
    "GameNetworking.cpp"


    "vertPeep.shad"
    "vertShader.shad"
    "vertMapTile.shad"

    "geomMapTile.shad"

    "fragPeep.shad"
    "fragShader.shad"
    "fragMapTile.shad"
)
source_group("Source Files" FILES ${Source_Files})

set(OPENCL_Source_Files

    "clGame.c"
   
)
source_group("CL Source Files" FILES ${OPENCL_Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${OPENCL_Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})


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
)



set_property(SOURCE ${OPENCL_Source_Files} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")




################################################################################
# Dependencies
################################################################################

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "OpenCL;"
    "SDL2;"
    "SDL2main;"
    "Opengl32;"
    "glew32;"
    "glfw3;"
    "imgui;"
    "SLikeNet"
)

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")


target_link_directories(${PROJECT_NAME} PRIVATE
    "C:/Users/casht/repos/opencltest/SDL2-2.0.22/lib/x64"
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/lib/x64"
    "C:/Users/casht/repos/opencltest/glew-2.1.0/lib/Release/x64"
    "C:/Users/casht/repos/opencltest/GLFW/lib"
)


