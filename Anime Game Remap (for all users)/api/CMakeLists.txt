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

# -----------------------------------
# MSVC: drop unreferenced inline functions from object files (/Zc:inline)
# -----------------------------------

# Without /Zc:inline, MSVC emits a COMDAT for every inline function a translation unit sees, used or
#   not -- and with pybind11 and this project's header-only templates that is most of each object
#   file. The linker throws them away again, but only after every one has been written, archived
#   into AGRemapCore.lib and read back. It is the standard-conforming behaviour (on by default in
#   Visual Studio projects, off for a plain cl command line, which is what CMake generates).
#
# Measured 2026-09-16 on a 6-core / 16GB laptop, full `core` build, same sources:
#
#                                        without        with
#     AGRemapCore.lib                    678 MB         227 MB
#     all object files                   ~2.2 GB        0.94 GB
#     one core .cpp -> rebuilt .pyd      20.5-36s       8.4s
#     one binding .cpp -> rebuilt .pyd   32-37s         21.3s
#     full build                         1316s          1248s
#
#   Compile time per file barely moves; the win is the archive and the link. The module's bound
#   surface was compared name-by-name and docstring-by-docstring and is identical. Code that
#   relied on the extra copies (an inline function declared but not defined where it is used)
#   would fail to LINK, not misbehave.
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:inline>)
endif()

add_subdirectory(src/cpp/core)

if(NOT "${BUILD_MODE}" STREQUAL "${CORESDK_MODE}")
    add_subdirectory(src/cpp/py)
    add_subdirectory(src/cy)
endif()