cmake_minimum_required(VERSION 3.0)
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)

project(musicgame)

set(PL_WINDOWS 1)
set(PL_LINUX 2)
set(PL_OSX 3)

set(PLATFORM 0)

if(NOT DEFINED(USE_LIB_PNGCPP))
    set(USE_LIB_PNGCPP 0)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")


if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "Building for Windows.")
    set(ENV{SDL2DIR} "${PROJECT_SOURCE_DIR}/extern/sdl2")
    set(ENV{FREETYPE_DIR} "${PROJECT_SOURCE_DIR}/extern/freetype")
    set(ENV{GLM_ROOT_DIR} "${PROJECT_SOURCE_DIR}/extern/glm")
    set(PLATFORM PL_WINDOWS)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Building for Linux.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(PLATFORM PL_LINUX)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Building for Mac OS X.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    set(PLATFORM PL_OSX)

endif()

include(FindBoost)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/src/core)
include_directories(${PROJECT_SOURCE_DIR}/src/engine)
include_directories(${PROJECT_SOURCE_DIR}/src/game)
include_directories(${PROJECT_SOURCE_DIR}/src/gl)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(OSX_APP_BUNDLE 0)

if(NOT DEFINED ENV{NO_APP})
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(OSX_APP_BUNDLE 1)
    endif()
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/src/config.hpp"
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mesh2d.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/vfs.hpp)

set(CORE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mesh2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/vfs.cpp)

if(INC_PATH)
    string(REPLACE ":" ";" _INC_PATH ${INC_PATH})
    foreach(PATH ${_INC_PATH})
        include_directories(${PATH})
    endforeach()
endif()

source_group("src\\core" FILES ${CORE_SOURCE} ${CORE_HEADERS})


set(GAME_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.cpp)
set(GAME_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.hpp)

source_group("src\\game" FILES ${GAME_SOURCE} ${GAME_HEADERS})

set(GL ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/gl_core32.h
       ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/gl_core32.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/gl.hpp)

source_group("src\\gl" FILES ${GL})


set(LIBRARY_SOURCE  ${CMAKE_CURRENT_SOURCE_DIR}/src/library/node.cpp)

set(LIBRARY_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/library/node.hpp)

source_group("src\\library" FILES ${LIBRARY_SOURCE} ${LIBRARY_HEADERS})



include_directories(${CMAKE_SOURCE_DIR}/extern/ttvfs/ttvfs)
include_directories(${CMAKE_SOURCE_DIR}/extern/ttvfs/ttvfs_zip)
include_directories(${CMAKE_SOURCE_DIR}/extern/stb)

if(USE_LIB_PNGCPP)
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/extern/png++)
    source_group("extern\\png++" FILES ${PNGPP_SRC})
endif()

source_group("extern\\stb" FILES ${STB_SRC})

set( SOURCE ${CORE_SOURCE}      ${CORE_HEADERS}
            ${GAME_SOURCE}      ${GAME_HEADERS}
            ${LIBRARY_SOURCE}   ${LIBRARY_HEADERS}
            ${ttvfs_SRC} ${ttvfs_zip_SRC} ${GL})

add_executable(musicgame ${SOURCE})

set(BINARY_DATA_DIR ${CMAKE_BINARY_DIR}/data)
set(SOURCE_DATA_DIR ${CMAKE_SOURCE_DIR}/data)

add_custom_command(TARGET musicgame PRE_BUILD
                  COMMAND ${CMAKE_COMMAND} -E make_directory
                  ${BINARY_DATA_DIR})
add_custom_command(TARGET musicgame PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${SOURCE_DATA_DIR} $<TARGET_FILE_DIR:musicgame>/data/)



include(ExternalProject)
ExternalProject_add(ttvfs
    DOWNLOAD_COMMAND ""
    CMAKE_ARGS CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DTTVFS_BUILD_CFILEAPI=FALSE -DTTVFS_SUPPORT_ZIP=FALSE -DTTVFS_LARGEFILE_SUPPORT=FALSE
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/ttvfs
    INSTALL_COMMAND "")

add_dependencies(musicgame ttvfs)

if(OSX_APP_BUNDLE)
    set_target_properties(musicgame PROPERTIES MACOSX_BUNDLE TRUE)
endif()

ExternalProject_Get_Property(ttvfs BINARY_DIR)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(CMAKE_CONFIGURATION_TYPES MATCHES "Release" AND CMAKE_CONFIGURATION_TYPES MATCHES "Release")
        set(ttvfs_libs ${BINARY_DIR}/ttvfs/Release/ttvfs.lib)
    else()
        set(ttvfs_libs ${BINARY_DIR}/ttvfs/Debug/ttvfs.lib)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin" AND CMAKE_GENERATOR MATCHES "Xcode")
    if(CMAKE_CONFIGURATION_TYPES MATCHES "Release" AND CMAKE_CONFIGURATION_TYPES MATCHES "Release")
        set(ttvfs_libs ${BINARY_DIR}/ttvfs/Release/libttvfs.a)
    else()
        set(ttvfs_libs ${BINARY_DIR}/ttvfs/Debug/libttvfs.a)
    endif()
else()
    set(ttvfs_libs ${BINARY_DIR}/ttvfs/libttvfs.a)
endif()

target_link_libraries(musicgame
    ${ttvfs_libs}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    ${FREETYPE_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    )
