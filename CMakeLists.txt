cmake_minimum_required(VERSION 3.25)

include(fetchcontent)

project(game)


#if multiconfiguration restrict project to the CMAKE_BUILD_TYPE
if(WIN32 OR APPLE)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        SET(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        SET(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
    endif()

endif()




add_subdirectory(SLikeNet)
#add_subdirectory(angelscriptsdk/angelscript/projects/cmake)
add_subdirectory(gcem)




set(SPRINTFS_FUNCTION_EXISTS true)
set(VSNPRINTFS_FUNCTION_EXISTS true)

#Bring in sdl2
fetchcontent_declare(
    sdl2
    GIT_REPOSITORY
    https://github.com/libsdl-org/SDL
    GIT_TAG SDL2
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)
fetchcontent_populate(sdl2)
fetchcontent_makeavailable(sdl2)


add_subdirectory(${sdl2_SOURCE_DIR})


#Bring in magnum
fetchcontent_declare(
    magnum
    GIT_REPOSITORY
    https://github.com/mosra/magnum
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)
fetchcontent_populate(magnum)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${magnum_SOURCE_DIR}/modules")
fetchcontent_makeavailable(magnum)


#Bring in corrade
fetchcontent_declare(
    corrade
    GIT_REPOSITORY
    https://github.com/mosra/corrade.git
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)

fetchcontent_populate(corrade)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${corrade_SOURCE_DIR}/modules")
fetchcontent_makeavailable(corrade)


#Bring in magnum_integrations
fetchcontent_declare(
    magnum_integrations
    GIT_REPOSITORY
    https://github.com/mosra/magnum-integration
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)

fetchcontent_populate(magnum_integrations)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${magnum_integrations_SOURCE_DIR}/modules")
fetchcontent_makeavailable(magnum_integrations)

fetchcontent_declare(
    imgui
    GIT_REPOSITORY
        https://github.com/ocornut/imgui
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)

fetchcontent_populate(imgui)


fetchcontent_declare(
    implot
    GIT_REPOSITORY
        https://github.com/epezent/implot
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)

fetchcontent_populate(implot)




set(IMGUI_DIR ${imgui_SOURCE_DIR})



fetchcontent_declare(
    glm
    GIT_REPOSITORY
        https://github.com/TrevorCash/glm
    GIT_TAG master
    GIT_PROGRESS
    TRUE
    GIT_SHALLOW
    TRUE
)

fetchcontent_populate(glm)

#set GLM_INCLUDE_DIR so magnum can find it.
set(GLM_INCLUDE_DIR ${glm_SOURCE_DIR} CACHE STRING "" FORCE)



set(MAGNUM_WITH_IMGUI ON CACHE BOOL "" FORCE)
set(MAGNUM_WITH_GLM ON CACHE BOOL "" FORCE)


#set(MAGNUM_INCLUDE_INSTALL_DIR "magnum")
#set(CORRADE_INCLUDE_INSTALL_DIR "corrade")

set(WITH_SDL2APPLICATION ON CACHE BOOL "" FORCE)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "")

add_subdirectory(${corrade_SOURCE_DIR})
add_subdirectory(${magnum_SOURCE_DIR})
add_subdirectory(${magnum_integrations_SOURCE_DIR})



add_subdirectory(game)