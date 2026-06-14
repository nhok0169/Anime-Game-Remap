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
)

add_library(AGRemapCore::AGRemapCore ALIAS AGRemapCore)

target_include_directories(AGRemapCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(AGRemapCore
    PRIVATE
        z3::libz3
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