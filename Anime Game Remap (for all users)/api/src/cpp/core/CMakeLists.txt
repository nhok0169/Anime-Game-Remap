cmake_minimum_required(VERSION 3.21)
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
    src/constants/GameTypeId.cpp
    src/constants/ModTypeId.cpp
    src/constants/GameType.cpp
    src/constants/IfPredPartType.cpp
    src/constants/GIBuilder.cpp
    src/constants/GlobalIniRemoveBuilders.cpp
    src/data/HashData.cpp
    src/data/IndexData.cpp
    src/data/HashToModObjData.cpp
    src/data/VertexCountData.cpp
    src/data/VGRemapData.cpp
    src/data/ModDataAssets.cpp
    src/data/IniParseBuilderData.cpp
    src/data/IniFixBuilderData.cpp
    src/data/IniRemoveBuilderData.cpp
    src/constants/GlobalIniClassifiers.cpp
    src/model/Version.cpp
    src/model/VersionSet.cpp
    src/tools/Heading.cpp
    src/tools/StringHash.cpp
    src/tools/StringTools.cpp
    src/tools/TextTools.cpp
    src/tools/grapheme/GraphemeIterator.cpp
    src/tools/grapheme/GraphemeRange.cpp
    src/tools/parsing/Token.cpp
    src/tools/parsing/ParseContext.cpp
    src/tools/parsing/SyntaxErr.cpp
    src/tools/parsing/BaseTokenizer.cpp
    src/tools/parsing/FilteredTokenizer.cpp
    src/tools/parsing/IfPredTokenizer.cpp
    src/tools/parsing/SympyTokenizer.cpp
    src/tools/hashing/HashInt.cpp
    src/tools/hashing/Hash128.cpp
    src/tools/hashing/Hash64.cpp
    src/tools/hashing/HashTools.cpp
    src/tools/idGenerator/UuidIdGenerator.cpp
    src/model/iftemplate/IfTemplatePart.cpp
    src/model/iftemplate/SympyParser.cpp
    src/model/iftemplate/IfPredParser.cpp
    src/model/iftemplate/IfPredZ3Generator.cpp
    src/model/iftemplate/Z3IfPredGenerator.cpp
    src/model/iftemplate/IfPredPart.cpp
    src/tools/z3/Z3Context.cpp
    src/tools/z3/Z3Predicate.cpp
    src/model/strategies/ModTypeIdData.cpp
    src/model/assets/Hashes.cpp
    src/model/assets/Indices.cpp
    src/model/assets/VertexCounts.cpp
    src/model/assets/VGRemaps.cpp
    src/model/strategies/ModType.cpp
    src/model/strategies/iniClassifiers/IniClassifyStats.cpp
    src/model/strategies/iniClassifiers/BaseIniClassifier.cpp
    src/model/strategies/iniClassifiers/IniClassifier.cpp
    src/model/buffers/BufType.cpp
    src/model/buffers/BufDataType.cpp
    src/model/buffers/BufInt.cpp
    src/model/buffers/BufFloat.cpp
    src/model/buffers/BufUnorm.cpp
    src/model/buffers/BufElementType.cpp
    src/model/files/BufFileErrors.cpp
    src/model/files/BinaryFile.cpp
    src/model/files/BufFile.cpp
    src/model/VGRemap.cpp
    src/model/files/BlendFile.cpp
    src/model/files/PositionFile.cpp
    src/model/strategies/iniParsers/IniParseBuilder.cpp
    src/model/strategies/iniFixers/IniFixBuilder.cpp
    src/model/strategies/iniRemovers/BaseIniRemover.cpp
    src/model/strategies/iniRemovers/IniRemoveBuilder.cpp
    src/model/strategies/iniFixers/BaseIniPartEdit.cpp
    src/model/files/IniFile.cpp
    src/model/IniNamingTools.cpp
    src/model/textures/Colour.cpp
    src/model/textures/ColourRange.cpp
    src/model/files/TextureFile.cpp
    src/model/strategies/texEditors/pixelTransforms/BasePixelTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/CorrectGamma.cpp
    src/model/strategies/texEditors/pixelTransforms/ColourReplace.cpp
    src/model/strategies/texEditors/pixelTransforms/HighlightShadow.cpp
    src/model/strategies/texEditors/pixelTransforms/InvertAlpha.cpp
    src/model/strategies/texEditors/pixelTransforms/TempControl.cpp
    src/model/strategies/texEditors/pixelTransforms/TintTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/Transparency.cpp
    src/model/strategies/texEditors/texFilters/BaseTexFilter.cpp
    src/model/strategies/texEditors/texFilters/GammaFilter.cpp
    src/model/strategies/texEditors/texFilters/ColourReplaceFilter.cpp
    src/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.cpp
    src/model/strategies/texEditors/texFilters/InvertAlphaFilter.cpp
    src/model/strategies/texEditors/texFilters/HueAdjust.cpp
    src/model/strategies/texEditors/texFilters/PixelFilter.cpp
    src/model/strategies/texEditors/BaseTexEditor.cpp
    src/model/strategies/texEditors/TexEditor.cpp
    src/model/strategies/texEditors/TexCreator.cpp
    src/tools/files/FileService.cpp
    src/tools/files/FileDownload.cpp
    src/model/stats/FileStats.cpp
    src/model/stats/CachedFileStats.cpp
    src/model/stats/RemapStats.cpp
    src/model/iniresources/IniSrcResourceModel.cpp
    src/model/iniresources/IniFixResourceModel.cpp
    src/model/iniresources/IniTexModel.cpp
    src/model/iniresources/IniDownloadModel.cpp
    src/model/iniresources/IniResource.cpp
    src/model/iniresources/RemapIniResource.cpp
    src/model/iniresources/RemapBlendResource.cpp
    src/model/iniresources/RemapTexResource.cpp

    ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash/xxhash.c
)

add_library(AGRemapCore::AGRemapCore ALIAS AGRemapCore)

# -----------------------------------
# UTF8Proc
# -----------------------------------

set(UTF8PROC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8PROC_INSTALL OFF CACHE BOOL "" FORCE)

set(UTF8PROC_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../extern/utf8proc)
set(UTF8PROC_BIN ${CMAKE_BINARY_DIR}/utf8proc)

add_subdirectory(${UTF8PROC_SRC} ${UTF8PROC_BIN})

# -----------------------------------
# Compressonator
# -----------------------------------

set(COMPRESSONATOR_SRC
    ${CMAKE_CURRENT_LIST_DIR}/../../../extern/Compressonator
)

# Compressonator's child CMakeLists expect these platform variables.
if(WIN32)
    set(CMP_HOST_WINDOWS ON)
elseif(UNIX)
    set(CMP_HOST_LINUX ON)
endif()

# Disable optional components that would pull in additional dependencies.
set(OPTION_BUILD_ASTC OFF)
set(OPTION_BUILD_BROTLIG OFF)
set(OPTION_CMP_GTC OFF)
set(OPTION_CMP_APC OFF)

message(STATUS "BEFORE Compressonator:")
message(STATUS "  PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
message(STATUS "  CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

# Compressonator's child CMakeLists use PROJECT_SOURCE_DIR
# assuming it points to the Compressonator source root.
set(AGREMAP_PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_SOURCE_DIR ${COMPRESSONATOR_SRC})

if(MSVC)
    add_compile_options(/Zc:strictStrings-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Compressonator's applications/_plugins/common headers (cmp_fileio.h, utilfuncs.h) refer to
    #   uintmax_t/uint16_t without including <cstdint>. MSVC pulls those in transitively, GCC 13+
    #   does not, so on Linux they fail with "'uintmax_t' does not name a type" / "'uint16_t' was
    #   not declared in this scope" -- plus a cascading, misleading "redefinition of 'float
    #   HalfToFloat'" that is really just the same unparsed declaration seen twice.
    # Force-including the header fixes it without patching the vendored submodule (whose changes
    #   this repo can't commit anyway). stdint.h rather than <cstdint> so it stays valid for
    #   Compressonator's C sources too, and because the code uses these names unqualified.
    add_compile_options(-include stdint.h)
endif()

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_core
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_core
    EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_framework
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_framework
    EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_compressonatorlib
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_compressonatorlib
    EXCLUDE_FROM_ALL
)

# Restore our project's PROJECT_SOURCE_DIR.
set(PROJECT_SOURCE_DIR ${AGREMAP_PROJECT_SOURCE_DIR})


# -----------------------------------
# libcurl
# -----------------------------------

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_MISC_DOCS OFF CACHE BOOL "" FORCE)

set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
endif()

set(CURL_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../extern/curl)
set(CURL_BIN ${CMAKE_BINARY_DIR}/curl)

add_subdirectory(
    ${CURL_SRC}
    ${CURL_BIN}
    EXCLUDE_FROM_ALL
)

# -----------------------------------
# Link Libraries
# -----------------------------------

target_include_directories(AGRemapCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # BaseOrderedMultiMap.h (a public AGRemapCore header) includes tsl/ordered_map.h and
        # tsl/ordered_set.h directly, so any consumer building against AGRemapCore -- not just
        # AGRemapCore's own .cpp files -- needs this on its include path too. PUBLIC (not
        # PRIVATE) so it actually propagates; BUILD_INTERFACE-only since this vendored path
        # isn't itself installed as part of the SDK.
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../../../extern/ordered-map/include>
        # TextureFile.h (a public AGRemapCore header) includes compressonator.h directly (it's a
        # lightweight, plain-C-enum-and-struct header, unlike z3++.h -- see Z3Internal.h's own
        # comment for why *that* one stays PRIVATE behind a pimpl instead), so any consumer
        # building against AGRemapCore needs this on its include path too. Same PUBLIC +
        # BUILD_INTERFACE-only treatment as ordered-map's include dir above, for the same reason.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../extern/Compressonator/cmp_compressonatorlib>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash
        # Lets any AGRemapCore .cpp (eg. IfPredZ3Generator.cpp) reach the private,
        # never-installed "tools/z3/Z3Internal.h" (the one place z3::context::Impl/
        # z3::expr::Impl are actually defined in terms of z3++.h) via a
        # public-header-shaped path, without that path living under include/ where it'd
        # ship as part of the public SDK.
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AGRemapCore
    PRIVATE
        z3::libz3
        utf8proc
        CMP_Compressonator
        CMP_Framework
        CURL::libcurl
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