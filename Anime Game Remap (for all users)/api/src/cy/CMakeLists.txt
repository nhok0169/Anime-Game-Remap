cmake_minimum_required(VERSION 3.21)
project(FixRaidenBoss2Cy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(PYTHON_DEV_MODE "python_dev")
set(CIBUILDWHEEL_MODE "cibuildwheel")

# BUILD_MODE: [python_dev|cibuildwheel]
set(BUILD_MODE PYTHON_DEV_MODE CACHE STRING "What mode to use to build the binaries")

# -------------------------------------------------
# Python
# -------------------------------------------------

if ("${BUILD_MODE}" STREQUAL "${CIBUILDWHEEL_MODE}")
    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
else()
    find_package(Python REQUIRED COMPONENTS Interpreter Development)
endif()

# -------------------------------------------------
# NumPy
# -------------------------------------------------

execute_process(
    COMMAND ${Python_EXECUTABLE} -c
    "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "NumPy include dir: ${NUMPY_INCLUDE_DIR}")

# -------------------------------------------------
# Cython modules
# -------------------------------------------------

execute_process(
    COMMAND ${Python_EXECUTABLE} -c
    "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX') or '')"
    OUTPUT_VARIABLE PY_EXT_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

function(add_cython_module name pyx_file)

    # ALWAYS resolve relative paths safely
    get_filename_component(pyx_abs ${pyx_file} ABSOLUTE)

    if(NOT EXISTS ${pyx_abs})
        message(FATAL_ERROR "Cython file not found: ${pyx_abs}")
    endif()

    set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/cython")
    set(gen_cpp "${gen_dir}/${name}.cpp")

    # Generate C++ from .pyx
    add_custom_command(
        OUTPUT ${gen_cpp}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${gen_dir}

        COMMAND ${Python_EXECUTABLE}
            -m cython
            --cplus
            -3
            -I ${NUMPY_INCLUDE_DIR}
            --module-name ${name}
            -o ${gen_cpp}
            ${pyx_abs}

        DEPENDS ${pyx_abs}

        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

        VERBATIM
    )

    # Build extension module
    add_library(${name} MODULE ${gen_cpp})

    target_include_directories(${name} PRIVATE
        ${Python_INCLUDE_DIRS}
        ${NUMPY_INCLUDE_DIR}
    )

    if ("${BUILD_MODE}" STREQUAL "${CIBUILDWHEEL_MODE}")
        target_link_libraries(${name} PRIVATE Python::Module)
    else()
        target_link_libraries(${name} PRIVATE Python::Python)
    endif()

    set_target_properties(${name} PROPERTIES
        PREFIX ""
        SUFFIX "${PY_EXT_SUFFIX}"
        OUTPUT_NAME "${name}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cython"
    )

    # install into package
    install(TARGETS ${name}
        LIBRARY DESTINATION FixRaidenBoss2
        RUNTIME DESTINATION FixRaidenBoss2
        ARCHIVE DESTINATION FixRaidenBoss2
    )

endfunction()

# -------------------------------------------------
# Modules
# -------------------------------------------------

add_cython_module(CyDictTools src/tools/DictTools.pyx)
add_cython_module(CyListTools src/tools/ListTools.pyx)
add_cython_module(CyHashTools src/tools/HashTools.pyx)
add_cython_module(CyAlgo src/tools/Algo.pyx)