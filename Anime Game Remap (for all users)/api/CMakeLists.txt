cmake_minimum_required(VERSION 3.18)
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

find_package(Z3 CONFIG REQUIRED)

add_subdirectory(src/cpp/core)

if(NOT "${BUILD_MODE}" STREQUAL "${CORESDK_MODE}")
    add_subdirectory(src/cpp/py)
    add_subdirectory(src/cy)
endif()