cmake_minimum_required(VERSION 3.21)
project(FixRaidenBoss2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------------
# IMPORTANT: wheel-safe flags
# -----------------------------------

set(PYTHON_DEV_MODE "python_dev")
set(CIBUILDWHEEL_MODE "cibuildwheel")
set(CORESDK_MODE "core_sdk")

# BUILD_MODE: [python_dev|cibuildwheel|coresdk]
set(BUILD_MODE "${PYTHON_DEV_MODE}" CACHE STRING "What mode to use to build the binaries")
set(BUILD_CPP_SDK OFF CACHE BOOL "" FORCE)

set(CMAKE_STRIP "" CACHE FILEPATH "" FORCE)
set(CMAKE_INSTALL_DO_STRIP OFF CACHE BOOL "" FORCE)

if ("${BUILD_MODE}" STREQUAL "${CORESDK_MODE}")
    set(BUILD_CPP_SDK ON)
else()
    set(BUILD_CPP_SDK OFF)
endif()

# -----------------------------------
# Python and Pybind11
# -----------------------------------

if (NOT BUILD_CPP_SDK)
    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

    execute_process(
        COMMAND ${Python_EXECUTABLE} -c
        "import pybind11, os; print(os.path.join(os.path.dirname(pybind11.__file__), 'share', 'cmake', 'pybind11'))"
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    find_package(pybind11 REQUIRED CONFIG)
endif()


# -----------------------------------
# subprojects
# -----------------------------------

# Z3
find_package(Z3 CONFIG REQUIRED)

# uni-algo
add_library(uni-algo INTERFACE)
target_include_directories(uni-algo INTERFACE
    ${CMAKE_SOURCE_DIR}/extern/uni-algo/include
)

# -----------------------------------
# Compiler cache (sccache)
# -----------------------------------

# sccache caches object files keyed on the *content* of the preprocessed source plus the flags,
#   so it does nothing for a first-ever compile and almost everything for a repeated one. What
#   that buys, concretely (measured, one binding source, MSVC):
#
#     plain cl                                   5.8s
#     sccache, cold cache (miss)                 6.2s     ~8% overhead
#     sccache, warm cache (hit)                  0.5s     ~12x
#
#   Crucially the hit survives being in a DIFFERENT CHECKOUT: compiled once through a second path
#   to the same tree, with entirely different -I arguments, the real tree's build still hit
#   (verified against a genuinely empty cache -- an earlier attempt at this test only zeroed the
#   statistics, not the cache, and "proved" it by hitting an entry the previous test had left
#   behind). So this is the one lever here that helps a fresh `git worktree`, which is where the
#   long builds actually are; the precompiled headers do nothing for that case.
#
# IT IS MUTUALLY EXCLUSIVE WITH THE PRECOMPILED HEADERS, which is why turning it on turns those
#   off below. sccache will not cache a compilation that uses one -- `sccache --show-stats`
#   reports it under "Non-cacheable reasons: /Fp", silently, while still paying its own overhead.
#   Leaving both on is the worst of the two.
#
# So: pick per workload. PCH (the default) is better for editing in one tree, where every compile
#   is of something you just changed and can never be a cache hit. sccache is better when whole
#   trees get rebuilt from scratch -- new worktrees, wiped cbuild/, branch switching.
option(AGREMAP_SCCACHE "Route compiles through sccache instead of using precompiled headers" OFF)

if(AGREMAP_SCCACHE)
    find_program(SCCACHE_EXECUTABLE sccache)

    if(NOT SCCACHE_EXECUTABLE)
        message(FATAL_ERROR
            "AGREMAP_SCCACHE=ON but sccache was not found on PATH. "
            "Install it (winget install Mozilla.sccache) or configure with -DAGREMAP_SCCACHE=OFF.")
    endif()

    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")

    # MSVC writes debug info into a shared .pdb, which cannot be cached; sccache refuses those
    #   too. The Release flags this project builds with carry no /Zi, so nothing to do here --
    #   but if a debug configuration is ever added, it needs /Z7 rather than /Zi to stay
    #   cacheable.
    message(STATUS "AGRemap: compiling through sccache (${SCCACHE_EXECUTABLE}); "
                   "precompiled headers disabled, they are not cacheable")
endif()

add_subdirectory(src/cpp/core)

if(NOT "${BUILD_MODE}" STREQUAL "${CORESDK_MODE}")
    add_subdirectory(src/cpp/py)
    add_subdirectory(src/cy)
endif()