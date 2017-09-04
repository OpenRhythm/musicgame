####################################################################
#   Project configuration
####################################################################
cmake_minimum_required(VERSION 3.0)

project (OpenRhythm)
set     (OpenRhythm_VERSION_MAJOR 0)
set     (OpenRhythm_VERSION_MINOR 1)

set(APPLE_APP_NAME "openrhythm.app")
set(LINUX_HOMEPATH ".openrhythm")

set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
####################################################################
#   C++14 / C11 support and flags
####################################################################
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
if(MSVC)
    add_definitions("/MP")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    # Clean unused sections on link
    add_definitions("-ffunction-sections -fdata-sections -Wl,--gc-sections")
    # Colored output
    add_definitions("-fdiagnostics-color")
endif()

####################################################################
#   Platform detection and rules
####################################################################
set(PL_DEFAULT  0)
set(PL_WINDOWS  1)
set(PL_LINUX    2)
set(PL_OSX      3)

set(OSX_APP_BUNDLE 0)

## This is needed when cross-compiling
list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/extern")

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "Building for Windows.")
    set(PLATFORM PL_WINDOWS)

    set(ENV{GLM_ROOT_DIR}    "${PROJECT_SOURCE_DIR}/extern/glm")
    set(ENV{OGG_ROOT}        "${PROJECT_SOURCE_DIR}/extern/ogg")
    set(ENV{SDL2DIR}         "${PROJECT_SOURCE_DIR}/extern/sdl2")
    set(ENV{VORBIS_ROOT}     "${PROJECT_SOURCE_DIR}/extern/vorbis")
    set(ENV{FMT_ROOT}        "${PROJECT_SOURCE_DIR}/extern/fmt")
    set(Gettext_BINARIES     "${PROJECT_SOURCE_DIR}/extern/gettext")
    set(ENV{CUBEB_ROOT}      "${PROJECT_SOURCE_DIR}/extern/cubeb")


    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
    endif()

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Building for Linux.")
    set(PLATFORM PL_LINUX)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Building for Mac OS X.")
    set(PLATFORM PL_OSX)

    if(NOT DEFINED ENV{NO_APP})
        set(OSX_APP_BUNDLE 1)
    endif()

else()
    message(STATUS "Platform not detected!")
    set(PLATFORM PL_DEFAULT)
    # Error and abort here ?
endif()


####################################################################
#   Libraries
####################################################################

include(ExternalProject)

ExternalProject_Add(cubeb
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/cubeb
    INSTALL_DIR "${CMAKE_BINARY_DIR}/cubeb"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release
    )

ExternalProject_Get_Property(cubeb INSTALL_DIR)

# Set package locations manually
set(CUBEB_LIBRARY ${INSTALL_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}cubeb${CMAKE_STATIC_LIBRARY_SUFFIX})
set(CUBEB_INCLUDE_DIR "${INSTALL_DIR}/include/")

ExternalProject_Add(soundtouchc
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/soundtouch-c
    INSTALL_DIR "${CMAKE_BINARY_DIR}/soundtouch-c"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release
    )

ExternalProject_Get_Property(soundtouchc INSTALL_DIR)

# Set package locations manually
set(SOUNDTOUCH_C_LIBRARY ${INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}soundtouchc${CMAKE_SHARED_LIBRARY_SUFFIX})
message(STATUS ${SOUNDTOUCH_C_LIBRARY})
set(SOUNDTOUCH_C_INCLUDE_DIR "${INSTALL_DIR}/include")
message(STATUS ${SOUNDTOUCH_C_INCLUDE_DIR})

find_package(Gettext)
find_package(GLM        REQUIRED)
find_package(OggVorbis  REQUIRED)
find_package(OpenGL     REQUIRED)
find_package(SDL2       REQUIRED)
find_package(fmt        REQUIRED)

set(LIBRARIES
    ${CMAKE_DL_LIBS}
    ${OGG_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    ${VORBIS_LIBRARY}
    ${VORBISFILE_LIBRARY}
    ${CUBEB_LIBRARY}
    ${SOUNDTOUCH_C_LIBRARY}
    ${FMT_LIBRARY}
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(LIBRARIES ${LIBRARIES} winmm avrt)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")

    find_package(ALSA REQUIRED)
    set(LIBRARIES ${LIBRARIES}
        ${ALSA_LIBRARIES}
    )

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")

    set(LIBRARIES ${LIBRARIES}
    "-framework AudioUnit"
    "-framework CoreAudio")

    #RPATH Behavior modified from:
    #https://github.com/russelldub/OpenRhythm.git
    #Not sure if it will work correctly on install

    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    SET(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/lib;${CMAKE_BINARY_DIR}/soundtouch-c")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/soundtouch-c")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    #SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    ENDIF("${isSystemDir}" STREQUAL "-1")

endif()

# These includes are defined a system headers so any warnings within
# don't get shown during compilation.
include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/extern/glad/include
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include
    ${CMAKE_SOURCE_DIR}/extern/stb
    ${FREETYPE_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OGG_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}
    ${CUBEB_INCLUDE_DIR}
    ${SOUNDTOUCH_C_INCLUDE_DIR}
    ${FMT_INCLUDE_DIR}
)

if(INC_PATH)
    string(REPLACE ":" ";" _INC_PATH ${INC_PATH})
    foreach(PATH ${_INC_PATH})
        include_directories(SYSTEM ${PATH})
    endforeach()
endif()

if(GETTEXT_FOUND)
    set(TRANSLATE_ENABLED "true")
    message("Translation enabled")
else()
    set(TRANSLATE_ENABLED "false")
    message("Translation disabled")
endif()


####################################################################
#   Source
####################################################################
configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/src/config.hpp"
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/vorbissource.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/cubeboutput.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/streams.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/mixer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/timestretch.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/configuration/parameter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/batch.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/parseutils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/smf.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stringutils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.hpp
)
set(CORE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/vorbissource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/cubeboutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/mixer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio/timestretch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/batch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/smf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stringutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/src/glad.c
)

set(GAME_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/song.hpp
)
set(GAME_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/song.cpp
)

set(ALL_SOURCE
    ${CORE_SOURCE} ${CORE_HEADERS} ${GAME_SOURCE} ${GAME_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/audiotests.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/core/audio
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/core/renderer
    ${PROJECT_SOURCE_DIR}/src/game
    ${PROJECT_SOURCE_DIR}/src
)

# To create tabs in VisualStudio
source_group("src\\core"  FILES ${CORE_SOURCE}  ${CORE_HEADERS})
source_group("src\\game"  FILES ${GAME_SOURCE}  ${GAME_HEADERS})

add_library(ORCore-obj OBJECT ${CORE_SOURCE})

add_dependencies(ORCore-obj cubeb)
add_dependencies(ORCore-obj soundtouchc)


add_executable(openrhythm $<TARGET_OBJECTS:ORCore-obj>
                          ${GAME_SOURCE}
                          ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(openrhythm ${LIBRARIES})
install(TARGETS       openrhythm DESTINATION bin)

set(BINARY_DATA_DIR ${CMAKE_BINARY_DIR}/data)
set(SOURCE_DATA_DIR ${CMAKE_SOURCE_DIR}/data)

add_custom_command(TARGET openrhythm POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                ${BINARY_DATA_DIR})
add_custom_command(TARGET openrhythm POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${SOURCE_DATA_DIR} $<TARGET_FILE_DIR:openrhythm>/data/)


if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(TARGET openrhythm POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${CMAKE_SOURCE_DIR}/extern/sdl2/lib/SDL2.dll" $<TARGET_FILE_DIR:openrhythm>/SDL2.dll)
endif()


if(OSX_APP_BUNDLE)
    set_target_properties(openrhythm PROPERTIES MACOSX_BUNDLE TRUE)
endif()

####################################################################
#   Localization
####################################################################
if(GETTEXT_FOUND)
    set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
    include(GettextTranslate)
    GettextTranslateTargets()

    add_subdirectory(po)
endif()

####################################################################
#   Test binaries
####################################################################


add_executable(audiotest
    $<TARGET_OBJECTS:ORCore-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/audiotests.cpp)

target_link_libraries(audiotest ${LIBRARIES})

add_executable(midiplayer
    $<TARGET_OBJECTS:ORCore-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/midiplayer/midimain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/midiplayer/MidiPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/midiplayer/midi_song.cpp)

target_link_libraries(midiplayer ${LIBRARIES})

add_executable(uitest
    $<TARGET_OBJECTS:ORCore-obj>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/uitest.cpp)

target_link_libraries(uitest ${LIBRARIES})


####################################################################
#   Documentation
####################################################################
find_package(Cldoc)

if(Cldoc_FOUND)
    # Adds "doc" and "doc/fast" make targets
    add_cldoc_documentation(
        FILES ${ALL_SOURCE}
        FLAGS
            -I${SDL2_INCLUDE_DIR}/.
            -I${SDL2Mixer_INCLUDE_DIRS}/.
            -I${LibVorbis_INCLUDE_DIRS}/.
            -I${PortAudio_INCLUDE_DIRS}/.
            -I${CMAKE_SOURCE_DIR}/extern/stb
            -I${CMAKE_SOURCE_DIR}/extern/glad/include
            -I${CMAKE_SOURCE_DIR}/extern/spdlog/include
            -I"${CMAKE_CURRENT_SOURCE_DIR}/src"
            -I"${CMAKE_CURRENT_SOURCE_DIR}/src/core"
            -I"${CMAKE_CURRENT_SOURCE_DIR}/src/core/audio"
            -I"${CMAKE_CURRENT_SOURCE_DIR}/src/engine"
            -I"${CMAKE_CURRENT_SOURCE_DIR}/src/game"
            -std=gnu++14
        OUTPUT "docs"
        LANGUAGE c++
    )
endif()
