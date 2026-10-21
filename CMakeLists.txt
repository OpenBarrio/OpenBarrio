cmake_minimum_required(VERSION 3.28)
project(openbarrio_core LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_options(/permissive- /utf-8 /Zc:__cplusplus /wd4324)

# Reproducible output: no timestamps, and no absolute source paths in __FILE__.
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" OPENBARRIO_SOURCE_DIR_NATIVE)
add_compile_options(/Brepro "/d1trimfile:${OPENBARRIO_SOURCE_DIR_NATIVE}")
string(APPEND CMAKE_STATIC_LINKER_FLAGS " /Brepro")

# EA's open-source EASTL and EABase, vendored unmodified (see THIRD_PARTY.md). Don't upgrade: the game's container
# layouts depend on this exact version.
file(GLOB EASTL_SOURCES ${CMAKE_SOURCE_DIR}/third_party/EASTL/source/*.cpp)
add_library(eastl STATIC ${EASTL_SOURCES})
target_include_directories(eastl SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/EASTL/include
    ${CMAKE_SOURCE_DIR}/third_party/EABase/include/Common)
target_compile_definitions(eastl PUBLIC EASTL_NAME_ENABLED=1)  # the game's container layouts carry names
target_compile_options(eastl PRIVATE /W0)

# The rewritten functions. A static library: some functions call code that isn't released yet, so a program that
# links everything can hit unresolved symbols. That's expected for this release.
file(GLOB_RECURSE OPENBARRIO_CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/core/*.cpp)
add_library(openbarrio_core STATIC ${OPENBARRIO_CORE_SOURCES})
target_include_directories(openbarrio_core PUBLIC src/core/include)
target_compile_options(openbarrio_core PRIVATE /W4)
target_link_libraries(openbarrio_core PUBLIC eastl)
