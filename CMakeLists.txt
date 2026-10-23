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
string(APPEND CMAKE_EXE_LINKER_FLAGS " /Brepro")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " /Brepro")

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
# /Gy /Gw: function-level linking, so a program takes only the functions it uses (see the test harness below).
target_compile_options(openbarrio_core PRIVATE /W4 /Gy /Gw)
target_link_libraries(openbarrio_core PUBLIC eastl)

# The engine test harness: the rewritten functions inside the running game (32-bit only, like the game).
# openbarrio_launch.exe starts the game with openbarrio_test.dll; see harness/README.txt and docs/engine/.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_library(openbarrio_test SHARED harness/harness_public.cpp)
    target_link_libraries(openbarrio_test PRIVATE openbarrio_core bcrypt)
    target_link_options(openbarrio_test PRIVATE /OPT:REF /OPT:ICF)
    add_executable(openbarrio_launch harness/launcher.cpp)
    add_custom_command(TARGET openbarrio_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/harness/harness_public.ini
                $<TARGET_FILE_DIR:openbarrio_test>/harness_public.ini)
endif()

# The save mover (release 1): 32-bit only, like the game.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # The program and its DLL go into one folder, as they ship.
    set(OPENBARRIO_BIN ${CMAKE_BINARY_DIR}/bin)

    add_library(openbarrio_mover STATIC mover/mover.cpp)
    target_include_directories(openbarrio_mover PUBLIC mover)
    target_compile_options(openbarrio_mover PRIVATE /W4)

    # Only the SpecialDirectory rewrite goes into the DLL, nothing else from the core.
    add_library(openbarrio_redirect SHARED hook/redirect_dll.cpp src/core/io/special_dir.cpp hook/version.rc)
    target_include_directories(openbarrio_redirect PRIVATE src/core/include)
    target_compile_options(openbarrio_redirect PRIVATE /W4)
    set_target_properties(openbarrio_redirect PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OPENBARRIO_BIN})

    add_executable(OpenBarrio WIN32 launcher/main.cpp launcher/game.cpp launcher/version.rc
                   launcher/openbarrio.manifest)
    target_link_libraries(OpenBarrio PRIVATE openbarrio_mover)
    target_compile_options(OpenBarrio PRIVATE /W4)
    set_target_properties(OpenBarrio PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OPENBARRIO_BIN})

    enable_testing()
    add_executable(mover_tests tests/mover/mover_tests.cpp)
    target_link_libraries(mover_tests PRIVATE openbarrio_mover)
    target_compile_options(mover_tests PRIVATE /W4)
    add_test(NAME mover_tests COMMAND mover_tests)
endif()

# Interactive engine Test Lab; release-1 targets above remain unchanged.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_options(openbarrio_test PRIVATE /MAP)
    add_library(testlab_logic STATIC testlab/logic.cpp)
    target_include_directories(testlab_logic PUBLIC testlab)
    target_compile_options(testlab_logic PRIVATE /W4 /WX)
    add_executable(testlab_tests tests/testlab/logic_tests.cpp)
    target_link_libraries(testlab_tests PRIVATE testlab_logic)
    target_compile_options(testlab_tests PRIVATE /W4 /WX)
    add_test(NAME testlab_tests COMMAND testlab_tests)

    add_library(testlab_runtime STATIC testlab/runtime.cpp launcher/game.cpp)
    target_include_directories(testlab_runtime PUBLIC testlab launcher)
    target_link_libraries(testlab_runtime PUBLIC testlab_logic openbarrio_mover dbghelp psapi shell32 ole32 advapi32)
    target_compile_options(testlab_runtime PRIVATE /W4 /WX)
    add_executable(testlab_debug_fixture tests/testlab/debug_fixture.cpp)
    target_compile_options(testlab_debug_fixture PRIVATE /W4 /WX)
    add_executable(testlab_debug_tests tests/testlab/debug_tests.cpp)
    target_link_libraries(testlab_debug_tests PRIVATE testlab_runtime)
    target_compile_options(testlab_debug_tests PRIVATE /W4 /WX)
    add_test(NAME testlab_debugger COMMAND testlab_debug_tests $<TARGET_FILE:testlab_debug_fixture>)
    set_tests_properties(testlab_debugger PROPERTIES TIMEOUT 40)

    add_executable(OpenBarrioTestLab WIN32 testlab/main.cpp testlab/testlab.manifest)
    target_compile_definitions(OpenBarrioTestLab PRIVATE UNICODE _UNICODE)
    target_compile_options(OpenBarrioTestLab PRIVATE /W4 /WX)
    target_link_libraries(OpenBarrioTestLab PRIVATE testlab_runtime comctl32 comdlg32)
    add_dependencies(OpenBarrioTestLab openbarrio_test)
    add_custom_command(TARGET OpenBarrioTestLab POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/docs/engine/HOOKS.csv
            $<TARGET_FILE_DIR:OpenBarrioTestLab>/HOOKS.csv)

    add_executable(testlab_gui_tests tests/testlab/gui_tests.cpp)
    target_compile_options(testlab_gui_tests PRIVATE /W4 /WX)
    add_dependencies(testlab_gui_tests OpenBarrioTestLab)
    add_test(NAME testlab_gui COMMAND testlab_gui_tests $<TARGET_FILE:OpenBarrioTestLab>)
    set_tests_properties(testlab_gui PROPERTIES TIMEOUT 40)
endif()
