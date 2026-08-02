cmake_minimum_required(VERSION 3.18)
project(AGRemapCore VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# -----------------------------------
# Options
# -----------------------------------

option(BUILD_CPP_SDK "Install C++ SDK (headers + cmake config)" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# -----------------------------------
# Library
# -----------------------------------

add_library(AGRemapCore STATIC
    src/tools/IntTools.cpp
    src/tools/ListTools.cpp
    src/tools/Ranges.cpp
    src/tools/StringHash.cpp
    src/tools/StringTools.cpp
    src/tools/grapheme/GraphemeIterator.cpp
    src/tools/grapheme/GraphemeRange.cpp
    src/tools/parsing/ParseContext.cpp
    src/tools/parsing/BaseTokenizer.cpp
)

add_library(AGRemapCore::AGRemapCore ALIAS AGRemapCore)

target_include_directories(AGRemapCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# -----------------------------------
# UTF8Proc
# -----------------------------------

set(UTF8PROC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8PROC_INSTALL OFF CACHE BOOL "" FORCE)

set(UTF8PROC_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../extern/utf8proc)
set(UTF8PROC_BIN ${CMAKE_BINARY_DIR}/utf8proc)

add_subdirectory(${UTF8PROC_SRC} ${UTF8PROC_BIN})

target_link_libraries(AGRemapCore
    PRIVATE
        z3::libz3
        utf8proc
)

set_target_properties(AGRemapCore PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# -----------------------------------
# Export + Config setup (required for find_package)
# -----------------------------------

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/AGRemapCore)

# Generate AGRemapCoreConfig.cmake
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AGRemapCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

# Optional version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# -----------------------------------
# INSTALL (SDK only)
# -----------------------------------

if(BUILD_CPP_SDK)

    # Library
    install(TARGETS AGRemapCore
        EXPORT AGRemapCoreTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Headers
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Export targets
    install(EXPORT AGRemapCoreTargets
        FILE AGRemapCoreTargets.cmake
        NAMESPACE AGRemapCore::
        DESTINATION ${CONFIG_INSTALL_DIR}
    )

    # Config + version files
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfigVersion.cmake
        DESTINATION ${CONFIG_INSTALL_DIR}
    )

endif()