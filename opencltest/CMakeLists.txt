project(opencltest)
cmake_minimum_required(VERSION 3.20)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "peep.h"
    "GEShaderProgram.h"
    "GEShader.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
    "GEShaderProgram.cpp"
    "GEShader.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
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
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../glm-master/glm"
)





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
    "imgui"
)

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")


target_link_directories(${PROJECT_NAME} PRIVATE
    "C:/Users/casht/repos/opencltest/SDL2-2.0.22/lib/x64"
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/lib/x64"
    "C:/Users/casht/repos/opencltest/glew-2.1.0/lib/Release/x64"
    "C:/Users/casht/repos/opencltest/GLFW/lib"
)


