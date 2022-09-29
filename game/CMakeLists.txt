project(MinerTycoon)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)



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
    "openCL/clGame.c"
)
source_group("CL Source Files" FILES ${OPENCL_Source_Files})

set(OPENCL_Header_Files
    "openCL/peep.h"
    "openCL/PeepFuncs.h"
    "openCL/cpu_gpu_structs.h"

    "openCL/gpuvectortypes.h"
    "openCL/fixedpoint_opencl.h"
    "openCL/perlincl.h"
    "openCL/randomcl.h"
    "openCL/clLimits.h"
    "openCL/cl_type_glue.h"
)
source_group("CL Header Files" FILES ${OPENCL_Header_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${OPENCL_Source_Files}
    ${OPENCL_Header_Files}
)
set(COMPILE_FILES
    ${Header_Files}
    ${Source_Files}
)


################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${COMPILE_FILES})









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
    "OpenCL;"
    "SDL2;"
    "SDL2main;"
    "Opengl32;"
    "glew32;"
    "glfw3;"
    "imgui;"
    "SLikeNet;"
    "angelscript"
)


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
                ${CMAKE_CURRENT_SOURCE_DIR}/openCL
                ${CMAKE_CURRENT_BINARY_DIR}/Debug/openCL)
        
add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/angelScript
        ${CMAKE_CURRENT_BINARY_DIR}/Debug/angelScript)

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
add_custom_command(
    TARGET MinerTycoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/Debug/openCL/dynamicDefines.h
            ${CMAKE_CURRENT_SOURCE_DIR}/openCL/dynamicDefines.h)

























