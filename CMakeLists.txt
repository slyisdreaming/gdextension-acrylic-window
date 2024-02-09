cmake_minimum_required(VERSION 3.19)
project(AcrylicWindow LANGUAGES C CXX)

set(GDEXTENSION_NAME acrylic-window)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/demo/addons/${GDEXTENSION_NAME}/bin)

#---------------------------------------------------------------------------
# Fetch godot-cpp.
#---------------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    godot-cpp
    GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
    GIT_TAG godot-4.2-stable
)

FetchContent_MakeAvailable(godot-cpp)

#---------------------------------------------------------------------------
# Add Library.
#---------------------------------------------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


# Add a dynamic library named ${PROJECT_NAME}
add_library(${PROJECT_NAME} SHARED ${SOURCES})

#---------------------------------------------------------------------------
# Configure the OUTPUT_NAME.
#---------------------------------------------------------------------------

# TODO Check this code on other non-Windwows platforms.
# TODO Add support for ARM.

# macos.debug            macos.debug
# macos.release          macos.release
# windows.debug.x86_32   windows.debug.x86_32
# windows.release.x86_32 windows.release.x86_32
# windows.debug.x86_64   windows.debug.x86_64
# windows.release.x86_64 windows.release.x86_64
# linux.debug.x86_64     linux.debug.x86_64
# linux.release.x86_64   linux.release.x86_64
# linux.debug.arm64      linux.debug.arm64
# linux.release.arm64    linux.release.arm64
# linux.debug.rv64       linux.debug.rv64
# linux.release.rv64     linux.release.rv64
# android.debug.x86_64   android.debug.x86_64
# android.release.x86_64 android.release.x86_64
# android.debug.arm64    android.debug.arm64
# android.release.arm64  android.release.arm64

set(OUTPUT_NAME ${GDEXTENSION_NAME})
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Windows>:.windows>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Linux>:.linux>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Android>:.android>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Darwin>:.macos>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<CONFIG:Debug>:.debug>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<CONFIG:Release>:.release>)

if (CMAKE_SIZE_OF_VOID_P EQUAL 4)
    set(OUTPUT_NAME ${OUTPUT_NAME}.x86_32)
else()
    set(OUTPUT_NAME ${OUTPUT_NAME}.x86_64)
endif()

#---------------------------------------------------------------------------
# Setup TARGET.
#---------------------------------------------------------------------------

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PUBLIC godot::cpp)
